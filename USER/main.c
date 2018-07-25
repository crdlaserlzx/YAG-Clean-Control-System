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

/*20170713�޸ģ���ΪӲ����ʱû���ģ�����ͣ��Ϊ�ı�ɨ������ȵ��ź�ʹ��
�Ķ���1����ͣ�ж�ȥ����״̬���Լ�ͣ�ķ�Ӧȥ����
     2�����ӱ���i��1-4ѭ���仯�����ڸı���档
     3���ֶ��Զ��������޷�ʹ�á�
     4��control_gpio.c�н��ⲿ�ж������ˡ�
 */
 
 /*20170803�޸ģ�������Ϣ������������ʾ
�Ķ���1��ȥ������˵�����棬��Ϊ������Ϣ����
     2�����ӱ���alarm��ָʾ����ĳһ�챨����Ϣ
		 3�����ӱ���alarm_string,���ڴ洢���屨����Ϣ
 */
 
//////////////////////////////////////////////////////////////////////////////////	 
//������ϴ���Ƴ���
//�޸�����:2015/9
//�汾��V0.0
//��Ȩ���У�����ؾ���
//Copyright(C) ����ϼ��� 2015-2015
//All rights reserved
//********************************************************************************

u8 auto_run_time=0,refresh=0,touch_in=1,touch_flag=0,touch_time=0; 
u8 auto_run_step=0;   //�Զ����еĲ����¼
u8 auto_delay_time_array[6]={10,30,40,50,60,70};  //�Զ�����ʱ�����������ִ�е��ӳ�ʱ��
                       //����->��Q->����->ɨ��->����->����->->

u16 state=0xffff,state0;  //�豸״ָ̬ʾ�Ĵ���,1��ʾ������bit15:ˮ��bit14������bit13����Q
                   //bit0��ʾ��ͣ����,state0��¼��ʷ��Ϣ
u16 action=0x0000,action0; //�豸�����Ĵ�����1��ʾ�����,action0��¼��ʷ��Ϣ
// u16 action_flag=0x0000; //�豸������־�Ĵ���
u32 alarm=0;
u8 alarm_string[21][50]={"#1���������ȷ���÷�ΧΪ��0A��32A����           ",
												 "#2����Ƶ����ȷ���÷�ΧΪ��0KHz��20KHz����       ",
												 "#3ɨ������ȷ���÷�ΧΪ��0.1mm��10mm����       ",
												 "#4ɨ�������ȷ���÷�ΧΪ��10mm��100mm����       ",
												 "#5ɨ��Ƶ����ȷ���÷�ΧΪ��1KHz��8KHz����       ",
                         "#6��ȴϵͳ���ϣ�����ˮ·��                    ",
	                       "#7��ѹ��������������·����Դ��ѹ��            ",	        
	                       "#8��Qϵͳ���ϣ������Ƶ����                   ",
                         "#9��ͣ�Ѱ��£�                                 "};

	
float parameter[5]={18.5,20,0.1,10.5,2.01};      // ����ֵ
float parameter_up[5]={32,20,10,100,8};   //����ֵ���ޣ������0.4ms��һ��
float parameter_down[5]={0,0,0.1,10,1}; //����ֵ����

u8  parameter_tab[5][5]={"18.5","200 ", "0.1 ","10.5", "2.01"};
u16 parameter_number,interface_number;  
float parameter_done_float[5]; 
u16 parameter_done[5];    //ϵͳ���������任ֱ�����ڳ������ֵ
  //parameter_done[0]���������д��ADоƬ����ֵ
  // parameter_done[1]������Ƶ�ʶ�Ӧд��ADоƬ����ֵ
  // parameter_done[2]����ÿ�������ľ���ת��Ϊд���16λ����ֵ
  // parameter_done[3]����ɨ�跶Χת��Ϊ16λ������ΧΪ��0x8000-parameter_done[3],0x8000+parameter_done[3]��
  // parameter_done[4]����ÿ������һ�ε�ʱ����ת��Ϊ��ʱ��2����������
u8 scan_direction=0; //0��ʾɨ����ֵ��С��1����
u16 scan_X_position=0x8000,scan_Y_position=0x8000;  //��ʾ

u32 scan_time=0;  //��ɨ���ʱ��Ĵ���
u32 scan_time_max;

