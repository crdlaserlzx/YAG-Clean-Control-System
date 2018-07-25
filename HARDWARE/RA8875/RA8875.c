
//=============================================================
//RA8875 4ÏßSPIÇý¶¯³ÌÐò

#include "delay.h"
#include "sys.h"
#include "RA8875.h"
#include <math.h>


void RA8875_GPIO_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
 	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //Ê¹ÄÜPORTA,PORTB¶Ë¿ÚÊ±ÖÓ
	
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8|GPIO_Pin_10|GPIO_Pin_11|GPIO_Pin_14;	 //¶Ë¿ÚÅäÖÃ, ÍÆÍìÊä³ö
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //ÍÆÍìÊä³ö
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO¿ÚËÙ¶ÈÎª50MHz
  GPIO_Init(GPIOB, &GPIO_InitStructure);	  				   //ÍÆÍìÊä³ö £¬IO¿ÚËÙ¶ÈÎª50MHz

	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_9|GPIO_Pin_12|GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; //PA0ÉèÖÃ³ÉÊäÈë£¬Ä¬ÈÏÏÂÀ­	  
	GPIO_Init(GPIOB, &GPIO_InitStructure);//³õÊ¼»¯GPIOA.0
}



void NextStep(void)
{ 
 	while(next==1)
	{
			delay_ms(1);
	}
}

//*********4W_SPI_Init()
void RA8875_SPI_Init(void)
{
	SCLK = 1;	
	SDI = 1;
// 	SDO = 1;	
	SCS = 1;
}


//*********4W_SPI_Write()
void SPI_Write(unsigned char dat)
{
	unsigned char t = 8;
	do
	{
		if(dat&0x80)
		{
			SDI =1;
		}
		else
		{
			SDI =0;
		}
		delay_us(5);
		dat <<= 1;
		SCLK = 0;	
		delay_us(5);
		SCLK = 1;
	} while ( --t != 0 );
}

//*********4W_SPI_Read()
unsigned char SPI_Read(void)
{
   unsigned char dat = 0;
   unsigned char t = 8;
// 	SDO = 1;
	do
	{
		SCLK = 0;
    delay_us(12);
		dat <<= 1;
		if(SDO)  dat++;
		SCLK = 1;	
    delay_us(12);
	} while ( --t != 0 );
	return dat;
}

//////////////SPI Write command
void LCD_CmdWrite(u8 cmd)
{	
	SCLK = 1;	
	SDI = 1;	
	SCS = 0;
	//SPI_Delay();
	SPI_Write(0x80); 
	SPI_Write(cmd);
	SCS = 1;
	//SPI_Delay();
}

//////////////SPI Write data or  parameter
void LCD_DataWrite(u8 Data)
{
	SCLK = 1;	
	SDI = 1;		
	SCS = 0;
	SPI_Write(0x00); 
	SPI_Write(Data);
	//SPI_Delay();
	SCS = 1;
}

///////////////Read data or  parameter
u8 LCD_DataRead(void)
{
	u8 Data;
	SCLK = 1;
	SDO = 1;
	SCS = 0;
	SPI_Write(0x40);  
	Data = SPI_Read();
	//SPI_Delay();
	SCS = 1;
	return Data;
}  

////////////////Write command and parameter
void Write_Dir(u8 Cmd,u8 Data)
{
  LCD_CmdWrite(Cmd);
  LCD_DataWrite(Data);
}

///////////SPI Read  status
u8 LCD_StatusRead(void)
{
	u8 Data;	
	SCLK = 1;	
//	SDO = 1;	
	SCS = 0;
	//SPI_Delay();
	SPI_Write(0xc0); 	
	Data = SPI_Read();
	//SPI_Delay();
	SCS = 1;	
	return Data;
}

////////LCM reset
void LCD_Reset(void)
{
	lcd_reset = 0;
	delay_ms(1);
	lcd_reset = 1;
	delay_ms(1);
}

///////////////check busy
void Chk_Busy(void)
{
	u8 temp; 	
	do
	{
	  temp=LCD_StatusRead();
	}while((temp&0x80)==0x80);		   
}
///////////////check bte busy
void Chk_BTE_Busy(void)
{
	u8 temp; 	
	do
	{
	temp=LCD_StatusRead();
	}while((temp&0x40)==0x40);		   
}
///////////////check dma busy
void Chk_DMA_Busy(void)
{
	u8 temp; 	
	do
	{
	LCD_CmdWrite(0xBF);
	temp =LCD_DataRead();
	}while((temp&0x01)==0x01);   
}



/////////////PLL setting
void PLL_ini(void)
{
    LCD_CmdWrite(0x88);      
    LCD_DataWrite(0x0a);
    delay_ms(1);     
    LCD_CmdWrite(0x89);        
    LCD_DataWrite(0x02);  
    delay_ms(1);
}	

//     LCD_CmdWrite(0x88);      
//     LCD_DataWrite(0x0a);
//     delay_ms(1);     
//     LCD_CmdWrite(0x89);        
//     LCD_DataWrite(0x02);  
//     delay_ms(1);


/////////////////Set the working window area
void Active_Window(u16 XL,u16 XR ,u16 YT ,u16 YB)
{
	u8 temp;
    //setting active window X
	temp=XL;   
    LCD_CmdWrite(0x30);//HSAW0
	LCD_DataWrite(temp);
	temp=XL>>8;   
    LCD_CmdWrite(0x31);//HSAW1	   
	LCD_DataWrite(temp);

	temp=XR;   
    LCD_CmdWrite(0x34);//HEAW0
	LCD_DataWrite(temp);
	temp=XR>>8;   
    LCD_CmdWrite(0x35);//HEAW1	   
	LCD_DataWrite(temp);

    //setting active window Y
	temp=YT;   
    LCD_CmdWrite(0x32);//VSAW0
	LCD_DataWrite(temp);
	temp=YT>>8;   
    LCD_CmdWrite(0x33);//VSAW1	   
	LCD_DataWrite(temp);

	temp=YB;   
    LCD_CmdWrite(0x36);//VEAW0
	LCD_DataWrite(temp);
	temp=YB>>8;   
    LCD_CmdWrite(0x37);//VEAW1	   
	LCD_DataWrite(temp);
}




