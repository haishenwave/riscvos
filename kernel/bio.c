#include "riscv.h"
#include "fs.h"
#include "buf.h"
#include "printf.h"

// 引用外部函数
extern void virtio_disk_rw(struct buf *b, int write);
extern void panic(char*);

// 缓存块数量 (xv6 默认是 30)
#define NBUF 30

struct {
    struct buf buf[NBUF];
    struct buf head; // LRU 链表头
    // struct spinlock lock; // 暂时省略锁，假设单核无抢占
} bcache;

void binit(void) {
    struct buf *b;

    // 初始化锁 (如果有锁的话)
    // initlock(&bcache.lock, "bcache");

    //构建双向链表环
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    
    for(b = bcache.buf; b < bcache.buf+NBUF; b++){
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        // 初始化每个块的睡眠锁 (暂时只是占位)
        // initsleeklock(&b->lock, "buffer"); 
        
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
    printf("binit: buffer cache initialized\n");
}

// 在缓存中查找块。如果找到，增加引用计数并返回。
// 如果没找到，分配一个新块（可能会驱逐旧块）。
static struct buf* bget(uint32_t dev, uint32_t blockno) {
    struct buf *b;

    // acquire(&bcache.lock);

    // 1. 缓存命中？
    for(b = bcache.head.next; b != &bcache.head; b = b->next){
        if(b->dev == dev && b->blockno == blockno){
            b->refcnt++;
            // release(&bcache.lock);
            // acquiresleep(&b->lock);
            return b;
        }
    }

    // 2. 未命中，回收最久未使用的块 (LRU)
    // 从链表尾部（prev）向前找 refcnt == 0 的块
    for(b = bcache.head.prev; b != &bcache.head; b = b->prev){
        if(b->refcnt == 0) {
            b->dev = dev;
            b->blockno = blockno;
            b->valid = 0; // 数据无效，因为是新分配的
            b->refcnt = 1;
            // release(&bcache.lock);
            // acquiresleep(&b->lock);
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
    }
    return b;
}

// 写回磁盘块
void bwrite(struct buf *b) {
    // if(!holdingsleep(&b->lock)) panic("bwrite");
    virtio_disk_rw(b, 1); // 1 = write
}

// 释放块
void brelse(struct buf *b) {
    // if(!holdingsleep(&b->lock)) panic("brelse");
    // releasesleep(&b->lock);

    // acquire(&bcache.lock);
    b->refcnt--;
    if (b->refcnt == 0) {
        // 如果引用计数为0，把它移到链表头（MRU），
        // 但 xv6 的实现是把空闲块放在链表头？
        // 不，xv6 这里是把它放在 head.next。
        // bget 是从 head.prev (尾部) 开始找空闲块，
        // 也就是说，刚释放的块（放在头部）是最晚被回收的（MRU）。
        
        // 从当前位置断开
        b->next->prev = b->prev;
        b->prev->next = b->next;
        
        // 插入到 head 后面
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }
    // release(&bcache.lock);
}