// //ɨ����صĲ���
// focal_length=150;  //�۽�͸������

// ģ���ѹ�����ز���
double voltage_A,voltage_B;
u8 i,j;

int main()
{
	u8 m;   //touch_in ��ʾ�����������ź�����
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
  Scan_Init(); //��ɨ���ʼ��
	TIM2_Int_Init(1,359);//100kHZ�ļ���Ƶ�ʣ�������ɨ�裬�൱��0.01ms����һ��
	TIM3_Int_Init(1999,35999);//2Khz�ļ���Ƶ�ʣ������Զ����У�1s����һ��
	TIM4_Int_Init(999,35999);//2Khz�ļ���Ƶ�ʣ����ڴ����������ʱ,1s����һ��
	
  AD5722R_GPIO_Init();
  AD5722R_soft_Init();
  AD5722R_write_data(0x080001);  //����Aͨ����Χ 0~10V
	AD5722R_write_data(0x0a0001);  //����Bͨ����Χ 0~10V
	
	
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
	  parameter[m]=Tab_To_Number(parameter_tab[m]);   //���������ַ�����ʽת��Ϊfloat��ֵ
	}
  	delay_ms(600);
// //������float��ʽ�Ĳ������д���������parameter_done������
// 	parameter_done[0]=parameter[0]*10/32;
// 	parameter_done[1]=parameter[1]*10/12;
// 	parameter_done[2]=parameter[2]*850*(150/60);  //f150ʱ��500��Ӧ1mm
// 	parameter_done[3]=parameter[3]*850*(150/60); 
// 	parameter_done[4]=100000/parameter[4];
// 	
// 	temp=parameter[3]/parameter[2];
// 	scan_Y_position=0x8000-temp*parameter_done[2]/2;
// 	scan_direction=1;
// 	
// 	scan_time_max=100/parameter[4];
	
	//������float��ʽ�Ĳ������д���������parameter_done������
	parameter_done_float[0]=parameter[0]*5/16;
	parameter_done_float[1]=parameter[1]/2;
	parameter_done_float[2]=parameter[2]*650;  //f150ʱ��850��Ӧ1mm
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
	
  IWDG_Init(IWDG_Prescaler_64,2500);    //���Ƶ��Ϊ64,����ֵΪ625,���ʱ��Ϊ1s	
	
	voltage_A=parameter[0]*5/16;
	voltage_B=parameter[1]/2;
	
	AD5722R_write_vol(voltage_A,'A');
	AD5722R_write_vol(voltage_B,'B');

