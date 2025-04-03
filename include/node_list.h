#ifndef _NODE_LIST_H_
#define _NODE_LIST_H_

#include <stdbool.h>
#include <stdio.h>
#include <xil_types.h>
#define MAX_PHYSICAL_BLOCK_NUM 128
#define MAX_LOGICAL_ID_NUM 16


#define GRAN_256MB 0
#define GRAN_512MB 1
#define GRAN_1GB 2
#define GRAN_2GB 3

#define SZ_256MB 1ULL
#define SZ_512MB 2ULL
#define SZ_1GB 4ULL
#define SZ_2GB 8ULL
#define SZ_4GB 16ULL
#define SZ_8GB 32ULL

#define LD_REG_GRANULARITY_OFFSET 28
#define LD_REG_WIDTH_OFFSET 52UL

#define REG_SIZE 8

#define HW_META_INVALID_ADDR -1

// typedef unsigned int u32;
// typedef unsigned long long  u64;

// the granularity of the physical dram block, represent with bytes
extern u64 physical_dram_block_granularity;

// define the physical block status
typedef struct physical_dram_block {
  struct physical_dram_block* next;
  bool is_used;    // 0: idle  1:used
  u64 start_addr;  // physical address
  u64 length;
} pbnode;

extern pbnode pbpools[MAX_PHYSICAL_BLOCK_NUM];

// define the list header structure
typedef struct physical_dram_block_list_header {
  struct physical_dram_block* head;
  struct physical_dram_block* tail;
  int count;
  u64 meta_start_addr;
} pblistheader;

extern pblistheader gfreelist;
extern pblistheader ldlist_headers[MAX_LOGICAL_ID_NUM];

// insert one node into list
int insert(pblistheader* list, pbnode* node);

// initial the list
int init_freelist_ldlist(int granularity);

// initial the bram
int init_bram(void);

// get node from list
pbnode* remove_list(pblistheader* list);

// add node into ld list
int add_pb_into_ldlist(pblistheader* ldlist, pblistheader* freelist, int count);

// remove node from ldlist
int remove_pb_from_ldlist(pblistheader* ldlist,
                          pblistheader* freelist,
                          int count);

int init_after_change_granularity(int old_granularity, int new_granularity);
#endif //_NODE_LIST_H_

