#include "mylib.h"
char *strcpy(char* dest, const char* src)
{
    return (char *)memcpy(dest, src, strlen(src));
}
