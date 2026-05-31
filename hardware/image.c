#include "image.h"
#include "w25q64.h"
#include "mylcd.h"
#include "pic.h"

/* Check if a flash address has valid image data (not all 0xFF) */
uint8_t Image_HasData(uint32_t addr)
{
    uint8_t check[4];
    W25Q64_ReadData(addr, check, 4);
    return !(check[0] == 0xFF && check[1] == 0xFF &&
             check[2] == 0xFF && check[3] == 0xFF);
}

/* Write built-in image (pic.h) to W25Q64 flash slot 2 */
uint8_t Image_InitSlot2(void)
{
    uint32_t num_sectors, i;

    if (Image_HasData(IMG2_FLASH_ADDR))
        return 1;

    num_sectors = (IMG_SIZE + W25Q64_SECTOR_SIZE - 1) / W25Q64_SECTOR_SIZE;

    for (i = 0; i < num_sectors; i++) {
        W25Q64_EraseSector(IMG2_FLASH_ADDR + i * W25Q64_SECTOR_SIZE);
    }

    W25Q64_WriteData(IMG2_FLASH_ADDR, gImage_pic, IMG_SIZE);
    return 1;
}

/* Display slot 1 image (横取模, row-major) */
void Image_ShowSlot1(void)
{
    LCD_ShowImage_FromFlash(0, 0, IMG_WIDTH, IMG_HEIGHT, IMG1_FLASH_ADDR);
}

/* Display slot 2 image (纵取模, column-major) */
void Image_ShowSlot2(void)
{
    LCD_ShowImage_VerScan(0, 0, IMG_WIDTH, IMG_HEIGHT, IMG2_FLASH_ADDR);
}
