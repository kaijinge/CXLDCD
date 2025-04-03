#include "include/example.h"
#include "include/node_list.h"
#include "include/resource_manager_config.h"
#include "include/resource_manager_io.h"
#include "include/ipc_driver.h"
#include "sleep.h"
#include "xil_printf.h"
#include <stdio.h>
#include "include/mbox_function.h"

void samt_ddr_rw_test(void)
{
    int value = 0;
    int i = 0;
    int flag = 0;

    printf("\r DDR0 read write test:\n");
    for (i = 0; i < 0x100000; i++) {
        pcie_mem_writel(SAMT_DDR0_ADDR + i * 4, 0x12345678);
        value = pcie_mem_readl(SAMT_DDR0_ADDR + i * 4);
        // printf("0x%x ", value);
        // if((i+1)%4 == 0) {
        //	printf("\n");
        // }
        if (value != 0x12345678) {
            printf("DDR0 test error\n");
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        printf("DDR0 read write success!\n");
    }

    printf("\r\n DDR1 read write test:\n");
    for (i = 0; i < 0x100000; i++) {
        pcie_mem_writel(SAMT_DDR1_ADDR + i * 4, 0x12345678);
        value = pcie_mem_readl(SAMT_DDR1_ADDR + i * 4);
        // printf("0x%x ", value);
        // if((i+1)%4 == 0) {
        //	printf("\n");
        // }
        if (value != 0x12345678) {
            printf("DDR1 test error\n");
            flag = 1;
            break;
        }
    }
    if (flag == 0) {
        printf("DDR1 read write success!\n");
    }
}

void samt_config_space_rw_test(void)
{
    int value = 0;
    int i = 0;

    printf("\r config_space read test:\n");
    printf("-----------------------------------------------\n");
    printf("\r internal reg:\n");
    for (i = 0; i < 0x400; i++) {
        // pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR, 0x12345678);
        value = pcie_mem_readl(SAMT_CONFIG_SPACE_ADDR + i * 4);

        printf("%x ", value);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("-----------------------------------------------\n");
    printf("\r pcie config space reg:\n");
    for (i = 0x400; i < 0x800; i++) {
        // pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR, 0x12345678);
        value = pcie_mem_readl(SAMT_CONFIG_SPACE_ADDR + i * 4);

        printf("%x ", value);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }

    printf("-----------------------------------------------\n");
    printf("\r bridge external reg:\n");
    for (i = 0x800; i < 0x1000; i++) {
        // pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR, 0x12345678);
        value = pcie_mem_readl(SAMT_CONFIG_SPACE_ADDR + i * 4);
        pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR + i * 4, 0x12345678);
        printf("%x ", value);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("-----------------------------------------------\n");

    printf("\r RCRB config_space:\n");
    for (i = 0x3c00; i < 0x4000; i++) {
        // pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR, 0x12345678);
        value = pcie_mem_readl(SAMT_CONFIG_SPACE_ADDR + i * 4);

        printf("%x ", value);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("-----------------------------------------------\n");
    printf("\r CXL component reg:\n");
    for (i = 0x4000; i < 0x5000; i++) {
        // pcie_mem_writel(SAMT_CONFIG_SPACE_ADDR, 0x12345678);
        value = pcie_mem_readl(SAMT_CONFIG_SPACE_ADDR + i * 4);

        printf("%x ", value);
        if ((i + 1) % 4 == 0) {
            printf("\n");
        }
    }
    printf("----------------------end-------------------------\n");
}

void samt_mailbox_rw_test(void)
{
    int value = 0;

    pcie_mem_writel(SAMT_BRAM_ADDR + CXL_MB_DOORBELL_OFFSET, 0x12345678);
    value = pcie_mem_readl(SAMT_BRAM_ADDR + CXL_MB_DOORBELL_OFFSET);
    printf("\r\nbram base:0x%lx,value:0x%x \n", SAMT_BRAM_ADDR + CXL_MB_DOORBELL_OFFSET, value);

    pcie_mem_writel(SAMT_MAILBOX_ADDR + CXL_MB_DOORBELL_OFFSET, 0x3);
    //usleep(100);
    value = pcie_mem_readl(SAMT_MAILBOX_ADDR + CXL_MB_DOORBELL_OFFSET);
    printf("\r\nmailbox base doorbell:0x%lx,value:0x%x \n", SAMT_MAILBOX_ADDR + CXL_MB_DOORBELL_OFFSET, value);

    pcie_mem_writel(SAMT_MAILBOX_ADDR + 0x08, 0x12345678);
    //usleep(100);
    value = pcie_mem_readl(SAMT_MAILBOX_ADDR + 0x08);
    printf("\r\nmailbox base cmd:0x%lx,value:0x%x \n", SAMT_MAILBOX_ADDR + 0x08, value);

    usleep(10000);
    pcie_mem_writel(SAMT_MAILBOX_ADDR + 0x20, 0x12345678);
    usleep(10000);
    value = pcie_mem_readl(SAMT_MAILBOX_ADDR + 0x20);
    printf("\r\nmailbox base payload:0x%lx,value:0x%x \n", SAMT_MAILBOX_ADDR + 0x20, value);


    printf("\r\nMbox addr base:0x%lx\n", PCIE_SHARE_MEMORY_BASE);
}


void register_io_example() {
  u64 write_value = 0x11111111ULL;
  xil_printf("the start address of register is 0x%lx\r\n",
             HW_REG_BRAM_START_ADDR);
  xil_printf("write 0x%llx in register\r\n", write_value);
  write_register64(HW_REG_BRAM_START_ADDR, write_value);
  u64 read_value;
  read_register64(HW_REG_BRAM_START_ADDR, &read_value);
  xil_printf("the reading value is 0x%llx\r\n", read_value);
  u64 tmp;
  int i = 0, ret = 0;
  for (i = 0; i < 1024; i++) {
    write_bram64(HW_BRAM_START_ADDR + i * 8, i);
    read_bram64(HW_BRAM_START_ADDR + i * 8, &tmp);
    if (i != tmp) {
      ret++;
      xil_printf("bram_%d = %llx!\r\n", i, tmp);
    }
  }
  xil_printf("ret=%d !\r\n", ret);
  for (i = 0; i < 22; i++) {
    write_register64(HW_REG_BRAM_START_ADDR + i * 8, i);
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    if (i != tmp) {
      ret++;
      xil_printf("reg_%d = %llx!\r\n", i, tmp);
    }
  }
  xil_printf("ret=%d !\r\n", ret);
}

void config_granularity_example() {
  u64 granularity, tmp;
  int i = 0;
  granularity = resour_mgt_get_physical_block_granularity();
  xil_printf("the defalut granularity = 0x%llx\r\n", granularity);
  xil_printf("set granularity 1GB:\r\n");
  resour_mgt_set_physical_block_granularity(SZ_1GB);
  granularity = resour_mgt_get_physical_block_granularity();
  xil_printf("the current granularity = 0x%llx\r\n", granularity);
  resour_mgt_set_physical_block_granularity(SZ_256MB);
  granularity = resour_mgt_get_physical_block_granularity();
  xil_printf("the current granularity = 0x%llx\r\n", granularity);
  resour_mgt_set_physical_block_granularity(SZ_512MB);
  granularity = resour_mgt_get_physical_block_granularity();
  xil_printf("the current granularity = 0x%llx\r\n", granularity);
  for (i = 0; i < 22; i++) {
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    xil_printf("@@@@reg_%d = 0x%llx\r\n", i, tmp);
  }
}

void config_cmd_from_host_example() {
  resour_mgt_set_physical_block_granularity(SZ_1GB);
  int ld_num = 16;
  unsigned char alloc_num[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};
  resour_mgt_exec_set_cmd(ld_num, alloc_num);
  int ret = -1;
  int i = 0;

  unsigned char alloc_test[] = {0, 0, 0, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 14, 0, 16};
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  xil_printf("read ld_num = %d\r\n", ret);

  // for (i = 0; i < ld_num; i++) {
  //   /* code */
  //   xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
  //              ldlist_headers[i].meta_start_addr);
  //   xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  // }
  xil_printf("Complete a set of configuration operations !\r\n");

  resour_mgt_set_physical_block_granularity(SZ_512MB);
  u64 tmp;
  tmp = resour_mgt_get_physical_block_granularity();
  xil_printf("####granularity = 0x%llx\r\n", tmp);
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  // for (i = 0; i < ld_num; i++) {
  //   /* code */
  //   xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
  //              ldlist_headers[i].meta_start_addr);
  //   xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  // }

  ld_num = 15;
  unsigned char alloc_num1[] = {16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};
  resour_mgt_exec_set_cmd(ld_num, alloc_num1);
  ret = -1;
  i = 0;
  unsigned char alloc_test1[] = {0, 0, 0, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 14, 0, 16};
  resour_mgt_exec_get_cmd(&ret, alloc_test1);
  xil_printf("read ld_num1 = %d\r\n", ret);
  // for (i = 0; i < ld_num; i++) {
  //   /* code */
  //   xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
  //              ldlist_headers[i].meta_start_addr);
  //   xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test1[i]);
  // }
  xil_printf("Complete a set of configuration operations !\r\n");
}
char test11[128];

void cxl_1_1_server_example() {
  config_cmd_from_host_example();
  // activate the module of resource manager:  0:open 1:close
  write_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, 0);

  write_register64(HW_TEST_REG_ADDR, 16);
  // print the register[0-15]
  int i = 0;
  u64 tmp = 0;
  for (i = 0; i < 16; i++) {
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    xil_printf("REG_%d = %llx!\r\n", i, tmp);
  }
  // print
  read_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, &tmp);
  xil_printf("REG_20 = %llx!\r\n", tmp);
  read_register64(HW_TEST_REG_ADDR, &tmp);
  xil_printf("REG_21 = %llx!\r\n", tmp);

  write_register64(HW_TEST_REG_ADDR, 15);
  read_register64(HW_TEST_REG_ADDR, &tmp);
  xil_printf("set 15 and REG_21 = %llx!\r\n", tmp);

  write_bram64(HW_BRAM_START_ADDR + 8, 111);
  read_bram64(HW_BRAM_START_ADDR + 8, &tmp);
  u64 addr_test = (u64)&test11;
  int a = 111;
  for (i = 0; i < 16; i++) {
    write_bram64(addr_test + i * 8, a++);
    read_bram64(addr_test + i * 8, &tmp);
  }
  // xil_printf("ram = %llx!\r\n",tmp);
}

void cxl_1_1_cfg_cmd_from_host_example() {

  // activate the module of resource manager:  0:open 1:close
  write_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, 0);

  write_register64(HW_TEST_REG_ADDR, 16);

  u64 tmp, tmp1;
  // read_register64(HW_REG_BRAM_START_ADDR+20*8,&tmp);
  // xil_printf("!!!!0@@@@reg_20 = 0x%llx\r\n", tmp);

  int i = 0;

  // for(i=0;i<1024;i++){
  //   write_bram64(HW_BRAM_START_ADDR + i*8,i);
  //   read_bram64(HW_BRAM_START_ADDR + i*8,&tmp);
  //   if(i!=tmp){
  //   ret1++;
  //   xil_printf("bram_%d = %llx!\r\n",i,tmp);
  //   }
  // }
  // xil_printf("ret=%d !\r\n",ret1);

  // read_register64(HW_REG_BRAM_START_ADDR+20*8,&tmp);
  // xil_printf("!!!!@@@@1reg_20 = 0x%llx\r\n", tmp);

  resour_mgt_set_physical_block_granularity(SZ_256MB);
  int ld_num = 16;
  unsigned char alloc_num[] = {8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
  resour_mgt_exec_set_cmd(ld_num, alloc_num);
  int ret = -1;

  // read_register64(HW_REG_BRAM_START_ADDR+20*8,&tmp);
  // xil_printf("!!!!2@@@@reg_20 = 0x%llx\r\n", tmp);

  unsigned char alloc_test[] = {0, 0, 0, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 14, 0, 16};
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  xil_printf("read ld_num = %d\r\n", ret);

  for (i = 0; i < ld_num; i++) {
    /* code */
    xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
               ldlist_headers[i].meta_start_addr);
    xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  }
  xil_printf("Complete a set of configuration operations !\r\n");
  for (i = 0; i < 128; i++) {
    read_bram64(HW_BRAM_STANDBY_ADDR + i * 8, &tmp);
    if (tmp != 0)
      xil_printf("!!!!bram_%d = %llx!\r\n", i, tmp);
  }

  sleep(60);
  resour_mgt_set_physical_block_granularity(SZ_512MB);

  tmp = resour_mgt_get_physical_block_granularity();
  xil_printf("####granularity = 0x%llx\r\n", tmp);
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  for (i = 0; i < 128; i++) {
    read_bram64(HW_BRAM_START_ADDR + i * 8, &tmp);
    if (tmp != 0)
      xil_printf("!!!!bram_%d = %llx!\r\n", i, tmp);
  }
  for (i = 0; i < ld_num; i++) {
    /* code */
    xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
               ldlist_headers[i].meta_start_addr);
    xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  }
  for (i = 0; i < 22; i++) {
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    xil_printf("@@@@reg_%d = 0x%llx\r\n", i, tmp);
  }
  read_register64(HW_REG_BRAM_START_ADDR, &tmp);
  xil_printf("!!!!read_reg_0 = 0x%llx\r\n", tmp);
  u64 ram0_addr;
  ram0_addr = (tmp % (1ULL << 32));
  ram0_addr += HW_BRAM_START_ADDR;
  xil_printf("!!!! ram0_addr= 0x%llx\r\n", ram0_addr);
  read_bram64(ram0_addr, &tmp);
  read_bram64(ldlist_headers[0].meta_start_addr, &tmp1);
  xil_printf("@@@@ddr addr from reg0 = 0x%llx,bram = 0x%llx\r\n", tmp, tmp1);
}

