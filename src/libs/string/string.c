#include "string.h"
#include "../print/print.h"

void str_copy(char *dest, const char *src)
{
    int i = 0;
    while (src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

void str_append(char *dest, const char *src)
{
    int i = 0;
    while (dest[i] != '\0')
    {
        i++;
    }

    int j = 0;
    while (src[j] != '\0')
    {
        dest[i] = src[j];
        i++;
        j++;
    }
    dest[i] = '\0';
}

void str_append_uint(char *dest, u32 num)
{
    char buf[20];
    int i = 0;

    if (num == 0)
    {
        buf[i++] = '0';
    }
    else
    {
        u32 temp = num;
        int digits = 0;
        while (temp > 0)
        {
            temp /= 10;
            digits++;
        }

        i = digits;
        while (num > 0)
        {
            buf[--i] = '0' + (num % 10);
            num /= 10;
        }
        i = digits;
    }

    buf[i] = '\0';
    str_append(dest, buf);
}

int str_len(const char *str)
{
    int len = 0;
    while (str[len] != '\0')
    {
        len++;
    }
    return len;
}

void print_str(const char *str, u32 color)
{
    string(str, color);
    putchar('\n', color);
}
