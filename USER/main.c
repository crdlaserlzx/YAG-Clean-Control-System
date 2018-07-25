 #include <string.h>
 #include <stdlib.h>
 #include "sys.h"
 #include "delay.h"
 #include "LCD.h"
 #include "exti.h"
 #include "scan.h"
 #include "AD5722R.h"
 #include "RA8875.h"
 #include "FlashStorage.h"
 #include <math.h>
 #include "wdg.h"
 #include "timer.h"
 #include "control_gpio.h"
 
//  #include "stdbool.h"

/*20170713修改：因为硬件暂时没法改，将急停作为改变扫描幅面宽度的信号使用
改动：1、急停中断去掉，状态栏对急停的反应去掉。
     2、增加变量i，1-4循环变化，用于改变幅面。
     3、手动自动启动将无法使用。
     4、control_gpio.c中将外部中断屏蔽了。
 */
 
 /*20170803修改：报警信息不在主界面显示
改动：1、去掉操作说明界面，改为报警信息界面
     2、增加变量alarm，指示存在某一天报警信息
		 3、增加变量alarm_string,用于存储具体报警信息
 */
 
//////////////////////////////////////////////////////////////////////////////////	 
//激光清洗控制程序
//修改日期:2015/9
//版本：V0.0
//版权所有，盗版必究。
//Copyright(C) 凯瑞迪激光 2015-2015
//All rights reserved
//********************************************************************************

u8 auto_run_time=0,refresh=0,touch_in=1,touch_flag=0,touch_time=0; 
u8 auto_run_step=0;   //自动运行的步骤记录
u8 auto_delay_time_array[6]={10,30,40,50,60,70};  //自动运行时，各个步骤的执行的延迟时间
                       //制冷->调Q->吹气->扫描->抽气->激光->->

u16 state=0xffff,state0;  //设备状态指示寄存器,1表示正常；bit15:水；bit14：气；bit13：调Q
                   //bit0表示急停按下,state0记录历史信息
u16 action=0x0000,action0; //设备动作寄存器，1表示有输出,action0记录历史信息
// u16 action_flag=0x0000; //设备动作标志寄存器
u32 alarm=0;
u8 alarm_string[21][50]={"#1激光电流正确设置范围为（0A，32A）！           ",
												 "#2激光频率正确设置范围为（0KHz，20KHz）！       ",
												 "#3扫描间距正确设置范围为（0.1mm，10mm）！       ",
												 "#4扫描幅面正确设置范围为（10mm，100mm）！       ",
												 "#5扫描频率正确设置范围为（1KHz，8KHz）！       ",
                         "#6冷却系统故障，请检查水路！                    ",
	                       "#7气压不正常，请检查气路和气源气压。            ",	        
	                       "#8调Q系统故障，请检查变频器。                   ",
                         "#9急停已按下！                                 "};

	
float parameter[5]={18.5,20,0.1,10.5,2.01};      // 参数值
float parameter_up[5]={32,20,10,100,8};   //参数值上限，振镜最快0.4ms跳一次
float parameter_down[5]={0,0,0.1,10,1}; //参数值下限

u8  parameter_tab[5][5]={"18.5","200 ", "0.1 ","10.5", "2.01"};
u16 parameter_number,interface_number;  
float parameter_done_float[5]; 
u16 parameter_done[5];    //系统参数经过变换直接用于程序的数值
  //parameter_done[0]：激光电流写入AD芯片的数值
  // parameter_done[1]：激光频率对应写入AD芯片的数值
  // parameter_done[2]：振镜每次跳动的距离转换为写入的16位数据值
  // parameter_done[3]：振镜扫描范围转换为16位数，范围为【0x8000-parameter_done[3],0x8000+parameter_done[3]】
  // parameter_done[4]：振镜每次跳动一次的时间间隔转化为计时器2触发次数。
u8 scan_direction=0; //0表示扫描数值减小，1增加
u16 scan_X_position=0x8000,scan_Y_position=0x8000;  //表示

u32 scan_time=0;  //振镜扫描的时间寄存器
u32 scan_time_max;

// //扫描相关的参数
// focal_length=150;  //聚焦透镜焦距

// 模拟电压输出相关参数
double voltage_A,voltage_B;
u8 i,j;

