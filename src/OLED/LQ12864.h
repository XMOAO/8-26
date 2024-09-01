#ifndef OLED_H
#define OLED_H

#include "../usr_header.h"

void OLED_CLS(void);
void OLED_Init(void);
void OLED_WrDat(uint8_t IIC_Data);
void OLED_WrCmd(uint8_t IIC_Command);
void OLED_Set_Pos(uint8_t x, uint8_t y);
void OLED_Fill(uint8_t bmp_dat);
void OLED_P8x16Str(uint8_t x, uint8_t y,uint8_t ch[]);
void OLED_P16x16Ch(uint8_t x, uint8_t y, uint8_t N);
void OLED_ShowString16(uint8_t x, uint8_t y, const char* str);
void OLED_ClearRaw(uint8_t y, uint8_t size);

#endif