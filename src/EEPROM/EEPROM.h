#ifndef EEPROM_H
#define EEPROM_H

#include "../usr_header.h"

// AT24C02 数据存储空间分配（共256 byte）
enum
{
	AT24C02_MEMORY_PASSWORD = 0,
	AT24C02_MEMORY_QUES1 = 16,
	AT24C02_MEMORY_QUES2 = 33,
};

void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t Byte);
uint8_t I2C_ReceiveByte(void);
void I2C_SendAck(uint8_t AckBit);
uint8_t I2C_ReceiveAck(void);

void AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t AT24C02_ReadByte(uint8_t WordAddress);
void AT24C02_WriteString(uint8_t StartAddr, uint8_t* Data);
uint8_t* AT24C02_ReadString(uint8_t StartAddr);


#endif