// 	AD5722R_write_vol(9,'A');
// 	AD5722R_write_vol(9.9,'B');
	
	
		while(1)
	{
		//ι�� 
	  IWDG_Feed();
		
		
		/*********************************************************
		//�����ⲿ״̬�ı�state�Ĵ���
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
	
// //���Գ����		
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
// //���Գ����	
		
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
		//���״̬ �� ������û�з����仯������б仯�������ʾ
		**********************************************************/
		
		if(state0!=state && interface_number==1)
		{
			if( (state & 0x8000) != (state0 & 0x8000) )
			{
				if(state & 0x8000)
				{
					Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"��ȴ����",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"��ȴ����",0x45,31,61,29);
				}	
			}	

			if( (state & 0x4000) != (state0 & 0x4000) )
			{
	      if(state & 0x4000)
				{
					Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"��ѹ����",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"��ѹ �� ",0x45,31,61,29);
				}								
			}
			
			if( (state & 0x2000) != (state0 & 0x2000) )	
			{
				if(state & 0x2000)
				{
					Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"�� Q����",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"�� Q����",0x45,31,61,29);
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
					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
				}	
			}
			
			if( (action & 0x4000) != (action0 & 0x4000) )
			{
				if(action & 0x4000)
				{
					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"�� Qֹͣ",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,31,0,0,"�� Q����",0x45,31,61,29);
				}		
			}
			
			if( (action & 0x2000) != (action0 & 0x2000) )
			{
				if(action & 0x2000)
				{
					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"����ʧ��",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,31,0,0,"����ʹ��",0x45,31,61,29);
				}			
			}
			
			if( (action & 0x1000) != (action0 & 0x1000) )
			{
				if(action & 0x1000)
				{

					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"ɨ��ֹͣ",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,31,0,0,"ɨ������",0x45,31,61,29);
				}								
			}
			
			if( (action & 0x0800) != (action0 & 0x0800) )
			{
				if(action & 0x0800)
				{
					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
				}						
			}
			
			if( (action & 0x0400) != (action0 & 0x0400) )
			{
				if(action & 0x0400)
				{
					Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"��բʧ��",0x45,31,61,29);
				}
				else
				{
					Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"��բʹ��",0x45,31,61,29);
				}							
			}
			
			if( (action & 0x0200) != (action0 & 0x0200) )
			{
				if(action & 0x0200)
				{
					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
				}
				else 
				{
					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
				}							
			}
			
			if( (action & 0x0100) != (action0 & 0x0100) )
			{
				if(action & 0x0100)
				{
					Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,0,32,0,"�Զ�ֹͣ",0x45,31,61,29);
				}
				else 
				{
					Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,31,0,0,"�Զ�����",0x45,31,61,29);
				}							
			}
			
			action0=action;
		}
				
		/*********************************************************
		//�ⲿ�Զ�����
		**********************************************************/	
		
		if(in_zidong==0 && in_jiting==0)
		{
			delay_ms(10);
			if(in_zidong==0 && in_jiting==0)
			{
				if(action & 0x0100)  //����Ѿ����Զ�����
				{
					action=action & 0xb2ff;			
					out_tiaoQ=0;					
					out_guangza=0;							
					out_zidong=0;
					out_laser=0;
					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
					auto_run_time=0;
					auto_run_step=0;
					
				}
				else if( (action & 0x0c00)==0 )
				{
					action=action | 0x0100;
					out_zidong=1;
												
					TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx	
          auto_run_step=0;
					auto_run_time=0;
				}
			}
		}
		
		/**********************************************************
		//����豸״̬��ˮ��������Q����ͣ����բ����������Ӧ����
		**********************************************************/	
		
		if( (state & 0x8000)==0 &&  interface_number==1 )   //ˮ����
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
		
		if( (state & 0x4000)==0 && interface_number==1)   //������
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
				TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
			}
// 			delay_ms(1000);
		}		
		
		if( (state & 0x2000)==0  && interface_number==1)   //���Ʊ���
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
				TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
			}
// 			delay_ms(1000);
		}
		
		if( (state & 0x0001)==0 && interface_number==1 )  //��ͣ
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
			TIM_Cmd(TIM2,DISABLE);  //ʧ��TIMx	
// 							Scan_Open();
			Scan_Close();
		}	
		
		/**********************************************************
		//�Զ����г���
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
// 					Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);	
// 					
// 					Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"����ʧ��",0x45,31,61,29);
					
					TIM_Cmd(TIM2,ENABLE);  //ʹ��TIMx
					scan_time=0;
					Scan_Open();
// 					Scan_Close();
// 					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"ɨ��ֹͣ",0x45,31,61,29);
					
					out_couqi=1;
// 					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
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
// 					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"�� Qֹͣ",0x45,31,61,29);
					
					out_laser=1;
// 					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
					
					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
					auto_run_time=0;
					auto_run_step=0;
				}
				else
				{
					action=action & 0xfeff;							
					out_zidong=0;
					
					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
					auto_run_time=0;
					auto_run_step=0;
					refresh=1;
					break;			
				}
				break;
			}
		}
		
		/*************************************
			��������ˢ��
		**************************************/
		
		if( (refresh==1) && (interface_number==2) )
		{
			interface2(alarm_string,alarm);
			refresh=0;
		}
		
		/*************************************
			������ˢ��
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
		// �д���   ,���������־λ����һ�β��ɹ�����Ҫ��β���
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
		
		
		if(touch_flag)  // �д��������Ҽ�ͣδ����&& in_jiting==0 
		{
			refresh=1;
// 			X=TP_X_Coordinate();
// 			Y=TP_Y_Coordinate();
			touch_flag=0;

			switch(interface_number) //�ж����Ǹ�������
			{
				/*************************************
				��ӭ����
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
				������
				**************************************/
					
				case 1:
