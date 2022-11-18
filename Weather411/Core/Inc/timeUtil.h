/*
 * time.h
 *
 *  Created on: 27 Dec 2020
 *      Author: AlFly
 */

#ifndef INC_TIME_H_
#define INC_TIME_H_

#include <settings.h>
#include "main.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "UartRingbuffer.h"
#include "measure_buffer.h"
#include "util.h"
#include "sim7600.h"
#include "time.h"

typedef enum
{
	TIMESTATE_INIT=0,
    TIMESTATE_OK_NITZ=1,
    TIMESTATE_OK_SERVER= 2,
	TIMESTATE_ERR= 4,

} TIMESTATE;




void initTime(void);

void setTime(char * line, TIMESTATE successTimestate);
void getEpochTime(EpochTime * epoch);
TIMESTATE getTimeState(void);

void getYearCharDirName(char * buf);

void getDateCharFileName(char * buf);

uint8_t getHour(void);

void getTimeChar(char * buf);

#endif /* INC_TIME_H_ */
