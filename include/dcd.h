#ifndef CXL_DCD_H
#define CXL_DCD_H

#include <xil_io.h>
#include <stdint.h>

#define MIN_REGIONS 1   // Minimum number of regions
#define MAX_REGIONS 8   // Maximum number of regions
#define MAX_HOSTS   16  // Max number of host IDs
#define MAX_EVENT_RECORDS   100      // Maximum number of event records
#define CXL_MAX_PAYLOAD_SIZE    1024  // Maximum payload size
#define MAX_PENDING_EVENTS  10    // Max number of events waiting for a response
#define MAX_EXTENTS 100    // Max number of extents the device can track
#define BLOCK_SIZE  0x10000ULL // Block size for alignment (e.g., 64KB)
#define MAX_FREE_EXTENTS    100

#define POLICY_FREE           0x0  // Unassigned extents, non-contiguous
#define POLICY_CONTIGUOUS     0x1  // Unassigned extents, contiguous
#define POLICY_PRESCRIPTIVE   0x2  // Specific extents in payload
#define POLICY_ENABLE_SHARED  0x3  // Enable access to shared extents

/**
 * @brief Structure representing a Dynamic Capacity region configuration.
 */
struct dc_region {
    uint64_t base;            // Region Base DPA, aligned to 256 MB
    uint64_t decode_length;   // Region Decode Length, multiple of 256 MB
    uint64_t length;          // Region Length, <= decode_length
    uint64_t block_size;      // Block Size, power of 2, multiple of 0x40
    uint32_t dsmad_handle;    // CDAT DSMAD instance handle
    uint8_t flags;            // Flags (e.g., Sanitize on Release)
};

/**
 * @brief Structure representing the device's Dynamic Capacity configuration.
 */
struct dc_config {
    uint8_t num_regions;                    // Number of available DC regions
    struct dc_region regions[MAX_REGIONS];  // Array of region configurations (max 8)
    uint32_t total_supported_extents;       // Total supported extents
    uint32_t available_extents;             // Available extents
    uint32_t total_supported_tags;          // Total supported tags
    uint32_t available_tags;                // Available tags
};


/**
 * @brief Structure representing a Dynamic Capacity Extent.
 */
struct dc_extent {
    uint64_t start_dpa; /** Always aligned to the Configured Region Block Size (dc_region.block_size) 
                            returned in Get Dynamic Capacity Configuration. */
    uint64_t length;    /** Always a multiple of the configured Region Block Size (dc_region.block_size) 
                            returned in Get Dynamic Capacity Configuration. 
                            shall be > 0, the extent described by the start_dpa and length 
                            shall not cross a DC Region boundary. */
    uint8_t tag[16];    /** Within sharable regions, all extents targeting the same range 
                            this field is required and all Tag values shall be identical 
                            for all hosts sharing the same extent. */
    uint16_t shared_extent_sequence; /** The relative order that hosts place this extent within the virtual address space. 
                                         For extents describing shareable regions this field shall be
                                         within the range of 0 to n-1 where n is the number of extents, with each
                                         value appearing only once.*/
    uint8_t reserved[6];
};

// Structure for a pending event (like an "Add Capacity Event Record")
struct pending_event {
    uint64_t start_dpa;     // Starting Device Physical Address (DPA) of the event
    uint64_t length;        // Size of the capacity in bytes
    bool is_shared;         // Is this shared capacity or not?
    bool processed;         // Has this event been fully handled?
};

/**
 * @brief Structure representing an Updated Extent as per Table 8-184.
 */
struct updated_extent {
    uint64_t start_dpa;  // Starting DPA of the extent, Bits[63:6] DPA, Bits[5:0] Reserved
    uint64_t length;     // Length of the extent in bytes, must be > 0
    uint64_t reserved;   // Reserved field
};

struct free_extent {
    uint64_t start_dpa;     // Starting DPA of the free chunk
    uint64_t length;        // Size of the free chunk in bytes
};

// Global variables to keep track of things
static struct pending_event g_pending_events[MAX_PENDING_EVENTS]; // List of pending events
static int g_pending_event_count = 0;           // How many events are waiting
static int g_expected_response_index = 0;       // Which event we're expecting a response for
static struct updated_extent g_accepted_extents[MAX_EXTENTS];   // List of extents the host took
static int g_accepted_extent_count = 0;         // How many extents the host has accepted
static struct free_extent g_free_extents[MAX_FREE_EXTENTS];     // List of free extents we can reuse
static int g_free_extent_count = 0;             // How many free extents we’ve tracked

static struct {
    uint8_t num_hosts;                  // Number of supported hosts
    uint8_t num_regions;                // Number of supported DC regions
    uint16_t add_capacity_policies;     // Supported Add Capacity Selection Policies
    uint16_t release_capacity_policies; // Supported Release Capacity Removal Policies
    uint8_t sanitize_on_release_mask;   // Sanitize on Release support mask
    uint64_t total_capacity;            // Total Dynamic Capacity in bytes
    uint64_t region_block_size_masks[MAX_REGIONS];  // Supported block sizes per region
} g_dcd_info;

//Interfaces exposed to common_mbox_function.c
int mailbox_get_dynamic_capacity_configuration(int opcode, int *payload_length, char *payload);
int mailbox_get_dynamic_capacity_extent_list(int opcode, int *payload_length, char *payload);
int mailbox_add_dynamic_capacity_response(int opcode, int *payload_length, char *payload);
int mailbox_release_dynamic_capacity(int opcode, int *payload_length, char *payload);
int mailbox_get_dcd_info(int opcode, int *payload_length, char *payload);
int mailbox_get_host_dc_region_configuration(int opcode, int *payload_length, char *payload);
int mailbox_set_dc_region_configuration(int opcode, int *payload_length, char *payload);
int mailbox_get_dc_region_extent_lists(int opcode, int *payload_length, char *payload);
int mailbox_initiate_dynamic_capacity_add(int opcode, int *payload_length, char *payload);
int mailbox_initiate_dynamic_capacity_release(int opcode, int *payload_length, char *payload);
int mailbox_dynamic_capacity_add_reference(int opcode, int *payload_length, char *payload);
int mailbox_dynamic_capacity_remove_reference(int opcode, int *payload_length, char *payload);
int mailbox_dynamic_capacity_list_tags(int opcode, int *payload_length, char *payload);

// Not completely implemented yet
uint8_t get_available_dc_regions_num(void);
uint32_t get_total_extents(void);
uint32_t get_generation_number(void);

#endif // DCD_H