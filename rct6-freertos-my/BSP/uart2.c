
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
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; // 复用推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING; // 复用开漏输入
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /*USART2 mode Config*/
    USART_InitStructure.USART_BaudRate = bound;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    // Usart2 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0; // 抢占优先级0~3
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;		  // 子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			  // IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);							  // 根据指定的参数初始化VIC寄存器


    USART_Init(USART2, &USART_InitStructure);

    // USART_ITConfig(USART2, USART_IT_RXNE, ENABLE); // 开启串口接受中断，和DMA接收互斥

    USART2_DMA_Config();
    USART2_DMA_Rec_Config() ;

    USART_Cmd(USART2, ENABLE);
}

volatile uint8_t USART2_DMA_TX_Complete = 1;

void USART2_DMA_Config(void)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 开启相关时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

    // 配置DMA1通道7 (USART2_TX对应的DMA通道)
    DMA_DeInit(DMA1_Channel7);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = 0; // 后续发送时设置
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
    DMA_InitStructure.DMA_BufferSize = 0; // 后续发送时设置
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel7, &DMA_InitStructure);

    // 配置DMA传输完成中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel7_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能DMA传输完成中断
    DMA_ITConfig(DMA1_Channel7, DMA_IT_TC, ENABLE);

    // 使能USART2的DMA发送
    USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);

    // 使能USART2
    // USART_Cmd(USART2, ENABLE);
}

void USART2_DMA_Send(uint8_t *buffer, uint16_t size)
{
    // 等待上次传输完成
    while(!USART2_DMA_TX_Complete);
    USART2_DMA_TX_Complete = 0;

    // 关闭DMA传输
    DMA_Cmd(DMA1_Channel7, DISABLE);
    
    // 设置内存地址和传输数量
    DMA1_Channel7->CMAR = (uint32_t)buffer;
    DMA1_Channel7->CNDTR = size;
    
    // 启动DMA传输
    DMA_Cmd(DMA1_Channel7, ENABLE);
}

// DMA1通道7中断服务函数
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

// 接收相关变量
// 接收缓冲区大小
#define USART2_RX_BUFFER_SIZE 256
volatile uint8_t USART2_RX_Buffer[USART2_RX_BUFFER_SIZE];
volatile uint16_t USART2_RX_Count = 0;
volatile uint8_t USART2_RX_Flag = 0;

void USART2_DMA_Rec_Config(void) {
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    // 配置DMA1通道6 (USART2_RX)
    DMA_DeInit(DMA1_Channel6);
    DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
    DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USART2_RX_Buffer;
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
    DMA_InitStructure.DMA_BufferSize = USART2_RX_BUFFER_SIZE;
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
    DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;  // 循环模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_High;
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
    DMA_Init(DMA1_Channel6, &DMA_InitStructure);
    
    // 配置DMA接收中断
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel6_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 使能DMA接收通道的半传输、传输完成中断
    DMA_ITConfig(DMA1_Channel6, DMA_IT_HT, ENABLE);  // 半传输中断
    DMA_ITConfig(DMA1_Channel6, DMA_IT_TC, ENABLE);  // 传输完成中断
    // DMA_ITConfig(DMA1_Channel6, DMA_IT_TE, ENABLE);  // 传输错误中断

    // 使能USART2空闲中断
    USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);

    // 使能USART2的DMA发送和接收请求
    // USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
    USART_DMACmd(USART2, USART_DMAReq_Rx, ENABLE);

    // 使能DMA通道
    DMA_Cmd(DMA1_Channel6, ENABLE);  // 启动接收通道

}

// 保证这个buf可以装下完整一帧数据, 配合uart idle检测到结束事件
u8 dma_full_buf[1024];
int dma_full_buf_id = 0;
void processFullBuffer(void) {
    // 复制数据
    for(int i=0; i < USART2_RX_BUFFER_SIZE; i++)
    {
        dma_full_buf[dma_full_buf_id] = USART2_RX_Buffer[i];
        dma_full_buf_id++;
    }
    dma_full_buf[dma_full_buf_id] = 0;    
}

