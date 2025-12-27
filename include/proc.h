#ifndef _PROC_H
#define _PROC_H

#include <stdint.h>
#include "fs.h"

#define NPROC 64  //最大进程数
#define PGSIZE 4096
#define MAXVA (1ULL << 38)
#define TRAMPOLINE (MAXVA - PGSIZE)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)
struct file;

//陷阱帧结构体
struct trapframe {
    uint64_t kernel_satp;   // 内核页表
    uint64_t kernel_sp;     // 内核栈指针
    uint64_t kernel_trap;   // 内核陷阱处理函数入口
    uint64_t epc;          // 用户程序计数器
    uint64_t kernel_hartid; // 内核hart ID
    uint64_t ra;
    uint64_t sp;
    uint64_t gp;
    uint64_t tp;
    uint64_t t0;
    uint64_t t1;
    uint64_t t2;
    uint64_t s0;
    uint64_t s1;
    uint64_t a0;
    uint64_t a1;
    uint64_t a2;
    uint64_t a3;
    uint64_t a4;
    uint64_t a5;
    uint64_t a6;
    uint64_t a7;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
    uint64_t t3;
    uint64_t t4;
    uint64_t t5;
    uint64_t t6;
};

//进程状态枚举
enum procstate { UNUSED, USED, RUNNABLE, RUNNING, SLEEPING, ZOMBIE };

//进程上下文
struct context {
    uint64_t ra;
    uint64_t sp;
    uint64_t s0;
    uint64_t s1;
    uint64_t s2;
    uint64_t s3;
    uint64_t s4;
    uint64_t s5;
    uint64_t s6;
    uint64_t s7;
    uint64_t s8;
    uint64_t s9;
    uint64_t s10;
    uint64_t s11;
};

//进程控制块
struct proc {
    struct context context; // 上下文切换保存
    enum procstate state;   // 进程状态
    int pid;               // 进程ID
    void* kstack;          // 内核栈指针
    char name[16];         // 进程名称
    struct proc *parent;
    void *chan;            // 睡眠等待的chan
    int exit_code;
    int killed;

    uint64_t sz;           // 进程大小
    uint64_t* pagetable;   // 页表
    struct trapframe *trapframe; //陷阱帧
    struct file *ofile[NOFILE]; // 打开的文件
};

//函数声明
struct proc* myproc(void);
void yield(void);
void exit_process(int status);
void wakeup(void *chan);
void sleep(void *chan, void *lk);
int fork(void);
int kill(int);
int wait_process(int *status);
uint64_t sbrk(int n);


#endif