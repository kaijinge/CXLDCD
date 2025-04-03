#include "include/config_pl.h"
#include "include/node_list.h"
#include "include/example.h"
#include "include/ipc_driver.h"
#include "include/mbox_function.h"
#include "include/resource_manager_io.h"
#include "include/cxl_interrupt_conf.h"
#include "include/cxl_interrupt.h"
#include "sleep.h"

// #define SAMT_DEBUG_MODE

#ifdef SAMT_DEBUG_MODE
int g_ld_num = 0;
unsigned char g_alloc_num[20] = {0};
#endif
int g_granulariy = CXL_FM_PAYLOAD_GRANULARITY_256;

int samt_get_mld_gran(void)
{
	int gran = 0;

	gran = g_granulariy;
	return gran;
}

int samt_fm_mailbox_handler(void)
{
    int opcode = 0;
    int ld_num = 0;
    unsigned char alloc_num[SAMT_MAX_LD_NUM] = {0};
    int ret = 0;

//    ret = fm_mailbox_doorbell_isset();
    ret += fm_get_mailbox_cmd(&opcode, &ld_num);
    ret += fm_get_mailbox_payload(ld_num, alloc_num);
    ret += fm_mailbox_cmd_conversion(opcode, &ld_num, alloc_num);
    ret += fm_set_mailbox_status(ret);
    ret += fm_send_mailbox_cmd(opcode, ld_num);         // return cmd  length
    ret += fm_send_mailbox_payload(ld_num, alloc_num);  // retrun LD allocation list
    ret += fm_clear_mailbox_doorbell();
    // ret += fm_clear_mailbox_intr_clr();

    return ret;
}

int mbox_is_bg_start(void)
{
    int ret = 0;

    ret = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_L);

    return ret;
}

int mbox_set_status_bg_start(void)
{
    int bg_start = 0x01;

    pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_L, bg_start);
    pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_H, bg_start);

    return 0;
}

int mbox_set_status_bg_end(void)
{
    int bg_end = 0x0;

    pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_L, bg_end);
    pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_H, bg_end);

    return 0;
}

int mbox_update_bg_status(int opcode, int percent)
{
    int bg_status = opcode;
    int retcode = 0;

    printf("update percent %d\n",percent);
    bg_status |= percent << 16;
    pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_BG_STATUS_OFFSET_L, bg_status);
    if (percent == 100) {
    	pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_BG_STATUS_OFFSET_H, retcode);
    	mbox_set_status_bg_end();
    }

    return 0;
}

/***************************************************************
 * @brief according to cmd and payload，set bram or regs
 * *************************************************************/
 int fm_mailbox_cmd_conversion(int fm_opcode, int *ld_num, unsigned char *alloc_num)
{
    int ret = 0;

    printf("FM resource manager setting\n");
    if (fm_opcode == FM_LD_GET) {
        ret = ld_get_alloc_num(ld_num, alloc_num);
    } else if (fm_opcode == FM_LD_SET) {
        ret = ld_set_alloc_num(*ld_num, alloc_num);
    } else if (fm_opcode == FM_LD_RM_DISABLE) {
        ret = samt_set_fm_enable(*ld_num);
    } else if (fm_opcode == FM_LD_SET_ID) {
        ret = samt_set_fm_ldid(*ld_num);

    } else if (fm_opcode == FM_LD_SET_GRAN) {
        resour_mgt_set_physical_block_granularity(*ld_num);
    }

    return ret;
}

int samt_set_fm_enable(int flag)
{
    if (flag == 0) {
        // set reg enable
        set_rm_enable();
    } else {
        // set reg disable
        set_rm_disable();
    }
    return 0;
}

int samt_set_fm_ldid(int ldid)
{
    // set ldid reg;
    set_fm_ldid(ldid);
    return 0;
}

/***************************************************************
 * @brief FM_LD_GET cmd interface
 * *************************************************************/
int ld_get_alloc_num(int *ld_num, unsigned char *alloc_num)
{
#ifdef SAMT_DEBUG_MODE
    int i = 0;

    *ld_num = g_ld_num;
    for (i = 0; i < *ld_num; i++) {
        alloc_num[i] = g_alloc_num[i];
        printf("\r\ld_get_alloc_num%d:%d \n", i, alloc_num[i]);
    }

#else
    resour_mgt_exec_get_cmd(ld_num, alloc_num);  // resource manager cmd interface
#endif
    return 0;
}
/***************************************************************
 * @brief FM_LD_SET cmd interface
 * *************************************************************/
