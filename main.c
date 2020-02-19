#include "stm32f10x_conf.h"
#include "ff.h"

char	buff[1024];		// буфер для чтения/записи
    volatile FRESULT result;
void test_SD_1(void)
{
 // FatFs function common result code
    static FATFS FATFS_Obj; // Work area (file system object) for logical drives
    static FIL file; // file objects

    // смонтировать диск
    result = f_mount(0, &FATFS_Obj);	//f_mount(&FATFS_Obj, "0", 1);

    if (result == FR_OK)
    	{
    		//printf("Ошибка монтирования диска %d\r\n", result);
    	}

    // открываем файл readme.txt для чтения
    UINT nRead, nWritten;

    result = f_open(&file, "readme.txt", FA_OPEN_EXISTING | FA_READ);
    if (result == FR_OK)
    	{
		GPIOC->ODR ^= (GPIO_Pin_8 | GPIO_Pin_9);
    	    f_read(&file, &buff, 1024, &nRead);
    	    f_close(&file);
    	}

    // создаем файл write.txt
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
   volatile FRESULT res; // FatFs function common result code
   static FATFS fs; // Work area (file system object) for logical drives
   static FIL data_file; // file objects
   UINT bw; // File read/write count
   uint8_t str_cnt = 0;

   // Register a work area for logical drive 0
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

   // Unregister a work area before discard it
   f_mount(0, 0);
   
}

void test_SD_3()
{
	DIR dir;
   static FATFS FATFS_Obj; // Work area (file system object) for logical drives
    static FIL file; // file objects

    // смонтировать диск
    result = f_mount(0, &FATFS_Obj);	//f_mount(&FATFS_Obj, "0", 1);

    if (result == FR_OK)
    	{
    		//printf("Ошибка монтирования диска %d\r\n", result);
    	}

    // создаем файл write.txt
    result = f_mkdir("Log_19.02.2020");
	
    if (result == FR_EXIST)
    {
		result = f_opendir(&dir, "Log_19.02.2020");
	    
		f_open(&file, "Log_15-43-00.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
		f_puts("Ia(A)	Ib(A)	Ic(A)	U(V)\n", &file);
		f_printf(&file, "%03d	%03d	%03d	%03d\n", 100, 200, 300, 400);
    }
	//f_closedir(&dir);
      f_close(&file);

   // Unregister a work area before discard it
	f_mount(0, 0);
}

void SetLEDsPins(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_9 | GPIO_Pin_8);
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOC , &GPIO_InitStructure);
}

int main(void)
{
	SetLEDsPins();
    
  //  	    test_SD_1();
	    test_SD_3();
		GPIOC->ODR ^= (GPIO_Pin_8 | GPIO_Pin_9);
    while(1)
    {

    }
}
