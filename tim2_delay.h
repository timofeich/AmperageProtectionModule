#ifndef TIMERS_TIM2DELAY_H_
#define TIMERS_TIM2DELAY_H_

#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_tim.h"
#include "misc.h"

void Timer2Initialization(void);
void IWDGInitialization(u16 tw);

void delay_us(uint32_t n_usec);
void delay_ms(uint32_t n_msec);

#endif //TIMERS_TIM2DELAY_H_
