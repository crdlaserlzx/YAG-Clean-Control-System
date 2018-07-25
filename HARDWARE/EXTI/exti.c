#include "exti.h"
#include "delay.h"
#include "sys.h"
#include "lcd.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//�ⲿ�ж� ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/3
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////   
//�ⲿ�ж�0�������

void EXTIX_Init(void)
{
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��

  //GPIOA.1 �ж����Լ��жϳ�ʼ������   �½��ش���   lcd_int
  //GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource1);    //lcd����
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource5);

  EXTI_InitStructure.EXTI_Line=EXTI_Line5;	//
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //�������ȼ����飻��ռ���ȼ�2λ�������ȼ�2λ
		
//   NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQn;			//ʹ��lcd_int���ڵ��ⲿ�ж�ͨ��
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;	//��ռ���ȼ�0�� 
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;					//�����ȼ�0
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
  NVIC_Init(&NVIC_InitStructure); 
 
}


// //�ⲿ�ж�1�������lcd_int
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
// 	EXTI_ClearITPendingBit(EXTI_Line1); //���LINE1�ϵ��жϱ�־λ  
// }
 
// //�ⲿ�ж�2�������
// void EXTI2_IRQHandler(void)
// {
// 	delay_ms(10);//����
// // 	if(KEY2==0)	  //����KEY2
// // 	{
// // 		LED0=!LED0;
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line2);  //���LINE2�ϵ��жϱ�־λ  
// }
// //�ⲿ�ж�3�������
// void EXTI3_IRQHandler(void)
// {
// 	delay_ms(10);//����
// // 	if(KEY1==0)	 //����KEY1
// // 	{				 
// // 		LED1=!LED1;
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line3);  //���LINE3�ϵ��жϱ�־λ  
// }

// void EXTI4_IRQHandler(void)
// {
// 	delay_ms(10);//����
// // 	if(KEY0==0)	 //����KEY0
// // 	{
// // 		LED0=!LED0;
// // 		LED1=!LED1; 
// // 	}		 
// 	EXTI_ClearITPendingBit(EXTI_Line4);  //���LINE4�ϵ��жϱ�־λ  
// }
 