// 				//���Գ���Σ��ڵ��Գ���ʱ��
// 					if(X>85 && X<155 && Y>65 && Y<135)
// 					{
// 						if(state & 0x8000)
// 	          {
// 							state =state & 0x7ffe;
// 							Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"��ȴ����",0x45,31,61,29);
// 	          }
// 	          else
// 	          {
// 							state =state | 0x8000;
// 		          Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"��ȴ����",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
// 					else if(X>165 && X<235 && Y>65 && Y<135)
// 					{
// 						if(state & 0x4000)
// 	          {
// 							state =state & 0xbffe;
// 							Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"��ѹ �� ",0x45,31,61,29);
// 		          
// 	          }
// 	          else
// 	          {
// 							state =state | 0x4000;
// 	            Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"��ѹ����",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
// 					else if(X>245 && X<315&&  Y>65 && Y<135)
// 					{
// 						if(state & 0x2000)
// 	          {
// 		          state =state & 0xdffe;
// 							Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"�� Q����",0x45,31,61,29);
// 							
// 	          }
// 	          else
// 	          {
// 							state =state | 0x2000;
// 	            Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"�� Q����",0x45,31,61,29);
// 	          }
// 						delay_ms(100);
// 					}
				
			  //��ʽ����				
				  if(X>85 && X<226 && Y>205 && Y<245 && ( (action & 0x0800) == 0) ) //����1
					{
// 						FontWrite_Position(226,205)��
// 						Write_Dir(0x40,0xe0);
						parameter_number=0;
						interface_number=4;
						interface4();
					}
					else if(X>226 && X<370 && Y>205 && Y<245)  //����2
					{
						parameter_number=1;
						interface_number=4;
						interface4();
					}
					else if(X>370 && X<514 && Y>205 && Y<245)  //����3
					{
						parameter_number=2;
						interface_number=4;
						interface4();
					}
					else if(X>514 && X<658 && Y>205 && Y<245)  //����4
					{ 
						parameter_number=3;
						interface_number=4;
						interface4();
					}
					else if(X>658 && X<799 && Y>205 && Y<245)  //����5
					{
						parameter_number=4;
						interface_number=4;
						interface4();
					}
					else if(X>80 && X<220 && Y>258 && Y<298 ) //����  80,220,258,298
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
				      TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx							
						}
						else if( (state & 0x4001)==0x4001)
						{
							action=action | 0x8000;
						  out_cool=1;					
// 							Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
						}
