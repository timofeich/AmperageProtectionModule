#include "lcd1602.h"

uint8_t LCD_ADDR = 0x3F;

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
	I2C_InitStructure.I2C_ClockSpeed = 400000;//400000

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

