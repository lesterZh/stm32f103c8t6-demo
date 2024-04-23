#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"
#include "led_key.h"
#include "24C08.h"

int cnt = 0;


// ��ʱ�� 3 �жϷ������
void TIM3_IRQHandler(void) // TIM3 �ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // ��� TIM3 �����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ��� TIM3 �����жϱ�־
        printf("cnt:%d\r\n", cnt++);
    }
}

void EXTI0_IRQHandler(void)
{
	delay_ms(10); // ����
	if (WK_UP == 1)
	{
		printf("key-PA0,intrrupt\n");
	}
	EXTI_ClearITPendingBit(EXTI_Line0); // ��� EXTI0 ��·����λ
}

uint8_t Sendbuff[17]={10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};//���ֽ�д����
uint8_t Recvbuff[18];//���ֽڶ�����
int i = 0;

// �������������ⲿ8M����72Mϵͳ��Ƶ��������void SetSysClock(void)������ѡ����Ƶ������
int main(void)
{
    u16 x = 28;
	u8 data;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(); // ��ʱ������ʼ��,ͨ��Systick�ж�ʵ��1ms��ʱ����
    LED_Init();   // ��ʼ��GPIO,PB4���ó��������
    uart_init(9600);
    // TIM3_Int_Init(9999, 7199);//10K, 500ms
    gpio_A0_interrupt_init();
	IIC_Init();//IIC��ʼ���������ٶ�Ϊ100K,I2C2
	
    OLED_Init();  // OLED��ʼ��
	OLED_Clear(); // ����

    // �����Һ
	OLED_ShowCHinese(x, 0, 5);
	OLED_ShowCHinese(x += 18, 0, 6);
	OLED_ShowCHinese(x += 18, 0, 7);
	OLED_ShowCHinese(x += 18, 0, 8);
    OLED_ShowString(6, 3, "0.96' OLED TEST"); // ��ʾ�ַ���
	
	AT24C08_WriteOneByte(0,0x33);//0��ַд��0x33
	data= AT24C08_ReadOneByte(0);//��0��ַ�����ݴ浽EEDATA

    AT24C08_WriteByte(0,17,Sendbuff); 
	AT24C08_ReadByte(0,17,Recvbuff); 
    printf("read at24c02 one:%x , arr:", data);
    for (i=0; i<17; i++) printf("%d, ", Recvbuff[i]);
    printf("\r\n");

    while (1)
    {
        delay_ms(500);
        flip_LED();

		if (USART_RX_STA == 2) //���ʹ�0D 0A
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