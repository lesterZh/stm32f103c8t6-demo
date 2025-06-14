#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"
#include "led_key.h"
#include "24C08.h"

int t3_cnt = 0;
int sys_tick_1ms = 0;
int uart_last_rec_sys_tick = 0;

uint8_t Sendbuff[17]={10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26};//多字节写缓冲
uint8_t Recvbuff[18];//多字节读缓冲
int i = 0;
u8 oled_show[30];
u8 oled_cnt = 250;

// 定时器 3 中断服务程序
// 1ms触发
void TIM3_IRQHandler(void) // TIM3 中断
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // 检查 TIM3 更新中断发生与否
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // 清除 TIM3 更新中断标志
        // printf("t3_cnt:%d\r\n", t3_cnt++);
        t3_cnt++;
        sys_tick_1ms++;
        if (t3_cnt % 300 == 0) flip_LED();
    }
}

void EXTI0_IRQHandler(void)
{
	delay_ms(10); // 消抖，正常不应该在中断里面延时
	if (WK_UP == 1)
	{
		printf("WK-PA0,int\r\n");
        OLED_ClearLine(6);
        OLED_ShowString(6, 6, "WK Key Press");
	}
	EXTI_ClearITPendingBit(EXTI_Line0); // 清除 EXTI0 线路挂起位
}


// 主函数，采用外部8M晶振，72M系统主频，可以在void SetSysClock(void)函数中选择主频率设置
int main(void)
{
    // 默认调用static void SetSysClockTo72(void)，使用HSE外部晶振，设置时钟72M, APB1=36M,APB2=72M
    u16 x = 28;
	u8 data;

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init(); // 延时函数初始化,通过Systick中断实现1ms延时功能
    LED_Init();   // 初始化GPIO,PB4配置成推挽输出
    uart_init(115200);
    TIM3_Int_Init(9, 7199);//10K, 1ms

    gpio_A0_interrupt_init();
    led_key_init_mini_bsp();

    static int now = 0;
    int tick_1000ms = 0;
    int tick_500ms = 0;
    int tick_100ms = 0;

    while (1)
    {
        // delay_ms(1000);
        now = sys_tick_1ms;

        if (now - tick_1000ms >= 1000) {
            tick_1000ms = now;
            led_flip_mini_bsp();
            oled_cnt++;
            printf("cnt:%d\r\n", oled_cnt);
        }
 

		if (USART_RX_STA == 2) //串口收到结尾标识符:0D 0A
		{
			printf("rec:%s\r\n", USART_RX_BUF);
			USART_RX_STA = 0;
			USART_RX_LEN = 0;

			OLED_ClearLine(6);
			OLED_ShowString(6, 6, USART_RX_BUF);
			USART_RX_BUF[0] = 0;
		}

        // 主循环中检测“帧间隔”超时（ 空闲3ms视为一帧结束）
        int time_dur = now - uart_last_rec_sys_tick;
        if (USART_RX_LEN > 0 && time_dur >= 3) {
            printf("rec frame:%s\r\n", USART_RX_BUF);
			USART_RX_STA = 0;
			USART_RX_LEN = 0;
        }

    }
}
