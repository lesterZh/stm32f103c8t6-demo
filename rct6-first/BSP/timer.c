#include "timer.h"

// ͨ�ö�ʱ�� 3 �жϳ�ʼ��
// ����ʱ��ѡ��Ϊ APB1 �� 2 ������ APB1 Ϊ 36M
// arr���Զ���װֵ��
// psc��ʱ��Ԥ��Ƶ��; 7199��Ӧ10K 0.1ms; arr=100-1;��Ӧ10ms
// ����ʹ�õ��Ƕ�ʱ�� 3!
void TIM3_Int_Init(u16 arr, u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStr;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE); // ��ʱ�� TIM3 ʹ��

    // ��ʱ�� TIM3 ��ʼ��
    TIM_TimeBaseInitStr.TIM_Period = arr;                     // �����Զ���װ�ؼĴ������ڵ�ֵ
    TIM_TimeBaseInitStr.TIM_Prescaler = psc;                  // ����ʱ��Ƶ�ʳ�����Ԥ��Ƶֵ
    TIM_TimeBaseInitStr.TIM_ClockDivision = TIM_CKD_DIV1;     // ����ʱ�ӷָ�
    TIM_TimeBaseInitStr.TIM_CounterMode = TIM_CounterMode_Up; // TIM ���ϼ���
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStr);             // �ڳ�ʼ�� TIM3
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);                  // ����������ж�
    // �ж����ȼ� NVIC ����
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;           // TIM3 �ж�
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3; // ��ռ���ȼ� 0 ��
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;        // �����ȼ� 3 ��
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;           // IRQ ͨ����ʹ��
    NVIC_Init(&NVIC_InitStructure);                           // �ܳ�ʼ�� NVIC �Ĵ���
    TIM_Cmd(TIM3, ENABLE);                                    // ��ʹ�� TIM3
}