/////////////LCM initial
void LCD_Initial(void)
{ 	
  lcd_reset=0;
  delay_ms(10);
  lcd_reset=1;
	PLL_ini();
	LCD_CmdWrite(0x10);	 //System Configuration Register (SYSR)  bit[4:3] color  bit[2:1]=  MPU interface
	LCD_DataWrite(0x0c);   //          65K						 
	
	LCD_CmdWrite(0x04);    //Pixel Clock Setting Register (PCSR)
	LCD_DataWrite(0x81);   //
	delay_ms(1);

  //Horizontal set
  LCD_CmdWrite(0x14); //HDWR//Horizontal Display Width Setting Bit[6:0]                   
  LCD_DataWrite(0x63);//Horizontal display width(pixels) = (HDWR + 1)*8                   
  LCD_CmdWrite(0x15); //Horizontal Non-Display Period Fine Tuning Option Register (HNDFTR)
  LCD_DataWrite(0x03);//Horizontal Non-Display Period Fine Tuning(HNDFT) [3:0]            
  LCD_CmdWrite(0x16); //HNDR//Horizontal Non-Display Period Bit[4:0]                      
  LCD_DataWrite(0x03);//Horizontal Non-Display Period (pixels) = (HNDR + 1)*8             
  LCD_CmdWrite(0x17); //HSTR//HSYNC Start Position[4:0]                                   
  LCD_DataWrite(0x02);//HSYNC Start Position(PCLK) = (HSTR + 1)*8                         
  LCD_CmdWrite(0x18); //HPWR//HSYNC Polarity ,The period width of HSYNC.                  
  LCD_DataWrite(0x00);//HSYNC Width [4:0]   HSYNC Pulse width(PCLK) = (HPWR + 1)*8     
   //Vertical set    
  LCD_CmdWrite(0x19); //VDHR0 //Vertical Display Height Bit [7:0]                         
  LCD_DataWrite(0xdf);//Vertical pixels = VDHR + 1                                        
  LCD_CmdWrite(0x1a); //VDHR1 //Vertical Display Height Bit [8]                           
  LCD_DataWrite(0x01);//Vertical pixels = VDHR + 1                                        
  LCD_CmdWrite(0x1b); //VNDR0 //Vertical Non-Display Period Bit [7:0]                     
  LCD_DataWrite(0x14);//Vertical Non-Display area = (VNDR + 1)                            
  LCD_CmdWrite(0x1c); //VNDR1 //Vertical Non-Display Period Bit [8]                       
  LCD_DataWrite(0x00);//Vertical Non-Display area = (VNDR + 1)                            
  LCD_CmdWrite(0x1d); //VSTR0 //VSYNC Start Position[7:0]                                 
  LCD_DataWrite(0x06);//VSYNC Start Position(PCLK) = (VSTR + 1)                           
  LCD_CmdWrite(0x1e); //VSTR1 //VSYNC Start Position[8]                                   
  LCD_DataWrite(0x00);//VSYNC Start Position(PCLK) = (VSTR + 1)                           
  LCD_CmdWrite(0x1f); //VPWR //VSYNC Polarity ,VSYNC Pulse Width[6:0]                     
  LCD_DataWrite(0x01);//VSYNC Pulse Width(PCLK) = (VPWR + 1)  
 
  LCD_CmdWrite(0x29);//ÉèÖÃÐÐ¾à
	LCD_DataWrite(0x02);

	Active_Window(0,799,0,479);

	LCD_CmdWrite(0x8a);//PWM setting
	LCD_DataWrite(0x80);
	LCD_CmdWrite(0x8a);//PWM setting
	LCD_DataWrite(0x81);//open PWM
	LCD_CmdWrite(0x8b);//Backlight brightness setting
	LCD_DataWrite(0xff);//Brightness parameter 0xff-0x00
	
	//ÉèÖÃ×Ö·ûÏÔÊ¾µÄ×Ö¿â
	Text_Foreground_Color1(color_white);//Set the foreground color
	Text_Background_Color1(color_blue);//Set the background color
	Write_Dir(0X01,0X80);//display on
	Active_Window(0,799,0,479);//Set the working window size
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	Write_Dir(0x21,0x20);//Select the external CGROM  ISO/IEC 8859-1.
	Write_Dir(0x2f,0x80);//Serial Font ROM Setting
	Chk_Busy();
	
	//¿ªÆô´¥ÃþÆÁ
	Write_Dir(0x70,0xb2);  //open the touch function, touch the parameter settings
	Write_Dir(0x71,0x05);  //set to 4-wire touch screen
	Write_Dir(0xf0,0x04);  //open interruption
	
	//Çå³ý´¥¿Ø±êÖ¾Î»
	Write_Dir(0xf1,0x04);
}

//////////////writing text
void String(u8 *str)
{   
  Write_Dir(0x40,0x80);//Set the character mode
	LCD_CmdWrite(0x02);
	while(*str != '\0')
	{
	 LCD_DataWrite(*str);
	 ++str;	 	
// 	 Chk_Busy();		
	} 
}

void Text(u16 XL,u16 XR ,u16 YT ,u16 YB,u8 *str,u8 set,u8 setRs,u8 setGs,u8 setBs)
{
	Active_Window(XL,XR ,YT ,YB);
	Write_Dir(0X8E,0XC0);//Began to clear the screen (Active window)
	delay_ms(1);
	
	Write_Dir(0x22,set);
	Text_Foreground_Color(setRs,setGs,setBs);
	FontWrite_Position(XL,YT);
	
  Write_Dir(0x40,0x80);//Set the character mode
	LCD_CmdWrite(0x02);
	while(*str != '\0')
	{
	 LCD_DataWrite(*str);
	 ++str;	 	
// 	 Chk_Busy();		
	} 
	Active_Window(0,799 ,0 ,479);
}

