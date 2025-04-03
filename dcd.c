#include "include/common_mbox_function.h"
#include "include/cxl_dcd.h"
#include "include/cxl_log.h"
#include <string.h>

static struct dc_config g_dc_config = {
    .num_regions = 4,
    .regions = {
        {0x00000000ULL, 0x10000000ULL, 0x10000000ULL, 0x10000ULL, 0, 0}, // Region 0: Base 0 MB, 256 MB
        {0x10000000ULL, 0x10000000ULL, 0x10000000ULL, 0x10000ULL, 1, 0}, // Region 1: Base 256 MB
        {0x20000000ULL, 0x10000000ULL, 0x10000000ULL, 0x10000ULL, 2, 0}, // Region 2: Base 512 MB
        {0x30000000ULL, 0x10000000ULL, 0x10000000ULL, 0x10000ULL, 3, 0}, // Region 3: Base 768 MB
    },
    .total_supported_extents = 1000,
    .available_extents = 500,
    .total_supported_tags = 100,
    .available_tags = 50
};

static int total_supported_extents = 128;  // Total number of extents supported by the device
static int total_supported_tags = 64;      // Total number of tags supported by the device
extern struct extent g_accepted_extents[MAX_EXTENTS]; // List of accepted extents
extern int g_accepted_extent_count;        // Number of accepted extents

// Calculate the number of available regions
static int get_available_regions() {
    //
}

// Calculate the number of available extents
static int get_available_extents() {
    //
}

// Calculate the number of available tags (simplified, assumes one tag per used region)
static int get_available_tags() {
    //
}

static struct dc_config dcd;

// Opcode 4800h: Get Dynamic Capacity Configuration
int mailbox_get_dynamic_capacity_configuration(int opcode, int *payload_length, char *payload){
    u8 region_count = payload[0];
    u8 start_idx = payload[1];
    u8 regions_returned = 0;
    char *output = payload;

    // Validate input parameters
    if (*payload_length != 2) {
        xil_printf("Invalid payload length for DCD config\n");
        return INVALID_PAYLOAD_LENGTH;
    }

    if (start_idx >= dcd.num_regions) {
        xil_printf("Invalid start index for DCD regions\n");
        return INVALID_INPUT;
    }

    // Prepare output payload
    output[0] = dcd.num_regions;          // Number of Available Regions
    output[1] = 0;                        // Regions Returned (initialize to 0)
    memset(&output[2], 0, 6);             // Reserved bytes

    // Calculate actual regions to return
    u8 output_offset = 8;
    regions_returned = (region_count > (dcd.num_regions - start_idx)) ? 
                      (dcd.num_regions - start_idx) : region_count;

    // Fill region configurations
    for (int i = 0; i < regions_returned; i++) {
        struct dc_region_config *reg = &dcd.regions[start_idx + i];
        
        // Fill region structure
        memcpy(&output[output_offset], &reg->region_base, 8);
        memcpy(&output[output_offset+8], &reg->region_decode_length, 8);
        memcpy(&output[output_offset+16], &reg->region_length, 8);
        memcpy(&output[output_offset+24], &reg->region_block_size, 8);
        memcpy(&output[output_offset+32], &reg->dsmad_handle, 4);
        output[output_offset+36] = reg->flags;
        memset(&output[output_offset+37], 0, 3);  // Reserved bytes
        
        output_offset += 40;  // Size of each region structure
    }

    // Add extent and tag information
    memcpy(&output[output_offset], &dcd.total_extents, 4);
    memcpy(&output[output_offset+4], &dcd.avail_extents, 4);
    memcpy(&output[output_offset+8], &dcd.total_tags, 4);
    memcpy(&output[output_offset+12], &dcd.avail_tags, 4);
    
    // Update regions returned count
    output[1] = regions_returned;
    
    // Calculate total payload length
    *payload_length = output_offset + 16;  // 16 bytes for extent/tag info
    
    return SUCCESS;
}