int main()
{
	u8 m;   //touch_in 表示触摸屏触控信号输入
	u16 X,Y;
	u16 temp;
	u32 store_temp[5];
	u8 input_number_tab[5]={"    "};
  float input_number;
	u8 tab_num=0;
	u8 dot_mark=0;

  i=1;

	
  System_Clock_Init();
  delay_init();
  Control_GPIO_Init();
	RA8875_GPIO_Init();
	RA8875_SPI_Init();
	LCD_Initial();
  Scan_GPIO_Init();
  Scan_Init(); //振镜扫描初始化
	TIM2_Int_Init(1,359);//100kHZ的计数频率，用于振镜扫描，相当于0.01ms触发一次
	TIM3_Int_Init(1999,35999);//2Khz的计数频率，用于自动运行，1s触发一次
	TIM4_Int_Init(999,35999);//2Khz的计数频率，用于触摸屏点击延时,1s触发一次
	
  AD5722R_GPIO_Init();
  AD5722R_soft_Init();
  AD5722R_write_data(0x080001);  //设置A通道范围 0~10V
	AD5722R_write_data(0x0a0001);  //设置B通道范围 0~10V
	
	
	Scan_AD_Write(scan_X_position,scan_Y_position); 

	out_cool=0;
	out_tiaoQ=0;
// 	out_cuiqi=0;
	out_guangza=0;
	out_couqi=0;
	out_zidong=0;
	out_laser=0;
	
	
// 	scan_cs=0; 
//         scan_sclk=1;
//         scan_sdix=0;
//         scan_sdiy=1;
	
	delay_ms(600);

	//read parameter from flash
	for(m=0;m<5;m++)
	{
		store_temp[m]=Flash_Read_data(m*4);
		parameter_tab[m][0]=0xff & store_temp[m]>>24;
		parameter_tab[m][1]=0xff & store_temp[m]>>16;
		parameter_tab[m][2]=0xff & store_temp[m]>>8;
		parameter_tab[m][3]=0xff & store_temp[m];
		parameter_tab[m][4]='\0';
	  parameter[m]=Tab_To_Number(parameter_tab[m]);   //将参数由字符串形式转换为float数值
	}
  	delay_ms(600);
// //对所有float形式的参数进行处理，并存入parameter_done数组中
// 	parameter_done[0]=parameter[0]*10/32;
// 	parameter_done[1]=parameter[1]*10/12;
// 	parameter_done[2]=parameter[2]*850*(150/60);  //f150时，500对应1mm
// 	parameter_done[3]=parameter[3]*850*(150/60); 
// 	parameter_done[4]=100000/parameter[4];
// 	
// 	temp=parameter[3]/parameter[2];
// 	scan_Y_position=0x8000-temp*parameter_done[2]/2;
// 	scan_direction=1;
// 	
// 	scan_time_max=100/parameter[4];
	
	//对所有float形式的参数进行处理，并存入parameter_done数组中
	parameter_done_float[0]=parameter[0]*5/16;
	parameter_done_float[1]=parameter[1]/2;
	parameter_done_float[2]=parameter[2]*650;  //f150时，850对应1mm
	parameter_done_float[3]=parameter[3]*650; 
	parameter_done_float[4]=100000/parameter[4];
	
	parameter_done[0]=parameter_done_float[0];
	parameter_done[1]=parameter_done_float[1];
	parameter_done[2]=parameter_done_float[2];
	parameter_done[3]=parameter_done_float[3];
	parameter_done[4]=parameter_done_float[4];
	
// 							temp=parameter[3]/parameter[2];
// 							scan_Y_position=0x8000-temp*parameter_done[2]/2;
	scan_direction=1;
	
	scan_time_max=100/parameter[4];
	
	voltage_A=parameter[0]*5/16;
	voltage_B=parameter[1]/2;
	
	AD5722R_write_vol(voltage_A,'A');
	AD5722R_write_vol(voltage_B,'B');
	
	
	delay_ms(600);
	
	interface0();
	interface_number=0;
	
// 	out_zidong=1;
	
// 	Control_GPIO_Init();
	
  IWDG_Init(IWDG_Prescaler_64,2500);    //与分频数为64,重载值为625,溢出时间为1s	
	
	voltage_A=parameter[0]*5/16;
	voltage_B=parameter[1]/2;
	
	AD5722R_write_vol(voltage_A,'A');
	AD5722R_write_vol(voltage_B,'B');

// 	AD5722R_write_vol(9,'A');
// 	AD5722R_write_vol(9.9,'B');
	
	
		while(1)
	{
		//喂狗 
	  IWDG_Feed();
		
		
		/*********************************************************
		//根据外部状态改变state寄存器
		**********************************************************/
		
// 	  state=0xffff;
		
		if(in_water)
		{
			state=state & 0x7fff;			
		}
		else
		{
			state=state | 0x8000;
		}
		
		if(in_gas)
		{
		  state=state & 0xbfff;
			alarm=alarm | 0x0040;
		}
		else
		{
			state=state | 0x4000;
		}
		
		if(in_tiaoQ)
		{
		  state=state & 0xdfff;
			alarm=alarm | 0x0080;
		}
		else
		{
			state=state | 0x2000;
		}
		
		if(in_jiting)
		{
			state=state & 0xfffe;
			alarm=alarm | 0x0200;
		}
		else
		{
			state=state | 0x0001;
		}
	
// //测试程序段		
// 		if(in_jiting)
// 		{
// 			delay_ms(10);
// 			if(in_jiting)
// 			{
// 				i++;
// 				if(i>4)
// 				{
// 					i=1;
// 				}
// 				;
// 			}
// 		}
// //测试程序段	
		
		if(in_guangzha==0  &&  ((action & 0x0400)==0x0400))
		{
			out_guangza=1;
 			out_cuiqi=1;   
			refresh=1;
		}
		else
		{
			out_guangza=0;
			out_cuiqi=0;
		}
		
		
		/*********************************************************
		//检测状态 和 动作有没有发生变化，如果有变化则更新显示
		**********************************************************/
		
		if(state0!=state && interface_number==1)
		{
			if( (state & 0x8000) != (state0 & 0x8000) )
			{
				if(state & 0x8000)
				{
					Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"冷却正常",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"冷却故障",0x45,31,61,29);
				}	
			}	

			if( (state & 0x4000) != (state0 & 0x4000) )
			{
	      if(state & 0x4000)
				{
					Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"气压正常",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"气压 低 ",0x45,31,61,29);
				}								
			}
			
			if( (state & 0x2000) != (state0 & 0x2000) )	
			{
				if(state & 0x2000)
				{
					Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"调 Q正常",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"调 Q故障",0x45,31,61,29);
				}
			}		
			
			state0=state;
		}	

    if(action0!=action && interface_number==1)		
		{
			if( (action & 0x8000) != (action0 & 0x8000) )
			{
				if(action & 0x8000)
				{
					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"制冷停止",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,31,0,0,"制冷启动",0x45,31,61,29);
				}	
			}
			
			if( (action & 0x4000) != (action0 & 0x4000) )
			{
				if(action & 0x4000)
				{
					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"调 Q停止",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,31,0,0,"调 Q启动",0x45,31,61,29);
				}		
			}
			
			if( (action & 0x2000) != (action0 & 0x2000) )
			{
				if(action & 0x2000)
				{
					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"吹气失能",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,31,0,0,"吹气使能",0x45,31,61,29);
				}			
			}
			
			if( (action & 0x1000) != (action0 & 0x1000) )
			{
				if(action & 0x1000)
				{

					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"扫描停止",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,31,0,0,"扫描启动",0x45,31,61,29);
				}								
			}
			
			if( (action & 0x0800) != (action0 & 0x0800) )
			{
				if(action & 0x0800)
				{
					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"激光停止",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"激光启动",0x45,31,61,29);
				}						
			}
			
			if( (action & 0x0400) != (action0 & 0x0400) )
			{
				if(action & 0x0400)
				{
					Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"光闸失能",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"光闸使能",0x45,31,61,29);
				}							
			}
			
			if( (action & 0x0200) != (action0 & 0x0200) )
			{
				if(action & 0x0200)
				{
					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"抽气停止",0x45,31,61,29);
				}
				else 
				{
					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"抽气启动",0x45,31,61,29);
				}							
			}
			
			if( (action & 0x0100) != (action0 & 0x0100) )
			{
				if(action & 0x0100)
				{
					Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,0,32,0,"自动停止",0x45,31,61,29);
				}
				else 
				{
					Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,31,0,0,"自动运行",0x45,31,61,29);
				}							
			}
			
			action0=action;
		}
				
		/*********************************************************
		//外部自动运行
		**********************************************************/	
		
		if(in_zidong==0 && in_jiting==0)
		{
			delay_ms(10);
			if(in_zidong==0 && in_jiting==0)
			{
				if(action & 0x0100)  //如果已经在自动运行
				{
					action=action & 0xb2ff;			
					out_tiaoQ=0;					
					out_guangza=0;							
					out_zidong=0;
					out_laser=0;
					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
					auto_run_time=0;
					auto_run_step=0;
					
				}
				else if( (action & 0x0c00)==0 )
				{
					action=action | 0x0100;
					out_zidong=1;
												
					TIM_Cmd(TIM3, ENABLE);  //使能TIMx	
          auto_run_step=0;
					auto_run_time=0;
				}
			}
		}
		
		/**********************************************************
		//检查设备状态，水、气、调Q、急停、光闸，并进行相应操作
		**********************************************************/	
		
		if( (state & 0x8000)==0 &&  interface_number==1 )   //水报警
		{
			// 			Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,31,0,0,alarm_string[0],0x40,31,61,29);
			alarm=alarm | 0x0020;
			if( (action & 0x4d00)!=0 )
			{
				action=action & 0xb3ff; 
				out_tiaoQ=0;
				out_guangza=0;
				out_laser=0;
			}
// 			delay_ms(1000);
		}
		
		if( (state & 0x4000)==0 && interface_number==1)   //气报警
		{
// 			Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,31,0,0,alarm_string[1],0x40,31,61,29);
			alarm=alarm | 0x0040;
			if(  (action & 0xcd00)!=0 )
			{
				action=action & 0x32ff;		
				out_cool=0;
				out_tiaoQ=0;
				out_laser=0;
				out_guangza=0;
				out_zidong=0;
        refresh=1;
				TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
			}
// 			delay_ms(1000);
		}		
		
		if( (state & 0x2000)==0  && interface_number==1)   //调制报警
		{
// 			Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,31,0,0,alarm_string[2],0x40,31,61,29);
			alarm=alarm | 0x0080;
			if( (action & 0x4d00)!=0 )
			{
				action=action & 0xb2ff;
				out_tiaoQ=0;
				out_guangza=0;
				out_zidong=0;
				out_laser=0;
				TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
			}
// 			delay_ms(1000);
		}
		
		if( (state & 0x0001)==0 && interface_number==1 )  //急停
		{
// 			Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,31,0,0,alarm_string[3],0x40,31,61,29);
			alarm=alarm | 0x0200;
			action=action & 0x00ff;
			out_cool=0;
			out_tiaoQ=0;
			out_cuiqi=0;
			out_guangza=0;
			out_couqi=0;
			out_zidong=0;
			out_laser=0;
			Scan_AD_Write(0x8000,0x8000);
			TIM_Cmd(TIM2,DISABLE);  //失能TIMx	
// 							Scan_Open();
			Scan_Close();
		}	
		
		/**********************************************************
		//自动运行程序
		**********************************************************/		
		
		if( action & 0x0100 )
		{
			switch( auto_run_step)
			{
				case 0:
				if( (action & 0xb2000)!=0xb200)
				{
					action=action | 0xb200;
					out_cool=1;
// 					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"制冷停止",0x45,31,61,29);	
// 					
// 					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"吹气失能",0x45,31,61,29);
					
					TIM_Cmd(TIM2,ENABLE);  //使能TIMx
					scan_time=0;
					Scan_Open();
// 					Scan_Close();
// 					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"扫描停止",0x45,31,61,29);
					
					out_couqi=1;
// 					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"抽气停止",0x45,31,61,29);
					break;
				}	
				else
				{
					break;
				}
				break;
				
			  case 1:
        if( (action & 0xb200)==0xb200  &&  state==0xffff)
				{
					action=action | 0x4800;
					out_tiaoQ=1;
// 					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"调 Q停止",0x45,31,61,29);
					
					out_laser=1;
// 					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"激光停止",0x45,31,61,29);
					
					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
					auto_run_time=0;
					auto_run_step=0;
				}
				else
				{
					action=action & 0xfeff;							
					out_zidong=0;
					
					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
					auto_run_time=0;
					auto_run_step=0;
					refresh=1;
					break;			
				}
				break;
			}
		}
		
		/*************************************
			报警界面刷新
		**************************************/
		
		if( (refresh==1) && (interface_number==2) )
		{
			interface2(alarm_string,alarm);
			refresh=0;
		}
		
		/*************************************
			主界面刷新
		**************************************/
		if( (refresh==1) && (interface_number==1) )
		{
      refresh=0;
			if(alarm)
			{
			   Draw_Solid_Circle_Square(100,700,405,455,10,10,31,0,0);
			}
			else
			{
				Draw_Solid_Circle_Square(100,700,405,455,10,10,0,32,0);
			}
		}
		
		/**********************************************************
		// 有触摸   ,清除触摸标志位可能一次不成功，需要多次操作
		**********************************************************/
						
		if(Touch_Status() && touch_in)  
		{
			touch_in=0;
			TIM_Cmd(TIM4,ENABLE);
			X=TP_X_Coordinate();
			Y=TP_Y_Coordinate();
			touch_flag=1;
			while(Touch_Status())
			{
				Write_Dir(0xf1,0x04);
			}
		}
		
		
		if(touch_flag)  // 有触摸，并且急停未按下&& in_jiting==0 
		{
			refresh=1;
// 			X=TP_X_Coordinate();
// 			Y=TP_Y_Coordinate();
			touch_flag=0;

			switch(interface_number) //判断在那个画面上
			{
				/*************************************
				欢迎界面
				**************************************/	
				case 0:   
					if(X>345 && X<500 && Y>280 && Y<320)
					{
						interface_number=1;
						interface1(parameter_tab,state,action);
					}
					else
						break;
					break;
					
				/*************************************
				主界面
				**************************************/
					
				case 1:
// 				//测试程序段，在调试程序时用
// 					if(X>85 && X<155 && Y>65 && Y<135)
// 					{
// 						if(state & 0x8000)
// 	          {
// 							state =state & 0x7ffe;
// 							Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"冷却故障",0x45,31,61,29);
// 	          }
// 	          else
// 	          {
// 							state =state | 0x8000;
// 		          Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"冷却正常",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
// 					else if(X>165 && X<235 && Y>65 && Y<135)
// 					{
// 						if(state & 0x4000)
// 	          {
// 							state =state & 0xbffe;
// 							Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"气压 低 ",0x45,31,61,29);
// 		          
// 	          }
// 	          else
// 	          {
// 							state =state | 0x4000;
// 	            Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"气压正常",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
// 					else if(X>245 && X<315&&  Y>65 && Y<135)
// 					{
// 						if(state & 0x2000)
// 	          {
// 		          state =state & 0xdffe;
// 							Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"调 Q故障",0x45,31,61,29);
// 							
// 	          }
// 	          else
// 	          {
// 							state =state | 0x2000;
// 	            Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"调 Q正常",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
				
			  //正式程序				
				  if(X>85 && X<226 && Y>205 && Y<245 && ( (action & 0x0800) == 0) ) //参数1
					{
// 						FontWrite_Position(226,205)；
// 						Write_Dir(0x40,0xe0);
						parameter_number=0;
						interface_number=4;
						interface4();
					}
					else if(X>226 && X<370 && Y>205 && Y<245)  //参数2
					{
						parameter_number=1;
						interface_number=4;
						interface4();
					}
					else if(X>370 && X<514 && Y>205 && Y<245)  //参数3
					{
						parameter_number=2;
						interface_number=4;
						interface4();
					}
					else if(X>514 && X<658 && Y>205 && Y<245)  //参数4
					{ 
						parameter_number=3;
						interface_number=4;
						interface4();
					}
					else if(X>658 && X<799 && Y>205 && Y<245)  //参数5
					{
						parameter_number=4;
						interface_number=4;
						interface4();
					}
					else if(X>80 && X<220 && Y>258 && Y<298 ) //制冷  80,220,258,298
					{
						if(action & 0x8000)
						{
							action=action & 0x32ff;
							out_cool=0;
							out_tiaoQ=0;
							out_guangza=0;
							out_zidong=0;
							out_laser=0;
// 							refresh=1;
				      TIM_Cmd(TIM3,DISABLE);  //失能TIMx							
						}
						else if( (state & 0x4001)==0x4001)
						{
							action=action | 0x8000;
						  out_cool=1;					
// 							Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"制冷停止",0x45,31,61,29);
						}
// 					delay_ms(100);
					}
					else if(X>80 && X<220 && Y>302 && Y<342) //调Q  80,220,302,342
					{
						if((action & 0x4000)==0x4000)
						{
							action=action & 0xb2ff;
							out_tiaoQ=0;			
							out_guangza=0;
							out_zidong=0;
							out_laser=0;
				      TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
							
						}
						else if(((action & 0x8000)==0x8000) && ((state & 0xe001)==0xe001))
						{
							action=action | 0x4000;
							out_tiaoQ=1;
// 							Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"调 Q停止",0x45,31,61,29);
						}
// 					delay_ms(10);
					}
					
					else if(X>225 && X<365 && Y>258 && Y<298) //吹气  225,365,258,298
					{
						if(action & 0x2000)
						{
							action=action & 0xd2ff;
// 							out_cuiqi=0;
							out_guangza=0;
							out_zidong=0;
							out_laser=0;
							refresh=1;
				      TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
						}
						else if( (state & 0x0001)==0x0001 )
						{
							action=action | 0x2000;												
// 					    out_cuiqi=1;
// 							Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"吹气失能",0x45,31,61,29);
						}
// 					delay_ms(100);
					}
					else if(X>225 && X<365 &&  Y>302 && Y<342) //扫描  225,365,302,342
					{
						if(action & 0x1000)
						{
							action=action & 0xeaff;
					    out_guangza=0;
					    out_zidong=0;
							Scan_AD_Write(0x8000,0x8000);
				      TIM_Cmd(TIM2,DISABLE);  //失能TIMx	
// 							Scan_Open();
					    Scan_Close();
						}
						else   //if((state & 0x0001)==0x0001)
						{
							action=action | 0x1000;
// 							Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"扫描停止",0x45,31,61,29);
							TIM_Cmd(TIM2,ENABLE);  //使能TIMx
							scan_X_position=0x8000;
							scan_Y_position=0x8000;
							scan_time=0;
							Scan_Open();
// 					Scan_Close();
						}
// 					delay_ms(100);
					}					
					else if(X>370 && X<510 && Y>258 && Y<298) //激光   370,510,258,298
					{
						if(action & 0x0800)
						{
							action=action & 0xf7ff;
					    out_laser=0;
							//Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"激光启动",0x45,31,61,29);	
						}
						else if( ((action & 0xf000)==0xf000) && ((state & 0xffff)==0xffff) && ( (action | 0xfbff)==0xfbff ) )
						{
							action=action | 0x0800;
							out_laser=1;
							//Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"激光停止",0x45,31,61,29);
						}
         // delay_ms(100);
					}					
					
					else if(X>370 && X<510 &&  Y>302 && Y<342) //光闸   370,510,302,342
					{
						if(action & 0x0400)
						{
							action=action & 0xfbff; 
// 							out_guangza=0;
// 							Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"光闸 开 ",0x45,31,61,29);
						}
						else if((action & 0xd000)==0xd000 )    //(state & 0x0001)==0x0001
						{
							action=action | 0x0400;
// 							out_guangza=1;
// 							Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"光闸 关 ",0x45,31,61,29);
						}
        //	delay_ms(100);
					}
					
					else if(X>515 && X<655 && Y>258 && Y<298) //抽气  515,655,258,298
					{
						if(action & 0x0200)
						{
							action=action & 0xfdff;
							out_couqi=0;
// 							Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"抽气启动",0x45,31,61,29);
						}
						else if((state & 0x0001)==0x0001)
						{
							action=action | 0x0200;
							out_couqi=1;
// 							Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"抽气停止",0x45,31,61,29);
						}
// 					delay_ms(100);	
					}
					
					else if(X>515 && X<655 && Y>302 && Y<342) //自动运行 515,655,302,342
					{
						if(action & 0x0100)
						{
							action=action & 0xb2ff;						
							out_tiaoQ=0;					
							out_guangza=0;							
							out_zidong=0;
							out_laser=0;
							TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 							TIM_Cmd(TIM2,DISABLE);  //失能TIMx
// 							scan_X_position=0x8000;
// 							scan_Y_position=0x8000;
							auto_run_time=0;
              auto_run_step=0;
						}
						else if( (action & 0x0c00)==0 )
						{
							action=action | 0x0100;
							out_zidong=1;				
							auto_run_time=0;
							TIM_Cmd(TIM3, ENABLE);  //使能TIMx	
              auto_run_step=0;							
						}
				//	delay_ms(100);	
					}					
					
					else if(X>660 && X<744 && Y>258 && Y<342) //系统复位 660,744,258,342
					{
						action = action & 0x80ff; 				
						out_tiaoQ=0;
						out_guangza=0;
						out_couqi=0;
						out_zidong=0;
						out_laser=0;
						TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
						TIM_Cmd(TIM2,DISABLE);  //失能TIMx
						Scan_AD_Write(0x8000,0x8000);
// 					Scan_Open();
				  	Scan_Close();
					}		

					else if(X>0 && X<80 && Y>400 && Y<480)  //报警信息
					{
						interface_number=2;
						interface2(alarm_string,alarm);
					}
					else if(X>720 && X<800 && Y>400 && Y<480)  //故障复位
					{
            Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,0,32,0,"     ",0x40,31,61,29);
						alarm=0;
					//	refresh=1;
					}
					else
						break;
					break;
					
/*************************************
报警信息界面
**************************************/						
					
				case 2:
					if(X>720 && X<790 && Y>430 && Y<470) // 返回
					{
						interface_number=1;
						interface1(parameter_tab,state,action);
					}
// 					else if(X>240 && X<340 && Y>430 && Y<470) //"上一页"  240,340,430,470
// 					{

// 					}
					else if(X>350 && X<450 && Y>430 && Y<470) //"报警复位", 350,450,430,470
					{
						alarm=0;
						interface2(alarm_string,alarm);	
            refresh=1;						
					}
// 					else if(X>460 && X<560 && Y>430 && Y<470) //"上一页"  460,560,430,470
// 					{

// 					}
					else
						break;
					break;
					
/*************************************
参数输入界面
**************************************/					
					
				case 4:
					if(X>242 && X<318 && Y>182 && Y<218)  //数值1 242,318,182,218
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='1';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>182 && Y<218)//数值2  322,398,182,218
	        {   
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='2';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>402 && X<478 && Y>182 && Y<218)//数值3  402,478,182,218,
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='3';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>222 && Y<258)//数值4   242,318,222,258
	        {  
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='4';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>222 && Y<258)//数值5   322,398,222,258
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='5';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>402 && X<478 && Y>222 && Y<258)//数值6    402,478,222,258
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='6';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>262 && Y<298)//数值7   242,318,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='7';
						  tab_num++;
						}
						else
							break;				
	        }
	        else if(X>322 && X<398 && Y>262 && Y<298)//数值8   322,398,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='8';
						  tab_num++;
						}
						else
							break;				
	        }
	        else if(X>402 && X<478 && Y>262 && Y<298)//数值9   402,478,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='9';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>302 && Y<338)//数值0   322,398,302,338
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='0';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>302 && Y<338)//小数点.   242,318,302,338
	        {
						if(tab_num>0  &&  tab_num<4  &&  dot_mark==0)
						{
						    input_number_tab[tab_num]='.';
					  	  tab_num++;
					  		dot_mark=1;
            }
						else
						  	break;
	        }
					
	        else if(X>482 && X<558 && Y>182 && Y<218)     //clear   482,558,182,218
	        {
//             input_number_tab[10]={"          "};
						for(m=0;m<4;m++)
						{
							input_number_tab[m]=' ';
						}
						tab_num=0;
						dot_mark=0;
	        }
					
	        else if(X>482 && X<558 && Y>222 && Y<258)//delete   482,558,222,258
	        {			  
				    if(tab_num>0)
						{			
						   tab_num--;
						   if(input_number_tab[tab_num]=='.')
						   {
						   	dot_mark=0;
					    	}
               input_number_tab[tab_num]=' ';	
						}
            else
						{
							input_number_tab[tab_num]=' ';
						}
	        }
					
					 else if(X>482 && X<558 && Y>262 && Y<298)//ok   482,558,262,298
	        {
						while( input_number_tab[0]=='0' && input_number_tab[1]!='.')
						{
							for(m=0;m<3;m++)
							{
								input_number_tab[m]=input_number_tab[m+1];
							}
							input_number_tab[3]=' ';
						}
            //将输入的字符串转换为浮点数，并存入input_number中		
						input_number=Tab_To_Number(input_number_tab);
            
						//判断输入参数是否在设定范围内，在范围外则显示报警
						if(input_number<parameter_down[parameter_number] || input_number>parameter_up[parameter_number] || (parameter_number==2 && input_number>parameter[3]))
						{
							for(m=0;m<4;m++)
						  {
							  input_number_tab[m]=' ';
						  }
							tab_num=0;
						  dot_mark=0;
						  interface_number=1;
						  interface1(parameter_tab,state,action);
							alarm=alarm+pow(2,parameter_number); 
// 							Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,31,0,0,parameter_set_alarm[parameter_number],0x40,31,61,29);
						}
						else
						{
							parameter[parameter_number]=input_number;
							for(m=0;m<4;m++)
						  {
							  parameter_tab[parameter_number][m]=input_number_tab[m];
							  input_number_tab[m]=' ';
						  }
							
							//对所有float形式的参数进行处理，并存入parameter_done数组中
							parameter_done_float[0]=parameter[0]*5/16;
	            parameter_done_float[1]=parameter[1]/2;
	            parameter_done_float[2]=parameter[2]*650;  //f150时，850对应1mm
              parameter_done_float[3]=parameter[3]*650; 
              parameter_done_float[4]=100000/parameter[4];
							
							parameter_done[0]=parameter_done_float[0];
							parameter_done[1]=parameter_done_float[1];
							parameter_done[2]=parameter_done_float[2];
							parameter_done[3]=parameter_done_float[3];
							parameter_done[4]=parameter_done_float[4];
							
// 							temp=parameter[3]/parameter[2];
// 							scan_Y_position=0x8000-temp*parameter_done[2]/2;
							scan_direction=1;
							
							scan_time_max=100/parameter[4];
							
							voltage_A=parameter[0]*5/16;
							voltage_B=parameter[1]/2;
							
						  AD5722R_write_vol(voltage_A,'A');
							AD5722R_write_vol(voltage_B,'B');
							
							//store data deal
	            for(m=0;m<5;m++)
						  {
					      store_temp[m]=parameter_tab[m][0]*0x1000000+parameter_tab[m][1]*0x10000+parameter_tab[m][2]*0x100+parameter_tab[m][3];
						  }
						
						  //store parameter to flash
		          Flash_Write_Array(0,store_temp);
						
						  tab_num=0;
						  dot_mark=0;
						  interface_number=1;
						  interface1(parameter_tab,state,action);
						}
						break;
	        }	
	        else if(X>482 && X<558 && Y>302 && Y<338)//返回  482,558,302,338
	        {				
						for(m=0;m<4;m++)
						{
							input_number_tab[m]=' ';
						}
						tab_num=0;
						dot_mark=0;
						interface_number=1;
						interface1(parameter_tab,state,action);
						break;
	        }	
					
					delay_ms(200);
					Draw_Solid_Circle_Square_With_String(242,558,142,178,5,5,31,63,31, input_number_tab,0x45,0,0,0);
					
        default:
    			break;
			}		
		Write_Dir(0xf1,0x04);
	}
}
}



