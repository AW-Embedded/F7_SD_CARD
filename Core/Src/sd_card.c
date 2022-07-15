/*
 * sd_card.c
 *
 *  Created on: 15 Jul 2022
 *      Author: Ant
 */
#include <string.h>
#include <stdio.h>
#include <stdarg.h> //for va_list var arg functions

#include "sd_card.h"
#include "main.h"
#include "usart.h"
#include "fatfs.h"

uint8_t workBuff[_MAX_SS];
uint8_t testReadBuf[45];


/* Mount drive, default root 0:/ */
FRESULT sd_mount()
{
    FRESULT res;

    res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 1);

    return res;
}

/* Unmount drive, default root 0:/ */
FRESULT sd_unmount()
{
    FRESULT res;

    res = f_mount(NULL, (TCHAR const*)SDPath, 1);

    return res;
}


/* List files in root directory only */
FRESULT sd_scan()
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    char path[20];

    myprintf("--- ROOT FILE LIST ---\r\n");

    sprintf(path, "%s",(TCHAR const*)SDPath);

    res = f_opendir(&dir, path);

    if (res == FR_OK)
    {
        for (;;)
        {
            res = f_readdir(&dir, &fno);

            /* Reached end of root directory */
            if (res != FR_OK || fno.fname[0] == 0)
            {
                break;
            }

            if (fno.fattrib & AM_DIR) /* Ignore any other directories */
            {
                continue;
            }
            else /* Print the file name */
            {
                myprintf("File: %s/%s\n", path, fno.fname);
            }
        }
        res = f_closedir(&dir);
    }
    return res;
}

/* Re-Format the whole drive - time consuming */
FRESULT sd_format()
{
    FRESULT res;
    res = f_mkfs((TCHAR const*)SDPath, FM_ANY, 0, workBuff, sizeof(workBuff));

    if(res != FR_OK)
    {
        myprintf("f_mkfs error (%i)\r\n", res);
    }
    return res;
}

/* Retrieve drive space */
FRESULT sd_stats()
{
    FRESULT res;
    DWORD free_clusters, free_sectors, total_sectors;
    FATFS* getFreeFs;

    res = f_getfree("", &free_clusters, &getFreeFs);
    if(res != FR_OK)
    {
        return res;
    }

    /* Formula comes from ChaN's documentation */
    total_sectors = (getFreeFs->n_fatent - 2) * getFreeFs->csize;
    free_sectors = free_clusters * getFreeFs->csize;
    myprintf("SD card stats:\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", total_sectors / 2, free_sectors / 2);

    return res;
}


/* Create a file in the root directory */
FRESULT sd_create_file(char *name)
{
    FRESULT res;
    FIL fil;

    /* Returns FR_EXIST if the file already exists */
    res = f_open(&fil, name, FA_CREATE_NEW|FA_READ|FA_WRITE);
    if(res != FR_OK)
    {
        return res;
    }

    res = f_close(&fil);

    return res;
}

/*
 * Test functions & Example usage
 */

/* Test reading an existing file and read out data */
FRESULT sd_test_read()
{
    FRESULT res;
    myprintf("--- TEST READ FILE ---\r\n");

    res = f_open(&SDFile, "test.txt", FA_READ);

    if(res != FR_OK)
    {
        return res;
    }

    myprintf("**Opened file: 'test.txt'**\r\n");

    TCHAR* rres = f_gets((TCHAR*)testReadBuf, 45, &SDFile);
    if(rres != 0)
    {
        myprintf("Read 45 Bytes from 'test.txt': %s\r\n", testReadBuf);
    }
    else
    {
        myprintf("f_gets error (%i)\r\n", rres);
    }

    res = f_close(&SDFile);
    if(res != FR_OK)
    {
        return res;
    }
    myprintf("**Closed file: 'test.txt'**\r\n");

    return res;
}

/* Test creating a new file and writing to it */
FRESULT sd_test_write()
{
    FRESULT res;

    myprintf("--- TEST WRITE FILE ---\r\n");
    res = f_open(&SDFile, "write.txt", FA_WRITE | FA_OPEN_ALWAYS | FA_CREATE_ALWAYS);
    if(res != FR_OK)
    {
        return res;
    }
    myprintf("**Opened file: 'write.txt'**\r\n");

    strncpy((char*)testReadBuf, "a new file is made!", 20);
    UINT bytesWrote;

    res = f_write(&SDFile, testReadBuf, 20, &bytesWrote);

    if(res == FR_OK)
    {
        myprintf("Wrote %i bytes to 'write.txt'\r\n", bytesWrote);
    }
    else
    {
        return res;
    }

    res = f_close(&SDFile);
    if(res != FR_OK)
    {
        return res;
    }

    myprintf("**Closed file: 'write.txt'**\r\n");
    return res;
}

/* Usage examples */
void sd_example()
{
    FRESULT res;

    res = sd_mount();
    if(res != FR_OK)
        myprintf("sd_mount error (%i)\r\n", res);
    else
        myprintf("SD Mounted\r\n");

    res = sd_stats();
    if(res != FR_OK)
        myprintf("sd_stats error (%i)\r\n", res);

    res = sd_test_read();
    if(res != FR_OK)
        myprintf("sd_test_read error (%i)\r\n", res);

    res = sd_test_write();
    if(res != FR_OK)
        myprintf("sd_test_write error (%i)\r\n", res);

    res = sd_scan();
    if(res != FR_OK)
        myprintf("sd_scan error (%i)\r\n", res);

    res = sd_unmount();
    if(res != FR_OK)
        myprintf("sd_unmount error (%i)\r\n", res);
    else
        myprintf("SD Unmounted\r\n");
}
