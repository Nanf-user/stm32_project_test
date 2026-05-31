#include "mylcd.h"
#include "w25q64.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "stdarg.h"
#include "oledfont.h"
#include "delay.h"

uint16_t BACK_COLOR = WHITE;
uint16_t POINT_COLOR = BLACK;   

void ReadWrite(uint8_t ByteSend){
	#if SoftWare_SPI//SPI,SSPI
		uint8_t i;
		for (i = 0; i < 8; i ++){
			OLED_SDA(ByteSend & (0x80 >> i));//write Byte one by one
			OLED_SCL(0);
			OLED_SCL(1);
		}
	#else			//SPI,HSPI
		#if	Fast_Mode
			SPI_I2S_SendData(ST7789_HSPI, ByteSend);
		#else
			uint8_t retrun_time=0;
			while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_TXE) != SET&&retrun_time<=2000)retrun_time++;
			SPI_I2S_SendData(ST7789_HSPI, ByteSend);
		#endif
	#endif
}

static void ST7789_WR_DATA8(uint16_t da){ 
	#if ENABLE_CS
		OLED_CS(0);
	#endif
	#if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);//ΪָPB10(1)
	#else
		OLED_DC(1);
	#endif
		ReadWrite(da);
	#if !SoftWare_SPI
		while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_BSY) == SET);
	#endif
	#if ENABLE_CS
		OLED_CS(1)
	#endif
}
static void ST7789_WR_DATA(int da){
	#if ENABLE_CS
		OLED_CS(0);
	#endif
    #if Fast_Mode
		GPIOB->BSRR = DC_Pin(10);//ָPB10(1)
	#else
		OLED_DC(1);
	#endif
	ReadWrite(da>>8);
	ReadWrite(da);
	#if !SoftWare_SPI
		while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_BSY) == SET);
	#endif
	#if ENABLE_CS
		OLED_CS(1);
	#endif
}
static void ST7789_WR_REG(uint16_t da)	 {
	#if ENABLE_CS
		OLED_CS(0);
	#endif
    #if Fast_Mode
		GPIOB->BRR = DC_Pin(10);//PB10(0)
	#else
		OLED_DC(0);
	#endif
	ReadWrite(da);
	#if !SoftWare_SPI
		while (SPI_I2S_GetFlagStatus(ST7789_HSPI, SPI_I2S_FLAG_BSY) == SET);
	#endif
	#if ENABLE_CS
		OLED_CS(1);
	#endif
}
//
void ST7789_HSPI_Init(SPI_TypeDef* SPIx)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    if (SPIx == SPI1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOA, &GPIO_InitStructure);

        GPIO_PinAFConfig(GPIOA, GPIO_PinSource5, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_SPI1);
        GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_SPI1);
    }
    else if (SPIx == SPI2)
    {
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
        GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
        GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOB, &GPIO_InitStructure);

        GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
        GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
    }

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;   
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;

    SPI_Init(SPIx, &SPI_InitStructure);
    SPI_Cmd(SPIx, ENABLE);

    ReadWrite(0x00);   //  dummy send
}

void ST7789_Cursor(unsigned int x1,unsigned int y1,unsigned int x2,unsigned int y2){
   ST7789_WR_REG(0x2a);
   ST7789_WR_DATA(x1);
   ST7789_WR_DATA(x2);
  
   ST7789_WR_REG(0x2b);
   ST7789_WR_DATA(y1);
   ST7789_WR_DATA(y2);

   ST7789_WR_REG(0x2C);					 						 
}

void ST7789_Init(uint16_t Back_color,uint16_t Pen_color){
	  GPIO_InitTypeDef  GPIO_InitStructure;	
		int i;
	  BACK_COLOR = Back_color;
	  POINT_COLOR = Pen_color;
	//---------------------------------- DC,RST------------------------------------
 				
	  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;   //PB10=DC, PB11=RST
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_10 | GPIO_Pin_11);

	#if SoftWare_SPI									
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;	 
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_15);
	//------------------------------------SCL|SDA-----------------------------------------
	#else
		ST7789_HSPI_Init(ST7789_HSPI);//Ӳ��SPI��ʼ��
	#endif

//	if(ST7789_HSPI == SPI1)
//		MYDMA_Init(DMA1_Channel3,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,480);
//	else
//		MYDMA_Init(DMA1_Channel5,(uint32_t)&(ST7789_HSPI->DR),(uint32_t)SendBuff,480);

