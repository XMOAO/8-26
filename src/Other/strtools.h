#ifndef STRTOOLS_H
#define STRTOOLS_H

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef unsigned long uint32_t;

typedef signed char int8_t;
typedef signed int int16_t;
typedef signed long int32_t;

void _itoa(int num, char* str);
void _ftoa(float num, char* str, int precision);
uint8_t* _va(const char* format, ...);

#endif // STRTOOLS_H
