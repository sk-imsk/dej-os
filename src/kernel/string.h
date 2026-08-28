#pragma once
#include "stdint.h"
#include "stddef.h"

void* memcpy(void* dst, const void* src, size_t num);
void* memset(void* ptr, int value, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
void *memmove(void *dest, const void *src, size_t n);
char * stpcpy (char *restrict dst, const char *restrict src);
char * strchr (const char *s, int c);
char * strrchr(const char *s, int c);
char * strcpy (char *restrict dst, const char *restrict src);
char * strncpy(char *restrict dst, const char *restrict src, size_t size);
int    strcmp (const char * s1, const char * s2);
int    strncmp(const char * s1, const char * s2, size_t size);
int    strcoll(const char * s1, const char * s2);
char * strcat (char *restrict dst, const char *restrict src);
size_t strlen (const char *restrict src);
size_t strnlen(const char *restrict src, size_t size);
