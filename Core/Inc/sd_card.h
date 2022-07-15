/*
 * sd_card.h
 *
 *  Created on: 15 Jul 2022
 *      Author: Ant
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
