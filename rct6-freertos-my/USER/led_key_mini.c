
#include "led_key.h"

void led_key_init_mini_bsp(void) {
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE); // ʹ�� �˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;        // LED0-->PC7 �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // �������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // IO���ٶ�Ϊ 2, 10, 50MHz
    GPIO_Init(GPIOC, &GPIO_InitStructure);           // �����趨������ʼ��GPIOB.4
    GPIO_SetBits(GPIOC, GPIO_Pin_7);


    // mini���ӵ� ����ֱ�ӽӵ���, ��������������ʱ����0
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

