#include "include/common_mbox_function.h"
#include "include/cxl_identify.h"
#include "include/cxl_log.h"
#include "include/cxl_fw.h"
#include "include/cxl_events.h"

extern Event_log event_log;
// Only used for initializing the allocation of contiguous address space.
struct identify_info g_identify_info = {{0},1,0,0,0,{0},{0},{0},{0},{0},{0},{0},0,0};

int mailbox_identify_mem_dev(int opcode, int *payload_length, char *payload)
{
    if(*payload_length != 0){
        return INVALID_PAYLOAD_LENGTH;
    }
    // Retrieve identify information and fill it into the payload.
    struct identify_info info = get_identify_info();
    memcpy(payload, &info, IDENTIFY_OUT_PAYLOAD_LENGTH);

    *payload_length = IDENTIFY_OUT_PAYLOAD_LENGTH;
    char *command_effect = get_command_effect(NONE_COMMAND_EFFECT);
    cxl_gen_vdl(opcode, command_effect);
    return SUCCESS;
}


struct identify_info get_identify_info()
{
    //Retrieve actual runtime firmware information
    // char *fw_revison = get_active_fw_revison();
    // memcpy(g_identify_info.fw_revision, fw_revison, 16);

	//fw_version
	memcpy((void*)&g_identify_info.fw_revision,(void*)FW_VERSION_REG_ADDR, 4);

	//event size
	int info_event_size = INFO_EVENT_OVERFLOW_CAP;
	int warning_event_size = WARNING_EVENT_OVERFLOW_CAP;
	int failure_event_size = FAILURE_EVENT_OVERFLOW_CAP;
	int fatal_event_size = FATAL_EVENT_OVERFLOW_CAP;

	memcpy((void*)&(g_identify_info.info_event_size),(void*)&info_event_size,2);
	memcpy((void*)&(g_identify_info.warning_event_size),(void*)&warning_event_size,2);
	memcpy((void*)&(g_identify_info.failure_event_size),(void*)&failure_event_size,2);
	memcpy((void*)&(g_identify_info.fatal_event_size),(void*)&fatal_event_size,2);

    //tbd:Retrieve additional information and append it to the payload
    return g_identify_info;
}

