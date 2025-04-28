
#ifndef __FREERTOS_DEMO_H
#define __FREERTOS_DEMO_H

#include "stm32f10x.h" // Device header
#include "FreeRTOS.h"
#include "task.h"
#include "led_key.h"
#include "usart.h"
#include "oled.h"
#include "diwen.h"
#include "uart2.h"
#include <queue.h>

#include "timers.h"
#include "sys.h"

void freertos_start_tasks(void);

#endif

