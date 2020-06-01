#include "lcd1602.h"

uint8_t LCD_ADDR = 0x3F;

static char StatusOfSdCard[16][17] = 
{
	"                ",
	"SD card error   ", //FR_DISK_ERR
	"Filesystem error", //FR_INT_ERR
	"SD is not ready ", //FR_NOT_READY
	"No file in a dir", //FR_NO_FILE
	"No such path    ",	//FR_NO_PATH
	"Invalid filepath",	//FR_INVALID_NAME
	"Access denied   ",	//FR_DENIED
	"File alrdy exist",	//FR_EXIST
	"Invalid file obj",	//FR_INVALID_OBJECT
	"Write protection",	//FR_WRITE_PROTECTED
	"Invalid drv numb",	//FR_INVALID_DRIVE
	"Drive not enable",	//FR_NOT_ENABLED
	"No filesystem   ",	//FR_NO_FILESYSTEM
	"Volume too small",	//FR_MKFS_ABORTED
	"Timeout    error"	//FR_TIMEOUT
}; 

void I2CInitialization(void) 
{
	I2C_InitTypeDef  I2C_InitStructure;
	GPIO_InitTypeDef  GPIO_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1, ENABLE);	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStructure.I2C_OwnAddress1 = 0x15;
	I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStructure.I2C_ClockSpeed = 150000;

	I2C_Cmd(I2C1, ENABLE);
	I2C_Init(I2C1, &I2C_InitStructure);
}

void LCDInitialization (void)
{
	delay_ms(50);  // wait for >40ms
	SendCommandToLCD(0x30);
	delay_ms(5);  // wait for >4.1ms
	SendCommandToLCD(0x30);
	delay_ms(1);  // wait for >100us
	SendCommandToLCD(0x30);
	delay_ms(10);
	SendCommandToLCD(0x20);  // 4bit mode
	delay_ms(10);

	SendCommandToLCD(0x28); // Function set --> DL=0 (4 bit mode), N = 1 (2 line display) F = 0 (5x8 characters)
	delay_ms(1);
	SendCommandToLCD(0x08); //Display on/off control --> D=0,C=0, B=0  ---> display off
	delay_ms(1);
	SendCommandToLCD(0x01);  // clear display
	delay_ms(1);
	delay_ms(1);
	SendCommandToLCD(0x06); //Entry mode set --> I/D = 1 (increment cursor) & S = 0 (no shift)
	delay_ms(1);
	SendCommandToLCD(0x0C); //Display on/off control --> D = 1, C and B = 0. (Cursor and blink, last two bits)
}

void SendCommandToLCD(char cmd)
{
	char data_u, data_l;
	uint8_t data_t[4];
	
	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4) & 0xf0);
	
	data_t[0] = data_u | 0x0C;  //en=1, rs=0
	data_t[1] = data_u | 0x08;  //en=0, rs=0
	data_t[2] = data_l | 0x0C;  //en=1, rs=0
	data_t[3] = data_l | 0x08;  //en=0, rs=0
	
	I2C_GenerateSTART(I2C1, ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, LCD_ADDR << 1, I2C_Direction_Transmitter);

	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	for(int i = 0; i < 4; i++)
	{
		I2C_SendData(I2C1, data_t[i]);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	
	I2C_GenerateSTOP(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
}

void SendDataToLCD (char data)
{
	char data_u, data_l;
	uint8_t data_t[4];
	
	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	
	data_t[0] = data_u | 0x0D;  //en=1, rs=1
	data_t[1] = data_u | 0x09;  //en=0, rs=1
	data_t[2] = data_l | 0x0D;  //en=1, rs=1
	data_t[3] = data_l | 0x09;  //en=0, rs=1
	
	I2C_GenerateSTART(I2C1,ENABLE);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
	
	I2C_Send7bitAddress(I2C1, LCD_ADDR << 1, I2C_Direction_Transmitter);
	while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

	for(int i = 0; i < 4; i++)
	{
		I2C_SendData(I2C1, data_t[i]);
		while(!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
	}
	I2C_GenerateSTOP(I2C1, ENABLE);
	while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

}

void SetXYCoordinatsToLCD(uint8_t x, uint8_t y) 
{
	uint8_t addr = x + (y * 0x40);
	SendCommandToLCD(0x80 | addr);
}

void SendStringToLCD(char *str)
{
	while(*str) SendDataToLCD(*str++);
}

void PrintDataOnLCD(char * string, uint8_t x, uint8_t y) 
{
	SetXYCoordinatsToLCD(x,y);
	SendStringToLCD(string);
}

void OutputADCDataAtDisplay(uint16_t maxVoltageValue, uint16_t maxAmperageValue,  
	uint16_t maxAmperageValueB,  uint16_t maxAmperageValueC)
{
	char firstValueFromADC[17];
	char secondValueFromADC[17]; 
	
	float CurrentVoltageOnVagon = ((float)maxVoltageValue / 16);
	
	sprintf(firstValueFromADC, "U=%.1f Ia=%.1f", CurrentVoltageOnVagon, 
		(((float)maxAmperageValue * 0.000835 - 2.54) / 0.0123));
	
	sprintf(secondValueFromADC,"Ib=%.1f Ic=%.1f", (((float)maxAmperageValueB * 0.000839 - 2.54) / 0.0123), 
		(((float)maxAmperageValueC * 0.000844 - 2.54) / 0.0123));
	
	PrintDataOnLCD(firstValueFromADC, 0, 0);
	PrintDataOnLCD(secondValueFromADC, 0, 1);
}

void OutputSdCardStatusOnLCD(int status)
{	
	PrintDataOnLCD(StatusOfSdCard[0], 0, 0);
	PrintDataOnLCD(StatusOfSdCard[status], 0, 1);
}