// Opcode 4801h: Get Dynamic Capacity Extent List
int mailbox_get_dynamic_capacity_extent_list(int opcode, int *payload_length, char *payload){
    // Extract input payload fields
    uint32_t extent_count;
    uint32_t starting_extent_index;
    memcpy(&extent_count, payload, 4);               // Bytes 0-3: Extent Count
    memcpy(&starting_extent_index, payload + 4, 4);  // Bytes 4-7: Starting Extent Index

    uint32_t g_total_extents = get_total_extents(); // Assuming this function exists
    uint32_t g_generation_number = get_generation_number(); // Assuming this function exists

    // Check if starting_extent_index is valid
    if (starting_extent_index > g_total_extents) {
        return INVALID_INPUT;
    }

    // If extent_count is 0, return only header information
    if (extent_count == 0) {
        char output_payload[16] = {0};
        memcpy(output_payload + 4, &g_total_extents, 4);      // Total Extent Count
        memcpy(output_payload + 8, &g_generation_number, 4);  // Generation Number
        // Bytes 0-3 (Returned Extent Count) and 12-15 (Reserved) are 0
        memcpy(payload, output_payload, 16);
        *payload_length = 16;
        return SUCCESS;
    }

    // Calculate number of extents to return
    uint32_t extents_to_return = g_total_extents - starting_extent_index;
    if (extents_to_return > extent_count) {
        extents_to_return = extent_count;  // Cap at requested count
    }

    // Calculate output payload size: 16-byte header + extents
    const int extent_size = sizeof(struct dc_extent); 
    int output_size = 16 + extent_size * extents_to_return;
    if (output_size > CXL_MAX_PAYLOAD_SIZE) {
        return INTERNAL_ERROR;  // Payload too large
    }

    // Prepare output payload
    char output_payload[output_size];
    memset(output_payload, 0, output_size);

    // Fill header (Table 8-182)
    memcpy(output_payload, &extents_to_return, 4);      // Returned Extent Count
    memcpy(output_payload + 4, &g_total_extents, 4);    // Total Extent Count
    memcpy(output_payload + 8, &g_generation_number, 4); // Generation Number
    // Bytes 12-15 reserved, already zeroed

    // Fill extent data
    for (uint32_t i = 0; i < extents_to_return; i++) {
        uint32_t idx = starting_extent_index + i;
        struct dc_extent *extent = &g_extent_list[idx];
        char *extent_data = output_payload + 16 + i * extent_size;
        memcpy(extent_data, &extent->start_dpa, 8);
        memcpy(extent_data + 8, &extent->length, 8);
        memcpy(extent_data + 16, &extent->tag, 16);
        memcpy(extent_data + 32, &extent->flags, 2);
    }

    // Update output
    memcpy(payload, output_payload, output_size);
    *payload_length = output_size;

    return SUCCESS;
}

static void reclaim_unused_capacity(struct pending_event *event, 
                                    struct updated_extent *extent_list, 
                                    uint32_t extent_count) {
    uint64_t event_start = event->start_dpa;
    uint64_t event_end = event_start + event->length;
    uint64_t current_pos = event_start;

    // If no extents accepted, the whole thing is free
    if (extent_count == 0) {
        if (g_free_extent_count >= MAX_FREE_EXTENTS) {
            return;  // No room to track more free extents
        }
        g_free_extents[g_free_extent_count].start_dpa = event_start;
        g_free_extents[g_free_extent_count].length = event->length;
        g_free_extent_count++;
        return;
    }

    // Go through the accepted extents and find gaps
    for (uint32_t i = 0; i < extent_count; i++) {
        uint64_t extent_start = extent_list[i].start_dpa;
        uint64_t extent_end = extent_start + extent_list[i].length;

        // If there’s a gap before this extent, mark it free
        if (current_pos < extent_start) {
            if (g_free_extent_count < MAX_FREE_EXTENTS) {
                g_free_extents[g_free_extent_count].start_dpa = current_pos;
                g_free_extents[g_free_extent_count].length = extent_start - current_pos;
                g_free_extent_count++;
            }
        }
        current_pos = extent_end;  // Move past this extent
    }

    // Check for a gap at the end
    if (current_pos < event_end) {
        if (g_free_extent_count < MAX_FREE_EXTENTS) {
            g_free_extents[g_free_extent_count].start_dpa = current_pos;
            g_free_extents[g_free_extent_count].length = event_end - current_pos;
            g_free_extent_count++;
        }
    }
}

/**
 * @brief Initializes sample pending Add Capacity Events.
 * This is a placeholder for actual event generation logic, typically tied to event logs.
 */
static void init_pending_events(void) {
    g_pending_event_count = 2;  // Example: Initialize with 2 pending events
    g_pending_events[0].start_dpa = 0x10000000ULL;  // Starting at 256MB
    g_pending_events[0].length = 0x10000000ULL;     // 256MB length
    g_pending_events[0].processed = false;
    g_pending_events[1].start_dpa = 0x20000000ULL;  // Starting at 512MB
    g_pending_events[1].length = 0x10000000ULL;     // 256MB length
    g_pending_events[1].processed = false;
}

/** Opcode 4802h: Add Dynamic Capacity Response, In response to an Add Capacity Event Record
 *  This processes the host's reply to an "Add Capacity Event Record", checking what the host
 *  wants and updating the device's records.
    After this command is received, the device is free to reclaim capacity that the host does not utilize.*/
