#include <stdint.h>

#define UART_BASE 0x10000000UL // QEMU virt机器的UART0物理地址
#define UART_THR (UART_BASE + 0x00) //发送保持寄存器（写这个发数据）
#define UART_LSR (UART_BASE + 0x05) //线路状态寄存器（读这个看状态）
#define LSR_THRE 0x20 //发送缓冲区中断位（为1表示可以发送）

//读写内存工具函数
static inline void write8(uintptr_t addr,uint8_t v){
    *(volatile uint8_t *)addr = v; 
}

static inline uint8_t read8(uintptr_t addr){
    return *(volatile uint8_t *)addr;
}

//输出一个字符
void uart_putc(char c){
    while((read8(UART_LSR)&LSR_THRE)==0){} //死循环等待，直到UART准备好发送
    //把字符写给UART
    write8(UART_THR,(uint8_t)c);
}

void uart_puts(const char *s){
    for(; *s; s++){
        if(*s == '\n') uart_putc('\r'); // add \r 
        uart_putc(*s);
    }
}