// 					delay_ms(100);
					}
					else if(X>80 && X<220 && Y>302 && Y<342) //��Q  80,220,302,342
					{
						if((action & 0x4000)==0x4000)
						{
							action=action & 0xb2ff;
							out_tiaoQ=0;			
							out_guangza=0;
							out_zidong=0;
							out_laser=0;
				      TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
							
						}
						else if(((action & 0x8000)==0x8000) && ((state & 0xe001)==0xe001))
						{
							action=action | 0x4000;
							out_tiaoQ=1;
// 							Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"�� Qֹͣ",0x45,31,61,29);
						}
// 					delay_ms(10);
					}
					
					else if(X>225 && X<365 && Y>258 && Y<298) //����  225,365,258,298
					{
						if(action & 0x2000)
						{
							action=action & 0xd2ff;
// 							out_cuiqi=0;
							out_guangza=0;
							out_zidong=0;
							out_laser=0;
							refresh=1;
				      TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
						}
						else if( (state & 0x0001)==0x0001 )
						{
							action=action | 0x2000;												
// 					    out_cuiqi=1;
// 							Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"����ʧ��",0x45,31,61,29);
						}
// 					delay_ms(100);
					}
					else if(X>225 && X<365 &&  Y>302 && Y<342) //ɨ��  225,365,302,342
					{
						if(action & 0x1000)
						{
							action=action & 0xeaff;
					    out_guangza=0;
					    out_zidong=0;
							Scan_AD_Write(0x8000,0x8000);
				      TIM_Cmd(TIM2,DISABLE);  //ʧ��TIMx	
// 							Scan_Open();
					    Scan_Close();
						}
						else   //if((state & 0x0001)==0x0001)
						{
							action=action | 0x1000;
// 							Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"ɨ��ֹͣ",0x45,31,61,29);
							TIM_Cmd(TIM2,ENABLE);  //ʹ��TIMx
							scan_X_position=0x8000;
							scan_Y_position=0x8000;
							scan_time=0;
							Scan_Open();
// 					Scan_Close();
						}
// 					delay_ms(100);
					}					
					else if(X>370 && X<510 && Y>258 && Y<298) //����   370,510,258,298
					{
						if(action & 0x0800)
						{
							action=action & 0xf7ff;
					    out_laser=0;
							//Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"��������",0x45,31,61,29);	
						}
						else if( ((action & 0xf000)==0xf000) && ((state & 0xffff)==0xffff) && ( (action | 0xfbff)==0xfbff ) )
						{
							action=action | 0x0800;
							out_laser=1;
							//Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
						}
         // delay_ms(100);
					}					
					
					else if(X>370 && X<510 &&  Y>302 && Y<342) //��բ   370,510,302,342
					{
						if(action & 0x0400)
						{
							action=action & 0xfbff; 
// 							out_guangza=0;
// 							Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"��բ �� ",0x45,31,61,29);
						}
						else if((action & 0xd000)==0xd000 )    //(state & 0x0001)==0x0001
						{
							action=action | 0x0400;
// 							out_guangza=1;
// 							Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"��բ �� ",0x45,31,61,29);
						}
        //	delay_ms(100);
					}
					
					else if(X>515 && X<655 && Y>258 && Y<298) //����  515,655,258,298
					{
						if(action & 0x0200)
						{
							action=action & 0xfdff;
							out_couqi=0;
// 							Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
						}
						else if((state & 0x0001)==0x0001)
						{
							action=action | 0x0200;
							out_couqi=1;
// 							Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
						}
// 					delay_ms(100);	
					}
					
					else if(X>515 && X<655 && Y>302 && Y<342) //�Զ����� 515,655,302,342
					{
						if(action & 0x0100)
						{
							action=action & 0xb2ff;						
							out_tiaoQ=0;					
							out_guangza=0;							
							out_zidong=0;
							out_laser=0;
							TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
// 							TIM_Cmd(TIM2,DISABLE);  //ʧ��TIMx
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
							TIM_Cmd(TIM3, ENABLE);  //ʹ��TIMx	
              auto_run_step=0;							
						}
				//	delay_ms(100);	
					}					
					
					else if(X>660 && X<744 && Y>258 && Y<342) //ϵͳ��λ 660,744,258,342
					{
						action = action & 0x80ff; 				
						out_tiaoQ=0;
						out_guangza=0;
						out_couqi=0;
						out_zidong=0;
						out_laser=0;
						TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
						TIM_Cmd(TIM2,DISABLE);  //ʧ��TIMx
						Scan_AD_Write(0x8000,0x8000);
// 					Scan_Open();
				  	Scan_Close();
					}		

					else if(X>0 && X<80 && Y>400 && Y<480)  //������Ϣ
					{
						interface_number=2;
						interface2(alarm_string,alarm);
					}
					else if(X>720 && X<800 && Y>400 && Y<480)  //���ϸ�λ
					{
            Draw_Solid_Circle_Square_With_String(100,700,405,455,10,10,0,32,0,"     ",0x40,31,61,29);
						alarm=0;
					//	refresh=1;
					}
					else
						break;
					break;
					
/*************************************
������Ϣ����
**************************************/						
					
				case 2:
					if(X>720 && X<790 && Y>430 && Y<470) // ����
					{
						interface_number=1;
						interface1(parameter_tab,state,action);
					}
// 					else if(X>240 && X<340 && Y>430 && Y<470) //"��һҳ"  240,340,430,470
// 					{

// 					}
					else if(X>350 && X<450 && Y>430 && Y<470) //"������λ", 350,450,430,470
					{
						alarm=0;
						interface2(alarm_string,alarm);	
            refresh=1;						
					}
// 					else if(X>460 && X<560 && Y>430 && Y<470) //"��һҳ"  460,560,430,470
// 					{

// 					}
					else
						break;
					break;
					
