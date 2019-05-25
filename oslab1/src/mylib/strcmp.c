#include "mylib.h"
int strcmp(const char* s1, const char* s2)
{
    if(strlen(s1) != strlen(s2))
        return -1;
    int i = 0;
    while(s1[i] && s2[i])
    {
        if(s1[i] != s2[i])
            return -1;
        i++;
    }
    return 0;
}
