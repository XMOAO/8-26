#include "EEPROM.h"

// 基址
#define AT24C02_ADDRESS		0xA0

// 串行时钟 串行数据
sbit SCL = P1 ^ 1;
sbit SDA = P1 ^ 0;

// 以下内容参考 : https://blog.csdn.net/2302_80796399/article/details/136091365
void I2C_Delay() 
{
    uint8_t i;
    for (i = 0; i < 10; i++);
}

void I2C_Start(void)
{
	SDA = 1;
	SCL = 1;
    I2C_Delay();
	SDA = 0;
    I2C_Delay();
	SCL = 0;
}

void I2C_Stop(void)
{
	SDA = 0;
	SCL = 1;
    I2C_Delay();
	SDA = 1;
    I2C_Delay();
}

void I2C_SendByte(uint8_t Byte)
{
	uint8_t i;
	for(i = 0; i < 8; i++)
	{
		SDA = Byte & (0x80 >> i);
		SCL = 1;
        I2C_Delay();
		SCL = 0;
        I2C_Delay();
	}
}

uint8_t I2C_ReceiveByte(void)
{
	uint8_t i, Byte = 0x00;
	SDA = 1;
	for(i = 0; i < 8; i++)
	{
		SCL = 1;
        I2C_Delay();
		if(SDA)
            Byte |= (0x80 >> i);
		SCL = 0;
        I2C_Delay();
	}
	return Byte;
}

/**
  * @brief  I2C发送应答
  * @param  AckBit 应答位，0为应答，1为非应答
  * @retval 无
  */
void I2C_SendAck(uint8_t AckBit)
{
	SDA = AckBit;
	SCL = 1;
	SCL = 0;
}

/**
  * @brief  I2C接收应答位
  * @param  无
  * @retval 接收到的应答位，0为应答，1为非应答
  */
uint8_t I2C_ReceiveAck(void)
{
	uint8_t AckBit;
	SDA = 1;
	SCL = 1;
	AckBit = SDA;
	SCL = 0;
	return AckBit;
}

/**
  * @brief  AT24C02写入一个字节
  * @param  WordAddress 要写入字节的地址
  * @param  Data 要写入的数据
  * @retval 无
  */
void AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data)
{
	I2C_Start();
	I2C_SendByte(AT24C02_ADDRESS);
	I2C_ReceiveAck();
	I2C_SendByte(WordAddress);
	I2C_ReceiveAck();
	I2C_SendByte(Data);
	I2C_ReceiveAck();
	I2C_Stop();
}

/**
  * @brief  AT24C02读取一个字节
  * @param  WordAddress 要读出字节的地址
  * @retval 读出的数据
  */
uint8_t AT24C02_ReadByte(uint8_t WordAddress)
{
	uint8_t Data;
	I2C_Start();
	I2C_SendByte(AT24C02_ADDRESS);
	I2C_ReceiveAck();
	I2C_SendByte(WordAddress);
	I2C_ReceiveAck();
	I2C_Start();
	I2C_SendByte(AT24C02_ADDRESS | 0x01);
	I2C_ReceiveAck();
	Data = I2C_ReceiveByte();
	I2C_SendAck(1);
	I2C_Stop();
	return Data;
}

/**
  * @brief  AT24C02写入字符串
  * @param  Data 写入内容
  */
void AT24C02_WriteString(uint8_t StartAddr, uint8_t* Data)
{
	uint8_t i;

	// 标志位
	AT24C02_WriteByte(StartAddr, '&');

	for(i = 0; i < 16; i++)
	{
		delay_ms(20);
		AT24C02_WriteByte(StartAddr + i + 1, Data[i]);
	}
}

/**
  * @brief  AT24C02读取字符串
  * @retval 读出的数据
  */
uint8_t* AT24C02_ReadString(uint8_t StartAddr)
{
	uint8_t i = 0;
	static uint8_t Data[16];

	for(; i < sizeof Data; i++)
	{
		Data[i] = AT24C02_ReadByte(StartAddr + i);
		delay_ms(10);

		if(Data[0] != '&')
			return (void*) 0;
	}

	return Data;
}