void Text1(u16 XL,u16 XR ,u16 YT ,u16 YB,u8 *str,u8 set,u8 setRs,u8 setGs,u8 setBs)
{
	Active_Window(XL,XR ,YT ,YB);
// 	Write_Dir(0X8E,0XC0);//Began to clear the screen (Active window)
	delay_ms(1);
	
	Write_Dir(0x22,set);
	Text_Foreground_Color(setRs,setGs,setBs);
	FontWrite_Position(XL,YT);
	
  Write_Dir(0x40,0x80);//Set the character mode
	LCD_CmdWrite(0x02);
	while(*str != '\0')
	{
	 LCD_DataWrite(*str);
	 ++str;	 	
// 	 Chk_Busy();		
	} 
	Active_Window(0,799 ,0 ,479);
}


///////////////Background color settings
void Text_Background_Color1(u16 b_color)
{
	LCD_CmdWrite(0x60);//BGCR0
	LCD_DataWrite((u8)(b_color>>11));
	
	LCD_CmdWrite(0x61);//BGCR0
	LCD_DataWrite((u8)(b_color>>5));
	
	LCD_CmdWrite(0x62);//BGCR0
	LCD_DataWrite((u8)(b_color));
} 

///////////////Background color settings
void Text_Background_Color(u8 setR,u8 setG,u8 setB)
{
  LCD_CmdWrite(0x60);//BGCR0
	LCD_DataWrite(setR);
   
  LCD_CmdWrite(0x61);//BGCR0
	LCD_DataWrite(setG);

  LCD_CmdWrite(0x62);//BGCR0
	LCD_DataWrite(setB);
} 

////////////////Foreground color settings
void Text_Foreground_Color1(u16 b_color)
{
	
	LCD_CmdWrite(0x63);//BGCR0
	LCD_DataWrite((u8)(b_color>>11));
	
	LCD_CmdWrite(0x64);//BGCR0
	LCD_DataWrite((u8)(b_color>>5));
	
	LCD_CmdWrite(0x65);//BGCR0
	LCD_DataWrite((u8)(b_color));
} 

////////////////Foreground color settings
void Text_Foreground_Color(u8 setR,u8 setG,u8 setB)
{	    
    LCD_CmdWrite(0x63);//BGCR0
	LCD_DataWrite(setR);
   
    LCD_CmdWrite(0x64);//BGCR0
	LCD_DataWrite(setG);

    LCD_CmdWrite(0x65);//BGCR0·
	LCD_DataWrite(setB);
}

////////////////Text write position
void FontWrite_Position(u16 X,u16 Y)
{
	u8 temp;
	temp=X;   
    LCD_CmdWrite(0x2A);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x2B);	   
	LCD_DataWrite(temp);

	temp=Y;   
    LCD_CmdWrite(0x2C);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x2D);	   
	LCD_DataWrite(temp);
}

/////////////Touch the interrupt judgment
u8 Touch_Status(void)
{
  u8 temp;
	LCD_CmdWrite(0xF1);//INTC	
	temp =LCD_DataRead();
	if ((temp&0x04)==0x04)
// 	temp=LCD_StatusRead();
// 	if ((temp&0x20)==0x20)
	return 0x01;
	else 
	return 0x00;
}

//////////////Resistive Touch Screen the coordinate display
u16 TP_X_Coordinate(void)	
{
	unsigned int x,xy,lx;
	delay_ms(1);
	
	LCD_CmdWrite(0x74);//TPXYL	  bit[3:2] Y_coordinate low byte  bit[1:0] X_coordinate low byte 
	//Chk_Busy();
	xy=LCD_DataRead()&0x0f;
	
	LCD_CmdWrite(0x72);//TPXH	 X_coordinate high byte
	//Chk_Busy();
	x=(LCD_DataRead()<<2)+(xy&0x03);
	
	lx=(((x-60)*20/23));

	Write_Dir(0xf1,0x04);//clear INT state      Must be clean TP_interrupt 
	
	return  lx;
}

//////////////Resistive Touch Screen the coordinate display
u16 TP_Y_Coordinate(void)	
{
	unsigned int y,xy,ly;
	delay_ms(1);
	
	LCD_CmdWrite(0x74);//TPXYL	  bit[3:2] Y_coordinate low byte  bit[1:0] X_coordinate low byte 
	//Chk_Busy();
	xy=LCD_DataRead()&0x0f;
	
	LCD_CmdWrite(0x73);//TPYH	  Y_coordinate high byte
    //Chk_Busy();
	y=(LCD_DataRead()<<2)+(xy>>2);
	
	ly=(((y-110)*4/7));			 
			
	Write_Dir(0xf1,0x04);//clear INT state      Must be clean TP_interrupt 
	
	return  ly;
}

