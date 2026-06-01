#include "Cutils.h"
void c_memcpy(void *dest, const void *src, uint32_t n)
{
    char *d = dest;
    const char *s = src;
    while (n--)
        *d++ = *s++;
    return;
}