void  TIM4_IRQHandler()   //TIM4中断，用于触摸屏延时
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //检查time4中断是否发生
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //清除TIM4中断标志	
		touch_in=1;
		TIM_Cmd(TIM4,DISABLE);
	}
}
//     touch_time++;
//     if(touch_time>1)		
// 		{
// 		  touch_in=1;
// 			touch_time=0;
// 			TIM_Cmd(TIM4,DISABLE);


//定时器3中断服务程序，用于自动运行
void  TIM3_IRQHandler()   //TIM3中断
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //检查time3中断是否发生
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //清除TIM3中断标志
		auto_run_time++;	
// 		 Text(288,512 ,460,479,"武汉市凯瑞迪激光技术有限公司",0x00,28,46,0);
		if(auto_run_time>15)
		{
			auto_run_time=0;
			auto_run_step=1;
		}
	}
}
	
//定时器2中断服务程序，用于振镜扫描
void  TIM2_IRQHandler()   //TIM2中断
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //检查time2中断是否发生
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //清除TIM2中断标志
		if(action & 0x1000)  scan_time=scan_time+1;
		if(scan_time> scan_time_max-1)
		{
			scan_time=0;
			Scan_AD_Write(scan_Y_position,scan_Y_position); 

			if(scan_Y_position + parameter_done[2]>(0x8000+parameter_done[3]/2/i))
// 			if(scan_Y_position > 0xfffe-700)
			{
				scan_direction=0;
	//			out_zidong=~out_zidong;
// 				Scan_AD_Write(0xc000,0xc000);
				
// 			  scan_cs=~scan_cs; 
//         scan_sclk=~scan_sclk;
//         scan_sdix=~scan_sdix;
//         scan_sdiy=~scan_sdiy;
				
				
			}
			else if(scan_Y_position- parameter_done[2]<(0x8000-parameter_done[3]/2/i))
// 			else if(scan_Y_position < 700)
			{
				scan_direction=1;
// 				Scan_AD_Write(0x4000,0x4000);
			}
			
			if(scan_direction)
			{
// 				scan_Y_position=scan_Y_position + parameter_done[2];
				scan_Y_position=scan_Y_position + parameter_done[2];
			}
			else
			{
// 				scan_Y_position=scan_Y_position - parameter_done[2];
				scan_Y_position=scan_Y_position - parameter_done[2];
			}
		}			
	}
}

