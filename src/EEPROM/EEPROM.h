#ifndef EEPROM_H
#define EEPROM_H

#include "../usr_header.h"

void I2C_Start(void);
void I2C_Stop(void);
void I2C_SendByte(uint8_t Byte);
uint8_t I2C_ReceiveByte(void);
void I2C_SendAck(uint8_t AckBit);
uint8_t I2C_ReceiveAck(void);

void AT24C02_WriteByte(uint8_t WordAddress, uint8_t Data);
uint8_t AT24C02_ReadByte(uint8_t WordAddress);

void AT24C02_WriteString(uint8_t* Data);
uint8_t* AT24C02_ReadString();


#endif