//	SPI_I2S_DMACmd(ST7789_HSPI,SPI_I2S_DMAReq_Tx,ENABLE);//allow SPI connect to DMA
	#if ENABLE_CS
		OLED_CS(0);
	#endif
	OLED_RST(0);
	for(i =0;i<2000;i++)__NOP();//��ʱ
//	delay_ms(50);
	OLED_RST(1);
	for(i =0;i<2000;i++)__NOP();//��ʱ
//	delay_ms(50);
	
	
	ST7789_WR_REG(0x36);
	if(USE_HORIZONTAL==0)ST7789_WR_DATA8(0x00);
	else if(USE_HORIZONTAL==1)ST7789_WR_DATA8(0xC0);
	else if(USE_HORIZONTAL==2)ST7789_WR_DATA8(0x70);
	else ST7789_WR_DATA8(0xA0);

	ST7789_WR_REG(0x3A); 
	ST7789_WR_DATA8(0x05);

	ST7789_WR_REG(0xB2);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x00);
	ST7789_WR_DATA8(0x33);
	ST7789_WR_DATA8(0x33);

	ST7789_WR_REG(0xB7); 
	ST7789_WR_DATA8(0x35);  

	ST7789_WR_REG(0xBB);
	ST7789_WR_DATA8(0x19);

	ST7789_WR_REG(0xC0);
	ST7789_WR_DATA8(0x2C);

	ST7789_WR_REG(0xC2);
	ST7789_WR_DATA8(0x01);

	ST7789_WR_REG(0xC3);
	ST7789_WR_DATA8(0x12);   

	ST7789_WR_REG(0xC4);
	ST7789_WR_DATA8(0x20);  

	ST7789_WR_REG(0xC6);
	ST7789_WR_DATA8(0x0F);    

	ST7789_WR_REG(0xD0); 
	ST7789_WR_DATA8(0xA4);
	ST7789_WR_DATA8(0xA1);

	ST7789_WR_REG(0xE0);
	ST7789_WR_DATA8(0xD0);
	ST7789_WR_DATA8(0x04);
	ST7789_WR_DATA8(0x0D);
	ST7789_WR_DATA8(0x11);
	ST7789_WR_DATA8(0x13);
	ST7789_WR_DATA8(0x2B);
	ST7789_WR_DATA8(0x3F);
	ST7789_WR_DATA8(0x54);
	ST7789_WR_DATA8(0x4C);
	ST7789_WR_DATA8(0x18);
	ST7789_WR_DATA8(0x0D);
	ST7789_WR_DATA8(0x0B);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x23);

	ST7789_WR_REG(0xE1);
	ST7789_WR_DATA8(0xD0);
	ST7789_WR_DATA8(0x04);
	ST7789_WR_DATA8(0x0C);
	ST7789_WR_DATA8(0x11);
	ST7789_WR_DATA8(0x13);
	ST7789_WR_DATA8(0x2C);
	ST7789_WR_DATA8(0x3F);
	ST7789_WR_DATA8(0x44);
	ST7789_WR_DATA8(0x51);
	ST7789_WR_DATA8(0x2F);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x1F);
	ST7789_WR_DATA8(0x20);
	ST7789_WR_DATA8(0x23);

	ST7789_WR_REG(0x21); 

	ST7789_WR_REG(0x11); 
	for(i =0;i<5000;i++)__NOP();

	ST7789_WR_REG(0x29); 
}

