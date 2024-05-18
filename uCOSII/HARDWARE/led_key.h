#ifndef __LED_KEY_H__
#define __LED_KEY_H__

#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "sys.h"
#include "oled.h"
#include "timer.h"

// 按下为高电平
#define WK_UP_KEY GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0)
#define KEY1 PAin(1)

#define WKUP_PRES 2
#define LED PBout(4)

extern u8 WK_UP_INT_STATUS;

void LED_Init(void);
void gpio_A0_interrupt_init(void);
void flip_LED(void);

#endif

