#include "w25q64.h"

/* --------------- SPI2 Send/Recv (Full Duplex) --------------- */
static uint8_t SPI2_SendRecv(uint8_t data)
{
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI2, data);
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
    return SPI_I2S_ReceiveData(SPI2);
}

/* --------------- W25Q64 Init (SPI2 + GPIO) --------------- */
/*
 * Pin mapping (remapped MISO/MOSI):
 *   SCK  = PB13 (SPI2_SCK,  AF5)
 *   MISO = PC2  (SPI2_MISO, AF5)  ← remapped from PB14
 *   MOSI = PC3  (SPI2_MOSI, AF5)  ← remapped from PB15
 *   CS   = PB12 (GPIO output)
 */
void W25Q64_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    /* Enable clocks */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    /* CS pin: PB12 as output push-pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    W25Q_CS_HIGH();   /* Deselect */

    /* SCK: PB13 as AF5 (SPI2_SCK) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);

    /* MISO: PC2 as AF5 (SPI2_MISO) — remapped from PB14 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_SPI2);

    /* MOSI: PC3 as AF5 (SPI2_MOSI) — remapped from PB15 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_3;
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_SPI2);

    /* SPI2 configuration: Mode 0, 8-bit, Master */
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;       /* Mode 0 */
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;     /* Mode 0 */
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  /* 42MHz/4 = 10.5MHz */
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial     = 7;
    SPI_Init(SPI2, &SPI_InitStructure);
    SPI_Cmd(SPI2, ENABLE);

    /* Dummy send to stabilize */
    SPI2_SendRecv(0xFF);
}

/* --------------- Read JEDEC ID (expect 0xEF4017 for W25Q64) --------------- */
uint32_t W25Q64_ReadID(void)
{
    uint32_t id = 0;

    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_READ_ID);
    id  = (uint32_t)SPI2_SendRecv(0xFF) << 16;   /* Manufacturer ID */
    id |= (uint32_t)SPI2_SendRecv(0xFF) << 8;    /* Memory Type */
    id |= (uint32_t)SPI2_SendRecv(0xFF);          /* Capacity */
    W25Q_CS_HIGH();

    return id;
}

/* --------------- Wait until W25Q64 is not busy --------------- */
void W25Q64_WaitBusy(void)
{
    uint8_t status;
    do {
        W25Q_CS_LOW();
        SPI2_SendRecv(W25Q_CMD_READ_SR1);
        status = SPI2_SendRecv(0xFF);
        W25Q_CS_HIGH();
    } while (status & 0x01);   /* BUSY bit */
}

/* --------------- Write Enable --------------- */
void W25Q64_WriteEnable(void)
{
    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_WRITE_ENABLE);
    W25Q_CS_HIGH();
}

/* --------------- Read Data (0x03) --------------- */
void W25Q64_ReadData(uint32_t addr, uint8_t *buf, uint16_t len)
{
    uint16_t i;

    W25Q64_WaitBusy();
    W25Q_CS_LOW();

    SPI2_SendRecv(W25Q_CMD_READ_DATA);
    SPI2_SendRecv((addr >> 16) & 0xFF);   /* A23-A16 */
    SPI2_SendRecv((addr >> 8)  & 0xFF);   /* A15-A8  */
    SPI2_SendRecv(addr & 0xFF);            /* A7-A0   */

    for (i = 0; i < len; i++) {
        buf[i] = SPI2_SendRecv(0xFF);
    }

    W25Q_CS_HIGH();
}

/* --------------- Page Program (0x02) - max 256 bytes --------------- */
void W25Q64_WritePage(uint32_t addr, const uint8_t *buf, uint16_t len)
{
    uint16_t i;

    if (len == 0 || len > W25Q64_PAGE_SIZE) return;

    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_PAGE_PROGRAM);
    SPI2_SendRecv((addr >> 16) & 0xFF);
    SPI2_SendRecv((addr >> 8)  & 0xFF);
    SPI2_SendRecv(addr & 0xFF);

    for (i = 0; i < len; i++) {
        SPI2_SendRecv(buf[i]);
    }
    W25Q_CS_HIGH();

    W25Q64_WaitBusy();   /* Wait for page program to complete (~3ms) */
}

/* --------------- Write Data (auto page-boundary split) --------------- */
void W25Q64_WriteData(uint32_t addr, const uint8_t *buf, uint32_t len)
{
    uint32_t i = 0;
    uint16_t page_remain;

    while (i < len) {
        /* Bytes remaining in current page */
        page_remain = W25Q64_PAGE_SIZE - (addr % W25Q64_PAGE_SIZE);
        if (page_remain > (len - i))
            page_remain = (uint16_t)(len - i);

        W25Q64_WritePage(addr, &buf[i], page_remain);

        addr += page_remain;
        i    += page_remain;
    }
}

/* --------------- Sector Erase (0x20) - 4KB --------------- */
void W25Q64_EraseSector(uint32_t addr)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_SECTOR_ERASE);
    SPI2_SendRecv((addr >> 16) & 0xFF);
    SPI2_SendRecv((addr >> 8)  & 0xFF);
    SPI2_SendRecv(addr & 0xFF);
    W25Q_CS_HIGH();

    W25Q64_WaitBusy();   /* Wait for erase to complete (~100ms) */
}

/* --------------- Chip Erase (0xC7) - entire chip --------------- */
void W25Q64_EraseChip(void)
{
    W25Q64_WaitBusy();
    W25Q64_WriteEnable();

    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_CHIP_ERASE);
    W25Q_CS_HIGH();

    W25Q64_WaitBusy();   /* Wait for chip erase (~25s) */
}

/* --------------- Power Down --------------- */
void W25Q64_PowerDown(void)
{
    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_POWER_DOWN);
    W25Q_CS_HIGH();
}

/* --------------- Wake Up --------------- */
void W25Q64_WakeUp(void)
{
    W25Q_CS_LOW();
    SPI2_SendRecv(W25Q_CMD_RELEASE_PD);
    W25Q_CS_HIGH();
}