//////////////Resistive Touch Screen the coordinate display
void LCD_TP_Coordinate(void)	
{
	unsigned int x,y,xy,tabx[3],taby[3],lx,ly;
	delay_ms(1);
	
	LCD_CmdWrite(0x74);//TPXYL	  bit[3:2] Y_coordinate low byte  bit[1:0] X_coordinate low byte 
	//Chk_Busy();
	xy=LCD_DataRead()&0x0f;
	
	LCD_CmdWrite(0x72);//TPXH	 X_coordinate high byte
	//Chk_Busy();
	x=(LCD_DataRead()<<2)+(xy&0x03);
	
	LCD_CmdWrite(0x73);//TPYH	  Y_coordinate high byte
    //Chk_Busy();
	y=(LCD_DataRead()<<2)+(xy>>2);

  tabx[0]=x/1000;
  tabx[1]=x%1000/100;
  tabx[2]=x%100/10;
  tabx[3]=x%10;
  taby[0]=y/1000;
  taby[1]=y%1000/100;
  taby[2]=y%100/10;
  taby[3]=y%10;

  	FontWrite_Position(350,60);   //Set the display position
  	LCD_CmdWrite(0x02);
  	String("X = ");
  	LCD_DataWrite(tabx[0] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(tabx[1] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(tabx[2] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(tabx[3] |= 0x30);
	//Delay1ms(1);

  	FontWrite_Position(400, 140);   //Set the display position
  	LCD_CmdWrite(0x02);
  	String("Y = ");
  	LCD_DataWrite(taby[0] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(taby[1] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(taby[2] |= 0x30);
	//Delay1ms(1);
  	LCD_DataWrite(taby[3] |= 0x30);
	//Delay1ms(1);

  Write_Dir(0x40,0x00);//The drawing mode
	
	lx=(((x-60)*20/23));
	ly=(((y-110)*4/7));
					  
	MemoryWrite_Position(lx,ly);//Memory write position
    LCD_CmdWrite(0x02);//Memory write Data				   
	LCD_DataWrite(0x07e0);
	LCD_DataWrite(0x07e0);
	MemoryWrite_Position(lx,ly+1);//Memory write position
    LCD_CmdWrite(0x02);////Memory write Data					   
	LCD_DataWrite(0x07e0);
	LCD_DataWrite(0x07e0);					 
			
	Write_Dir(0xf1,0x04);//clear INT state      Must be clean TP_interrupt 
}


///////////////Memory write position
void MemoryWrite_Position(u16 X,u16 Y)
{
	u8 temp;

	temp=X;   
    LCD_CmdWrite(0x46);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x47);	   
	LCD_DataWrite(temp);

	temp=Y;   
    LCD_CmdWrite(0x48);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x49);	   
	LCD_DataWrite(temp);
}

/////////////////Scroll window size
void Scroll_Window(u16 XL,u16 XR ,u16 YT ,u16 YB)
{
	u8 temp;    
	temp=XL;   
    LCD_CmdWrite(0x38);//HSSW0
	LCD_DataWrite(temp);
	temp=XL>>8;   
    LCD_CmdWrite(0x39);//HSSW1	   
	LCD_DataWrite(temp);

	temp=XR;   
    LCD_CmdWrite(0x3c);//HESW0
	LCD_DataWrite(temp);
	temp=XR>>8;   
    LCD_CmdWrite(0x3d);//HESW1	   
	LCD_DataWrite(temp);   
    
	temp=YT;   
    LCD_CmdWrite(0x3a);//VSSW0
	LCD_DataWrite(temp);
	temp=YT>>8;   
    LCD_CmdWrite(0x3b);//VSSW1	   
	LCD_DataWrite(temp);

	temp=YB;   
    LCD_CmdWrite(0x3e);//VESW0
	LCD_DataWrite(temp);
	temp=YB>>8;   
    LCD_CmdWrite(0x3f);//VESW1	   
	LCD_DataWrite(temp);
}  


///////////////Window scroll offset Settings
void Scroll(u16 X,u16 Y)
{
	u8 temp;
    
	temp=X;   
    LCD_CmdWrite(0x24);//HOFS0
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x25);//HOFS1	   
	LCD_DataWrite(temp);

	temp=Y;   
    LCD_CmdWrite(0x26);//VOFS0
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x27);//VOFS1	   
	LCD_DataWrite(temp); 
}	   	  







///////////drawing circle
void  Draw_Circle(u16 X,u16 Y,u16 R)
{
	u8 temp;
    
	temp=X;   
    LCD_CmdWrite(0x99);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x9a);	   
	LCD_DataWrite(temp);  
	  
	temp=Y;   
    LCD_CmdWrite(0x9b);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x9c);	   
	LCD_DataWrite(temp);

	temp=R;   
    LCD_CmdWrite(0x9d);
	LCD_DataWrite(temp);
	
	LCD_CmdWrite(0x90);
	LCD_DataWrite(0x40);
} 


///////////drawing circle
void  Draw_Solid_Circle(u16 X,u16 Y,u16 R,u8 setR,u8 setG,u8 setB)
{
	u8 temp;
	
	Text_Foreground_Color(setR,setG,setB);
    
	temp=X;   
    LCD_CmdWrite(0x99);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x9a);	   
	LCD_DataWrite(temp);  
	  
	temp=Y;   
    LCD_CmdWrite(0x9b);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x9c);	   
	LCD_DataWrite(temp);

	temp=R;   
    LCD_CmdWrite(0x9d);
	LCD_DataWrite(temp);
	
	LCD_CmdWrite(0x90);
	LCD_DataWrite(0x60);
} 

///////////drawing circle
void  Draw_Solid_Circle_With_String(u16 X,u16 Y,u16 R,u8 setR,u8 setG,u8 setB,u8 *str,u8 set,u8 setRs,u8 setGs,u8 setBs)
{
	u8 temp;
	
	Text_Foreground_Color(setR,setG,setB);
    
	temp=X;   
    LCD_CmdWrite(0x99);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0x9a);	   
	LCD_DataWrite(temp);  
	  
	temp=Y;   
    LCD_CmdWrite(0x9b);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0x9c);	   
	LCD_DataWrite(temp);

	temp=R;   
    LCD_CmdWrite(0x9d);
	LCD_DataWrite(temp);
	
	LCD_CmdWrite(0x90);
	LCD_DataWrite(0x60);
	
	Text(X-R*0.7,X+R*0.7 ,Y-R*0.7,Y+R*0.7,str,set,setRs,setGs,setBs);
	
// 	Text_Foreground_Color(setRs,setGs,setBs);
// 	Active_Window(X-R*0.707,X+R*0.707,Y-R*0.707,Y+R*0.707);
// 	FontWrite_Position(X-R*0.707,Y-R*0.707);

// 	Write_Dir(0x22,set);
// 	String(str);
// 	
// 	Active_Window(0,799,0,479);
} 

