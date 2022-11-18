/*
 * sim7600.c
 *
 *  Created on: Nov 22, 2020
 *      Author: AlFly
 */

#include "sim7600.h"

//#define STATION_URL "alistairdickie.com/test.php"
#define STATION_URL "www.freeflightwx.com/station.php"
#define CSQ_CHECK_COUNTS 60 //try for 60 times for valid CSQ (this is equivalent to 2 minutes)

LineBuffer lineBuffer;
char line[128];
uint16_t csq;
char pbuffer[100];
uint8_t cgsn_flag;
uint32_t elapsedTime;

uint8_t sendMeasureFlag;

//uint8_t sendParamsToUpFlag;
uint8_t firstSent;

uint8_t csqCheckCount;


STATE readyState = STATE_INIT;
STATE cpinState = STATE_INIT;



STATE httpResponseState = STATE_INIT;



CMD_STATE cmdState = CMD_INIT;

POWER_STATE powerState = POWER_INIT;

WAIT_DATA wait;

uint16_t pauseTimeout = 0;
uint8_t pauseNextState;

uint8_t hold;


FFWX_Measurement measure;
char urlParamBuffer[200];
//uint8_t hasURLParams;

char cmdBuffer[200];

unsigned int httpStatusCode;
unsigned int httpDataLen;

int pos;

uint32_t returnIndex;


uint8_t errorMonitorFlag;

uint8_t cclkAttempts = 0;
uint8_t numToSend;
uint16_t pause;

uint8_t pauseRDYCount = 0;

uint8_t httpReadSuccessFlag = 0;
uint8_t httpReadWaitCount = 0;

uint8_t powerOffCount = 0;



void sim7600_configuration(void){




}


