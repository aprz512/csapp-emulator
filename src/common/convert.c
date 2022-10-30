#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "headers/common.h"

uint64_t string2uint(const char *src)
{
    return string2uint_range(src, 0, -1);
}

/**
 * @brief
 *
 * @param src
 * @param start index inclusive
 * @param end index inclusive
 * @return uint64_t
 */
uint64_t string2uint_range(const char *src, int start, int end)
{
    // 设置一个默认规则，如果是 -1 的话，取整个字符串的长度
    end = end == -1 ? strlen(src) : end;

    int state = 0;
    char current = 0;
    int num = 0;
    int error = 0;
    uint64_t result = 0;
    uint64_t pre_val = 0;
    int sign_bit = 0;
    for (size_t i = 0; i < end; i++)
    {
        current = src[i];

        if (state == 0)
        {
            if (current == ' ')
            {
                continue;
            }
            else if (current == '0')
            {
                state = 1;
            }
            else if (current == '-')
            {
                state = 2;
                sign_bit = 1;
            }
            else if (current >= '1' && current <= '9')
            {
                state = 3;
                num = current - '0';
                result = result * 10 + num;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 1)
        {
            if (current == 'x')
            {
                state = 4;
            }
            else if (current >= '0' && current <= '9')
            {
                state = 3;
                num = current - '0';
                result = result * 10 + num;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 2)
        {
            if (current == '0')
            {
                state = 1;
            }
            else if (current >= '1' && current <= '9')
            {
                state = 3;
                num = current - '0';
                result = result * 10 + num;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 3)
        {
            if (current >= '0' && current <= '9')
            {
                state = 3;
                num = current - '0';
                pre_val = result;
                result = result * 10 + num;
                if (pre_val > result)
                {
                    error = 2;
                    break;
                }
            }
            else if (current == ' ')
            {
                state = 6;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 4)
        {
            if (current >= '0' && current <= '9')
            {
                state = 5;
                num = current - '0';
                printf("debug num = %d\n", num);
                result = result * 16 + num;
                printf("debug result = %d\n", result);
            }
            else if (current >= 'a' && current <= 'f')
            {
                state = 5;
                num = current - 'a' + 10;
                result = result * 16 + num;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 5)
        {
            if (current >= '0' && current <= '9')
            {
                state = 5;
                num = current - '0';
                printf("debug num = %d\n", num);
                pre_val = result;
                result = result * 16 + num;
                printf("debug result = %d\n", result);
                if (pre_val > result)
                {
                    error = 2;
                    break;
                }
            }
            else if (current >= 'a' && current <= 'f')
            {
                state = 5;
                num = current - 'a' + 10;
                pre_val = result;
                result = result * 16 + num;
                if (pre_val > result)
                {
                    error = 2;
                    break;
                }
            }
            else if (current == ' ')
            {
                state = 6;
            }
            else
            {
                error = 1;
                break;
            }
        }
        else if (state == 6)
        {
            if (current == ' ')
            {
                state = 6;
            }
            else
            {
                error = 1;
                break;
            }
        }
    }

    if (error == 1)
    {
        printf("cannot convert to integer, illegal string format: %s\n", src);
        exit(error);
    }
    else if (error == 2)
    {
        printf("(uint64_t)%s overflow: cannot convert\n", src);
        exit(error);
    }

    if (sign_bit)
    {
        uint64_t one = 1;
        if ((result & (one << 63)) != 0 )  {
            printf("(uint64_t)%s overflow: cannot convert\n", src);
            exit(2);
        } else {
            result = (int64_t) result * (-1);
            return *((uint64_t *)&result);
        }
    }

    return result;
}