///////////drawing elliptic curve
void  Draw_Ellipse(u16 X,u16 Y,u16 R1,u16 R2)
{
	u8 temp;    
	temp=X;   
    LCD_CmdWrite(0xA5);
	LCD_DataWrite(temp);
	temp=X>>8;   
    LCD_CmdWrite(0xA6);	   
	LCD_DataWrite(temp);  
	  
	temp=Y;   
    LCD_CmdWrite(0xA7);
	LCD_DataWrite(temp);
	temp=Y>>8;   
    LCD_CmdWrite(0xA8);	   
	LCD_DataWrite(temp);

	temp=R1;   
    LCD_CmdWrite(0xA1);
	LCD_DataWrite(temp);
	temp=R1>>8;   
    LCD_CmdWrite(0xA2);	   
	LCD_DataWrite(temp);  
	  
	temp=R2;   
    LCD_CmdWrite(0xA3);
	LCD_DataWrite(temp);
	temp=R2>>8;   
    LCD_CmdWrite(0xA4);	   
	LCD_DataWrite(temp);
	
		LCD_CmdWrite(0xA0);
	LCD_DataWrite(0x80);
} 

///////////drawing elliptic curve
void  Draw_Circle_Square(u16 XS,u16 XE,u16 YS,u16 YE,u16 R1,u16 R2)
{
	u8 temp;       
	temp=XS;   
    LCD_CmdWrite(0x91);
	LCD_DataWrite(temp);
	temp=XS>>8;   
    LCD_CmdWrite(0x92);	   
	LCD_DataWrite(temp);

	temp=XE;
    LCD_CmdWrite(0x95);
	LCD_DataWrite(temp);
	temp=XE>>8;   
    LCD_CmdWrite(0x96);	   
	LCD_DataWrite(temp);

	temp=YS;   
    LCD_CmdWrite(0x93);
	LCD_DataWrite(temp);
	temp=YS>>8;   
    LCD_CmdWrite(0x94);	   
	LCD_DataWrite(temp);

	temp=YE;   
    LCD_CmdWrite(0x97);
	LCD_DataWrite(temp);
	temp=YE>>8;   
    LCD_CmdWrite(0x98);	   
	LCD_DataWrite(temp);

	temp=R1;   
    LCD_CmdWrite(0xA1);
	LCD_DataWrite(temp);
	temp=R1>>8;   
    LCD_CmdWrite(0xA2);	   
	LCD_DataWrite(temp);  
	  
	temp=R2;   
    LCD_CmdWrite(0xA3);
	LCD_DataWrite(temp);
	temp=R2>>8;   
    LCD_CmdWrite(0xA4);	   
	LCD_DataWrite(temp);
	
		LCD_CmdWrite(0xA0);
	LCD_DataWrite(0xA0);
} 

///////////drawing elliptic curve
void  Draw_Solid_Circle_Square(u16 XS,u16 XE,u16 YS,u16 YE,u16 R1,u16 R2,u8 setR,u8 setG,u8 setB)
{
	u8 temp;       
	
	Text_Foreground_Color(setR,setG,setB);
	
	temp=XS;   
    LCD_CmdWrite(0x91);
	LCD_DataWrite(temp);
	temp=XS>>8;   
    LCD_CmdWrite(0x92);	   
	LCD_DataWrite(temp);

	temp=XE;
    LCD_CmdWrite(0x95);
	LCD_DataWrite(temp);
	temp=XE>>8;   
    LCD_CmdWrite(0x96);	   
	LCD_DataWrite(temp);

	temp=YS;   
    LCD_CmdWrite(0x93);
	LCD_DataWrite(temp);
	temp=YS>>8;   
    LCD_CmdWrite(0x94);	   
	LCD_DataWrite(temp);

	temp=YE;   
    LCD_CmdWrite(0x97);
	LCD_DataWrite(temp);
	temp=YE>>8;   
    LCD_CmdWrite(0x98);	   
	LCD_DataWrite(temp);

	temp=R1;   
    LCD_CmdWrite(0xA1);
	LCD_DataWrite(temp);
	temp=R1>>8;   
    LCD_CmdWrite(0xA2);	   
	LCD_DataWrite(temp);  
	  
	temp=R2;   
    LCD_CmdWrite(0xA3);
	LCD_DataWrite(temp);
	temp=R2>>8;   
    LCD_CmdWrite(0xA4);	   
	LCD_DataWrite(temp);
	
		LCD_CmdWrite(0xA0);
	LCD_DataWrite(0xE0);
	

} 

///////////drawing elliptic curve
void  Draw_Solid_Circle_Square_With_String(u16 XS,u16 XE,u16 YS,u16 YE,u16 R1,u16 R2,u8 setR,u8 setG,u8 setB,u8 *str,u8 set,u8 setRs,u8 setGs,u8 setBs)
{
	u8 temp;       
	
	Text_Foreground_Color(setR,setG,setB);
	
	
	temp=XS;   
    LCD_CmdWrite(0x91);
	LCD_DataWrite(temp);
	temp=XS>>8;   
    LCD_CmdWrite(0x92);	   
	LCD_DataWrite(temp);

	temp=XE;
    LCD_CmdWrite(0x95);
	LCD_DataWrite(temp);
	temp=XE>>8;   
    LCD_CmdWrite(0x96);	   
	LCD_DataWrite(temp);

	temp=YS;   
    LCD_CmdWrite(0x93);
	LCD_DataWrite(temp);
	temp=YS>>8;   
    LCD_CmdWrite(0x94);	   
	LCD_DataWrite(temp);

	temp=YE;   
    LCD_CmdWrite(0x97);
	LCD_DataWrite(temp);
	temp=YE>>8;   
    LCD_CmdWrite(0x98);	   
	LCD_DataWrite(temp);

	temp=R1;   
    LCD_CmdWrite(0xA1);
	LCD_DataWrite(temp);
	temp=R1>>8;   
    LCD_CmdWrite(0xA2);	   
	LCD_DataWrite(temp);  
	  
	temp=R2;   
    LCD_CmdWrite(0xA3);
	LCD_DataWrite(temp);
	temp=R2>>8;   
    LCD_CmdWrite(0xA4);	   
	LCD_DataWrite(temp);
	
		LCD_CmdWrite(0xA0);
	LCD_DataWrite(0xE0);
	
// 	Text(XS+R1,XE-R1 ,YS+R2,YE-R2,str,set,setRs,setGs,setBs);	
	Active_Window(XS+R1,XE-R1 ,YS+R2 ,YE-R2);
// 	Write_Dir(0X8E,0XC0);//Began to clear the screen (Active window)
	Write_Dir(0x22,set);
	Text_Foreground_Color(setRs,setGs,setBs);
	FontWrite_Position(XS+R1,YS+R2);
	
  Write_Dir(0x40,0x80);//Set the character mode
	LCD_CmdWrite(0x02);
	while(*str != '\0')
	{
	 LCD_DataWrite(*str);
	 ++str;	 	
// 	 Chk_Busy();		
	} 
	Active_Window(0,799 ,0 ,479);

	
} 

