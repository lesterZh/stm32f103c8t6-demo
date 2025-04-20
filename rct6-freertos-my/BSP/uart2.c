
#include "uart2.h"

void USART2_DMA_Config(void);
void USART2_DMA_Rec_Config(void);

void uart2_init(int bound)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    /*config USART clock*/
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    /*USART2 GPIO config*/
    //  TX-PA2 RX-PA3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // �����������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // ���ÿ�©����
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*USART2 mode Config*/
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    // Usart2 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // ��ռ���ȼ�0~3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // �����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);							  // ����ָ���Ĳ�����ʼ��VIC�Ĵ���


    USART_Init(USART2, &USART_InitStructure);

    // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // �������ڽ����жϣ���DMA���ջ���

    USART2_DMA_Config();
    USART2_DMA_Rec_Config() ;

    USART_Cmd(USART2, ENABLE);
}

volatile uint8_t USART2_DMA_TX_Complete = 1;

void USART2_DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // �������ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // ����DMA1ͨ��7 (USART2_TX��Ӧ��DMAͨ��)
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = 0; // ��������ʱ����
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0; // ��������ʱ����
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    // ����DMA��������ж�
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // ʹ��DMA��������ж�
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    // ʹ��USART2��DMA����
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    // ʹ��USART2
    // USART_Cmd(USART2, ENABLE);
}

void USART2_DMA_Send(uint8_t *buffer, uint16_t size)
{
    // �ȴ��ϴδ������
    while(!USART2_DMA_TX_Complete);
    USART2_DMA_TX_Complete = 0;

    // �ر�DMA����
    DMA_Cmd(DMA1_Channel7, DISABLE);
    
    // �����ڴ��ַ�ʹ�������
    DMA1_Channel7->CMAR = (uint32_t)buffer;
    DMA1_Channel7->CNDTR = size;
    
    // ����DMA����
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

// DMA1ͨ��7�жϷ�����
void DMA1_Channel7_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC7))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC7);
        USART2_DMA_TX_Complete = 1;
        // printf("u2 dma end\r\n");
    }
}

void test_uart2_dma_send(void) {
    static int cnt = 0;
    static uint8_t tx_buf[40];
    sprintf(tx_buf, "Hello STM32 USART2 DMA! -- %d\r\n", cnt++);
    USART2_DMA_Send(tx_buf, sizeof(tx_buf) - 1);
}

// ������ر���
// ���ջ�������С
#define USART2_RX_BUFFER_SIZE 256
volatile uint8_t USART2_RX_Buffer[USART2_RX_BUFFER_SIZE];
volatile uint16_t USART2_RX_Count = 0;
volatile uint8_t USART2_RX_Flag = 0;

void USART2_DMA_Rec_Config(void) {
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // ����DMA1ͨ��6 (USART2_RX)
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_RX_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART2_RX_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // ѭ��ģʽ
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    // ����DMA�����ж�
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // ʹ��DMA����ͨ���İ봫�䡢��������ж�
    DMA_ITConfig(DMA1_Channel6, DMA_IT_HT, ENABLE);  // �봫���ж�
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);  // ��������ж�
    // DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, ENABLE);  // ��������ж�

    // ʹ��USART2�����ж�
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    // ʹ��USART2��DMA���ͺͽ�������
    // USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

    // ʹ��DMAͨ��
    DMA_Cmd(DMA1_Channel6, ENABLE);  // ��������ͨ��

}

// ��֤���buf����װ������һ֡����, ���uart idle��⵽�����¼�
u8 dma_full_buf[1024];
int dma_full_buf_id = 0;
void processFullBuffer(void) {
    // ��������
    for(int i=0; i < USART2_RX_BUFFER_SIZE; i++)
    {
        dma_full_buf[dma_full_buf_id] = USART2_RX_Buffer[i];
        dma_full_buf_id++;
    }
    dma_full_buf[dma_full_buf_id] = 0;    
}

// DMA1ͨ��6�жϷ�����������ͨ����
void DMA1_Channel6_IRQHandler(void)
{
    // �봫���ж�
    if(DMA_GetITStatus(DMA1_IT_HT6))
    {
        DMA_ClearITPendingBit(DMA1_IT_HT6);
        // �����ѽ������ݳ��ȣ�ǰ�벿�֣�
        // USART2_RX_Count = USART2_RX_BUFFER_SIZE / 2;
        // USART2_RX_Flag = 1;
        // printf("half\r\n");
        // ��������Դ���ǰ�벿������
        // ProcessHalfBuffer(void);
    }
    
    // ��������ж�
    if(DMA_GetITStatus(DMA1_IT_TC6))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        // �����ѽ������ݳ��ȣ���벿�֣�
        // USART2_RX_Count = USART2_RX_BUFFER_SIZE;
        // USART2_RX_Flag = 1;
        // printf("full\r\n");
        // ��������Դ����벿������
        processFullBuffer();
    }
    
    // ��������ж�
    if(DMA_GetITStatus(DMA1_IT_TE6))
    {
        DMA_ClearITPendingBit(DMA1_IT_TE6);
        
        // ������
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, USART2_RX_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}

