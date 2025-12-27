#include "riscv.h"
#include "syscall.h"
#include "printf.h"
#include "proc.h"

extern int copyinstr(pagetable_t, char*, uint64_t, uint64_t);

int argint(int n, int *ip) {
    struct proc * p = myproc();
    switch(n) {
        case 0: *ip = p->trapframe->a0; return 0;
        case 1: *ip = p->trapframe->a1; return 0;
        case 2: *ip = p->trapframe->a2; return 0;
        case 3: *ip = p->trapframe->a3; return 0;
        case 4: *ip = p->trapframe->a4; return 0;
        case 5: *ip = p->trapframe->a5; return 0;
        default: return -1;
    }
    return 0;
}

int argaddr(int n, uint64_t *ip) {
    struct proc * p = myproc();
    switch(n) {
        case 0: *ip = p->trapframe->a0; return 0;
        case 1: *ip = p->trapframe->a1; return 0;
        case 2: *ip = p->trapframe->a2; return 0;
        case 3: *ip = p->trapframe->a3; return 0;
        case 4: *ip = p->trapframe->a4; return 0;
        case 5: *ip = p->trapframe->a5; return 0;
        default: return -1;
    }
    return 0;
}
int argstr(int n, char *buf, int max) {
    uint64_t addr;
    if(argaddr(n, &addr) < 0) return -1;
    return copyinstr(myproc()->pagetable, buf, addr, max);
}

extern int sys_write(void);
extern int sys_exit(void);
extern int sys_getpid(void);
extern int sys_fork(void);
extern int sys_wait(void);
extern int sys_kill(void);
extern int sys_sbrk(void);
extern int sys_read(void);
extern int sys_open(void);
extern int sys_close(void);
extern int sys_uptime(void);
extern int sys_unlink(void);
extern int sys_logdump(void);
extern int sys_logstat(void);

static int (*syscalls[])(void) ={
    [SYS_write] sys_write,
    [SYS_exit] sys_exit,
    [SYS_getpid] sys_getpid,
    [SYS_fork] sys_fork,
    [SYS_wait] sys_wait,
    [SYS_kill] sys_kill,
    [SYS_sbrk] sys_sbrk,
    [SYS_read] sys_read,
    [SYS_open] sys_open,
    [SYS_close] sys_close,
    [SYS_uptime] sys_uptime,
    [SYS_unlink] sys_unlink,
    [SYS_logdump] sys_logdump,
    [SYS_logstat] sys_logstat,
};

static char * syscall_names[] = {
    [SYS_write] "write",
    [SYS_exit] "exit",
    [SYS_getpid] "getpid",
    [SYS_fork] "fork",
    [SYS_wait] "wait",
    [SYS_kill] "kill",
    [SYS_sbrk] "sbrk",
    [SYS_read] "read",
    [SYS_open] "open",
    [SYS_close] "close",
    [SYS_uptime] "uptime",
    [SYS_unlink] "unlink",
};

#define NELEM(x) (sizeof(x)/sizeof((x)[0]))

void syscall(void) {
    struct proc *p = myproc();
    int num = p->trapframe->a7;

    if(num > 0 && num < (int)NELEM(syscalls) && syscalls[num]) {
        p->trapframe->a0 = syscalls[num]();
    } else {
        printf("Unknown syscall %d\n", num);
        p->trapframe->a0 = -1;
    }
}