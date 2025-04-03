#ifndef _CONFIG_PL_H_
#define _CONFIG_PL_H_

// #include <stdio.h>
#include "node_list.h"
#include "xparameters.h"

#define HW_REG_BRAM_START_ADDR XPAR_M0_LITE2RM_BASEADDR
#define HW_BRAM_START_ADDR     HW_REG_BRAM_START_ADDR+0x0200
#define HW_BRAM_STANDBY_ADDR   HW_BRAM_START_ADDR+0x800

#define HW_PB_META_BYTE_OFFSET HW_BRAM_START_ADDR

#define HW_PB_REG_GRAULARITY_ADDR XPAR_M0_LITE2RM_BASEADDR+0x0088

#define HW_RM_ACTIVATE_SIGNAL_ADDR XPAR_M0_LITE2RM_BASEADDR+0x00A0
#define HW_TEST_REG_ADDR XPAR_M0_LITE2RM_BASEADDR+0x00A8

// write bram
// void write_bram(u64 addr, u64 val);

// write the ith ld meta
int config_ld_pb_meta(u64 start_addr, pblistheader *ldlist);

// write the bram
int config_pb_meta(void);

// write the ld reg
int config_ld_reg(void);

// read pl error reg
int read_ld_reg(void);

int set_rm_enable(void);

int set_rm_disable(void);

int set_fm_ldid(int ld_id);
#endif //_CONFIG_PL_H_