///////////drawing line, rectangle, triangle
void Draw_Line(u16 XS,u16 XE ,u16 YS,u16 YE)
{	
    u8 temp;    
	temp=XS;   
    LCD_CmdWrite(0x91);
	LCD_DataWrite(temp);
	temp=XS>>8;   
    LCD_CmdWrite(0x92);	   
	LCD_DataWrite(temp);

	temp=XE;
    LCD_CmdWrite(0x95);
	LCD_DataWrite(temp);
	temp=XE>>8;   
    LCD_CmdWrite(0x96);	   
	LCD_DataWrite(temp);

	temp=YS;   
    LCD_CmdWrite(0x93);
	LCD_DataWrite(temp);
	temp=YS>>8;   
    LCD_CmdWrite(0x94);	   
	LCD_DataWrite(temp);

	temp=YE;   
    LCD_CmdWrite(0x97);
	LCD_DataWrite(temp);
	temp=YE>>8;   
    LCD_CmdWrite(0x98);	   
	LCD_DataWrite(temp);
	
			LCD_CmdWrite(0x90);
	LCD_DataWrite(0x80);
}

////////////draw a triangle of three point 
void Draw_Triangle(u16 X3,u16 Y3)
{
    u8 temp;    
	temp=X3;   
    LCD_CmdWrite(0xA9);
	LCD_DataWrite(temp);
	temp=X3>>8;   
    LCD_CmdWrite(0xAA);	   
	LCD_DataWrite(temp);

	temp=Y3;
    LCD_CmdWrite(0xAB);
	LCD_DataWrite(temp);
	temp=Y3>>8;   
    LCD_CmdWrite(0xAC);	   
	LCD_DataWrite(temp);
}




///////////////The FLASH reading area   setting
void DMA_block_mode_size_setting(u16 BWR,u16 BHR,u16 SPWR)
{
  	LCD_CmdWrite(0xB4);
  	LCD_DataWrite(BWR);
  	LCD_CmdWrite(0xB5);
  	LCD_DataWrite(BWR>>8);

  	LCD_CmdWrite(0xB6);
  	LCD_DataWrite(BHR);
  	LCD_CmdWrite(0xB7);
  	LCD_DataWrite(BHR>>8);

  	LCD_CmdWrite(0xB8);
  	LCD_DataWrite(SPWR);
  	LCD_CmdWrite(0xB9);
  	LCD_DataWrite(SPWR>>8);  
}

/////////////FLASH read start position Settings
void DMA_Start_address_setting(u32 set_address)
{ 
  	LCD_CmdWrite(0xB0);
  	LCD_DataWrite(set_address);

  	LCD_CmdWrite(0xB1);
  	LCD_DataWrite(set_address>>8);

	LCD_CmdWrite(0xB2);
  	LCD_DataWrite(set_address>>16);

  	LCD_CmdWrite(0xB3);
  	LCD_DataWrite(set_address>>24);
}


////////////Show the picture of the flash
void Displaypicture(u8 picnum)
{  
   u8 picnumtemp;
   Write_Dir(0X06,0X00);//FLASH frequency setting
   Write_Dir(0X05,0X87);//FLASH setting 

	picnumtemp=picnum;

   Write_Dir(0XBF,0X02);//FLASH setting
   Active_Window(0,799,0,479); 
   MemoryWrite_Position(0,0);//Memory write position
   DMA_Start_address_setting(768000*(picnumtemp-1));//DMA Start address setting
   DMA_block_mode_size_setting(800,480,800);
   Write_Dir(0XBF,0X03);//FLASH setting
	Chk_DMA_Busy();
} 

/***************************************
// ¼¸¸öÖ÷Òª½çÃæ
*****************************************/

 void interface0(void)
{
	Active_Window(0,799,0,479);//Set the working window size

	Text_Foreground_Color1(color_white);//Set the foreground color
	Text_Background_Color1(color_blue);//Set the background color
	
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	
	Text(110,890 ,121,190," C-CL300 ÐÍ¼¤¹âÇåÏ´Éè±¸",0x4a,28,46,0); 	
	Draw_Solid_Circle_Square_With_String(330,470,280,320,5,5,23,28,6,"µã»÷¼ÌÐø",0x45,0,0,0);
	Text(160,640 ,380,440,"ÎäººÊÐ¿­ÈðµÏ¼¤¹â¼¼ÊõÓÐÏÞ¹«Ë¾",0x45,28,46,0); 	
// 	Text(250,550 ,420,440,"Wuhan Create Laser Technology Co.,LTD",0x40,28,46,0); 
	
}

 void interface1(u8 parameter_tab[5][5],u16 state,u16 action)
{
	Active_Window(0,799,0,479);//Set the working window size
	
// 	Text_Foreground_Color(22,8,4);//Set the foreground color
// 	delay_ms(1);
// 	Text_Background_Color(11,33,24);//Set the background color
// 	delay_ms(1);
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	delay_ms(10);
	
	Text(216,584 ,8,40,"C-CL300 ÐÍ¼¤¹âÇåÏ´Éè±¸",0x05,28,46,0);
	
  Draw_Circle_Square(0,799,53,147,10,10);
	Draw_Solid_Circle_Square_With_String(5,75,64,136,3,3,23,33,1,"×´Ì¬Ö¸Ê¾",0x45,30,63,31);
	
	if(state & 0x8000)
	{
		Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"ÀäÈ´Õý³£",0x45,31,61,29);
	}
	else
	{
		Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"ÀäÈ´¹ÊÕÏ",0x45,31,61,29);
	}
	
  	if(state & 0x4000)
	{
		Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"ÆøÑ¹Õý³£",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"ÆøÑ¹ µÍ ",0x45,31,61,29);
	}
	
  	if(state & 0x2000)
	{
		Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"µ÷ QÕý³£",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"µ÷ Q¹ÊÕÏ",0x45,31,61,29);
	}
	
	

	
