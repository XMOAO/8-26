#include <stdarg.h>
#include <stdio.h>

void _itoa(int num, char* str) {
    int i = 0, sign = num;

    // Handle the special case of zero explicitly
    if (num == 0) {
        str[i++] = '0';
        str[i] = '\0';
        return;
    }

    if (sign < 0) num = -num;

    // Convert number to string in reverse order
    while (num > 0) {
        str[i++] = num % 10 + '0';
        num /= 10;
    }

    if (sign < 0) str[i++] = '-';
    str[i] = '\0';

    // Reverse the string
    int j = 0, k = i - 1;
    while (j < k) {
        char temp = str[j];
        str[j] = str[k];
        str[k] = temp;
        j++;
        k--;
    }
}

void _ftoa(float num, char* str, int precision) {
    if (num < 0) {
        *str++ = '-';
        num = -num;
    }

    int int_part = (int)num;
    float frac_part = num - (float)int_part;

    _itoa(int_part, str);
    while (*str != '\0') str++;

    *str++ = '.';
    int i;
    for (i = 0; i < precision; i++) {
        frac_part *= 10;
        int digit = (int)frac_part;
        *str++ = digit + '0';
        frac_part -= digit;
    }
    *str = '\0';
}

char buffer[128];
uint8_t* _va(const char* format, ...) {
    
    va_list args;
    va_start(args, format);
    char* str;
    int int_temp;
    double double_temp;
    char temp_str[100];
    int precision;

    char* buffer_ptr = buffer; // Use a local pointer to traverse the global buffer

    while (*format) {
        if (*format == '%') {
            format++;
            precision = -1; // Default precision indicator

            // Check for precision specification
            if (*format == '.') {
                format++;
                precision = 0;
                while (*format >= '0' && *format <= '9') {
                    precision = precision * 10 + (*format - '0');
                    format++;
                }
            }

            switch (*format) {
                case 'd':
                    int_temp = va_arg(args, int);
                    _itoa(int_temp, temp_str);
                    // Copy the integer string to the buffer
                    {
                        char *ptr = temp_str;
                        while (*ptr) {
                            *buffer_ptr = *ptr;
                            buffer_ptr++;
                            ptr++;
                        }
                    }
                    break;
                case 'f':
                    double_temp = va_arg(args, double);
                    if (precision == -1) precision = 6; // Default precision if not specified
                    _ftoa((float)double_temp, temp_str, precision);
                    // Copy the float string to the buffer
                    {
                        char *ptr = temp_str;
                        while (*ptr) {
                            *buffer_ptr = *ptr;
                            buffer_ptr++;
                            ptr++;
                        }
                    }
                    break;
                case 's':
                    str = va_arg(args, char*);
                    // Copy the string to the buffer
                    while (*str) {
                        *buffer_ptr = *str;
                        buffer_ptr++;
                        str++;
                    }
                    break;
                default:
                    break;
            }
        } else {
            *buffer_ptr = *format;
            buffer_ptr++;
        }
        format++;
    }

    *buffer_ptr = '\0';  // Null-terminate the string
    va_end(args);

    return (uint8_t*) buffer;
}
