#include "stm32f10x_conf.h"
#include <stdio.h>

#include "tim2_delay.h"
#include "lcd1602.h"
#include "rtc.h"
#include "sdcard.h"
#include "led.h"
#include "DmaWithAdc.h"

RTC_DateTimeTypeDef RTC_DateTime;
uint16_t ADCBuffer[4] = {0x0000, 0x0000, 0x0000, 0x0000};

void OutputDateAtDisplay(void)
{
	char buffer[20];
	char timeBuffer[20];
	
	sprintf(buffer, "%02d.%02d.%04d", RTC_DateTime.RTC_Date, RTC_DateTime.RTC_Month, RTC_DateTime.RTC_Year);
	sprintf(timeBuffer, "%02d:%02d:%02d", RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);
	
	PrintDataOnLCD(buffer, 3, 0);
	PrintDataOnLCD(timeBuffer, 4, 1);
}

void SetStartRTCDate(uint8_t day, uint8_t month, uint16_t year, 
		uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	RTC_DateTime.RTC_Date = day; 
	RTC_DateTime.RTC_Month = month;
	RTC_DateTime.RTC_Year = year;

	RTC_DateTime.RTC_Hours = hours;
	RTC_DateTime.RTC_Minutes = minutes;
	RTC_DateTime.RTC_Seconds = seconds;

	delay_ms(500);
	RTC_SetCounter(RTC_GetRTC_Counter(&RTC_DateTime));
}	

void DetectCurrentLogFile(uint32_t RTC_Counter)
{
	RTC_Counter = RTC_GetCounter();
	RTC_GetDateTime(RTC_Counter, &RTC_DateTime);
	GetCurrentLogFile(&RTC_DateTime);
}

int main(void)
{
	uint32_t RTC_Counter = 0;
	uint16_t AmperageBuffer[25] = { };
	LEDsInitialization();
	
	SetSysClockToHSE();

	Timer2Initialization();
		
	I2CInitialization();	
	LCDInitialization();
	
	DMAInitializationForADCRecieve(ADCBuffer);
	IWDGInitialization(100);	
	
	if(RTC_Init() == 1)
	{
		SetStartRTCDate(25, 05, 2020, 19, 56, 30);
	}

	DetectCurrentLogFile(RTC_Counter);
	
	while(1)		
	{
		RTC_Counter = RTC_GetCounter();
		RTC_GetDateTime(RTC_Counter, &RTC_DateTime);

		SendSensorDataToSDCard(ADCBuffer, &RTC_DateTime);
		OutputADCDataAtDisplay(ADCBuffer[0], ADCBuffer[1], ADCBuffer[2], ADCBuffer[3]);		
		
		BlinkGreenLed();
		
		IWDG -> KR = 0xAAAA;		
	}
}
