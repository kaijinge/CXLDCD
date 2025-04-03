#include "include/common_mbox_function.h"
#include "include/cxl_log.h"
#include "include/cxl_events.h"
#include "include/cxl_fw.h"
char test_payload[1024] = {0};
int test_payload_length = 0;

void gen_test_cel(){}
void gen_test_vdl(){}
void gen_test_events(){
	u64 physical_addr = 0x1000000000;
	char transaction_type = 1;
	int i;
	for(i=0;i<10;++i)
	{
	gen_err_rep_event_record(physical_addr, transaction_type);
	}

}
void print_payload(int opcode)
{
    int i;
    xil_printf("the cmd opcode is 0x%llx and output payload length is %d\r\n",opcode,test_payload_length);
    for(i = 0;i<test_payload_length;++i){
        xil_printf("output payload[%d] is :0x%.2x\r\n",i,test_payload[i]);
    }
    return;
}
void set_input_payload(int opcode,char payload[],int input_length)
{
    test_payload_length = input_length;
    //set the input payload by specific cmd
    int index[8] ={0,1,2,3,4,5,6,7};
    int i=0;
    switch (opcode)
    {
    	case SET_TIMESTAMP:{
    		payload[0] =0x11;
    		payload[1] =0x21;
    		payload[2] =0x31;
    		payload[3] = 0x41;
    		payload[4] =0x11;
    		payload [5] = 0x12;
    		payload[6] =0x1a;
    		payload[7] =0x1b;
    		break;}
    	case GET_LOG:{
    	//change the offset and len to adpat the input payload
//    		int offset =0;
//    		int len =8;
//    		memcpy(payload,cel_uuid,16);
//    		memcpy(payload+16,&offset,4);
//    		memcpy(payload+20,&len,4);
    		int offset =0;
    		int len =12;
    		memcpy(payload,cel_uuid,16);
    		memcpy(payload+16,&offset,4);
    		memcpy(payload+20,&len,4);
    		break;
    	}
    	case GET_EVENT_RECORDS:{
    		payload[0] = 2;
    		break;
    	}
    	case CLEAR_EVENT_RECORDS:{
    		payload[0] = 2;
    		payload[1] = 0;
    		payload[2] = 8;
    		payload[3] = 0;
			payload[4] = 0;
			payload[5] = 0;
			for(;i<8;++i)
			{
				memcpy(payload+6+(i*4),&index[i],4);
			}
    		break;
    	}
    	case SET_EVENT_INTR_POLICY:{
    		payload[0] = 0;
    		payload[1] = 1;
    		payload[2] = 2;
    		payload[3] = 0;
    		break;
    	}
    	default:{
    		break;}

    }
}

int cmd_test(int opcode, int input_length)
{
    int ret = 0;
    set_input_payload(opcode,test_payload,input_length);
    ret = mailbox_execute_cmd(opcode, &test_payload_length, test_payload);
    if(ret!=0){
    	xil_printf("ret!=0 cmd opcode is 0x%llx,ret =%d\r\n",opcode,ret);
    	return ret;
    }
    print_payload(opcode);
    return ret;
}
void test_mbox_cmd()
{
    int ret = 0;

    //1.get timestamp,0 means dont need input payload
//    cmd_test(GET_TIMESTAMP, 0);

    //2.set timestamp
//    cmd_test(SET_TIMESTAMP, 8);
//    cmd_test(SET_TIMESTAMP, 8);
//    cmd_test(SET_TIMESTAMP, 8);
//    cmd_test(GET_TIMESTAMP, 0);

    //3.log
//    cmd_test(GET_SUPPORTED_LOGS, 0);
//    cmd_test(GET_SUPPORTED_LOGS, 0);
//    cmd_test(GET_SUPPORTED_LOGS, 0);
//    cmd_test(GET_LOG, 24);

    //4.get_supported_log
    // cmd_test(GET_SUPPORTED_LOGS, 0);

    //5.identify
    gen_test_events();
    cmd_test(IDENTIFY_MEM_DEV, 0);

    //6.get_fw_info
    // cmd_test(GET_FW_INFO, 0);

    //7.transfer_fw,the special test

    //8.get events
    //plz gen a event before get,the func name is gen_event_record
//    gen_test_events();
//    cmd_test(GET_EVENT_RECORDS, 1);
//    cmd_test(GET_EVENT_RECORDS, 1);

    //9.clear events
    //pre: brefore use the clear cmd clear some events,you should use the get cmd to get them
    //length:n*4+6
//    cmd_test(CLEAR_EVENT_RECORDS, 38);

    //10.set event intr policy
//    cmd_test(SET_EVENT_INTR_POLICY, 4);
//
//    //11.get event intr policy
//    cmd_test(GET_EVENT_INTR_POLICY, 0);


}



//int main()
//{
//
//    test_mbox_cmd();
//    return 0;
//}







