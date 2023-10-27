#ifndef __ADC_H
#define __ADC_H

#include "sys.h"
#include "stm32f10x_adc.h"
#include "stm32f10x_tim.h"
void TIM2_Config(void);
void ADC1_Init(void);
u16 Get_Adc(u8 ch);

#endif

