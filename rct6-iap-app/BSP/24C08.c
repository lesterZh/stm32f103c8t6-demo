/*****************************************************************************
  文件: 24C08.c
  作者: Zhengyu https://gzwelink.taobao.com
  版本: V1.0.0
  时间: 20200401
    平台:MINI-STM32F103C8T6

*******************************************************************************/
#include "sys.h"
#include "delay.h"

#define AT24C08_READ_ADDR 0xA1  // 读数据时的地址
#define AT24C08_WRITE_ADDR 0xA0 // 写数据时的地址

// IIC 初始化PB10,PB11
void GPIO_Config_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    // 使能端口时钟使能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; // 端口配置
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;          // AF功能
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;        // IO口速度为50MHz
    GPIO_Init(GPIOB, &GPIO_InitStructure);                   // 根据设定参数初始化
}

// IIC2初始化
void IIC_Init(void)
{
    I2C_InitTypeDef I2C_InitStructure;
    GPIO_Config_Init();
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2 | RCC_APB2Periph_AFIO, ENABLE);

    I2C_DeInit(I2C2); // 使用I2C2
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = 0x30; // 主机的I2C地址,随便写的
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    I2C_InitStructure.I2C_ClockSpeed = 100000; // 100K
    I2C_Init(I2C2, &I2C_InitStructure);
    I2C_Cmd(I2C2, ENABLE);
}
/*
函数功能: 发送起始信号
当时钟线为高电平的时候，数据线由高电平变为低电平的过程
*/
void IIC_SendStart(void)
{
    I2C_GenerateSTART(I2C2, ENABLE);
    while (!I2C_CheckEvent(I2C2, I2C_EVENT_MASTER_MODE_SELECT))
    {
        //可以加入超时退出机制
    }
}
/*
函数功能: 停止信号
当时钟线为高电平的时候，数据线由低电平变为高电平的过程
*/
void IIC_SendStop(void)
{
    I2C_GenerateSTOP(I2C2, ENABLE);
    delay_ms(5);
}

void IIC_SendAddr(u8 addr)
{
    I2C_SendData(I2C2, addr);                               // 发送数据(地址)
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_ADDR & 0xFFFFFF)) // 等待地址发送完成
    {
        // 可以加入超时退出机制
    }
}
/*
函数功能: 发送数据
*/
void IIC_SendOneByte(u8 addr)
{
    I2C_SendData(I2C2, addr);                              // 发送数据
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_BTF & 0xFFFFFF)) // 等待发送完成
    {
        // 可以加入超时退出机制
    }
}

/*
函数功能: 接收一个字节数据
*/
u8 IIC_RecvOneByte(void)
{
    u8 data = 0;
    I2C_AcknowledgeConfig(I2C2, ENABLE);                    // 使能应答
    while (!I2C_CheckEvent(I2C2, I2C_FLAG_RXNE & 0xFFFFFF)) // 等待数据
    {
        // 可以加入超时退出机制
    }
    data = I2C_ReceiveData(I2C2);         // 接收数据
    I2C_AcknowledgeConfig(I2C2, DISABLE); // 关闭应答使能
    return data;
}

/*
函数功能: 写一个字节
函数参数:
u8 addr 数据的位置(0~1023)
u8 data 数据范围(0~255)
*/
void AT24C08_WriteOneByte(u16 addr, u8 data)
{
    u8 read_device_addr = AT24C08_READ_ADDR;
    u8 write_device_addr = AT24C08_WRITE_ADDR;
    if (addr < 256 * 1) // 第一个块
    {
        write_device_addr |= 0x0 << 1;
        read_device_addr |= 0x0 << 1;
    }
    else if (addr < 256 * 2) // 第二个块
    {
        write_device_addr |= 0x1 << 1;
        read_device_addr |= 0x1 << 1;
    }
    else if (addr < 256 * 3) // 第三个块
    {
        write_device_addr |= 0x2 << 1;
        read_device_addr |= 0x2 << 1;
    }
    else if (addr < 256 * 4) // 第四个块
    {
        write_device_addr |= 0x3 << 1;
        read_device_addr |= 0x3 << 1;
    }
    addr = addr % 256;               // 得到地址范围
    IIC_SendStart();                 // 起始信号
    IIC_SendAddr(write_device_addr); // 发送设备地址
    IIC_SendOneByte(addr);           // 数据存放的地址
    IIC_SendOneByte(data);           // 发送将要存放的数据
    IIC_SendStop();                  // 停止信号
    delay_ms(10);                    // 等待写
}

