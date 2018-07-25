#include "delay.h"
#include "sys.h"
#include "LCD.h"

////////////////////////////////////////////////////////////////////////////////// 
//LCD屏控制相关端口的定义
//PB8-PB15对应DB0-DB7
//PA0对应RST；PA1对应INT；PA2对应BUSY；PA3对应CS2；
//PB0对应CS1；PB1对应E；PB3对应R/W；PB3对应RS；
////////////////////////////////////////////////////////////////////////////////// 

void LCD_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PORTA,PORTB端口时钟
	
	//PORTA:PA0-PA3推挽输出 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;	 //端口配置, 推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_Init(GPIOA, &GPIO_InitStructure);	  				   //推挽输出 ，IO口速度为50MHz
	
	//PORTB:PB0-PB3,PB8-PB15复用推挽输出 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;   //端口配置
  GPIO_Init(GPIOB, &GPIO_InitStructure);					     //推挽输出 ，IO口速度为50MHz
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
	
	//初始化 WK_UP-->GPIOA.0	  下拉输入
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; //PA0设置成输入，默认下拉	  
	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA.0
	
}


void lcd_soft_reset(void)
{
	lcd_cs1=0;
  lcd_cs2=1;
	lcd_rd =1;
	
	lcd_rst=0;
	delay_ms(30);
	lcd_rst=1;
	delay_ms(30);
	
	lcd_cs1=1;
	GPIO_SetBits(GPIOB, 0xff00 & 0xff00);
	GPIO_ResetBits(GPIOB, (~0xff00 & 0xff00));               //DB0-DB7输出
}


void LCD_write_reg(u8 reg)                     //写寄存器地址
{
	u16  temp;
	temp=reg*0x0100;
	
	lcd_cs1=0;
	lcd_rd=1;
	lcd_rs=1;
	
	GPIO_SetBits(GPIOB, temp & 0xff00);
	GPIO_ResetBits(GPIOB, (~temp & 0xff00));               //DB0-DB7输出
	
	lcd_wr=0;
	
	delay_us(100);
	
	lcd_wr=1;
	lcd_cs1=1;
}

void LCD_write_data(u8 data)        //向ddram写数据
{
	u16  temp;
	temp=data*0x0100;
	
	lcd_cs1=0;
	lcd_rd=1;
	lcd_rs=0;
	
	GPIO_SetBits(GPIOB, temp & 0xff00);
	GPIO_ResetBits(GPIOB, (~temp & 0xff00));               //DB0-DB7输出
	
	lcd_wr=0;
	
	delay_us(100);
	
	lcd_wr=1;
	
	lcd_rs=1;
	lcd_cs1=1;
	
}


u8 LCD_read_data(void)     
{
	u16 temp;
	u8  rddata;
	
	GPIO_InitTypeDef  GPIO_InitStructure;	
	//DB0-DB7上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		 //上拉输入
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_9|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;   //端口配置
  GPIO_Init(GPIOB, &GPIO_InitStructure);	
	
  delay_ms(5);
	
	lcd_wr=1;
	lcd_rs=0;
	lcd_cs1=0;
	lcd_rd=0;
	
	delay_us(500);
	
	temp=GPIO_ReadInputData(GPIOB);
	
	lcd_rd=1;
	lcd_cs1=1;
	lcd_rs=1;
	
	rddata=temp/0x0100;

	//DB0-DB7推挽输出
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	return(rddata);
}



