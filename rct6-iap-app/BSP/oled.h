
#ifndef __OLED_H
#define __OLED_H			  	 
#include "sys.h" 

/*
���õ�OLED��Ļ����ɫ����ɫ����ɫ�ȼ��֡����Ĵ�СΪ0.96�磬���ص�Ϊ128*64���������ǳ�Ϊ0.96oled������12864����
����оƬ��SSD1306
��֧�����½ӿڷ�ʽ��
1. I2C��Inter-Integrated Circuit��
����Ҫ�󣺽��� 2 ���ߣ�SCL ʱ���� + SDA �����ߣ���
�ص㣺
��ʡ���ţ��ʺ���Դ���޵�΢���������� Arduino��ESP8266/ESP32����
��ַͨ��Ϊ 0x3C �� 0x3D����ͨ���������ã���
�ٶȽ�������׼ģʽ 100kHz������ģʽ 400kHz����
����Ӧ�ã��͸��Ӷȡ��͹��ĳ�����

2. SPI��Serial Peripheral Interface)
4 �� SPI ģʽ��
���ţ�SCK��ʱ�ӣ���MOSI���������룩��CS��Ƭѡ����DC������/������ƣ���
֧�ָ���ͨ�ţ�ͨ���� I2C ���죩��
3 �� SPI ģʽ��
�ϲ� MOSI �� DC �ߣ�ͨ������λ��������/���ݣ������ã���
�ص㣺
�ٶȸ��죬�ʺ�ˢ����Ҫ��ߵĳ�����
��ռ�ø��� GPIO ���š�

7pin SPI�ӿ�

OLED������û���Դ�ģ������ִ�������SSD1306�ṩ�ģ���SSD1306�ṩһ���Դ档
SSD1306�Դ��ܹ�Ϊ128*64bit��С��SSD1306����Щ�Դ�ֳ���8ҳ��ÿҳ������128���ֽڡ�
�ֱ���Ϊ128*64����ô������ص㣩��128�У�64����ɣ�����64���ֱ���Ϊ8ҳ��ÿҳ8�У�
STM32�ڲ�����һ������(��128*8���ֽڣ���ÿ���޸ĵ�ʱ��ֻ���޸�STM32�ϵĻ���(ʵ���Ͼ���SRAM)��
�޸����һ���԰�STM32�ϵĻ�������д�뵽OLED��GRAM��
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



//-----------------OLED�˿ڶ���----------------  					   
#define OLED_SCLK_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_8)//CLK 
#define OLED_SCLK_Set() GPIO_SetBits(GPIOB,GPIO_Pin_8)

#define OLED_SDIN_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_9)//DIN
#define OLED_SDIN_Set() GPIO_SetBits(GPIOB,GPIO_Pin_9)


#define OLED_DC_Clr() GPIO_ResetBits(GPIOB,GPIO_Pin_12)//DC
#define OLED_DC_Set() GPIO_SetBits(GPIOB,GPIO_Pin_12)
 		     
#define OLED_CS_Clr()  GPIO_ResetBits(GPIOB,GPIO_Pin_13)//CS
#define OLED_CS_Set()  GPIO_SetBits(GPIOB,GPIO_Pin_13)

#define OLED_CMD  0	//д����
#define OLED_DATA 1	//д����


//OLED�����ú���
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
	 