int mailbox_add_dynamic_capacity_response(int opcode, int *payload_length, char *payload){
    // Check if the payload is big enough (needs at least 8 bytes for header)
    if (*payload_length < 8) {
        return INVALID_INPUT;
    }

    // Grab the number of extents the host wants to accept (Bytes 0-3)
    uint32_t updated_extent_list_size;
    memcpy(&updated_extent_list_size, payload, 4);

    // Check the Flags byte (Byte 4), especially the "More" bit
    uint8_t flags = payload[4];
    bool more_flag = (flags & 0x01) != 0;  // More bit means there's another response coming

    // Make sure the payload size matches what we expect
    int extent_size = sizeof(struct updated_extent);
    int expected_size = 8 + updated_extent_list_size * extent_size;
    if (*payload_length != expected_size) {
        return INVALID_INPUT;  // Wrong size, something's off
    }

    // Make sure the host is replying to the right event in order
    if (g_expected_response_index >= g_pending_event_count || g_pending_events[g_expected_response_index].processed) {
        return INVALID_INPUT;  // Out of order or already done
    }

    // Get the event we're working on
    struct pending_event *event = &g_pending_events[g_expected_response_index];
    uint64_t event_start_dpa = event->start_dpa;
    uint64_t event_length = event->length;
    uint64_t event_end_dpa = event_start_dpa + event_length;
    bool is_shared = event->is_shared;

    // Point to the list of extents the host wants (starts at Byte 8)
    struct updated_extent *extent_list = (struct updated_extent *)(payload + 8);

    // Handle depending on if it's shared or not
    if (is_shared) {
        // Shared capacity: Host has to take all or nothing
        if (updated_extent_list_size == 0) {
            // Host doesn't want it

        } else if (updated_extent_list_size == 1) {
            struct updated_extent *extent = &extent_list[0]; // first and only extent
            // Check if the host took the whole thing
            if (extent->start_dpa != event_start_dpa || extent->length != event_length) {
                return INVALID_EXTENT_LIST;  // Gotta take all or nothing for shared
            }
            // Make sure we have room to record it
            if (g_accepted_extent_count + 1 > MAX_EXTENTS) {
                return RESOURCES_EXHAUSTED;
            }
            // Record the accepted extent
            memcpy(&g_accepted_extents[g_accepted_extent_count], extent, extent_size);
            g_accepted_extent_count++;
        } else {
            return INVALID_EXTENT_LIST;  // Can't have more than one extent for shared
        }
    } else {
        // Non-shared capacity: Check each extent the host wants
        for (uint32_t i = 0; i < updated_extent_list_size; i++) {
            uint64_t start_dpa = extent_list[i].start_dpa;
            uint64_t length = extent_list[i].length;
            uint64_t end_dpa = start_dpa + length;

            // Make sure the range fits in what we offered
            if (start_dpa < event_start_dpa || end_dpa > event_end_dpa || length == 0) {
                return INVALID_PHYSICAL_ADDRESS;
            }
            // Check if it's aligned properly
            if ((start_dpa % BLOCK_SIZE) != 0 || (length % BLOCK_SIZE) != 0) {
                return INVALID_EXTENT_LIST;
            }
            // Look for overlaps with stuff we already gave out
            for (int j = 0; j < g_accepted_extent_count; j++) {
                uint64_t accepted_start = g_accepted_extents[j].start_dpa;
                uint64_t accepted_end = accepted_start + g_accepted_extents[j].length;
                if (start_dpa < accepted_end && end_dpa > accepted_start) {
                    return INVALID_PHYSICAL_ADDRESS;  // No overlapping allowed
                }
            }
            // Make sure we can store it
            if (g_accepted_extent_count + 1 > MAX_EXTENTS) {
                return RESOURCES_EXHAUSTED;
            }
            // Record the accepted extent
            memcpy(&g_accepted_extents[g_accepted_extent_count], &extent_list[i], extent_size);
            g_accepted_extent_count++;
        }
    }

    // If there's no more responses coming, mark this event as done
    if (!more_flag) {
        reclaim_unused_capacity(event, extent_list, updated_extent_list_size);
        event->processed = true;
        g_expected_response_index++;  // Move on to the next event
    }

    return SUCCESS;  // All good!
}

