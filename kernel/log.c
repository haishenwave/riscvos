#include "riscv.h"
#include "fs.h"
#include "buf.h"
#include "printf.h"
#include "klog.h"
#include "proc.h" 

extern struct buf* bread(uint32_t, uint32_t);
extern void brelse(struct buf*);
extern void bwrite(struct buf*);
extern void panic(const char*);

struct logheader {
    int n;
    int block[30]; 
};

struct log {
    int start;
    int size;
    int outstanding; 
    int committing; 
    int dev;
    struct logheader lh;
    int lock; 
};

struct log log;

void acquire_log(void) {
    while(log.lock) {} // 忙等待
    log.lock = 1;
}

void release_log(void) {
    log.lock = 0;
}

static void install_trans(int recovering) {
    int tail;
    for (tail = 0; tail < log.lh.n; tail++) {
        struct buf *lbuf = bread(log.dev, log.start + tail + 1); 
        struct buf *dbuf = bread(log.dev, log.lh.block[tail]);
        
        for(int i = 0; i < BSIZE; i++) 
            dbuf->data[i] = lbuf->data[i];
        
        bwrite(dbuf); 
        
        if(!recovering) {
            // 事务已提交并写回磁盘，标记缓冲区为干净
            // 这样 bget 就可以回收它了
            dbuf->disk = 0; 
        }
        
        brelse(lbuf);
        brelse(dbuf);
    }
}

static void read_head(void) {
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *lh = (struct logheader *) (buf->data);
    log.lh.n = lh->n;
    for (int i = 0; i < log.lh.n; i++) {
        log.lh.block[i] = lh->block[i];
    }
    brelse(buf);
}

static void write_head(void) {
    struct buf *buf = bread(log.dev, log.start);
    struct logheader *lh = (struct logheader *) (buf->data);
    lh->n = log.lh.n;
    for (int i = 0; i < log.lh.n; i++) {
        lh->block[i] = log.lh.block[i];
    }
    bwrite(buf);
    brelse(buf);
}

static void recover_from_log(void) {
    read_head();
    if(log.lh.n > 0) {
        LOG_I("Recovering %d blocks from log...\n", log.lh.n);
        install_trans(1);
        log.lh.n = 0;
        write_head(); 
    }
}

static void write_log(void) {
    int tail;
    for (tail = 0; tail < log.lh.n; tail++) {
        struct buf *to = bread(log.dev, log.start + tail + 1); 
        struct buf *from = bread(log.dev, log.lh.block[tail]);
        
        for(int i = 0; i < BSIZE; i++) 
            to->data[i] = from->data[i];
            
        bwrite(to); 
        brelse(from);
        brelse(to);
    }
}

static void commit(void) {
    if (log.lh.n > 0) {
        write_log();       
        write_head();      
        install_trans(0);  
        log.lh.n = 0;
        write_head();      
    }
}

void init_log(int dev, struct superblock *sb) {
    if (sizeof(struct logheader) >= BSIZE)
        panic("init_log: too big logheader");

    log.start = sb->logstart;
    log.size = sb->nlog;
    log.dev = dev;
    log.lock = 0;
    
    recover_from_log();
    
    LOG_I("Log system initialized: dev=%d start=%d size=%d\n", dev, log.start, log.size);
}

void begin_op(void) {
    acquire_log();
    while(1) {
        if (log.committing) {
            sleep(&log, &log.lock);
        } else if (log.lh.n + (log.outstanding + 1) * 5 > log.size - 5) {
            sleep(&log, &log.lock);
        } else {
            log.outstanding += 1;
            release_log();
            break;
        }
    }
}

void end_op(void) {
    int do_commit = 0;

    acquire_log();
    log.outstanding -= 1;
    
    if(log.committing)
        panic("log: committing");
    
    if(log.outstanding == 0) {
        do_commit = 1;
        log.committing = 1;
    } else {
        wakeup(&log);
    }
    release_log();

    if(do_commit) {
        commit();
        acquire_log();
        log.committing = 0;
        wakeup(&log);
        release_log();
    }
}

void log_write(struct buf *b) {
    int i;

    acquire_log();
    if (log.lh.n >= log.size - 1)
        panic("log: too big");
    if (log.outstanding < 1)
        panic("log: outside of trans");

    for (i = 0; i < log.lh.n; i++) {
        if (log.lh.block[i] == b->blockno)
            break;
    }
    
    if (i == log.lh.n) {
        log.lh.block[i] = b->blockno;
        log.lh.n++;
    }
    
    // 标记 buffer 为脏
    // 有了 bio.c 的修改，脏块不会被回收，所以不需要额外的 pin
    b->disk = 1; 
    
    release_log();
}