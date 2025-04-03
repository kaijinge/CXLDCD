#include "include/cxl_events.h"
#include "include/common_tool.h"
#include "include/common_mbox_function.h"
#include "include/mbox_function.h"
#include "include/cxl_interrupt_conf.h"
#include "include/cxl_interrupt.h"
#include "include/cxl_events.h"
#include "include/cxl_fw.h"
#include "include/cxl_log.h"
#include "include/node_list.h"
#include "xil_cache.h"
#include "include/ipc_driver.h"
#include "include/resource_manager_config.h"
#include "include/config_manager.h"
//global init
#define GEN_EVENTS_TESTNUM 100

void gen_events(int num){
	u64 physical_addr = 0x1000000000;
	char transaction_type = 1;
	int i;
	for(i=0;i<num;++i)
	{
	gen_err_rep_event_record(physical_addr, transaction_type);
	}

}

int global_init()
{
	xil_printf("now global init all config\r\n");
    int res = 0;

    // config manager init
    xil_printf("init config manager\r\n");
    config_mgt_init();

    //cache config
    xil_printf("init cache\r\n");
    Xil_DCacheDisable();
    Xil_ICacheDisable();

    //test the reg
    xil_printf("test reg write\r\n");
    pcie_mem_writel(PCIE_DEVICE_STATUS_BASE,0x10);

    //init cel
    xil_printf("init cel\r\n");
    cxl_init_cel();


    //init intr
    xil_printf("now init intr\r\n");
    cxl_int_init(INTC_DEVICE_ID, MSIC_INTR_ID, MSIC_INTR_HANDLER, XIN_REAL_MODE);
    cxl_int_init(INTC_DEVICE_ID, ERR_REPORT_INTR_ID, ERR_REPORT_HANDLER, XIN_REAL_MODE);

    //init config
    init_freelist_ldlist(SZ_256MB);
    resour_mgt_set_physical_block_granularity(GRAN_256MB);


    //init bram
    init_bram();

    //gen some events for test
//    gen_events(EVENT_OVERFLOW_CAP+4);
    // xil_printf("now gen some test events,just use err handler to gen if you wanna more\r\n");
    // gen_events(100);

    xil_printf("global init finished\r\n");
    return res;
}

int samt_handler_adapter()
{
	int ret = 0;
	int opcode = 0;
	int length = 0;
	ret = mailbox_doorbell_isset();
	if(ret==DOORBELL_NOT_SET){
		return -1;
	}
	ret = mailbox_get_cmd(&opcode,&length);

	if(ret==-1){
		xil_printf("get opcode failed\r\n");
	}

	if(opcode == FM_LD_GET||opcode == FM_LD_SET||opcode == FM_LD_RM_DISABLE||opcode == FM_LD_SET_GRAN||opcode == FM_LD_SET_ID)
	{
		samt_fm_mailbox_handler();
	}
	else
	{
		samt_mailbox_handler();
	}
	return 0;
}







//init u32 array by num
int init_u32_array(u32 array[], int length, u32 num)
{
    int i;
    for(i=0;i<length;++i)
    {
        array[i] = num;
    }
    return 0;
}


//init u64 array by num
int init_u64_array(u64 array[], int length, u64 num)
{
    int i;
    for(i=0;i<length;++i)
    {
        array[i] = num;
    }
    return 0;
}
