#include "riscv.h"

#define TIMER_INTERVAL 1000000 //时钟周期

static inline void sbi_set_timer(uint64_t stime){
    register uint64_t a0 asm("a0") = stime;
    register uint64_t a7 asm("a7") = 0x54494D45;
    register uint64_t a6 asm("a6") = 0;
    asm volatile("ecall"
                 : "+r"(a0)
                 : "r"(a7), "r"(a6));
}

uint64_t get_time(void){
    uint64_t t;
    asm volatile("rdtime %0" : "=r"(t));
    return t;
}

//定时器初始化
void timer_init(void){
    uint64_t current_time = get_time();
    sbi_set_timer(current_time + TIMER_INTERVAL);   //设置第一次中断
    asm volatile("csrs sie, %0": : "r"(1ULL << 5)); //使能时钟中断
    intr_on();
}