/*************************************
�����������
**************************************/					
					
				case 4:
					if(X>242 && X<318 && Y>182 && Y<218)  //��ֵ1 242,318,182,218
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='1';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>182 && Y<218)//��ֵ2  322,398,182,218
	        {   
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='2';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>402 && X<478 && Y>182 && Y<218)//��ֵ3  402,478,182,218,
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='3';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>222 && Y<258)//��ֵ4   242,318,222,258
	        {  
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='4';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>222 && Y<258)//��ֵ5   322,398,222,258
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='5';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>402 && X<478 && Y>222 && Y<258)//��ֵ6    402,478,222,258
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='6';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>262 && Y<298)//��ֵ7   242,318,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='7';
						  tab_num++;
						}
						else
							break;				
	        }
	        else if(X>322 && X<398 && Y>262 && Y<298)//��ֵ8   322,398,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='8';
						  tab_num++;
						}
						else
							break;				
	        }
	        else if(X>402 && X<478 && Y>262 && Y<298)//��ֵ9   402,478,262,298
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='9';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>322 && X<398 && Y>302 && Y<338)//��ֵ0   322,398,302,338
	        {
						if(tab_num<4)
						{
						  input_number_tab[tab_num]='0';
						  tab_num++;
						}
						else
							break;
	        }
	        else if(X>242 && X<318 && Y>302 && Y<338)//С����.   242,318,302,338
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
            //��������ַ���ת��Ϊ��������������input_number��		
						input_number=Tab_To_Number(input_number_tab);
            
						//�ж���������Ƿ����趨��Χ�ڣ��ڷ�Χ������ʾ����
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
							
							//������float��ʽ�Ĳ������д���������parameter_done������
							parameter_done_float[0]=parameter[0]*5/16;
	            parameter_done_float[1]=parameter[1]/2;
	            parameter_done_float[2]=parameter[2]*650;  //f150ʱ��850��Ӧ1mm
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
	        else if(X>482 && X<558 && Y>302 && Y<338)//����  482,558,302,338
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



void  TIM4_IRQHandler()   //TIM4�жϣ����ڴ�������ʱ
{
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)  //���time4�ж��Ƿ���
	{
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);  //���TIM4�жϱ�־	
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


//��ʱ��3�жϷ�����������Զ�����
void  TIM3_IRQHandler()   //TIM3�ж�
{
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)  //���time3�ж��Ƿ���
	{
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update  );  //���TIM3�жϱ�־
		auto_run_time++;	
// 		 Text(288,512 ,460,479,"�人�п���ϼ��⼼�����޹�˾",0x00,28,46,0);
		if(auto_run_time>15)
		{
			auto_run_time=0;
			auto_run_step=1;
		}
	}
}
	
//��ʱ��2�жϷ������������ɨ��
void  TIM2_IRQHandler()   //TIM2�ж�
{
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)  //���time2�ж��Ƿ���
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  //���TIM2�жϱ�־
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

// void EXTI0_IRQHandler()  //��ͣ
// {
//   state=state & 0xfffe;
// 	if(EXTI_GetITStatus(EXTI_Line0) != RESET)  //���time2�ж��Ƿ���
// 	{
// 		EXTI_ClearITPendingBit(EXTI_Line0);  //���TIM2�жϱ�־
// 	  action=action & 0x0000;
// 		GPIO_ResetBits(GPIOG, 0xffff);	
// 	}
// }

//
// void EXTI9_5_IRQHandler()
// {
// 	state=state & 0xfffe;
// 	if(EXTI_GetITStatus(EXTI_Line5) != RESET) //��ȴ����
//   {
// 		EXTI_ClearITPendingBit(EXTI_Line5);    // Clear the EXTI line 5 pending bit     state=state & 0x7fff;
// 		delay_ms(5);
// 		if(water)
// 		{
// // 		action=action & 0xb2ff;
// // 			GPIO_SetBits(GPIOB, temp & 0xff00);
// 	  GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));               //DB0-DB7���
// 		}
//   }
// 	if(EXTI_GetITStatus(EXTI_Line6) != RESET)  //��ѹ����
//   {
// 		EXTI_ClearITPendingBit(EXTI_Line6);    // Clear the EXTI line 5 pending bit  
// 		state=state & 0xbfff;
// // 		action=action & 0x32ff;
// 		GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));
//   } 
//   if(EXTI_GetITStatus(EXTI_Line7) != RESET)  //��Q����
//   {          
// 		EXTI_ClearITPendingBit(EXTI_Line7);    // Clear the EXTI line 5 pending bit  
// 		state=state & 0xdfff;
// // 		action=action & 0xb2ff;
// 		GPIO_ResetBits(GPIOG, ~(action & 0xb2ff));
//   }
// }


