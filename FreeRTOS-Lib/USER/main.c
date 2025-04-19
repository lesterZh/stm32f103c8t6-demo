/*****************************************************************************
  文件: main.c
  作者: Zhengyu https://gzwelink.taobao.com
  版本: V1.0.0
  时间: 20200401
    平台:MINI-STM32F103RCT6

*******************************************************************************/
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

// 任务优先级
#define START_TASK_PRIO 1
// 任务堆栈大小
#define START_STK_SIZE 128
// 任务句柄
TaskHandle_t StartTask_Handler;
// 任务函数
void start_task(void *pvParameters);

// 任务优先级
#define LED0_TASK_PRIO 2
// 任务堆栈大小
#define LED0_STK_SIZE 50
// 任务句柄
TaskHandle_t LED0Task_Handler;

// 任务优先级
#define LED1_TASK_PRIO 3
// 任务堆栈大小
#define LED1_STK_SIZE 50
// 任务句柄
TaskHandle_t LED1Task_Handler;
// 任务函数
void led0_task(void *pvParameters);

// 任务1，灯灭
void LED1_task(void const *argument)
{

    for (;;)
    {
        // GPIO_SetBits(GPIOB, GPIO_Pin_4); // PB4输出高
        // vTaskDelay(100);                 // 等待100ms
    }
}

// 任务2，灯亮
void LED2_task(void const *argument)
{

    for (;;)
    {
        // set 0, 灯亮
        GPIO_SetBits(GPIOB, GPIO_Pin_4);
        vTaskDelay(750);
        GPIO_ResetBits(GPIOB, GPIO_Pin_4); // PB4输出低
        vTaskDelay(250);                   // 等待250ms
    }
}

// LED 初始化PB4
void GPIO_Config_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能PB端口时钟使能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;             // PB4 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                // 根据设定参数初始化GPIOB4
    GPIO_ResetBits(GPIOB, GPIO_Pin_4);                    // PB4 输出低
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // 配置复用时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);  // 全功能SW+JTAG，禁用JTRST，PB4可用
}
// 开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); // 进入临界区
    // 创建LED0任务
    xTaskCreate((TaskFunction_t)LED1_task,
                (const char *)"LED_Thread1",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&LED0Task_Handler);
    // 创建Uart任务
    xTaskCreate((TaskFunction_t)LED2_task,
                (const char *)"LED_Thread2",
                (uint16_t)LED1_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED1_TASK_PRIO,
                (TaskHandle_t *)&LED1Task_Handler);
    vTaskDelete(StartTask_Handler); // 删除开始任务
    taskEXIT_CRITICAL();            // 退出临界区
}

void SetSysClockTo72(void)
{
    /* 1. 重置 RCC 配置为默认值 */
    RCC_DeInit();

    /* 2. 打开外部高速晶振 HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* 3. 等待 HSE 就绪 */
    ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        /* 4. 设置时钟预分频因子 */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);    // AHB = SYSCLK /1 = 72MHz
        RCC_PCLK1Config(RCC_HCLK_Div2);     // APB1 = HCLK /2 = 36MHz
        RCC_PCLK2Config(RCC_HCLK_Div1);     // APB2 = HCLK /1 = 72MHz

        /* 5. 设置 PLL：HSE 作为输入 × 9 = 72MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        /* 6. 启动 PLL */
        RCC_PLLCmd(ENABLE);

        /* 7. 等待 PLL 稳定 */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

        /* 8. 设置系统时钟来源为 PLL */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* 9. 等待切换完成 */
        while (RCC_GetSYSCLKSource() != 0x08);

        /* 10. 设置 Flash 延迟周期（72MHz = 2 wait states） */
        FLASH_SetLatency(FLASH_Latency_2);
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    }
    else
    {
        // HSE 启动失败，处理异常
        while (1);
    }
}

// 主函数，采用外部8M晶振，72M系统主频，可以在void SetSysClock(void)函数中选择主频率设置
int main(void)
{
    // 默认调用static void SetSysClockTo72(void)，使用HSE外部晶振，设置时钟72M, APB1=36M,APB2=72M
    SetSysClockTo72();
    
    GPIO_Config_Init(); // 初始化GPIO,PB4配置成推挽输出

    // 创建开始任务
    xTaskCreate((TaskFunction_t)start_task,          // 任务函数
                (const char *)"start_task",          // 任务名称
                (uint16_t)START_STK_SIZE,            // 任务堆栈大小
                (void *)NULL,                        // 传递给任务函数的参数
                (UBaseType_t)START_TASK_PRIO,        // 任务优先级
                (TaskHandle_t *)&StartTask_Handler); // 任务句柄
    vTaskStartScheduler();                           // 开启任务调度
    while (1)
    {
    }
}
