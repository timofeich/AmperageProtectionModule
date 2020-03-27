#include "stm32f10x_conf.h"
#include <stdio.h>

#include "tim2_delay.h"
#include "lcd1602.h"
#include "rtc.h"
#include "sdcard.h"
#include "led.h"
#include "DmaWithAdc.h"

RTC_DateTimeTypeDef RTC_DateTime;
uint16_t ADCBuffer[] = {0xAAAA};

void OutputDateAtDisplay(void)
{
	char buffer[20];
	char timeBuffer[20];
	
	sprintf(buffer, "%02d.%02d.%04d", RTC_DateTime.RTC_Date, RTC_DateTime.RTC_Month, RTC_DateTime.RTC_Year);
	sprintf(timeBuffer, "%02d:%02d:%02d", RTC_DateTime.RTC_Hours, RTC_DateTime.RTC_Minutes, RTC_DateTime.RTC_Seconds);
	
	PrintDataOnLCD(buffer, 3, 0);
	PrintDataOnLCD(timeBuffer, 4, 1);
}

void OutputADCDataAtDisplay()
{
	char firstValueADC[17];
	char secondValueADC[17];
	
	sprintf(firstValueADC, "Ia=%2d   Uc=%1.2f",  ADCBuffer[0] / 16, (float)(ADCBuffer[0]) / 1241);		
	
	PrintDataOnLCD(firstValueADC, 0, 0);
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
	
	LEDsInitialization();
	SetSysClockToHSE();
	Timer2Initialization();
			
	I2CInitialization();	
	LCDInitialization();
	
	DMAInitializationForADCRecieve(ADCBuffer);
	
	if(RTC_Init() == 1)
	{
		SetStartRTCDate(27, 03, 2020, 10, 55, 50);
	}

	DetectCurrentLogFile(RTC_Counter);
	IWDGInitialization(2000);
	
	while(1)
	{
		RTC_Counter = RTC_GetCounter();
		RTC_GetDateTime(RTC_Counter, &RTC_DateTime);
		
		//OutputDateAtDisplay();
		//OutputADCDataAtDisplay();
				
		SendSensorDataToSDCard(ADCBuffer, &RTC_DateTime);
		BlinkGreenLed();
		
		IWDG -> KR = 0xAAAA; // перезагрузка		
//		while (RTC_Counter == RTC_GetCounter()) 
//		{ 
//		
//		}
	}
}
