#ifndef INTERRUPT_H

#define INTERRUPT_H

#include "xintc.h"

// globa interrupt controller single instance
//XIntc GINTC;
//
//// GINTC init flag
//bool GINTC_INITED = false;

////Reserve an interface for the device to trigger interrupts on the host.
//int send_intr_to_host(int type, int intr_num);

// get a single intrc
XIntc get_Intc_Instance(u16 DeviceId);

//set the index bit to 1,the reg addr is base_Addr + offset
void cxl_reg_bit_set(u64 reg_addr, int index);

// trigger the Mb Interrupt then ps will recieve a interrupt
void cxl_raise_mb_int();

// init the interrupt by Int_Id and Handler
int cxl_int_init(u16 device_id, u8 int_id, int handler_code , u8 intc_mode);

// gen a soft ware_int,and the Int_Id is 0~15
int cxl_gen_software_int(u16 device_id, u8 int_id);

// error_report handler
void err_report_intr_handler(void *CallbackRef);

// test_software_intr_handler
void test_software_intr_handler(void *CallbackRef);

// mb_intr_handler
void mb_intr_handler(void *CallbackRef);

// msic_handler
void msic_intr_handler(void *CallbackRef);

//clear the mailbox int
void clr_mb_int();


void write_mb_doorbell_reg();

void write_mb_int_clr_reg();

//clear the msic int
void clr_msic_int();

//clear the error_report_in
void clr_error_report_int();

void send_intr_to_host();

void get_err_rep_msg(struct err_rep_msg *msg);


#endif
