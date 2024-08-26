#include "REG52.H"

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

typedef signed char int8_t;
typedef signed int int16_t;
typedef signed long int32_t;

// Usr
#define KEY_PORT P3

sbit Key_R1 = KEY_PORT ^ 7;
sbit Key_R2 = KEY_PORT ^ 6;
sbit Key_R3 = KEY_PORT ^ 5;
sbit Key_R4 = KEY_PORT ^ 4;

sbit Key_C1 = KEY_PORT ^ 3;
sbit Key_C2 = KEY_PORT ^ 2;
sbit Key_C3 = KEY_PORT ^ 1;
sbit Key_C4 = KEY_PORT ^ 0;

uint8_t code g_iSignaturesCC[] = {0x3f,0x06,0x5b,0x4f,0x66,0x6d,0x7d,0x07,0x7f,0x6f,0x40};
uint8_t iCurKey = 0;

// LED pin define
sbit LED = P1 ^ 1;

void delay_ms(unsigned int n)
{
	unsigned int i,j;
	for(i=0;i<n;i++)
		for(j=0;j<123;j++);
}

void display_number(unsigned int h,unsigned int s,unsigned int m)
{
		unsigned char sig_code[8];
		unsigned int i;
	
		sig_code[0] = h / 10;
		sig_code[1] = h % 10;
		sig_code[2] = 10;
		sig_code[3] = m / 10;
		sig_code[4] = m % 10;
		sig_code[5] = 10;
		sig_code[6] = s / 10;
		sig_code[7] = s % 10;
		
		for(i = 0;i < 8;i++)
		{
			P0 = g_iSignaturesCC[sig_code[i]];
			P2 = (P2 | (i << 2));
			delay_ms(1);
			P0 = P2 = 0;
		}
}


uint8_t key_scanner()
{
    uint8_t iKey = 0;
	uint8_t iCol, iRow;
	
	KEY_PORT = 0xFF; 
    
    for (iRow = 0; iRow < 4; iRow++) 
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
				display_number(0, iKey, 0);
			}
            return iKey;
        }
    }
	return 0;
}

void main()
{
    // set T0 1ms
    TMOD = 0x01;
    TH0 = 0xFC;
    TL0 = 0x18;

    // enable interrupt
    EA = 1;
    ET0 = 1;

    // led pin init
    LED = 1;

    // launch T0
    TR0 = 1;

    while (1)
    {
        iCurKey = key_scanner();

        if(iCurKey)
        {

        }
        else
        {
            // Display nums
            display_number(0, iCurKey, 0);
        }
    }
}
