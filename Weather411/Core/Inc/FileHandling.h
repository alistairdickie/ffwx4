/*
 * File_Handling_RTOS.h
 *
 *  Created on: 14-May-2020
 *      Author: Controllerstech
 */

#ifndef FILE_HANDLING_RTOS_H_
#define FILE_HANDLING_RTOS_H_

#include "fatfs.h"
#include "string.h"
#include "stdio.h"



/* mounts the sd card*/
FRESULT Mount_SD (const TCHAR* path);

/* unmounts the sd card*/
FRESULT Unmount_SD (const TCHAR* path);

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_SD (char* pat);

/* Only supports removing files from home directory. Directory remover to be added soon */
FRESULT Format_SD (void);

/* write the data to the file
 * @ name : is the path to the file*/
FRESULT Write_File (char *name, char *data);

/* read data from the file
 * @ name : is the path to the file*/
FRESULT Read_File (char *name);

/* creates the file, if it does not exists
 * @ name : is the path to the file*/
FRESULT Create_File (char *name);

/* Removes the file from the sd card
 * @ name : is the path to the file*/
FRESULT Remove_File (char *name);

/* creates a directory
 * @ name: is the path to the directory
 */
FRESULT Create_Dir (char *name);

/* checks the free space in the sd card*/
FRESULT Check_SD_Space (void);

/* updates the file. write pointer is set to the end of the file
 * @ name : is the path to the file
 */
FRESULT Update_File (char *name, char *data, uint16_t dataSize);

//file o methods

FRESULT File_O_Open (char *name);

//opens a file for reading and writing if it exists, if not, creates it and keeps it open for reading and writing
FRESULT File_O_Close (void);

//prints data to the open file
FRESULT File_O_Print (char *data);

//reads a line from the open file, returns 0 if there is nothing more to read
TCHAR* File_O_Read_Line (char *buffer);




#endif /* FILE_HANDLING_RTOS_H_ */
