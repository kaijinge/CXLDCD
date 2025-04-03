#include "include/ipc_driver.h"
#include "include/cxl_log.h"
#include "include/cxl_timestamp.h"
#include "include/cxl_identify.h"
#include "include/common_mbox_function.h"
#include "include/common_tool.h"
#include "include/cxl_dcd.h"

//todo:add some err handler
int samt_mailbox_handler(void)
{
    int opcode = 0;
//    tbd:max_length set by host
//    int payload_max_length = 0;
	int payload_length = 0;
    int ret = 0;

    //the true size set by mailbox capabilities reg description
    char payload[CXL_MAX_PAYLOAD_SIZE] = {0};

//    ret = mailbox_doorbell_isset();
//    if(ret==DOORBELL_NOT_SET)
//    {
//        return -1;
//    }
    ret += mailbox_get_cmd(&opcode, &payload_length);
    ret += mailbox_get_payload(payload_length, payload);

    //the payload and payload_length will be transfered to output type
    ret += mailbox_execute_cmd(opcode, &payload_length, payload);

    //write status to status reg
    ret += mailbox_set_mb_status(ret);

    //write the payload and payload length
    
    ret += mailbox_send_cmd(opcode, payload_length);      //return cmd  length
    ret += mailbox_send_payload(payload_length, payload);  //retrun LD allocation list
    ret += mailbox_clear_doorbell(); 

    return ret;
}

/****************************************************************
 * @brief wait the door bell set ,no time out
 * **************************************************************/
int mailbox_doorbell_isset(void)
{
    int val = 0;
    //tbd:change the addr
	val = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_DOORBELL_OFFSET);

    // CXL_MB_DOORBELL_OFFSET实际指Mailbox Control Register的偏移，其中最低位表示doorbell的状态
	val &= 0x01;
    if(val != 0x01){
//        xil_printf(" FM cmd dont recieve doorbell:%d\r\n", val);
        return DOORBELL_NOT_SET;
    }
    xil_printf("device recieved doorbell:%d\r\n", val);
    return DOORBELL_IS_SET;
}

int mailbox_get_cmd(int *opcode, int *payload_length)
{
    uint64 value = 0;
    int ret = 0;

    value = pcie_mem_readd(PCIE_SHARE_MEMORY_BASE + CXL_MB_CMD_OFFSET);
    *opcode = value & 0xffff;
    *payload_length = value >> 16;
    if ((*payload_length > CXL_MAX_PAYLOAD_SIZE) || (*payload_length < 0)) {
        ret = -1;//todo
    }
//    xil_printf("device received cmd opcode:0x%x, payload_length:%d\r\n", *opcode,*payload_length);

    return ret;
}

/****************************************************************
 * execute the cmd and set the output payload and otput payload_length
 * **************************************************************/
