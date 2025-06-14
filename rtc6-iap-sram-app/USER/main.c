#include "led.h"
#include "delay.h"
#include "sys.h"
//ALIENTEK miniSTM32������ʵ��1
//�����ʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾

int t3_cnt = 0;
int sys_tick_1ms = 0;
// ��ʱ�� 3 �жϷ������
// 1ms����
void TIM3_IRQHandler(void) // TIM3 �ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // ��� TIM3 �����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ��� TIM3 �����жϱ�־
        // printf("t3_cnt:%d\r\n", t3_cnt++);
        t3_cnt++;
        sys_tick_1ms++;
        if (t3_cnt % 2000 == 0) {
            printf("app t3:%d\r\n", t3_cnt);
        }
    }
}


 int main(void)
 {  
    int cnt = 0;
	SCB->VTOR = SRAM_BASE | 0X1000; /* Vector Table Relocation in Internal SRAM. */	
	
    delay_init();	    	 //��ʱ������ʼ��	  
	LED_Init();		  	//��ʼ����LED���ӵ�Ӳ���ӿ�
    uart_init(115200);
	while(1)
	{
		LED0=0;
		delay_ms(1000);	 //��ʱ300ms
		LED0=1;
		delay_ms(1000);	//��ʱ300ms
        printf("app cnt:%d\r\n", cnt++);
	}
 }

