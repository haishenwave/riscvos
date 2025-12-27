#include <stdarg.h>
#include<stdint.h>

void console_putc(char c);

static  void outc(char c, int *cnt){
    console_putc(c);
    if(cnt) (*cnt)++;
}

static void prints(const char* s,int *cnt){
    if(!s) s="(null)";
    for(; *s; s++) outc(*s, cnt);
}

static void printu(unsigned long long x, int base, int *cnt){
    char buf[32]; 
    int i=0;
    if(x == 0){
        outc('0',cnt);
        return;
    }
    while(x){
        unsigned r = (unsigned)(x % (unsigned)base);
        buf[i++]=(r<10)? ('0'+r) : ('a'+(r-10));
        x/=(unsigned)base;
    }
    while(i-- > 0) outc(buf[i],cnt);
}

static void printd(long long v, int *cnt){
    if(v<0){
        if(v == (long long)0x80000000){
            prints("-2147483648", cnt);
            return;
        }
        outc('-',cnt);
        unsigned long long u=(unsigned long long)(-v);
        printu(u,10,cnt);
    }
    else{
        printu((unsigned long long)v,10,cnt);
    }
}



int printf(const char* fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    int n=0;
    for(; *fmt; fmt++){
        if(*fmt!='%'){
            outc(*fmt,&n);
            continue;
        }
        char f=*++fmt;
        if(!f)break;
        switch (f)
        {
        case 'd':
            printd((long long)va_arg(ap,int),&n); break;
        case 'x':
            printu((unsigned long long)va_arg(ap,unsigned int),16,&n); break;
        case 'p':
            uintptr_t p=(uintptr_t)va_arg(ap,void*);
            prints("0x",&n);
            printu((unsigned long long)p,16,&n);
            break;
        case 's':
            prints(va_arg(ap,const char*),&n);
            break;
        case 'c':
            outc((char)va_arg(ap,int),&n);
            break;
        case '%':
            outc('%',&n);
            break;
        default:
            outc('%',&n);
            outc(f,&n);
            break;
        }
    }
    va_end(ap);
    return n;
}