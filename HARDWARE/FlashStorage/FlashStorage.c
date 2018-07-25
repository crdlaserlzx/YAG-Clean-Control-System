// 使用Flash存储程序参数


#include "delay.h"
#include "sys.h"
#include "FlashStorage.h"
#include "stm32f10x_flash.h"

volatile  FLASH_Status FLASHStatus = FLASH_COMPLETE; 

void Flash_Write_data(u32 addr,u32 dat)
{
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//
  //FLASH_FLAG_BSY            FLASH????
  //FLASH_FLAG_EOP            FLASH???????
  //FLASH_FLAG_PGERR            FLASH???????
  //FLASH_FLAG_WRPRTERR       FLASH?????????         
  FLASHStatus=FLASH_ErasePage(FLASH_START_ADDR);
	if(FLASHStatus == FLASH_COMPLETE)
	{
	  FLASHStatus = FLASH_ProgramWord(FLASH_START_ADDR +addr, dat); 
	}		
  //   FLASH_ProgramHalfWord(FLASH_START_ADDR+addr*2,dat);  
  //   FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);  
  if(FLASHStatus == FLASH_COMPLETE)
	{
    FLASH_Lock();  
	}
}

void Flash_Write_Array(u32 addr,u32 dat[5])
{
	u8 m;
	FLASH_SetLatency(FLASH_Latency_2);
	FLASH_Unlock();
  FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//
  //FLASH_FLAG_BSY            FLASH????
  //FLASH_FLAG_EOP            FLASH???????
  //FLASH_FLAG_PGERR            FLASH???????
  //FLASH_FLAG_WRPRTERR       FLASH?????????         
  FLASHStatus=FLASH_ErasePage(FLASH_START_ADDR);
	
	for(m=0;m<5;m++)
	{
	  if(FLASHStatus == FLASH_COMPLETE)
	  {
	    FLASHStatus = FLASH_ProgramWord(FLASH_START_ADDR +addr+m*4, dat[m]); 
  	}		
  }
  //   FLASH_ProgramHalfWord(FLASH_START_ADDR+addr*2,dat);  
  //   FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);  
  if(FLASHStatus == FLASH_COMPLETE)
	{
    FLASH_Lock();  
	}
}

// void Flash_Write_N_data(u32 addr,u16 parameter_tab_temp[5][3])
// {
// 	u8 m,n;
// 	FLASH_SetLatency(FLASH_Latency_2);
// 	FLASH_Unlock();
//   FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);//
//   //FLASH_FLAG_BSY            FLASH????
//   //FLASH_FLAG_EOP            FLASH???????
//   //FLASH_FLAG_PGERR            FLASH???????
//   //FLASH_FLAG_WRPRTERR       FLASH?????????         
//   //   FLASHStatus=FLASH_ErasePage(FLASH_START_ADDR);
// 	
// 	for(m=0;m<5;m++)
// 	{
// 		for(n=0;n<6;n++)
// 		{
// 				FLASHStatus = FLASH_ProgramHalfWord(FLASH_START_ADDR +addr+2*(m*3+n), parameter_tab_temp[m][n]);   
// // 			  while(FLASHStatus == FLASH_COMPLETE)
// // 				{
// // 					delay_us(5);
// // 				}
// 		}
// 	}
  
// 	FLASHStatus = FLASH_ProgramHalfWord(FLASH_START_ADDR +addr, dat);    
//   //   FLASH_ProgramHalfWord(FLASH_START_ADDR+addr*2,dat);  
//   //   FLASH_ClearFlag(FLASH_FLAG_BSY|FLASH_FLAG_EOP|FLASH_FLAG_PGERR|FLASH_FLAG_WRPRTERR);  
//   if(FLASHStatus == FLASH_COMPLETE)
// 	{
//     FLASH_Lock();  
// 	}
// }

u32 Flash_Read_data(u32 addr)
{
	u32 temp;
	temp=*(u32*)(FLASH_START_ADDR+addr);
	return temp;
}