int mailbox_release_dynamic_capacity(int opcode, int *payload_length, char *payload){
    // Validate payload length (minimum 8 bytes: 4 for size, 1 for flags, 3 reserved)
    if (*payload_length < 8) {
        return INVALID_INPUT;
    }

    // Extract Updated Extent List Size (Bytes 0-3)
    uint32_t updated_extent_list_size;
    memcpy(&updated_extent_list_size, payload, 4);
    if (updated_extent_list_size == 0) {
        return INVALID_INPUT;  // Must have at least one extent
    }

    // Extract Flags (Byte 4), check More bit
    uint8_t flags = payload[4];
    bool more_flag = (flags & 0x01) != 0;

    // Calculate expected payload size
    int extent_size = sizeof(struct extent);
    int expected_size = 8 + updated_extent_list_size * extent_size;
    if (*payload_length != expected_size) {
        return INVALID_INPUT;
    }

    // Point to the Updated Extent List (starts at Byte 8)
    struct extent *release_list = (struct extent *)(payload + 8);

    // Validate each extent
    for (uint32_t i = 0; i < updated_extent_list_size; i++) {
        uint64_t release_start = release_list[i].start_dpa;
        uint64_t release_length = release_list[i].length;
        uint64_t release_end = release_start + release_length;

        // Check alignment and length
        if ((release_start % REGION_BLOCK_SIZE) != 0 || 
            (release_length % REGION_BLOCK_SIZE) != 0 || 
            release_length == 0) {
            return INVALID_EXTENT_LIST;
        }

        // Check for overlap in the release list
        for (uint32_t j = 0; j < i; j++) {
            uint64_t other_start = release_list[j].start_dpa;
            uint64_t other_end = other_start + release_list[j].length;
            if (release_start < other_end && release_end > other_start) {
                return INVALID_EXTENT_LIST;
            }
        }

        // Check if extent is in accepted list
        bool found = false;
        for (int j = 0; j < g_accepted_extent_count; j++) {
            if (g_accepted_extents[j].start_dpa == release_start && 
                g_accepted_extents[j].length == release_length) {
                found = true;
                break;
            }
        }
        if (!found) {
            return INVALID_PHYSICAL_ADDRESS;
        }

        // Check if already released
        for (int j = 0; j < g_free_extent_count; j++) {
            if (g_free_extents[j].start_dpa == release_start && 
                g_free_extents[j].length == release_length) {
                return INVALID_PHYSICAL_ADDRESS;
            }
        }
    }

    // Process the release
    for (uint32_t i = 0; i < updated_extent_list_size; i++) {
        uint64_t release_start = release_list[i].start_dpa;
        uint64_t release_length = release_list[i].length;

        // Remove from accepted extents
        for (int j = 0; j < g_accepted_extent_count; j++) {
            if (g_accepted_extents[j].start_dpa == release_start && 
                g_accepted_extents[j].length == release_length) {
                for (int k = j; k < g_accepted_extent_count - 1; k++) {
                    g_accepted_extents[k] = g_accepted_extents[k + 1];
                }
                g_accepted_extent_count--;
                break;
            }
        }

        // Add to free extents
        if (g_free_extent_count + 1 > MAX_FREE_EXTENTS) {
            return RESOURCES_EXHAUSTED;
        }
        memcpy(&g_free_extents[g_free_extent_count], &release_list[i], extent_size);
        g_free_extent_count++;
    }

    return SUCCESS;
}

int mailbox_get_dcd_info(int opcode, int *payload_length, char *payload){
    // No input payload expected
    if (*payload_length != 0) {
        return INVALID_PAYLOAD_LENGTH;  // Not strictly in spec, but enforces no input
    }

    // Calculate output payload size: 12 bytes fixed + 8 bytes per region
    int output_size = 12 + 8 * g_dcd_info.num_regions;
    if (output_size > CXL_MAX_PAYLOAD_SIZE) {
        return INTERNAL_ERROR;  // Payload exceeds maximum size
    }

    // Prepare output payload
    char output_payload[output_size];
    memset(output_payload, 0, output_size);

    // Fill fields per Table 7-63
    output_payload[0] = g_dcd_info.num_hosts;                        // Byte 0: Number of Hosts
    output_payload[1] = g_dcd_info.num_regions;                     // Byte 1: Number of Supported DC Regions
    // Bytes 2-3: Reserved (already zeroed)
    memcpy(output_payload + 4, &g_dcd_info.add_capacity_policies, 2);    // Bytes 4-5: Add Capacity Policies
    // Bytes 6-7: Reserved (already zeroed)
    memcpy(output_payload + 8, &g_dcd_info.release_capacity_policies, 2); // Bytes 8-9: Release Capacity Policies
    output_payload[10] = g_dcd_info.sanitize_on_release_mask;       // Byte 10: Sanitize on Release Mask
    // Byte 11: Reserved (already zeroed)
    memcpy(output_payload + 12, &g_dcd_info.total_capacity, 8);     // Bytes 12-19: Total Dynamic Capacity

    // Fill Region Supported Block Size Masks (Bytes 20+)
    for (int i = 0; i < g_dcd_info.num_regions; i++) {
        memcpy(output_payload + 20 + i * 8, &g_dcd_info.region_block_size_masks[i], 8);
    }

    // Update output
    memcpy(payload, output_payload, output_size);
    *payload_length = output_size;

    return SUCCESS;
}

