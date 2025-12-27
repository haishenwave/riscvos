#include "riscv.h"
#include "printf.h"
#include "klog.h"

// BSS 段清零
extern char bss_start[];
extern char bss_end[];

static void clear_bss(void) {
    for(char *dst = bss_start; dst < bss_end; dst++)
        *dst = 0;
}

// 外部函数声明
extern void pmm_init(void);
extern void kvminit(void);
extern void kvminithart(void);
extern void trap_init(void);
extern void proc_init(void);
extern void virtio_disk_init(void);
extern void binit(void);
extern void fsinit(int);
extern void fileinit(void);
extern void user_init(void);
extern void scheduler(void);
extern void log_init(void);

void kmain(void) {
    clear_bss();
    printf("=== Lab 7 & 8: File System + Kernel Log ===\n");

    // 初始化日志系统
    log_init();

    // 内存管理
    LOG_I("Initializing physical memory manager...\n");
    pmm_init();

    LOG_I("Setting up kernel page table...\n");
    kvminit();
    kvminithart();

    // 中断与进程
    LOG_I("Initializing trap handlers...\n");
    trap_init();
    
    LOG_I("Initializing process table...\n");
    proc_init();

    // 文件系统
    LOG_I("Initializing disk driver...\n");
    virtio_disk_init();
    
    LOG_I("Initializing buffer cache...\n");
    binit();
    
    LOG_I("Mounting file system...\n");
    fsinit(0);
    
    LOG_I("Initializing file table...\n");
    fileinit();

    // 用户进程
    LOG_I("Creating init process...\n");
    user_init();

    // 显示日志统计
    log_stats();

    LOG_I("Starting scheduler...\n");
    scheduler();
}