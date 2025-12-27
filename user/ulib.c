#include "user.h"
#include <stdarg.h>

// 字符串长度
int strlen(const char *s) {
    int n = 0;
    while(s[n]) n++;
    return n;
}

// 内存设置
void* memset(void *dst, int c, int n) {
    char *d = (char*)dst;
    for(int i = 0; i < n; i++)
        d[i] = (char)c;
    return dst;
}

// 输出字符串
void puts(const char *s) {
    write(1, s, strlen(s));
}

// 输出单个字符
static void putc(char c) {
    write(1, &c, 1);
}

// 输出字符串（内部使用）
static void print_str(const char *s) {
    if(s == 0) s = "(null)";
    write(1, s, strlen(s));
}

// 输出无符号整数
static void print_uint(unsigned long long x, int base) {
    char buf[32];
    int i = 0;
    
    if(x == 0) {
        putc('0');
        return;
    }
    
    while(x > 0) {
        int d = x % base;
        buf[i++] = (d < 10) ? ('0' + d) : ('a' + d - 10);
        x /= base;
    }
    
    while(i > 0) {
        putc(buf[--i]);
    }
}

// 输出有符号整数
static void print_int(long long x) {
    if(x < 0) {
        putc('-');
        x = -x;
    }
    print_uint((unsigned long long)x, 10);
}

// 简单的 printf 实现
int printf(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    
    int count = 0;
    
    for(; *fmt; fmt++) {
        if(*fmt != '%') {
            putc(*fmt);
            count++;
            continue;
        }
        
        fmt++;  // 跳过 '%'
        if(*fmt == 0) break;
        
        switch(*fmt) {
        case 'd':
            print_int(va_arg(ap, int));
            break;
        case 'x':
            print_uint(va_arg(ap, unsigned int), 16);
            break;
        case 'p':
            print_str("0x");
            print_uint(va_arg(ap, unsigned long), 16);
            break;
        case 's':
            print_str(va_arg(ap, char*));
            break;
        case 'c':
            putc((char)va_arg(ap, int));
            break;
        case '%':
            putc('%');
            break;
        default:
            putc('%');
            putc(*fmt);
            break;
        }
        count++;
    }
    
    va_end(ap);
    return count;
}