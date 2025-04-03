//
#include <xil_exception.h>
#include <xil_io.h>
#include <xil_printf.h>
#include <stdio.h>
#include <xintc.h>
#include "xstatus.h"

#include "xparameters.h"
#include "include/cxl_interrupt_conf.h"
#include "include/cxl_interrupt.h"
#include "include/cxl_events.h"
#include <stdbool.h>
#include "include/mbox_function.h"

struct err_rep_msg g_msg ={0,0,0,0,0,0};
// interrupt controller single instance
XIntc GINTC;

// flag coresbonding with GINTC intialized or not
bool GINTC_INITED = false;

bool test_flag=true;
// read reg named ERR_REPORT_REG_BASE_ADD in conf.h,and print some error msg
void __attribute__((interrupt_handler))err_report_intr_handler(void *CallbackRef)
{
    xil_printf("err_report_intr_Handler now is being used\r\n");
    //1.write a err count
    u32 err_count;
    err_count = Xil_In32(REG_BASE_ADDR+ERROR_COUNT_REG_OFFSET);
    Xil_Out32(REG_BASE_ADDR+ERROR_COUNT_REG_OFFSET,err_count+1);

	//2.gen general media event
    get_err_rep_msg(&g_msg);
    char ld_id;
	char transaction_type = 0;
	if(g_msg.rd_intr==1){
		transaction_type = 1;
		ld_id = g_msg.rd_ld_id;
	}
	else{
		transaction_type = 2;
		ld_id = g_msg.wr_ld_id;
	}
	xil_printf("err_report_intr_handler and the ld_id is %d.\r\n",ld_id);
	gen_err_rep_event_record(g_msg.addr, transaction_type);

    //3.write clear intr reg
    clr_error_report_int();
}

void test_software_intr_handler(void *CallbackRef)
{
	xil_printf("test_software_intr_handler now is being used\r\n");
}

void msic_intr_handler(void *CallbackRef)
{
	xil_printf("msic_intr_handler now is being used\r\n");
	//TBD:clear thr intr
	clr_msic_int();
}
void mb_intr_handler(void *CallbackRef)
{
	xil_printf("mb_intr_handler now is being used\r\n");

	//clear the intr signal on PL
//	clr_mb_int();
}




//init a interrupt by Int_Id and Handler,the Device Id is default 0
//intc mode means
int cxl_int_init(u16 device_id, u8 int_id, int handler_code , u8 intc_mode)
{
	int Status;

	//  1.initialize the XScuGic
	XIntc intc=get_Intc_Instance(device_id);

	//  2.XScuGic selftest !!!!!!!!!:changed cauz selftest maybe faile by started intc
//	Status = XIntc_SelfTest(&intc);
//	if (Status != XST_SUCCESS) {
//		xil_printf("failed to intc selftest\r\n");
//		return XST_FAILURE;
//	}

	//  3.connect the int_id and corresbonding handler_function then pass the argument by func pointer
	switch(handler_code) {
	case ERR_REPORT_HANDLER:
		Status = XIntc_Connect(&intc, int_id, err_report_intr_handler, (void *)0);
		break;
	case MB_INTR_HANDLER:
		Status = XIntc_Connect(&intc, int_id, mb_intr_handler, (void *)0);
		break;
	case MSIC_INTR_HANDLER:
		Status = XIntc_Connect(&intc, int_id, msic_intr_handler, (void *)0);;
		break;
	case TEST_SOFT_WARE_HANDLER:
		Status = XIntc_Connect(&intc, int_id, test_software_intr_handler, (void *)0);
		break;
	default:
		break;
	}

	if (Status != XST_SUCCESS) {
		xil_printf("failed to connect\r\n");
		return XST_FAILURE;
	}

	Status = XIntc_Start(&intc, intc_mode);
	if (Status != XST_SUCCESS) {
		xil_printf("failed to start\r\n");
		return XST_FAILURE;
	}


	//  4. Enable interrupt by Interrupt id
	XIntc_Enable(&intc, int_id);


	//  5.regist the Exception handler and init the exception table
	Xil_ExceptionInit();

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler) XIntc_InterruptHandler, &intc);

	Xil_ExceptionEnable();

    return XST_SUCCESS;
}


//raise the mailbox interrupt
void cxl_raise_mb_int()
{
	write_mb_doorbell_reg();
}

//clear the mb interrupt
void clr_mb_int()
{
	write_mb_int_clr_reg();
}


