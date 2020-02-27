#ifndef SDCARD_SDCARD_H_
#define SDCARD_SDCARD_H_

#include "ff.h"
#include "stm32f10x_conf.h"
#include "rtc.h"

void SendSensorDataToSDCard(uint16_t sensorData[4], RTC_DateTimeTypeDef* RTC_DateTimeStruct);
void GetCurrentLogFile(RTC_DateTimeTypeDef* RTC_DateTimeStruct);

#endif //SDCARD_SDCARD_H_