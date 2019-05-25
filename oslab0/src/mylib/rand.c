#include "mylib.h"
static uint32_t next = 1;
void srand(uint32_t seed)
{
    next = seed;
}
int rand()
{
    next = next * 1103515245 + 12345;
    int ret = (uint32_t)(next / 65535) % 32768;
    return ret;
}
