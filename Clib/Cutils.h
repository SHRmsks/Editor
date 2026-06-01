#ifndef CUTILS_H
#define CUTILS_H
#include <stdint.h>
#include <stddef.h>
void c_memcpy(void *dest, const void *src, uint32_t n);
extern void console_log_int(int value);
extern void console_log_string(const char *value);
#endif // CUTILS_H