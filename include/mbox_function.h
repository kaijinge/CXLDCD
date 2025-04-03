
#define SAMT_BOARD_AU250
#define SAMT_TRANSFER_MBOX  // or SAMT_TRANSFER_IPC

// bram bar2 address
#if defined SAMT_BOARD_19EG
#    define PCIE_SHARE_MEMORY_BASE 0x80000000
#elif defined SAMT_BOARD_AU250
#    if defined SAMT_TRANSFER_MBOX
#        define PCIE_SHARE_MEMORY_BASE SAMT_MAILBOX_ADDR
#    else
#        define PCIE_SHARE_MEMORY_BASE SAMT_BRAM_ADDR
#    endif
#endif

#if defined SAMT_BOARD_AU250
#    define SAMT_BRAM_ADDR         0x1000000000
#    define SAMT_MAILBOX_ADDR      0x10E0002000
#    define SAMT_DDR0_ADDR         0x1400000000
#    define SAMT_DDR1_ADDR         0x1800000000
#    define SAMT_CONFIG_SPACE_ADDR 0x44a20000
#    define SAMT_DEBUG_SPACE_ADDR  0x44a40000
#    define HW_REG_VER_FW_ADDR     0x44A22008
#endif

// fm reg offset define
#define CXL_MB_DOORBELL_OFFSET  0x04
#define CXL_MB_CMD_OFFSET       0x08
#define CXL_MB_STATUS_OFFSET_L  0x10
#define CXL_MB_STATUS_OFFSET_H  0x14
#define CXL_MB_BG_STATUS_OFFSET_L 0x18
#define CXL_MB_BG_STATUS_OFFSET_H 0x1C
#define CXL_MB_PAYLOAD_OFFSET   0x20

#define MB_DOORBELL_CLEAR_MASK         0x01
#define MB_DOORBELL_SET_VALUE          0x03
#define CXL_FM_PAYLOAD_GRANULARITY_256 0
#define CXL_FM_PAYLOAD_GRANULARITY_512 1
#define CXL_FM_PAYLOAD_GRANULARITY_1G  2
#define CXL_FM_MAX_LDS_NUM             16

enum fm_return_code
{
    FM_LD_SUCCESS = 0,
    FM_LD_INVALID,
    FM_LD_UNSUPPORT,
    FM_LD_INTERNAL_ERR,
    FM_LD_RETRY_REQ,
};

#define SAMT_MAX_LD_NUM      16
#define SAMT_MAX_PAYLOAD_NUM 32

enum mbox_opcode
{
    FM_LD_GET = 0x5401,
    FM_LD_SET = 0x5402,
    FM_LD_RM_DISABLE = 0xC401,
    FM_LD_SET_GRAN = 0xC402,
    FM_LD_SET_ID = 0xC403,
};

int ld_get_alloc_num(int *ld_num, unsigned char *alloc_num);
int ld_set_alloc_num(int ld_num, unsigned char *alloc_num);

 int fm_mailbox_doorbell_isset(void);
 int fm_mailbox_cmd_conversion(int fm_opcode, int *ld_num, unsigned char *alloc_num);
 int fm_clear_mailbox_doorbell(void);
 int fm_set_mailbox_status(int ret_codes);
 int fm_send_mailbox_cmd(int opcode, int ld_num);
 int fm_send_mailbox_payload(int ld_num, unsigned char *alloc_num);
 int fm_get_mailbox_cmd(int *opcode, int *ld_num);
 int fm_get_mailbox_payload(int ld_num, unsigned char *alloc_num);
int samt_set_fm_enable(int flag);
int samt_set_fm_granu(int gran);
int samt_set_fm_ldid(int ldid);

int samt_fm_mailbox_handler(void);
int samt_get_mld_gran(void);
int mbox_update_bg_status(int opcode, int percent);
int mbox_set_status_bg_end(void);
int mbox_set_status_bg_start(void);
int mbox_is_bg_start(void);