// 	  if(state==0xffff)
// 	{
// 	  Draw_Solid_Circle_Square_With_String(325,395,65,135,3,2,0,32,0,"ÏµÍ³Õý³£",0x45,31,61,29);		
// 	}
// 	else
// 	{
// 		Draw_Solid_Circle_Square_With_String(325,395,65,135,3,2,31,0,0,"ÏµÍ³¹ÊÕÏ",0x45,31,61,29);
// 	}

	Draw_Circle_Square(0,799,153,247,10,10);
	Draw_Solid_Circle_Square_With_String(5,75,164,236,3,3,23,33,1,"²ÎÊýÉèÖÃ",0x45,30,63,31);
	Text(90,799 ,160 ,195,"¼¤¹âµçÁ÷ ¼¤¹âÆµÂÊ É¨Ãè¼ä¾à É¨Ãè·ùÃæ É¨ÃèÆµÂÊ",0x45,31,61,29);
	Text(90,799 ,205 ,240,"       A      kHz       mm       mm      kHz",0x45,31,61,29);
	
	Text(90,202 ,205 ,240,parameter_tab[0],0x45,31,61,29);
	Text(234,314 ,205 ,240,parameter_tab[1],0x45,31,61,29);
	Text(378,474 ,205 ,240,parameter_tab[2],0x45,31,61,29);
	Text(522,618 ,205 ,240,parameter_tab[3],0x45,31,61,29);
	Text(666,746 ,205 ,240,parameter_tab[4],0x45,31,61,29);
	
	
	Draw_Line(85,795 ,155,155);
	Draw_Line(85,795 ,200,200);
	Draw_Line(85,795 ,245,245);
	
	Draw_Line(85,85 ,155,245);
	Draw_Line(226,226 ,155,245);
	Draw_Line(370,370 ,155,245);
	Draw_Line(514,514,155,245);
	Draw_Line(658,658,155,245);
	Draw_Line(795,795,155,245);
	

	Draw_Circle_Square(0,799,253,347,10,10);
	Draw_Solid_Circle_Square_With_String(5,75,260,340,3,7,23,33,1,"²Ù×÷½çÃæ",0x45,30,63,31);
	
	if(action & 0x8000)
	{
	  Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"ÖÆÀäÍ£Ö¹",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,31,0,0,"ÖÆÀäÆô¶¯",0x45,31,61,29);
	}
	
	if(action & 0x4000)
	{
		Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"µ÷ QÍ£Ö¹",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,31,0,0,"µ÷ QÆô¶¯",0x45,31,61,29);
	}
	
	if(action & 0x2000)
	{
	  Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"´µÆøÊ§ÄÜ",0x45,31,61,29);
	}
	else
	{
  	Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,31,0,0,"´µÆøÊ¹ÄÜ",0x45,31,61,29);
	}

	if(action & 0x1000)
	{

		Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"É¨ÃèÍ£Ö¹",0x45,31,61,29);
	}
	else
	{
    Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,31,0,0,"É¨ÃèÆô¶¯",0x45,31,61,29);
	}

		if(action & 0x0800)
	{
		Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"¼¤¹âÍ£Ö¹",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"¼¤¹âÆô¶¯",0x45,31,61,29);
	}
	
	if(action & 0x0400)
	{
    Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"¹âÕ¢  ¹Ø",0x45,31,61,29);
	}
	else
	{
	  Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"¹âÕ¢  ¿ª",0x45,31,61,29);
	}
	
	if(action & 0x0200)
	{
		Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"³éÆøÍ£Ö¹",0x45,31,61,29);
	}
	else 
	{
		Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"³éÆøÆô¶¯",0x45,31,61,29);
	}
	
	if(action & 0x0100)
	{
		Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,0,32,0,"×Ô¶¯Í£Ö¹",0x45,31,61,29);
	}
	else 
	{
	  Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,31,0,0,"×Ô¶¯ÔËÐÐ",0x45,31,61,29);
	}
	
	Draw_Solid_Circle_Square_With_String(660,744,258,342,9,9,12,22,10,"ÏµÍ³¸´Î»",0x45,31,61,29);


	 		if( state==0xffff )
		{
			Draw_Solid_Circle_Square(100,700,405,455,10,10,0,32,0);
		}
		else
		{
			Draw_Solid_Circle_Square(100,700,405,455,10,10,31,0,0);
		}
		Draw_Circle_Square(99,701,404,456,10,10);

		Draw_Solid_Circle_Square_With_String(5,85,395,475,8,7,20,20,5,"±¨¾¯ÐÅÏ¢",0x45,31,47,0);
		Draw_Solid_Circle_Square_With_String(715,795,395,475,8,7,12,22,10,"¹ÊÕÏ¸´Î»",0x45,31,61,29);

		Text(288,512 ,460,479,"ÎäººÊÐ¿­ÈðµÏ¼¤¹â¼¼ÊõÓÐÏÞ¹«Ë¾",0x00,28,46,0);
}


