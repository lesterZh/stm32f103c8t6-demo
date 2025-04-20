/*****************************************************************************
  �ļ�: 24C08.c
  ����: Zhengyu https://gzwelink.taobao.com
  �汾: V1.0.0
  ʱ��: 20200401
    ƽ̨:MINI-STM32F103C8T6

*******************************************************************************/
#include "sys.h"
#include "delay.h"

#define AT24C08_READ_ADDR 0xA1  // ������ʱ�ĵ�ַ
#define AT24C08_WRITE_ADDR 0xA0 // д����ʱ�ĵ�ַ

// IIC ��ʼ��PB10,PB11
void GPIO_Config_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    // ʹ�ܶ˿�ʱ��ʹ��
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // �˿�����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;          // AF����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        // IO���ٶ�Ϊ50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                   // �����趨������ʼ��
}

// IIC2��ʼ��
void IIC_Init(void)
{
    I2C_InitTypeDef I2C_InitStructure;
    GPIO_Config_Init();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2 | RCC_APB2Periph_AFIO, ENABLE);

    I2C_DeInit(I2C2); // ʹ��I2C2
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x30; // ������I2C��ַ,���д��
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100K
    I2C_Init(I2C2, &I2C_InitStructure);
    I2C_Cmd(I2C2, ENABLE);
}
/*
��������: ������ʼ�ź�
��ʱ����Ϊ�ߵ�ƽ��ʱ���������ɸߵ�ƽ��Ϊ�͵�ƽ�Ĺ���
*/
void IIC_SendStart(void)
{
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
    {
        //���Լ��볬ʱ�˳�����
    }
}
/*
��������: ֹͣ�ź�
��ʱ����Ϊ�ߵ�ƽ��ʱ���������ɵ͵�ƽ��Ϊ�ߵ�ƽ�Ĺ���
*/
void IIC_SendStop(void)
{
    I2C_GenerateSTOP(I2C2, ENABLE);
    delay_ms(5);
}

void IIC_SendAddr(u8 addr)
{
    I2C_SendData(I2C2, addr);                               // ��������(��ַ)
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_ADDR & 0xFFFFFF)) // �ȴ���ַ�������
    {
        // ���Լ��볬ʱ�˳�����
    }
}
/*
��������: ��������
*/
void IIC_SendOneByte(u8 addr)
{
    I2C_SendData(I2C2, addr);                              // ��������
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_BTF & 0xFFFFFF)) // �ȴ��������
    {
        // ���Լ��볬ʱ�˳�����
    }
}

/*
��������: ����һ���ֽ�����
*/
u8 IIC_RecvOneByte(void)
{
    u8 data = 0;
    I2C_AcknowledgeConfig(I2C2, ENABLE);                    // ʹ��Ӧ��
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_RXNE & 0xFFFFFF)) // �ȴ�����
    {
        // ���Լ��볬ʱ�˳�����
    }
    data = I2C_ReceiveData(I2C2);         // ��������
    I2C_AcknowledgeConfig(I2C2, DISABLE); // �ر�Ӧ��ʹ��
    return data;
}

/*
��������: дһ���ֽ�
��������:
u8 addr ���ݵ�λ��(0~1023)
u8 data ���ݷ�Χ(0~255)
*/
void AT24C08_WriteOneByte(u16 addr, u8 data)
{
    u8 read_device_addr = AT24C08_READ_ADDR;
    u8 write_device_addr = AT24C08_WRITE_ADDR;
    if (addr < 256 * 1) // ��һ����
    {
        write_device_addr |= 0x0 << 1;
        read_device_addr |= 0x0 << 1;
    }
    else if (addr < 256 * 2) // �ڶ�����
    {
        write_device_addr |= 0x1 << 1;
        read_device_addr |= 0x1 << 1;
    }
    else if (addr < 256 * 3) // ��������
    {
        write_device_addr |= 0x2 << 1;
        read_device_addr |= 0x2 << 1;
    }
    else if (addr < 256 * 4) // ���ĸ���
    {
        write_device_addr |= 0x3 << 1;
        read_device_addr |= 0x3 << 1;
    }
    addr = addr % 256;               // �õ���ַ��Χ
    IIC_SendStart();                 // ��ʼ�ź�
    IIC_SendAddr(write_device_addr); // �����豸��ַ
    IIC_SendOneByte(addr);           // ���ݴ�ŵĵ�ַ
    IIC_SendOneByte(data);           // ���ͽ�Ҫ��ŵ�����
    IIC_SendStop();                  // ֹͣ�ź�
    delay_ms(10);                    // �ȴ�д
}

