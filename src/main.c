#include "REG52.H"
#include "OLED/LQ12864.h"
#include <stdlib.h> 
#include <stdio.h> 
typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

typedef signed char int8_t;
typedef signed int int16_t;
typedef signed long int32_t;


#define KEY_PORT    P3

sbit Key_R1 = KEY_PORT ^ 7;
sbit Key_R2 = KEY_PORT ^ 6;
sbit Key_R3 = KEY_PORT ^ 5;
sbit Key_R4 = KEY_PORT ^ 4;

sbit Key_C1 = KEY_PORT ^ 3;
sbit Key_C2 = KEY_PORT ^ 2;
sbit Key_C3 = KEY_PORT ^ 1;
sbit Key_C4 = KEY_PORT ^ 0;

#define MAX_LENGTH  11


uint8_t iCurKey;
uint8_t iCurMode = 0, iCurStage = 0;
bit bNeedToCls = 0;

char iCurPassword[MAX_LENGTH] = "", iPassword[MAX_LENGTH] = "12356";
uint8_t iCurPointer;

#define IS_NUMKEY   (iCurKey <= 9)

uint8_t key_scanner();
void delay_ms(uint16_t n);

void main()
{
    OLED_Init();
    OLED_Fill(0x00);
    delay_ms(50);

    // 初始化数组
    memset(iCurPassword, 0, sizeof iCurPassword);
    iCurPassword[10] = '\0';
    
    while (1)
    {
        iCurKey = key_scanner();

        switch(iCurMode)
        {   
            // 输入密码模式：
            case 0:
            {
                OLED_ShowString16(0, 0, "请输入密码:");

                if(IS_NUMKEY && iCurKey)
                {
                    if(iCurPointer < MAX_LENGTH - 1)
                        iCurPassword[iCurPointer ++] = 0x30 + iCurKey;

                    if(!strcmp(iCurPassword, iPassword))
                    {
                        iCurStage = 1;
                    }

                    if(!iCurStage)
                    {
                        OLED_P8x16Str(0, 3, iCurPassword);
                    }
                    else if(iCurStage == 1)
                    {
                        OLED_ClearRaw(0, 16);
                        OLED_ClearRaw(3, 8);

                        OLED_ShowString16(0, 0, "密码正确");
                        memset(iCurPassword, 0, sizeof iCurPassword);
                        iCurPassword[10] = '\0';

                        iCurStage = 2;
                    }    
                }
                break;
            }
            default:break;
        }
    }
}

uint8_t key_scanner()
{
    uint8_t iKey = 0;
	uint8_t iRow = 0;
    uint8_t iResult = 0;
	
	KEY_PORT = 0xFF; 
    
    for (; iRow < 4; iRow++) 
	{
        switch (iRow) 
		{
            case 0: Key_R1 = 0; Key_R2 = 1; Key_R3 = 1; Key_R4 = 1; break;
            case 1: Key_R1 = 1; Key_R2 = 0; Key_R3 = 1; Key_R4 = 1; break;
            case 2: Key_R1 = 1; Key_R2 = 1; Key_R3 = 0; Key_R4 = 1; break;
            case 3: Key_R1 = 1; Key_R2 = 1; Key_R3 = 1; Key_R4 = 0; break;
        }
        
        if (Key_C1 == 0) iKey = iRow * 4 + 1;
        if (Key_C2 == 0) iKey = iRow * 4 + 2;
        if (Key_C3 == 0) iKey = iRow * 4 + 3;
        if (Key_C4 == 0) iKey = iRow * 4 + 4;
        
        if (iKey != 0) break;
	}
    
    if (iKey != 0) 
	{
		delay_ms(10);
		if (Key_C1 == 0 || Key_C2 == 0 || Key_C3 == 0 || Key_C4 == 0) 
		{
			while(Key_C1 == 0 || Key_C2 == 0 || Key_C3 == 0 || Key_C4 == 0)
			{

			}
            
            if(iKey > 4 && iKey < 8)
                iResult = iKey - 1;
            else if(iKey > 8 && iKey < 12)
                iResult = iKey - 2;
            else if(iKey == 4) 
                iResult = 10;
            else if(iKey == 8)
                iResult = 11;
            else if(iKey == 12)
                iResult = 12;
            else 
                iResult = iKey;
            
            return iResult;
        }
    }
	return 0;
}

void delay_ms(uint16_t n)
{
	uint16_t i, j;
	for(i = 0; i < n; i++)
		for(j = 0; j < 123; j++);
}