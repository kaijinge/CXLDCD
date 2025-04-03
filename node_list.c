#include "include/node_list.h"
#include "include/ipc_driver.h"
#include <assert.h>
#include <stdio.h>
#include "include/config_pl.h"
#include "include/resource_manager_config.h"
#include "include/resource_manager_io.h"
#include "include/mbox_function.h"

pbnode pbpools[MAX_PHYSICAL_BLOCK_NUM];
pblistheader gfreelist;
pblistheader ldlist_headers[MAX_LOGICAL_ID_NUM];
u64 physical_dram_block_granularity = (SZ_256MB << 28);

// insert one node into list 
//head insert
// int insert(pblistheader *list, pbnode *node)
// {
//     pbnode *next = NULL;

//     if (list) {
//         next = list->next;
//         node->next = next;
//         list->next = node;
//         list->count++;
//     }

//     return 0;
// }

//tail insert
int insert(pblistheader *list, pbnode *node)
{
    if (list) {
        if(list->tail){
            list->tail->next = node;
        }
        list->tail = node;        
        list->count++;
        if(list->head == NULL){
            list->head = node;
        }
    }

    return 0;
}

// init the free list
int init_freelist_ldlist(int granularity)
{
    int i = 0;
    u64 offset = 0;

    // initialize the free list
    gfreelist.count = 0;
    gfreelist.head = NULL;
    gfreelist.tail = NULL;

    // initial the LD list
    for (i = 0; i < MAX_LOGICAL_ID_NUM; i++) {
        ldlist_headers[i].head = NULL;
        ldlist_headers[i].tail = NULL;
        ldlist_headers[i].count = 0;
        ldlist_headers[i].meta_start_addr = HW_META_INVALID_ADDR;        
    }

    for (i = 0; i < MAX_PHYSICAL_BLOCK_NUM / granularity; i++) {
        pbpools[i].is_used = false;
        pbpools[i].length = physical_dram_block_granularity;  // indicates with byte
        pbpools[i].next = NULL;
        pbpools[i].start_addr = offset + i * physical_dram_block_granularity;
        insert(&gfreelist, &pbpools[i]);
    }
    return 0;
}

int init_bram(){
    int ret = 0;
    int i = 0;
    for ( i = 0; i < MAX_PHYSICAL_BLOCK_NUM; i++)
    {
        write_bram64(HW_BRAM_START_ADDR,pbpools[i].start_addr);
        write_bram64(HW_BRAM_STANDBY_ADDR,pbpools[i].start_addr);
    }
    return ret;
}

// get the node from list
pbnode *remove_list(pblistheader *list)
{
    pbnode *tmp = NULL;

    if (list) {
        tmp = list->head;
        if (tmp) {
            list->head = tmp->next;
            list->count--;
        } else {
            list->head = NULL;
            list->tail = NULL;
        }
    }
    return tmp;
}

// get count pbnode from freelist, adding them into ldlist
int add_pb_into_ldlist(pblistheader *ldlist, pblistheader *freelist, int count)
{
    int i = 0;
    int ret = 0;
    pbnode *tmppb = NULL;

    for (i = 0; i < count; i++) {
        tmppb = remove_list(freelist);
        if (tmppb) {
            insert(ldlist, tmppb);
            tmppb->is_used = true;
            // xil_printf("!!!!ldlist->count=%d,write_bram = %llx\r\n",ldlist->count-1,tmppb->start_addr);
            if(ldlist->meta_start_addr!=-1)
            write_bram64(ldlist->meta_start_addr + (ldlist->count - 1) * 8, tmppb->start_addr);
            // read_bram64(ldlist->meta_start_addr + (ldlist->count - 1) * 8,&tmp);
        } else {
            ret = -1;
            break;
        }
    }

    return ret;
}
#define SAMT_CDMA_MAX_TRANS_LENGTH 0x1000000
int cdma_trans_exe(u64 start_addr, u64 dest_addr, int length)
{
	int value = 0;
    Xil_Out64(0x44a60000, 0x1010);
    Xil_Out64(0x44a60018, start_addr);
    Xil_Out64(0x44a60020, dest_addr);
    Xil_Out64(0x44a60028, length);
    value = pcie_mem_readl(0x44a60004);
    value &= 0x2;
    while(!value)
    {
    	value = pcie_mem_readl(0x44a60004);
    	value &= 0x2;
    }
    return 0;
}

