#include "riscv.h"
#include "buf.h"

void init_log(int dev, struct superblock *sb) {
    // 暂时啥也不做
}

void begin_op(void) {
    // 暂时啥也不做
}

void end_op(void) {
    // 暂时啥也不做
}

void log_write(struct buf *b) {
    b->disk = 1; // 标记为脏
    bwrite(b);   // 直接写回磁盘 (没有事务保护，但这符合实验初期的分层策略)
}