#include "led.h"
#include "delay.h"
#include "sys.h"
//ALIENTEK miniSTM32开发板实验1
//跑马灯实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司

int t3_cnt = 0;
int sys_tick_1ms = 0;
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
        if (t3_cnt % 2000 == 0) {
            printf("app t3:%d\r\n", t3_cnt);
        }
    }
}


 int main(void)
 {  
    int cnt = 0;
	SCB->VTOR = SRAM_BASE | 0X1000; /* Vector Table Relocation in Internal SRAM. */	
	
    delay_init();	    	 //延时函数初始化	  
	LED_Init();		  	//初始化与LED连接的硬件接口
    uart_init(115200);
	while(1)
	{
		LED0=0;
		delay_ms(1000);	 //延时300ms
		LED0=1;
		delay_ms(1000);	//延时300ms
        printf("app cnt:%d\r\n", cnt++);
	}
 }

