#include "riscv.h"
#include "printf.h"
#include "syscall.h"
#include "proc.h"
#include "fs.h"

extern int argint(int, int*);
extern int argaddr(int, uint64_t*);
extern int argstr(int, char*, int);
extern int copyin(pagetable_t, char*, uint64_t, uint64_t);
extern int copyout(pagetable_t, uint64_t, char*, uint64_t);
extern void console_putc(char c);
extern uint64_t get_time(void);
extern void log_dump(void);
extern void log_stats(void);

// 引入日志事务控制函数
extern void begin_op(void);
extern void end_op(void);

// 文件系统相关外部函数
extern int filewrite(struct file*, char*, int);
extern int fileread(struct file*, char*, int);

int sys_fork(void) {
    return fork();
}

int sys_exit(void) {
    int n;
    argint(0, &n);
    exit_process(n);
    return 0;
}

int sys_wait(void) {
    uint64_t addr;
    if(argaddr(0, &addr) < 0) return -1;

    int status;
    int pid = wait_process(&status);

    if(pid > 0 && addr != 0) {
        //将状态写回用户空间
        struct proc *p = myproc();
        pte_t *pte = walk_lookup(p->pagetable, addr);
        if(pte && (*pte & PTE_V) && (*pte & PTE_U)) {
            uint64_t pa = PTE_PA(*pte) + (addr & (PGSIZE -1));
            *((int*)pa) = status;
        }
    }
    return pid;
}

int sys_kill(void) {
    int pid;
    if(argint(0, &pid) < 0) return -1;
    return kill(pid);
}

int sys_getpid(void) {
    return myproc()->pid;
}

int sys_sbrk(void) {
    int n;
    if(argint(0, &n) < 0) return -1;
    return sbrk(n);
}

// sys_write: 写文件或控制台
// 关键修复：添加 begin_op() 和 end_op() 事务保护
int sys_write(void) {
    int fd, n;
    uint64_t addr;
    struct proc *p = myproc();

    if(argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0)
        return -1;

    if(n < 0)
        return -1;

    // 控制台输出 (fd = 1 或 2)
    // 控制台输出不需要文件系统事务
    if(fd == 1 || fd == 2) {
        char buf[128];
        int total = 0;
        while(n > 0) {
            int len = (n > 128) ? 128 : n;
            if(copyin(p->pagetable, buf, addr, len) < 0)
                return -1;

            for(int i = 0; i < len; i++)
                console_putc(buf[i]);
            total += len;
            n -= len;
            addr += len;
        }
        return total;
    }

    // 文件写入
    if(fd < 0 || fd >= NOFILE || p->ofile[fd] == 0)
        return -1;

    struct file *f = p->ofile[fd];
    char buf[512];
    int total = 0;
    while(n > 0) {
        int m = n > 512 ? 512 : n;
        
        // --- 事务开始 ---
        begin_op();
        
        if(copyin(p->pagetable, buf, addr, m) < 0) {
            end_op(); // 错误退出前必须结束事务
            return -1;
        }
        
        // 写入数据 (内部可能会调用 log_write)
        int r = filewrite(f, buf, m);
        
        // --- 事务结束 ---
        end_op();
        
        if(r <= 0)
            break;
        total += r;
        addr += r;
        n -= r;
    }
    return total;
}

int sys_read(void) {
    int fd, n;
    uint64_t addr;
    struct proc *p = myproc();

    if(argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0)
        return -1;

    if(n < 0)
        return -1;

    // 控制台输入 (fd = 0) - 暂不支持
    if(fd == 0) {
        return 0;
    }

    // 文件读取
    // 读取通常不需要事务（除非涉及更新访问时间等元数据，这里简化处理）
    if(fd < 0 || fd >= NOFILE || p->ofile[fd] == 0)
        return -1;

    struct file *f = p->ofile[fd];
    char buf[512];
    int total = 0;
    while(n > 0) {
        int m = n > 512 ? 512 : n;
        int r = fileread(f, buf, m);
        if(r <= 0)
            break;
        if(copyout(p->pagetable, addr, buf, r) < 0)
            return -1;
        total += r;
        addr += r;
        n -= r;
        if(r < m)
            break;
    }
    return total;
}

int sys_uptime(void) {
    return (int)get_time();
}

int sys_logdump(void) {
    log_dump();
    return 0;
}

int sys_logstat(void) {
    log_stats();
    return 0;
}