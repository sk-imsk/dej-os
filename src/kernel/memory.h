#pragma once
#include "stdint.h"
#include "stddef.h"

void* memcpy(void* dst, const void* src, uint16_t num);
void* memset(void* ptr, int value, uint16_t num);
int memcmp(const void* ptr1, const void* ptr2, uint16_t num);
void *memmove(void *dest, const void *src, size_t n);
