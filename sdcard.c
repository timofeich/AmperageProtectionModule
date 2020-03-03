#include "sdcard.h"

volatile FRESULT result;

static XCHAR CurrentLogFileName[17];
static XCHAR CurrentLogDirectoryName[17];
static XCHAR CurrentLogPath[35];	

uint8_t IsSdCardFull(FATFS *fs)
{
	DWORD fre_Clusters, fre_Sectors;
	if(f_getfree("/", &fre_Clusters, &fs) == FR_OK)
	{
		fre_Sectors = fre_Clusters * fs->csize / 2;
		
		if(fre_Sectors < MINIMUM_FREE_SPACE_ON_SD_CARD)
		{
			return 0;
		}
		else return 1;
	}
	else	return 1;
}

uint8_t CreateTodayDateDirectory(RTC_DateTimeTypeDef* RTC_DateTimeStruct)
{
	sprintf(CurrentLogDirectoryName, "Log_%02d.%02d.%04d",  RTC_DateTimeStruct -> RTC_Date,  
		RTC_DateTimeStruct -> RTC_Month,  RTC_DateTimeStruct -> RTC_Year);
	return f_mkdir(CurrentLogDirectoryName);
}

void CreateFileWithHeader(FIL *file)
{
	result = f_open(file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
	f_puts("Time of record \t Ia(A) \t Ib(A) \t Ic(A) \t U(V)\r\n", file);
}

void SetFilePath(uint8_t hours, uint8_t minutes, uint8_t seconds)
{
	sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);
	sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
}	

void DeleteFirstDirectory(DIR *directory, FILINFO *fileInformation)
{
	XCHAR LogFileName[17];
	XCHAR DirectoryName[17];
	XCHAR LogPath[35];
	
	f_opendir(directory, "/");
	result = f_readdir(directory, fileInformation);
	
	while((result == FR_OK) && (fileInformation -> fname[0] != 'L'))
	{
		result = f_readdir(directory, fileInformation);	
	}
	
	sprintf(DirectoryName, fileInformation -> lfname);
	
	if(f_opendir(directory, DirectoryName) == FR_OK)
	{
		for(;;)
		{
			result = f_readdir(directory, fileInformation);
			if ((result != FR_OK) || (fileInformation -> fname[0] == 0))
			break;
			
			sprintf(LogFileName, fileInformation -> lfname);
			sprintf(LogPath, "0:/%s/%s", DirectoryName, LogFileName);
			f_unlink(LogPath);
		}	
		f_unlink(DirectoryName);
	}	
}

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

	if (f_mount(0, &FATFS_Obj) == FR_OK)
	{	
		if(IsSdCardFull(&FATFS_Obj) == 0)
		{
			DeleteFirstDirectory(&dir, &fileInfo);
		}	
		
		if(CreateTodayDateDirectory(RTC_DateTimeStruct) == FR_OK)
		{
			SetFilePath(hours, minutes, seconds);
		}
		
		CreateFileWithHeader(&file);
		
		if(file.fsize < MAXIMUM_FILE_SIZE && CurrentLogFileName[0] != 0)
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
			SetFilePath(hours, minutes, seconds);	
			CreateFileWithHeader(&file);
			
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
      static FILINFO fileInfo;
	static XCHAR lfname[_MAX_LFN];
	FATFS *fs;
	
	fileInfo.lfname = lfname;
	fileInfo.lfsize = _MAX_LFN - 1; 
			
	if (f_mount(0, &FATFS_Obj) == FR_OK)
	{
		if(IsSdCardFull(fs) == 0)
		{
			DeleteFirstDirectory(&dir, &fileInfo);
		}	
		
		CreateTodayDateDirectory(RTC_DateTimeStruct);
		if(f_opendir(&dir, CurrentLogDirectoryName) == FR_OK)
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
	f_mount(0, 0);
}

