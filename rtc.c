#include "rtc.h"

int year = 0;
int mon = 0;
int wday = 0;
int mday = 0;
int hour = 0;
int min = 0;
int sec = 0;

void SetSysClockToHSE(void)
{
	ErrorStatus HSEStartUpStatus;
  
	RCC_DeInit(); //    сброс настроек тактового генератора
	RCC_HSEConfig(RCC_HSE_ON); //   включение внешнего тактового генератора
	RCC_PREDIV1Config(RCC_PREDIV1_Source_HSE, RCC_PREDIV1_Div1);
	RCC_PLLConfig(RCC_PLLSource_PREDIV1 ,RCC_PLLMul_1); 
	RCC_PLLCmd(ENABLE); //  Включаем PLL
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);	//На системную шину подаем тактирование с внешнего резонатора
		
	RCC_DeInit();
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if (HSEStartUpStatus == SUCCESS)
	{
		RCC_HCLKConfig(RCC_SYSCLK_Div1);
		RCC_PCLK2Config(RCC_HCLK_Div1);
		RCC_PCLK1Config(RCC_HCLK_Div1);
		RCC_SYSCLKConfig(RCC_SYSCLKSource_HSE);
		
		while (RCC_GetSYSCLKSource() != 0x08)
		{ 	
			
		}
	}
    else
    { 
        while (1)
        {
        }
    }
}

uint8_t RTC_Init(void)
{
	// Включить тактирование модулей управления питанием и управлением резервной областью
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR | RCC_APB1Periph_BKP, ENABLE);
	// Разрешить доступ к области резервных данных
	PWR_BackupAccessCmd(ENABLE);
	// Если RTC выключен - инициализировать
	if ((RCC->BDCR & RCC_BDCR_RTCEN) != RCC_BDCR_RTCEN)
	{
		// Сброс данных в резервной области
		RCC_BackupResetCmd(ENABLE);
		RCC_BackupResetCmd(DISABLE);

		// Установить источник тактирования кварц 32768
		RCC_LSEConfig(RCC_LSE_ON);
		while ((RCC->BDCR & RCC_BDCR_LSERDY) != RCC_BDCR_LSERDY) {}
		BKP->RTCCR |= 3;        
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
			
		RTC_SetPrescaler(0x7FFF); // Устанавливаем делитель, чтобы часы считали секунды

		RCC_RTCCLKCmd(ENABLE);// Включаем RTC

		RTC_WaitForSynchro();// Ждем синхронизацию
			return 1;
	}
	
	return 0;
}

// (UnixTime = 00:00:00 01.01.1970 = JD0 = 2440588)
#define JULIAN_DATE_BASE	2440588

void RTC_GetDateTime(uint32_t RTC_Counter, RTC_DateTimeTypeDef* RTC_DateTimeStruct) 
{
	unsigned long time;
	unsigned long t1, a, b, c, d, e, m;

	uint64_t jd = 0;;
	uint64_t jdn = 0;

	jd = ((RTC_Counter + 43200) / (86400 >> 1)) + (2440587 << 1) + 1;
	jdn = jd >> 1;

	time = RTC_Counter;
	t1 = time / 60;
	sec = time - t1 * 60;

	time = t1;
	t1 = time / 60;
	min = time - t1 * 60;

	time = t1;
	t1 = time / 24;
	hour = time - t1 * 24;

	wday = jdn % 7;

	a = jdn + 32044;
	b = (4 * a + 3) / 146097;
	c = a - (146097 * b) / 4;
	d = (4 * c + 3) / 1461;
	e = c - (1461 * d) / 4;
	m = (5 * e + 2) / 153;
	mday = e - (153 * m + 2) / 5 + 1;
	mon = m + 3 - 12 * (m / 10);
	year = 100 * b + d - 4800 + (m / 10);

	RTC_DateTimeStruct -> RTC_Year = year;
	RTC_DateTimeStruct -> RTC_Month = mon;
	RTC_DateTimeStruct -> RTC_Date = mday;
	RTC_DateTimeStruct -> RTC_Hours = hour;
	RTC_DateTimeStruct -> RTC_Minutes = min;
	RTC_DateTimeStruct -> RTC_Seconds = sec;
	RTC_DateTimeStruct -> RTC_Wday = wday;
}

DWORD get_fattime (void) {
	return	  ((DWORD)(year - 1980) << 25)	
			| ((DWORD)mon << 21)			
			| ((DWORD)mday << 16)			
			| ((DWORD)hour << 11)			
			| ((DWORD)min << 5)			
			| ((DWORD)sec >> 1);			
}

uint32_t RTC_GetRTC_Counter(RTC_DateTimeTypeDef* RTC_DateTimeStruct) 
{
	uint8_t a;
	uint16_t y;
	uint8_t m;
	uint32_t JDN;

	a = (14 - RTC_DateTimeStruct -> RTC_Month) / 12;
	y = RTC_DateTimeStruct -> RTC_Year + 4800 - a;
	m = RTC_DateTimeStruct -> RTC_Month + (12 * a) - 3;

	JDN = RTC_DateTimeStruct -> RTC_Date;
	JDN += (153 * m + 2) / 5;
	JDN += 365 * y;
	JDN += y / 4;
	JDN += -y / 100;
	JDN += y / 400;
	JDN = JDN - 32045;
	JDN = JDN - JULIAN_DATE_BASE;
	JDN *= 86400;
	JDN += (RTC_DateTimeStruct -> RTC_Hours * 3600);
	JDN += (RTC_DateTimeStruct -> RTC_Minutes * 60);
	JDN += (RTC_DateTimeStruct -> RTC_Seconds);

	return JDN;
}
