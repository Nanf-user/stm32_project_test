#include "mylcd.h"
#include "w25q64.h"
#include "image.h"
#include "delay.h"

int main(void)
{
    uint32_t flash_id;
    uint8_t has_img1, has_img2;

    /* ---------- Init ---------- */
    delay_init(168);
    ST7789_Init(WHITE, RED);
    POINT_COLOR = RED;
    ST7789_Clear(WHITE);
    W25Q64_Init();

    /* ---------- Check W25Q64 ---------- */
    flash_id = W25Q64_ReadID();
    if (flash_id == 0 || flash_id == 0xFFFFFF) {
        LCD_ShowString(10, 10, (u8 *)"W25Q64 NOT FOUND!", RED, WHITE, 16, 0);
        while (1) { delay_ms(1000); }
    }

    /* ---------- Init image slots ---------- */
    has_img1 = Image_HasData(IMG1_FLASH_ADDR);
    has_img2 = Image_InitSlot2();  /* write pic.h if empty */

    /* ---------- Main loop: carousel ---------- */
    while (1)
    {
        if (has_img1) {
            Image_ShowSlot1();
            delay_ms(3000);
        }

        if (has_img2) {
            Image_ShowSlot2();
            delay_ms(3000);
        }

        if (!has_img1 && !has_img2) {
            ST7789_Clear(WHITE);
            LCD_ShowString(10, 10, (u8 *)"No images!", RED, WHITE, 16, 0);
            delay_ms(1000);
        }
    }
}
