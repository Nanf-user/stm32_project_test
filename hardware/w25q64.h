#ifndef __W25Q64_H
#define __W25Q64_H

#include "stm32f4xx.h"
#include "stdint.h"

/* --------------- W25Q64 Pin Definitions (SPI2, remapped MISO/MOSI) --------------- */
/* SCK  = PB13 (AF5),  MISO = PC2 (AF5),  MOSI = PC3 (AF5),  CS = PB12 (GPIO) */
#define W25Q_SPI        SPI2
#define W25Q_CS_PORT    GPIOB
#define W25Q_CS_PIN     GPIO_Pin_12

#define W25Q_CS_LOW()   GPIO_ResetBits(W25Q_CS_PORT, W25Q_CS_PIN)
#define W25Q_CS_HIGH()  GPIO_SetBits(W25Q_CS_PORT, W25Q_CS_PIN)

/* --------------- W25Q64 Commands --------------- */
#define W25Q_CMD_WRITE_ENABLE   0x06
#define W25Q_CMD_WRITE_DISABLE  0x04
#define W25Q_CMD_READ_SR1       0x05
#define W25Q_CMD_READ_SR2       0x35
#define W25Q_CMD_READ_DATA      0x03
#define W25Q_CMD_FAST_READ      0x0B
#define W25Q_CMD_PAGE_PROGRAM   0x02
#define W25Q_CMD_SECTOR_ERASE   0x20    /* 4KB */
#define W25Q_CMD_BLOCK_ERASE    0xD8    /* 64KB */
#define W25Q_CMD_CHIP_ERASE     0xC7
#define W25Q_CMD_READ_ID        0x9F    /* JEDEC ID */
#define W25Q_CMD_POWER_DOWN     0xB9
#define W25Q_CMD_RELEASE_PD     0xAB

/* --------------- W25Q64 Specs --------------- */
#define W25Q64_PAGE_SIZE    256
#define W25Q64_SECTOR_SIZE  4096
#define W25Q64_BLOCK_SIZE   65536
#define W25Q64_FLASH_SIZE   (8 * 1024 * 1024)   /* 8MB = 64Mbit */

/* --------------- Function Declarations --------------- */
void     W25Q64_Init(void);
uint32_t W25Q64_ReadID(void);
void     W25Q64_WaitBusy(void);
void     W25Q64_WriteEnable(void);

void     W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len);
void     W25Q64_WritePage(uint32_t addr, const uint8_t *buf, uint16_t len);
void     W25Q64_WriteData(uint32_t addr, const uint8_t *buf, uint32_t len);

void     W25Q64_EraseSector(uint32_t addr);
void     W25Q64_EraseChip(void);

void     W25Q64_PowerDown(void);
void     W25Q64_WakeUp(void);

#endif
