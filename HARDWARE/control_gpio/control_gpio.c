#include "control_gpio.h"
#include "delay.h"
#include "sys.h"


//输入:
//包括急停-PE0-外部中断0；
//自动运行采用PE1，普通输入；
//冷却报警-PE5，气压报警-PE6，调Q报警-PE7，三个报警一起使用EXTI9_5_IRQHandler  
//输出:
//制冷启动：PG0，驱动断路器
//调Q启动：PG1，驱动继电器
//吹气启动：PG2，驱动电磁阀
//
//激光启动：PG9，驱动电磁阀
//关闸启动：PG6，驱动关闸
//抽气启动：PG7，驱动断路器
//自动运行指示：PG8，驱动指示灯

void Control_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE | RCC_APB2Periph_GPIOG, ENABLE);	 //使能PORTE,PORTG端口时钟
	
	//PORTE:PE0-PE7下拉输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;	 //端口配置, 下拉输入
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_IPD; 		 //
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOE, &GPIO_InitStructure);	  				   //推挽输出 ，IO口速度为50MHz
	
		//PORTG:PG0-PG7推挽输出
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 //端口配置, 推挽输出
	GPIO_InitStructure.GPIO_Mode =  GPIO_Mode_Out_PP; 	 //
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 		 //IO口速度为50MHz
  GPIO_Init(GPIOG, &GPIO_InitStructure);	  				   //推挽输出 ，IO口速度为50MHz

// 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	//使能复用功能时钟
// 	
// 	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);     //设置优先级分组；先占优先级2位，从优先级2位

// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource0);	
// 	
// 	EXTI_InitStructure.EXTI_Line=EXTI_Line0;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
// 	
// 	NVIC_InitStructure.NVIC_IRQChannel =EXTI0_IRQn;			//使能lcd_int所在的外部中断通道
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;					//子优先级3
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
//   NVIC_Init(&NVIC_InitStructure); 
}	
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource5);
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource6);
// 	GPIO_EXTILineConfig(GPIO_PortSourceGPIOE,GPIO_PinSource7);

// 	EXTI_InitStructure.EXTI_Line=EXTI_Line5;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
// 	
// 	EXTI_InitStructure.EXTI_Line=EXTI_Line6;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

// 	EXTI_InitStructure.EXTI_Line=EXTI_Line7;	//
//   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
//   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
//   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//   EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器
// 		
//   NVIC_InitStructure.NVIC_IRQChannel =EXTI9_5_IRQn;			//使能lcd_int所在的外部中断通道
//   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
//   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
//   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
//   NVIC_Init(&NVIC_InitStructure); 