int samt_clean_mld_block(u64 addr)
{
	int ret = 0;
	int gran = 0;
	int length = 0;
	int times = 0;
	int i = 0;
	u64 dest_addr = addr;
	u64 start_addr = addr;

	gran = samt_get_mld_gran();
	switch (gran)
	{
	case 0:
		length = 0x10000000;
		break;
	case 1:
		length = 0x20000000;
		break;
	case 2:
		length = 0x40000000;
		break;
	default:
		break;
	}
	printf("\r\n rm block addr 0x%lx\n",addr);
	times = length / SAMT_CDMA_MAX_TRANS_LENGTH -1;
	//clean first length
	for(i=0;i<SAMT_CDMA_MAX_TRANS_LENGTH/4;i++) {
		pcie_mem_writel(start_addr + i * 4, 0);
	}
	dest_addr += SAMT_CDMA_MAX_TRANS_LENGTH;
	for(i=0;i<times;i++) {
		ret += cdma_trans_exe(start_addr,dest_addr,SAMT_CDMA_MAX_TRANS_LENGTH);
		dest_addr += SAMT_CDMA_MAX_TRANS_LENGTH;
	}

    return ret;
}

// remove count pbnode from ldlist, adding them into free list
int remove_pb_from_ldlist(pblistheader *ldlist, pblistheader *freelist, int count)
{
    int i = 0;
    int ret = 0;
    pbnode *tmppb = NULL;
    mbox_set_status_bg_start();
    for (i = 0; i < count; i++) {
        tmppb = remove_list(ldlist);
        if (tmppb) {
            insert(freelist, tmppb);
            tmppb->is_used = false;
        } else {
            ret = -1;
            break;
        }
        samt_clean_mld_block(tmppb->start_addr + 0x1400000000);  //zerod block
    }

    return ret;
}

// input:macro definition of granularity
int init_after_change_granularity(int old_granularity, int new_granularity)
{
    int ret = -1;
    int i = 0;
    unsigned char alloc_num[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    resour_mgt_exec_get_cmd(&ret, alloc_num);  // step 1:read original register

    // step 2:update physical_dram_block_granularity
    // to avoid numerical overflow,using u64 type to do bit operation
    int rate = 0;
    xil_printf("1granularity in init_after_change is 0x%llx\r\n",physical_dram_block_granularity);
    if (new_granularity > old_granularity) {
        rate = (new_granularity / old_granularity) / 2;
        while (rate != 0) {
            physical_dram_block_granularity <<= 1;
            rate >>= 1;
        }
    } else {
        rate = (old_granularity / new_granularity) / 2;
        while (rate != 0) {
            physical_dram_block_granularity >>= 1;
            rate >>= 1;
        }
    }
    xil_printf("2granularity in init_after_change is 0x%llx\r\n",physical_dram_block_granularity);
    // physical_dram_block_granularity); step 3:init physical block and
    // ldlist_header
    init_freelist_ldlist(new_granularity);
    if(ret==0){
      
    }
    else if (ret > 0) {
        // step 4:reallocate the block ram according to the register
        for (i = 0; i < MAX_LOGICAL_ID_NUM; i++) {
            // change the alloc_num[] according to the rate of change
            alloc_num[i] = (alloc_num[i] * old_granularity) / new_granularity;
        }
        resour_mgt_exec_set_cmd(ret, alloc_num);
    } else {
        // return false;
        assert(0);
    }
    return ret;
}
