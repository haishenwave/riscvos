#ifndef _BUF_H
#define _BUF_H

#include <stdint.h>
#include "riscv.h" // 获取 spinlock 定义 (如果有的话，没有的话暂时不用)

// 简单的自旋锁定义 (为了独立性，如果你没在 riscv.h 定义，这里补一个简化的)
#ifndef _SPINLOCK_H
struct spinlock {
    int locked;
};
#endif

// 简单的睡眠锁
struct sleeplock {
    uint32_t locked;       // 是否被锁住
    struct spinlock lk;    // 保护 locked 字段的自旋锁
    char *name;            // 锁名称
    int pid;               // 持有锁的进程PID
};

struct buf {
    int valid;     // 数据是否已从磁盘读入 (1=有效)
    int disk;      // 内容是否已被修改 (1=需写回磁盘)
    uint32_t dev;  // 设备号
    uint32_t blockno; // 块号
    struct sleeplock lock;
    uint32_t refcnt;
    struct buf *prev; // LRU 链表
    struct buf *next;
    uint8_t data[BSIZE]; // 实际数据 (1024字节)
};

#endif