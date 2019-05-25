#include "mylib.h"
char* strstr(const char* s1, const char* s2)
{
    int len = min(strlen(s1), strlen(s2));
    for(int i = 0; i < len; ++i)
        if(s1[i] != s2[i])
            return NULL;
    return (char*)s1;
}
