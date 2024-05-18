
#include "led_key.h"

u8 WK_UP_INT_STATUS = 0;

//LED PB4, WK_UP KEY:PA0, KEY1:PA1; I2C2接EEPROM
void LED_Init(void)
{

    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE); // 使能PB,PE端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;        // LED0-->PB.4 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz; // IO口速度为 2, 10, 50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);           // 根据设定参数初始化GPIOB.4
    // GPIO_ResetBits(GPIOB, GPIO_Pin_4);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE); // 配置复用时钟
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE); // 全功能SW+JTAG，禁用JTRST，PB4可用
}

static void init_Key()
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;	  // PA0
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; // PA0 设置成输入，默认下拉
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void gpio_A0_interrupt_init(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	init_Key();

	// GPIOA.0 中断线以及中断初始化配置
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource0);
	EXTI_InitStructure.EXTI_Line = EXTI_Line0;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising; //_Rising  Falling下降沿触发
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // 抢占优先级 2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		 // 子优先级 1
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void flip_LED(void)
{
    if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_4) == Bit_SET)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_4);
    }
    else
    {
        GPIO_SetBits(GPIOB, GPIO_Pin_4);
    }
}
