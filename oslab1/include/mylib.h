#include "am.h"
#include "arch.h"
#include <stdarg.h>
#define min(x,y) x>y?y:x
//stdio.h
int printf(const char* fmt, ...);
//string.h
void* memcpy(void* dest, const void* src, size_t n);
void* memset(void* arr, int val, size_t n);
size_t strlen(const char* s);
size_t strlen(const char* s);
char *strcat(char* dest, char* src);
char *strcut(char* s, int n);
int strcmp(const char* s1, const char* s2);
char *strcpy(char *dest, const char *src);
char *strstr(const char* s1, const char* s2);
void itoa(int d, char *s);
int atoi(const char *s);
//stdlib.h
void srand(uint32_t seed);
int rand();
