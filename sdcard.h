#ifndef SDCARD_SDCARD_H_
#define SDCARD_SDCARD_H_

#define MINIMUM_FREE_SPACE_ON_SD_CARD 7000000
#define MAXIMUM_FILE_SIZE 5242880

#include "ff.h"
#include "stm32f10x_conf.h"
#include <stdio.h>
#include "rtc.h"
#include "led.h"
#include "lcd1602.h"

void SendSensorDataToSDCard(uint16_t sensorData[4], RTC_DateTimeTypeDef* RTC_DateTimeStruct);
void GetCurrentLogFile(RTC_DateTimeTypeDef* RTC_DateTimeStruct);
int GetMaxValue(uint16_t * buffer);

#endif //SDCARD_SDCARD_H_
