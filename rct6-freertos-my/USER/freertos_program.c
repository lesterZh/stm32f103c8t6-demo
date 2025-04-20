

#include "freertos_program.h"

static TaskHandle_t startTaskHandler;
static TaskHandle_t myTaskHandler_1;
static TaskHandle_t myTaskHandler_2;
static TaskHandle_t uart1_rec_task_handler;
static TaskHandle_t uart2_rec_task_handler;
static TaskHandle_t lcd_show_task_handler;

void task_begin(void *arg);
void myTask_1(void *arg);
void myTask_2(void *arg);
void uart1_rec_task(void *arg);
void uart2_rec_task(void *arg);
void lcd_show_task(void *arg);

void test_queue_init(void);
void createTimer(void);

void freertos_start_tasks(void)
{
    xTaskCreate(task_begin, "task_begin", 128, NULL, 1, &startTaskHandler);
    vTaskStartScheduler();
}

void task_begin(void *arg)
{
    taskENTER_CRITICAL();
    xTaskCreate(myTask_1, "myTask_1", 128, NULL, 2, &myTaskHandler_1);
    xTaskCreate(myTask_2, "myTask_2", 128, NULL, 2, &myTaskHandler_2);
    xTaskCreate(uart1_rec_task, "uart1_rec_task", 128, NULL, 2, &uart1_rec_task_handler);
    xTaskCreate(uart2_rec_task, "uart2_rec_task", 128, NULL, 1, &uart2_rec_task_handler);
    xTaskCreate(lcd_show_task, "lcd_show_task", 128, NULL, 2, &lcd_show_task_handler);
    vTaskDelete(NULL);

    createTimer();
    test_queue_init();

    taskEXIT_CRITICAL();
}

void myTask_1(void *arg)
{
    while (1)
    {
        flip_LED();
        vTaskDelay(300);
    }
}

void myTask_2(void *arg)
{
    while (1)
    {
        // flip_LED(void);
        vTaskDelay(1000);
        // printf("free rtos:%d\r\n", cnt++);
        // uart2SendString("hi, uart2\r\n");
        test_uart2_dma_send();
    }
}

char * bytes_to_hexstr(const uint8_t *input, int len)
{
    static char output[128] = {0};

    for (int i = 0; i < len; i++)
    {
        sprintf(&output[i * 3], "%02X,", input[i]);  // 大写 hex
        // 或者用 "%02x" 为小写 hex
    }
    output[len * 3] = '\0';  // 添加字符串结束符
    return output;
}

void uart1_rec_task(void *arg) {
    while (1)
    {
        vTaskDelay(10);
        if (isUart1RecFrame()) {
            u16 addr = 0, val = 0;
            u8 *buf = getUart1RecBuf();
            int len = getUart1RecLen();
            printf("rec frame:%s\r\n", buf);
            printf("rec frame hex:%s\r\n", bytes_to_hexstr(buf, len));
            parseDiwenOneWord(buf, len , &addr, &val);

            if (addr == 0x1234) {
                printf("ok\r\n");
            }
            resetUart1RecBuf();
        }
    }
}

void uart2_rec_task(void *arg) {
    while (1)
    {
        vTaskDelay(10);
        test_dma_rec();

        if (0 && isUart2RecFrame()) {
            u8 *buf = getUart2RecBuf();
            int len = getUart2RecLen();
            printf("uart2 rec frame:%s\r\n", buf);
            resetUart2RecBuf();
        }
    }
}

void lcd_show_task(void *arg) {
    int oled_cnt = 0;
    u8 oled_show[20];

    while (1) {
        sprintf((char *)oled_show, "cnt:%d", oled_cnt++);
        OLED_ClearLine(4);
        OLED_ShowString(6, 4, oled_show);
        vTaskDelay(1000);
    }
}

typedef struct {
    uint8_t sensor_id;
    float value;
} SensorData;

QueueHandle_t sensorQueue;

void vSenderTask(void *pvParameters)
{
    SensorData data = { .sensor_id = 1, .value = 36.5 };

    while (1)
    {
        data.sensor_id++;
        xQueueSend(sensorQueue, &data, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vReceiverTask(void *pvParameters)
{
    SensorData rxData;

    while (1)
    {
        if (xQueueReceive(sensorQueue, &rxData, portMAX_DELAY) == pdPASS)
        {
            printf("Sensor %d: %.2f\r\n", rxData.sensor_id, rxData.value);
        }
    }
}

void test_queue_init(void) {
    sensorQueue = xQueueCreate(10, sizeof(SensorData));
    if (sensorQueue == NULL) {
        // 队列创建失败
        while (1);
    }

    xTaskCreate(vSenderTask, "Sender", 128, NULL, 1, NULL);
    xTaskCreate(vReceiverTask, "Receiver", 128, NULL, 1, NULL);
}


void vTimerCallback(void * xTimer)
{
    // 超时后执行的函数
    printf("Timer expired afer 1000 ms! \r\n");
}

void createTimer(void)
{
    void * xTimerHandle = xTimerCreate(
        "MyTimer",                 // 名称
        pdMS_TO_TICKS(1000),      // 周期（tick）
        pdFALSE,                   // 是否自动重载, false一次性任务
        NULL,                     // timer ID（可用作上下文）
        vTimerCallback            // 回调函数
    );

    if (xTimerHandle != NULL)
    {
        xTimerStart(xTimerHandle, 0); // 启动定时器
    }
}

