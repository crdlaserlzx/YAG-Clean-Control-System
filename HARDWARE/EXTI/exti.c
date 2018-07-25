#include "exti.h"
#include "delay.h"
#include "sys.h"
#include "lcd.h"

//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//外部中断 驱动代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/3
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   
//外部中断0服务程序

void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟

  //GPIOA.1 中断线以及中断初始化配置   下降沿触发   lcd_int
  //GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);    //lcd金鹏
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource5);

  EXTI_InitStructure.EXTI_Line=EXTI_Line5;	//
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //设置优先级分组；先占优先级2位，从优先级2位
		
//   NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQn;			//使能lcd_int所在的外部中断通道
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//抢占优先级0， 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//子优先级0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  NVIC_Init(&NVIC_InitStructure); 
 
}


// //外部中断1服务程序，lcd_int
// void EXTI1_IRQHandler(void)
// {
// 	//u8  temp;
// 	delay_ms(1000);
// 	
//  	LCD_sceenDisplay(test2);
// 	
// //	temp=LCD_read_data(INTR) & 0x01;	

//   LCD_write_reg(INTR);		//[0FH] , Default --> 0x00,Interrupt Setup and Status Register
// 	LCD_write_data(0x10);
// 	
// 	EXTI_ClearITPendingBit(EXTI_Line1); //清除LINE1上的中断标志位  
// }
 
// //外部中断2服务程序
// void EXTI2_IRQHandler(void)
// {
// 	delay_ms(10);//消抖
// // 	if(KEY2==0)	  //按键KEY2
// // 	{
// // 		LED0=!LED0;
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line2);  //清除LINE2上的中断标志位  
// }
// //外部中断3服务程序
// void EXTI3_IRQHandler(void)
// {
// 	delay_ms(10);//消抖
// // 	if(KEY1==0)	 //按键KEY1
// // 	{				 
// // 		LED1=!LED1;
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line3);  //清除LINE3上的中断标志位  
// }

// void EXTI4_IRQHandler(void)
// {
// 	delay_ms(10);//消抖
// // 	if(KEY0==0)	 //按键KEY0
// // 	{
// // 		LED0=!LED0;
// // 		LED1=!LED1; 
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line4);  //清除LINE4上的中断标志位  
// }
 