void ST7789_Clear(uint16_t Color){
	uint16_t i,j;  	
	ST7789_Cursor(0,0,ST7789_W-1,ST7789_H-1);
    for(i=0;i<ST7789_W;i++){
	  for (j=0;j<ST7789_H;j++){
        	ST7789_WR_DATA(Color);	 			 
	    }
	}
}
/******************************************************************************
      函数说明：画单个点
      入口数据：x,y显示坐标

      返回值：  无
******************************************************************************/
void ST7789_DrawPoint(uint16_t x,uint16_t y){
	ST7789_Cursor(x,y,x,y);//设置光标位置 
	ST7789_WR_DATA(POINT_COLOR);
}
/******************************************************************************
      函数说明：显示汉字串
      入口数据：x,y显示坐标
                *s 要显示的汉字串
                fc 字的颜色
                bc 字的背景色
                sizey 字号 可选 16 24 32
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	while(*s!=0)
	{
		if(sizey==12) LCD_ShowChinese12x12(x,y,s,fc,bc,sizey,mode);
		else if(sizey==16) LCD_ShowChinese16x16(x,y,s,fc,bc,sizey,mode);
		else if(sizey==24) LCD_ShowChinese24x24(x,y,s,fc,bc,sizey,mode);
		else if(sizey==32) LCD_ShowChinese32x32(x,y,s,fc,bc,sizey,mode);
		else return;
		s+=2;
		x+=sizey;
	}
}
/******************************************************************************
      函数说明：显示单个12x12汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese12x12(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	                         
	HZnum=sizeof(tfont12)/sizeof(typFNT_GB12);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if((tfont12[k].Index[0]==*(s))&&(tfont12[k].Index[1]==*(s+1)))
		{ 	
			ST7789_Cursor(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))ST7789_WR_DATA(fc);
						else ST7789_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont12[k].Msk[i]&(0x01<<j))	ST7789_DrawPoint(x,y);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

/******************************************************************************
      函数说明：显示单个16x16汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese16x16(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
  TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont16)/sizeof(typFNT_GB16);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont16[k].Index[0]==*(s))&&(tfont16[k].Index[1]==*(s+1)))
		{ 	
			ST7789_Cursor(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))ST7789_WR_DATA(fc);
						else ST7789_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont16[k].Msk[i]&(0x01<<j))	ST7789_DrawPoint(x,y);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 


/******************************************************************************
      函数说明：显示单个24x24汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese24x24(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont24)/sizeof(typFNT_GB24);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont24[k].Index[0]==*(s))&&(tfont24[k].Index[1]==*(s+1)))
		{ 	
			ST7789_Cursor(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))ST7789_WR_DATA(fc);
						else ST7789_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont24[k].Msk[i]&(0x01<<j))	ST7789_DrawPoint(x,y);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
} 

/******************************************************************************
      函数说明：显示单个32x32汉字
      入口数据：x,y显示坐标
                *s 要显示的汉字
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChinese32x32(u16 x,u16 y,u8 *s,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 i,j,m=0;
	u16 k;
	u16 HZnum;//汉字数目
	u16 TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	TypefaceNum=(sizey/8+((sizey%8)?1:0))*sizey;
	HZnum=sizeof(tfont32)/sizeof(typFNT_GB32);	//统计汉字数目
	for(k=0;k<HZnum;k++) 
	{
		if ((tfont32[k].Index[0]==*(s))&&(tfont32[k].Index[1]==*(s+1)))
		{ 	
			ST7789_Cursor(x,y,x+sizey-1,y+sizey-1);
			for(i=0;i<TypefaceNum;i++)
			{
				for(j=0;j<8;j++)
				{	
					if(!mode)//非叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))ST7789_WR_DATA(fc);
						else ST7789_WR_DATA(bc);
						m++;
						if(m%sizey==0)
						{
							m=0;
							break;
						}
					}
					else//叠加方式
					{
						if(tfont32[k].Msk[i]&(0x01<<j))	ST7789_DrawPoint(x,y);//画一个点
						x++;
						if((x-x0)==sizey)
						{
							x=x0;
							y++;
							break;
						}
					}
				}
			}
		}				  	
		continue;  //查找到对应点阵字库立即退出，防止多个汉字重复取模带来影响
	}
}

/******************************************************************************
      函数说明：显示单个字符
      入口数据：x,y显示坐标
                num 要显示的字符
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowChar(u16 x,u16 y,u8 num,u16 fc,u16 bc,u8 sizey,u8 mode)
{
	u8 temp,sizex,t,m=0;
	u16 i,TypefaceNum;//一个字符所占字节大小
	u16 x0=x;
	sizex=sizey/2;
	TypefaceNum=(sizex/8+((sizex%8)?1:0))*sizey;
	num=num-' ';    //得到偏移后的值
	ST7789_Cursor(x,y,x+sizex-1,y+sizey-1);  //设置光标位置 
	for(i=0;i<TypefaceNum;i++)
	{ 
		if(sizey==12)temp=ascii_1206[num][i];		       //调用6x12字体
		else if(sizey==16)temp=ascii_1608[num][i];		 //调用8x16字体
		else if(sizey==24)temp=ascii_2412[num][i];		 //调用12x24字体
		else if(sizey==32)temp=ascii_3216[num][i];		 //调用16x32字体
		else return;
		for(t=0;t<8;t++)
		{
			if(!mode)//非叠加模式
			{
				if(temp&(0x01<<t))ST7789_WR_DATA(fc);
				else ST7789_WR_DATA(bc);
				m++;
				if(m%sizex==0)
				{
					m=0;
					break;
				}
			}
			else//叠加模式
			{
				if(temp&(0x01<<t))ST7789_DrawPoint(x,y);//画一个点
				x++;
				if((x-x0)==sizex)
				{
					x=x0;
					y++;
					break;
				}
			}
		}
	}   	 	  
}
/******************************************************************************
      函数说明：显示字符串
      入口数据：x,y显示坐标
                *p 要显示的字符串
                fc 字的颜色
                bc 字的背景色
                sizey 字号
                mode:  0非叠加模式  1叠加模式
      返回值：  无
******************************************************************************/
void LCD_ShowString(u16 x,u16 y,const u8 *p,u16 fc,u16 bc,u8 sizey,u8 mode)
{         
	while(*p!='\0')
	{       
		LCD_ShowChar(x,y,*p,fc,bc,sizey,mode);
		x+=sizey/2;
		p++;
	}  
}
/******************************************************************************
      函数说明：显示数字
      入口数据：m底数，n指数
      返回值：  无
******************************************************************************/
u32 mypow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;
	return result;
}