int mailbox_get_host_dc_region_configuration(int opcode, int *payload_length, char *payload){
    // Validate input payload length (4 bytes as per spec)
    if (*payload_length != 4) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint16_t host_id;
    uint8_t region_count;
    uint8_t starting_region_index;
    memcpy(&host_id, payload, 2);          // Bytes 0-1: Host ID
    region_count = payload[2];             // Byte 2: Region Count
    starting_region_index = payload[3];    // Byte 3: Starting Region Index

    // Validate Host ID
    if (host_id >= MAX_HOSTS) {
        return INVALID_INPUT;
    }

    struct host_config *config = &g_host_configs[host_id];

    // Validate Starting Region Index
    if (starting_region_index >= config->num_available_regions) {
        return INVALID_INPUT;
    }

    // Calculate regions to return
    uint8_t regions_to_return = config->num_available_regions - starting_region_index;
    if (regions_to_return > region_count) {
        regions_to_return = region_count;
    }

    // Calculate output payload size: 4 bytes header + 40 bytes per region + 16 bytes extents/tags
    int output_size = 4 + 40 * regions_to_return + 16;
    if (output_size > CXL_MAX_PAYLOAD_SIZE) {
        return INTERNAL_ERROR;  // Payload exceeds maximum size
    }

    // Prepare output payload
    char output_payload[output_size];
    memset(output_payload, 0, output_size);

    // Fill header (Bytes 0-3)
    memcpy(output_payload, &config->host_id, 2);           // Bytes 0-1: Host ID
    output_payload[2] = config->num_available_regions;     // Byte 2: Number of Available Regions
    output_payload[3] = regions_to_return;                 // Byte 3: Number of Regions Returned

    // Fill Region Configuration List (Bytes 4+)
    for (int i = 0; i < regions_to_return; i++) {
        int idx = starting_region_index + i;
        struct dc_region_config *region = &config->regions[idx];
        char *region_data = output_payload + 4 + i * 40;
        memcpy(region_data, &region->region_base, 8);      // Bytes 0-7: Region Base
        memcpy(region_data + 8, &region->decode_length, 8); // Bytes 8-15: Decode Length
        memcpy(region_data + 16, &region->length, 8);      // Bytes 16-23: Length
        memcpy(region_data + 24, &region->block_size, 8);  // Bytes 24-31: Block Size
        region_data[32] = region->flags1;                  // Byte 32: Flags1
        // Bytes 33-35: Reserved (zeroed)
        region_data[36] = region->flags2;                  // Byte 36: Flags2
        // Bytes 37-39: Reserved (zeroed)
    }

    // Fill extents and tags (after regions)
    int offset = 4 + 40 * regions_to_return;
    memcpy(output_payload + offset, &config->total_supported_extents, 4);  // Total Supported Extents
    memcpy(output_payload + offset + 4, &config->available_extents, 4);    // Available Extents
    memcpy(output_payload + offset + 8, &config->total_supported_tags, 4); // Total Supported Tags
    memcpy(output_payload + offset + 12, &config->available_tags, 4);      // Available Tags

    // Update output
    memcpy(payload, output_payload, output_size);
    *payload_length = output_size;

    return SUCCESS;
}

int mailbox_set_dc_region_configuration(int opcode, int *payload_length, char *payload){
    // Validate payload length (16 bytes as per spec)
    if (*payload_length != 16) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint8_t region_id = payload[0];          // Byte 0: Region ID
    uint64_t region_block_size;
    uint8_t flags;
    memcpy(&region_block_size, payload + 4, 8);  // Bytes 4-11: Region Block Size
    flags = payload[12];                         // Byte 12: Flags (Sanitize on Release)

    // Validate Region ID
    if (region_id >= MAX_REGIONS) {
        return INVALID_INPUT;
    }

    // Check if device is locked (confidential computing)
    if (g_device_locked) {
        return INVALID_SECURITY_STATE;
    }

    // Check if all capacity is released across all hosts for this region
    for (int h = 0; h < MAX_HOSTS; h++) {
        struct host_config *config = &g_host_configs[h];
        if (region_id < config->num_available_regions) {
            if (config->regions[region_id].length != 0) {
                return UNSUPPORTED;  // Capacity still allocated
            }
        }
    }

    // Validate block size against supported mask
    uint64_t block_size_mask = g_dcd_info.region_block_size_masks[region_id];
    int block_size_shift = __builtin_ctzll(region_block_size);  // Count trailing zeros
    if (block_size_shift < 6 || block_size_shift > 51 || !(block_size_mask & (1ULL << block_size_shift))) {
        return INVALID_INPUT;  // Block size not supported or invalid
    }

    // Validate Sanitize on Release configuration
    bool sanitize_on_release = (flags & 0x01) != 0;
    bool sanitize_configurable = (g_dcd_info.sanitize_on_release_mask & (1 << region_id)) != 0;
    struct dc_region_config *region = &g_host_configs[0].regions[region_id];  // Assuming Host 0 for simplicity
    if (!sanitize_configurable && sanitize_on_release != (region->flags2 & 0x01)) {
        return UNSUPPORTED;  // Sanitize setting mismatch and not configurable
    }

    // Update region configuration
    region->block_size = region_block_size;
    region->flags2 = sanitize_on_release ? 0x01 : 0x00;

    // Generate event record
    generate_region_config_updated_event(region_id);

    return SUCCESS;
}

