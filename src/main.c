#include "usr_header.h"
#include "OLED/LQ12864.h"
#include "EEPROM/EEPROM.h"

// 蜂鸣器
sbit BUZZER = P1 ^ 7;
sbit SWITCH = P1 ^ 6;

// 矩阵按键
#define KEY_PORT    P2
#define IS_NUMKEY   (iCurKey >= 0 && iCurKey <= 9)

//76543210

sbit Key_R1 = KEY_PORT ^ 4;
sbit Key_R2 = KEY_PORT ^ 5;
sbit Key_R3 = KEY_PORT ^ 6;
sbit Key_R4 = KEY_PORT ^ 7;

sbit Key_C1 = KEY_PORT ^ 3;
sbit Key_C2 = KEY_PORT ^ 2;
sbit Key_C3 = KEY_PORT ^ 1;
sbit Key_C4 = KEY_PORT ^ 0;

// 密码长度
#define MAX_PASSWORD_DIG            10
#define MAX_PASSWORD_LENGTH         MAX_PASSWORD_DIG + 1

// 初始密码
#define PASSWORD_INIT               "123"
#define PASSWORD_ERROR_THRESHOLD    3
#define PASSWORD_ERROR_WAITTIME     3

// 全局变量
enum
{
    Mode_InputPassword = 1,
    Mode_ResetPassword = (1 << 2),
    Mode_SecureDisplay = (1 << 3)
};

int8_t iCurKey, iCurPointer;
uint8_t iCurMode = (Mode_InputPassword | Mode_SecureDisplay), iCurStage = 0;

uint8_t iErrorTimes = 0;
bit bInColdDownTime = 0;
char iCurPassword[MAX_PASSWORD_LENGTH], iPassword[MAX_PASSWORD_LENGTH] = PASSWORD_INIT;
char szSecureInput[MAX_PASSWORD_LENGTH];

#define LINE_TIPS       0
#define LINE_PASSWORD   3

void Init_Device(void);
void Init_OLED(void);
void Init_Password(void);
void Init_Timer0(void);
void MainLoop(void);

int8_t key_scanner();

void main()
{
    Init_Device();
    Init_OLED();
    Init_Password();
    Init_Timer0();

    while (1)
        MainLoop();
}

void Init_Device()
{
    BUZZER = 0;
    SWITCH = 1;
}

void Init_OLED()
{
    OLED_Init();
    OLED_Fill(0x00);
    delay_ms(50);
}

void Init_Password()
{
    int i = 0;

    uint8_t x[8] = "zfy zbl";
    uint8_t x2[8];

    for(; i < 8; i++)
    {
        AT24C02_WriteByte(i, x[i]);
        delay_ms(20);
    }

    for(i = 0; i < 8; i++)
    {
        *(x2 + i) = AT24C02_ReadByte(i);
        delay_ms(10);
    }

    // 初始化数组
    memset(iCurPassword, 0, sizeof iCurPassword);
    iCurPassword[MAX_PASSWORD_DIG] = '\0';

    memset(szSecureInput, '-', sizeof szSecureInput);
    szSecureInput[MAX_PASSWORD_DIG] = '\0';


    OLED_P8x16Str(0, 6, x2);
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

        if( ++ iTimerCurent >= iTimerTarget)
        {
            bTimerSignal = 1;
            iTimerTarget = 0;
            iTimerCurent = 0;
        }
    }
}

void RefreshTimer(uint8_t s)
{
    TH0 = (65536 - 50000) / 256;
	TL0 = (65536 - 50000) % 256;

    bTimerSignal = 0;
    iTimerTarget = s;
    iTimerCount = 0;
}

