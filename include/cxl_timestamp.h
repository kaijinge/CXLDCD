#include "xil_types.h"
#include "xparameters.h"
#include "xtmrctr.h"

#define TMRCTR_DEVICE_ID XPAR_TMRCTR_0_DEVICE_ID
#define TIMER_COUNTER_0 0
#define TMRCTR_DEVICE_FREQ_US (XPAR_TMRCTR_0_CLOCK_FREQ_HZ/1000)
#define TMRCTR_DEVICE_FREQ_MS (TMRCTR_DEVICE_FREQ_US/1000)
#define TMRCTR_DEVICE_PERIOD_NS ((1000*1000*1000)/XPAR_AXI_TIMER_0_CLOCK_FREQ_HZ)

//XTmrCtr TimerCounter_0;

void cxl_getTime(u64 *Time_Global);
int mailbox_get_timestamp(int opcode, int *payload_length, char *payload);
int mailbox_set_timestamp(int opcode, int *payload_length, char *payload);
//void XTime_SetTime(XTime Xtime_Global);
//void XTime_GetTime(XTime *Xtime_Global);