int mailbox_get_dc_region_extent_lists(int opcode, int *payload_length, char *payload){
    // Validate input payload length (12 bytes as per spec)
    if (*payload_length != 12) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint16_t host_id;
    uint32_t extent_count;
    uint32_t starting_extent_index;
    memcpy(&host_id, payload, 2);           // Bytes 0-1: Host ID
    memcpy(&extent_count, payload + 4, 4);  // Bytes 4-7: Extent Count
    memcpy(&starting_extent_index, payload + 8, 4); // Bytes 8-11: Starting Extent Index

    // Validate Host ID
    if (host_id >= MAX_HOSTS) {
        return INVALID_INPUT;
    }

    struct host_extent_list *list = &g_host_extent_lists[host_id];

    // Validate Starting Extent Index
    if (starting_extent_index > list->total_extent_count) {
        return INVALID_INPUT;
    }

    // If extent_count is 0, return only header information
    if (extent_count == 0) {
        char output_payload[24] = {0};  // 24 bytes for header
        memcpy(output_payload, &host_id, 2);               // Bytes 0-1: Host ID
        memcpy(output_payload + 4, &starting_extent_index, 4); // Bytes 4-7: Starting Extent Index
        // Bytes 8-11: Returned Extent Count (0)
        memcpy(output_payload + 12, &list->total_extent_count, 4); // Bytes 12-15: Total Extent Count
        memcpy(output_payload + 16, &list->generation_number, 4);  // Bytes 16-19: Generation Number
        memcpy(payload, output_payload, 24);
        *payload_length = 24;
        return SUCCESS;
    }

    // Calculate extents to return
    uint32_t extents_to_return = list->total_extent_count - starting_extent_index;
    if (extents_to_return > extent_count) {
        extents_to_return = extent_count;
    }

    // Calculate output payload size: 24 bytes header + 40 bytes per extent
    int output_size = 24 + 40 * extents_to_return;
    if (output_size > CXL_MAX_PAYLOAD_SIZE) {
        return INTERNAL_ERROR;  // Payload exceeds maximum size
    }

    // Prepare output payload
    char output_payload[output_size];
    memset(output_payload, 0, output_size);

    // Fill header (Bytes 0-23)
    memcpy(output_payload, &host_id, 2);               // Bytes 0-1: Host ID
    // Bytes 2-3: Reserved (zeroed)
    memcpy(output_payload + 4, &starting_extent_index, 4); // Bytes 4-7: Starting Extent Index
    memcpy(output_payload + 8, &extents_to_return, 4);     // Bytes 8-11: Returned Extent Count
    memcpy(output_payload + 12, &list->total_extent_count, 4); // Bytes 12-15: Total Extent Count
    memcpy(output_payload + 16, &list->generation_number, 4);  // Bytes 16-19: Generation Number
    // Bytes 20-23: Reserved (zeroed)

    // Fill Extent List (Bytes 24+)
    for (uint32_t i = 0; i < extents_to_return; i++) {
        uint32_t idx = starting_extent_index + i;
        struct dc_extent *extent = &list->extents[idx];
        char *extent_data = output_payload + 24 + i * 40;
        memcpy(extent_data, &extent->start_dpa, 8);          // Bytes 0-7: Starting DPA
        memcpy(extent_data + 8, &extent->length, 8);         // Bytes 8-15: Length
        memcpy(extent_data + 16, extent->tag, 16);           // Bytes 16-31: Tag
        memcpy(extent_data + 32, &extent->shared_extent_sequence, 2); // Bytes 32-33: Shared Extent Sequence
        memcpy(extent_data + 34, extent->reserved, 6);       // Bytes 34-39: Reserved
    }

    // Update output
    memcpy(payload, output_payload, output_size);
    *payload_length = output_size;

    return SUCCESS;
}

int mailbox_initiate_dynamic_capacity_add(int opcode, int *payload_length, char *payload) {
    // Verify minimum payload length (28 bytes, excluding Extent List)
    if (*payload_length < 28) {
        return INVALID_INPUT;
    }

    // Extract fields
    uint16_t host_id;
    memcpy(&host_id, payload, 2);
    if (host_id >= MAX_HOSTS) {
        return INVALID_INPUT; // Invalid Host ID
    }

    uint8_t selection_policy = payload[2] & 0x0F;
    if (selection_policy > POLICY_ENABLE_SHARED_ACCESS) {
        return INVALID_INPUT; // Unsupported selection policy
    }

    uint8_t region_num = payload[3];
    if (region_num >= MAX_REGIONS && selection_policy != POLICY_PRESCRIPTIVE) {
        return INVALID_INPUT; // Invalid region number
    }

    uint64_t length;
    memcpy(&length, payload + 4, 8);
    if ((selection_policy == POLICY_FREE || selection_policy == POLICY_CONTIGUOUS) &&
        (length == 0 || (length % g_regions[region_num].block_size) != 0)) {
        return INVALID_INPUT; // Length must be > 0 and a multiple of block size
    }

    uint8_t tag[16];
    memcpy(tag, payload + 12, 16);

    // Check if the event log will overflow
    if (queueSize(&event_log, INFORMATION_EVENT_SEVERITY) >= EVENT_OVERFLOW_CAP - 1) {
        return RETRY_REQUIRED;
    }

    // Handle based on selection policy
    if (selection_policy == POLICY_FREE || selection_policy == POLICY_CONTIGUOUS) {
        // Check region capacity
        if (g_regions[region_num].current_capacity + length > g_regions[region_num].total_capacity) {
            return RESOURCES_EXHAUSTED; // Exceeds decode length
        }

    } else if (selection_policy == POLICY_PRESCRIPTIVE) {
        
    } else if (selection_policy == POLICY_ENABLE_SHARED_ACCESS) {
        
    }

    // Set command effect
    char *command_effect = get_command_effect(0b00010111); // Immediate Config/Data Change + Config Change after Reset
    cxl_gen_vdl(opcode, command_effect);

    *payload_length = 0; // No output payload
    return SUCCESS;
}


