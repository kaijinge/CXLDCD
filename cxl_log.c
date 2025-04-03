#include "include/cxl_log.h"
#include "include/common_mbox_function.h"

// global variables for cel and vdl structures
struct cel g_cel = {0, 0, 0,CXL_MAX_CEL_NUM,CXL_CEL_LOW_ADDR};
struct vdl g_vdl = {0, 0, 0,CXL_MAX_VDL_NUM,CXL_VDL_LOW_ADDR};


char log_data[CXL_MAX_CEL_SIZE] = {0};
char command_effect[2] = {0};


int cxl_init_cel()
{
    int res = 0;
    //event
    cxl_gen_cel(GET_EVENT_RECORDS,get_command_effect(GET_EVENT_RECORDS_EFFECT));
    cxl_gen_cel(CLEAR_EVENT_RECORDS,get_command_effect(CLEAR_EVENT_RECORDS_EFEECT));
    cxl_gen_cel(SET_EVENT_INTR_POLICY,get_command_effect(SET_EVENT_INTR_POLICY_EFFECT));
    cxl_gen_cel(GET_EVENT_INTR_POLICY,get_command_effect(GET_EVENT_INTR_POLICY_EFFECT));

    //log
    cxl_gen_cel(GET_SUPPORTED_LOGS,get_command_effect(GET_SUPPORTED_LOGS_EFFECT));
    cxl_gen_cel(GET_LOG,get_command_effect(GET_LOG_EFFECT));

    //identify
    cxl_gen_cel(IDENTIFY_MEM_DEV,get_command_effect(IDENTIFY_MEM_DEV_EFFECT));

    //timesatmp
    cxl_gen_cel(GET_TIMESTAMP,get_command_effect(GET_TIMESTAMP_EFFECT));
    cxl_gen_cel(SET_TIMESTAMP,get_command_effect(SET_TIMESTAMP_EFFECT));
    
    return res;
}
//compose a 4-byte entry from the input and insert it into the corresponding actual storage area based on the write_index.
int cxl_gen_cel(int opcode, char *command_effect)
{
    int ret = 0;
    char cel_entry[4] = {0};
    get_cel_entry(opcode, command_effect, cel_entry);
    // Write the entry into the corresponding position at write_index.
    u64 index = get_cel_write_index(g_cel.write_index,CXL_CEL_LOW_ADDR);
    xil_printf("A new cel write index is :0x%llx and opcode is 0x%llx\r\n",index,opcode);
    memcpy((void*)index, cel_entry, CXL_CEL_ENTRY_SIZE);
    // Update the position at write_index, considering circular behavior.
    g_cel.write_index = (g_cel.write_index + 1) % CXL_MAX_CEL_NUM;
    //  Update the entry_num
    if(g_cel.entry_num != g_cel.max_entry_num){
        g_cel.entry_num += 1;
    }
    else{
    	g_cel.physic_start = (g_cel.physic_start + 1) % CXL_MAX_CEL_NUM;
    }
    return ret;
}



// Length in bytes, generate vdl logs
// todo: The usage scenario is unclear.
int cxl_gen_vdl(int opcode, char *command_effect)
{
    int ret = 0;
    char vdl_entry[4] = {0};
    get_vdl_entry(opcode, command_effect, vdl_entry);
    // Write the entry into the corresponding position at write_index.
    u64 index = get_vdl_write_index(g_vdl.write_index,CXL_VDL_LOW_ADDR);
    xil_printf("the vdl write index is :0x%llx\r\n",index);
    memcpy((void*)index, vdl_entry, CXL_VDL_ENTRY_SIZE);
    // Update the position at write_index, considering circular behavior.
    g_vdl.write_index = (g_vdl.write_index + 1) % CXL_MAX_VDL_NUM;
    //  Update the entry_num
    if(g_vdl.entry_num != g_vdl.max_entry_num){
        g_vdl.entry_num += 1;
    }
    else{
    	g_vdl.physic_start = (g_vdl.physic_start + 1) % CXL_MAX_VDL_NUM;
    }
    return ret;
}



