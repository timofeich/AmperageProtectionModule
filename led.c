#include "led.h"

void LEDsInitialization(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_10 | GPIO_Pin_11);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA , &GPIO_InitStructure);
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_9 | GPIO_Pin_8);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC , &GPIO_InitStructure);
}

void BlinkGreenLed(void)
{	
	GPIOA->ODR ^= (GPIO_Pin_10);
}

void BlinkBlueLed(void)
{
	GPIOA->ODR ^= (GPIO_Pin_11);
}

void BlinkCGreenLed(void)
{	
	GPIOC->ODR ^= (GPIO_Pin_9);
}

void BlinkCBlueLed(void)
{	
	GPIOC->ODR ^= (GPIO_Pin_8);
}