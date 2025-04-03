#include "include/resource_manager_config.h"
#include <assert.h>
#include <stdio.h>
#include "include/config_pl.h"
#include "include/node_list.h"
#include "include/resource_manager_io.h"
#include "xil_io.h"
#include "include/mbox_function.h"

unsigned int pingpong_para = 0;
unsigned int logic_device_number = 0;

//define granularity 
// typedef enum
// {
//     GRANULARITY_256MB = 28ULL,
//     GRANULARITY_512MB,
//     GRANULARITY_1GB,
//     GRANULARITY_2GB,
//     GRANULARITY_4GB,
//     
// } Granularity;

// set the graularity of the physical block
void resour_mgt_set_physical_block_granularity(int granularity)
{
    u64 data = 0;
    u64 data_16 = 0;
    switch (granularity)
    
    // set 1 form 0 to (n-1),invalid bit: set 0 form n to 51,granularity width set
    // data bit form 47 to 63.
    {
        case GRAN_256MB:
            granularity = 1;
            data = ((SZ_256MB << LD_REG_GRANULARITY_OFFSET) - 1) + (28ULL << LD_REG_WIDTH_OFFSET);  //
            // data_16= 0x18FFFFFF0000000;
            data_16 = ~(data & ((1ULL << LD_REG_WIDTH_OFFSET) - 1)) + (25ULL << LD_REG_WIDTH_OFFSET);
            break;
        case GRAN_512MB:
            granularity = 2;
            data = ((SZ_512MB << LD_REG_GRANULARITY_OFFSET) - 1) + (29ULL << LD_REG_WIDTH_OFFSET);  //
            data_16 = ~(data & ((1ULL << LD_REG_WIDTH_OFFSET) - 1)) + (24ULL << LD_REG_WIDTH_OFFSET);
            break;
        case GRAN_1GB:
            granularity = 4;
            data = ((SZ_1GB << LD_REG_GRANULARITY_OFFSET) - 1) + (30ULL << LD_REG_WIDTH_OFFSET);  //
            data_16 = ~(data & ((1ULL << LD_REG_WIDTH_OFFSET) - 1)) + (23ULL << LD_REG_WIDTH_OFFSET);
            break;
            /*
                    case GRAN_2GB:
                        data = ((SZ_2GB << LD_REG_GRANULARITY_OFFSET) - 1) + (31ULL << LD_REG_WIDTH_OFFSET);  //
                        break;
                    case GRAN_4GB:
                        data = ((SZ_4GB << LD_REG_GRANULARITY_OFFSET) - 1) + (32ULL << LD_REG_WIDTH_OFFSET);  //
                        break;
                    case GRAN_8GB:
                        data = ((SZ_8GB << LD_REG_GRANULARITY_OFFSET) - 1) + (33ULL << LD_REG_WIDTH_OFFSET);  //
                        break;
            */
        default:
            assert(0);
            break;
    }
    if (write_register64(HW_PB_REG_GRAULARITY_ADDR, data) && write_register64(HW_PB_REG_GRAULARITY_ADDR - 8, data_16)) {
        xil_printf("write granularity register success,reg17= 0x%llx reg16=0x%llx\r\n", data, data_16);
    } else {
        xil_printf("failed to write granularity register\r\n");
    }
    // init
    int old_granularity = (int)(physical_dram_block_granularity >> 28);
    if (old_granularity != granularity) {
    	printf("init_after_change_granularity\r\n");
        init_after_change_granularity(old_granularity, granularity);
    }

    /*u64 tmp = 0;
    if (read_register64(HW_PB_REG_GRAULARITY_ADDR, &tmp))
    {
        // if (tmp == data)
        //     xil_printf("read is equal write\r\n");
        xil_printf("tmp=0x%llx\r\n", tmp); //  TBD CONVERT
    }
    else
    {
        xil_printf("failed to read register\r\n");
    }
    // convert
    tmp >>= LD_REG_WIDTH_OFFSET;
    // xil_printf("tmp = %lld \r\n", tmp);
    physical_dram_block_granularity = (1ULL << tmp);
    xil_printf("physical_dram_block_granularity=0x%llx\r\n",
    physical_dram_block_granularity);*/
}