int mailbox_initiate_dynamic_capacity_release(int opcode, int *payload_length, char *payload){
    // Validate minimum payload length (28 bytes for non-Prescriptive)
    if (*payload_length < 28) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint16_t host_id;
    uint8_t flags;
    uint64_t length;
    uint8_t tag[16];
    uint32_t extent_count = 0;
    memcpy(&host_id, payload, 2);            // Bytes 0-1: Host ID
    flags = payload[2];                      // Byte 2: Flags
    uint8_t removal_policy = flags & 0x0F;   // Bits 3:0: Removal Policy
    bool forced_removal = (flags & 0x10) != 0; // Bit 4: Forced Removal
    bool sanitize_on_release = (flags & 0x20) != 0; // Bit 5: Sanitize on Release
    memcpy(&length, payload + 4, 8);         // Bytes 4-11: Length
    memcpy(tag, payload + 12, 16);           // Bytes 12-27: Tag
    if (removal_policy == POLICY_PRESCRIPTIVE) {
        memcpy(&extent_count, payload + 28, 4);  // Bytes 28-31: Extent Count
        if (*payload_length != 32 + extent_count * sizeof(struct dc_extent)) {
            return INVALID_PAYLOAD_LENGTH;
        }
    }

    // Validate Host ID
    if (host_id >= MAX_HOSTS) {
        return INVALID_INPUT;
    }

    // Validate Removal Policy
    if (removal_policy > POLICY_PRESCRIPTIVE || 
        !(g_dcd_info.release_capacity_policies & (1 << removal_policy))) {
        return INVALID_INPUT;
    }

    // Validate Sanitize on Release support
    if (sanitize_on_release && !(g_dcd_info.sanitize_on_release_mask & (1 << 0))) {  // Assuming Region 0 for simplicity
        return INVALID_INPUT;
    }

    // Policy-specific validations
    struct dc_extent *extent_list = NULL;
    uint64_t total_assigned_capacity = g_host_configs[host_id].regions[0].length;  // Example: Region 0
    if (removal_policy == POLICY_TAG_BASED) {


    } else if (removal_policy == POLICY_PRESCRIPTIVE) {
        
        
    }

    // Check event log overflow (unless forced removal)
    bool event_log_would_overflow = false;  // Placeholder: Implement actual check
    if (event_log_would_overflow && !forced_removal) {
        return RETRY_REQUIRED;
    }

    // Initiate remove capacity procedure
    initiate_remove_capacity(host_id, length, removal_policy, tag, extent_list, extent_count, 
                             forced_removal, sanitize_on_release);

    return SUCCESS;
}

int mailbox_dynamic_capacity_add_reference(int opcode, int *payload_length, char *payload){
    // Validate payload length (16 bytes as per spec)
    if (*payload_length != 16) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint8_t tag[16];
    memcpy(tag, payload, 16);  // Bytes 0-15: Tag

    // Check if tag exists and is sharable
    bool tag_found = false;
    int tag_idx = -1;
    for (int i = 0; i < g_referenced_tag_count; i++) {
        if (memcmp(g_referenced_tags[i].tag, tag, 16) == 0) {
            tag_found = true;
            tag_idx = i;
            break;
        }
    }

    if (!tag_found) {
        // Check extents across all hosts to find the tag
        for (int h = 0; h < MAX_HOSTS; h++) {
            for (int e = 0; e < g_host_extent_lists[h].total_extent_count; e++) {
                if (memcmp(g_host_extent_lists[h].extents[e].tag, tag, 16) == 0) {
                    tag_found = true;
                    if (g_referenced_tag_count >= MAX_TAGS) {
                        return INTERNAL_ERROR;  // No space to add new tag
                    }
                    tag_idx = g_referenced_tag_count++;
                    memcpy(g_referenced_tags[tag_idx].tag, tag, 16);
                    g_referenced_tags[tag_idx].ref_count = 0;
                    g_referenced_tags[tag_idx].is_sharable = 
                        (g_host_configs[h].regions[0].flags1 & 0x08) != 0;  // Assuming Region 0
                    break;
                }
            }
            if (tag_found) break;
        }
    }

    if (!tag_found) {
        return INVALID_INPUT;  // Tag does not match existing sharable capacity
    }

    if (!g_referenced_tags[tag_idx].is_sharable) {
        return INVALID_INPUT;  // Tag is not sharable
    }

    // Add reference (no effect if already referenced)
    g_referenced_tags[tag_idx].ref_count++;

    return SUCCESS;
}

