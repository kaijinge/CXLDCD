#include <xil_io.h>
#include "resource_manager_io.h"

#define BASS_ADDR 0X44A40000
#define NEGO_LINK_WIDTH_ADDR (BASS_ADDR + 0x0060)
#define CUR_LINK_SPEED_ADDR (BASS_ADDR + 0x0064)
#define KSET_VALID_ADDR  (BASS_ADDR + 0X0740)
#define KSET_ADDR  (BASS_ADDR + 0X0744)
#define RESET_CFG_ADDR (BASS_ADDR+0x0730)

int config_mgt_read_speed_info();
int config_mgt_read_width_info();
int config_mgt_write_info();
int config_mgt_init();