#include "led.h"
#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "key.h"
#include "lcd.h"
#include "stmflash.h"
#include "iap.h"
#include "stm32f10x.h"

// ALIENTEK MiniSTM32������ʵ��32
// ����IAPʵ��
// FLASH����: 0X0800 0000~0x0800 8000,��IAPʹ��,��32K�ֽ�,FLASH APP����256-32=224KB����.
//  SRAM����: 0X2000 1000����ʼ, ���ڴ��SRAM IAP����,��44K�ֽڿ���,�û��������ж�44K�ռ����ROM��RAM���ķ���
//            �ر�ע�⣬SRAM APP��ROMռ�������ܴ���41K�ֽڣ���Ϊ�����������һ�ν���41K�ֽڣ����ܳ���������ơ�
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

int t3_cnt = 0;
int sys_tick_1ms = 0;

// ��ʱ�� 3 �жϷ������
// 1ms����
void TIM3_IRQHandler(void) // TIM3 �ж�
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET) // ��� TIM3 �����жϷ������
    {
        TIM_ClearITPendingBit(TIM3, TIM_IT_Update); // ��� TIM3 �����жϱ�־
        // printf("t3_cnt:%d\r\n", t3_cnt++);
        t3_cnt++;
        sys_tick_1ms++;
        if (t3_cnt % 2000 == 0) {
            printf("t3:%d\r\n", t3_cnt);
        }
    }
}

int main(void)
{
    // �������flag�л�flash��sram���������
    // flash��bootloader������3���ڼ�⴮���Ƿ��и��£����û����ֱ����ת��app��ַ���У�����оͽ��ղ�д��flash
    // �����sram�����һֱ�ȴ��ڽ��գ��յ����л���sram���С�
    u8 runFlashIAPApp = 1;

    u8 runSRAMIAPApp = 0;
    u8 uartRecAppEnd = 0;


    u32 t = 0;
    u8 key;
    u16 oldcount = 0; // �ϵĴ��ڽ�������ֵ
    u16 applenth = 0; // ���յ���app���볤��
    u8 clearflag = 0;
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // �����ж����ȼ�����2
    delay_init();                                   // ��ʱ������ʼ��
    uart_init(115200);                              // ���ڳ�ʼ��Ϊ
    LED_Init();                                     // ��ʼ����LED���ӵ�Ӳ���ӿ�
    LCD_Init();                                     // ��ʼ��LCD
    KEY_Init();                                     // ������ʼ��

    // TIM3_Int_Init(9, 7199);//10K, 1ms

    printf("bootloader start\r\n");

    while (1)
    {
        if (USART_RX_CNT)
        {
            if (oldcount == USART_RX_CNT) // ��������,û���յ��κ�����,��Ϊ�������ݽ������.
            {
                applenth = USART_RX_CNT;
                oldcount = 0;
                USART_RX_CNT = 0;
                printf("�û�����������!\r\n");
                printf("���볤��: %d Bytes\r\n", applenth);
                uartRecAppEnd = 1;
            }
            else
            {
                oldcount = USART_RX_CNT;
            }
        }

        t++;
        delay_ms(100);

        if (t % 10 == 0)
        {
            LED0 = !LED0;
            printf("run check %d\r\n", t / 10);

            if (runFlashIAPApp && uartRecAppEnd && t > 30) {
                printf("start update app to flash\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // �ж��Ƿ�Ϊ0X08XXXXXX.
                {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, applenth); // ����FLASH����
                    printf("�̼��������!\r\n");
                }
                else
                {
                    printf("��FLASHӦ�ó���!\r\n");
                }
                uartRecAppEnd = 0;

            }

            if (runFlashIAPApp && t > 30) {
                printf("go to app\r\n");
                iap_load_app(FLASH_APP1_ADDR); 
            }

            if (runSRAMIAPApp && uartRecAppEnd && t > 30) {
                printf("start go to sram app\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x20000000) // �ж��Ƿ�Ϊ0X20XXXXXX.
                {
                    printf("go to sram app\r\n");
                    iap_load_app(0X20001000); // SRAM��ַ
                }
                else
                {
                    printf("��SRAMӦ�ó���,�޷�ִ��!\r\n");
                }
            }
        }


        
        key = KEY_Scan(0);
        if (key == WKUP_PRES) // WK_UP��������
        {
            if (applenth)
            {
                printf("��ʼ���¹̼�...\r\n");
                if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x08000000) // �ж��Ƿ�Ϊ0X08XXXXXX.
                {
                    iap_write_appbin(FLASH_APP1_ADDR, USART_RX_BUF, applenth); // ����FLASH����
                    printf("�̼��������!\r\n");
                }
                else
                {
                    printf("��FLASHӦ�ó���!\r\n");
                }
            }
            else
            {
                printf("û�п��Ը��µĹ̼�!\r\n");
            }
            clearflag = 7; // ��־��������ʾ,��������7*300ms�������ʾ
        }

        if (key == KEY1_PRES)
        {
            printf("��ʼִ��FLASH�û�����!!\r\n");
            if (((*(vu32 *)(FLASH_APP1_ADDR + 4)) & 0xFF000000) == 0x08000000) // �ж��Ƿ�Ϊ0X08XXXXXX.
            {
                iap_load_app(FLASH_APP1_ADDR); // ִ��FLASH APP����
            }
            else
            {
                printf("��FLASHӦ�ó���,�޷�ִ��!\r\n");
            }
            clearflag = 7; // ��־��������ʾ,��������7*300ms�������ʾ
        }

        if (key == KEY0_PRES)
        {
            printf("��ʼִ��SRAM�û�����!!\r\n");
            if (((*(vu32 *)(0X20001000 + 4)) & 0xFF000000) == 0x20000000) // �ж��Ƿ�Ϊ0X20XXXXXX.
            {
                iap_load_app(0X20001000); // SRAM��ַ
            }
            else
            {
                printf("��SRAMӦ�ó���,�޷�ִ��!\r\n");
            }
            clearflag = 7; // ��־��������ʾ,��������7*300ms�������ʾ
        }
    }
}
