#include "control_gpio.h"
#include "delay.h"
#include "sys.h"


//����:
//������ͣ-PE0-�ⲿ�ж�0��
//�Զ����в���PE1����ͨ���룻
//��ȴ����-PE5����ѹ����-PE6����Q����-PE7����������һ��ʹ��EXTI9_5_IRQHandler  
//���:
//����������PG0��������·��
//��Q������PG1�������̵���
//����������PG2��������ŷ�
//
//����������PG9��������ŷ�
//��բ������PG6��������բ
//����������PG7��������·��
//�Զ�����ָʾ��PG8������ָʾ��

void Control_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG, ENABLE);	 //ʹ��PORTE,PORTG�˿�ʱ��
	
	//PORTE:PE0-PE7��������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	 //�˿�����, ��������
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPD; 		 //
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOE, &GPIO_InitStructure);	  				   //������� ��IO���ٶ�Ϊ50MHz
	
		//PORTG:PG0-PG7�������
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 //�˿�����, �������
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP; 	 //
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOG, &GPIO_InitStructure);	  				   //������� ��IO���ٶ�Ϊ50MHz

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//ʹ�ܸ��ù���ʱ��
// 	
// 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //�������ȼ����飻��ռ���ȼ�2λ�������ȼ�2λ

// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource0);	
// 	
// 	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
// 	
// 	NVIC_InitStructure.NVIC_IRQChannel =EXTI0_IRQn;			//ʹ��lcd_int���ڵ��ⲿ�ж�ͨ��
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//�����ȼ�3
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//   NVIC_Init(&NVIC_InitStructure); 
}	
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource5);
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource6);
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource7);

// 	EXTI_InitStructure.EXTI_Line=EXTI_Line5;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
// 	
// 	EXTI_InitStructure.EXTI_Line=EXTI_Line6;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���

// 	EXTI_InitStructure.EXTI_Line=EXTI_Line7;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//����EXTI_InitStruct��ָ���Ĳ�����ʼ������EXTI�Ĵ���
// 		
//   NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQn;			//ʹ��lcd_int���ڵ��ⲿ�ж�ͨ��
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//��ռ���ȼ�2�� 
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//�����ȼ�3
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//ʹ���ⲿ�ж�ͨ��
//   NVIC_Init(&NVIC_InitStructure); 






