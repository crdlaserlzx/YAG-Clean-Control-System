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
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);	 //使能PORTA,PORTB端口时钟
	
	//PORTA:PA0-PA3推挽输出 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4;	 //端口配置, 推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOC, &GPIO_InitStructure);	  				   //推挽输出 ，IO口速度为50MHz

}

void AD5722R_soft_Init(void)
{
	ad5722r_sync=1;
	ad5722r_clr=0;
	
	ad5722r_ldac=1;
	ad5722r_sdin=0;
  ad5722r_sclk=0;
	
  delay_us(100);	  //等待AD5722R输出电压复位
  
  ad5722r_clr=1;
	
	AD5722R_write_data(0x001c0000);   //AD5722R的寄存器清除
	AD5722R_write_data(0x00080001);   //AD5722R的A通道范围设置
	AD5722R_write_data(0x000a0001);   //AD5722R的B通道范围设置
	AD5722R_write_data(0x0019000d);   //AD5722R的寄存器设置
	AD5722R_write_data(0x00100015);   //AD5722R的 power control寄存器设置
	AD5722R_write_data(0x001d0001);   //AD5722R的寄存器设置锁存
}

void AD5722R_write_data(u32 data)   //u为电压值，12位数；AB选择通道
{
	u8 i;
  data<<=8;
	ad5722r_sync=0;     //使能AD5722R接收数据
	delay_us(10);
	for (i=0;i<24;i++)  //24位数据传输
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



//向AD5722R的A、B两通道中写入电压数据
void AD5722R_write_vol(double vol,char AB)   //AB为A或B；
{
	unsigned short int  vol_16;
	unsigned int vol_data;  //32位无符号数
	
	// 判断所赋电压值是否在0-10V范围内
	if(vol>=2&&vol<=10)
	{
		vol_16=vol*0x1000/10-1;
	}
	else
	{
		goto over;
	}
	
	// 判断所选通道是否为A、B两通道
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