void ResetCheckSys(bit clearstring)
{
    if(clearstring)
    {
        OLED_ClearRaw(LINE_TIPS, 16);
        OLED_ClearRaw(LINE_TIPS + 2, 16);
        OLED_ClearRaw(LINE_PASSWORD, 8);
    }

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
        ResetCheckSys(1);

        if(iCurMode & Mode_ResetPassword)
            iCurMode &= ~Mode_ResetPassword;
        else
            iCurMode |= Mode_ResetPassword;
    }

    // 输入密码模式
    if(iCurMode & Mode_InputPassword)
    {
        switch(iCurStage)
        {
            case 0: case 4:
            {
                if(!(iCurMode & Mode_ResetPassword))
                    OLED_ShowString16(0, 0, "请输入密码:");
                else
                {
                    if(!iCurStage)
                        OLED_ShowString16(0, 0, "重置 输入原密码:");
                    else
                        OLED_ShowString16(0, LINE_TIPS, "重置 输入新密码:");
                }
                    
                if(IS_NUMKEY)
                {
                    if(iCurPointer < MAX_PASSWORD_DIG)
                        iCurPassword[iCurPointer ++] = 0x30 + iCurKey;
                }
                // 回退键
                else if(iCurKey == 10)
                {
                    if(iCurPointer > 0)
                    {
                        if((int8_t)(iCurPointer - 1) >= 0)
                            iCurPointer --;

                        iCurPassword[iCurPointer] = 0;    
                        OLED_P8x16Str((8 * (iCurPointer)), LINE_PASSWORD, " ");
                    }
                }
                // 确定键
                else if(iCurKey == 11)
                {
                    bit bFailed = 0;

                    if((iCurMode & Mode_ResetPassword))
                    {
                        if(!iCurStage)
                        {
                            // 检测输入的密码与原密码是否一致
                            if(!strcmp(iCurPassword, iPassword))
                                iCurStage = 4;
                            else
                                bFailed = 1;
                            
                            OLED_ClearRaw(LINE_PASSWORD, 8);

                            iCurPointer = 0;
                            memset(iCurPassword, 0, sizeof iCurPassword);
                            iCurPassword[MAX_PASSWORD_DIG] = '\0';
                        }
                        else
                        {
                            // 输入的新密码
                            iCurStage = 5;

                            strncpy(iPassword, iCurPassword, sizeof iPassword - 1);
                            iPassword[10] = '\0';
                            ResetCheckSys(1);
                        }
                    }
                    else
                    {
                        if(!strcmp(iCurPassword, iPassword))
                            iCurStage = 1;
                        else
                            bFailed = 1;
                    }

                    if(bFailed)
                    {
                        BUZZER = 1;
                        
                        OLED_ClearRaw(LINE_TIPS, 16);
                        OLED_ClearRaw(LINE_TIPS + 2, 16);
                        OLED_ClearRaw(LINE_PASSWORD, 8);

                        BUZZER = 0;

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
                    int i;
                    strncpy(szSecureInput, iCurPassword, iCurPointer);
                    for(i = iCurPointer; i < sizeof szSecureInput; i++)
                        *(szSecureInput + i) = '-';
                    
                    szSecureInput[10] = '\0';

                    OLED_P8x16Str(0, LINE_PASSWORD, szSecureInput);
                }
                else
                {
                    int i;
                    memset(szSecureInput, '-', sizeof szSecureInput - 1);
                    for(i = 0; i < iCurPointer; i++)
                        *(szSecureInput + i) = '*';
                    
                    szSecureInput[10] = '\0';

                    OLED_P8x16Str(0, LINE_PASSWORD, szSecureInput);
                }

                break;
            }
            case 1:
            {
                ResetCheckSys(1);

                OLED_ShowString16(0, LINE_TIPS, "密码正确!");
                SWITCH = ~SWITCH;
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
                    ResetCheckSys(1);

                    bTimerSignal = 0;
                    iCurStage = 0;
                }
                break;
            }
            case 5:
            {
                OLED_ClearRaw(LINE_PASSWORD, 8);
                OLED_ShowString16(0, LINE_TIPS, "密码重置成功!");
                OLED_ShowString16(0, LINE_TIPS + 2, "请等待");

                RefreshTimer(2);
                iCurStage = 2;

                break;
            }
            default:break;     
        }
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