// void EXTI0_IRQHandler()  //急停
// {
//   state=state & 0xfffe;
// 	if(EXTI_GetITStatus(EXTI_Line0) != RESET)  //检查time2中断是否发生
// 	{
// 		EXTI_ClearITPendingBit(EXTI_Line0);  //清除TIM2中断标志
// 	  action=action & 0x0000;
// 		GPIO_ResetBits(GPIOG, 0xffff);	
// 	}
// }

//
// void EXTI9_5_IRQHandler()
// {
// 	state=state & 0xfffe;
// 	if(EXTI_GetITStatus(EXTI_Line5) != RESET) //冷却报警
//   {
// 		EXTI_ClearITPendingBit(EXTI_Line5);    // Clear the EXTI line 5 pending bit     state=state & 0x7fff;
// 		delay_ms(5);
// 		if(water)
// 		{
// // 		action=action & 0xb2ff;
// // 			GPIO_SetBits(GPIOB, temp & 0xff00);
// 	  GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));               //DB0-DB7输出
// 		}
//   }
// 	if(EXTI_GetITStatus(EXTI_Line6) != RESET)  //气压报警
//   {
// 		EXTI_ClearITPendingBit(EXTI_Line6);    // Clear the EXTI line 5 pending bit  
// 		state=state & 0xbfff;
// // 		action=action & 0x32ff;
// 		GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));
//   } 
//   if(EXTI_GetITStatus(EXTI_Line7) != RESET)  //调Q报警
//   {          
// 		EXTI_ClearITPendingBit(EXTI_Line7);    // Clear the EXTI line 5 pending bit  
// 		state=state & 0xdfff;
// // 		action=action & 0xb2ff;
// 		GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));
//   }
// }


