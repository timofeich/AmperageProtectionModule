#include "ff.h"
#include "stm32f10x_conf.h"

volatile FRESULT result;
char	buff[1024];		

void test_SD_1(void)
{
	static FATFS FATFS_Obj;
	static FIL file;

	result = f_mount(0, &FATFS_Obj);	

	if (result == FR_OK)
	{
		//printf("Ошибка монтирования диска %d\r\n", result);
	}

	UINT nRead, nWritten;

	result = f_open(&file, "readme.txt", FA_OPEN_EXISTING | FA_READ);
	if (result == FR_OK)
	{
		GPIOC->ODR ^= (GPIO_Pin_8 | GPIO_Pin_9);
		f_read(&file, &buff, 1024, &nRead);
		f_close(&file);
	}

	result = f_open(&file, "write2.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (result == FR_OK)
	{
		GPIOC->ODR ^= (GPIO_Pin_8 | GPIO_Pin_9);
		f_write(&file, &buff, nRead, &nWritten);
		f_close(&file);
	}
}

void test_SD_2()
{
	volatile FRESULT res;
	static FATFS fs; 
	static FIL data_file; 
	UINT bw; 
	uint8_t str_cnt = 0;

	f_mount(0, &fs);

	res = f_open(&data_file, "test.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);

	if(res != FR_OK)
	{
		f_close(&data_file);
		return;
	}

	char buffer[5];

	buffer[0] = 'A';
	buffer[1] = 'B';
	buffer[2] = 'C';
	buffer[3] = 'D';
	buffer[4] = 'E';

	if(data_file.fsize)
	res = f_lseek(&data_file, data_file.fsize);  // goto to the end of file only,
						   // if file exist

	if(res != FR_OK)
	{
		f_close(&data_file);
		return;
	}

	for(str_cnt = 0; str_cnt < 10; str_cnt++)
	{
		res = f_write(&data_file, buffer, 5, &bw);
		if(res != FR_OK)
		{
			f_close(&data_file);
			return;
		}

		f_puts("арнрвфавф \r\n", &data_file);
	}

	f_close(&data_file);

	f_mount(0, 0);  
}


void SendSensorData(uint16_t sensorData[4], uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	DIR dir;
	static FATFS FATFS_Obj;
	static FIL file;

	result = f_mount(0, &FATFS_Obj);

	if (result == FR_OK)
	{
		//result = f_mkdir("Log_19.02.2020");
		f_open(&file, "0:Log_19.02.2020/Log_15-43-00.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\r", &file);//TODO: try \t symbols
		
		if(file.fsize)
		result = f_lseek(&file, file.fsize); 

		f_printf(&file, "%02d:%02d:%02d \t %03d \t %03d \t %03d \t %03d\r", hours, minutes, seconds,
				sensorData[0], sensorData[1], sensorData[2], sensorData[3]);

		f_close(&file);
	}
	
	f_mount(0, 0);
}