// DMA1通道6中断服务函数（接收通道）
void DMA1_Channel6_IRQHandler(void)
{
    // 半传输中断
    if(DMA_GetITStatus(DMA1_IT_HT6))
    {
        DMA_ClearITPendingBit(DMA1_IT_HT6);
        // 计算已接收数据长度（前半部分）
        // USART2_RX_Count = USART2_RX_BUFFER_SIZE / 2;
        // USART2_RX_Flag = 1;
        // printf("half\r\n");
        // 在这里可以处理前半部分数据
        // ProcessHalfBuffer(void);
    }
    
    // 传输完成中断
    if(DMA_GetITStatus(DMA1_IT_TC6))
    {
        DMA_ClearITPendingBit(DMA1_IT_TC6);
        // 计算已接收数据长度（后半部分）
        // USART2_RX_Count = USART2_RX_BUFFER_SIZE;
        // USART2_RX_Flag = 1;
        // printf("full\r\n");
        // 在这里可以处理后半部分数据
        processFullBuffer();
    }
    
    // 传输错误中断
    if(DMA_GetITStatus(DMA1_IT_TE6))
    {
        DMA_ClearITPendingBit(DMA1_IT_TE6);
        
        // 错误处理
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
        // 计算接收到的数据长度
        count = USART2_RX_Count;
        
        // 复制数据
        for(; i < count; i++)
        {
            buffer[i] = USART2_RX_Buffer[i];
        }

        buffer[i] = 0;
        
        // 清除标志
        USART2_RX_Flag = 0;
        USART2_RX_Count = 0;
    }
    
    return count;
}

uint8_t receive_buffer[256];
uint16_t receive_len;

void test_dma_rec(void) {
    // 检查是否接收到数据
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
u8 USART2_RX_BUF[REC_BUF_MAX]; // 接收缓冲,最大USART_REC_LEN个字节.

// 接收状态， //串口收到结尾标识符:0D 0A， USART_RX_STA = 2，
// 如果收到完整一帧数据 = 3；如果收到最后一个字符后3ms没有下一个字符，认为一帧接收完成；
u8 USART2_RX_STA = 0; // 接收状态标记

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

void USART2_IRQHandler(void) // 串口2中断服务程序
{
	u8 data;

    // 通过读取 DR 寄存器（隐式清除 RXNE 标志）或显式调用 USART_ClearITPendingBit(USART1, USART_IT_RXNE)
	if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) // 接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		data = USART_ReceiveData(USART2); // 读取接收到的数据

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

    // USART空闲中断是指当USART接收线上持续一帧数据位时间没有数据传输时触发的中断。
    // DMA是持续接收的，它不知道何时是一个完整数据包的结束
    // IDLE中断可以用来判断一个数据包的接收完成
    // 如果一帧数据大于DMA的buf长度，那么DMA的buf满了之后会从头开始写，前面的内容被覆盖掉
    // 所以还需要检测DMA满的中断，在中断里面把数据copy中，配合这里即可得到完整的数据；前提是DMA的buf不能太大，能够在覆盖写入前把buf copy完；

    if(USART_GetITStatus(USART2, USART_IT_IDLE) != RESET)
    {
        // 清除IDLE标志位（需要先读SR，再读DR）

        USART2->SR;
        USART2->DR;
        
        // printf("dma u2 rec end\r\n");

        // 计算接收到的数据长度
        // DMA_GetCurrDataCounter：Returns the number of remaining data units in the current DMAy Channelx transfer.
        USART2_RX_Count = USART2_RX_BUFFER_SIZE - DMA_GetCurrDataCounter(DMA1_Channel6);
        
        // 设置接收完成标志
        USART2_RX_Flag = 1;
        
        // 重新设置DMA传输数据量
        DMA_Cmd(DMA1_Channel6, DISABLE);
        DMA_SetCurrDataCounter(DMA1_Channel6, USART2_RX_BUFFER_SIZE);
        DMA_Cmd(DMA1_Channel6, ENABLE);
    }
}
