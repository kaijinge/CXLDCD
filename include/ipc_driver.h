#include "xil_io.h"


typedef u64 uint64;

int pcie_mem_readl(uint64 addr);
int pcie_mem_writel(uint64 addr, int value);
int pcie_mem_readc(uint64 addr);
int pcie_mem_writec(uint64 addr, char value);

int pcie_mem_writed(uint64 addr, uint64 value);
uint64 pcie_mem_readd(uint64 addr);

//Provide an interface to write the entire data in one go, where the size of data is a multiple of 4 bytes
int pcie_mem_write_data();
