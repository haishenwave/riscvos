#include "riscv.h"
#include "fs.h"
#include "printf.h"
#include "proc.h"

extern void ilock(struct inode*);
extern void iunlock(struct inode*);
extern void iput(struct inode*);
extern int readi(struct inode*, char*, uint32_t, uint32_t);
extern int writei(struct inode*, char*, uint32_t, uint32_t);
extern void panic(const char*);

// 全局文件表
struct {
    struct file file[NFILE];
} ftable;

// 初始化文件表
void fileinit(void) {
    for(int i = 0; i < NFILE; i++) {
        ftable.file[i].ref = 0;
        ftable.file[i].type = FD_NONE;
    }
}

// 分配一个文件结构
struct file* filealloc(void) {
    for(int i = 0; i < NFILE; i++) {
        if(ftable.file[i].ref == 0) {
            ftable.file[i].ref = 1;
            return &ftable.file[i];
        }
    }
    return 0;
}

// 增加文件引用计数
struct file* filedup(struct file *f) {
    if(f->ref < 1)
        panic("filedup");
    f->ref++;
    return f;
}

// 关闭文件
void fileclose(struct file *f) {
    if(f->ref < 1)
        panic("fileclose");
    
    f->ref--;
    if(f->ref > 0)
        return;

    // 引用计数为 0，释放资源
    if(f->type == FD_INODE) {
        iput(f->ip);
    }
    f->type = FD_NONE;
}

// 读取文件
int fileread(struct file *f, char *addr, int n) {
    int r;

    if(f->readable == 0)
        return -1;

    if(f->type == FD_INODE) {
        ilock(f->ip);
        r = readi(f->ip, addr, f->off, n);
        if(r > 0)
            f->off += r;
        iunlock(f->ip);
        return r;
    }

    panic("fileread");
    return -1;
}

// 写入文件
int filewrite(struct file *f, char *addr, int n) {
    int r;

    if(f->writable == 0)
        return -1;

    if(f->type == FD_INODE) {
        ilock(f->ip);
        r = writei(f->ip, addr, f->off, n);
        if(r > 0)
            f->off += r;
        iunlock(f->ip);
        return r;
    }

    panic("filewrite");
    return -1;
}