void LCD_reg_init(void)    // LCD寄存器的初始化
{
	LCD_write_reg(WLCR);		//[00H] , Default --> 0x00,Whole Chip LCD Controller Register
	LCD_write_data(0x00);

	LCD_write_reg(MISC);		//[01H] , Default --> 0x00,Misc. Register
	LCD_write_data(0x34);

	LCD_write_reg(ADSR);		//[03H] ,（平移）Default --> 0x00 ,Advance Display Setup Register
	LCD_write_data(0x83);

	LCD_write_reg(INTR);		//[0FH] , Default --> 0x00,Interrupt Setup and Status Register
	LCD_write_data(0x10);

	LCD_write_reg(WCCR);		//[10H] , Default --> 0x00,Whole Chip Cursor Control Register
	LCD_write_data(0x06);
	//LCD_write_data(0x08);//旋转90度

	LCD_write_reg(CHWI);		//[11H] , Default --> 0x00,Cursor Height and Word Interval Register
	//LCD_write_data(0x00);
	LCD_write_data(0xb0);

	LCD_write_reg(MAMR);		//[12H] , Default --> 0x11,Memory Access Mode Register
	LCD_write_data(0x11);

	LCD_write_reg(AWRR);		//[20H] , Default --> 0x27,Active Window Right Register
	//LCD_write_data(0x27);
	LCD_write_data(0x1d);

	LCD_write_reg(DWWR);		//[21H] , Default --> 0x27,Display Window Width Register
	//LCD_write_data(0x27);
	LCD_write_data(0x1d);

	LCD_write_reg(AWBR);		//[30H] , Default --> 0xEF,Active Window Bottom Register
	//LCD_write_data(0xef);
	LCD_write_data(0x7f);

	LCD_write_reg(DWHR);		//[31H] , Default --> 0xEF,Display Window Height Register
	//LCD_write_data(0xef);
	LCD_write_data(0x7f);

	LCD_write_reg(AWLR);		//[40H] , Default --> 0x00,Active Window Left Register
	LCD_write_data(0x00);

	LCD_write_reg(AWTR);		//[50H] , Default --> 0x00,Active Window Top Register
	LCD_write_data(0x00);

	LCD_write_reg(CURX);		//[60H] , Default --> 0x00 ,
	LCD_write_data(0x00);

	LCD_write_reg(BGSG);		//[61H] , Default --> 0x00,REG[61h] defines the start position (left boundary) of scroll window
	LCD_write_data(0x00);

	LCD_write_reg(EDSG);		//[62H] , Default --> 0x00,REG[62h] defines the end position(right boundary) of scroll window
	LCD_write_data(0x00);

	LCD_write_reg(CURY);		//[70H] , Default --> 0x00
	LCD_write_data(0x00);

	LCD_write_reg(BGCM);		//[71H] , Default --> 0x00,
	LCD_write_data(0x00);

	LCD_write_reg(EDCM);		//[72H] , Default --> 0x00
	LCD_write_data(0x00);

	LCD_write_reg(BTMR);		//[80H] , Default --> 0x00
	LCD_write_data(0x00);

	LCD_write_reg(ITCR);		//[90H] , Default --> 0x00,The value can determine the scan time of each COM of the LCD
	LCD_write_data(0x84);

// 	LCD_write_reg(KSCR1);	//[A0H] , Default --> 0x00
// 	LCD_write_data(0x00);

// 	LCD_write_reg(KSCR2);	//[A1H] , Default --> 0x00
// 	LCD_write_data(0x00);

// 	LCD_write_reg(KSDR0);	//[A2H] , Default --> 0x00
// 	LCD_write_data(0x00);

// 	LCD_write_reg(KSDR1);	//[A3H] , Default --> 0x00
// 	LCD_write_data(0x00);

// 	LCD_write_reg(KSDR2);	//[A3H] , Default --> 0x00
// 	LCD_write_data(0x00);

//	LCD_write_reg(MRCR);		//[B1H] , Default --> NA

	LCD_write_reg(TPCR1);		//[C0H] , Default --> 0x00，Touch Panel Control Register 1 (TPCR1)
	LCD_write_data(0x80);

//	LCD_write_reg(TPXR);		//[C1H] , Default --> 0x00，Touch Panel X High Byte Data Register
//	LCD_write_data(0x00);

//	LCD_write_reg(TPYR);		//[C2H] , Default --> 0x00，Touch Panel Y High Byte Data Register
//	LCD_write_data(0x00);

//	LCD_write_reg(TPZR);		//[C3H] , Default --> 0x00，Touch Panel Segment/Common Low Byte Data Register
//	LCD_write_data(0x00);

 	LCD_write_reg( TPCR2);		
  LCD_write_data(0x81);

	LCD_write_reg(PCR);		//[D0H] , Default --> 0x00,PWM Control Register
	LCD_write_data(0x00);

	LCD_write_reg(PDCR);		//[D1H] , Default --> 0x00,PWM Duty Cycle Register
	LCD_write_data(0x00);

	LCD_write_reg(PNTR);		//[E0H] , Default --> 0x00,Pattern Data Register
	LCD_write_data(0x00);

	LCD_write_reg(FNCR);		//[F0H] , Default --> 0x00.Font Control Register
	LCD_write_data(0x00);
	
	LCD_write_reg(FVHT);		//[F1H] , Default --> 0x00,Font Size Control Register
	LCD_write_data(0x00);
	
	LCD_write_reg( INTR);		
	LCD_write_data(0x10);
		
}


