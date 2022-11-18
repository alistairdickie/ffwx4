/*
 * sim7600.h
 *
 *  Created on: Nov 22, 2020
 *      Author: AlFly
 */

#ifndef INC_SIM7600_H_
#define INC_SIM7600_H_

#define LINE_BUFFER_DIM 1024

#include "main.h"
#include "stm32f4xx_hal.h"
#include "stdio.h"
#include "string.h"
#include "UartRingbuffer.h"
#include "measure_buffer.h"
#include <settings.h>
#include "timeUtil.h"

typedef enum
{
	STATE_INIT=0,
    STATE_WAIT=1,
    STATE_OK= 2,
	STATE_SET= 3,
	STATE_ERR= 4,

} STATE;

//typedef enum
//{
//	CONFIG_INIT=0,
//	CONFIG_AT=1,
//	CONFIG_CTZU=2,
//	CONFIG_CMGF=3,
//	CONFIG_OK=4,
////	CONFIG_INIT=5,
////	CONFIG_INIT=6,
////	CONFIG_INIT=7,
//
//} CONFIG_STATE;

typedef enum
{
	POWER_INIT=0,
	POWER_KEY=1,
	POWER_KEY2=2,
	POWER_SET=3,
	POWER_OK=4,
	POWER_ERR=5,
	POWER_OFF=6,
	POWER_RESTART=7,
	POWER_SCHEDULE_SHUTDOWN=8,
	POWER_WAIT_SHUTDOWN=9,

} POWER_STATE;

typedef enum
{
	CMD_INIT=0,
    CMD_WAIT=1,
//    CMD_IDLE= 2,
	CMD_AT= 3,
	CMD_CCLK= 4,
	CMD_CCLK_CHECK=5,
	CMD_ERR=6,
	CMD_PAUSE=7,
	CMD_PAUSE_WAIT=8,
	CMD_CPIN=9,
	CMD_CPIN_CHECK=10,
	CMD_CSQ=11,
	CMD_CSQ_CHECK=12,
	CMD_CGREG=13,
	CMD_COPS=14,
	CMD_CGACT=15,
	CMD_HTTP_INIT=16,
	CMD_HTTP_PARA=17,
	CMD_HTTP_ACTION=18,
	CMD_HTTP_ACTION_WAIT=19,
	CMD_HTTP_READ=20,
	CMD_HTTP_TERM=21,
	CMD_CSQ2=22,
	CMD_CSQ_CHECK2=23,
	CMD_CMGR=24,
	CMD_CMGD=25,
	CMD_CONFIG_CTZU=26,
	CMD_CONFIG_CMGF=27,
	CMD_CGSN=28,
	CMD_HOLD_CHECK=29,
	CMD_HOLD_CYCLE=30,
	CMD_HTTP_READ_WAIT=31,
	CMD_NEXT_MEASUREMENT=32,
	//CMD_IPADDR=33,




} CMD_STATE;


typedef struct {

	char line[LINE_BUFFER_DIM];
	uint8_t numLines;
	uint8_t lineWriteIndex;
	uint8_t lineReadIndex;

}LineBuffer;



typedef void ( * WAIT_CALLBACK ) ();

typedef struct{

    const char * response;
    uint8_t index;
    uint8_t len;
    uint8_t nextState_success;
    uint8_t nextState_error;
    uint8_t nextState_timeout;
    uint32_t timeout;
    uint8_t waiting;
    WAIT_CALLBACK waitCallback;


} WAIT_DATA;






uint8_t sim7600_powerProcessor(void);

void sim7600_processChar(char c);

void lineBufferAdd(LineBuffer *uLineBuffer, char c);

uint8_t lineBufferNumLines(LineBuffer *uLineBuffer);

void lineBufferGetLine(LineBuffer *uLineBuffer, char *line);
void processLines(void);
void URCProcessor(char * line);
void NMEAProcessor(char * line);
uint8_t getCSQ(void);



void sim7600_command_wait(char * cmd, uint32_t timeout, const char * response, uint8_t successState, uint8_t timeoutState);
void sim7600_command_pause_wait(char * cmd, uint32_t timeout, const char * response, uint8_t successState, uint8_t timeoutState, uint16_t pause);

void Debug(char * line);

void setCommandState(CMD_STATE newState);
void pauseCommmandSate(CMD_STATE newState, uint32_t pause);
void sim7600_commandProcessor(void);

void scheduleSendParamsToUp(void);

void toggleHold(void);


#endif /* INC_SIM7600_H_ */
