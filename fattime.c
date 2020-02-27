/* Martin Thomas 4/2009 */

#include "integer.h"
#include "fattime.h"

/*
DWORD get_fattime (void)
{
	DWORD res;
	RTC_t rtc;

	rtc_gettime( &rtc );
	
	res =  (((1980) << 25)
			| ( << 21)
			| ((DWORD)rtc.mday << 16)
			| (WORD)(rtc.hour << 11)
			| (WORD)(rtc.min << 5)
			| (WORD)(rtc.sec >> 1);

	return res;
}
*/

//Use custom get_fattime function
//Implement RTC get time here if you need it
DWORD get_fattime (void) {
	return	  ((DWORD)(2014 - 1980) << 25)	// Year 2014
			| ((DWORD)7 << 21)				// Month 7
			| ((DWORD)10 << 16)				// Mday 10
			| ((DWORD)16 << 11)				// Hour 16
			| ((DWORD)0 << 5)				// Min 0
			| ((DWORD)0 >> 1);				// Sec 0
}