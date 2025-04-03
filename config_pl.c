
#include "include/config_pl.h"
#include "include/node_list.h"
#include "include/example.h"
#include "include/ipc_driver.h"
#include "include/mbox_function.h"
#include "include/resource_manager_io.h"
#include "include/cxl_interrupt_conf.h"
#include "include/cxl_interrupt.h"

// write the bram offset
int config_ld_pb_meta(u64 start_addr, pblistheader* ldlist) {
  int i = 0;
  pbnode* curnode = ldlist->head;
  u64 offset = start_addr;

  for (i = 0; i < ldlist->count; i++) {
    // write the node physical addr into meta
    write_bram64(offset, curnode->start_addr);

    offset += HW_PB_META_BYTE_OFFSET;
    curnode = curnode->next;
  }
}

// write the meta data for all ld
// the function should be called before calling config_ld_reg
int config_pb_meta() {
  int i = 0;
  u64 offset = HW_BRAM_START_ADDR;
  pblistheader* curld = NULL;
  for (i = 0; i < MAX_LOGICAL_ID_NUM; i++) {
    curld = &ldlist_headers[i];
    curld->meta_start_addr = offset;
    config_ld_pb_meta(offset, curld);
    offset = offset + (curld->count * HW_PB_META_BYTE_OFFSET);
  }
}

// write the ld register with lock
int config_ld_reg() {
  int i = 0;
  u64 offset = HW_REG_BRAM_START_ADDR;
  pblistheader* curld = NULL;

  for (i = 0; i < MAX_LOGICAL_ID_NUM; i++) {
    curld = &ldlist_headers[i];

    // add the lock
    // get_lock_for_reg();

    write_bram64(offset, curld->meta_start_addr);

    // release the lock
    // release_lock_for_reg();

    offset += HW_PB_META_BYTE_OFFSET;
  }

  // config the granularity, curret offset is the location of the granularity
  write_bram64(offset, physical_dram_block_granularity);
  return 0;
}

int set_rm_enable(){
  write_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, 0);
  return 0;
}

int set_rm_disable(){
  write_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, 1);
  return 0;
}

int set_fm_ldid(int ld_id){
  if(ld_id<17&&ld_id>=0)
  write_register64(HW_TEST_REG_ADDR, 16 + ld_id);
  else{
    //TBD add assert
  }
  return 0;
}