//possible returncodes:Success 、Internal Error、Retry Required、Invalid Payload Length
int mailbox_get_supported_logs(int opcode, int *payload_length, char *payload)
{
    int i;
    //retrieve the current count of logs from global variables.
    struct supported_log_entry cel_supported = {{0}, g_cel.entry_num * CXL_CEL_ENTRY_SIZE};
    struct supported_log_entry vdl_supported = {{0}, g_vdl.entry_num * CXL_VDL_ENTRY_SIZE};
    memcpy(cel_supported.log_uuid,cel_uuid,16);
    memcpy(vdl_supported.log_uuid,vdl_uuid,16);
    if(*payload_length != 0){
        return INVALID_PAYLOAD_LENGTH;
    }
    //1.assemble Number of Supported Log Entries
    payload [0] = 2 & 0xFF;
    payload [1] = 0;
    //2.Skip reserved fields; set reserved fields to 0
    for(i=2;i<8;++i)
    {
        payload [i] = 0;
    }
    //3.assemble Supported Log Entry, two sets of 20 bytes each, totaling 40 bytes
    //cel
    memcpy(payload+8, &cel_supported, CXL_SUPPORTED_LOG_ENTRY_SIZE);
    //vdl
    memcpy(payload+8+CXL_SUPPORTED_LOG_ENTRY_SIZE, &vdl_supported, CXL_SUPPORTED_LOG_ENTRY_SIZE);

    *payload_length = 8 + (CXL_SUPPORTED_LOG_ENTRY_NUM * 20);
    
    //4.assembling completed,gen cel
    char *command_effect = get_command_effect(GET_SUPPORTED_LOGS_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}


//returncodes:Success 、Invalid Prameter、Internal Error、Retry Required、Invalid Payload Length
int mailbox_get_log(int opcode, int *payload_length, char *payload){
    char *log_data;
    if(*payload_length != 24){
        return INVALID_PAYLOAD_LENGTH;
    }
    // 1. Extract UUID, offset, and length from the payload.
    // This offset is logically on the host side.
    int offset = get_int_by_char(payload + 16);
    //detecting offset issues
    if(offset<0 || offset>= CXL_MAX_CEL_SIZE ){
        return INVALID_INPUT;
    }
    int length = get_int_by_char(payload + 20);
    if(offset + length > CXL_MAX_CEL_SIZE){
        return INVALID_INPUT;
    }
    *payload_length = 0;
    // 2. handle VDL and CEL cases, return data to the payload.
    // ensure proper handling of offset, length, and payload_length beforehand; only data retrieval is required.
    if(memcmp(payload, cel_uuid, 16)==0){ //cel
    	if(offset>=(g_cel.entry_num*4)||offset+length>(g_cel.entry_num*4)){
    		return INVALID_INPUT;
    	}
        log_data = get_log_data(CXL_CEL_LOW_ADDR, g_cel.physic_start, offset, length);
        memcpy(payload, log_data, length);
    }
    else if(memcmp(payload, vdl_uuid, 16)==0){ //vdl
    	if(offset>=(g_vdl.entry_num*4)||offset+length>(g_vdl.entry_num*4)){
    		return INVALID_INPUT;
    	}
        log_data = get_log_data(CXL_VDL_LOW_ADDR, g_vdl.physic_start, offset, length);
        memcpy(payload, log_data, length);
    }
    else{
        //uuid not match
        return INVALID_INPUT;
    }
    *payload_length = length;

    //3.assembling completed,gen cel
    char *command_effect = get_command_effect(GET_LOG_EFFECT);
    cxl_gen_vdl(opcode, command_effect);

    return SUCCESS;
}

char* get_command_effect(char effect_binary_str)
{
    command_effect[0] = effect_binary_str;
    command_effect[1] = 0;
    return command_effect;
}

void get_cel_entry(int opcode, char *command_effect,char cel_entry[])
{
	cel_entry[0] = opcode & 0xff;
	cel_entry[1] = (opcode>>8) & 0xff;
	cel_entry[2] = command_effect[0];
	cel_entry[3] = command_effect[1];
}
void get_vdl_entry(int opcode, char *command_effect,char vdl_entry[])
{
	vdl_entry[0] = opcode & 0xff;
	vdl_entry[1] = (opcode>>8) & 0xff;
	vdl_entry[2] = command_effect[0];
	vdl_entry[3] = command_effect[1];
}

u64 get_cel_write_index(int write_index,u64 base_addr)
{
    return base_addr + (write_index * CXL_CEL_ENTRY_SIZE);
}

u64 get_vdl_write_index(int write_index,u64 base_addr)
{
    return base_addr + (write_index * CXL_VDL_ENTRY_SIZE);
}

// Retrieve data of length "length" starting from the logical offset in the physical storage area.
char *get_log_data(u64 base_addr,int physic_start, int logic_offset, int length)
{
    int max_num;
    int index;
    int temp_num;
    if(base_addr == CXL_CEL_LOW_ADDR){
        max_num = g_cel.max_entry_num;
    }
    else{
        max_num = g_vdl.max_entry_num;
    }
    if(physic_start+logic_offset < max_num){
        if(physic_start+logic_offset+length<=max_num){
        memcpy(log_data, (void*)(base_addr+(physic_start+logic_offset)*CXL_CEL_ENTRY_SIZE), length);
        }
        else
        {
            //data needs to be written in two segments
            temp_num = max_num - (physic_start + logic_offset);
            memcpy(log_data, (void*)(base_addr+(physic_start+logic_offset)*CXL_CEL_ENTRY_SIZE), temp_num *4);
            memcpy(log_data + (temp_num *4), (void*)base_addr, length-(temp_num *4));
        }
    }
    else{
        index = (physic_start+logic_offset) % max_num;
        memcpy(log_data, (void*)(base_addr + index*CXL_CEL_ENTRY_SIZE), length);
    }
    return log_data;
}
