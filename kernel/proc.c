#include "riscv.h"
#include "printf.h"
#include "syscall.h"
#include "proc.h"
#include "klog.h"

// === 在文件顶部添加外部符号声明 ===
// 这些符号由链接器生成，指向嵌入的用户程序二进制
extern char _binary_init_bin_start[];
extern char _binary_init_bin_end[];
extern char _binary_init_bin_size[];
//======进程表======
struct proc proc[NPROC]; //进程数组
struct proc *current_proc = 0; //记录当前运行的进程
int nextpid = 1;         //PID计数器
static struct context sched_ctx; // 调度器上下文


extern void swtch(struct context* , struct context* );
extern void* alloc_page(void);
extern void free_page(void*);
extern pagetable_t create_pagetable(void);
extern int map_page(pagetable_t, uint64_t, uint64_t, int);
extern char trampoline[]; //引用trampoline.S中的trampoline符号

//辅助函数，获取当前CPU的进程
struct proc* myproc(void){
    return current_proc;
}

//辅助函数，让出CPU
void yield(void){
    struct proc *p = myproc();
    if(p == 0) return;
    p->state = RUNNABLE;
    swtch(&p->context, &sched_ctx);
}

//辅助函数，简单的字符串复制
void safestrcpy(char *s,const char *t, int n){
    int i;
    for(i = 0; i < n - 1 && t[i] != 0; i++) s[i] = t[i];
    s[i] = 0;
}

//初始化进程表
void proc_init(void){
    for(int i = 0; i < NPROC; i++){
        proc[i].state = UNUSED;
    }
}

//查找一个未使用的进程槽位
static struct proc* alloc_process(void){
    for(int i = 0; i < NPROC; i++){
        if(proc[i].state == UNUSED){
            struct proc *p = &proc[i];
            
            p->pid = nextpid++;
            p->state = USED;
            //分配一个物理页作为内核栈
            p->kstack = alloc_page();
            if(!p->kstack) return 0;
            //创建用户页表
            p->pagetable = create_pagetable();
            if(!p->pagetable){free_page(p->kstack); return 0;}
            //分配Trapframe
            p->trapframe = (struct trapframe*)alloc_page();
            if(!p->trapframe) return 0; //错误处理（简化）
            //映射 Trampoline (RX) 和 Trapframe (RW) 到高地址
            map_page(p->pagetable, TRAMPOLINE, (uint64_t)trampoline, PTE_R | PTE_X);
            map_page(p->pagetable, TRAPFRAME, (uint64_t)p->trapframe, PTE_R | PTE_W); 

            //栈向下增长，栈顶指针指向页面最高地址
            p->sz =0;
            p->context.sp = (uint64_t)p->kstack + PGSIZE;
            p->parent = 0;
            p->chan = 0;
            return p;
        }
    }
    return 0; // 无可用进程槽
}
//启动第一个用户进程
void user_init(void) {
    struct proc *p = alloc_process();
    if(!p) {
        printf("user_init: alloc_process failed!\n");
        return;
    }
    
    p->parent = 0;
    safestrcpy(p->name, "init", 16);
    
    // 计算用户程序大小
    uint64_t sz = (uint64_t)(_binary_init_bin_end - _binary_init_bin_start);
    printf("user_init: loading init program, size=%d bytes\n", (int)sz);
    
    // 分配足够的页来容纳用户程序
    uint64_t num_pages = (sz + PGSIZE - 1) / PGSIZE;
    char *src = _binary_init_bin_start;
    
    for(uint64_t i = 0; i < num_pages; i++) {
        void *mem = alloc_page();
        if(!mem) {
            printf("user_init: alloc_page failed!\n");
            return;
        }
        
        // 清零
        char *dst = (char*)mem;
        for(int j = 0; j < PGSIZE; j++) dst[j] = 0;
        
        // 复制程序内容
        uint64_t offset = i * PGSIZE;
        uint64_t copy_len = sz - offset;
        if(copy_len > PGSIZE) copy_len = PGSIZE;
        
        for(uint64_t j = 0; j < copy_len; j++) {
            dst[j] = src[offset + j];
        }
        
        // 映射页面
        uint64_t va = i * PGSIZE;
        map_page(p->pagetable, va, (uint64_t)mem, PTE_R|PTE_W|PTE_X|PTE_U);
    }
    
    p->sz = num_pages * PGSIZE;
    
    // 分配用户栈（一页）
    void *stack_mem = alloc_page();
    if(!stack_mem) {
        printf("user_init: alloc stack failed!\n");
        return;
    }
    char *stack = (char*)stack_mem;
    for(int i = 0; i < PGSIZE; i++) stack[i] = 0;
    
    // 将栈映射到用户空间高地址
    uint64_t stack_va = p->sz;
    map_page(p->pagetable, stack_va, (uint64_t)stack_mem, PTE_R|PTE_W|PTE_U);
    p->sz += PGSIZE;
    
    // 设置 trapframe
    p->trapframe->epc = 0;                    // 程序入口
    p->trapframe->sp = stack_va + PGSIZE;     // 栈顶
    
    p->state = RUNNABLE;
    
    extern void usertrapret(void);
    p->context.ra = (uint64_t)usertrapret;
    p->context.sp = (uint64_t)p->kstack + PGSIZE;


    printf("user_init: init process created, pid=%d\n", p->pid);
}
//调度器（保持简单逻辑）
void scheduler(void){
    for(;;){ 
        intr_on(); 
        int found = 0;
        for(int i=0;i<NPROC;i++){
            if(proc[i].state == RUNNABLE){
                struct proc *p = &proc[i];

                p->state = RUNNING;
                current_proc = p;           

                swtch(&sched_ctx, &p->context);
                
                current_proc = 0;
                found = 1;
            }
        }
        if(!found) asm volatile("wfi"); //没有可运行进程，等待中断
    }
}


