#include "riscv.h"
#include "printf.h"
#include "proc.h"



extern void kernelvec(void);
extern void syscall(void);
extern char trampoline[];
extern char uservec[];
extern char userret[];

// panic函数
void panic(const char *s) {
    printf("PANIC: %s\n", s);
    for(;;) asm volatile("wfi");
}

// trap 初始化
void trap_init(void) {
    w_stvec((uint64_t)kernelvec);
}

// 内核陷阱
void kerneltrap(void) {
    uint64_t sepc = r_sepc();
    uint64_t sstatus = r_sstatus();
    uint64_t scause = r_scause();

    if((scause & 0x8000000000000000L) && (scause & 0xff) == 5) { 
       //yield(); //时钟中断,暂不处理
    }
    else {
        printf("kerneltrap: scause %p sepc %p stval %p\n", scause, sepc, r_stval());
        panic("kerneltrap");
    }
    w_sepc(sepc);
    w_sstatus(sstatus);
}

//用户陷阱
void usertrap(void) {
    struct proc *p = myproc();

    if((r_sstatus() & 0x100) != 0) panic("usertrap: not from user mode");

    //此时已经是内核页表， 切换 stvec 到 kernelvec
    w_stvec((uint64_t)kernelvec);

    p->trapframe->epc = r_sepc(); //保存用户pc

    uint64_t scause = r_scause();
    if(scause == 8) {
        //系统调用
        if(p->killed) exit_process(-1);

        p->trapframe->epc += 4; //跳过 ecall 指令
        intr_on();
        syscall();
    } else if((scause & 0x8000000000000000L) && (scause & 0xff) == 5) {
        yield(); //时钟中断
    } else {
        printf("usertrap(): unexpected scause %p pid=%d\n", scause, p->pid);
        printf("            sepc=%p stval=%p\n", r_sepc(), r_stval());
        p->killed = 1;
    }

    if(p->killed) exit_process(-1);

    extern void usertrapret(void);
    usertrapret();
}

// 返回用户态
void usertrapret(void) {
    struct proc * p = myproc();

    intr_off();

    // 设置 stvec 指向 trampoline 的 uservec
    w_stvec(TRAMPOLINE + ((uint64_t)uservec - (uint64_t)trampoline));

    // 设置 trapframe 中的内核信息
    p->trapframe->kernel_satp = r_satp(); // 内核页表
    p->trapframe->kernel_sp = (uint64_t)p->kstack + PGSIZE; // 内核栈指针
    p->trapframe->kernel_trap = (uint64_t)usertrap; // 内核陷阱处理函数入口
    p->trapframe->kernel_hartid = 0;

    // 设置 SSTATUS (SPP =0 User, SPIE =1 Enable Intr)
    unsigned long x = r_sstatus();
    x &= -(1ULL << 8); // SPP = 0
    x |= (1UL << 5);   // SPIE = 1
    w_sstatus(x);

    // 设置 SEPC
    w_sepc(p->trapframe->epc);

    // 计算 satp 切换回用户页表
    uint64_t satp = (8ULL << 60) | ((uint64_t)p->pagetable >> 12);

    // 跳到 trampoline 执行 userret
    uint64_t fn = TRAMPOLINE + ((uint64_t)userret - (uint64_t)trampoline);

    ((void (*)(uint64_t, uint64_t))fn)(TRAPFRAME , satp);

    // 不应该到达这里
    printf("usertrapret: ERROR - returned from userret!\n");
}