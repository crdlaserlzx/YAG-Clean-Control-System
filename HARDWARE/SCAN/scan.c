#include "scan.h"
#include "delay.h"



void Scan_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PORTA,PORTB�˿�ʱ��
	
	//PORTA:PA8-PA11�������,�����񾵿���
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11;	 //�˿�����, �������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);	  				   //������� ��IO���ٶ�Ϊ50MHz
	
}

void Scan_Init(void)
{
	Scan_GPIO_Init();
	Scan_AD_Write(0x8000,0x8000);
}


void Scan_Open(void)
{
	scan_sclk=0;
	scan_cs=0;	
	scan_sdix=1;
	scan_sdiy=1;
	scan_sclk=1;
	scan_cs=1;
}

void Scan_Close(void)
{
	scan_sclk=1;
	scan_cs=0;	
	scan_sdix=1;
	scan_sdiy=1;
	scan_sclk=0;
	delay_us(5);
}

void Scan_AD_Write(u16 valx,u16 valy)  
{
	u8 i;
	Scan_Open();
	
	for (i=16;i>0;i--)
	{
//  		delay_us(5);
		scan_sclk=1;
		
		if(valx & 0x8000)
		{
			scan_sdix=1;
		}
		else
		{
			scan_sdix=0;
		}
		valx<<=1;
		
		if(valy & 0x8000)
		{
			scan_sdiy=1;
		}
		else
		{
			scan_sdiy=0;
		}
		valy<<=1;
		
		scan_sclk=0;
//  		delay_us(5);
	}
	
// 	Scan_Close();
}