//让进程在chan上休眠
void sleep(void *chan, void *lk){
    struct proc *p = myproc();
    if(p == 0) return; //只有进程上下文才能sleep

    p->chan = chan;
    p->state = SLEEPING;
    swtch(&p->context, &sched_ctx);

    // 唤醒后继续执行
    p->chan = 0;
}

//唤醒在chan上睡眠的进程
void wakeup(void *chan){

    for(int i = 0; i < NPROC; i++){
        struct proc *p = &proc[i];
        if(p->state == SLEEPING && p->chan == chan){
            p->state = RUNNABLE;
            p->chan = 0;
        }
    }
}

//======进程退出======
void exit_process(int status){
    struct proc *p = myproc();
    intr_off(); //关闭中断

    LOG_I("Process %d exiting with status %d\n", p->pid, status);

    p->exit_code = status;
    p->state = ZOMBIE;

    if(p->parent){
        wakeup(p->parent); //唤醒父进程
    }
    swtch(&p->context, &sched_ctx);
    //不应该返回
    printf("PANIC: zombie exit\n");
    for(;;);
}

//======等待子进程退出======
int wait_process(int *status){
    struct proc *p = myproc();

    for(;;){

        intr_off();
        //检查是否有子进程
        int havekids = 0;
        for(int i = 0; i < NPROC; i++){
            struct proc *np = &proc[i];
            if(np->parent == p){
                havekids = 1;
                if(np->state == ZOMBIE){
                    //回收僵尸子进程
                    int pid = np->pid;
                    if( status != 0) *status = np->exit_code;

                    //回收资源
                    free_page(np->kstack);
                    np->kstack = 0;
                    np->state = UNUSED;
                    np->pid = 0;
                    np->parent = 0;
                    np->name[0] = 0;

                    intr_on();
                    return pid;
                }
            }
        }
        //没有子进程，返回-1
        if(!havekids) {
            intr_on();
            return -1;
        }

        //等待子进程退出
        sleep(p, 0);
        intr_on();
    }
}

// 复制当前进程， 创建子进程
int fork(void) {
    struct proc *p = myproc();
    struct proc *np;

    //分配新进程
    np = alloc_process();
    if(np == 0) return -1;
    
    // 遍历父进程的所有页面
    for(uint64_t va = 0; va < p->sz; va += PGSIZE) {
        pte_t *pte = walk_lookup(p->pagetable, va);
        
        // 如果该页存在且有效
        if(pte && (*pte & PTE_V)) {
            // 分配新物理页
            void *mem = alloc_page();
            if(!mem) {
                // 错误处理：释放已分配的（此处简化，未实现完整清理）
                np->state = UNUSED;
                return -1;
            }
            
            // 复制内容
            uint64_t pa = PTE_PA(*pte);
            char *src = (char*)pa;
            char *dst = (char*)mem;
            for(int i = 0; i < PGSIZE; i++) dst[i] = src[i];
            
            // 映射到子进程，使用相同的权限
            // 保留 PTE_U 等标志，去掉 PTE_A/PTE_D 等硬件标志
            // 这里简单起见，假设都是 RWXU
            int perm = PTE_R | PTE_W | PTE_X | PTE_U;
            if(map_page(np->pagetable, va, (uint64_t)mem, perm) < 0) {
                free_page(mem);
                np->state = UNUSED;
                return -1;
            }
        }
    }
    
    // 设置子进程大小
    np->sz = p->sz;

    //复制Trapframe
    char *src_tf = (char*)p->trapframe;
    char *dst_tf = (char*)np->trapframe;
    for(int i=0; i<PGSIZE; i++) dst_tf[i] = src_tf[i];

    //子进程返回值为0
    np->trapframe->a0 = 0;

    //设置父子关系
    np->parent = p;
    safestrcpy(np->name, p->name, sizeof(p->name));

    //设置子进程上下文， 使其从 usertrapret 开始
    extern void usertrapret(void);
    np->context.ra = (uint64_t)usertrapret;
    np->context.sp = (uint64_t)np->kstack + PGSIZE;

    np->state = RUNNABLE;

    //父进程返回子进程PID
    return np->pid;
}

//终止指定进程
int kill(int pid) {
    for(int i = 0; i < NPROC; i++){
        if(proc[i].pid == pid) {
            proc[i].killed = 1;
            //如果进程在睡眠，唤醒它以便它能检查killed标志
            if(proc[i].state == SLEEPING) {
                proc[i].state = RUNNABLE;
            }
            return 0;
        }
    }
    return -1; //未找到进程
}

//调整堆大小
uint64_t sbrk(int n) {
    struct proc *p = myproc();
    uint64_t oldsz = p->sz;
    uint64_t newsz = oldsz + n;

    if(n > 0) {
        //扩展内存
        uint64_t a = PGGROUNDUP(oldsz);
        for(; a < newsz; a += PGSIZE) {
            void *mem = alloc_page();
            if(mem ==0) return -1;

            //清零
            char *dst = (char*)mem;
            for(int i=0; i<PGSIZE; i++) dst[i] =0;

            if(map_page(p->pagetable, a, (uint64_t)mem, PTE_R|PTE_W|PTE_X|PTE_U) <0){
                free_page(mem);
                return -1;
            }
        }
    }else if(n < 0) {
        // 收缩空间：不释放
        if(newsz < 0) newsz = 0;
    }

    p->sz = newsz;
    return oldsz;
}