/******************************************************************************
      函数说明：显示整数变量
      入口数据：x,y显示坐标
                num 要显示整数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowIntNum(u16 x,u16 y,u16 num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp;
	u8 enshow=0;
	u8 sizex=sizey/2;
	for(t=0;t<len;t++)
	{
		temp=(num/mypow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				LCD_ShowChar(x+t*sizex,y,' ',fc,bc,sizey,0);
				continue;
			}else enshow=1; 
		 	 
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
} 
/******************************************************************************
      函数说明：显示两位小数变量
      入口数据：x,y显示坐标
                num 要显示小数变量
                len 要显示的位数
                fc 字的颜色
                bc 字的背景色
                sizey 字号
      返回值：  无
******************************************************************************/
void LCD_ShowFloatNum1(u16 x,u16 y,float num,u8 len,u16 fc,u16 bc,u8 sizey)
{         	
	u8 t,temp,sizex;
	u16 num1;
	sizex=sizey/2;
	num1=num*100;
	for(t=0;t<len;t++)
	{
		temp=(num1/mypow(10,len-t-1))%10;
		if(t==(len-2))
		{
			LCD_ShowChar(x+(len-2)*sizex,y,'.',fc,bc,sizey,0);
			t++;
			len+=1;
		}
	 	LCD_ShowChar(x+t*sizex,y,temp+48,fc,bc,sizey,0);
	}
}
/******************************************************************************
      函数说明：显示图片
      入口数据：x,y起点坐标
                length 图片长度
                width  图片宽度
                pic[]  图片数组    
      返回值：  无
******************************************************************************/
void LCD_ShowPicture(u16 x,u16 y,u16 length,u16 width,const u8 pic[])
{
	u16 i,j;
	u32 k=0;
	ST7789_Cursor(x,y,x+length-1,y+width-1);
	for(i=0;i<length;i++)
	{
		for(j=0;j<width;j++)
		{
			ST7789_WR_DATA8(pic[k*2+1]);
			ST7789_WR_DATA8(pic[k*2]);
			k++;
		}
	}			
}

void LCD_ShowImage_FromFlash(uint16_t x, uint16_t y,
                             uint16_t width, uint16_t height,
                             uint32_t flash_addr)
{
    uint16_t i, j;
    uint8_t buf[240 * 2];   // 最大一行（按240开）

    for (j = 0; j < height; j++)
    {
        // 读取一行数据
        W25Q64_ReadData(flash_addr + j * width * 2, buf, width * 2);

        // 设置窗口（只一行）
        ST7789_Cursor(x, y + j, x + width - 1, y + j);

        for (i = 0; i < width; i++)
        {
            // 与 LCD_ShowPicture 保持一致的字节序（低字节在前）
            ST7789_WR_DATA8(buf[i*2+1]);
            ST7789_WR_DATA8(buf[i*2]);
        }
    }
}

/* 纵取模（列优先）图片显示，用于 Img2LCD 生成的垂直扫描数据 */
void LCD_ShowImage_VerScan(uint16_t x, uint16_t y,
                            uint16_t width, uint16_t height,
                            uint32_t flash_addr)
{
    uint16_t i, j;
    uint8_t buf[320 * 2];   // 最大一列（按320开）

    for (i = 0; i < width; i++)
    {
        // 读取一列数据：从右到左（模式0x1B bit4=1）
        W25Q64_ReadData(flash_addr + (uint32_t)(width - 1 - i) * height * 2, buf, height * 2);

        // 设置窗口（只一列）
        ST7789_Cursor(x + i, y, x + i, y + height - 1);

        for (j = 0; j < height; j++)
        {
            ST7789_WR_DATA8(buf[j*2+1]);
            ST7789_WR_DATA8(buf[j*2]);
        }
    }
}






