# F7_SD_CARD
 
SD Card example project for STM32F746G-DISCO with card insertion detect.

- STM32Cube FW_F7 V1.17.0
- STM32CubeMX V6.6.1
- FatFS R0.12c

If you get an error whilst running the code for the first time with an SD card inserted:
- Format maybe required, edit the code in main.c to call sd_format() - note this is time consuming with a large SD card
- Once formatted you can remove the sd_format() call and add sd_example() at /* FatFS operations test code here */
- sd_unmount() call can then be removed from main.c as it is called from sd_example()

Note. The sd_example() code may require some fles to already be on the SD card, in case you get errors.

See top of sd_card.c for more description, full setup can be found in CubeMX, just open the .ioc file.