/*
��������: ��һ���ֽ�
��������:
u8 addr ���ݵ�λ��(0~1023)
����ֵ: ����������
*/
u8 AT24C08_ReadOneByte(u16 addr)
{
    u8 data = 0;
    u8 read_device_addr = AT24C08_READ_ADDR;
    u8 write_device_addr = AT24C08_WRITE_ADDR;
    if (addr < 256 * 1) // ��һ����
    {
        write_device_addr |= 0x0 << 1;
        read_device_addr |= 0x0 << 1;
    }
    else if (addr < 256 * 2) // �ڶ�����
    {
        write_device_addr |= 0x1 << 1;
        read_device_addr |= 0x1 << 1;
    }
    else if (addr < 256 * 3) // ��������
    {
        write_device_addr |= 0x2 << 1;
        read_device_addr |= 0x2 << 1;
    }
    else if (addr < 256 * 4) // ���ĸ���
    {
        write_device_addr |= 0x3 << 1;
        read_device_addr |= 0x3 << 1;
    }
    addr = addr % 256;               // �õ���ַ��Χ
    IIC_SendStart();                 // ��ʼ�ź�
    IIC_SendAddr(write_device_addr); // �����豸��ַ
    IIC_SendOneByte(addr);           // ��Ҫ��ȡ���ݵĵ�ַ
    IIC_SendStart();                 // ��ʼ�ź�
    IIC_SendAddr(read_device_addr);  // �����豸��ַ
    data = IIC_RecvOneByte();        // ��ȡ����
    IIC_SendStop();                  // ֹͣ�ź�
    return data;
}
/*
��������: ��ָ��λ�ö�ȡָ�����ȵ�����
��������:
u16 addr ���ݵ�λ��(0~1023)
u16 len ��ȡ�ĳ���
u8 *buffer ��Ŷ�ȡ������
����ֵ: ����������
*/
void AT24C08_ReadByte(u16 addr, u16 len, u8 *buffer)
{
    u16 i = 0;
    IIC_SendStart();                  // ��ʼ�ź�
    IIC_SendAddr(AT24C08_WRITE_ADDR); // �����豸��ַ
    IIC_SendOneByte(addr);            // ��Ҫ��ȡ���ݵĵ�ַ
    IIC_SendStart();                  // ��ʼ�ź�
    IIC_SendAddr(AT24C08_READ_ADDR);  // �����豸��ַ
    for (i = 0; i < len; i++)
    {
        buffer[i] = IIC_RecvOneByte(); // ��ȡ����
    }
    IIC_SendStop(); // ֹͣ�ź�
}

/*
��������: AT24C08ҳд����
��������:
u16 addr д���λ��(0~1023)
u8 len д��ĳ���(ÿҳ16�ֽ�)
u8 *buffer ��Ŷ�ȡ������
*/
void AT24C08_PageWrite(u16 addr, u16 len, u8 *buffer)
{
    u16 i = 0;
    IIC_SendStart();                  // ��ʼ�ź�
    IIC_SendAddr(AT24C08_WRITE_ADDR); // �����豸��ַ
    IIC_SendOneByte(addr);            // ���ݴ�ŵĵ�ַ
    for (i = 0; i < len; i++)
    {
        IIC_SendOneByte(buffer[i]); // ���ͽ�Ҫ��ŵ�����
    }
    IIC_SendStop(); // ֹͣ�ź�
    delay_ms(10);   // �ȴ�д
}

/*
��������: ��ָ��λ��д��ָ�����ȵ�����
��������:
u16 addr ���ݵ�λ��(0~1023)
u16 len д��ĳ���
u8 *buffer ��ż���д�������
����ֵ: ����������
*/
void AT24C08_WriteByte(u16 addr, u16 len, u8 *buffer)
{
    u8 page_byte = 16 - addr % 16; // �õ���ǰҳʣ����ֽ�����
    if (page_byte > len)           // �жϵ�ǰҳʣ����ֽڿռ��Ƿ�д
    {
        page_byte = len; // ��ʾһ���Կ���д��
    }
    while (1)
    {
        AT24C08_PageWrite(addr, page_byte, buffer); // дһҳ
        if (page_byte == len)
            break;           // д����
        buffer += page_byte; // ָ��ƫ��
        addr += page_byte;   // ��ַƫ��
        len -= page_byte;    // �õ�ʣ��û��д��ĳ���
        if (len > 16)
            page_byte = 16;
        else
            page_byte = len; // һ�ο���д��
    }
}
