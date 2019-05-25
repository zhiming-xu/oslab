#include "mylib.h"
void* memset(void *arr, int val, size_t n)
{
    for(int i = 0; i < n; ++i)
        ((char*)arr)[i] = val;
    return arr;
}