//EXTI9_5_IRQHandlerʵ������

// void EXTI9_5_IRQHandler(void)
// {
//   uint8_t data;
//                 
//        
// }





//flash�洢���ݲ��Գ���

//  		Flash_Write_data(0x0000,1234);
// 		delay_ms(10);
//  	  Flash_Write_data(0x0004,1221);
// 		delay_ms(10);

//     X=Flash_Read_data(0x0000);
// 		delay_ms(10);
//  	  Y=Flash_Read_data(0x0004);
// 		delay_ms(10);
// 		
// // 		Text(0,512 ,400,440,"����1��д��ȷ",0x00,28,46,0);
// 		if(X==1234)
// 			{
// 				 Text(0,512 ,400,440,"����1��д��ȷ",0x00,28,46,0);
//       }
// // // 		else
// // 			break;
// 		
// // 		Text(0,612 ,440,480,"����2��д��ȷ",0x00,28,46,0);
// 		if(Y==1221)
// 			{
// 				 Text(0,612 ,440,480,"����2��д��ȷ",0x00,28,46,0);
//       }
// // 		else
// // 			break;
// 		
// 		
// 		
// 			delay_ms(40);





//AD5722R���Գ���
// 	System_Clock_Init();
//   delay_init();
// 	
//   AD5722R_GPIO_Init();
//   AD5722R_soft_Init();
//   AD5722R_write_data(0x080001);  //����Aͨ����Χ
// 	AD5722R_write_data(0x0a0001);  //����Bͨ����Χ
//   AD5722R_write_vol(1,'A');
// 	AD5722R_write_vol(2,'B');


//LCD���Գ���
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
// // 	String("����������ʾ" );
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
// 				case 0:   //�ڻ�ӭ������
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
// 					if(X>720 && X<790 && Y>430 && Y<470) // ����
// 					{
// 						interface_number=1;
// 						interface1();
// 					}
// 					else if(X>430 && X<470 && Y>430 && Y<470) //����ȷ��
// 					{
// 						Number_Input_Window();
// 					}
// 					else
// 						break;
// 					break;
// 				
// 					
// 				case 3:
// 					if(X>720 && X<790 && Y>430 && Y<470) // ����
// 					{
// 						interface_number=1;
// 						interface1();
// 					}
// 					else if(X>430 && X<470 && Y>430 && Y<470) //�������
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

// //�ⲿ�ж�1�������lcd_int
// void EXTI5_IRQHandler(void)
// {

// }


		
		//����ˢ��

		
		
		
		
		
// 		if(state0!=state || action0!=action)
// 		{
// 			state0=state;
// 			action0=action;
// 			refresh=1;
// 		}
// 		
// 		//����ˢ��
// 		if(refresh && interface_number==1)
// 		{
// 			if(state & 0x8000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,0,32,0,"��ȴ����",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(85,155,65,135,3,2,31,0,0,"��ȴ����",0x45,31,61,29);
// 			}
// 			
// 				if(state & 0x4000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,0,32,0,"��ѹ����",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(165,235,65,135,3,2,31,0,0,"��ѹ �� ",0x45,31,61,29);
// 			}
// 			
// 			if(state & 0x2000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,0,32,0,"�� Q����",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(245,315,65,135,3,2,31,0,0,"�� Q����",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x8000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x4000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"�� Qֹͣ",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,31,0,0,"�� Q����",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x2000)
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"����ʧ��",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,31,0,0,"����ʹ��",0x45,31,61,29);
// 			}

// 			if(action & 0x1000)
// 			{

// 				Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"ɨ��ֹͣ",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,31,0,0,"ɨ������",0x45,31,61,29);
// 			}

// 				if(action & 0x0800)
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0400)
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,0,32,0,"��բ  ��",0x45,31,61,29);
// 			}
// 			else
// 			{
// 				Draw_Solid_Circle_Square_With_String(370,510,302,342,3,3,31,0,0,"��բ  ��",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0200)
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
// 			}
// 			else 
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,31,0,0,"��������",0x45,31,61,29);
// 			}
// 			
// 			if(action & 0x0100)
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,0,32,0,"�Զ�ֹͣ",0x45,31,61,29);
// 			}
// 			else 
// 			{
// 				Draw_Solid_Circle_Square_With_String(515,655,302,342,3,3,31,0,0,"�Զ�����",0x45,31,61,29);
// 			}
// 			
// 			refresh=0;
// 		}