//EXTI9_5_IRQHandler实例程序

// void EXTI9_5_IRQHandler(void)
// {
//   uint8_t data;
//                 
//        
// }





//flash存储数据测试程序

//  		Flash_Write_data(0x0000,1234);
// 		delay_ms(10);
//  	  Flash_Write_data(0x0004,1221);
// 		delay_ms(10);

//     X=Flash_Read_data(0x0000);
// 		delay_ms(10);
//  	  Y=Flash_Read_data(0x0004);
// 		delay_ms(10);
// 		
// // 		Text(0,512 ,400,440,"数据1读写正确",0x00,28,46,0);
// 		if(X==1234)
// 			{
// 				 Text(0,512 ,400,440,"数据1读写正确",0x00,28,46,0);
//       }
// // // 		else
// // 			break;
// 		
// // 		Text(0,612 ,440,480,"数据2读写正确",0x00,28,46,0);
// 		if(Y==1221)
// 			{
// 				 Text(0,612 ,440,480,"数据2读写正确",0x00,28,46,0);
//       }
// // 		else
// // 			break;
// 		
// 		
// 		
// 			delay_ms(40);





//AD5722R测试程序
// 	System_Clock_Init();
//   delay_init();
// 	
//   AD5722R_GPIO_Init();
//   AD5722R_soft_Init();
//   AD5722R_write_data(0x080001);  //设置A通道范围
// 	AD5722R_write_data(0x0a0001);  //设置B通道范围
//   AD5722R_write_vol(1,'A');
// 	AD5722R_write_vol(2,'B');


