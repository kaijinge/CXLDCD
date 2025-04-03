#ifndef CXL_FW_H
#define CXL_FW_H
//the output payload length of command:get firmware
#define GET_FW_LEGNTH 80

#define FW_ACTIVATION_TAG 1

#define FW_VERSION_REG_ADDR 0x44A22008

//version of fw
#define CXL_VER 0x11
#define CXL_TYPE 3
#define FUNCTION_VER 0x01
#define RELEASE_VER 0x01

// Offset: The byte offset in the FW package data. Expressed in multiples of

static const unsigned char cxl_fw_version[] = {
    0x1, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0,
    0, 0, 0, 0
};
// 128 bytes. Ignored if Action = Full FW transfer.
#define NOT_IN_PROGRESS_TAG 0
#define IN_PROGRESS_TAG 1

//128KB is the limit of firmware on au250?
#define SLOT_SIZE (1024*128)

#define SLOT_NUM 2

#define SLOT_BASE_ADDR RSVD_BRAM_BASE_ADDR

#define SLOT_HIGH_ADDR SLOT_BASE_ADDR + (SLOT_NUM * SLOT_SIZE)

//action code
enum transfer_action {
    FULL_FW_TRANSFER_ACTION = 0x00,
    INITIATE_FW_TRANSFER_ACTION = 0x01,
    CONTINUE_FW_TRANSFER_ACTION = 0x02,
    END_TRANSFER_ACTION = 0x03,
    ABORT_TRANSFER_ACTION = 0x04,
};


//The field is an ASCII string
//If a slot has no firmware or the device does not support this slot, the field should be cleared to 0
struct slot_fw_revision{
    char revision[16];
};

//fw_info struct
struct fw_info{
    char fw_slots_supported;
    char fw_slot_info;
    char fw_activation_cap;
    char rsvd[13];
    struct slot_fw_revision slot[4];
};




int mailbox_get_fw_info(int opcode, int *payload_length, char *payload);

int mailbox_transfer_fw(int opcode, int *payload_length, char *payload);

char get_fw_slot_info(void);

char *get_active_fw_revison(void);

struct fw_info get_fw_info(void);


//interface:gen a false fw_info
struct fw_info cxl_gen_fw_info(char fw_slots_supported, char fw_slot_info, char fw_activation_cap, struct slot_fw_revision slot[]);

#endif