int ld_set_alloc_num(int ld_num, unsigned char *alloc_num)
{
#ifdef SAMT_DEBUG_MODE
    int i = 0;

    g_ld_num = ld_num;
    for (i = 0; i < ld_num; i++) {
        g_alloc_num[i] = alloc_num[i];
        printf("\r\ld_set_alloc_numi%d:%d \n", i, alloc_num[i]);
    }
#else
    resour_mgt_exec_set_cmd(ld_num, alloc_num);
#endif
    return 0;
}

/****************************************************************
 * @brief waiting cmd recieved, no time limited
 * **************************************************************/
int fm_mailbox_doorbell_isset(void)
{
    int val = 0;

    do {
        val = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_DOORBELL_OFFSET);
        val &= 0x01;
        xil_printf("\r FM cmd recieved doorbell:%d\n", val);
        usleep(1000 * 1000);

    } while (val != 0x01);

    printf("\r FM cmd recieved doorbell:%d\n", val);

    return 0;
}

int fm_set_mailbox_status(int ret_codes)
{
    int ret = 0;
    int status = 0;

    status = ret_codes;
    ret = pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_STATUS_OFFSET_H, status);
    printf("Fm status:%d\n", status);

    return ret;
}

int fm_get_mailbox_payload(int ld_num, unsigned char *alloc_num)
{
    int i = 0;
    unsigned int *p_pay = NULL;

    if (ld_num % 4) {
        ld_num /= 4;
        ld_num++;
    } else {
        ld_num /= 4;
    }

    p_pay = (unsigned int *)alloc_num;
    printf("FM receiving cmd payload\n");
    for (i = 0; i < ld_num; i++) {
        *p_pay = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_PAYLOAD_OFFSET + i * 4 + 4);
#ifdef SAMT_DEBUG_MODE
         printf("\r\nfm_get_mailbox_payload%d:0x%x\n", i,*p_pay);
#endif
        p_pay++;
    }

    return 0;
}
int fm_send_mailbox_payload(int ld_num, unsigned char *alloc_num)
{
    int ret = 0;
    int paylen = 0;
    int i = 0;
    unsigned char payload[SAMT_MAX_PAYLOAD_NUM] = {0};
    unsigned int *p_pay = NULL;

    payload[0] = ld_num;
    payload[1] = g_granulariy;
    printf("FM sending cmd payload\n");

    for (i = 0; i < ld_num; i++) {
        payload[i + 4] = alloc_num[i];
    }

    if (ld_num % 4) {
        ld_num /= 4;
        ld_num++;
    } else {
        ld_num /= 4;
    }

    paylen = ld_num + 1;
    p_pay = (unsigned int *)payload;
    for (i = 0; i < paylen; i++) {
        ret += pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_PAYLOAD_OFFSET + i * 4, *p_pay);
#ifdef SAMT_DEBUG_MODE
         printf("FM sending payload:i:%d , p_pay:0x%x\n", i, *p_pay);
#endif
        p_pay++;
    }

    return ret;
}

int fm_get_mailbox_cmd(int *opcode, int *ld_num)
{
    int value = 0;
    int ret = 0;

    value = pcie_mem_readl(PCIE_SHARE_MEMORY_BASE + CXL_MB_CMD_OFFSET);
    *opcode = value & 0xffff;
    *ld_num = value >> 16;
    if ((*ld_num > CXL_FM_MAX_LDS_NUM) || (*ld_num < 0)) {
        ret = -1;
    }
    printf("FM received cmd opcode:0x%x, ld_num:%d\n", *opcode, *ld_num);

    return ret;
}

int fm_send_mailbox_cmd(int opcode, int ld_num)
{
    int ret = 0;
    int cmd = 0;
    int paylen = 0;

    paylen = ld_num << 16;
    cmd = opcode | paylen;

    printf("FM sending cmd opcode:0x%x, ld_num:%d\n", opcode, ld_num);
    ret = pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_CMD_OFFSET, cmd);

    return ret;
}

int fm_clear_mailbox_doorbell()
{
    int ret = 0;

    printf("FM clearing doorbell\n");
    ret = pcie_mem_writel(PCIE_SHARE_MEMORY_BASE + CXL_MB_DOORBELL_OFFSET, 0);

    return ret;
}