int mailbox_dynamic_capacity_remove_reference(int opcode, int *payload_length, char *payload){
    // Validate payload length (16 bytes as per spec)
    if (*payload_length != 16) {
        return INVALID_PAYLOAD_LENGTH;
    }

    // Extract input payload fields
    uint8_t tag[16];
    memcpy(tag, payload, 16);  // Bytes 0-15: Tag

    // Find the tag in referenced tags
    int tag_idx = -1;
    for (int i = 0; i < g_referenced_tag_count; i++) {
        if (memcmp(g_referenced_tags[i].tag, tag, 16) == 0) {
            tag_idx = i;
            break;
        }
    }

    if (tag_idx == -1 || !g_referenced_tags[tag_idx].is_sharable) {
        return INVALID_INPUT;  // Tag not found or not sharable
    }

    // Decrement reference count
    if (g_referenced_tags[tag_idx].ref_count > 0) {
        g_referenced_tags[tag_idx].ref_count--;
    }

    // Check if tag is still referenced by any extent lists
    bool tag_in_use = false;
    for (int h = 0; h < MAX_HOSTS; h++) {
        for (int e = 0; e < g_host_extent_lists[h].total_extent_count; e++) {
            if (memcmp(g_host_extent_lists[h].extents[e].tag, tag, 16) == 0) {
                tag_in_use = true;
                break;
            }
        }
        if (tag_in_use) break;
    }

    // If no references remain and not in use, free and sanitize
    if (g_referenced_tags[tag_idx].ref_count == 0 && !tag_in_use) {
        free_and_sanitize_capacity(tag);
        
        // Remove tag from g_referenced_tags
        for (int i = tag_idx; i < g_referenced_tag_count - 1; i++) {
            g_referenced_tags[i] = g_referenced_tags[i + 1];
        }
        g_referenced_tag_count--;

        // Apply command effects only when capacity is freed
        int effects = IMMEDIATE_CONFIG_CHANGE |
                      CONFIG_CHANGE_AFTER_COLD_RESET | CONFIG_CHANGE_AFTER_CONVENTIONAL_RESET |
                      CONFIG_CHANGE_AFTER_CXL_RESET;
        char *command_effect = get_command_effect(effects);
        cxl_gen_vdl(opcode, command_effect);  // Log with effects
    }

    return SUCCESS;
}

int mailbox_dynamic_capacity_list_tags(int opcode, int *payload_length, char *payload){
    // Validate input payload length (8 bytes per spec)
    if (*payload_length != 8) {
        return INVALID_INPUT;
    }

    // Extract input fields
    uint32_t starting_index;
    uint32_t max_tags;
    memcpy(&starting_index, payload, 4);  // Bytes 0-3: Starting Index
    memcpy(&max_tags, payload + 4, 4);    // Bytes 4-7: Max Tags

    // Validate Starting Index
    if (starting_index > g_tag_count) {
        return INVALID_INPUT;  // Starting Index exceeds total tags
    }

    // Calculate number of tags to return
    uint32_t tags_to_return = g_tag_count - starting_index;
    if (max_tags > 0 && tags_to_return > max_tags) {
        tags_to_return = max_tags;
    }

    // Calculate output payload size: 16 bytes header + 80 bytes per tag
    uint32_t output_size = 16;  // Header size (Table 7-75)
    if (max_tags > 0) {
        output_size += tags_to_return * sizeof(struct dc_tag_info);
    }
    if (output_size > CXL_MAX_PAYLOAD_SIZE) {
        return INTERNAL_ERROR;  // Exceeds mailbox capacity
    }

    // Prepare output payload
    char output_payload[CXL_MAX_PAYLOAD_SIZE];  // Use max size to avoid overflow
    memset(output_payload, 0, output_size);

    // Fill header (Bytes 0-15, Table 7-75)
    memcpy(output_payload, &g_tag_generation_number, 4);  // Bytes 0-3: Generation Number
    memcpy(output_payload + 4, &g_tag_count, 4);          // Bytes 4-7: Total Number of Tags
    memcpy(output_payload + 8, &tags_to_return, 4);       // Bytes 8-11: Number of Tags Returned
    // Byte 12: Validity Bitmap
    output_payload[12] = g_is_gfd ? 0x00 : 0x03;  // Bit 0 and 1 valid for non-GFD, 0 for GFD
    // Bytes 13-15: Reserved (already zeroed)

    // Fill Tags List if requested (Bytes 16+, Table 7-76)
    if (max_tags > 0) {
        for (uint32_t i = 0; i < tags_to_return; i++) {
            uint32_t idx = starting_index + i;
            struct dc_tag_info tag_info = {0};

            // Fill tag info
            memcpy(tag_info.tag, g_tags[idx].tag, TAG_SIZE);  // Bytes 0-15: Tag
            tag_info.flags = (g_tags[idx].ref_count > 0) ? 0x01 : 0x00;  // Byte 16: FM Holds Reference
            // Bytes 17-19: Reserved (zeroed)

            // Fill bitmaps (only if not GFD)
            if (!g_is_gfd) {
                memcpy(tag_info.ref_bitmap, g_tags[idx].ref_bitmap, BITMAP_SIZE);  // Bytes 20-51
                memcpy(tag_info.pending_bitmap, g_tags[idx].pending_bitmap, BITMAP_SIZE);  // Bytes 52-83
            }

            // Copy to output payload
            memcpy(output_payload + 16 + i * sizeof(struct dc_tag_info), 
                   &tag_info, sizeof(struct dc_tag_info));
        }
    }

    // Update output
    memcpy(payload, output_payload, output_size);
    *payload_length = output_size;

    return SUCCESS;
}