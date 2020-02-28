#include "sdcard.h"

volatile FRESULT result;

static XCHAR CurrentLogFileName[17];
static XCHAR CurrentLogDirectoryName[17];
static XCHAR CurrentLogPath[35];

DWORD fre_clust, fre_sect, tot_sect;

DWORD isSDCardEmpty, freeClusters;

char	buff[1024];		

void SendSensorDataToSDCard(uint16_t sensorData[4], RTC_DateTimeTypeDef* RTC_DateTimeStruct)
{
	DIR dir;
	static FATFS FATFS_Obj;
	static FIL file;
      static FILINFO fileInfo;
	char filename[255];
	
	uint8_t hours = RTC_DateTimeStruct -> RTC_Hours;
	uint8_t minutes = RTC_DateTimeStruct -> RTC_Minutes;
	uint8_t seconds = RTC_DateTimeStruct -> RTC_Seconds;
		
	result = f_mount(0, &FATFS_Obj);

	if (result == FR_OK)
	{	
		sprintf(CurrentLogDirectoryName, "Log_%02d.%02d.%04d",  RTC_DateTimeStruct -> RTC_Date,  
		RTC_DateTimeStruct -> RTC_Month,  RTC_DateTimeStruct -> RTC_Year);
		result = f_mkdir(CurrentLogDirectoryName);
		if(result == 0)
		{
				sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);
				sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
		}
		
		result = f_open(&file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
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
			sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
			
			result = f_open(&file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
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

void GetCurrentLogFile(RTC_DateTimeTypeDef* RTC_DateTimeStruct)
{
	static DIR dir;
	static FATFS FATFS_Obj;
	FATFS *fs;
      static FILINFO fileInfo;
	static XCHAR lfname[_MAX_LFN];
	 DWORD fre_clust;
	
	fileInfo.lfname = lfname;
	fileInfo.lfsize = _MAX_LFN - 1; 
	
	sprintf(CurrentLogDirectoryName, "Log_%02d.%02d.%04d",  RTC_DateTimeStruct -> RTC_Date,  
		RTC_DateTimeStruct -> RTC_Month,  RTC_DateTimeStruct -> RTC_Year);
		
	result = f_mount(0, &FATFS_Obj);
	
	//result = f_unlink("0:/Log_01.03.2020");
	if (result == FR_OK)
	{
		result = f_mkdir(CurrentLogDirectoryName);
		result = f_opendir(&dir, CurrentLogDirectoryName);
		if(result == FR_OK)
		{
			for(;;)
			{
				result = f_readdir(&dir, &fileInfo);
				if ((result != FR_OK) || (fileInfo.fname[0] == 0))
				break;
				sprintf(CurrentLogFileName, fileInfo.lfname);
				sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
			}	
					
			if(fileInfo.fname[0] == 0)
			{

				result = f_getfree("/", &fre_clust, &fs);
				if(result == FR_OK)
				{
					fre_sect = fre_clust * fs->csize / 2;
				}				
			}
		}		
	}
	result = f_mount(0, 0);
}

void EraseSdCard(void)
{

}
