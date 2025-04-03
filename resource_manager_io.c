#include "include/resource_manager_io.h"
#include <stdio.h>
#include <xil_io.h>
#include "include/config_pl.h"
#include "include/node_list.h"
#include "include/resource_manager_config.h"
#include <stdbool.h>

bool read_register8(u64 Addr, u8 *Value)
{
    *Value = Xil_In8(Addr);
    return true;
}
bool read_register16(u64 Addr, u16 *Value)
{
    *Value = Xil_In16(Addr);
    return true;
}
bool read_register32(u64 Addr, u32 *Value)
{
    *Value = Xil_In32(Addr);
    return true;
}
bool read_register64(u64 Addr, u64 *Value)
{
    *Value = Xil_In64(Addr);
    return true;
}
bool write_register8(u64 Addr, u8 Value)
{
    Xil_Out8(Addr, Value);
    return true;
}
bool write_register16(u64 Addr, u16 Value)
{
    Xil_Out16(Addr, Value);
    return true;
}
bool write_register32(u64 Addr, u32 Value)
{
    Xil_Out32(Addr, Value);
    return true;
}
bool write_register64(u64 Addr, u64 Value)
{
    Xil_Out64(Addr, Value);
    return true;
}
bool read_register64_perbyte(u64 Addr, u64 *Value)
{
    u64 tmp = 0;
    for (int i = 0; i < 8; i++, Addr++) {
        tmp <<= 8;
        tmp += Xil_In8(Addr);
    }
    *Value = tmp;
    return true;
}
bool write_register64_perbyte(u64 Addr, u64 Value)
{
    for (int i = 0; i < 8; i++, Addr++) {
        u8 tmp = Value % 256;
        Xil_Out8(Addr, tmp);
        Value >>= 8;
    }
    return true;
}
bool write_bram8(u64 Addr, u8 Value)
{
    Xil_Out8(Addr, Value);
    return true;
}
bool write_bram16(u64 Addr, u16 Value)
{
    Xil_Out16(Addr, Value);
    return true;
}
bool write_bram32(u64 Addr, u32 Value)
{
    Xil_Out32(Addr, Value);
    return true;
}
bool write_bram64(u64 Addr, u64 Value)
{
    Xil_Out64(Addr, Value);
    // xil_printf("!!!!ADDR=%llx,Value = %llx\r\n",Addr,Value);
    return true;
}
bool read_bram8(u64 Addr, u8 *Value)
{
    *Value = Xil_In8(Addr);
    return true;
}
bool read_bram16(u64 Addr, u16 *Value)
{
    *Value = Xil_In16(Addr);
    return true;
}
bool read_bram32(u64 Addr, u32 *Value)
{
    *Value = Xil_In32(Addr);
    return true;
}
bool read_bram64(u64 Addr, u64 *Value)
{
    *Value = Xil_In64(Addr);

    return true;
}

// bool resource_manager_write(u64 Addr, u64 Value, bool flag)
// {
//     if (flag)
//         write_bram8(Addr, Value);
//     else
//         write_register8(Addr, Value);
//     return true;
// }

