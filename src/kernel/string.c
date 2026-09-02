#include "string.h"

void* memcpy(void* dst, const void* src, size_t num)
{
    uint8_t* u8Dst = (uint8_t *)dst;
    const uint8_t* u8Src = (const uint8_t *)src;

    for (size_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void * memset(void * ptr, int value, size_t num)
{
    uint8_t* u8Ptr = (uint8_t *)ptr;

    for (size_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    const uint8_t* u8Ptr1 = (const uint8_t *)ptr1;
    const uint8_t* u8Ptr2 = (const uint8_t *)ptr2;

    for (size_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}

void *memmove(void *dest, const void *src, size_t n) {
    uint8_t *pdest = dest;
    const uint8_t *psrc = src;

    if ((uintptr_t)src > (uintptr_t)dest) {
        for (size_t i = 0; i < n; i++) {
            pdest[i] = psrc[i];
        }
    } else if ((uintptr_t)src < (uintptr_t)dest) {
        for (size_t i = n; i > 0; i--) {
            pdest[i-1] = psrc[i-1];
        }
    }

    return dest;
}


char * stpcpy(char *restrict dst, const char *restrict src){

	while ((*dst++ = *src++)){
	//nothing
	}
	return dst;
}

char * strchr(const char *s, int c){
	for (;; ++s){
		if (*s == (char)c){

		return ((char *)s);
	}
	if (!*s){
	return ((char *)NULL);
	}
}// for (;;, ++s)

}

char * strrchr(const char *p, int c){
	char *save;

	for (save = NULL;; ++p) {
		if (*p == (char)c)
			save = (char *)p;
		if (!*p)
			return(save);
	}
	/* NOTREACHED */
}



char * strcpy(char *restrict dst, const char *restrict src){
	char * ret = dst;
	while ((*dst++ = *src++)){
	// nothing
	}
	return ret;
}

char * strncpy(char *restrict dst, const char *restrict src, size_t size){
	size_t i = 0;
	char * ret = dst;
	while ((*dst++ = *src++ )&& (i < size)){
		i++;
	}
	while (i < size){
		*dst++ = '\0';
	}

	return ret;

}

int strcmp(const char * s1, const char * s2){
    while (*s1 && (*s1 == *s2)){
        s1++;
        s2++;
    }
    return (unsigned char )*s1 - (unsigned char)*s2;
}




int strncmp (const char * s1, const char *s2, size_t size){
while (*s1 && (*s1 == *s2)){
	if (size == 0){
		return 1;
	}
	s1++;
	s2++;
	size--;
}

return 0;
}



char *strcat(char *restrict dst, const char *restrict src)
{
    char *ret = dst;

    while (*dst != '\0')
        dst++;

    while ((*dst++ = *src++) != '\0');

    return ret;
}


size_t strlen(const char *restrict src){
	size_t ret = 0;
	while (*src++ != '\0'){
		ret++;
	}
	return ret;
}


size_t strnlen(const char *restrict src, size_t size){
	size_t ret = 0;
	while (*src++ != '\0'){
	    size--;
					ret++;
	if (size == 0) return ret;
	}

	return ret;
}
