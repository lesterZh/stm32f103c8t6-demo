
#include "led_key.h"

void led_key_init_mini_bsp(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // 使能 端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;        // LED0-->PC7 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // IO口速度为 2, 10, 50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);           // 根据设定参数初始化GPIOB.4
    GPIO_SetBits(GPIOC, GPIO_Pin_7);


    // mini板子的 按键直接接地了, 这里上拉，按下时输入0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;	  // PC6
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; // 
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

int isPC6KeyDown(void) {
    u8 val = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_6);
    return val == 0;
}

void led_off_mini_bsp(void) {
    GPIO_SetBits(GPIOC, GPIO_Pin_7);
}

void led_on_mini_bsp(void) {
    GPIO_ResetBits(GPIOC, GPIO_Pin_7);
}

void led_flip_mini_bsp(void) {
    if (GPIO_ReadOutputDataBit(GPIOC, GPIO_Pin_7) == Bit_SET)
    {
        GPIO_ResetBits(GPIOC, GPIO_Pin_7);
    }
    else
    {
        GPIO_SetBits(GPIOC, GPIO_Pin_7);
    }
}

