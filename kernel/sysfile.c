#include "riscv.h"
#include "fs.h"
#include "printf.h"
#include "proc.h"

// 外部函数
extern struct proc* myproc(void);
extern int argint(int, int*);
extern int argaddr(int, uint64_t*);
extern int argstr(int, char*, int);
extern int copyin(uint64_t*, char*, uint64_t, uint64_t);
extern int copyout(uint64_t*, uint64_t, char*, uint64_t);
extern void panic(const char*);

extern struct file* filealloc(void);
extern void fileclose(struct file*);
extern int fileread(struct file*, char*, int);
extern int filewrite(struct file*, char*, int);

extern struct inode* namei(char*);
extern struct inode* nameiparent(char*, char*);
extern struct inode* ialloc(uint32_t, short);
extern void ilock(struct inode*);
extern void iunlock(struct inode*);
extern void iput(struct inode*);
extern int dirlink(struct inode*, char*, uint32_t);
extern void iunlockput(struct inode*);
extern void itrunc(struct inode*);

static void* memset_local(void *dst, int c, int n) {
    char *d = (char*)dst;
    for(int i = 0; i < n; i++)
        d[i] = (char)c;
    return dst;
}
// 分配文件描述符
static int fdalloc(struct file *f) {
    struct proc *p = myproc();
    for(int fd = 3; fd < NOFILE; fd++) {
        if(p->ofile[fd] == 0) {
            p->ofile[fd] = f;
            return fd;
        }
    }
    return -1;
}

// sys_open: 打开或创建文件
// sys_open: 打开或创建文件
int sys_open(void) {
    char path[128];
    char name[DIRSIZ];  // 添加独立的 name 缓冲区
    int omode;
    struct inode *ip;
    struct file *f;
    int fd;

    if(argstr(0, path, 128) < 0 || argint(1, &omode) < 0)
        return -1;


    if(omode & O_CREATE) {
        // 创建文件
        ip = nameiparent(path, name);  // 使用独立的 name 缓冲区
        if(ip == 0)
            return -1;
        ilock(ip);
        
        // 检查是否已存在
        struct inode *dp = ip;
        ip = dirlookup(dp, name, 0);  // 使用 name 而不是 path
        if(ip != 0) {
            // 文件已存在
            iunlock(dp);
            iput(dp);
            ilock(ip);
        } else {
            // 创建新文件
            ip = ialloc(dp->dev, T_FILE);
            if(ip == 0) {
                iunlock(dp);
                iput(dp);
                return -1;
            }
            ilock(ip);
            ip->nlink = 1;
            iupdate(ip);
            // 添加目录项
            if(dirlink(dp, name, ip->inum) < 0) {  // 使用 name 而不是 path
                ip->nlink = 0;
                iupdate(ip);
                iunlock(ip);
                iput(ip);
                iunlock(dp);
                iput(dp);
                return -1;
            }
            iunlock(dp);
            iput(dp);
        }
    } else {
        // 打开已存在的文件
        ip = namei(path);
        if(ip == 0)
            return -1;
        ilock(ip);
    }

    // 分配文件结构
    f = filealloc();
    if(f == 0) {
        iunlock(ip);
        iput(ip);
        return -1;
    }

    // 分配文件描述符
    fd = fdalloc(f);
    if(fd < 0) {
        fileclose(f);
        iunlock(ip);
        iput(ip);
        return -1;
    }

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    iunlock(ip);
    return fd;
}

// sys_close: 关闭文件
int sys_close(void) {
    int fd;
    struct file *f;
    struct proc *p = myproc();

    if(argint(0, &fd) < 0)
        return -1;
    if(fd < 0 || fd >= NOFILE || p->ofile[fd] == 0)
        return -1;

    f = p->ofile[fd];
    p->ofile[fd] = 0;
    fileclose(f);
    return 0;
}

// sys_fread: 从文件读取 (用于文件系统的 read)
int sys_fread(void) {
    int fd, n;
    uint64_t addr;
    struct file *f;
    struct proc *p = myproc();

    if(argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0)
        return -1;
    if(fd < 0 || fd >= NOFILE || (f = p->ofile[fd]) == 0)
        return -1;

    // 读取到内核缓冲区，再复制到用户空间
    char buf[512];
    int total = 0;
    while(n > 0) {
        int m = n > 512 ? 512 : n;
        int r = fileread(f, buf, m);
        if(r <= 0)
            break;
        // 复制到用户空间 (需要 copyout)
        // 这里简化处理，直接写到用户地址
        extern int copyout(uint64_t*, uint64_t, char*, uint64_t);
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

// sys_fwrite: 写入文件 (用于文件系统的 write)
int sys_fwrite(void) {
    int fd, n;
    uint64_t addr;
    struct file *f;
    struct proc *p = myproc();

    if(argint(0, &fd) < 0 || argaddr(1, &addr) < 0 || argint(2, &n) < 0)
        return -1;
    if(fd < 0 || fd >= NOFILE || (f = p->ofile[fd]) == 0)
        return -1;

    char buf[512];
    int total = 0;
    while(n > 0) {
        int m = n > 512 ? 512 : n;
        if(copyin(p->pagetable, buf, addr, m) < 0)
            return -1;
        int r = filewrite(f, buf, m);
        if(r <= 0)
            break;
        total += r;
        addr += r;
        n -= r;
    }
    return total;
}

int sys_unlink(void) {
    char path[128];
    char name[DIRSIZ];
    struct inode *ip, *dp;
    struct dirent de;
    uint32_t off;

    if(argstr(0, path, 128) < 0)
        return -1;

    // 获取父目录
    dp = nameiparent(path, name);
    if(dp == 0)
        return -1;

    ilock(dp);

    // 不能删除 "." 和 ".."
    if(name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
        iunlockput(dp);
        return -1;
    }

    // 查找文件
    ip = dirlookup(dp, name, &off);
    if(ip == 0) {
        iunlockput(dp);
        return -1;
    }
    ilock(ip);

    if(ip->nlink < 1)
        panic("unlink: nlink < 1");

    // 如果是目录，检查是否为空（简化：暂不支持删除目录）
    if(ip->type == T_DIR) {
        iunlockput(ip);
        iunlockput(dp);
        return -1;
    }

    // 清除目录项
    memset_local(&de, 0, sizeof(de));
    if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei");

    // 减少链接计数
    ip->nlink--;
    iupdate(ip);

    iunlockput(dp);

    // 如果 nlink 为 0，释放 inode
    if(ip->nlink == 0) {
        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
    }

    iunlockput(ip);
    return 0;
}