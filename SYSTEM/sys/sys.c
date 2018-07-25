#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//光纤激光器控制系统
//修改日期:2013/12/24
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 凯瑞迪激光 2012-2022
//All rights reserved
//******************************************************************************** 

void System_Clock_Init(void)  //时钟初始化
{
	//将系统时钟使能，使用内部8MHz晶振
	//SystemInit()；
	//将系统时钟使能，使用内外部晶振
	RCC_DeInit();                                       //复位
	RCC_HSEConfig(RCC_HSE_ON);                          //开启外部晶振,8MHZ
	while(RCC_WaitForHSEStartUp()==ERROR);              //等待外部晶振开启
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9); //设置PLL时钟源及PLL倍频系数9,系统时钟为72MHZ
	FLASH->ACR|=0x32;                                   //FLASH延时两个周期 
	RCC_PLLCmd(ENABLE);                                 //使能PLL
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);          //选择PLL为系统时钟
	while(RCC_GetSYSCLKSource()!=0x08);                 //等待PLL设置为系统时钟
	RCC_ClockSecuritySystemCmd(ENABLE);                 //使能时钟安全系统
} 

void System_Soft_Reset(void)
{

	__set_FAULTMASK(1);   /* Enable the PRIMASK priority */
  NVIC_SystemReset();  	/* Generate a system reset */
}

