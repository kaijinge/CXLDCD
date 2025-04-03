/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

 #include <stdio.h>
 #include "xil_printf.h"
 #include "include/main.h"
 //#include "include/mbox_function.h"
 #include "include/ipc_driver.h"
 #include "include/common_mbox_function.h"
 #include "include/mbox_function.h"
 #include "include/common_tool.h"
 
 #include "sleep.h"
 #include "include/cxl_interrupt_conf.h"
 #include "include/cxl_interrupt.h"
 #include "include/cxl_events.h"
 #include "include/cxl_fw.h"
 #include "include/example.h"
 #include "xparameters.h"
 #include "xil_cache.h"
 #include "xuartlite.h"
 
 /*FW version start with 0x24010100 Notes:
 0x24(Compatibility)01(important func)01(min release granularity)00(for Internal)
 */
 
 #define SAMT_FW_VERSION 0x24010400
 
 int main()
 {    
     int value = 0x10;
     //config manager need initialize firstly to avoid microblaze crash
     global_init();
     samt_set_version(SAMT_FW_VERSION);
     printf("Samt FW version %x\r\n", SAMT_FW_VERSION);
 
     while (1) {
         samt_handler_adapter();
         value = pcie_mem_readl(PCIE_DEVICE_STATUS_BASE);
         pcie_mem_writel(PCIE_DEVICE_STATUS_BASE, value | 0x10);
         pcie_mem_writel(0x44a210e0,0x1c9f805);
         usleep(100000);
     }
 
 
     return 0;
 }
 
 int samt_set_version(int version)
 {
     pcie_mem_writel(FW_VERSION_REG_ADDR, version);
 
     return 0;
 }
 