void cxl_1_1_test_case() {
  // register_io_example();
  write_register64(HW_TEST_REG_ADDR, 16);
  u64 tmp, tmp1;
  resour_mgt_set_physical_block_granularity(SZ_512MB);
  int ld_num = 1, ret = -1, i;
  unsigned char alloc_num[] = {64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  ret = rm_config_test_case(ld_num, alloc_num);

  read_register64(HW_REG_BRAM_START_ADDR, &tmp);
  read_bram64(HW_BRAM_START_ADDR, &tmp1);
  xil_printf("!!!!reg0=%llx,bram0=%llx\r\n", tmp, tmp1);
  for (i = 0; i < 22; i++) {
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    xil_printf("@@@@reg_%d = 0x%llx\r\n", i, tmp);
  }
  ret = 0;
  for (i = 0; i < 64; i++) {
    read_bram64(HW_BRAM_START_ADDR + i * 8, &tmp);
    if (tmp != physical_dram_block_granularity * i) {
      ret++;
      xil_printf("bram_%d = %llx!\r\n", i, tmp);
    }
  }
  xil_printf("!!!there are %d io_erro!\r\n", ret);
  read_register64(HW_REG_BRAM_START_ADDR, &tmp);
  xil_printf("!!!!read_reg_0 = 0x%llx\r\n", tmp);
  u64 ram0_addr;
  ram0_addr = (tmp % (1ULL << 32));
  ram0_addr += HW_BRAM_START_ADDR;
  xil_printf("@@@@ram0_addr= 0x%llx\r\n", ram0_addr);
  read_bram64(ram0_addr, &tmp);
  read_bram64(ldlist_headers[0].meta_start_addr, &tmp1);
  xil_printf("@@@@ddr addr from reg0 = 0x%llx,from bram = 0x%llx\r\n", tmp,
             tmp1);
}

void cxl_1_1_test_case_1228() {
  // activate the module of resource manager:  0:open 1:close
  write_register64(HW_RM_ACTIVATE_SIGNAL_ADDR, 1);

  write_register64(HW_TEST_REG_ADDR, 16 + 0);

  u64 tmp, tmp1;

  int i = 0;


  resour_mgt_set_physical_block_granularity(SZ_512MB);
  int ld_num = 16;
  unsigned char alloc_num[] = {64, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  resour_mgt_exec_set_cmd(ld_num, alloc_num);
  int ret = -1;

  unsigned char alloc_test[] = {0, 0, 0, 4, 5, 6, 7, 8, 9, 10, 0, 12, 13, 14, 0, 16};
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  xil_printf("read ld_num = %d\r\n", ret);

  for (i = 0; i < ld_num; i++) {
    /* code */
    xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
               ldlist_headers[i].meta_start_addr);
    xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  }
  xil_printf("Complete a set of configuration operations !\r\n");
  for (i = 0; i < 128; i++) {
    read_bram64(HW_BRAM_STANDBY_ADDR + i * 8, &tmp);
    if (tmp != 0)
      xil_printf("!!!!bram_%d = %llx!\r\n", i, tmp);
  }

  sleep(60);
  resour_mgt_set_physical_block_granularity(SZ_1GB);

  tmp = resour_mgt_get_physical_block_granularity();
  xil_printf("####granularity = 0x%llx\r\n", tmp);
  resour_mgt_exec_get_cmd(&ret, alloc_test);
  for (i = 0; i < 128; i++) {
    read_bram64(HW_BRAM_START_ADDR + i * 8, &tmp);
    if (tmp != 0)
      xil_printf("!!!!bram_%d = %llx!\r\n", i, tmp);
  }
  for (i = 0; i < ld_num; i++) {
    /* code */
    xil_printf("ldlist_headers[%d].meta_start_addr=%x\t", i,
               ldlist_headers[i].meta_start_addr);
    xil_printf("reading alloc_num [%d] = %d \r\n", i, alloc_test[i]);
  }
  for (i = 0; i < 22; i++) {
    read_register64(HW_REG_BRAM_START_ADDR + i * 8, &tmp);
    xil_printf("@@@@reg_%d = 0x%llx\r\n", i, tmp);
  }
  read_register64(HW_REG_BRAM_START_ADDR, &tmp);
  xil_printf("!!!!read_reg_0 = 0x%llx\r\n", tmp);
  u64 ram0_addr;
  ram0_addr = (tmp % (1ULL << 32));
  ram0_addr += HW_BRAM_START_ADDR;
  xil_printf("!!!! ram0_addr= 0x%llx\r\n", ram0_addr);
  read_bram64(ram0_addr, &tmp);
  read_bram64(ldlist_headers[0].meta_start_addr, &tmp1);
  xil_printf("@@@@ddr addr from reg0 = 0x%llx,bram = 0x%llx\r\n", tmp, tmp1);
  //resour_mgt_set_physical_block_granularity(SZ_2GB);
}