//±¨¾¯ÐÅÏ¢
 void interface2(u8 user_manual[21][50],u32 alarm)
{
	u16 i,j=0;
	Active_Window(0,799,0,479);//Set the working window size
	
	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
	
	delay_ms(10);
	
	Text(288,512,18,50,"±¨  ¾¯  ÐÅ  Ï¢",0x05,28,46,0);
	Draw_Circle_Square(0,799,55,425,10,10);
	
// 	alarm=0xffff;
	
	for(i=0;i<9;i++)
	{
		if(alarm & 0x0001)
		{
			Text(2,798,60+40*j,100+40*j,user_manual[i],0x05,28,46,0);
			j++;
		}
		alarm=alarm>>1;
	}
// 	
// 	Text(2,798,60,100,user_manual[0],0x05,28,46,0);
// 	Text(2,798,100,140,user_manual[1],0x05,28,46,0);
// 	Text(2,798,140,180,user_manual[2],0x05,28,46,0);
// 	Text(2,798,180,220,user_manual[3],0x05,28,46,0);
// 	Text(2,798,220,260,user_manual[4],0x05,28,46,0);
// 	Text(2,798,260,300,user_manual[5],0x05,28,46,0);
// 	Text(2,798,300,340,user_manual[6],0x05,28,46,0);
// 	Text(2,798,340,380,user_manual[7],0x05,28,46,0);
// 	Text(2,798,380,420,user_manual[8],0x05,28,46,0);
// 	
// 	Draw_Solid_Circle_Square_With_String(240,340,430,470,2,4,31,63,31,"ÉÏÒ»Ò³",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(330,470,430,470,2,4,31,63,31,"±¨¾¯¸´Î»",0x45,0,0,0);
// 	Draw_Solid_Circle_Square_With_String(420,580,430,470,2,4,31,63,31,"ÏÂÒ»Ò³",0x45,0,0,0);

	Draw_Solid_Circle_Square_With_String(730,799,430,470,2,4,31,63,31,"·µ»Ø",0x45,0,0,0);
}

// //±¨¾¯
//  void interface3(void)
// {
// 	Active_Window(0,799,0,479);//Set the working window size
// 	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
// 		delay_ms(6);
// 	
// 	Text(288,512,18,50,"±¨  ¾¯  ¼Ç  Â¼",0x05,28,46,0);
// 	
// 	Draw_Solid_Circle_Square_With_String(330,470,430,470,6,4,31,63,31,"±¨¾¯Çå³ý",0x45,0,0,0);

// 	Draw_Solid_Circle_Square_With_String(730,799,430,470,2,4,31,63,31,"·µ»Ø",0x45,0,0,0);
// 	
// }

//c²ÎÊýµÈÊýÖµÊäÈë½çÃæ
void interface4()
{
	Active_Window(240,560,100,340);//Set the working window size
	
	Draw_Solid_Circle_Square(240,560,100,340,10,10,27,55,27);
	Text1(245,560,105,140,"²ÎÊýÊäÈë",0x45,28,46,0);
	
	Draw_Solid_Circle_Square(242,558,142,178,5,5,31,63,31);
	
	FontWrite_Position(555,145);
	Write_Dir(0x40,0xe0);
	
	Draw_Solid_Circle_Square_With_String(242,318,182,218,5,5,31,63,31,"  1",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(242,318,222,258,5,5,31,63,31,"  4",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(242,318,262,298,5,5,31,63,31,"  7",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(242,318,302,338,5,5,31,63,31,"  .",0x45,0,0,0);
	
	Draw_Solid_Circle_Square_With_String(322,398,182,218,5,5,31,63,31,"  2",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(322,398,222,258,5,5,31,63,31,"  5",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(322,398,262,298,5,5,31,63,31,"  8",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(322,398,302,338,5,5,31,63,31,"  0",0x45,0,0,0);
	
	Draw_Solid_Circle_Square_With_String(402,478,182,218,5,5,31,63,31,"  3",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(402,478,222,258,5,5,31,63,31,"  6",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(402,478,262,298,5,5,31,63,31,"  9",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(402,478,302,338,5,5,31,63,31,"   ",0x45,0,0,0);
	
	Draw_Solid_Circle_Square_With_String(482,558,182,218,5,5,31,63,31," clr",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(482,558,222,258,5,5,31,63,31," del",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(482,558,262,298,5,5,31,63,31," ok ",0x45,0,0,0);
	Draw_Solid_Circle_Square_With_String(482,558,302,338,5,5,31,63,31,"·µ»Ø",0x45,0,0,0);
	
	FontWrite_Position(242,555);
	Write_Dir(0x40,0xe0);
}

float Tab_To_Number(u8* input_number_tab)
{
	u8 m,dot_number=0,total_number=0,space_number=0;
	float number=0;
	for(m=0;m<4;m++)
	{
		if(input_number_tab[m]=='1')
		{
			number=number*10+1;
		}
		else if(input_number_tab[m]=='2')
		{
			number=number*10+2;
		}
		else if(input_number_tab[m]=='3')
		{
			number=number*10+3;
		}
		else if(input_number_tab[m]=='4')
		{
			number=number*10+4;
		}
		else if(input_number_tab[m]=='5')
		{
			number=number*10+5;
		}
		else if(input_number_tab[m]=='6')
		{
			number=number*10+6;
		}
		else if(input_number_tab[m]=='7')
		{
			number=number*10+7;
		}
		else if(input_number_tab[m]=='8')
		{
			number=number*10+8;
		}
		else if(input_number_tab[m]=='9')
		{
			number=number*10+9;
		}
		else if(input_number_tab[m]=='0' )
		{
			number=number*10+0;
		}
		else if(input_number_tab[m]==' ')
		{
			space_number=space_number+1;
			number=number*10+0;
		}
// 		else if(input_number_tab[m]=='/0')
// 		{
// 			dot_number=4;
// 		}
		
		
		else if(input_number_tab[m]=='.')
		{
			dot_number=4-m;
		}
		else
			break;
	}
	
	if(dot_number==0)
	{
		number=number/pow(10,space_number);
	}
	else
	{
		number=number/pow(10,dot_number-1);
	}
	return number;
}


