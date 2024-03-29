/*
 * sd_card.c
 *
 *  Created on: 15 Jul 2022
 *      Author: Anthony Marshall, AW Embedded
 *
 *  High level Driver for FatFS file system (FAT32, LFN disabled)
 *    - Assumes setup using STM32CubeMX
 *    - Assumes following startup functions have been called:
 *      - MX_SDMMC1_SD_Init();
 *      - MX_DMA_Init(); if using DMA
 *      - MX_FATFS_Init();
 *      - MX_USART1_UART_Init(); if using UART for debugging via myprintf()
 *
 *   -------- Example usage --------
 *
 *   For reference see sd_example() function.
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h> //for va_list var arg functions
#include <stdbool.h>

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
    res = f_mkfs((TCHAR const*)SDPath, FM_FAT32, 0, workBuff, sizeof(workBuff));

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

/*TODO: Add some error handling returns */
FRESULT sd_append_file(char *name, char* dataStr, bool init, bool closeFile)
{
    FRESULT res;
    UINT bytesWrote;

    /* Keep these alive between function calls */
    static FIL file;
    static uint32_t countSync = 0;

    countSync++;

    /* Only open if first time in */
    if(init == true)
        res = f_open(&file, name, FA_OPEN_APPEND|FA_READ|FA_WRITE);

    /* Write data string to file */
    res = f_write(&file, dataStr, strlen(dataStr), &bytesWrote);

    /* Sync with file system after a number of iterations */
    /* No need to call if file to be closed, as this will sync */
    if((countSync == SYNC_NUM) && (closeFile == false))
    {
        res = f_sync(&file);
        countSync = 0;
    }

    /* Optional close file */
    if(closeFile == true)
    {
        res = f_close(&file);
        countSync = 0;
    }

    return res;
}



/*  Attempt to remove a file from the volume */
FRESULT sd_delete_file(char* name)
{
    FRESULT res;

    res = f_unlink(name);

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

    /* Mount SD before file operations - power up / card swap */
    res = sd_mount();
    if(res != FR_OK)
        myprintf("sd_mount error (%i)\r\n", res);
    else
        myprintf("SD Mounted\r\n");

    /* Retrieve drive space */
    res = sd_stats();
    if(res != FR_OK)
        myprintf("sd_stats error (%i)\r\n", res);

    /* Create a file in the root directory */
    res = sd_create_file("data.dat");
    if(res == FR_EXIST)
        myprintf("sd_create_file error : file already exists (%i)\r\n", res);
    else if(res != FR_OK)
        myprintf("sd_create_file error (%i)\r\n", res);
    else
        myprintf("**File: data.dat created**\r\n");

    /*  Attempt to remove a file from the volume */
    res = sd_delete_file("test.txt");
    if(res != FR_OK)
        myprintf("sd_delete_file error (%i)\r\n", res);
    else
        myprintf("File deleted\r\n");

    /* Test reading an existing file and read out data */
    res = sd_test_read();
    if(res != FR_OK)
        myprintf("sd_test_read error (%i)\r\n", res);

    /* Test creating a new file and writing to it */
    res = sd_test_write();
    if(res != FR_OK)
        myprintf("sd_test_write error (%i)\r\n", res);

    /* List files in root directory only */
    res = sd_scan();
    if(res != FR_OK)
        myprintf("sd_scan error (%i)\r\n", res);

    /* Data logging batch write test */
    sd_append_file("welp.txt", "Test append to file1\r\n", START_LOG, CONT_LOG);

    /* Bulk logging to open file */
    for(uint32_t i = 0; i < 998; i++)
    {
      sd_append_file("welp.txt", "Test append to file2\r\n", WRITE_LOG, CONT_LOG);
    }

    /* Stop logging to file */
    sd_append_file("welp.txt", "Test append to file3\r\n", WRITE_LOG, END_LOG);

    /* Unmount SD after file operations - power down / card swap */
    res = sd_unmount();
    if(res != FR_OK)
        myprintf("sd_unmount error (%i)\r\n", res);
    else
        myprintf("SD Unmounted\r\n");
}