// get the graularity of the physical block
u64 resour_mgt_get_physical_block_granularity(void)
{
    u64 tmp = 0;

    if (read_register64(HW_PB_REG_GRAULARITY_ADDR, &tmp)) {
        // if (tmp == data)
        //     xil_printf("read is equal write\r\n");
        xil_printf("tmp=0x%llx\r\n", tmp);  //  TBD CONVERT
    } else {
        xil_printf("failed to read register\r\n");
        assert(0);
    }
    // convert

    if (tmp != 0) {
        // test
        //  u64 mask = 0xFFFF800000000000;
        //  u64 b=1ULL;
        // tmp = (u64)tmp & mask;
        // for(i = 0 ;i<47;i++){
        //     tmp >>= 1;
        //      xil_printf("in loop tmp=0x%llx\r\n", tmp);
        // }
        //
        tmp = (tmp >> LD_REG_WIDTH_OFFSET);
        xil_printf("after offset tmp=0x%llx\r\n", tmp);

        // unsigned long long res=0;

        // u64 a = 0x100000FFFFFFFF;
        // a = (a>>LD_REG_WIDTH_OFFSET);
        // xil_printf("a=0x%llx\r\n", a);
        // u64 physical_dram_block_granularity1 = (1ULL << 31);
        // xil_printf("!!!!1=0x%llx\r\n",physical_dram_block_granularity1);
        // physical_dram_block_granularity1 <<= 1;
        // xil_printf("####1=0x%llx\r\n",physical_dram_block_granularity1);
        // physical_dram_block_granularity1 <<= 1;
        // xil_printf("￥￥￥￥1=0x%llx\r\n",physical_dram_block_granularity1);
        physical_dram_block_granularity = (1ULL << tmp);
        // u64 physical_dram_block_granularity2 = (1ULL<<tmp);
        // for(i = 0 ;i<64;i++){
        //     u64 c = 1ULL;
        //     b = (1ULL<<(i));
        //      xil_printf("in loop b=0x%llx\r\n", b);
        // }
        // xil_printf("$$$$b=0x%llx\r\n",b);
        // u64 physical_dram_block_granularity2 = b;
        // xil_printf("1=0x%llx,0=x%llx,2=x%llx,a==tmp?%d\r\n",physical_dram_block_granularity1,physical_dram_block_granularity,physical_dram_block_granularity2,a==tmp);
    }
    xil_printf("physical_dram_block_granularity=0x%llx\r\n", physical_dram_block_granularity);
    // physical_dram_block_granularity);
    return physical_dram_block_granularity;
}

// execute the set configuration command from FM
// input para: TBD
// LD set  , size
int resour_mgt_exec_set_cmd(int ld_num, unsigned char *alloc_num)
{
    // u64 data = 0;
    // u64 meta_data = 0;
    int variation = 0;
    int ret = 0;
    u64 address_base = HW_BRAM_START_ADDR;
    u64 start_addr_last = 0;
    // ping-pone operation

    if (pingpong_para % 2 == 0) {
        address_base = HW_BRAM_STANDBY_ADDR;
        start_addr_last = HW_BRAM_START_ADDR;
    } else {
        address_base = HW_BRAM_START_ADDR;
        start_addr_last = HW_BRAM_STANDBY_ADDR;
    }
    //xil_printf("ld_num%d logic_device_number%d address_base=0x%llx\r\n",ld_num, logic_device_number,address_base);
    // step 1 : allocate physical block
    for (int i = 0; i < ld_num; i++) {
        if (i == 0) {
            ldlist_headers[i].meta_start_addr = address_base;
        } else {
            ldlist_headers[i].meta_start_addr = (ldlist_headers[i - 1].meta_start_addr + alloc_num[i - 1] * 8);
        }
        
        variation = alloc_num[i] - ldlist_headers[i].count;
        if (variation >= 0)  // step 2 : write physcical block address into bram
        {
            cp_config_to_standby(&ldlist_headers[i], ldlist_headers[i].count, start_addr_last);
            start_addr_last += ldlist_headers[i].count * 8;
            add_pb_into_ldlist(&ldlist_headers[i], &gfreelist, variation);
        } else if (variation < 0) {
            variation = -variation;
            cp_config_to_standby(&ldlist_headers[i], alloc_num[i], start_addr_last);
            start_addr_last += ldlist_headers[i].count * 8;
            remove_pb_from_ldlist(&ldlist_headers[i], &gfreelist, variation);
        }
        if (mbox_is_bg_start()) {
            mbox_update_bg_status(FM_LD_SET, (i+1)*100/logic_device_number);
        }
    }
    // if (ld_num < 16)
    // {
    //     ldlist_headers[ld_num].next = NULL;
    // }
    set_register_all(ld_num, alloc_num);  // step3 : change the register of the LD
    pingpong_para++;
    // xil_printf("pingpong_para=%d\r\n", pingpong_para);

    xil_printf("finish write a set of register\r\n");
    return ret;
}

