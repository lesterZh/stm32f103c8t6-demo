#ifndef __LED_KEY_H__
#define __LED_KEY_H__

#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"

#define WK_UP GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define WKUP_PRES 2

extern u8 WK_UP_INT_STATUS;

void LED_Init(void);
void gpio_A0_interrupt_init(void);
void flip_LED(void);

void led_key_init_mini_bsp(void);
void led_flip_mini_bsp(void);
int isPC6KeyDown(void);

#endif

