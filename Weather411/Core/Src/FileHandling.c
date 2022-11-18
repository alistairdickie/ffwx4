/*
 * File_Handling_RTOS.c
 *
 *  Created on: 14-May-2020
 *      Author: Controllerstech
 */

#include <FileHandling.h>
#include "stm32f4xx_hal.h"
#include "UartRingbuffer.h"






/* =============================>>>>>>>> NO CHANGES AFTER THIS LINE =====================================>>>>>>> */

FATFS fs;  // file system
FIL fil; // File
FIL open_fil; // File
FILINFO fno;
FRESULT fresult;  // result
UINT br, bw;  // File read/write count

/**** capacity related *****/
FATFS *pfs;
DWORD fre_clust;
uint32_t total, free_space;


void Send_Uart (char *string)
{

	Uart_SendString(string, pc_uart);
}



FRESULT Mount_SD (const TCHAR* path)
{
	fresult = f_mount(&fs, path, 1);
	if (fresult != FR_OK){
		Send_Uart ("ERROR!!! in mounting SD CARD...\r\n");
	}
	else{
		Send_Uart("SD CARD mounted successfully...\r\n");
	}
	return fresult;
}

FRESULT Unmount_SD (const TCHAR* path)
{
	fresult = f_mount(NULL, path, 1);
	if (fresult == FR_OK){
		Send_Uart ("SD CARD UNMOUNTED successfully...\r\n");
	}
	else{
		Send_Uart("ERROR!!! in UNMOUNTING SD CARD\r\n");
	}
	return fresult;

}

/* Start node to be scanned (***also used as work area***) */
FRESULT Scan_SD (char* pat)
{
    DIR dir;
    UINT i;
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s",pat);

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (fresult != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", fno.fname))) continue;
            	char *buf = malloc(30*sizeof(char));
            	sprintf (buf, "Dir: %s\r\n", fno.fname);
            	Send_Uart(buf);
            	free(buf);
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                fresult = Scan_SD(path);                     /* Enter the directory */
                if (fresult != FR_OK) break;
                path[i] = 0;
            }
            else
            {   /* It is a file. */
           	   char *buf = malloc(30*sizeof(char));
               sprintf(buf,"File: %s/%s\n", path, fno.fname);
               Send_Uart(buf);
               free(buf);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}

/* Only supports removing files from home directory */
FRESULT Format_SD (void)
{
    DIR dir;
    char *path = malloc(20*sizeof (char));
    sprintf (path, "%s","/");

    fresult = f_opendir(&dir, path);                       /* Open the directory */
    if (fresult == FR_OK)
    {
        for (;;)
        {
            fresult = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (fresult != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR)     /* It is a directory */
            {
            	if (!(strcmp ("SYSTEM~1", fno.fname))) continue;
            	fresult = f_unlink(fno.fname);
            	if (fresult == FR_DENIED) continue;
            }
            else
            {   /* It is a file. */
               fresult = f_unlink(fno.fname);
            }
        }
        f_closedir(&dir);
    }
    free(path);
    return fresult;
}




FRESULT Write_File (char *name, char *data)
{

	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\r\n", name);
		Send_Uart (buf);
	    free(buf);
	    return fresult;
	}

	else
	{
	    /* Create a file with read write access and open it */
	    fresult = f_open(&fil, name, FA_OPEN_EXISTING | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
	    	Send_Uart(buf);
	        free(buf);
	        return fresult;
	    }

	    else
	    {
	    	fresult = f_write(&fil, data, strlen(data), &bw);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d while writing to the FILE *%s*\r\n", fresult, name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}

	    	/* Close file */
	    	fresult = f_close(&fil);
	    	if (fresult != FR_OK)
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "ERROR!!! No. %d in closing file *%s* after writing it\r\n", fresult, name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}
	    	else
	    	{
	    		char *buf = malloc(100*sizeof(char));
	    		sprintf (buf, "File *%s* is WRITTEN and CLOSED successfully\r\n", name);
	    		Send_Uart(buf);
	    		free(buf);
	    	}
	    }
	    return fresult;
	}
}

FRESULT Read_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERRROR!!! *%s* does not exists\r\n", name);
		Send_Uart (buf);
		free(buf);
	    return fresult;
	}

	else
	{
		/* Open file to read */
		fresult = f_open(&fil, name, FA_READ);

		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
		    Send_Uart(buf);
		    free(buf);
		    return fresult;
		}

		/* Read data from the file
		* see the function details for the arguments */

		char *buffer = malloc(sizeof(f_size(&fil)));
		fresult = f_read (&fil, buffer, f_size(&fil), &br);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			free(buffer);
		 	sprintf (buf, "ERROR!!! No. %d in reading file *%s*\r\n", fresult, name);
		  	Send_Uart(buffer);
		  	free(buf);
		}

		else
		{
			Send_Uart(buffer);
			free(buffer);

			/* Close file */
			fresult = f_close(&fil);
			if (fresult != FR_OK)
			{
				char *buf = malloc(100*sizeof(char));
				sprintf (buf, "ERROR!!! No. %d in closing file *%s*\r\n", fresult, name);
				Send_Uart(buf);
				free(buf);
			}
			else
			{
				char *buf = malloc(100*sizeof(char));
				sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
				Send_Uart(buf);
				free(buf);
			}
		}
	    return fresult;
	}
}

