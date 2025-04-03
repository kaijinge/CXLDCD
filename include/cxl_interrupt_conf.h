#ifndef INTERRUPT_CONF_H
#define INTERRUPT_CONF_H

//msic
#define MSIC_BASE_ADDR XPAR_M2_LITE2MISC_BASEADDR
#define MSIC_CLR_REG_OFFSET 0x80
#define MSIC_CLR_REG_BIT_INDEX 0

//error report
#define ERR_REPORT_CLR_REG_BIT_INDEX 0

//resource manbager reg
#define REG_BASE_ADDR 0x44A00000
#define ERROR_COUNT_REG_OFFSET 0x00B0
#define ERROR_REPORT_CLR_INTR_REG_OFFSET 0x0098
#define ERROR_ADDR_MSG_REG_OFFSET 0x0090

//msic

#define SIMU_SOFTWARE_INTR_ID 3

#define ERR_REPORT_INTR_ID XPAR_MICROBLAZE_0_AXI_INTC_SYSTEM_IN5_INTR

#define MSIC_INTR_ID XPAR_MICROBLAZE_0_AXI_INTC_SYSTEM_IN9_0_INTR

#define MB_INTR_ID XPAR_MICROBLAZE_0_AXI_INTC_SYSTEM_IN8_0_INTR



//error report reg base address and bit wide is 64
#define ERR_REPORT_REG_BASE_ADD 0x0098

// interrupt status reg and bit wide is 64
#define INTR_STATUS_REG_BASE_ADD 0x009F

//the mail box base addr
#define MB_BASE_ADDR 0x80300000


//default INTC device id
#define INTC_DEVICE_ID	XPAR_INTC_0_DEVICE_ID

//the doorbell reg use the highest bit to raise interrupt
#define MB_INT_DOORBELL_REG_BIT_INDEX 31

//the mailbox clear interrupt reg use the lowest bit to clear interrupt
#define MB_INT_CLR_BIT_INDEX 0


//doorbell reg offset
#define MB_DOORBELL_REG_OFFSET 0x4

//mailbox interrupt clear reg offset
#define MB_INT_CLR_REG_OFFSET 0x80


/*Mode determines if software is allowed to simulate interrupts or
*		real interrupts are allowed to occur. Note that these modes are
*		mutually exclusive. The interrupt controller hardware resets in
*		a mode that allows software to simulate interrupts until this
*		mode is exited. It cannot be reentered once it has been exited.
*/

#define REAL_MODE XIN_REAL_MODE

#define SIMU_MODE XIN_SIMULATION_MODE


//fm_handler code
#define ERR_REPORT_HANDLER 0
#define MB_INTR_HANDLER 1
#define MSIC_INTR_HANDLER 2
#define TEST_SOFT_WARE_HANDLER 3




//err rep msg struct
struct err_rep_msg{
	char rsvd;
	char wr_intr;
	char rd_intr;
	char wr_ld_id;
	char rd_ld_id;
	u64 addr;
};
#endif
