

#include "stm32f10x.h" // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "led_key.h"
#include "usart.h"
#include "oled.h"
#include "diwen.h"


static TaskHandle_t startTaskHandler;
static TaskHandle_t myTaskHandler_1;
static TaskHandle_t myTaskHandler_2;
static TaskHandle_t uart1_rec_task_handler;
static TaskHandle_t lcd_show_task_handler;

void task_begin(void *arg);
void myTask_1(void *arg);
void myTask_2(void *arg);
void uart1_rec_task(void *arg);
void lcd_show_task(void *arg);

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
    xTaskCreate(lcd_show_task, "lcd_show_task", 128, NULL, 2, &lcd_show_task_handler);
    vTaskDelete(NULL);
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
    int cnt = 0;
    while (1)
    {
        // flip_LED();
        vTaskDelay(1000);
        // printf("free rtos:%d\r\n", cnt++);
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
            s16 addr = 0, val = 0;
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

void lcd_show_task(void *arg) {
    int oled_cnt = 0;
    u8 oled_show[20];

    while (1) {
        sprintf(oled_show, "cnt:%d", oled_cnt++);
        OLED_ClearLine(4);
        OLED_ShowString(6, 4, oled_show);
        vTaskDelay(1000);
    }
}