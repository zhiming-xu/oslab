#include "mylib.h"
int atoi(const char *s)
{
    int len = strlen(s);
    int base = 1, ret = 0;
    for(int i = len - 1; i >= 0; --i)
    {
        ret += base * (s[i] - '0');
        base *= 10;
    }
    return ret;
}
