#include "tim2_delay.h"

volatile uint8_t f_timer_2_end;

void IWDGInitialization(u16 tw) // ѕараметр tw от 7мс до 26200мс
{
	// включаем LSI
	RCC_LSICmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET); // ƒл¤ IWDG_PR=7 Tmin=6,4мс RLR=Tмс*40/256
	IWDG -> KR = 0x5555; //  люч дл¤ доступа к таймеру
	IWDG -> PR = 7; // ќбновление IWDG_PR
	IWDG -> RLR = tw * 40 / 256; // «агрузить регистр перезагрузки
	IWDG -> KR = 0xAAAA; // ѕерезагрузка
	IWDG -> KR = 0xCCCC; // ѕуск таймера
}


void Timer2Initialization(void)
{
	TIM_TimeBaseInitTypeDef TIMER_InitStructure;
	NVIC_InitTypeDef  NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructInit(&TIMER_InitStructure);

	TIMER_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIMER_InitStructure.TIM_Prescaler = 8;
	TIMER_InitStructure.TIM_Period = 1;
	TIM_TimeBaseInit(TIM2, &TIMER_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_SelectOnePulseMode(TIM2, TIM_OPMode_Single);
}

void TIM2_IRQHandler(void)
{
	extern volatile uint8_t f_timer_2_end;

	TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	TIM2->SR &= ~TIM_SR_UIF;
	f_timer_2_end = 1;

	TIM_Cmd(TIM2, DISABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, DISABLE);
}

void delay_us(uint32_t n_usec)
{
	f_timer_2_end = 0;

	TIM2->PSC = 0;
	TIM2->ARR = (uint16_t)(16 * n_usec);
	TIM_Cmd(TIM6, ENABLE);

	TIM2->EGR |= TIM_EGR_UG;
	TIM2->SR &= ~TIM_SR_UIF;

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	while(f_timer_2_end == 0);
}

void delay_ms(uint32_t n_msec)
{
	f_timer_2_end = 0;

	TIM2->PSC = 1000 - 1;
	TIM2->ARR = (uint16_t)(16 * n_msec);
	
	TIM2->EGR |= TIM_EGR_UG;
	TIM2->SR &= ~TIM_SR_UIF;

	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);

	while(f_timer_2_end == 0);
}

