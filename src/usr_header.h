#ifndef USR_HEADER_H
#define USR_HEADER_H

typedef unsigned char uint8_t;
typedef unsigned int uint16_t;
typedef signed char int8_t;
typedef signed int int16_t;

#include "REG52.H"
#include <stdio.h> 
#include <string.h> 

void delay_ms(uint16_t n);
#endif