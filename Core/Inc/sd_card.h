/*
 * sd_card.h
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

#ifndef INC_SD_CARD_H_
#define INC_SD_CARD_H_

#include "fatfs.h"

FRESULT sd_mount();
FRESULT sd_unmount();
FRESULT sd_scan();
FRESULT sd_format();
FRESULT sd_stats();
FRESULT sd_create_file(char *name);
FRESULT sd_delete_file(char* name);

FRESULT sd_test_read();
FRESULT sd_test_write();
void sd_example();

#endif /* INC_SD_CARD_H_ */
