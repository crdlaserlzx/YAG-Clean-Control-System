#ifndef __AD5722R_H 
#define __AD5722R_H 			   
#include "sys.h"

void AD5722R_GPIO_Init(void);
void AD5722R_soft_Init(void);
void AD5722R_write_data(u32 data);
void AD5722R_write_vol(double vol,char AB);   //��AD5722R��A��B��ͨ����д���ѹ����
// u32 AD5722R_read_reg(void);


#endif


