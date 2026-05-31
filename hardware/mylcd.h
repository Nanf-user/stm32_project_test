#ifndef __LCD_H
#define __LCD_H			  	 
#include "stm32f4xx.h"
#include "stdint.h"

#define USE_HORIZONTAL 0  //���ú�������������ʾ 0��1Ϊ���� 2��3Ϊ����

#if USE_HORIZONTAL==0||USE_HORIZONTAL==1
#define ST7789_W 240
#define ST7789_H 320

#else
#define ST7789_W 320
#define ST7789_H 240
#endif

//������ɫ
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 
#define BRRED 			 0XFC07 
#define GRAY  			 0X8430 
//-------------------ģʽѡ��---------------------
#define ST7789_HSPI SPI1     //ѡ��SPI1��2,choose Hardware SPI2 or SPI1
#define SoftWare_SPI 0       //�Ƿ�ʹ������SPI���ʹ��������Ϊ1,do ou want to use softerware SPI?if you do,set here to 1
#define Fast_Mode 0          //�Ƿ�ʹ�ÿ���ģʽ���ʹ��������Ϊ1,do ou want to use FastMode?if you do,set here to 1
#define ENABLE_CS 0          //�������Ƭѡ�ˣ��빴ѡ����,if there is a CS pin in your ST7789 ,set here to 1(Actually, you don't have to)

//-----------------OLED�˿ڶ���---------------- //ע���������DC�������Ƕ�������SPI������
#define OLED_SCL(x) GPIO_WriteBit(GPIOB,GPIO_Pin_13,(BitAction)x)    //����SCL 
#define OLED_SDA(x) GPIO_WriteBit(GPIOB,GPIO_Pin_15,(BitAction)(x))  //����SDA
#define OLED_RST(x) GPIO_WriteBit(GPIOB,GPIO_Pin_11,(BitAction)x)    //RES
#define OLED_CS(x) GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)x)      //CS

#define OLED_DC(x) GPIO_WriteBit(GPIOB,GPIO_Pin_10,(BitAction)x)     //DC
#define DC_Pin(x) 0x01<<x//�Ĵ��������������ٶ�
extern uint16_t BACK_COLOR, POINT_COLOR;//����ɫ������ɫ
//Ĭ��Deault: White Black
////////////////////////////////User////////////////////////////////////////////
void ST7789_Init(uint16_t Back_color,uint16_t Pen_color);
void ST7789_Cursor(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2);
void ST7789_Clear(uint16_t Color);
void ST7789_Clear_DMA(uint16_t Color);
void ST7789_DrawPoint(uint16_t x,uint16_t y);

void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode);

void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode);
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey);
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey);

void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[]);
void LCD_ShowImage_FromFlash(uint16_t x, uint16_t y,
                             uint16_t width, uint16_t height,
                             uint32_t flash_addr);
void LCD_ShowImage_VerScan(uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height,
                            uint32_t flash_addr);
void LCD_ShowLine(uint16_t y, uint8_t *buf);

#endif