// execute the get configuraiton command from FM
// input para: TBD
int resour_mgt_exec_get_cmd(int *ld_num, unsigned char *alloc_num)
{
    // get data from global data
    //  int i = 0;
    //  for (i = 0; ldlist_headers[i].next != NULL; i++)
    //  {
    //      alloc_num[i] = ldlist_headers[i].count;
    //  }
    //  *ld_num = logic_device_number;

    // get data form register
    u64 data = 0;
    int i = 0;
    int tmp = 0;
    int ret = 0;
    for (i = 0; i < logic_device_number; i++) {
        // step 1: read data form reg storaged alloc_num information
        if (read_register64(HW_REG_BRAM_START_ADDR + i * 8, &data) && data != 0) {
            // step 2: decode the data and assigns alloc_num
            tmp = (int)(data >> 32);
            alloc_num[i] = tmp;
        } else {
            break;
        }
    }
    // step 3: set ld_num while register's data equal to 0
    *ld_num = i;
    // xil_printf("In get_cmd,ld_num=%d\r\n", *ld_num);
    return ret;
}

int resour_mgt_switch_dev(int dev_id)
{
    int ret = 0;
    ret = write_register64(HW_TEST_REG_ADDR, 16 + dev_id);
    return ret;
}

int rm_config_test_case(int ld_num, unsigned char *alloc_num)
{
    ldlist_headers[0].meta_start_addr = HW_BRAM_START_ADDR;
    int i = 0;
    for (i = 0; i < 64; i++) {
        write_bram64(HW_BRAM_START_ADDR + i * 8, physical_dram_block_granularity * i);
    }

    set_register_all(ld_num, alloc_num);
    return 0;
}

int cp_config_to_standby(pblistheader *ldlist, int count, u64 start_addr_last)
{
    int ret = 0;
    u64 tmp = 0;

    for (int i = 0; i < count; i++) {
        read_bram64(start_addr_last + i * 8, &tmp);
        write_bram64(ldlist->meta_start_addr + i * 8, tmp);
    }
    return ret;
}

int set_register_all(int ld_num, unsigned char *alloc_num)
{
    int i = 0;
    int ret = 0;
    u64 data = 0;

    for (i = 0; i < ld_num; i++) {
        // 31:0,BRAM addr 63:32,ld_id num
        data = alloc_num[i];
        data = (ldlist_headers[i].meta_start_addr) % (1 << 16) - 0x0200 + (data << 32);
        write_register64(HW_REG_BRAM_START_ADDR + i * 8, data);
    }
	for (i = ld_num; i < logic_device_number; i++) {
		remove_pb_from_ldlist(&ldlist_headers[i], &gfreelist, ldlist_headers[i].count);
		write_register64(HW_REG_BRAM_START_ADDR + i * 8, 0);
		if (mbox_is_bg_start()) {
			mbox_update_bg_status(FM_LD_SET, (i+1)*100/logic_device_number);
		}
	}
    logic_device_number = ld_num;

    return ret;
}
