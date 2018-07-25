#include "delay.h"
#include "sys.h"
#include "AD5722R.h"

#define ad5722r_sdin PCout(2)// PA8
#define ad5722r_sclk PCout(1)// PA9
#define ad5722r_sync PCout(0)// PA10
// #define ad5722r_sdo PAout(4)// PA8
#define ad5722r_ldac PCout(3)// PA9
#define ad5722r_clr PCout(4)// PA10

void AD5722R_GPIO_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //ʹ��PORTA,PORTB�˿�ʱ��
	
	//PORTA:PA0-PA3������� 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	 //�˿�����, �������
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);	  				   //������� ��IO���ٶ�Ϊ50MHz

}

void AD5722R_soft_Init(void)
{
	ad5722r_sync=1;
	ad5722r_clr=0;
	
	ad5722r_ldac=1;
	ad5722r_sdin=0;
  ad5722r_sclk=0;
	
  delay_us(100);	  //�ȴ�AD5722R�����ѹ��λ
  
  ad5722r_clr=1;
	
	AD5722R_write_data(0x001c0000);   //AD5722R�ļĴ������
	AD5722R_write_data(0x00080001);   //AD5722R��Aͨ����Χ����
	AD5722R_write_data(0x000a0001);   //AD5722R��Bͨ����Χ����
	AD5722R_write_data(0x0019000d);   //AD5722R�ļĴ�������
	AD5722R_write_data(0x00100015);   //AD5722R�� power control�Ĵ�������
	AD5722R_write_data(0x001d0001);   //AD5722R�ļĴ�����������
}

void AD5722R_write_data(u32 data)   //uΪ��ѹֵ��12λ����ABѡ��ͨ��
{
	u8 i;
  data<<=8;
	ad5722r_sync=0;     //ʹ��AD5722R��������
	delay_us(10);
	for (i=0;i<24;i++)  //24λ���ݴ���
	{
		ad5722r_sclk=1;
		
		if(data&0x80000000)
		{
			ad5722r_sdin=1;
		}
		else
		{
			ad5722r_sdin=0;
		}
		data<<=1;
	  delay_us(10);
		
		ad5722r_sclk=0;
		
		delay_us(10);
	}
	delay_us(10);
	ad5722r_sync=1;
}



//��AD5722R��A��B��ͨ����д���ѹ����
void AD5722R_write_vol(double vol,char AB)   //ABΪA��B��
{
	unsigned short int  vol_16;
	unsigned int vol_data;  //32λ�޷�����
	
	// �ж�������ѹֵ�Ƿ���0-10V��Χ��
	if(vol>=2&&vol<=10)
	{
		vol_16=vol*0x1000/10-1;
	}
	else
	{
		goto over;
	}
	
	// �ж���ѡͨ���Ƿ�ΪA��B��ͨ��
	if(AB=='A')
	{
		vol_data=0x00000000+vol_16*0x10;
	}
	else if(AB=='B')
	{
		vol_data=0x00020000+vol_16*0x10;
	}
	else
	{
		goto over;
	}
	
	AD5722R_write_data(vol_data);
	
	ad5722r_ldac=0;
	;
	;
	;
	;
	;
	ad5722r_ldac=1;

	over:
	;
}

// u32 AD5722R_read_reg(void)
// {
// 	
// 	
// }