/*
函数功能: 读一个字节
函数参数:
u8 addr 数据的位置(0~1023)
返回值: 读到的数据
*/
u8 AT24C08_ReadOneByte(u16 addr)
{
    u8 data = 0;
    u8 read_device_addr = AT24C08_READ_ADDR;
    u8 write_device_addr = AT24C08_WRITE_ADDR;
    if (addr < 256 * 1) // 第一个块
    {
        write_device_addr |= 0x0 << 1;
        read_device_addr |= 0x0 << 1;
    }
    else if (addr < 256 * 2) // 第二个块
    {
        write_device_addr |= 0x1 << 1;
        read_device_addr |= 0x1 << 1;
    }
    else if (addr < 256 * 3) // 第三个块
    {
        write_device_addr |= 0x2 << 1;
        read_device_addr |= 0x2 << 1;
    }
    else if (addr < 256 * 4) // 第四个块
    {
        write_device_addr |= 0x3 << 1;
        read_device_addr |= 0x3 << 1;
    }
    addr = addr % 256;               // 得到地址范围
    IIC_SendStart();                 // 起始信号
    IIC_SendAddr(write_device_addr); // 发送设备地址
    IIC_SendOneByte(addr);           // 将要读取数据的地址
    IIC_SendStart();                 // 起始信号
    IIC_SendAddr(read_device_addr);  // 发送设备地址
    data = IIC_RecvOneByte();        // 读取数据
    IIC_SendStop();                  // 停止信号
    return data;
}
/*
函数功能: 从指定位置读取指定长度的数据
函数参数:
u16 addr 数据的位置(0~1023)
u16 len 读取的长度
u8 *buffer 存放读取的数据
返回值: 读到的数据
*/
void AT24C08_ReadByte(u16 addr, u16 len, u8 *buffer)
{
    u16 i = 0;
    IIC_SendStart();                  // 起始信号
    IIC_SendAddr(AT24C08_WRITE_ADDR); // 发送设备地址
    IIC_SendOneByte(addr);            // 将要读取数据的地址
    IIC_SendStart();                  // 起始信号
    IIC_SendAddr(AT24C08_READ_ADDR);  // 发送设备地址
    for (i = 0; i < len; i++)
    {
        buffer[i] = IIC_RecvOneByte(); // 读取数据
    }
    IIC_SendStop(); // 停止信号
}

/*
函数功能: AT24C08页写函数
函数参数:
u16 addr 写入的位置(0~1023)
u8 len 写入的长度(每页16字节)
u8 *buffer 存放读取的数据
*/
void AT24C08_PageWrite(u16 addr, u16 len, u8 *buffer)
{
    u16 i = 0;
    IIC_SendStart();                  // 起始信号
    IIC_SendAddr(AT24C08_WRITE_ADDR); // 发送设备地址
    IIC_SendOneByte(addr);            // 数据存放的地址
    for (i = 0; i < len; i++)
    {
        IIC_SendOneByte(buffer[i]); // 发送将要存放的数据
    }
    IIC_SendStop(); // 停止信号
    delay_ms(10);   // 等待写
}

/*
函数功能: 从指定位置写入指定长度的数据
函数参数:
u16 addr 数据的位置(0~1023)
u16 len 写入的长度
u8 *buffer 存放即将写入的数据
返回值: 读到的数据
*/
void AT24C08_WriteByte(u16 addr, u16 len, u8 *buffer)
{
    u8 page_byte = 16 - addr % 16; // 得到当前页剩余的字节数量
    if (page_byte > len)           // 判断当前页剩余的字节空间是否够写
    {
        page_byte = len; // 表示一次性可以写完
    }
    while (1)
    {
        AT24C08_PageWrite(addr, page_byte, buffer); // 写一页
        if (page_byte == len)
            break;           // 写完了
        buffer += page_byte; // 指针偏移
        addr += page_byte;   // 地址偏移
        len -= page_byte;    // 得到剩余没有写完的长度
        if (len > 16)
            page_byte = 16;
        else
            page_byte = len; // 一次可以写完
    }
}
