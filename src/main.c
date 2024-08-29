#include "REG52.H"
#include "OLED/LQ12864.h"
#include <stdio.h> 

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef signed char int8_t;
typedef signed int int16_t;

// 矩阵按键
#define KEY_PORT    P3
#define IS_NUMKEY   (iCurKey >= 0 && iCurKey <= 9)

sbit Key_R1 = KEY_PORT ^ 7;
sbit Key_R2 = KEY_PORT ^ 6;
sbit Key_R3 = KEY_PORT ^ 5;
sbit Key_R4 = KEY_PORT ^ 4;

sbit Key_C1 = KEY_PORT ^ 3;
sbit Key_C2 = KEY_PORT ^ 2;
sbit Key_C3 = KEY_PORT ^ 1;
sbit Key_C4 = KEY_PORT ^ 0;

// 密码长度
#define MAX_PASSWORD_DIG            10
#define MAX_LENGTH                  MAX_PASSWORD_DIG + 1

// 初始密码
#define PASSWORD_INIT               "2004040114"
#define PASSWORD_ERROR_THRESHOLD    3
#define PASSWORD_ERROR_WAITTIME     3

// 全局变量
int8_t iCurKey, iCurPointer;
uint8_t iCurMode = 1, iCurStage = 0;
enum
{
    Mode_InputPassword = 1,
    Mode_ResetPassword = (1 << 2),
    Mode_SecureDisplay = (1 << 3)
};

uint8_t iErrorTimes = 0;
bit bInColdDownTime = 0;
char iCurPassword[MAX_LENGTH] = "", iPassword[MAX_LENGTH] = PASSWORD_INIT;

#define LINE_TIPS       0
#define LINE_PASSWORD   3

void Init_OLED(void);
void Init_Password(void);
void Init_Timer0(void);
void MainLoop(void);
void delay_ms(uint16_t n);
int8_t key_scanner();

void main()
{
    Init_OLED();
    Init_Password();
    Init_Timer0();

    while (1)
        MainLoop();
}

void Init_OLED()
{
    OLED_Init();
    OLED_Fill(0x00);
    delay_ms(50);
}

void Init_Password()
{
    // 初始化数组
    memset(iCurPassword, 0, sizeof iCurPassword);
    iCurPassword[MAX_PASSWORD_DIG] = '\0';
}

void Init_Timer0()
{
    TMOD |= 0X01;
	TH0 = (65536 - 50000) / 256;
	TL0 = (65536 - 50000) % 256;
    ET0 = 1;
	TR0 = 1;
	EA  = 1;
}

bit bTimerSignal = 0;
uint8_t iTimerTarget = 0, iTimerCount = 0;
int16_t iTimerCurent = 0;

void Timer0_Handler() interrupt 1
{
    TH0 = (65536 - 50000) / 256;
	TL0 = (65536 - 50000) % 256;

    if(!iTimerTarget)
        return;

    if(++iTimerCount >= 20)
    {
        iTimerCount = 0;

        if(++ iTimerCurent >= iTimerTarget)
        {
            bTimerSignal = 1;
            iTimerTarget = 0;
            iTimerCurent = 0;
        }
    }
}

void RefreshTimer(uint8_t s)
{
    bTimerSignal = 0;
    iTimerTarget = s;
    iTimerCount = 0;
}

void ResetCheckSys()
{
    OLED_ClearRaw(LINE_TIPS, 16);
    OLED_ClearRaw(LINE_TIPS + 2, 16);
    OLED_ClearRaw(LINE_PASSWORD, 8);

    iCurPointer = 0;
    memset(iCurPassword, 0, sizeof iCurPassword);
    iCurPassword[MAX_PASSWORD_DIG] = '\0';
}

void MainLoop()
{
    iCurKey = key_scanner();

    // 明文显示密码
    if(iCurKey == 12)
    {
        if(iCurMode & Mode_SecureDisplay)
            iCurMode &= ~Mode_SecureDisplay;
        else
            iCurMode |= Mode_SecureDisplay;
    }
    // 准备更改密码
    else if(iCurKey == 16)
    {
        
    }

    // 输入密码模式
    if(iCurMode & Mode_InputPassword)
    {
        switch(iCurStage)
        {
            case 0:
            {
                OLED_ShowString16(0, 0, "请输入密码:");

                if(IS_NUMKEY)
                {
                    if(iCurPointer < MAX_LENGTH - 1)
                        iCurPassword[iCurPointer ++] = 0x30 + iCurKey;
                }
                // 回退键
                else if(iCurKey == 10)
                {
                    if(iCurPointer > 0)
                    {
                        if((int8_t)(iCurPointer - 1) >= 0)
                            iCurPointer --;

                        OLED_P8x16Str((8 * (iCurPointer)), LINE_PASSWORD, " ");
                        iCurPassword[iCurPointer] = 0;
                    }
                }
                // 确定键
                else if(iCurKey == 11)
                {
                    if(!strcmp(iCurPassword, iPassword))
                    {
                        iCurStage = 1;
                    }
                    else
                    {
                        OLED_ClearRaw(LINE_TIPS, 16);
                        OLED_ClearRaw(LINE_TIPS + 2, 16);
                        OLED_ClearRaw(LINE_PASSWORD, 8);

                        iErrorTimes ++;
                        if(iErrorTimes >= PASSWORD_ERROR_THRESHOLD)
                        {
                            iErrorTimes = 0;
                            bInColdDownTime = 1;

                            OLED_ShowString16(0, LINE_TIPS, "密码错误次数过多");
                            OLED_ShowString16(0, LINE_TIPS + 2, "请等待");

                            RefreshTimer(5);
                            iCurStage = 2;
                        }
                        else
                        {
                            OLED_ShowString16(0, LINE_TIPS, "密码错误!");
                            OLED_ShowString16(0, LINE_TIPS + 2, "请等待");

                            RefreshTimer(2);
                            iCurStage = 2;
                        }
                        break;
                    }
                }

                if(!(iCurMode & Mode_SecureDisplay))
                {
                    OLED_P8x16Str(0, LINE_PASSWORD, iCurPassword);
                }
                else
                {
                    uint8_t i = 0;
                    static uint8_t szSecureInput[MAX_LENGTH];
                    memset(szSecureInput, '-', sizeof szSecureInput);

                    for(; i < iCurPointer; i++)
                        *(szSecureInput + i) = '*';

                    OLED_P8x16Str(0, LINE_PASSWORD, szSecureInput);
                }

                break;
            }
            case 1:
            {
                ResetCheckSys();
                OLED_ShowString16(0, LINE_TIPS, "密码正确!");

                iCurStage = 3;
                break;
            }
            case 2:
            {
                char szText[2];
                szText[0] = 0x30 + (iTimerTarget - iTimerCurent); szText[1] = 's';
                OLED_P8x16Str(16 * 3, LINE_TIPS + 2, szText);

                if(bTimerSignal)
                {
                    ResetCheckSys();

                    bTimerSignal = 0;
                    iCurStage = 0;
                }
                break;
            }
            default:break;     
        }
    }
    // 修改密码模式
    else if(iCurMode == 1)
    {
        OLED_ShowString16(0, LINE_TIPS, "请输入密码:");
    }
}

int8_t key_scanner()
{
    int8_t iKey = 0;
	uint8_t iRow = 0;
    int8_t iResult = 0;
	
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
            else if(iKey == 13)
                iResult = 0;
            else 
                iResult = iKey;
            
            return iResult;
        }
    }
	return -1;
}

void delay_ms(uint16_t n)
{
	uint16_t i, j;
	for(i = 0; i < n; i++)
		for(j = 0; j < 123; j++);
}