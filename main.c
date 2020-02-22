#include "stm32f10x_conf.h"
#include <stdio.h>

#include "tim2_delay.h"
#include "lcd1602.h"
#include "rtc.h"
#include "sdcard.h"
//#include "adc.h"
#include "led.h"

RTC_DateTimeTypeDef RTC_DateTime;

uint16_t ADCBuffer[] = {0x0000, 0x0000, 0x0000, 0x0000};

void ADC1_Configure(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;  // we work in continuous sampling mode
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_NbrOfChannel = 1;

	ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_28Cycles5); // define regular conversion config
	ADC_Init ( ADC1, &ADC_InitStructure);   //set config of ADC1

	ADC_Cmd (ADC1,ENABLE);  //enable ADC1

	ADC_ResetCalibration(ADC1); // Reset previous calibration
	while(ADC_GetResetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1); // Start new calibration (ADC must be off at that time)
	while(ADC_GetCalibrationStatus(ADC1));

	ADC_Cmd(ADC1 , ENABLE ) ;
	ADC_DMACmd(ADC1 , ENABLE );

	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

void DMAInit_ADCRecieve(void)
{
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_BufferSize = 4;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)ADCBuffer;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1 , ENABLE ) ;

	ADC1_Configure();
}

void Init_IWDG(u16 tw) // ѕараметр tw от 7мс до 26200мс
{
	// включаем LSI
	RCC_LSICmd(ENABLE);
	while (RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
	// ƒл¤ IWDG_PR=7 Tmin=6,4мс RLR=Tмс*40/256
	IWDG->KR=0x5555; //  люч дл¤ доступа к таймеру
	IWDG->PR=7; // ќбновление IWDG_PR
	IWDG->RLR=tw*40/256; // «агрузить регистр перезагрузки
	IWDG->KR=0xAAAA; // ѕерезагрузка
	IWDG->KR=0xCCCC; // ѕуск таймера
}


int main(void)
{
	SetLEDsPins();
   
	char buffer[20] = {'\0'};
	char timeBuffer[20] = {'\0'};
	char firstValueADC[17];
	char secondValueADC[17];
	
	uint32_t RTC_Counter = 0;
	SetSysClockToHSE();
	TIM2_init();
		
	//Init_IWDG(7000);
	
//	I2CInit();	
//	lcd_init();
	
	DMAInit_ADCRecieve();

	//CreateHomeDirectory();
	if(RTC_Init() == 1)
	{
		RTC_DateTime.RTC_Date = 30;//TODO; create function - SetStartRTCData()  
		RTC_DateTime.RTC_Month = 1;
		RTC_DateTime.RTC_Year = 2020;

		RTC_DateTime.RTC_Hours = 13;
		RTC_DateTime.RTC_Minutes = 36;
		RTC_DateTime.RTC_Seconds = 30;

		delay_ms(500);
		RTC_SetCounter(RTC_GetRTC_Counter(&RTC_DateTime));
	}
		
	while(1)
	{
		RTC_Counter = RTC_GetCounter();
		
		RTC_GetDateTime(RTC_Counter, &RTC_DateTime);
		
//		sprintf(buffer, "%02d.%02d.%04d", RTC_DateTime.RTC_Date, RTC_DateTime.RTC_Month, RTC_DateTime.RTC_Year);
//		sprintf(timeBuffer, "%02d:%02d:%02d", RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);
//		
//		Display_Print(buffer, 3, 0);
//		Display_Print(timeBuffer, 4, 1);	
		
//		sprintf(firstValueADC, "Ia=%04d  Ib=%04d", ADCBuffer[0], ADCBuffer[1]);//TODO: create function - OutputDataOnDisplay
//		sprintf(secondValueADC, "Ic=%04d  Id=%04d", ADCBuffer[2], ADCBuffer[3]);
//			
//		Display_Print(firstValueADC, 0, 0);
//		Display_Print(secondValueADC, 0, 1);
		
		//SendSensorData(ADCBuffer, RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);
		//test_SD_2();
		BlinkLeds();
		
		SendSensorData(ADCBuffer, RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);
		
		while (RTC_Counter == RTC_GetCounter()) 
		{
			
		}
	}
}
