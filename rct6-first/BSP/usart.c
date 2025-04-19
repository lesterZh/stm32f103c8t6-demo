#include "sys.h"
#include "usart.h"
//////////////////////////////////////////////////////////////////////////////////
// ���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_OS
#include "includes.h" //ucos ʹ��
#endif
//////////////////////////////////////////////////////////////////////////////////
// ������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
// ALIENTEK STM32������
// ����1��ʼ��
// ����ԭ��@ALIENTEK
// ������̳:www.openedv.com
// �޸�����:2012/8/18
// �汾��V1.5
// ��Ȩ���У�����ؾ���
// Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
// All rights reserved
//********************************************************************************
// V1.3�޸�˵��
// ֧����Ӧ��ͬƵ���µĴ��ڲ���������.
// �����˶�printf��֧��
// �����˴��ڽ��������.
// ������printf��һ���ַ���ʧ��bug
// V1.4�޸�˵��
// 1,�޸Ĵ��ڳ�ʼ��IO��bug
// 2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
// 3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
// 4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
// V1.5�޸�˵��
// 1,�����˶�UCOSII��֧��
//////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// �������´���,֧��printf����,������Ҫѡ��use MicroLIB
#if 1
#pragma import(__use_no_semihosting)
// ��׼����Ҫ��֧�ֺ���
struct __FILE
{
	int handle;
};

FILE __stdout;
// ����_sys_exit()�Ա���ʹ�ð�����ģʽ
void _sys_exit(int x)
{
	x = x;
}
// �ض���fputc����
int fputc(int ch, FILE *f)
{
	while ((USART1->SR & 0X40) == 0)
		; // ѭ������,ֱ���������
	USART1->DR = (u8)ch;
	return ch;
}
#endif

#if EN_USART1_RX // ���ʹ���˽���
// ����1�жϷ������
// ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN]; // ���ջ���,���USART_REC_LEN���ֽ�.

// ����״̬�� //�����յ���β��ʶ��:0D 0A�� USART_RX_STA = 2��
// ����յ�����һ֡���� = 3������յ����һ���ַ���3msû����һ���ַ�����Ϊһ֡������ɣ�
u8 USART_RX_STA = 0; // ����״̬���

u8 USART_RX_LEN = 0;
#endif

void uart_init(u32 bound)
{
	// GPIO�˿�����
	GPIO_InitTypeDef GPIO_InitStructure;
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1 | RCC_APB2Periph_GPIOA, ENABLE); // ʹ��USART1��GPIOAʱ��

	// USART1_TX   GPIOA.9
    // ������Ƭ�����ڷ����쳣����λ���ղ������ݣ�����������������
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9; // PA.9
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
	GPIO_Init(GPIOA, &GPIO_InitStructure);			// ��ʼ��GPIOA.9

	// USART1_RX	  GPIOA.10��ʼ��
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;			  // PA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ��������
	GPIO_Init(GPIOA, &GPIO_InitStructure);				  // ��ʼ��GPIOA.10

	// Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0~3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���

	// USART ��ʼ������
	USART_InitStructure.USART_BaudRate = bound;										// ���ڲ�����
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;						// �ֳ�Ϊ8λ���ݸ�ʽ
	USART_InitStructure.USART_StopBits = USART_StopBits_1;							// һ��ֹͣλ
	USART_InitStructure.USART_Parity = USART_Parity_No;								// ����żУ��λ
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; // ��Ӳ������������
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;					// �շ�ģʽ

	USART_Init(USART1, &USART_InitStructure);	   // ��ʼ������1
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE); // �������ڽ����ж�
	USART_Cmd(USART1, ENABLE);					   // ʹ�ܴ���1
}

void sendChar(char ch)
{
	USART_SendData(USART1, ch);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TC) != SET)
		;
}

void sendChars(char chs[], int len)
{
	u8 i;
	for (i = 0; i < len; i++)
	{
		sendChar(chs[i]);
	}
}

void sendString(char *str)
{
	while (*str != '\0')
	{
		sendChar(*str);
		str++;
	}
}

extern int sys_tick_1ms;
extern int uart_last_rec_sys_tick;

void USART1_IRQHandler(void) // ����1�жϷ������
{
	u8 Res;
#if SYSTEM_SUPPORT_OS // ���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntEnter();
#endif

    // ͨ����ȡ DR �Ĵ�������ʽ��� RXNE ��־������ʽ���� USART_ClearITPendingBit(USART1, USART_IT_RXNE)
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) // �����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res = USART_ReceiveData(USART1); // ��ȡ���յ�������
        USART_RX_BUF[USART_RX_LEN] = Res;
        USART_RX_LEN++;
        if (USART_RX_LEN > 2) {
            if (USART_RX_BUF[USART_RX_LEN-1] == 0x0A && USART_RX_BUF[USART_RX_LEN-2] == 0x0D) {
                USART_RX_STA = 2;
            }
        }
        
        uart_last_rec_sys_tick = sys_tick_1ms;


		// USART_SendData(USART1, Res);
        
		// if (USART_RX_STA == 0)
		// {
		// 	if (Res == 0x0d)
		// 	{
		// 		USART_RX_STA = 1;
		// 	}
		// 	else
		// 	{
		// 		// buf������od oa�Ľ����ֽ�
		// 		USART_RX_BUF[USART_RX_LEN++] = Res;
		// 	}
		// }
		// else if (USART_RX_STA == 1)
		// {
		// 	if (Res == 0x0a)
		// 	{
		// 		USART_RX_STA = 2;
		// 		USART_RX_BUF[USART_RX_LEN] = 0;
		// 	}
		// 	else
		// 	{
		// 		// ���ִ���
		// 		USART_RX_LEN = 0;
		// 		USART_RX_STA = 0;
		// 	}
		// }
	}


#if SYSTEM_SUPPORT_OS // ���SYSTEM_SUPPORT_OSΪ�棬����Ҫ֧��OS.
	OSIntExit();
#endif
}
