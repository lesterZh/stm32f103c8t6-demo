#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "lcd.h"
#include "stmflash.h"
#include "iap.h"
#include "stm32f10x.h"

// ALIENTEK MiniSTM32开发板实验32
// 串口IAP实验
// FLASH分区: 0X0800 0000~0x0800 8000,给IAP使用,共32K字节,FLASH APP还有256-32=224KB可用.
//  SRAM分区: 0X2000 1000处开始, 用于存放SRAM IAP代码,共44K字节可用,用户可以自行对44K空间进行ROM和RAM区的分配
//            特别注意，SRAM APP的ROM占用量不能大于41K字节，因为本例程最大是一次接收41K字节，不能超过这个限制。
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStr;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ①时钟 TIM3 使能

    // 定时器 TIM3 初始化
    TIM_TimeBaseInitStr.TIM_Period = arr;                     // 设置自动重装载寄存器周期的值
    TIM_TimeBaseInitStr.TIM_Prescaler = psc;                  // 设置时钟频率除数的预分频值
    TIM_TimeBaseInitStr.TIM_ClockDivision = TIM_CKD_DIV1;     // 设置时钟分割
    TIM_TimeBaseInitStr.TIM_CounterMode = TIM_CounterMode_Up; // TIM 向上计数
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStr);             // ②初始化 TIM3
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                  // ③允许更新中断
    // 中断优先级 NVIC 设置
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           // TIM3 中断
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // 先占优先级 0 级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // 从优先级 3 级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ 通道被使能
    NVIC_Init(&NVIC_InitStructure);                           // ④初始化 NVIC 寄存器
    TIM_Cmd(TIM3, ENABLE);                                    // ⑤使能 TIM3
}

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
            printf("t3:%d\r\n", t3_cnt);
        }
    }
}

int main(void)
{
    // 这里根据flag切换flash和sram程序的引导
    // flash在bootloader启动后3秒内检测串口是否有更新，如果没有则直接跳转到app地址运行，如果有就接收并写入flash
    // 如果是sram，则会一直等串口接收，收到就切换到sram运行。
    u8 runFlashIAPApp = 1;

    u8 runSRAMIAPApp = 0;
    u8 uartRecAppEnd = 0;


    u32 t = 0;
    u8 key;
    u16 oldcount = 0; // 老的串口接收数据值
    u16 applenth = 0; // 接收到的app代码长度
    u8 clearflag = 0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
    delay_init();                                   // 延时函数初始化
    uart_init(115200);                              // 串口初始化为
    LED_Init();                                     // 初始化与LED连接的硬件接口
    LCD_Init();                                     // 初始化LCD
    KEY_Init();                                     // 按键初始化

    // TIM3_Int_Init(9, 7199);//10K, 1ms

    printf("bootloader start\r\n");

    while (1)
    {
        if (USART_RX_CNT)
        {
            if (oldcount == USART_RX_CNT) // 新周期内,没有收到任何数据,认为本次数据接收完成.
            {
                applenth = USART_RX_CNT;
                oldcount = 0;
                USART_RX_CNT = 0;
                printf("用户程序接收完成!\r\n");
                printf("代码长度: %d Bytes\r\n", applenth);
                uartRecAppEnd = 1;
            }
            else
            {
                oldcount = USART_RX_CNT;
            }
        }

        t++;
        delay_ms(100);

        if (t % 10 == 0)
        {
            LED0 = !LED0;
            printf("run check %d\r\n", t / 10);

            if (runFlashIAPApp && uartRecAppEnd && t > 30) {
                printf("start update app to flash\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // 判断是否为0X08XXXXXX.
                {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, applenth); // 更新FLASH代码
                    printf("固件更新完成!\r\n");
                }
                else
                {
                    printf("非FLASH应用程序!\r\n");
                }
                uartRecAppEnd = 0;

            }

            if (runFlashIAPApp && t > 30) {
                printf("go to app\r\n");
                iap_load_app(FLASH_APP1_ADDR); 
            }

            if (runSRAMIAPApp && uartRecAppEnd && t > 30) {
                printf("start go to sram app\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x20000000) // 判断是否为0X20XXXXXX.
                {
                    printf("go to sram app\r\n");
                    iap_load_app(0X20001000); // SRAM地址
                }
                else
                {
                    printf("非SRAM应用程序,无法执行!\r\n");
                }
            }
        }


        
        key = KEY_Scan(0);
        if (key == WKUP_PRES) // WK_UP按键按下
        {
            if (applenth)
            {
                printf("开始更新固件...\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // 判断是否为0X08XXXXXX.
                {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, applenth); // 更新FLASH代码
                    printf("固件更新完成!\r\n");
                }
                else
                {
                    printf("非FLASH应用程序!\r\n");
                }
            }
            else
            {
                printf("没有可以更新的固件!\r\n");
            }
            clearflag = 7; // 标志更新了显示,并且设置7*300ms后清除显示
        }

        if (key == KEY1_PRES)
        {
            printf("开始执行FLASH用户代码!!\r\n");
            if (((*(vu32 *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) // 判断是否为0X08XXXXXX.
            {
                iap_load_app(FLASH_APP1_ADDR); // 执行FLASH APP代码
            }
            else
            {
                printf("非FLASH应用程序,无法执行!\r\n");
            }
            clearflag = 7; // 标志更新了显示,并且设置7*300ms后清除显示
        }

        if (key == KEY0_PRES)
        {
            printf("开始执行SRAM用户代码!!\r\n");
            if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x20000000) // 判断是否为0X20XXXXXX.
            {
                iap_load_app(0X20001000); // SRAM地址
            }
            else
            {
                printf("非SRAM应用程序,无法执行!\r\n");
            }
            clearflag = 7; // 标志更新了显示,并且设置7*300ms后清除显示
        }
    }
}