uint8_t sim7600_powerProcessor(void){
	//check power state - does the sim7600 have main power and is RDY?
	//returns 1 if all is ready, 0 if it is managing power state

	uint8_t result = 0;
	switch (powerState) {
		case POWER_INIT:

			readyState = STATE_INIT;
			cpinState = STATE_INIT;


			httpResponseState = STATE_INIT;

			cmdState = CMD_INIT;

			csq = 99;

			//LED On
			HAL_GPIO_WritePin(GPIO_OUT_LED_GPIO_Port, GPIO_OUT_LED_Pin,GPIO_PIN_SET);

			//set DTR
			Uart_SendString("SIM7600 Set DTR\r\n", pc_uart);
			HAL_GPIO_WritePin(GPIO_OUT_DTR_GPIO_Port, GPIO_OUT_DTR_Pin, GPIO_PIN_SET);


			Uart_SendString("SIM7600 Power Start \r\n", pc_uart);
			HAL_GPIO_WritePin(GPIO_OUT_RESET_GPIO_Port, GPIO_OUT_RESET_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIO_OUT_PWRKEY_GPIO_Port, GPIO_OUT_PWRKEY_Pin, GPIO_PIN_SET);
			HAL_GPIO_WritePin(GPIO_OUT_4_1EN_GPIO_Port, GPIO_OUT_4_1EN_Pin, GPIO_PIN_SET);

			timer5_ms_restart();
			powerState = POWER_KEY;
			result = 0;
			break;

		case POWER_KEY:
			if(timer5_ms_get() > 500){
				Uart_SendString("SIM7600 Power Key Down \r\n", pc_uart);
				HAL_GPIO_WritePin(GPIO_OUT_PWRKEY_GPIO_Port, GPIO_OUT_PWRKEY_Pin, GPIO_PIN_RESET);
				timer5_ms_restart();
				powerState = POWER_KEY2;
				result = 0;
			}
			break;

		case POWER_KEY2:
			if(timer5_ms_get() > 500){
				Uart_SendString("SIM7600 Power Key Up \r\n", pc_uart);
				HAL_GPIO_WritePin(GPIO_OUT_PWRKEY_GPIO_Port, GPIO_OUT_PWRKEY_Pin, GPIO_PIN_SET);
				powerState = POWER_SET;
				//LED Off
				HAL_GPIO_WritePin(GPIO_OUT_LED_GPIO_Port, GPIO_OUT_LED_Pin,GPIO_PIN_RESET);
				timer5_ms_restart();
				return 0;
			}
			break;

		case POWER_SET:
			elapsedTime = timer5_ms_get();
			if (HAL_GPIO_ReadPin(GPIO_IN_POWERSTAT_GPIO_Port, GPIO_IN_POWERSTAT_Pin)) {
				powerState = POWER_OK;
				sprintf(pbuffer, "SIM7600 Power Status OK after %lu ms\r\n", elapsedTime);
				Uart_SendString(pbuffer, pc_uart);
				result = 1;

			}

			if(timer5_ms_get() > 20000){
				sprintf(pbuffer, "SIM7600 Power Status Timeout after %lu ms\r\n", elapsedTime);
				Uart_SendString(pbuffer, pc_uart);
				powerState = POWER_ERR;
				result = 0;
			}
			break;

		case POWER_OK:
			if (!HAL_GPIO_ReadPin(GPIO_IN_POWERSTAT_GPIO_Port,GPIO_IN_POWERSTAT_Pin)) {
				Uart_SendString("SIM7600 Power Status Lost\r\n", pc_uart);
				powerState = POWER_ERR;
				result = 0;
				break;
			}
			else{
				result = 1;
			}


			break;

		case POWER_ERR:
			powerState = POWER_OFF;
			readyState = STATE_INIT;
			cpinState = STATE_INIT;
			httpResponseState = STATE_INIT;
			cmdState = CMD_INIT;
			result = 0;
			break;

		case POWER_OFF:
			powerOffCount++;
			sprintf(pbuffer, "SIM7600 Main Power Off %lu \r\n", powerOffCount);
			Uart_SendString(pbuffer, pc_uart);
			HAL_GPIO_WritePin(GPIO_OUT_4_1EN_GPIO_Port, GPIO_OUT_4_1EN_Pin, GPIO_PIN_RESET);
			timer5_ms_restart();

			if(powerOffCount >= 5){
				Uart_SendString("SIM7600 Main Power Off > 5 - Goto Shutdown\r\n", pc_uart);
				powerState = POWER_SCHEDULE_SHUTDOWN;
			}
			else{
				powerState = POWER_RESTART;
				result = 0;
			}

			break;

		case POWER_RESTART:
			if(timer5_ms_get() > 1000){
				powerState = POWER_INIT;
				result = 0;
			}
			break;

		case POWER_SCHEDULE_SHUTDOWN:
			scheduleShutdown(5000);
			powerState = POWER_WAIT_SHUTDOWN;
			scheduleShutdown(5000);
			result = 0;
			break;
		case POWER_WAIT_SHUTDOWN:
			result = 0;
			break;

	}


	return result;

}

