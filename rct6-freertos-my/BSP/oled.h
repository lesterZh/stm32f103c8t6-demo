
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h" 

/*
常用的OLED屏幕有蓝色、黄色、白色等几种。屏的大小为0.96寸，像素点为128*64，所以我们称为0.96oled屏或者12864屏。
驱动芯片：SSD1306
OLED本身是没有显存的，他的现存是依赖SSD1306提供的，而SSD1306提供一块显存。
SSD1306显存总共为128*64bit大小，SSD1306将这些显存分成了8页。每页包含了128个字节。
分辨率为128*64（这么多个像素点）由128列，64行组成，其中64行又被分为8页，每页8行！
STM32内部建立一个缓存(共128*8个字节），每次修改的时候，只是修改STM32上的缓存(实际上就是SRAM)，
修改完后一次性把STM32上的缓存数据写入到OLED的GRAM。
*/

#define OLED_MODE 1
#define SIZE 16
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    



//-----------------OLED端口定义----------------  					   
#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_8)//CLK 
#define OLED_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_8)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_9)//DIN
#define OLED_SDIN_Set() GPIO_SetBits(GPIOB,GPIO_Pin_9)


#define OLED_DC_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_12)//DC
#define OLED_DC_Set() GPIO_SetBits(GPIOB,GPIO_Pin_12)
 		     
#define OLED_CS_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_13)//CS
#define OLED_CS_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_13)

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


//OLED控制用函数
void OLED_WR_Byte(u8 dat,u8 cmd);	    
void OLED_Display_On(void);
void OLED_Display_Off(void);	   							   		    
void OLED_Init(void);
void OLED_Clear(void);
void OLED_DrawPoint(u8 x,u8 y,u8 t);
void OLED_Fill(u8 x1,u8 y1,u8 x2,u8 y2,u8 dot);
void OLED_ShowChar(u8 x,u8 y,u8 chr);
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size);
void OLED_ShowString(u8 x,u8 y, u8 *p);	 
void OLED_Set_Pos(unsigned char x, unsigned char y);
void OLED_ShowCHinese(u8 x,u8 y,u8 no);
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,unsigned char BMP[]);
void OLED_ClearLine(u8 y);

#endif  
	 



