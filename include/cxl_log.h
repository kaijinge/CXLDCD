#ifndef CXL_LOG_H
#define CXL_LOG_H
#include<xil_io.h>
#include "cxl_events.h"

#define CXL_OPCODE_BYTE_SIZE 2
#define CXL_CMD_EFFECT_SIZE 2
#define CXL_SUPPORTED_LOG_ENTRY_NUM 2
#define CXL_SUPPORTED_LOG_ENTRY_SIZE 20
#define CXL_CEL_ENTRY_SIZE 4
#define CXL_VDL_ENTRY_SIZE 4

#define CXL_MAX_CEL_NUM 128
#define CXL_MAX_VDL_NUM 128
#define CXL_MAX_CEL_SIZE (CXL_CEL_ENTRY_SIZE*CXL_MAX_CEL_NUM)
#define CXL_MAX_VDL_SIZE (CXL_VDL_ENTRY_SIZE*CXL_MAX_VDL_NUM)


#define CXL_CEL_LOW_ADDR    EVENT_LOG_HIGH_ADDR
#define CXL_CEL_HIGH_ADDR   CXL_CEL_LOW_ADDR + CXL_MAX_CEL_SIZE
#define CXL_VDL_LOW_ADDR    CXL_CEL_HIGH_ADDR
#define CXL_VDL_HIGH_ADDR   CXL_VDL_LOW_ADDR + CXL_MAX_VDL_SIZE


//enum command effect
// Command Effect: Bit mask containing one or more effects for the
// command opcode
// • Bit[0]: Configuration Change after Cold Reset - When set, this opcode
// makes a driver visible change to the configuration of the device or
// data contained within persistent memory regions of the device. The
// change does not take effect until a device cold reset.
// • Bit[1]: Immediate Configuration Change - When set, this opcode
// makes an immediate driver visible change to the configuration of the
// device or data contained within persistent memory regions of the
// device.
// • Bit[2]: Immediate Data Change - When set, this opcode makes an
// immediate driver visible change to the data written to the device.
// • Bit[3]: Immediate Policy Change - When set, this opcode makes an
// immediate change to the policies utilized by the device.
// • Bit[4]: Immediate Log Change - When set, this opcode makes an
// immediate change to a device log.
// • Bit[5]: Security State Change - When set, this opcode results in an
// immediate driver visible change in the security state of the device.
// Security state changes that require a reboot to take effect do not use
// this effect.
// • Bit[6]: Background Operation - When set, thi#ifndef CXL_FW_H
#define CXL_FW_Hs opcode is executed in
// the background.
// • Bit[7]: Secondary Mailbox Supported – When set, submitting this
// opcode via the secondary mailbox is supported, otherwise this opcode
// will return Unsupported Mailbox if issued on the secondary mailbox.
// • Bits[15:8]: Reserved, shall be set to zero.

//8-bit binary number, add the suffix "EFFECT" to distinguish it from the opcode macro name
enum command_effect{
    //default
    NONE_COMMAND_EFFECT = 0b00000000,
    //events
    GET_EVENT_RECORDS_EFFECT = 0b00000000,
    CLEAR_EVENT_RECORDS_EFEECT = 0b00010000,
    GET_EVENT_INTR_POLICY_EFFECT = 0b00000000,
    SET_EVENT_INTR_POLICY_EFFECT = 0b00001000,

    //log
    GET_SUPPORTED_LOGS_EFFECT = 0b00000000,
    GET_LOG_EFFECT = 0b00000000,

    //timestamp
    GET_TIMESTAMP_EFFECT = 0b00000000,
    SET_TIMESTAMP_EFFECT = 0b00001000,
    
    //fw_info
    GET_FW_INFO_EFFECT = 0b00000000,

    //identify
    IDENTIFY_MEM_DEV_EFFECT = 0b00000000,
    
};

//UUID def
static const unsigned char  cel_uuid[] = {
    0x0d, 0xa9, 0xc0, 0xb5,
    0xbf, 0x41, 0x4b, 0x78,
    0x8f, 0x79, 0x96, 0xb1,
    0x62, 0x3b, 0x3f, 0x17
};

static const unsigned char vdl_uuid[] = {
    0x5e, 0x18, 0x19, 0xd9,
    0x11, 0xa9, 0x40, 0x0c,
    0x81, 0x1f, 0xd6, 0x07,
    0x19, 0x40, 0x3d, 0x86
};

enum log_type{
    CEL = 0,
    VDL = 1,
};


//structures
struct cel {
	int physic_start;
    int write_index;
    int entry_num;
    int max_entry_num;
    u64 base_addr;
};

struct vdl {
	int physic_start;
    int write_index;
    int entry_num;
    int max_entry_num;
    u64 base_addr;
};


struct supported_log_entry {
    unsigned char log_uuid[16];
    u32 log_size;
};


int cxl_init_cel(void);

//after executing the mailbox command, the device generates a cel_entry and inserts it into the cel_log
int cxl_gen_cel(int opcode, char command_effect[]);

//todo:Reserved for the device to generate vdl logs. 
//The generation scenarios still need to be discussed.
int cxl_gen_vdl(int opcode, char *command_effect);

//get_supported_logs
int mailbox_get_supported_logs(int opcode, int *payload_length, char *payload);

//get_log
int mailbox_get_log(int opcode, int *payload_length, char *payload);

char* get_command_effect(char effect_binary_str);

void get_cel_entry(int opcode, char *command_effect, char cel_entry[]);

void get_vdl_entry(int opcode, char *command_effect, char vdl_entry[]);

u64 get_cel_write_index(int write_index,u64 base_addr);

u64 get_vdl_write_index(int write_index,u64 base_addr);

char *get_log_data(u64 base_addr,int physic_start, int logic_offset, int length);

#endif