void sim7600_commandProcessor(void){
	switch (cmdState){
		case CMD_INIT:
			if(readyState == STATE_OK){
				Uart_SendString("RDY State OK\r\n", pc_uart);
				pauseTimeout = 1000;
				pauseNextState = CMD_AT;
				cmdState = CMD_PAUSE;
				cgsn_flag = 0;
				setUpdateStationListFlag(1);
//				sendParamsToUpFlag = 0;
			}
			else if(pauseRDYCount < 10){
				Uart_SendString("Waiting for RDY\r\n", pc_uart);
				pauseTimeout = 1000;
				pauseNextState = CMD_INIT;
				cmdState = CMD_PAUSE;
				pauseRDYCount++;
			}
			else{
				Uart_SendString("Waiting for RDY Timed Out - Switching to Power Error\r\n", pc_uart);
				powerState = POWER_ERR;
			}



			break;

		case CMD_WAIT:
			if(!wait.waiting){
				setCommandState(wait.nextState_success);
//				Debug("Wait Success\r\n");
			}
			else if(timer2_ms_get() > wait.timeout){//we have timed out
				setCommandState(wait.nextState_timeout);
//				Debug("Wait Timeout\r\n");
			}
			break;

		case CMD_PAUSE:
			timer2_ms_restart();
//			Debug("Pause\r\n");
			setCommandState(CMD_PAUSE_WAIT);
			break;

		case CMD_PAUSE_WAIT:
			if(timer2_ms_get() > pauseTimeout){//we have timed out
//				Debug("Pause Timeout\r\n");
				setCommandState(pauseNextState);
			}
			break;

		case CMD_AT:
			Uart_SendString("Sending AT\r\n", pc_uart);
			sim7600_command_pause_wait("AT\r\n", 1000, "OK", CMD_CGSN, CMD_ERR, 10);
			break;

		case CMD_CGSN:
			//get imei
			cgsn_flag = 1;
			sim7600_command_pause_wait("AT+CGSN\r\n", 1000, "OK", CMD_CONFIG_CTZU, CMD_ERR, 10);
			break;

		case CMD_CONFIG_CTZU:
			//	AT+CTZU=1 set automatic time update from network so AT_CCLK works
			sim7600_command_pause_wait("AT+CTZU=1\r\n", 1000, "OK", CMD_CONFIG_CMGF, CMD_ERR, 10);
			break;

		case CMD_CONFIG_CMGF:
			//	AT+CMGF=1 SMS in text mode

//			sim7600_command_pause_wait("AT+CMGF=1\r\n", 1000, "OK", CMD_CCLK, CMD_ERR, 10);
			sim7600_command_pause_wait("AT+CMGF=1\r\n", 1000, "OK", CMD_CPIN, CMD_ERR, 10);
//			pauseCommmandSate(CMD_CPIN, 10);
			break;

//skip over getting time from network. Next two states not used...
		case CMD_CCLK:

			if(cclkAttempts >= 5){
				Uart_SendString("Timestate from network attempts > 5. Continuing regardless. Will try for server time.\r\n", pc_uart);
				pauseCommmandSate(CMD_CPIN, 10);
				cclkAttempts = 0;
				break;
			}

			sim7600_command_pause_wait("AT+CCLK?\r\n", 1000, "OK", CMD_CCLK_CHECK, CMD_ERR, 1000);

			break;

		case CMD_CCLK_CHECK:

			if(getTimeState() == TIMESTATE_OK_NITZ){
				Uart_SendString("Timestate from network success\r\n", pc_uart);
				pauseCommmandSate(CMD_CPIN, 10);
			}
			else{
				Uart_SendString("Timestate from network not received. Trying again\r\n", pc_uart);
				cclkAttempts ++;
				pauseCommmandSate(CMD_CCLK, 2000);
			}

			break;

//			skip to here

		case CMD_CPIN:
			sim7600_command_wait("AT+CPIN?\r\n", 1000, "OK", CMD_CPIN_CHECK, CMD_ERR);
			break;

		case CMD_CPIN_CHECK:
			if(cpinState == STATE_OK){
				csqCheckCount = 0;
				pauseCommmandSate(CMD_CSQ, 1000);

			}
			else{
				Uart_SendString("CPIN not ok. Check SIM Card is properly inserted\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 2000);
			}

			break;

		case CMD_CSQ:
			if(csqCheckCount > CSQ_CHECK_COUNTS){
				Uart_SendString("CSQ Check Counts exceeded, try to restart modem.\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 2000);

			}
			else{
				sim7600_command_wait("AT+CSQ\r\n", 1000, "OK\r\n", CMD_CSQ_CHECK, CMD_ERR);
				csqCheckCount++;
			}


			break;

		case CMD_CSQ_CHECK:
			if(csq == 99 || csq == 199){
				Uart_SendString("Signal Strength not known or not detectable (CSQ equals 99/199). Check 4G antenna\r\n", pc_uart);
				pauseCommmandSate(CMD_CSQ, 2000);

			}
			else{
				pauseCommmandSate(CMD_CGREG, 10);
				csqCheckCount = 0;
			}

			break;

		case CMD_CGREG:
			Uart_SendString("Send CGREG\r\n", pc_uart);
			sim7600_command_pause_wait("AT+CGREG?\r\n", 1000, "OK", CMD_COPS, CMD_ERR,100);
			break;

		case CMD_COPS:
			Uart_SendString("Send COPS\r\n", pc_uart);
			sim7600_command_pause_wait("AT+COPS?\r\n", 1000, "OK", CMD_CGACT, CMD_ERR,100);
			break;

		case CMD_CGACT:
			Uart_SendString("Send CGACT\r\n", pc_uart);
			sim7600_command_pause_wait("AT+CGACT=1,1\r\n", 10000, "OK", CMD_CMGR, CMD_ERR,100);
			break;

		case CMD_HTTP_INIT:

			numToSend = numMeasurementsToSend();
//			if(getUpdateStationListFlag() || sendParamsToUpFlag == 1){
			if(getUpdateStationListFlag()){

				settingsToParams(urlParamBuffer);
				Uart_SendString("Send HTTPINIT for settings\r\n", pc_uart);
				timer5_ms_restart();//start http timer
				sim7600_command_pause_wait("AT+HTTPINIT\r\n", 1000, "OK", CMD_HTTP_PARA, CMD_ERR,100);

			}
			else if(numToSend >= 1){

				Uart_SendString("Send HTTPINIT for measurements\r\n", pc_uart);
				timer5_ms_restart();//start http timer
				sim7600_command_pause_wait("AT+HTTPINIT\r\n", 1000, "OK", CMD_NEXT_MEASUREMENT, CMD_ERR,10);
			}
			else if(timer5_ms_get() > 120000){//we have tried for two minutes, check for sms
				Uart_SendString("Waiting for a long time in http init - jumping to check for sms. \r\n", pc_uart);
				sprintf(pbuffer, "Number of Measurements To Send = %u\r\n",numToSend);
				Uart_SendString(pbuffer, pc_uart);
				pauseCommmandSate(CMD_CMGR, 1000);

			}
			break;





		case CMD_NEXT_MEASUREMENT:
			sprintf(pbuffer, "Number of Measurements To Send = %u\r\n",numToSend);
			Uart_SendString(pbuffer, pc_uart);
			getMeasurementToSend(&measure);
			measure.csq = csq;
			measurementToParams(&measure, urlParamBuffer);
			returnIndex = 0;
			pauseCommmandSate(CMD_HTTP_PARA, 10);
			break;

		case CMD_HTTP_PARA:

			pos = 0;
			pos = appendStr(cmdBuffer, "AT+HTTPPARA=\"URL\",\"http://", pos);
			if(getUpdateStationListFlag()){//send the settings to the station.php

				pos = appendStr(cmdBuffer, STATION_URL,pos);
				setUpdateStationListFlag(0);
				sendMeasureFlag = 0;

			}
//			else if(sendParamsToUpFlag == 1){//send the settings to the up.php
//				pos = appendStr(cmdBuffer, getSettings()->URL,pos);
//				sendParamsToUpFlag = 2;//we are done, and do not need to do it again until it equals 1
//				sendMeasureFlag = 0;
//			}
			else{//send the currently loaded measure to the up.php
				pos = appendStr(cmdBuffer, getSettings()->URL,pos);
				sendMeasureFlag = 1;

			}

			pos = appendStr(cmdBuffer, urlParamBuffer,pos);
			pos = appendStr(cmdBuffer, "\"\r\n",pos);
//			hasURLParams=0;



			sim7600_command_pause_wait(cmdBuffer, 1000, "OK", CMD_HTTP_ACTION, CMD_ERR,10);


			break;

//		case CMD_IPADDR:
//			sim7600_command_pause_wait("AT+IPADDR\r\n", 1000, "OK", CMD_NEXT_MEASUREMENT, CMD_ERR,10);
//			break;

		case CMD_HTTP_ACTION:

			httpDataLen = 0;
			httpStatusCode = 0;
			errorMonitorFlag = 0;
			sim7600_command_wait("AT+HTTPACTION=0\r\n", 120000, "OK", CMD_HTTP_ACTION_WAIT, CMD_ERR);
			break;

		case CMD_HTTP_ACTION_WAIT:
			if(httpStatusCode == 200 && httpDataLen > 0){
				pauseCommmandSate(CMD_HTTP_READ, 10);

			}
			else if(httpStatusCode > 0){
				Uart_SendString("HTTP Status Code Not 200 - Restart SIM7600\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 1000);

			}
			else if(errorMonitorFlag){
				Uart_SendString("Error in HTTP Action - Perhaps invalid URL - Restart SIM7600\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 1000);
				errorMonitorFlag = 0;
			}
			else if(timer5_ms_get() > 110000){
				Uart_SendString("Error in HTTP Action - No server response after 110 seconds - Restart SIM7600\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 1000);
			}

			break;

		case CMD_HTTP_READ:

			sprintf(pbuffer, "AT+HTTPREAD=0,%u\r\n",httpDataLen);
			httpReadSuccessFlag = 0;
			httpReadWaitCount = 0;
			sim7600_command_wait(pbuffer, 120000, "OK", CMD_HTTP_READ_WAIT, CMD_ERR);


			break;

		case CMD_HTTP_READ_WAIT:
			if(httpReadSuccessFlag){
				Uart_SendString("HTTPRead Success\r\n", pc_uart);

				pause = 2250;//slow down unless we confirm sent
				if(sendMeasureFlag){
					if(confirmSent(returnIndex)){
//						if(sendParamsToUpFlag == 0){//if it = 0, and we have sent a measure successfully, the schedule a params send
//							sendParamsToUpFlag = 1;
//						}
						pause = 10;

					}
					sendMeasureFlag = 0;

				}


				if(numMeasurementsToSend()){
					pauseCommmandSate(CMD_NEXT_MEASUREMENT, 10);
				}
				else{
					pauseCommmandSate(CMD_HTTP_TERM, 10);
				}

			}
			else{
				httpReadWaitCount++;
				if(httpReadWaitCount > 25){
					Uart_SendString("HTTPRead Failure\r\n", pc_uart);
					pauseCommmandSate(CMD_ERR, 10);
				}
				else{
					pauseCommmandSate(CMD_HTTP_READ_WAIT, 10);
				}

			}

			break;

		case CMD_HTTP_TERM:



//			pauseCommmandSate(CMD_CMGR, pause);
			sim7600_command_pause_wait("AT+HTTPTERM\r\n", 1000, "OK", CMD_CMGR, CMD_ERR,pause);

			break;



		case CMD_CMGR:

			sprintf(pbuffer, "HTTP Time = %lu ms\r\n",timer5_ms_get());
			Uart_SendString(pbuffer, pc_uart);

			timer5_ms_restart();//start http timer again, so we can time out and come back here if needed from httpinit

			sim7600_command_pause_wait("AT+CMGL=\"ALL\"\r\n", 1000, "OK", CMD_CMGD, CMD_ERR,100);
			break;

		case CMD_CMGD:
			sim7600_command_pause_wait("AT+CMGD=,3\r\n", 1000, "OK", CMD_CSQ2, CMD_ERR,100);
			csqCheckCount = 0;
			break;

		case CMD_CSQ2:
			if(csqCheckCount > CSQ_CHECK_COUNTS){
				Uart_SendString("CSQ2 Check Counts exceeded, try to restart modem.\r\n", pc_uart);
				pauseCommmandSate(CMD_ERR, 2000);
			}
			else{
				sim7600_command_pause_wait("AT+CSQ\r\n", 1000, "OK\r\n", CMD_CSQ_CHECK2, CMD_ERR,100);
				csqCheckCount++;
			}
			break;

		case CMD_CSQ_CHECK2:
			if(csq == 99 || csq == 199){
				Uart_SendString("Signal Strength not known or not detectable (CSQ2 equals 99/199). Check 4G antenna\r\n", pc_uart);
				pauseCommmandSate(CMD_CSQ2, 2000);

			}
			else{
				pauseCommmandSate(CMD_HOLD_CHECK, 10);
				csqCheckCount = 0;
			}

			break;


		case CMD_ERR:
			Uart_SendString("Command not responded to successfully. Switch to Power Error\r\n", pc_uart);
			powerState = POWER_ERR;
			break;

		case CMD_HOLD_CHECK:
			if(hold){
				Uart_SendString("Hold\r\n", pc_uart);
				pauseCommmandSate(CMD_HOLD_CYCLE, 5000);

			}
			else{
				pauseCommmandSate(CMD_HTTP_INIT, 10);

			}

			break;

		case CMD_HOLD_CYCLE:
			pauseCommmandSate(CMD_HOLD_CHECK, 10);


			break;


	}

}

//void scheduleSendParamsToUp(void){
//	sendParamsToUpFlag = 0; //reset to 0 so it will schedule params send to up after the first successful measure.
//}

//Sends the command, then if it is successful (i.e. correct response received, pauses for 'pause' before going to the successState.
//If the response is not received within timeout then proceeds to the timeout state immediately.
void sim7600_command_pause_wait(char * cmd, uint32_t timeout, const char * response, uint8_t successState, uint8_t timeoutState, uint16_t pause){

	pauseTimeout = pause;
	pauseNextState = successState;
	sim7600_command_wait(cmd, timeout, response, CMD_PAUSE, timeoutState);

}

//Sends the command, then if it is successful proceeds to the successState immediately.
//If the response is not received within timeout then proceeds to the timeoutState immediately.
void sim7600_command_wait(char * cmd, uint32_t timeout, const char * response, uint8_t successState, uint8_t timeoutState){

    wait.response = response;
    wait.len = strlen(response);

    wait.waiting = 1;
    wait.nextState_success = successState;
    wait.nextState_timeout = timeoutState;
    wait.timeout = timeout;

    timer2_ms_restart();
    Uart_SendString(cmd, device_uart);
//    Debug(cmd);

    setCommandState(CMD_WAIT);

}

void sim7600_processChar(char c){
	lineBufferAdd(&lineBuffer, c);
	processLines();

	if(wait.waiting){
		if(c == wait.response[wait.index]){
			wait.index++;
			if(wait.index == wait.len){
				wait.waiting = 0;
			}
		}
		else{
			wait.index = 0;
		}
	}

}





void lineBufferAdd(LineBuffer *uLineBuffer, char c){
	if(!(uLineBuffer->numLines && uLineBuffer->lineWriteIndex == uLineBuffer->lineReadIndex)){//buffer is not full (assuming that there is at least one end line in there)
		uLineBuffer->line[uLineBuffer->lineWriteIndex++] = c;
		if (uLineBuffer->lineWriteIndex == LINE_BUFFER_DIM) {
			uLineBuffer->lineWriteIndex = 0;
		}
		if(c == '\n'){
			uLineBuffer->numLines++;
		}
	}
}

uint8_t lineBufferNumLines(LineBuffer *uLineBuffer){
	return uLineBuffer->numLines;

}

void lineBufferGetLine(LineBuffer *uLineBuffer, char * line){
	if(uLineBuffer->numLines){

		char c = 0;
		int i = 0;
		while(c != '\n'){
			c = uLineBuffer->line[uLineBuffer->lineReadIndex++];
			line[i] = c;
			i++;

			if (uLineBuffer->lineReadIndex == LINE_BUFFER_DIM) {
				uLineBuffer->lineReadIndex = 0;
			}
		}

		uLineBuffer->numLines--;
		line[i] = 0;//add eol char
	}
}



void processLines(void)
{


    if(lineBufferNumLines(&lineBuffer)){//we have a line ready to read


       lineBufferGetLine(&lineBuffer, line);
//       Uart_SendString("LINE:", pc_uart);
//       Uart_SendString(line, pc_uart);
//       Uart_SendString("\r\n", pc_uart);

       if(cgsn_flag){
    	   if(line[0] >= '0' && line [0] <= '9'){//first char is a number, assume this is the imei from cgsn

    		   int i = 0;
    		   while(line[i] >= '0' && line [i] <= '9'){
    			   getSettings()->MEI[i] = line[i];

    			   i++;
    		   }
    		   getSettings()->MEI[i] = 0;//terminate with null char

    		   Uart_SendString("Updated IMEI: ", pc_uart);
    		   Uart_SendString(getSettings()->MEI, pc_uart);
    		   Uart_SendString("\r\n", pc_uart);

    		   cgsn_flag = 0;
    	   }
       }

       if(line[0] == '+'){
           URCProcessor(line);
       }
       else if(line[0] == '$') {
    	   NMEAProcessor(line);
       }

       else if(!strncmp(line, "RDY", 3)) {
           readyState = STATE_OK;
       	   Uart_SendString("SIM7600 Ready\r\n", pc_uart);
       }
       else if(!strncmp(line, "ERROR", 5)) {
    	  Uart_SendString("SIM7600 ERROR\r\n", pc_uart);

		  errorMonitorFlag = 1;

	   }


    }
}

void NMEAProcessor(char * line){
	if(!strncmp(line, "$FFWX,2", 7)){
		//$FFWX,2,67,1607845207657
		//0123456789012345678901234
		returnIndex = util_atoi_n(line+8, 20);
	}
	else if(!strncmp(line, "$FFWX,3", 7)){
		//examples
		//$FFWX,3,URL=setting
		//$FFWX,3,SID=setting
		//$FFWX,3,CLK=setting
		//0123456789012345678901234
		settingsLineProcessor(line+8,1,1);
	}
	else if(!strncmp(line, "$FFWX,4", 7)){
		//examples
		//$FFWX,4,URL=setting
		//$FFWX,4,SID=setting
		//$FFWX,4,CLK=setting
		//0123456789012345678901234
		settingsLineProcessor(line+8,1,0);
	}
}



void URCProcessor(char * line){



    if(!strncmp(line, "+CSQ:", 5)) {
    	//+CSQ: 19,99
    	//012345678901

    	csq = util_atoi_n(line + 6 ,2);


    }


    else if(!strncmp(line, "+CPIN: READY", 12)) {
    	cpinState = STATE_OK;
    }

    else if(!strncmp(line, "+HTTPACTION:", 12)) {
    	//+HTTPACTION: 0,200,22
    	//0123456789012345678901234678
       	httpStatusCode = util_atoi_n(line + 15 ,3);
       	httpDataLen = util_atoi_n(line + 19,5);
   }

    else if(!strncmp(line, "+CCLK:", 6)) {

        //+CCLK: "80/01/06,11:00:15+44\"  invaid cclk time check for 80 in year
        //+CCLK: "20/11/28,12:23:16+44\" valid cclk time
    	//0123456789012345678901234678
    	setTime(line + 8, TIMESTATE_OK_NITZ);
    }

    else if(!strncmp(line, "+HTTPREAD: 0", 12)) {
    	httpReadSuccessFlag = 1;
    }

    else if(!strncmp(line, "+HTTPREAD:0", 11)) {
		httpReadSuccessFlag = 1;
	}

}







uint8_t getCSQ(void ){
	return csq;

}

void toggleHold(void){
	if(hold == 0){
		hold = 1;
		 Uart_SendString("Toggle Hold ON\r\n", pc_uart);
	}
	else{
		hold = 0;
		 Uart_SendString("Toggle Hold OFF\r\n", pc_uart);
	}
}

void Debug(char * line){
	if(1){
		Uart_SendString(line, pc_uart);
	}

}

void setCommandState(CMD_STATE newState){
//	Debug("\r\n#CommandSate=");
//	sprintf(pbuffer, "%u",newState);
//	Debug(pbuffer);
//	Debug("\r\n");

	cmdState = newState;
}

void pauseCommmandSate(CMD_STATE newState, uint32_t pause){
	pauseTimeout = pause;
	pauseNextState = newState;
	setCommandState(CMD_PAUSE);
}
