#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////	 
//���˼���������ϵͳ
//�޸�����:2013/12/24
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ϼ��� 2012-2022
//All rights reserved
//******************************************************************************** 

void System_Clock_Init(void)  //ʱ�ӳ�ʼ��
{
	//��ϵͳʱ��ʹ�ܣ�ʹ���ڲ�8MHz����
	//SystemInit()��
	//��ϵͳʱ��ʹ�ܣ�ʹ�����ⲿ����
	RCC_DeInit();                                       //��λ
	RCC_HSEConfig(RCC_HSE_ON);                          //�����ⲿ����,8MHZ
	while(RCC_WaitForHSEStartUp()==ERROR);              //�ȴ��ⲿ������
	RCC_PLLConfig(RCC_PLLSource_HSE_Div1,RCC_PLLMul_9); //����PLLʱ��Դ��PLL��Ƶϵ��9,ϵͳʱ��Ϊ72MHZ
	FLASH->ACR|=0x32;                                   //FLASH��ʱ�������� 
	RCC_PLLCmd(ENABLE);                                 //ʹ��PLL
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);          //ѡ��PLLΪϵͳʱ��
	while(RCC_GetSYSCLKSource()!=0x08);                 //�ȴ�PLL����Ϊϵͳʱ��
	RCC_ClockSecuritySystemCmd(ENABLE);                 //ʹ��ʱ�Ӱ�ȫϵͳ
} 

void System_Soft_Reset(void)
{

	__set_FAULTMASK(1);   /* Enable the PRIMASK priority */
  NVIC_SystemReset();  	/* Generate a system reset */
}

