#ifndef RESOURCE_MANAGER_IO_H
#define RESOURCE_MANAGER_IO_H
#include <stdbool.h>
#include <xil_io.h>
#define IO_FLAG_BRAM TRUE
#define IO_FLAG_REG  FALSE

#define IS_REG_CLEAR 1

bool read_register8(u64 Addr, u8 *Value);
bool read_register16(u64 Addr, u16 *Value);
bool read_register32(u64 Addr, u32 *Value);
bool read_register64(u64 Addr, u64 *Value);
bool write_register8(u64 Addr, u8 Value);
bool write_register16(u64 Addr, u16 Value);
bool write_register32(u64 Addr, u32 Value);
bool write_register64(u64 Addr, u64 Value);
bool read_register64_perbyte(u64 Addr, u64 *Value);
bool write_register64_perbyte(u64 Addr, u64 Value);
bool write_bram8(u64 Addr, u8 Value);
bool write_bram16(u64 Addr, u16 Value);
bool write_bram32(u64 Addr, u32 Value);
bool write_bram64(u64 Addr, u64 Value);
bool read_bram8(u64 Addr, u8 *Value);
bool read_bram16(u64 Addr, u16 *Value);
bool read_bram32(u64 Addr, u32 *Value);
bool read_bram64(u64 Addr, u64 *Value);

// bool resource_manager_write(u64 Addr, u64 *value, bool flag);


#endif