void clr_msic_int()
{
//	cxl_reg_bit_set(MSIC_BASE_ADDR+MSIC_CLR_REG_OFFSET,MSIC_CLR_REG_BIT_INDEX);
	Xil_Out32(MSIC_BASE_ADDR+MSIC_CLR_REG_OFFSET,0x1);
}

void clr_error_report_int()
{
//	cxl_reg_bit_set(REG_BASE_ADDR+ERROR_REPORT_CLR_INTR_REG_OFFSET,ERR_REPORT_CLR_REG_BIT_INDEX);
	Xil_Out32(REG_BASE_ADDR+ERROR_REPORT_CLR_INTR_REG_OFFSET,0x1);
	XIntc_Acknowledge(&GINTC,ERR_REPORT_INTR_ID);
}

// get a single INTC,if not initialized just init it

XIntc get_Intc_Instance(u16 DeviceId)
{
	if(!GINTC_INITED) {
		int Status;
		//  initialize the XScuGic
		Status = XIntc_Initialize(&GINTC, DeviceId);
		if (Status != XST_SUCCESS) {
			xil_printf("failed to get XIntc instance\r\n");
		}
	}
	GINTC_INITED = true;
	return GINTC;
}

// gen a software int by the Int_Id and Cpu_Identifier,the Cpu_Identifier is default XSCUGIC_SPI_CPU0_MASK
int cxl_gen_software_int(u16 device_id, u8 int_id)
{
	int status;

	if(int_id > 15 || int_id < 0){
		xil_printf("Int_Id is fault\r\n");
		return XST_FAILURE;
	}

	XIntc intc=get_Intc_Instance(device_id);

	status = XIntc_SimulateIntr(&intc, int_id);

	if (status != XST_SUCCESS) {
		xil_printf("software_gen_in failed\r\n");
		return XST_FAILURE;
	}

	xil_printf("software_gen_in success and int_Id is %d\r\n",int_id);
	return status;
}

//index bit set from 0 to 1
void cxl_reg_bit_set(u64 reg_addr, int index)
{
	u32 res;
	u32 data;
	u32 mask = 1 << index;
	if(index > 31 ||index < 0){
		xil_printf("index is out of bound\r\n");xil_printf("test_software_intr_handler now is being used\r\n");
		return;
	}

	data = Xil_In32(reg_addr);

	res = data|mask;

	Xil_Out32(reg_addr,res);
}



//triiger mb interrupt
void write_mb_doorbell_reg()
{
	cxl_reg_bit_set(MB_BASE_ADDR+MB_DOORBELL_REG_OFFSET, MB_INT_DOORBELL_REG_BIT_INDEX);
}

//clear mb interrupt signal
void write_mb_int_clr_reg()
{
	cxl_reg_bit_set(MB_BASE_ADDR+MB_INT_CLR_REG_OFFSET, MB_INT_CLR_BIT_INDEX);
}

void send_intr_to_host()
{
	Xil_Out32(SAMT_DEBUG_SPACE_ADDR + 0x608, 0x1);
	xil_printf("now is sending a intr to host\r\n");
	Xil_Out32(SAMT_DEBUG_SPACE_ADDR + 0x608, 0x0);
}

void get_err_rep_msg(struct err_rep_msg *msg)
{
	u64 err_rep_msg = 0;
	memcpy((void*)(&err_rep_msg),(void*)(REG_BASE_ADDR + ERROR_ADDR_MSG_REG_OFFSET),8);
	msg->rsvd = (err_rep_msg>>62) & 0x03;
	msg->wr_intr = (err_rep_msg>>61) & 0x01;
	msg->rd_intr = (err_rep_msg>>60) & 0x01;
	msg->wr_ld_id = (err_rep_msg>>56) & 0x0f;
	msg->rd_ld_id = (err_rep_msg>>52) & 0x0f;
	msg->addr = err_rep_msg & 0xfffffffffffff;
}



//int main()
//{
//	xil_printf("now microblaze_intrc is waiting interrupt\r\n");
//
//	//mode is exclusively only can be inited once
//	cxl_int_init(INTC_DEVICE_ID, ERR_REPORT_INTR_ID, ERR_REPORT_HANDLER, XIN_REAL_MODE);
//	cxl_int_init(INTC_DEVICE_ID, SIMU_SOFTWARE_INTR_ID, TEST_SOFT_WARE_HANDLER, SIMU_MODE);
//	cxl_gen_software_int(INTC_DEVICE_ID,SIMU_SOFTWARE_INTR_ID);
//	while(1);
//	xil_printf("all interrupt have been processed\r\n");
//    return 0;
//}
