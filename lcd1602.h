#ifndef LCD1602_LCD1602_H_
#define LCD1602_LCD1602_H_

#include "stm32f10x_i2c.h"
#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "tim2_delay.h"

void I2CInitialization(void);

void LCDInitialization (void);
void SendStringToLCD (char *str);

void SendCommandToLCD(char cmd);
void SendDataToLCD (char data);

void SetXYCoordinatsToLCD(uint8_t x, uint8_t y);
void PrintDataOnLCD(char * string, uint8_t x, uint8_t y);

void OutputADCDataAtDisplay(int maxVoltageValue);
void OutputSdCardStatusOnLCD(int status);

#endif //LCD1602_LCD1602_H_