// 				case 2:
// 				if( (action & 0x4000)==0x0000 && (action & 0x8000)==0x8000 )
// 				{
// 					action=action | 0x4000;
// 					out_tiaoQ=1;
// 					Draw_Solid_Circle_Square_With_String(80,220,302,342,3,3,0,32,0,"�� Qֹͣ",0x45,31,61,29);
// 				}	
// 				else if((action & 0x8000)==0)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
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
// 	        Draw_Solid_Circle_Square_With_String(225,365,258,298,3,3,0,31,0,"����ʧ��",0x45,31,61,29);
// 					break;
// 				}
// 				else if( (action & 0xc000)!= 0xc000 )
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
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
// 					Draw_Solid_Circle_Square_With_String(225,365,302,342,3,3,0,32,0,"ɨ��ֹͣ",0x45,31,61,29);
// 					TIM_Cmd(TIM2,ENABLE);  //ʹ��TIMx
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
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
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
// 					Draw_Solid_Circle_Square_With_String(515,655,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
// 					break;
// 				}
// 				else if((action & 0xf000)!=0xf000)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
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
// 					Draw_Solid_Circle_Square_With_String(370,510,258,298,3,3,0,32,0,"����ֹͣ",0x45,31,61,29);
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					break;
// 				}	
// 				else if((action & 0xf600)!= 0xf200)
// 				{
// 					action=action & 0xfeff;							
// 					out_zidong=0;
// 					refresh=1;
// 					TIM_Cmd(TIM3,DISABLE);  //ʧ��TIMx	
// 					auto_run_time=0;
// 					auto_run_step=0;
// 					refresh=1;
// 					break;
// 				}
//         break;				
// 			}
// 		}

// u8 user_manual[21][50]={"    һ  ����˵��                                 ",
// 	                      "    (1) �������������������ʱ�ı��ֵ�������λ��A",
// 	                      "�����ࣩ����ΧΪ0~32A��                          ",    
// 	                      "    (2) ����Ƶ�ʣ�����������ʱ�ĵ�QƵ�ʣ���λΪ  ",
// 	                      "KHz��ǧ���ȣ������÷�ΧΪ5~20KHz��               ",
// 	                      "    (3) ɨ���ࣺ������ϴ����С��࣬��λΪmm   ",
// 	                      "�����ף������÷�ΧΪ0.1~2mm��                    ",
// 		                    "    ��  ��������˵��                             ",
// 	                      "    (1)CH-QX300�ͼ�����ϴ�豸�Ĺ���ΪAC380V����  ",
// 	                      "�磬�������ߣ�ȷ���������ؼ���ǽӴ����á�     ",
// 	                      "    (2)CH-QX300�ͼ�����ϴ�豸����ȴ����Ϊ���Ϲ���",
// 	                      "��׼��ȥ����ˮ���ֳ�����ˮ����                   ",
// 		                    "    ��  ����˵��                                 ",
// 	                      "    ���豸��Ҫͨ�������������������в��������� ",
// 	                      "�����������£�                                   ",
// 	                      "    (1)                                          ",
// 		                    "    (2)                                          ",
// 	                      "    (3)                                          ",
// 			                  "    ��  ע������                                 ",
// 	                      "    (1)                                          ",
// 	                      "    (2)    

// u8 parameter_set_alarm[5][80]={"����������ó�����Χ����ȷ���÷�ΧΪ��0A��32A����",
// 	                             "����Ƶ�����ó�����Χ����ȷ���÷�ΧΪ��0KHz��20KHz����",
// 	                             "ɨ�������ó�����Χ����ڷ��棬��ȷ���÷�ΧΪ��0.1mm��10mm����",
// 	                             "ɨ��������ó�����Χ����ȷ���÷�ΧΪ��10mm��100mm����",
// 	                             "ɨ��Ƶ�����ó�����Χ����ȷ���÷�ΧΪ��1KHz��10KHz����"};
	
	