//LCD测试程序
// 	RA8875_GPIO_Init();
// 	RA8875_SPI_Init();
// 	
// 	delay_ms(4000);
// 	
// 	LCD_Initial();
// 	EXTIX_Init();
// 	
// 	delay_ms(4000);
// 	
// 	Write_Dir(0X01,0X80);//display on
// 	Active_Window(0,799,0,479);//Set the working window size
// 	Write_Dir(0X8E,0X80);//Began to clear the screen (display window)
// 	Write_Dir(0x21,0x20);//Select the external CGROM  ISO/IEC 8859-1.
// 	Write_Dir(0x2f,0x80);//Serial Font ROM Setting
// 	Chk_Busy();
// 	
// 	interface0();

// 	Write_Dir(0x70,0xb2);  //open the touch function, touch the parameter settings
// 	Write_Dir(0x71,0x05);  //set to 4-wire touch screen
// 	Write_Dir(0xf0,0x04);  //open interruption
// 		
// // 	Scroll_Window( 0, 800 ,440 ,480);
// // 	
// // 	Text_Foreground_Color1(color_red);//Set the foreground color
// // 	Text_Background_Color1(color_brown);//Set the background color
// // 	Write_Dir(0x22,0x00);
// // 		
// // 	FontWrite_Position(0,444);
// // 	String("报警滚动显示" );
// 		
// // 	Draw_Circle(400,240,100);	
// // 	Draw_Ellipse(600,100,50,100);


