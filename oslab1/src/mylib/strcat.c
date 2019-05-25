#include "mylib.h"
char *strcat(char* dest, char* src)
{
    char *tmp;
    tmp = dest;
    while(*tmp)
    {   
        tmp++;
    }
    strcpy(tmp, src);
    return dest;
}
