#ifndef __scan_H
#define __scan_H
#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//SPI���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////

#define scan_cs PCout(9)// 
#define scan_sclk PCout(8)//
#define scan_sdix PCout(10)// 
#define scan_sdiy PCout(11)// 

void Scan_Init(void);
void Scan_GPIO_Init(void);
void Scan_Open(void);
void Scan_Close(void);
void Scan_AD_Write(u16 valx,u16 valy);
		 
#endif