// 		Text_Background_Color(24,48,24);
//     Text_Foreground_Color(0,0,0);
// 		Write_Dir(0x22,0x45);
// 		
// 		
// 		Write_Dir(0xf1,0x04);
// 	
// 		while(1)
// 	{
// 		if(Touch_Status())  //)&&(temp1==0x00)&&(temp2==0x04)temp0==0xb2  temp4&0x04)==0x04
// 		{
// 			X=TP_X_Coordinate();
// 			Y=TP_Y_Coordinate();
// 			
// // 			Number_Input_Window();
// 			
// 			switch(interface_number)
// 			{
// 				case 0:   //在欢迎界面上
// 					if(X>345 && X<500 && Y>280 && Y<320)
// 					{
// 						interface_number=1;
// 						interface1();
// 					}
// 					else
// 						break;
// 					break;
// 					
// 				case 1:
// 					if(X>0 && X<100 && Y>380 && Y<480)
// 					{
// 						interface_number=2;
// 						interface2();
// 					}
// 					else if(X>700 && X<800 && Y>380 && Y<480)
// 					{
// 						interface_number=3;
// 						interface3();
// 					}
// 					else
// 						break;
// 					break;
// 					
// 				case 2:
// 					if(X>720 && X<790 && Y>430 && Y<470) // 返回
// 					{
// 						interface_number=1;
// 						interface1();
// 					}
// 					else if(X>430 && X<470 && Y>430 && Y<470) //参数确认
// 					{
// 						Number_Input_Window();
// 					}
// 					else
// 						break;
// 					break;
// 				
// 					
// 				case 3:
// 					if(X>720 && X<790 && Y>430 && Y<470) // 返回
// 					{
// 						interface_number=1;
// 						interface1();
// 					}
// 					else if(X>430 && X<470 && Y>430 && Y<470) //清除报警
// 					{
// 						
// 					}
// 					else
// 						break;
// 					break;
// 					
// 					
//         default:
//     			break;
// 			}
// 			
// 		delay_ms(100);
// 		
// 		Write_Dir(0xf1,0x04);
// 	}
// }
// }

