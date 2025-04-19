#include "led_key.h"
#include "delay.h"
#include "usart.h"
#include "sys.h"
#include "includes.h"
#include "oled.h"
#include "timer.h"

/////////////////////////UCOSII任务设置///////////////////////////////////
// START 任务
// 设置任务优先级
#define START_TASK_PRIO 10 // 开始任务的优先级设置为最低
// 设置任务堆栈大小
#define START_STK_SIZE 64
// 任务堆栈
OS_STK START_TASK_STK[START_STK_SIZE];
// 任务函数
void start_task(void *pdata);

// LED0任务
// 设置任务优先级
#define LED_TASK_PRIO 7
// 设置任务堆栈大小
#define LED_STK_SIZE 64
// 任务堆栈
OS_STK LED_TASK_STK[LED_STK_SIZE];
// 任务函数
void led_task(void *pdata);

#define STK_SIZE 64
#define STK_SIZE_BIG 128

#define OS_TASK_STAT_EN           1


void uart_task(void *pdata);
OS_STK uart_task_stk[STK_SIZE_BIG];

void oled_task(void *pdata);
OS_STK oled_task_stk[STK_SIZE_BIG];

void key_task(void *pdata);
OS_STK key_task_stk[STK_SIZE_BIG];

void seg_task(void *pdata);
OS_STK seg_task_stk[STK_SIZE_BIG];

void mq_task(void *pdata);
OS_STK mq_task_stk[STK_SIZE_BIG];



OS_EVENT* key_seg;
OS_EVENT* mail_signal;

OS_EVENT* mq_signal;
void *msgGrp[16];

// 定义指向定时器控制块的指针
OS_TMR   * os_soft_timer;	//软件定时器1
OS_TMR   * delay_key_checker;	//按键消抖延时回调
void KeyDelayCheckCallback(OS_TMR *ptmr, void *p_arg) {
    // printf("KeyDelayCheckCallback\r\n");
    if (WK_UP_KEY == 1) {
        printf("WK Key Down\r\n");
    }
}

void TimerCallback(OS_TMR *ptmr, void *p_arg) {
    printf("os soft timer run\r\n");
}

int main(void)
{   
    u16 x = 28;
	u8 data;

    uart_init(9600);
    delay_init();                                   // 延时函数初始化
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    LED_Init();                                     // 初始化与LED连接的硬件接口

    OLED_Init();  // OLED初始化
	OLED_Clear(); // 清屏

    // 中文 16*16
	OLED_ShowCHinese(x, 0, 9);
	OLED_ShowCHinese(x += 20, 0, 10);
	OLED_ShowCHinese(x += 20, 0, 11);
	OLED_ShowCHinese(x += 20, 0, 12);
    OLED_ShowString(6, 2, "0.96' OLED TEST"); // 显示字符串,一行字符后下一行 y = y+2
	
    gpio_A0_interrupt_init();

    OSInit();
    OSTaskCreate(start_task, (void *)0, (OS_STK *)&START_TASK_STK[START_STK_SIZE - 1], START_TASK_PRIO); // 创建起始任务
    OSStart();
}



int exit0_cnt = 0;
void EXTI0_IRQHandler(void)
{
    u8 err;
    OSIntEnter();
	exit0_cnt++;

	EXTI_ClearITPendingBit(EXTI_Line0); // 清除 EXTI0 线路挂起位
    OSIntExit();

    //不能放在OSIntEnter()和OSIntExit()之间
    OSTmrStart(delay_key_checker, &err);
}   

// 开始任务
void start_task(void *pdata)
{
    OS_CPU_SR cpu_sr = 0;
    u8 err = 0;

    pdata = pdata;
    OS_ENTER_CRITICAL(); // 进入临界区(无法被中断打断)

    //必须要在task内部创建，或者osInit之后创建
    key_seg = OSSemCreate(0);
    mail_signal = OSMboxCreate((void*)0);
    mq_signal = OSQCreate(&msgGrp[0], 16);

    //以1/OS_TMR_CFG_TICKS_PER_SEC为基本时间单位的; 定时器模块的最高精度等于OSTick的精度
    os_soft_timer = OSTmrCreate(1, 1000, OS_TMR_OPT_PERIODIC, TimerCallback, NULL, NULL, &err);
    OSTmrStart(os_soft_timer, &err);

    delay_key_checker = OSTmrCreate(1, 10, OS_TMR_OPT_ONE_SHOT, KeyDelayCheckCallback, NULL, NULL, &err);
    // OSTmrStart(delay_key_checker, &err);

    OSTaskCreate(led_task, (void *)0, (OS_STK *)&LED_TASK_STK[LED_STK_SIZE - 1], 2);
    OSTaskCreate(uart_task, (void *)0, (OS_STK *)&uart_task_stk[STK_SIZE_BIG-1], 1);
    OSTaskCreate(oled_task, (void *)0, (OS_STK *)&oled_task_stk[STK_SIZE_BIG-1], 3);
    OSTaskCreate(key_task, (void *)0, (OS_STK *)&key_task_stk[STK_SIZE_BIG-1], 60);
    OSTaskCreate(seg_task, (void *)0, (OS_STK *)&seg_task_stk[STK_SIZE_BIG-1], 9);
    OSTaskCreate(mq_task, (void *)0, (OS_STK *)&mq_task_stk[STK_SIZE_BIG-1], 11);

    OSTaskSuspend(START_TASK_PRIO); // 挂起起始任务.

    OS_EXIT_CRITICAL();             // 退出临界区(可以被中断打断)
}

