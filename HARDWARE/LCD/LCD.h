#ifndef __LCD_H 
#define __LCD_H 			   
#include "sys.h"
#include "ra8806reg.h"
//////////////////////////////////////////////////////////////////////////////////	 
//光纤激光器控制系统
//修改日期:2013/12/24
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 凯瑞迪激光2012-2022
//All rights reserved
//********************************************************************************  	 

////////////////////////////////////////////////////////////////////////////////// 
//LCD屏控制相关端口的定义
//PB8-PB15对应DB0-DB7
//PA0对应RST；PA1对应INT；PA2对应BUSY；PA3对应CS2；
//PB0对应CS1；PB1对应EN；PB3对应R/W；PB3对应RS；
///////////////////////////////////////////////////////////////////////
#define lcd_rst PAout(0)// PA0
//#define lcd_int PAout(1)// PA1
//#define lcd_busy PAout(2)// PA2
#define lcd_cs2 PAout(3)// PA3
#define lcd_cs1 PBout(0)// PB0
#define lcd_rd PBout(1)// PB1
#define lcd_wr PBout(5)// PB2
#define lcd_rs PBout(6)// PB6

void LCD_GPIO_Init(void);

void lcd_soft_reset(void);

void LCD_write_reg(u8 reg);

u8 LCD_read_data(void);

void LCD_write_data(u8 data);

void LCD_reg_init(void);

void LCD_clear(void);

void LCD_sceenDisplay(u8 * dispTemp);

u8 touch_x(void);

u8 touch_y(void);

void getTouchXY(int *touchx1,int *touchy1,int *touchx2,int *touchy2);


#endif