// //外部中断1服务程序，lcd_int
// void EXTI5_IRQHandler(void)
// {

// }


		
		//界面刷新

		
		
		
		
		
// 		if(state0!=state || action0!=action)
// 		{
// 			state0=state;
// 			action0=action;
// 			refresh=1;
// 		}
// 		
// 		//界面刷新
// 		if(refresh && interface_number==1)
// 		{
// 			if(state & 0x8000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"冷却正常",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"冷却故障",0x45,31,61,29);
// 			}
// 			
// 				if(state & 0x4000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"气压正常",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"气压 低 ",0x45,31,61,29);
// 			}
// 			
// 			if(state & 0x2000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"调 Q正常",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"调 Q故障",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x8000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"制冷停止",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,31,0,0,"制冷启动",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x4000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"调 Q停止",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,31,0,0,"调 Q启动",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x2000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"吹气失能",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,31,0,0,"吹气使能",0x45,31,61,29);
// 			}

// 			if(action & 0x1000)
// 			{

// 				Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"扫描停止",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,31,0,0,"扫描启动",0x45,31,61,29);
// 			}

// 				if(action & 0x0800)
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"激光停止",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"激光启动",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0400)
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"光闸  关",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"光闸  开",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0200)
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"抽气停止",0x45,31,61,29);
// 			}
// 			else 
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"抽气启动",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0100)
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,0,32,0,"自动停止",0x45,31,61,29);
// 			}
// 			else 
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,31,0,0,"自动运行",0x45,31,61,29);
// 			}
// 			
// 			refresh=0;
// 		}

// 				case 2:
// 				if( (action & 0x4000)==0x0000 && (action & 0x8000)==0x8000 )
// 				{
// 					action=action | 0x4000;
// 					out_tiaoQ=1;
// 					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"调 Q停止",0x45,31,61,29);
// 				}	
// 				else if((action & 0x8000)==0)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;
// 				}
// 				break;
// 					
// 				case 3:
// 				if( (action & 0x2000)==0 && (action & 0xc000)==0xc000 )
// 				{
// 	        action=action | 0x2000;
// // 					out_cuiqi=1;
// 	        Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"吹气失能",0x45,31,61,29);
// 					break;
// 				}
// 				else if( (action & 0xc000)!= 0xc000 )
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;
// 				}					
// 				break;
// 				

// 				case 4:
// 				if( (action & 0x1000)==0 && (action & 0xe000) ==0xe000)
// 				{
// 				  action=action | 0x1000;
// 					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"扫描停止",0x45,31,61,29);
// 					TIM_Cmd(TIM2,ENABLE);  //使能TIMx
// 					scan_time=0;
// 					Scan_Open();
// // 					Scan_Close();
// 					break;
// 				}	
// 				else if((action & 0xe000) !=0xe000)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;
// 				}
// 				break;
// 				
// 				case 5:
// 				if((action & 0x0200)==0 &&  (action & 0xf000)==0xf000)
// 				{
// 				  action=action | 0x0200;
// 					out_couqi=1;
// 					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"抽气停止",0x45,31,61,29);
// 					break;
// 				}
// 				else if((action & 0xf000)!=0xf000)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;		
// 				}
//         break;				

// 				case 6:
// 				if((action & 0x0800)==0 && (action & 0xf600)==0xf200)
// 				{
//  				  action=action | 0x0800;
// 					out_laser=1;
// 					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"激光停止",0x45,31,61,29);
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					break;
// 				}	
// 				else if((action & 0xf600)!= 0xf200)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //失能TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;
// 				}
//         break;				
// 			}
// 		}

// u8 user_manual[21][50]={"    一  参数说明                                 ",
// 	                      "    (1) 激光电流：激光器工作时的泵浦电流，单位是A",
// 	                      "（安培），范围为0~32A。                          ",    
// 	                      "    (2) 激光频率：激光器工作时的调Q频率，单位为  ",
// 	                      "KHz（千赫兹），设置范围为5~20KHz。               ",
// 	                      "    (3) 扫描间距：激光清洗的最小间距，单位为mm   ",
// 	                      "（毫米），设置范围为0.1~2mm。                    ",
// 		                    "    二  供辅介质说明                             ",
// 	                      "    (1)CH-QX300型激光清洗设备的供电为AC380V交流  ",
// 	                      "电，三相五线，确保地线与大地及外壳接触良好。     ",
// 	                      "    (2)CH-QX300型激光清洗设备的冷却介质为符合国家",
// 	                      "标准的去离子水（又称脱盐水）。                   ",
// 		                    "    三  操作说明                                 ",
// 	                      "    本设备主要通过触摸屏及操作面板进行操作。基本 ",
// 	                      "操作步骤如下：                                   ",
// 	                      "    (1)                                          ",
// 		                    "    (2)                                          ",
// 	                      "    (3)                                          ",
// 			                  "    四  注意事项                                 ",
// 	                      "    (1)                                          ",
// 	                      "    (2)    

// u8 parameter_set_alarm[5][80]={"激光电流设置超出范围，正确设置范围为（0A，32A）！",
// 	                             "激光频率设置超出范围，正确设置范围为（0KHz，20KHz）！",
// 	                             "扫描间距设置超出范围或大于幅面，正确设置范围为（0.1mm，10mm）！",
// 	                             "扫描幅面设置超出范围，正确设置范围为（10mm，100mm）！",
// 	                             "扫描频率设置超出范围，正确设置范围为（1KHz，10KHz）！"};
	
	