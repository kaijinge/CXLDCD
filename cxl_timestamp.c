#include "include/ipc_driver.h"
#include "include/common_mbox_function.h"
#include "include/cxl_log.h"
#include "include/cxl_timestamp.h"
#include <xil_io.h>
#include "xtmrctr.h"
//#include <xtime_l.h>

//XTime time_stamp = 0,mistiming = 0;
u64 time_stamp = 0,mistiming = 0;
int mailbox_get_timestamp(int opcode, int *payload_length, char *payload){
    int ret = 0;
    int i;
//    XTime tmp = time_stamp;
    u64 tmp = time_stamp;
    xil_printf("in get timestamp,time_stamp:0x%llx\r\n",tmp);
    // for ( i = 7; i >= 0; i--)
    // {
    //     payload[i] = tmp & 0xF;
    //     tmp >>= 8;
    // }
    memcpy(payload,&tmp,8);
    *payload_length = 8;
    //assemble the log and generate CEL logs
    char *command_effect = get_command_effect(GET_TIMESTAMP_EFFECT);
    cxl_gen_vdl(opcode, command_effect);
    return SUCCESS;
}
int mailbox_set_timestamp(int opcode, int *payload_length, char *payload){
    int ret = 0;
    int i;
    if(*payload_length != 8){
        return INVALID_PAYLOAD_LENGTH;
    }
    //assemeble timestamp
    // for ( i = 0; i < 8; i++)
    // {
    //     time_stamp <<= 8;
    //     time_stamp += payload[i];
    // }
    memcpy(&time_stamp,payload,8);
    *payload_length = 0;
//    XTime tcur;
    u64 tcur = time_stamp;
//    cxl_getTime(&tcur);
    mistiming = time_stamp - tcur;
    //assemble the log and generate CEL logs
    char *command_effect = get_command_effect(SET_TIMESTAMP_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}
