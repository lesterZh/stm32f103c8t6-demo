#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"

#define WK_UP GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define WKUP_PRES 2

void LED_Init(void)
{

	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��PB,PE�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;		 // LED0-->PB.4 �˿�����
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // IO���ٶ�Ϊ 2, 10, 50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			 // �����趨������ʼ��GPIOB.4
	//GPIO_ResetBits(GPIOB, GPIO_Pin_4);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // ���ø���ʱ��
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE); // ȫ����SW+JTAG������JTRST��PB4����
}

void flip_LED()
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_4) == Bit_SET) {
		GPIO_ResetBits(GPIOB, GPIO_Pin_4);
	} else {
		GPIO_SetBits(GPIOB, GPIO_Pin_4);
	}
}

void init_Key()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	  // PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // PA0 ���ó����룬Ĭ������
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

u8 KEY_Scan(u8 mode)
{
	static u8 key_up = 1; // �������ɿ���־
	if (mode)
		key_up = 1; // ֧������
	if (key_up && (WK_UP == 1))
	{
		delay_ms(10); // ȥ����
		key_up = 0;
		if (WK_UP == 1)
			return WKUP_PRES;
	}
	else if (WK_UP == 0)
		key_up = 1;
	return 0; // �ް�������
}

void interrupt_init()
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	init_Key();

	// GPIOA.0 �ж����Լ��жϳ�ʼ������
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //_Rising  Falling�½��ش���
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // ��ռ���ȼ� 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		 // �����ȼ� 1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void EXTI0_IRQHandler(void)
{
	delay_ms(10); // ����
	if (WK_UP == 1)
	{
		printf("key,intrrupt\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line0); // ��� EXTI0 ��·����λ
}

// ��ʱ�� 3 �жϷ������
void TIM3_IRQHandler(void) // TIM3 �ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // ��� TIM3 �����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ��� TIM3 �����жϱ�־
        flip_LED();
    }
}

int main(void)
{
	u8 t = 0;
	u16 x = 28;
	char buf[100];
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(9600);
	LED_Init();
	interrupt_init();
	OLED_Init();  // OLED��ʼ��
	OLED_Clear(); // ����
	TIM3_Int_Init(2499, 7199);//10K, 500ms

	// �����Һ
	OLED_ShowCHinese(x, 0, 5);
	OLED_ShowCHinese(x += 18, 0, 6);
	OLED_ShowCHinese(x += 18, 0, 7);
	OLED_ShowCHinese(x += 18, 0, 8);

	OLED_ShowString(6, 3, "0.96' OLED TEST"); // ��ʾ�ַ���

	while (1)
	{
		// printf("t:%d\r\n", t+=3);
		// if (KEY_Scan(0) == WKUP_PRES) printf("key\n");
		sprintf(buf, "t:%d    ", t += 1);
		OLED_ShowString(6, 6, buf);
		// sendString(buf);

		delay_ms(500);
		

		if (USART_RX_STA == 2)
		{
			printf("rec:%s\r\n", USART_RX_BUF);
			USART_RX_STA = 0;
			USART_RX_LEN = 0;
			

			OLED_ShowString(6, 3, "                  ");
			OLED_ShowString(6, 3, USART_RX_BUF);
			
			USART_RX_BUF[0] = 0;
		}
	}
}