int mailbox_execute_cmd(int opcode, int *payload_length, char *payload)
{
    int ret = 0;
    //set a dev_output_payload copy of payload 
    xil_printf("device cmd executing and opcode is 0x%llx ,input payload length is %d\r\n",opcode, *payload_length);
    switch (opcode)
    {
    case GET_LOG:
        ret = mailbox_get_log(opcode, payload_length, payload);
        break;
    case GET_SUPPORTED_LOGS:
        ret = mailbox_get_supported_logs(opcode, payload_length, payload);
        break;
    case GET_TIMESTAMP:
        ret = mailbox_get_timestamp(opcode, payload_length, payload);
        break;
    case SET_TIMESTAMP:
        ret = mailbox_set_timestamp(opcode, payload_length, payload);
        break;
    case IDENTIFY_MEM_DEV:
    	ret = mailbox_identify_mem_dev(opcode, payload_length, payload);
        break;
    case GET_EVENT_RECORDS:
    	ret = mailbox_get_event_records(opcode, payload_length, payload);
    	break;
    case CLEAR_EVENT_RECORDS:
    	ret = mailbox_clear_event_records(opcode, payload_length, payload);
    	break;
    case SET_EVENT_INTR_POLICY:
    	ret = mailbox_set_event_intr_policy(opcode, payload_length, payload);
    	break;
    case GET_EVENT_INTR_POLICY:
    	ret = mailbox_get_event_intr_policy(opcode, payload_length, payload);
    	break;
    case GET_DYNAMIC_CAPACITY_CONFIGURATION:
        ret = mailbox_get_dynamic_capacity_configuration(opcode, payload_length, payload);
        break;
    case GET_DYNAMIC_CAPACITY_EXTENT_LIST:
        ret = mailbox_get_dynamic_capacity_extent_list(opcode, payload_length, payload);
        break;
    case ADD_DYNAMIC_CAPACITY_RESPONSE:
        ret = mailbox_add_dynamic_capacity_response(opcode, payload_length, payload);
        break;
    case RELEASE_DYNAMIC_CAPACITY:
        ret = mailbox_release_dynamic_capacity(opcode, payload_length, payload);
        break;
    case GET_DCD_INFO:
        ret = mailbox_get_dcd_info(opcode, payload_length, payload);
        break;
    case GET_HOST_DC_REGION_CONFIGURATION:
        ret = mailbox_get_host_dc_region_configuration(opcode, payload_length, payload);
        break;
    case SET_DC_REGION_CONFIGURATION:
        ret = mailbox_set_dc_region_configuration(opcode, payload_length, payload);
        break;
    case GET_DC_REGION_EXTENT_LISTS:
        ret = mailbox_get_dc_region_extent_lists(opcode, payload_length, payload);
        break;
    case INITIATE_DYNAMIC_CAPACITY_ADD:
        ret = mailbox_initiate_dynamic_capacity_add(opcode, payload_length, payload);
        break;
    case INITIATE_DYNAMIC_CAPACITY_RELEASE:
        ret = mailbox_initiate_dynamic_capacity_release(opcode, payload_length, payload);
        break;
    case DYNAMIC_CAPACITY_ADD_REFERENCE:
        ret = mailbox_dynamic_capacity_add_reference(opcode, payload_length, payload);
        break;
    case DYNAMIC_CAPACITY_REMOVE_REFERENCE:
        ret = mailbox_dynamic_capacity_remove_reference(opcode, payload_length, payload);
        break;
    case DYNAMIC_CAPACITY_LIST_TAGS:
        ret = mailbox_dynamic_capacity_list_tags(opcode, payload_length, payload);
        break;
    default:
        ret = SUCCESS;
        break;
    }


    return ret;
}

int mailbox_send_cmd(int opcode, int payload_length)
{

    int ret = 0;

    uint64 paylen_mask = 0;
    uint64 cmd_reg = 0;
    //payload_length maybe have the max bit size :21,so choose the uint 64 data type
    paylen_mask = payload_length << 16;
    cmd_reg = paylen_mask | opcode;

    xil_printf("mailbox sending cmd opcode:0x%x, payload_length:%d\r\n", opcode, payload_length);
    ret = pcie_mem_writed(PCIE_SHARE_MEMORY_BASE + CXL_MB_CMD_OFFSET, cmd_reg);

    return ret;
}

//pcie_mem_readl
int mailbox_get_payload(int payload_length, char* payload)
{
    int i = 0;
    unsigned int *payload_temp = (unsigned int*)payload;

    xil_printf("device receiving cmd input payload\r\n");

    if (payload_length % 4) {
        payload_length /= 4;
        payload_length++;
    } else {
        payload_length /= 4;
    }

    for (i = 0; i < payload_length; i++) {
        *payload_temp = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_PAYLOAD_OFFSET + i * 4);
        payload_temp++;
    }


    return 0;
}

//pcie_mem_writel
int mailbox_send_payload(int payload_length, char* payload)
{
    int ret = 0;
    int i;
    unsigned int *payload_temp = (unsigned int*)payload;

    xil_printf("device sending output payload \r\n");   

    if (payload_length % 4) {
        payload_length /= 4;
        payload_length++;
    } else {
        payload_length /= 4;
    }

    for (i = 0; i < payload_length; i++) {
        ret += pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_PAYLOAD_OFFSET + i * 4,*payload_temp);
        payload_temp++;
    }

    return ret;
}


int mailbox_set_mb_status(int ret_codes)
{
    int ret = 0;
    //从CXL_MB_RETURN_CODE_OFFSET开始32位中低16位为return code
    int status_mask = 0xffff0000; 
    int temp = 0;

    temp = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_RETURN_CODE_OFFSET);
    ret = pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_RETURN_CODE_OFFSET, ((temp & status_mask)|ret_codes));

    xil_printf("device setting mb status reg which contains mb return code\r\n");
    return ret;
}


int mailbox_clear_doorbell()
{
    int ret = 0, mb_ctrl_reg = 0;
    int doorbell_bit_mask= 0xfffffffe;//low bit set 1

    xil_printf("clearing mailbox doorbell\r\n");

    // set the doorbell statuts
    mb_ctrl_reg = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_CTRL_OFFSET);
    ret = pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_DOORBELL_OFFSET, mb_ctrl_reg & doorbell_bit_mask);

    return ret;
}

int get_int_by_char(char *data)
{
	int res = 0;
	memcpy(&res,data,4);
	return res;
}