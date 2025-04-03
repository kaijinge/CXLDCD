// Change according to the device type
#define  PCIE_SHARE_MEMORY_BASE  0x10E0002000
#define  PCIE_DEVICE_STATUS_BASE 0x10E0004000
//used to slt events and cel、vdl
#define  RSVD_BRAM_BASE_ADDR (0x1000000000+ 4*1024)
//fm reg offset define 
//Change according to the device type

#define CXL_MB_CTRL_OFFSET 0x04
#define CXL_MB_CMD_OFFSET       0x08
#define CXL_MB_STATUS_OFFSET    0x10
#define CXL_MB_PAYLOAD_OFFSET   0x20

#define CXL_MB_DOORBELL_OFFSET  CXL_MB_CTRL_OFFSET
#define CXL_MB_RETURN_CODE_OFFSET CXL_MB_STATUS_OFFSET + 0x04

//The length of payload_length must be within the range of 0 to SAMT_MAX_PAYLOAD_SIZE.
//SAMT_MAX_PAYLOAD_SIZE is variable, ranging from a minimum of 256 bytes to a maximum of 1 megabyte.
//TBD:Evaluate the size.
#define CXL_MAX_PAYLOAD_SIZE 1*1024

//0代表设备端未设置时间戳
#define DEFAULT_TIMESTAMP 0


#define DOORBELL_NOT_SET -1
#define DOORBELL_IS_SET 0

////Type definition
//typedef unsigned long long u64;
typedef unsigned int u32;

//return codes
enum command_return_codes {
    SUCCESS = 0,
    BACKGROUND_COMMAND_STARTED,
    INVALID_INPUT, //== invalid parameter
    UNSUPPOTED,
    INTERNAL_ERROR,
    RETRY_REQUIRED,
    BUSY,
    MEDIA_DISABLED,
    FW_TRANSFER_IN_PROGRESS,
    FW_TRANSFER_OUT_OF_ORDER,
    FW_AUTHENTICATION_FAILED,
    INVALID_SLOT,
    ACTIVATION_FAILED_COLD_RESET_REQUIRED,
    INVALID_HANDLE,
    INVALID_PHYSICAL_ADDRESS,
    INJECT_POISON_LIMIT_REACHED,
    PERMANET_MEDIA_FAILURE,
    ABORTED,
    INVALID_SECURITY_STATE,
    INCORECCT_PASSPHRASE,
    UNSUPPORTED_MAILBOX,
    INVALID_PAYLOAD_LENGTH,

    INVALID_EXTENT_LIST,
    RESOURCES_EXHAUSTED
};


enum samt_mbox_opcode{
//    //LD
//    GET_LD_INFO = 0x5400,
//    GET_LD_ALLOCATIONS = 0x5401,
//    SET_LD_ALLOCATIONS = 0x5402,

    //events
    GET_EVENT_RECORDS = 0x0100,
    CLEAR_EVENT_RECORDS = 0x0101,
    GET_EVENT_INTR_POLICY = 0x0102,
    SET_EVENT_INTR_POLICY = 0x0103,

    //logs
    GET_SUPPORTED_LOGS = 0x0400,
    GET_LOG = 0x0401,
    
    //timestamp
    GET_TIMESTAMP = 0x0300,
    SET_TIMESTAMP = 0x0301,


    //FW
    GET_FW_INFO = 0x0200,
    TRANSFER_FW = 0x0201,
    ACTIVATE_FW = 0x0202,

    //identify
    IDENTIFY_MEM_DEV = 0x4000,

    // 4 commands for DYNAMIC CAPACITY FOR LD-FAM, for some specific Host.
    GET_DYNAMIC_CAPACITY_CONFIGURATION = 0x4800,
    GET_DYNAMIC_CAPACITY_EXTENT_LIST = 0x4801,
    ADD_DYNAMIC_CAPACITY_RESPONSE = 0x4802,
    RELEASE_DYNAMIC_CAPACITY = 0x4803,

    // 9 commands for DCD MANAGEMENT FOR LD-FAM, for global Hosts.
    GET_DCD_INFO = 0x5600,
    GET_HOST_DC_REGION_CONFIGURATION = 0x5601,
    SET_DC_REGION_CONFIGURATION = 0x5602,
    GET_DC_REGION_EXTENT_LISTS = 0x5603,
    INITIATE_DYNAMIC_CAPACITY_ADD = 0x5604,
    INITIATE_DYNAMIC_CAPACITY_RELEASE = 0x5605,
    DYNAMIC_CAPACITY_ADD_REFERENCE = 0x5606,
    DYNAMIC_CAPACITY_REMOVE_REFERENCE = 0x5607,
    DYNAMIC_CAPACITY_LIST_TAGS = 0x5608
};

//1 indicates support for the optional command
enum mbox_cmd_supported{
    //FW
    GET_FW_INFO_SUP = 1,
    TRANSFER_FW_SUP = 1,
    ACTIVATE_FW_SUP = 0,

    //timestamp
    GET_TIMESTAMP_SUP = 1,
    SET_TIMESTAMP_SUP = 1,
};


int samt_mailbox_handler(void);

int mailbox_doorbell_isset(void);


int mailbox_get_cmd(int *opcode, int *payload_length);
int mailbox_get_payload(int payload_length, char* payload);

int mailbox_execute_cmd(int opcode, int *payload_length, char *payload);


int mailbox_set_mb_status(int ret_codes);

int mailbox_send_cmd(int opcode, int payload_length);
int mailbox_send_payload(int payload_length, char* payload);

int mailbox_clear_doorbell(void);

int get_int_by_char(char *data);
