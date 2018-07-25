#ifndef __LCD_H 
#define __LCD_H 			   
#include "sys.h"
#include "ra8806reg.h"
//////////////////////////////////////////////////////////////////////////////////	 
//���˼���������ϵͳ
//�޸�����:2013/12/24
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ϼ���2012-2022
//All rights reserved
//********************************************************************************  	 

////////////////////////////////////////////////////////////////////////////////// 
//LCD��������ض˿ڵĶ���
//PB8-PB15��ӦDB0-DB7
//PA0��ӦRST��PA1��ӦINT��PA2��ӦBUSY��PA3��ӦCS2��
//PB0��ӦCS1��PB1��ӦEN��PB3��ӦR/W��PB3��ӦRS��
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
