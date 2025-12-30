#include "riscv.h"
#include "fs.h"
#include "buf.h"
#include "printf.h"

// 引用外部函数
extern void virtio_disk_rw(struct buf *b, int write);
extern void panic(char*);

#define NBUF 30

struct {
    struct buf buf[NBUF];
    struct buf head; // LRU 链表头
} bcache;

void binit(void) {
    struct buf *b;
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    
    for(b = bcache.buf; b < bcache.buf+NBUF; b++){
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
        b->valid = 0;
        b->disk = 0; 
        b->refcnt = 0;
    }
    printf("binit: buffer cache initialized\n");
}

static struct buf* bget(uint32_t dev, uint32_t blockno) {
    struct buf *b;

    // 1. 缓存命中？
    for(b = bcache.head.next; b != &bcache.head; b = b->next){
        if(b->dev == dev && b->blockno == blockno){
            b->refcnt++;
            return b;
        }
    }

    // 2. 未命中，回收最久未使用的块 (LRU)
    // 关键修改：必须跳过 b->disk == 1 的块（脏块），因为它们属于未提交的日志事务
    for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
        if(b->refcnt == 0 && b->disk == 0) { // <--- 增加了 && b->disk == 0
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0; 
            b->refcnt = 1;
            return b;
        }
    }
    
    panic("bget: no buffers");
    return 0;
}

// 读取磁盘块
struct buf* bread(uint32_t dev, uint32_t blockno) {
    struct buf *b = bget(dev, blockno);
    if(!b->valid) {
        virtio_disk_rw(b, 0); // 0 = read
        b->valid = 1;
        b->disk = 0; // 刚读出来，是干净的
    }
    return b;
}

// 写回磁盘块
void bwrite(struct buf *b) {
    virtio_disk_rw(b, 1); // 1 = write
}

// 释放块
void brelse(struct buf *b) {
    b->refcnt--;
    if (b->refcnt == 0) {
        // 移到 MRU (头部)
        b->next->prev = b->prev;
        b->prev->next = b->next;
        
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
}