uint16_t USART2_DMA_Receive(uint8_t *buffer)
{
    uint16_t count = 0;
    uint16_t i = 0;

    if(USART2_RX_Flag)
    {
        // ������յ������ݳ���
        count = USART2_RX_Count;
        
        // ��������
        for(; i < count; i++)
        {
            buffer[i] = USART2_RX_Buffer[i];
        }

        buffer[i] = 0;
        
        // �����־
        USART2_RX_Flag = 0;
        USART2_RX_Count = 0;
    }
    
    return count;
}

uint8_t receive_buffer[256];
uint16_t receive_len;

void test_dma_rec(void) {
    // ����Ƿ���յ�����
    receive_len = USART2_DMA_Receive(receive_buffer);
    if(receive_len > 0)
    {
        // printf("dma rec uart2:%s\r\n", receive_buffer);
        if (dma_full_buf_id > 0) {
            dma_full_buf_id = 0;
            printf("%s", dma_full_buf);
        }

        printf("%s", receive_buffer);
    }
}


void uart2SendChar(char ch)
{
	USART_SendData(USART2, ch);
	while (USART_GetFlagStatus(USART2, USART_FLAG_TC) != SET)
		;
}

void uart2SendChars(char chs[], int len)
{
	u8 i;
	for (i = 0; i < len; i++)
	{
		uart2SendChar(chs[i]);
	}
}


void uart2SendString(char *str)
{
	while (*str != '\0')
	{
		uart2SendChar(*str);
		str++;
	}
}

#define REC_BUF_MAX 64
u8 USART2_RX_BUF[REC_BUF_MAX]; // ���ջ���,���USART_REC_LEN���ֽ�.

// ����״̬�� //�����յ���β��ʶ��:0D 0A�� USART_RX_STA = 2��
// ����յ�����һ֡���� = 3������յ����һ���ַ���3msû����һ���ַ�����Ϊһ֡������ɣ�
u8 USART2_RX_STA = 0; // ����״̬���

int USART2_RX_LEN = 0;

void resetUart2RecBuf(void);
u8 * getUart2RecBuf(void);
void set_uart2_rec_tick(void);
int isUart2RecFrame(void);
int getUart2RecLen(void);

void resetUart2RecBuf(void) {
    USART2_RX_LEN = 0;
    USART2_RX_STA = 0;
    USART2_RX_BUF[0] = 0;
}

u8 * getUart2RecBuf(void) {
    return USART2_RX_BUF;
}

int getUart2RecLen(void) {
    return USART2_RX_LEN;
}

int uart2_last_rec_sys_tick = 0;
extern int sys_tick_1ms;

void set_uart2_rec_tick(void) {
    uart2_last_rec_sys_tick = sys_tick_1ms;
}

int isUart2RecFrame(void) {
    int dur = sys_tick_1ms - uart2_last_rec_sys_tick;
    return USART2_RX_LEN > 0 && dur >= 3;
}

void USART2_IRQHandler(void) // ����2�жϷ������
{
	u8 data;

    // ͨ����ȡ DR �Ĵ�������ʽ��� RXNE ��־������ʽ���� USART_ClearITPendingBit(USART1, USART_IT_RXNE)
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) // �����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		data = USART_ReceiveData(USART2); // ��ȡ���յ�������

        // uart2SendChar(data);

        USART2_RX_BUF[USART2_RX_LEN] = data;
        if (USART2_RX_LEN > 2) {
            if (USART2_RX_BUF[USART2_RX_LEN-1] == 0x0A && USART2_RX_BUF[USART2_RX_LEN-2] == 0x0D) {
                USART2_RX_STA = 2;
            }
        }

        USART2_RX_LEN = (USART2_RX_LEN+1) % REC_BUF_MAX;
        set_uart2_rec_tick();
	}

    // USART�����ж���ָ��USART�������ϳ���һ֡����λʱ��û�����ݴ���ʱ�������жϡ�
    // DMA�ǳ������յģ�����֪����ʱ��һ���������ݰ��Ľ���
    // IDLE�жϿ��������ж�һ�����ݰ��Ľ������
    // ���һ֡���ݴ���DMA��buf���ȣ���ôDMA��buf����֮����ͷ��ʼд��ǰ������ݱ����ǵ�
    // ���Ի���Ҫ���DMA�����жϣ����ж����������copy�У�������Ｔ�ɵõ����������ݣ�ǰ����DMA��buf����̫���ܹ��ڸ���д��ǰ��buf copy�ꣻ

    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // ���IDLE��־λ����Ҫ�ȶ�SR���ٶ�DR��

        USART2->SR;
        USART2->DR;
        
        // printf("dma u2 rec end\r\n");

        // ������յ������ݳ���
        // DMA_GetCurrDataCounter��Returns the number of remaining data units in the current DMAy Channelx transfer.
        USART2_RX_Count = USART2_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        // ���ý�����ɱ�־
        USART2_RX_Flag = 1;
        
        // ��������DMA����������
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, USART2_RX_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}