//--------------------
void LCD_clear(void)
{
	LCD_write_reg(PNTR);  //向PNTR写0
	LCD_write_data(0x00);
	LCD_write_reg(FNCR);   //
	LCD_write_data(0xa8);
  delay_ms(3);
	
} 

//-----------------------------------------------------
void LCD_character(u8 *cha) 
{                                   //显示中文或字符
  u16 i,j;
	j=strlen(cha);
  for(i=0;i<j;i++)
  {
    LCD_write_data(*cha);           //8*15*2=240
    ++cha;
  }
}


// void lcd_int(uchar *cha,uchar count)
// {                                   
//   uchar i;
//   for(i=0;i<count;i++)
//   {
//     LCD_write_data(*cha);
//     ++cha;
//   }
// }

//-------------------------------------------------------
// u8 touch_x(void) 
// {
//    u8 touchxl;
//    u8 touchx;
//    
//    touchx= LCD_read_data(TPXR);     // Read high byte of X-axis 

//    touchxl= LCD_read_data(TPZR)&0x03;  // Read Least two Bits of X-axis 

//   touchx = (touchx<<2)|touchxl;
//   return(touchx);
// }								            
//-------------------------------------------
// u8 touch_y(void) 
// {
//    u8 touchyl;
//    u8 touchy;
//   
//    touchy= LCD_read_data(TPYR);     // Read high byte of Y-axis 

//    touchyl= LCD_read_data(TPZR)&0x0c;  // Read Least two Bits of Y-axis 

//    touchy = (touchy<<2)|(touchyl>>2);
//    return(touchy);
// }

//  //-------------------------------------------------------------------
// void getTouchXY(int *touchx1,int *touchy1,int *touchx2,int *touchy2)
// {	
// 	u8 INT_Sta,temp;
// 	LCD_write_reg( TPCR1);	
// 	LCD_write_data(0x80);    	//触控屏幕功能的致能位。  Default --> 0x00

//   LCD_write_reg( TPCR2);		
// 	LCD_write_data(0x80);        //手动模式.    Default --> 0x00
//     
//   LCD_write_reg( TPCR2);		
// 	LCD_write_data(0x81);        //手动模式. Wait for TP event Mode     Default --> 0x00
//           
//     INT_Sta = LCD_read_data (INTR);
// 	if( INT_Sta &  0x08 )            // Check INTR.Bit-3  
//   	{ 
//     	delay_ms(30);
// 		*touchx1=touch_x();
// 		*touchy1=touch_y();	
//      	INT_Sta = LCD_read_data(INTR );
// 		delay_us(300);		
// 		if( INT_Sta &  0x08 )            // Check INTR.Bit-3  
// 		{ 
//         	*touchx2=touch_x();
// 			*touchy2=touch_y();	
// 		}
// 	}
// 	LCD_write_reg( TPCR2);		
//     LCD_write_data(0x82);           // Set REG[C4h][1:0] = 10 
//     delay_ms(6);                  // Delay enough time 

