#ifndef _RESOURCE_MANAGER_CONFIG_H_
#define _RESOURCE_MANAGER_CONFIG_H_
#include <stdio.h>
#include "config_pl.h"
#include "node_list.h"

// the list head of the physical resource block
// struct physical_dram_block *dram_block_list = NULL;

// set the graularity of the physical block
void resour_mgt_set_physical_block_granularity(int index);  // TBD:

// get the graularity of the physical block
u64 resour_mgt_get_physical_block_granularity(void);

// execute the set configuration command from FM
// input para: TBD
int resour_mgt_exec_set_cmd(int ld_num, unsigned char *alloc_num);

// execute the get configuraiton command from FM
// input para: TBD
int resour_mgt_exec_get_cmd(int *ld_num, unsigned char *alloc_num);

// dev_id range 0-15
int resour_mgt_switch_dev(int dev_id);

int rm_config_test_case(int ld_num, unsigned char *alloc_num);

int cp_config_to_standby(pblistheader *ldlist, int count, u64 start_addr_last);
int set_register_all(int ld_num, unsigned char *alloc_num);
#endif
