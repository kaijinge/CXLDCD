#include <xil_io.h>
#include "include/resource_manager_io.h"
#include "include/config_manager.h"
/***********************************************************************************
* 	Current Link Speed: indicates the negotiated Link speed of the PCI Express Link:
* • 001b: 2.5 GT/s
* • 010b: 5.0 GT/s
* • 011b: 8.0 GT/s
* • 100b: 16.0 GT/s
* • 101b: 32.0 GT/s
* Values are undefined when the Link is not up
************************************************************************************/
int config_mgt_read_speed_info(){
	u32 speed = 0;
	read_register32(CUR_LINK_SPEED_ADDR,&speed);

	xil_printf("speed = %d\r\n",speed);
	return 0;
}

/***********************************************************************************
* 	Current Link Speed: indicates the negotiated Link speed of the PCI Express Link:
* • 001b: 2.5 GT/s
* • 010b: 5.0 GT/s
* • 011b: 8.0 GT/s
* • 100b: 16.0 GT/s
* • 101b: 32.0 GT/s
* Values are undefined when the Link is not up
************************************************************************************/
int config_mgt_read_width_info(){
	u32 width = 0;
	read_register32(NEGO_LINK_WIDTH_ADDR,&width);

	xil_printf("wildth = %d\r\n",width);
	return 0;
}
/*********************************************************************************
// • Bit [4:1]: PCI Express version:
// • 3h: 3.0/3.1 compliant
// • 4h: 4.0 compliant
// • 5h: 5.0 compliant
// Link up-configuration supported (see Section 16.9.1)
// • Bit [11:8]: supported Link widths
// • Bit 8 = x2 supported
// • Bit 9 = x4 supported
// • Bit 10 = x8 supported
// • Bit 11 = x16 supported
// You can use these bits to force the PCIe device to use less lanes than are physically implemented. For example, if the Core is x8 and Bit 10 is 0b, lanes 7-4 are disabled.
// 5.0 GT/s supported
// 8.0 GT/s supported
// 16.0 GT/s supported
// 32.0 GT/S supported
// Reseved
*********************************************************************************/
int config_mgt_write_info(){
	u32 tmp = 0;
	read_register32(KSET_ADDR,&tmp);
	xil_printf("kset_default= %lx\r\n",tmp);
	//write_register32(KSET_ADDR,0x8138a);//5.0GT/s x4
    write_register32(KSET_ADDR,0x8F38a);//32.0GT/s 
	write_register32(KSET_VALID_ADDR,1);//1-valid / 0-invalid
	write_register32(RESET_CFG_ADDR,0);
	return 0;
}

int config_mgt_init()
{
    int res = 0;

    config_mgt_write_info();
    
    return res;
}