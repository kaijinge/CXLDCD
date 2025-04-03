
#include "include/ipc_driver.h"



int pcie_mem_readl(uint64 addr)
{
    // R5 mem address rd

    return Xil_In32(addr);
}

int pcie_mem_writel(uint64 addr, int value)
{
    // R5 mem address wr
    Xil_Out32(addr, value);

    return 0;
}

int pcie_mem_readc(uint64 addr)
{
    return Xil_In8(addr);
}

int pcie_mem_writec(uint64 addr, char value)
{
    Xil_Out8(addr, value);

    return 0;
}


int pcie_mem_writed(uint64 addr, uint64 value)
{
    Xil_Out64(addr, value);

    return 0;
}

uint64 pcie_mem_readd(uint64 addr)
{
    return Xil_In64(addr);
}