FRESULT Create_File (char *name)
{
	fresult = f_stat (name, &fno);
	if (fresult == FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* already exists!!!!\r\n use Update_File \r\n",name);
		Send_Uart(buf);
		free(buf);
	    return fresult;
	}
	else
	{
		fresult = f_open(&fil, name, FA_CREATE_ALWAYS|FA_READ|FA_WRITE);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR!!! No. %d in creating file *%s*\r\n", fresult, name);
			Send_Uart(buf);
			free(buf);
		    return fresult;
		}
		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "*%s* created successfully\r\n Now use Write_File to write data\r\n",name);
			Send_Uart(buf);
			free(buf);
		}

		fresult = f_close(&fil);
		if (fresult != FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in closing file *%s*\r\n", fresult, name);
			Send_Uart(buf);
			free(buf);
		}
		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
			Send_Uart(buf);
			free(buf);
		}
	}
    return fresult;
}

FRESULT Update_File (char *name, char *data, uint16_t dataSize)
{
	/**** check whether the file exists or not ****/
//	fresult = f_stat (name, &fno);
//	if (fresult != FR_OK)
//	{
//		char *buf = malloc(100*sizeof(char));
//		sprintf (buf, "ERROR!!! *%s* does not exists\r\n", name);
//		Send_Uart (buf);
//		free(buf);
////	    return fresult;
//	}
//
//	else
//	{
		 /* Create a file with read write access and open it */
	    fresult = f_open(&fil, name, FA_OPEN_APPEND | FA_WRITE);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
	    	Send_Uart(buf);
	        free(buf);
	        return fresult;
	    }

	    /* Writing text */
	    fresult = f_write(&fil, data, dataSize, &bw);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in writing file *%s*\r\n", fresult, name);
	    	Send_Uart(buf);
	    	free(buf);
	    }

	    else
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "File *%s* UPDATED successfully\r\n", name);
	    	Send_Uart(buf);
	    	free(buf);
	    }

	    /* Close file */
	    fresult = f_close(&fil);
	    if (fresult != FR_OK)
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "ERROR!!! No. %d in closing file *%s*\r\n", fresult, name);
	    	Send_Uart(buf);
	    	free(buf);
	    }
	    else
	    {
	    	char *buf = malloc(100*sizeof(char));
	    	sprintf (buf, "File *%s* CLOSED successfully\r\n", name);
	    	Send_Uart(buf);
	    	free(buf);
	     }
//	}
    return fresult;
}

FRESULT Remove_File (char *name)
{
	/**** check whether the file exists or not ****/
	fresult = f_stat (name, &fno);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! *%s* does not exists\r\n", name);
		Send_Uart (buf);
		free(buf);
		return fresult;
	}

	else
	{
		fresult = f_unlink (name);
		if (fresult == FR_OK)
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "File *%s* has been removed successfully\r\n", name);
			Send_Uart (buf);
			free(buf);
		}

		else
		{
			char *buf = malloc(100*sizeof(char));
			sprintf (buf, "ERROR No. %d in removing *%s*\r\n",fresult, name);
			Send_Uart (buf);
			free(buf);
		}
	}
	return fresult;
}

FRESULT Create_Dir (char *name)
{
    fresult = f_mkdir(name);
    if (fresult == FR_OK)
    {
    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "Dir *%s* has been created successfully\r\n", name);
    	Send_Uart (buf);
    	free(buf);
    }
    else
    {
    	char *buf = malloc(100*sizeof(char));
    	sprintf (buf, "ERROR No. %d in creating directory *%s*\r\n", fresult,name);
    	Send_Uart(buf);
    	free(buf);
    }
    return fresult;
}

FRESULT Check_SD_Space (void)
{
    /* Check free space */
	fresult = f_getfree("", &fre_clust, &pfs);

    total = (uint32_t)((pfs->n_fatent - 2) * pfs->csize * 0.5);
    char *buf = malloc(30*sizeof(char));
    sprintf (buf, "SD CARD Total Size: \t%lu\r\n",total);
    Send_Uart(buf);
    free(buf);
    free_space = (uint32_t)(fre_clust * pfs->csize * 0.5);
    buf = malloc(30*sizeof(char));
    sprintf (buf, "SD CARD Free Space: \t%lu\r\n",free_space);
    Send_Uart(buf);
    free(buf);
    return fresult;
}

//Added parts for managing an open file

//opens a file for reading and writing if it exists, if not, creates it and keeps it open for reading and writing
FRESULT File_O_Open (char *name)
{

	fresult = f_open(&open_fil, name, FA_OPEN_ALWAYS|FA_READ|FA_WRITE);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! No. %d in opening file *%s*\r\n", fresult, name);
		Send_Uart(buf);
		free(buf);
	}
	else
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "File *%s* OPENED successfully\r\n",name);
		Send_Uart(buf);
		free(buf);
	}
	return fresult;
}

//closes the file
FRESULT File_O_Close ()
{

	/* Close file */
	fresult = f_close(&open_fil);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! No. %d in closing file\r\n", fresult);
		Send_Uart(buf);
		free(buf);
	}
	else
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "File CLOSED successfully\r\n");
		Send_Uart(buf);
		free(buf);
	 }
	return fresult;
}

//prints data to the open file
FRESULT File_O_Print (char *data)
{

	/* Writing text */
	fresult = f_write(&open_fil, data, strlen (data), &bw);
	if (fresult != FR_OK)
	{
		char *buf = malloc(100*sizeof(char));
		sprintf (buf, "ERROR!!! No. %d in writing file\r\n", fresult);
		Send_Uart(buf);
		free(buf);
	}

	else
	{
//		char *buf = malloc(100*sizeof(char));
//		sprintf (buf, "UPDATED successfully\r\n");
//		Send_Uart(buf);
//		free(buf);
	}
	return fresult;

}

//reads a line from the open file, returns 0 if there is nothing more to read
TCHAR* File_O_Read_Line (char *buffer)
{

	/* Reading line text */
	TCHAR* result;
	result = f_gets (
	 buffer, /* [OUT] Read buffer */
	  200,     /* [IN] Size of the read buffer */
	  &open_fil      /* [IN] File object */
	);

	return result;

}

