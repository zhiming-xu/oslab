#include "mylib.h"
void itoa(int d, char* dest)
{
    int n = d, tmp[33], i = 0;
    if(n == 0)
    {
        *dest = '0';
        return;
    }
    while( n >= 10)
    {
        tmp[i] = n % 10;
        n /= 10;
        ++i;
    }
    if(n > 0)
    {
        tmp[i] = n;
        ++i;
    }
    while(i > 0)
    {
        --i;
        *dest = (char)(tmp[i] + (int)'0');
        dest++;
    }
    *dest = '\0';
}
