#include "sdcard.h"

volatile FRESULT result;

static XCHAR CurrentLogFileName[17];
static XCHAR CurrentLogDirectoryName[17];
static XCHAR CurrentLogPath[35];

// 1) возникновение папки 1970
// 2) создание файла при доставании флешки
// 3) удаление 
// 4) если сбилась дата на начальную что произойдет тогда?
// 5) проверить сработает ли вотчдог, при удалении файлов
// 6) вывод на дисплей - раз в секунду, по прерыванию/ либо 
// 	ничего не трогать если не сильно токи меняются
// 7) тактирование

DWORD fre_clust, fre_sect, tot_sect;

char StatusOfSdCard[][17] = 
{
	"OK              ",
	"SD card error   ", 	//FR_DISK_ERR
	"Filesystem error", 	//FR_INT_ERR
	"SD is not ready ", 	//FR_NOT_READY
	"No file in a dir", 	//FR_NO_FILE
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

void OutputSdCardStatusOnLCD(int status)
{	
	PrintDataOnLCD(StatusOfSdCard[status], 0, 0);
}

int GetIndexOfMinimalValue(int * array)
{
	int min = array[0];
	int indexOfminimalValue = 0;
	
	for(int i = 0; i < sizeof(array); ++i)
	{
	    if(array[i] < min && array[i] != 0)
	    {
		  min = array[i];
		 indexOfminimalValue = i;		    
	    }
	}
	
	return indexOfminimalValue;
}

void DeleteOldestDirectory(void)
{
	static DIR dir;
	static FILINFO fileInfo;
	static XCHAR lfname[_MAX_LFN];
	
	int dateOfFileCreation[60] = {'\0'};
	char nameOfMinimalDir[30][25] = {'\0'};
	int i = 0;
		
	XCHAR LogFileName[17];
	XCHAR DirectoryName[17];
	XCHAR LogPath[35];
	
	fileInfo.lfname = lfname;
	fileInfo.lfsize = _MAX_LFN - 1; 
	
	result = f_opendir(&dir, "/");
	while((result == FR_OK))
	{
		result = f_readdir(&dir, &fileInfo);
		dateOfFileCreation[i] = fileInfo.fdate;
		sprintf(nameOfMinimalDir[i], fileInfo.lfname);
		
		if(fileInfo.fname[0] == 0) break; 

		i++;	
	}
	
	int indexOfOldestDirectory = GetIndexOfMinimalValue(dateOfFileCreation);
	sprintf(DirectoryName, nameOfMinimalDir[indexOfOldestDirectory]);	
	
	result = f_opendir(&dir, DirectoryName);
	if(result == FR_OK)
	{
		do
		{
			result = f_readdir(&dir, &fileInfo);
			sprintf(LogFileName, fileInfo.lfname);
			sprintf(LogPath, "0:/%s/%s", DirectoryName, LogFileName);
			result = f_unlink(LogPath);
		}
		while((result == FR_OK) && (fileInfo.fname[0] != 0));
			
		result = f_unlink(DirectoryName);
	}	
}

void SendSensorDataToSDCard(uint16_t sensorData[4], RTC_DateTimeTypeDef* RTC_DateTimeStruct)
{
	static FATFS FATFS_Obj;
	static FIL file;
	FATFS *fs;

	char filename[255];
	
	uint8_t hours = RTC_DateTimeStruct -> RTC_Hours;
	uint8_t minutes = RTC_DateTimeStruct -> RTC_Minutes;
	uint8_t seconds = RTC_DateTimeStruct -> RTC_Seconds;
				
	result = f_mount(0, &FATFS_Obj);
	if (result == FR_OK)
	{	
		result = f_getfree("/", &fre_clust, &fs);
		if(result == FR_OK)
		{	
			fre_sect = fre_clust * fs->csize / 2;
			
			if(fre_sect < MINIMUM_FREE_SPACE_ON_SD_CARD)
			{
				DeleteOldestDirectory();
				//PrintDataOnLCD("File Deleted", 0, 0);
			}
			
			sprintf(CurrentLogDirectoryName, "Log_%02d.%02d.%04d",  RTC_DateTimeStruct -> RTC_Date,  
			RTC_DateTimeStruct -> RTC_Month,  RTC_DateTimeStruct -> RTC_Year);
			result = f_mkdir(CurrentLogDirectoryName); 
			if(result == 0)
			{
				sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);
				sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
			}
			
			result = f_open(&file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
			f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\n", &file);
			
			if(file.fsize < MAXIMUM_FILE_SIZE && CurrentLogFileName[0] != 0)
			{	
				result = f_lseek(&file, file.fsize); 
									
				for(int i = 0; i < 50; i++)
				{
					f_printf(&file, "%02d:%02d:%02d.%03d \t %03d \t %03d \t %03d \t %03d\n", hours, minutes, seconds, i * 20,
						sensorData[0], sensorData[1], sensorData[2], sensorData[3]);
				}
				
				BlinkBlueLed();
			}
			else 
			{			
				sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);
				sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
				
				result = f_open(&file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
				f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\n", &file);	
				
				result = f_lseek(&file, file.fsize); 
					
				for(int i = 0; i < 50; i++)
				{
					f_printf(&file, "%02d:%02d:%02d.%03d \t %03d \t %03d \t %03d \t %03d\n", hours, minutes, seconds, i * 20,
						sensorData[0], sensorData[1], sensorData[2], sensorData[3]);
				}
	
				BlinkBlueLed();				
			}
			f_close(&file);
		}			
		else
		{
			OutputSdCardStatusOnLCD(result);
		}
	}
	else
	{
		OutputSdCardStatusOnLCD(result);
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
		}		
	}
	result = f_mount(0, 0);
}


