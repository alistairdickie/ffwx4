#include "timeUtil.h"

char tbuffer[100];
TIMESTATE timeState = TIMESTATE_INIT;
EpochTime epochTime = {0};

RTC_TimeTypeDef sTime;
RTC_DateTypeDef sDate;

RTC_HandleTypeDef * p_hrtc;

void initTime(void){
	timeState = TIMESTATE_INIT;
	p_hrtc = getHRTC();
}

void setTime(char * line, TIMESTATE successTimestate){
	//+CCLK: "80/01/06,11:00:15+44\"  invaid cclk time check for 80 in year
	//+CCLK: "20/11/28,12:23:16+44\" valid cclk time

	//expect a line like this:
	//		  20/11/28,12:23:16+44
	//        012345678901234567890

	int year;
	int month;
	int day;
	int hour;
	int minute;
	int second;
	int utc_offset;//quarter hours - can be negative


	year = util_atoi_n(line ,2);//year starts at position 9, 2 chars.
	month = util_atoi_n(line + 3 ,2);
	day = util_atoi_n(line +  6,2);
	hour = util_atoi_n(line +  9,2);
	minute = util_atoi_n(line + 12 ,2);
	second = util_atoi_n(line + 15 ,2);
	utc_offset = util_atoi_n(line + 18 ,2);

	sprintf(tbuffer, "CCLK Year,%i\r\n",year);
	Uart_SendString(tbuffer, pc_uart);

	if(year == 80){//not valid time

		Uart_SendString("CCLK Time invalid - Year = 80\r\n", pc_uart);
		timeState = TIMESTATE_ERR;

	}
	else{
		Uart_SendString("CCLK Time valid\r\n", pc_uart);

		sprintf(tbuffer, "Valid time %i/%i/%i %i:%i:%i %i\r\n",year,month,day,hour,minute,second,utc_offset);
		Uart_SendString(tbuffer, pc_uart);

		struct tm dateTime;
		time_t time;
		struct tm * dateTime2_p;

		dateTime.tm_year = 100 + year;
		dateTime.tm_mon = month - 1;
		dateTime.tm_mday = day;
		dateTime.tm_hour = hour;
		dateTime.tm_min = minute;
		dateTime.tm_sec = second;

		time = mktime(&dateTime);

		sprintf(tbuffer, "EpochTime in Local,%i\r\n",(int)time);
		Uart_SendString(tbuffer, pc_uart);

		time -= (utc_offset * 15 * 60);

		sprintf(tbuffer, "EpochTime in UTC,%i\r\n",(int)time);
		Uart_SendString(tbuffer, pc_uart);

		dateTime2_p = gmtime(&time);


		sDate.Year = (uint8_t)(dateTime2_p->tm_year - 100);
		sDate.Month = (uint8_t)(dateTime2_p->tm_mon + 1);
		sDate.Date = (uint8_t)(dateTime2_p->tm_mday);
		sDate.WeekDay = 0;

		sTime.Hours = (uint8_t)(dateTime2_p->tm_hour);
		sTime.Minutes = (uint8_t)(dateTime2_p->tm_min);
		sTime.Seconds = (uint8_t)(dateTime2_p->tm_sec);
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;

		sprintf(tbuffer, "RTCTime %u/%u/%u %u:%u:%u %lu \r\n",(unsigned int)sDate.Year,(unsigned int)sDate.Month,(unsigned int)sDate.Date,(unsigned int)sTime.Hours,(unsigned int)sTime.Minutes,(unsigned int)sTime.Seconds, (unsigned long)time);
		Uart_SendString(tbuffer, pc_uart);

		RTC_HandleTypeDef * p_hrtc = getHRTC();

		HAL_RTC_SetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_SetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);

		timeState = successTimestate;
		getEpochTime(&epochTime);
	}

}



void getEpochTime(EpochTime * epoch){
	if(timeState == TIMESTATE_OK_NITZ || timeState == TIMESTATE_OK_SERVER){


		RTC_HandleTypeDef * p_hrtc = getHRTC();

		HAL_RTC_GetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
		HAL_RTC_GetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);

		struct tm dateTime;
		time_t time;

		dateTime.tm_year = 100 + sDate.Year;
		dateTime.tm_mon = sDate.Month - 1;
		dateTime.tm_mday = sDate.Date;
		dateTime.tm_hour = sTime.Hours;
		dateTime.tm_min = sTime.Minutes;
		dateTime.tm_sec = sTime.Seconds;

		time = mktime(&dateTime);

		uint32_t sub = sTime.SubSeconds;
		uint32_t fract = sTime.SecondFraction;



		uint32_t ms;
		ms = (unsigned int)(((float)fract-(float)sub)/((float)fract+1) * 1000.0);

		epoch->seconds = (uint32_t)time;
		epoch->ms = (uint16_t)ms;

//
//		uint32_t epoch_ms_new = time * 1000 + ms;
//
//		unsigned int diff = epoch_ms_new - epoch_ms;
//
//		epoch_ms = epoch_ms_new;


//		sprintf(pbuffer, "Fract:%u Sub:%u = %03u\r\n",(unsigned int)fract, (unsigned int)sub,ms);
//		Uart_sendstring(pbuffer, pc_uart);

		sprintf(tbuffer, "RTCTime %02u/%02u/%02u %02u:%02u:%02u.%03u %lu %03u\r\n",(unsigned int)sDate.Year,(unsigned int)sDate.Month,(unsigned int)sDate.Date,(unsigned int)sTime.Hours,(unsigned int)sTime.Minutes,(unsigned int)sTime.Seconds,(unsigned int)epoch->ms, (unsigned long)epoch->seconds, (unsigned int)epoch->ms);
		Debug(tbuffer);

	}
	else{
		epoch->seconds = 0;
		epoch->ms = 0;
	}


}

TIMESTATE getTimeState(void){
	return timeState;
}

void getYearCharDirName(char * buf){
	HAL_RTC_GetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);
	sprintf(buf, "/20%02u_%02u",(unsigned int)sDate.Year,(unsigned int)sDate.Month);
}

void getDateCharFileName(char * buf){
	HAL_RTC_GetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);
	sprintf(buf, "/20%02u_%02u/%02u%02u%02u.txt",(unsigned int)sDate.Year,(unsigned int)sDate.Month,(unsigned int)sDate.Year,(unsigned int)sDate.Month,(unsigned int)sDate.Date);
}

uint8_t getHour(void){
	HAL_RTC_GetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);
	return (uint8_t)sTime.Hours;
}

void getTimeChar(char * buf){
	HAL_RTC_GetTime(p_hrtc, &sTime, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(p_hrtc, &sDate, RTC_FORMAT_BIN);
	sprintf(buf, "%02u:%02u:%02u UTC",(unsigned int)sTime.Hours,(unsigned int)sTime.Minutes,(unsigned int)sTime.Seconds);
}


