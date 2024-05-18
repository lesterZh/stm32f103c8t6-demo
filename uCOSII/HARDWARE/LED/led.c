#include "led.h"

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK mini�SSTM32������
//LED��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/2
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	   

//��ʼ��PB5��PE5Ϊ�����.��ʹ���������ڵ�ʱ��		    
//LED IO��ʼ��
void LED_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��PB,PE�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;        // LED0-->PB.4 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // IO���ٶ�Ϊ 2, 10, 50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);           // �����趨������ʼ��GPIOB.4
    // GPIO_ResetBits(GPIOB, GPIO_Pin_4);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // ���ø���ʱ��
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE); // ȫ����SW+JTAG������JTRST��PB4����
}
