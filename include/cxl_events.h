#ifndef CXL_EVENTS_H
#define CXL_EVENTS_H
#include <xil_io.h>
#include "cxl_fw.h"
//four severity
#define EVENT_RECORD_SEVERITY_NUM 4

#define SEVERITY_MASK_IN_FLAGS 0x3

//mean invalid event record handle
#define INVALID_EVENT_HANDLE -1 

//event queue size (Byte)
#define EVENT_QUE_SIZE 256*1024
#define INFO_EVENT_QUE_SIZE EVENT_QUE_SIZE
#define WARNING_EVENT_QUE_SIZE EVENT_QUE_SIZE
#define FAILURE_EVENT_QUE_SIZE EVENT_QUE_SIZE
#define FATAL_EVENT_QUE_SIZE EVENT_QUE_SIZE

//the field flags contain 3 bytes the handle contain 2 bytes
#define EVENT_RECORD_FLAGS_BYTE_SIZE 3
#define EVENT_RECORD_HANDLE_BYTE_SIZE 2


//single event record size
#define COMMON_EVENT_RECORD_SIZE 128

//The size of the data in the record, excluding the 48-byte header
#define RECORD_DATA_SIZE 80

//The number of event records that the payload size can accommodate
#define PAYLOAD_EVENT_RECORD_CAP (((CXL_MAX_PAYLOAD_SIZE) -32)/COMMON_EVENT_RECORD_SIZE)

//The number of event records that each of the four severity queues can accommodate before overflowing
#define EVENT_OVERFLOW_CAP ((EVENT_QUE_SIZE)/COMMON_EVENT_RECORD_SIZE)
#define INFO_EVENT_OVERFLOW_CAP ((INFO_EVENT_QUE_SIZE)/COMMON_EVENT_RECORD_SIZE)
#define WARNING_EVENT_OVERFLOW_CAP ((WARNING_EVENT_QUE_SIZE)/COMMON_EVENT_RECORD_SIZE)
#define FAILURE_EVENT_OVERFLOW_CAP ((FAILURE_EVENT_QUE_SIZE)/COMMON_EVENT_RECORD_SIZE)
#define FATAL_EVENT_OVERFLOW_CAP ((FATAL_EVENT_QUE_SIZE)/COMMON_EVENT_RECORD_SIZE)


// The starting address allocated to the event log area, currently occupying 1MB, partitioned from the reserved 2MB space
#define EVENT_LOG_LOW_ADDR SLOT_HIGH_ADDR
#define INFO_EVENT_ADDR EVENT_LOG_LOW_ADDR
#define WARNING_EVENT_ADDR INFO_EVENT_ADDR + EVENT_QUE_SIZE
#define FAILURE_EVENT_ADDR WARNING_EVENT_ADDR + EVENT_QUE_SIZE
#define FATAL_EVENT_INTR_ADDR FAILURE_EVENT_ADDR + EVENT_QUE_SIZE

#define EVENT_LOG_HIGH_ADDR (EVENT_LOG_LOW_ADDR+ (EVENT_QUE_SIZE * EVENT_RECORD_SEVERITY_NUM))

// The effective number of bytes containing the common header for each different event type
enum record_valid_size{
    GENERAL_MEDIA_SIZE = 128,
    DRAM_SIZE = 128,
    MEMORY_MODULE_SIZE = 128,
};

// Related to the event_record_type
enum event_record_type{
    GENERAL_MEDIA_TYPE = 0,
    DRAM_TYPE = 1,
    MEMORY_TYPE = 2,
};


//event uuid def
#define EVENT_UUID_SIZE 16
static const unsigned char general_media_event_uuid[] = {
    0xfb, 0xcd, 0x0a, 0x77,
    0xc2, 0x60, 0x41, 0x7f,
    0x85, 0xa9, 0x08, 0x8b,
    0x16, 0x21, 0xeb, 0xa6
};

static const unsigned char dram_event_uuid[] = {
    0x60, 0x1d, 0xcb, 0xb3,
    0x9c, 0x06, 0x4e, 0xab,
    0xb8, 0xaf, 0x4e, 0x9b,
    0xfb, 0x5c, 0x96, 0x24
};

static const unsigned char memory_module_event_uuid[] = {
    0xfe, 0x92, 0x74, 0x75,
    0xdd, 0x59, 0x43, 0x39,
    0xa5, 0x86, 0x79, 0xba,
    0xb1, 0x13, 0xb7, 0x74
};

