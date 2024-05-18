#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"
#include "led_key.h"
#include "24C08.h"

int cnt = 0;

uint8_t Sendbuff[17]={10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};//���ֽ�д����
uint8_t Recvbuff[18];//���ֽڶ�����
int i = 0;
u8 oled_show[30];
u8 oled_cnt = 250;

// ��ʱ�� 3 �жϷ������
void TIM3_IRQHandler(void) // TIM3 �ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // ��� TIM3 �����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ��� TIM3 �����жϱ�־
        // printf("cnt:%d\r\n", cnt++);
        flip_LED();
    }
}

void EXTI0_IRQHandler(void)
{
	delay_ms(10); // ����
	if (WK_UP == 1)
	{
		printf("WK-PA0,int\r\n");
        oled_cnt = 0;
	}
	EXTI_ClearITPendingBit(EXTI_Line0); // ��� EXTI0 ��·����λ
}

// �������������ⲿ8M����72Mϵͳ��Ƶ��������void SetSysClock(void)������ѡ����Ƶ������
int main(void)
{
    u16 x = 28;
	u8 data;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(); // ��ʱ������ʼ��,ͨ��Systick�ж�ʵ��1ms��ʱ����
    LED_Init();   // ��ʼ��GPIO,PB4���ó��������
    uart_init(9600);
    TIM3_Int_Init(4999, 7199);//10K, 500ms

    gpio_A0_interrupt_init();
	IIC_Init();//IIC��ʼ���������ٶ�Ϊ100K,I2C2
	
    OLED_Init();  // OLED��ʼ��
	OLED_Clear(); // ����

    // ���� 16*16
	OLED_ShowCHinese(x, 0, 9);
	OLED_ShowCHinese(x += 20, 0, 10);
	OLED_ShowCHinese(x += 20, 0, 11);
	OLED_ShowCHinese(x += 20, 0, 12);
    OLED_ShowString(6, 2, "0.96' OLED TEST"); // ��ʾ�ַ���,һ���ַ�����һ�� y = y+2
	
	AT24C08_WriteOneByte(0,0x33);//0��ַд��0x33
	data= AT24C08_ReadOneByte(0);//��0��ַ�����ݴ浽EEDATA

    AT24C08_WriteByte(0,17,Sendbuff); 
	AT24C08_ReadByte(0,17,Recvbuff); 

    printf("read at24c02 one:%x , arr:", data);
    for (i=0; i<17; i++) printf("%d, ", Recvbuff[i]);
    printf("\r\n");

    while (1)
    {
        delay_ms(1000);

        oled_cnt++;
        sprintf(oled_show, "cnt:%d", oled_cnt);
        OLED_ClearLine(4);
        OLED_ShowString(6, 4, oled_show);

		if (USART_RX_STA == 2) //�����յ���β��ʶ��:0D 0A
		{
			printf("rec:%s\r\n", USART_RX_BUF);
			USART_RX_STA = 0;
			USART_RX_LEN = 0;

			OLED_ClearLine(6);
			OLED_ShowString(6, 6, USART_RX_BUF);
			USART_RX_BUF[0] = 0;
		}
    }
}
