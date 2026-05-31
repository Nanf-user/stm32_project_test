#ifndef __IMAGE_H
#define __IMAGE_H

#include "stm32f4xx.h"
#include "stdint.h"

/* Image parameters (240x320 RGB565) */
#define IMG_WIDTH       240
#define IMG_HEIGHT      320
#define IMG_SIZE        (IMG_WIDTH * IMG_HEIGHT * 2)  /* 153600 bytes */

/* Two image slots in W25Q64 */
#define IMG1_FLASH_ADDR  0x000000
#define IMG2_FLASH_ADDR  0x026000

/* Check if a flash slot has valid image data */
uint8_t Image_HasData(uint32_t addr);

/* Write pic.h (gImage_pic) to slot 2 if empty, returns 1 if image ready */
uint8_t Image_InitSlot2(void);

/* Display image from flash slot (slot1=横取模, slot2=纵取模) */
void Image_ShowSlot1(void);
void Image_ShowSlot2(void);

#endif
