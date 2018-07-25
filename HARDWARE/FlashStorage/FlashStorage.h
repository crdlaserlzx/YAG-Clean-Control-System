#ifndef __FlashStorage_H
#define __FlashStorage_H
#include "sys.h"

#define     FLASH_START_ADDR        0x0807F800    // Flash start address  ×îºóÒ»Ò³255  2KB


void Flash_Write_data(u32 addr,u32 dat);
// void Flash_Write_N_data(u32 addr,u16 parameter_tab[5][3]);
u32 Flash_Read_data(u32 addr);
void Flash_Write_Array(u32 addr,u32 dat[5]);


#endif