//     LCD_write_reg( TPCR2);		
//     LCD_write_data(0x83);            // Set REG[C4h][1:0] = 11 
//     delay_ms(6);                  // Delay enough time
// 	 
//     temp = LCD_read_data (INTR)&0xfe; 
//     LCD_write_reg ( INTR ); 
//     LCD_write_data ( temp );
// 	LCD_write_reg( TPCR1);	
// 	LCD_write_data(0x00);    //关闭触摸屏
// }

void LCD_sceenDisplay(u8 * dispTemp)
{
	delay_ms(5);
	
	//lcd_soft_reset();
	//LCD_reg_init();
  LCD_clear();
	
	LCD_write_reg(WLCR);                 //Whole Chip LCD Controller Register
	LCD_write_data(0x0c);
	
	
	LCD_write_reg(AWLR);		//[40H] , Default --> 0x00,Active Window Left Register  
	LCD_write_data(0x00);
	
	LCD_write_reg(AWRR);		//[20H] , Default --> 0x27,Active Window Right Register
	//LCD_write_data(0x1d);
	LCD_write_data(0x1d);

	LCD_write_reg(AWTR);		//[50H] , Default --> 0x00,Active Window Top Register
	LCD_write_data(0x00);
	
	LCD_write_reg(AWBR);		//[50H] , Default --> 0x00,Active Window Top Register
	LCD_write_data(0x7f);
	
	LCD_write_reg(DWWR);		//[21H] , Default --> 0x27,Display Window Width Register
	//LCD_write_data(0x1d);
	LCD_write_data(0x1d);

	LCD_write_reg(DWHR);		//[31H] , Default --> 0xEF,Display Window Height Register
	//LCD_write_data(0xef);
	LCD_write_data(0x7f);
	
	
	
	LCD_write_reg(CURX);                 //
	LCD_write_data(0x00);
	
	LCD_write_reg(CURY);                 //
	LCD_write_data(0x00);
	
  LCD_write_reg(MWCR);                  //Write memory data, user must write the MWCR command first, then write DATA cycle.
  
	LCD_character(dispTemp);    
	
	delay_ms(30);
	
	LCD_write_reg(CURX);                  //Define the cursor address of segment, a value from 0h ~ 27h(0 ~ 40 in decimal)
	LCD_write_data(0);
	
	LCD_write_reg(CURY);      //Define the cursor address of common, a value from 0h ~ EFh(0 ~ 239 in decimal)
	LCD_write_data(0x66);

// 	LCD_write_reg(BGSG);      //REG[61h] defines the start position (left boundary) of scroll window,                        
// 	LCD_write_data(0x00);     //
// 	
//   LCD_write_reg(EDSG);     //REG[62h] defines the end position(right boundary) of scroll window
// 	LCD_write_data(0x1d);    //
// 	
//   LCD_write_reg(BGCM);      //REG[71h] defines the begin position(top boundary) of scroll window
// 	LCD_write_data(0x00);     //
// 	
//   LCD_write_reg(EDCM);      // REG[72h] defines the end position(bottom boundary) of scroll window
// 	LCD_write_data(0x7f);     //
	
  LCD_write_reg(BTMR);     //Cursor Blink Time and Scroll Time,Blinking Time = Bit[7:0] x (Frame width),Frame width = 1/Frame Rate
  LCD_write_data(0x55);    //
}

// void displayWarning(uchar code * dispTemp)
// {
// 	LCD_write_reg(0x60);    
// 	LCD_write_data(0x00);
// 	LCD_write_reg(0x70);  
// 	LCD_write_data(0x70);
// 	 LCD_write_reg(0xb0); 
//     lcd_character(dispTemp,30);
// 	delay_nms(30);
// 	LCD_write_reg(0x60);    
// 	LCD_write_data(0x128);
// 	LCD_write_reg(0x70);  
// 	LCD_write_data(0x240);		
// }