static const unsigned char physical_switch_event_uuid[] = {
    0x77, 0xcf, 0x92, 0x71,
    0x9c, 0x02, 0x47, 0x0b,
    0x9f, 0xe4, 0xbc, 0x7b,
    0x75, 0xf2, 0xda, 0x97
};

static const unsigned char virtual_switch_event_uuid[] = {
    0x40, 0xd2, 0x64, 0x25,
    0x33, 0x96, 0x4c, 0x4d,
    0xa5, 0xda, 0x3d, 0x47,
    0x26, 0x3a, 0xf4, 0x25
};

static const unsigned char mld_port_event_uuid[] = {
    0x8d, 0xc4, 0x43, 0x63,
    0x0c, 0x96, 0x47, 0x10,
    0xb7, 0xbf, 0x04, 0xbb,
    0x99, 0x53, 0x4c, 0x3f
};

enum event_record_severity{
    INFORMATION_EVENT_SEVERITY = 0,
    WARNING_EVENT_SEVERITY = 1,
    FAILURE_EVENT_SEVERITY = 2,
    FATAL_EVENT_SEVERITY = 3,
};

enum interrupt_settings{
    INFORMATION_EVENT_INTR_SETTING = 0b00000000,
    WARNING_EVENT_INTR_SETTING = 0b00000000,
    FAILURE_EVENT_INTR_SETTING = 0b00000000,
    FATAL_EVENT_INTR_SETTING = 0b00000000,
};


//event log structure
typedef struct {
    char overflow_flag[4];//four severity 's overflow flag
    u32 overflow_errcount[4];//overflow count
    int front[4];//que front pointer
    int rear[4];//que rear pointer
    u64 overflow_first_time[4];
    u64 overflow_last_time[4];
    int handle_index[4];//the next handle to get(get events)
    u64 event_status_reg_cpy;//the copy of a 64 bit reg
} Event_log;

struct common_event_record {
    char event_record_uuid[16];
    char event_record_length;
    char event_record_flags[3];
    char event_record_handle[2];
    char related_event_record_handle[2];
    char event_timestamp[8];
    char rsvd[16];
    char event_record_data[80];
};

struct general_media_record_data{
    u64 physical_addr;
    char memory_event_desc;
    char memory_event_type;
    char transaction_type;
    char validity_flags[2];
    char channel;
    char rank;
    char dev[3];
    char component_id[16];
    char rsvd[46];
};



//Interfaces exposed to common_mbox_function.c

int mailbox_get_event_records(int opcode, int *payload_length, char *payload);

int mailbox_clear_event_records(int opcode, int *payload_length, char *payload);


int mailbox_set_event_intr_policy(int opcode, int *payload_length, char *payload);

int mailbox_get_event_intr_policy(int opcode, int *payload_length, char *payload);


//Generate four bytes of false policy data
int gen_false_event_intr_policy(char *false_intrl_policy);


//Precondition: If generation would result in overflow, it will not be generated
//event_record_type is related to uuid. When generating record data, it is filled based on this type.
struct common_event_record* gen_event_record(int event_record_type, char *flags, char *record_data);

//err_rep handler use the func to gen a err_rep event
struct common_event_record* gen_err_rep_event_record(u64 physical_addr, char transaction_type);

//Circular queue-related content: encapsulate the handle as a char array of size 2,
// where the content of the handle represents the next index for inserting data into the queue.
int get_next_handle(char *handle, int severity);

//Prepare record data for generating events during error reporting interrupts.
void get_err_rep_record_data(char *record_data,u64 physical_addr, char transaction_type);

//que empty
int isEmpty(Event_log* event_log, int severity);

//que full
int isFull(Event_log* event_log, int severity);

//enque
void enqueue(Event_log* event_log, int severity, struct common_event_record* record);

//deque:dequeue one item at a time
struct common_event_record* dequeue(Event_log* event_log, int severity);

// retrieve the current number of elements in the queue
int queueSize(Event_log* event_log, int severity);

// calculate the number of elements from the specified index to the end of the queue
int elementsAfterIndex(Event_log* event_log, int severity, int startIndex);

//copy 128 bytes of content to the corresponding area directly through the pointer's address and return the status code
int write_event_record(struct common_event_record* record);

//clean up records in a severity area based on handles transmitted by the host
int delete_event_record(int severity, int clear_flag, int handle_num, char handles[]);
int samt_unset_dev_status(int severity);
int samt_set_dev_status(int severity);

#endif

