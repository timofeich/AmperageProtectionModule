#include "ff.h"
#include "stm32f10x_conf.h"

volatile FRESULT result;
static XCHAR CurrentLogFileName[17];
DWORD isSDCardEmpty, freeClusters;
char	buff[1024];		

void SendSensorData(uint16_t sensorData[4], uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	DIR dir;
	static FATFS FATFS_Obj;
	static FIL file;
      static FILINFO fileInfo;
	char filename[255];
	
	result = f_mount(0, &FATFS_Obj);

	if (result == FR_OK)
	{

		result = f_open(&file, CurrentLogFileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\r\n", &file);
		
		if(file.fsize < 5000000 && CurrentLogFileName[0] != 0)//&& isSDCardEmpty != 0
		{	
			result = f_lseek(&file, file.fsize); 
				
			for(int i = 0; i < 50; i++)
			{
				f_printf(&file, "%02d:%02d:%02d.%03d \t %03d \t %03d \t %03d \t %03d\r\n", hours, minutes, seconds, i * 20,
					sensorData[0], sensorData[1], sensorData[2], sensorData[3]);
			}
		}
		else 
		{			
			sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);

			result = f_open(&file, CurrentLogFileName, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\r\n", &file);	
			
			result = f_lseek(&file, file.fsize); 
				
			for(int i = 0; i < 50; i++)
			{
				f_printf(&file, "%02d:%02d:%02d.%03d \t %03d \t %03d \t %03d \t %03d\r\n", hours, minutes, seconds, i * 20,
					sensorData[0], sensorData[1], sensorData[2], sensorData[3]);
			}			
		}
			
		f_close(&file);
	}
	
	f_mount(0, 0);
}

void GetCurrentLogFile(void)
{
	static DIR dir;
	static FATFS FATFS_Obj;
	static FIL file;
      static FILINFO fileInfo;
	static XCHAR lfname[_MAX_LFN];
	
	fileInfo.lfname = lfname;
	fileInfo.lfsize = _MAX_LFN - 1; 
	
	result = f_mount(0, &FATFS_Obj);
	if (result == FR_OK)
	{
		result = f_mkdir("123");
		result = f_opendir(&dir, "0:123");

		for(;;)
		{
			result = f_readdir(&dir, &fileInfo);
			if ((result != FR_OK) || (fileInfo.fname[0] == 0))
			break;
			sprintf(CurrentLogFileName, fileInfo.lfname);	
		}	
				
		if(fileInfo.fname[0] == 0)
		{
			isSDCardEmpty = FATFS_Obj.csize * ((FATFS_Obj.max_clust - 2) - FATFS_Obj.free_clust);
		}
		
		f_close(&file);		
	}
	result = f_mount(0, 0);
}