// LED0任务
void led_task(void *pdata)
{
    uint8_t err;
    while (1)
    {
        LED = 0;
        delay_ms(500);

        // OSSemPend(key_seg, 0, &err);
        // printf("seg ret:%d\r\n", err);

        LED = 1;
        delay_ms(1500);
    };
}

void uart_task(void *pdata) {
    while (1) {
        delay_ms(10);
        if (USART_RX_STA == 2) //串口收到结尾标识符:0D 0A
		{
			printf("rec:%s\r\n", USART_RX_BUF);

			OLED_ClearLine(6);
			OLED_ShowString(6, 6, USART_RX_BUF);

            USART_RX_STA = 0;
			USART_RX_LEN = 0;
			USART_RX_BUF[0] = 0;
		}
    }
}

void oled_task(void *pdata) {
    u32 cnt = 0, k = 0;
    
    static int oled_cnt = 0;
    u8 oled_show[20];

    while (1) {
        oled_cnt++;
        sprintf(oled_show, "cnt:%d", oled_cnt);
        OLED_ClearLine(4);
        OLED_ShowString(6, 4, oled_show);

        sprintf(oled_show, "int0_cnt:%d", exit0_cnt);
        OLED_ClearLine(2);
        OLED_ShowString(6, 2, oled_show);

        delay_ms(1000);
    }
}

void key_task(void *pdata) {
    u8 err;
    u8 key1_down_cnt = 0, key1_up_cnt = 0;
    u8 oled_show[20];
    u8 mail_cnt = 0;
    u8 mq_cnt = 0, mq_id = 0;
    while (1) {
        delay_ms(10);
        if (KEY1 == 0) {
            key1_down_cnt++;
            if (key1_down_cnt >= 5) key1_down_cnt = 5;
            key1_up_cnt = 0;
        } else {
            key1_up_cnt++;
            if (key1_up_cnt >= 5) key1_up_cnt = 5;
            key1_down_cnt = 0;
        }

        if (key1_down_cnt == 2) {
            printf("Key1 Down\r\n");

            OLED_ClearLine(6);
            OLED_ShowString(6, 6, "key1 down");

            //发送信号量
            OSSemPost(key_seg); //计数值 +1

            mail_cnt++;
            //消息邮箱一次传递一则消息
            sprintf(oled_show, "mail box:%d", mail_cnt);
            OSMboxPost(mail_signal, oled_show); //发送字符串

            //发送消息队列
            for (mq_id = 0; mq_id < 3; mq_id++) {
                // sprintf(oled_show, "mq box:%d", mq_cnt++); 因为传递的是地址，所以只会保留最后一次修改，
                OSQPost(mq_signal, "mq msg");
            }

        } else if (key1_up_cnt == 2) {
            OLED_ClearLine(6);
            OLED_ShowString(6, 6, "key1 up");
            printf("Key1 up\r\n");
        }

        // sprintf(oled_show, "seg:%d", key_seg->OSEventCnt);
        // OLED_ClearLine(6);
        // OLED_ShowString(6, 6, oled_show);
    }
}

void seg_task(void *pdata) {
    u8 err;
    char * ret;
    while (1) {
        delay_ms(10);
        ret = OSMboxPend(mail_signal, 0, &err);
        printf("rec mail:%s\r\n", ret);
    }
}

void mq_task(void *pdata) {
    u8 err;
    char * ret;
    printf("start mq_task\r\n");

    while (1) {
        delay_ms(10);
        ret = OSQPend(mq_signal, 0, &err);
        printf("rec mq:%s\r\n", ret);
    }
}
