#ifndef __control_gpio_H 
#define __control_gpio_H 			   
#include "sys.h"

// ‰»Î
#define   in_water   PEin(5)	   
#define   in_gas     PEin(6)	
#define   in_tiaoQ   PEin(7)	
#define   in_jiting  PEin(0)	
#define   in_zidong  PEin(4)
#define   in_guangzha  PEin(3)

// ‰≥ˆ
#define  	out_cool    PGout(0)     //8
#define  	out_tiaoQ   PGout(1)     //8
#define  	out_cuiqi   PGout(2)     //8

#define  	out_guangza PGout(5)     //8
#define  	out_couqi   PGout(6)     //8
#define  	out_zidong  PGout(7)     //8

#define  	out_laser   PGout(9)      //8


void Control_GPIO_Init(void);

#endif

