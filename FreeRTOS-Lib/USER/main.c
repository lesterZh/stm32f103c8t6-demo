/*****************************************************************************
  �ļ�: main.c
  ����: Zhengyu https://gzwelink.taobao.com
  �汾: V1.0.0
  ʱ��: 20200401
    ƽ̨:MINI-STM32F103RCT6

*******************************************************************************/
#include "sys.h"
#include "FreeRTOS.h"
#include "task.h"

// �������ȼ�
#define START_TASK_PRIO 1
// �����ջ��С
#define START_STK_SIZE 128
// ������
TaskHandle_t StartTask_Handler;
// ������
void start_task(void *pvParameters);

// �������ȼ�
#define LED0_TASK_PRIO 2
// �����ջ��С
#define LED0_STK_SIZE 50
// ������
TaskHandle_t LED0Task_Handler;

// �������ȼ�
#define LED1_TASK_PRIO 3
// �����ջ��С
#define LED1_STK_SIZE 50
// ������
TaskHandle_t LED1Task_Handler;
// ������
void led0_task(void *pvParameters);

// ����1������
void LED1_task(void const *argument)
{

    for (;;)
    {
        // GPIO_SetBits(GPIOB, GPIO_Pin_4); // PB4�����
        // vTaskDelay(100);                 // �ȴ�100ms
    }
}

// ����2������
void LED2_task(void const *argument)
{

    for (;;)
    {
        // set 0, ����
        GPIO_SetBits(GPIOB, GPIO_Pin_4);
        vTaskDelay(750);
        GPIO_ResetBits(GPIOB, GPIO_Pin_4); // PB4�����
        vTaskDelay(250);                   // �ȴ�250ms
    }
}

// LED ��ʼ��PB4
void GPIO_Config_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // ʹ��PB�˿�ʱ��ʹ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;             // PB4 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;      // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;     // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                // �����趨������ʼ��GPIOB4
    GPIO_ResetBits(GPIOB, GPIO_Pin_4);                    // PB4 �����
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);  // ���ø���ʱ��
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);  // ȫ����SW+JTAG������JTRST��PB4����
}
// ��ʼ����������
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); // �����ٽ���
    // ����LED0����
    xTaskCreate((TaskFunction_t)LED1_task,
                (const char *)"LED_Thread1",
                (uint16_t)LED0_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED0_TASK_PRIO,
                (TaskHandle_t *)&LED0Task_Handler);
    // ����Uart����
    xTaskCreate((TaskFunction_t)LED2_task,
                (const char *)"LED_Thread2",
                (uint16_t)LED1_STK_SIZE,
                (void *)NULL,
                (UBaseType_t)LED1_TASK_PRIO,
                (TaskHandle_t *)&LED1Task_Handler);
    vTaskDelete(StartTask_Handler); // ɾ����ʼ����
    taskEXIT_CRITICAL();            // �˳��ٽ���
}

void SetSysClockTo72(void)
{
    /* 1. ���� RCC ����ΪĬ��ֵ */
    RCC_DeInit();

    /* 2. ���ⲿ���پ��� HSE */
    RCC_HSEConfig(RCC_HSE_ON);

    /* 3. �ȴ� HSE ���� */
    ErrorStatus HSEStartUpStatus = RCC_WaitForHSEStartUp();
    if (HSEStartUpStatus == SUCCESS)
    {
        /* 4. ����ʱ��Ԥ��Ƶ���� */
        RCC_HCLKConfig(RCC_SYSCLK_Div1);    // AHB = SYSCLK /1 = 72MHz
        RCC_PCLK1Config(RCC_HCLK_Div2);     // APB1 = HCLK /2 = 36MHz
        RCC_PCLK2Config(RCC_HCLK_Div1);     // APB2 = HCLK /1 = 72MHz

        /* 5. ���� PLL��HSE ��Ϊ���� �� 9 = 72MHz */
        RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

        /* 6. ���� PLL */
        RCC_PLLCmd(ENABLE);

        /* 7. �ȴ� PLL �ȶ� */
        while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET);

        /* 8. ����ϵͳʱ����ԴΪ PLL */
        RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

        /* 9. �ȴ��л���� */
        while (RCC_GetSYSCLKSource() != 0x08);

        /* 10. ���� Flash �ӳ����ڣ�72MHz = 2 wait states�� */
        FLASH_SetLatency(FLASH_Latency_2);
        FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);
    }
    else
    {
        // HSE ����ʧ�ܣ������쳣
        while (1);
    }
}

// �������������ⲿ8M����72Mϵͳ��Ƶ��������void SetSysClock(void)������ѡ����Ƶ������
int main(void)
{
    // Ĭ�ϵ���static void SetSysClockTo72(void)��ʹ��HSE�ⲿ��������ʱ��72M, APB1=36M,APB2=72M
    SetSysClockTo72();
    
    GPIO_Config_Init(); // ��ʼ��GPIO,PB4���ó��������

    // ������ʼ����
    xTaskCreate((TaskFunction_t)start_task,          // ������
                (const char *)"start_task",          // ��������
                (uint16_t)START_STK_SIZE,            // �����ջ��С
                (void *)NULL,                        // ���ݸ��������Ĳ���
                (UBaseType_t)START_TASK_PRIO,        // �������ȼ�
                (TaskHandle_t *)&StartTask_Handler); // ������
    vTaskStartScheduler();                           // �����������
    while (1)
    {
    }
}
