#include "sdcard.h"
#include <string.h>

volatile FRESULT result;

static XCHAR CurrentLogFileName[17];
static XCHAR CurrentLogDirectoryName[17];
static XCHAR CurrentLogPath[35];

DWORD fre_clust, fre_sect, tot_sect;

int16_t AmperageBuffer[25];

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

int GetMaxValue(uint16_t * buffer)
{
	int max = buffer[0];
	int indexOfMaxValue = 0;
	
	for(int i = 0; i < 25; ++i)
	{
	    if(buffer[i] > max)
	    {
			max = buffer[i];		    
	    }
	}
	
	return max;
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
		
		if(fileInfo.fname[0] == 'L' && fileInfo.fname[1] != 'o' && fileInfo.fname[2] != 'g')
		{
			sprintf(nameOfMinimalDir[i], fileInfo.lfname);
			dateOfFileCreation[i] = fileInfo.fdate;
		
			i++;
		}
		else
		{
			if(fileInfo.fname[0] == 0) break; 
		}
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

void SendSensorDataToSDCard(int16_t * sensorData, RTC_DateTimeTypeDef* RTC_DateTimeStruct)
{
	static FATFS FATFS_Obj;
	static FIL file;
	FATFS *fs;
	
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
				PrintDataOnLCD("File Deleted", 0, 1);
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
			f_puts("Time of record \t U(V) \r\n", &file);
			
			int j = 0;
				
			if(file.fsize < MAXIMUM_FILE_SIZE && CurrentLogFileName[0] != 0)
			{	
				result = f_lseek(&file, file.fsize); 
									
				for(int i = 0; i < 50; i++)
				{
					f_printf(&file, "%02d:%02d:%02d.%03d \t %3d\n", hours, minutes, seconds, i * 20, 
							(int)(((float)sensorData[0] / 1467 - 1.56) * 201.61 * 1.414));
					
					AmperageBuffer[i] = sensorData[0];
					
					if(i % 25 == 0 && i != 0)
					{
						int maxAmperage = GetMaxValue(AmperageBuffer);
						OutputADCDataAtDisplay(maxAmperage);
						
						memset(AmperageBuffer, 0, sizeof(AmperageBuffer));
					}
				}
				
				BlinkBlueLed();
			}
			else 
			{			
				sprintf(CurrentLogFileName, "Log_%02d-%02d-%02d.txt", hours, minutes, seconds);
				sprintf(CurrentLogPath, "0:/%s/%s", CurrentLogDirectoryName, CurrentLogFileName);
				
				result = f_open(&file, CurrentLogPath, FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
				f_puts("Time of record \t Ua(V) \r\n", &file);	
				
				result = f_lseek(&file, file.fsize); 
					
				for(int i = 0; i < 50; i++)
				{
					f_printf(&file, "%02d:%02d:%02d.%03d \t %3d\n", hours, minutes, seconds, i * 20, 
							(int)(((float)sensorData[0] / 1467 - 1.56) * 201.61 * 1.414));				
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
    static FILINFO fileInfo;
	static XCHAR lfname[_MAX_LFN];
	
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


