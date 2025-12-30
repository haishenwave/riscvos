#include "riscv.h"
#include "fs.h"
#include "printf.h"
#include "proc.h"

// 外部函数声明
extern struct proc* myproc(void);
extern int argint(int, int*);
extern int argaddr(int, uint64_t*);
extern int argstr(int, char*, int);
extern int copyin(pagetable_t, char*, uint64_t, uint64_t);
extern int copyout(pagetable_t, uint64_t, char*, uint64_t);
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
extern void iupdate(struct inode*);
extern struct inode* dirlookup(struct inode*, char*, uint32_t*);
extern int writei(struct inode*, char*, uint32_t, uint32_t);

// 引入日志事务控制函数
extern void begin_op(void);
extern void end_op(void);

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
// 增加了事务保护，因为 O_CREATE 会修改文件系统
int sys_open(void) {
    char path[128];
    char name[DIRSIZ];
    int omode;
    struct inode *ip;
    struct file *f;
    int fd;

    // 开启事务
    begin_op();

    if(argstr(0, path, 128) < 0 || argint(1, &omode) < 0) {
        end_op();
        return -1;
    }

    if(omode & O_CREATE) {
        // 创建文件逻辑
        ip = nameiparent(path, name);
        if(ip == 0) {
            end_op();
            return -1;
        }
        ilock(ip);
        
        struct inode *dp = ip;
        ip = dirlookup(dp, name, 0);
        if(ip != 0) {
            // 文件已存在
            iunlockput(dp);
            ilock(ip);
            if(ip->type == T_DIR && (omode & O_RDWR || omode & O_WRONLY)){
                 iunlockput(ip);
                 end_op();
                 return -1;
            }
        } else {
            // 创建新文件：分配 inode
            ip = ialloc(dp->dev, T_FILE);
            if(ip == 0) {
                iunlockput(dp);
                end_op();
                return -1;
            }
            ilock(ip);
            ip->nlink = 1;
            iupdate(ip); // 更新 inode 到磁盘
            
            // 将新文件链接到父目录
            if(dirlink(dp, name, ip->inum) < 0) {
                iunlockput(ip);
                iunlockput(dp);
                end_op();
                return -1;
            }
            iunlockput(dp);
        }
    } else {
        // 仅打开文件
        ip = namei(path);
        if(ip == 0) {
            end_op();
            return -1;
        }
        ilock(ip);
        if(ip->type == T_DIR && omode != O_RDONLY){
            iunlockput(ip);
            end_op();
            return -1;
        }
    }

    // 分配文件结构体
    f = filealloc();
    if(f == 0) {
        iunlockput(ip);
        end_op();
        return -1;
    }

    // 分配 FD
    fd = fdalloc(f);
    if(fd < 0) {
        fileclose(f);
        iunlockput(ip);
        end_op();
        return -1;
    }

    f->type = FD_INODE;
    f->ip = ip;
    f->off = 0;
    f->readable = !(omode & O_WRONLY);
    f->writable = (omode & O_WRONLY) || (omode & O_RDWR);

    iunlock(ip);
    
    // 事务结束
    end_op();
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

// sys_fread: 读取文件
// 读取不修改磁盘结构，不需要 begin_op/end_op (除非涉及 access time 更新，这里简化略过)
int sys_fread(void) {
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

// sys_fwrite: 写入文件
// 关键：因为写入可能很大，不能把整个循环放在一个事务里（会爆日志区）。
// 必须分块进行事务保护。
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
        
        // --- 事务开始 ---
        begin_op();
        ilock(f->ip); // 锁定 inode

        if(copyin(p->pagetable, buf, addr, m) < 0) {
            iunlock(f->ip);
            end_op();
            return -1;
        }
        
        // 写入数据 (内部会调用 log_write)
        int r = filewrite(f, buf, m); 
        
        iunlock(f->ip);
        end_op();
        // --- 事务结束 ---

        if(r <= 0)
            break;
        total += r;
        addr += r;
        n -= r;
    }
    return total;
}

// sys_unlink: 删除文件
// 修改目录项和 inode，必须全过程保护
int sys_unlink(void) {
    char path[128];
    char name[DIRSIZ];
    struct inode *ip, *dp;
    struct dirent de;
    uint32_t off;

    // 开启事务
    begin_op();

    if(argstr(0, path, 128) < 0) {
        end_op();
        return -1;
    }

    dp = nameiparent(path, name);
    if(dp == 0) {
        end_op();
        return -1;
    }

    ilock(dp);

    // 不能删除 "." 和 ".."
    if(name[0] == '.' && (name[1] == 0 || (name[1] == '.' && name[2] == 0))) {
        iunlockput(dp);
        end_op();
        return -1;
    }

    ip = dirlookup(dp, name, &off);
    if(ip == 0) {
        iunlockput(dp);
        end_op();
        return -1;
    }
    ilock(ip);

    if(ip->nlink < 1)
        panic("unlink: nlink < 1");

    if(ip->type == T_DIR) {
        // 简化：不允许 unlink 目录
        iunlockput(ip);
        iunlockput(dp);
        end_op();
        return -1;
    }

    // 清除目录项
    memset_local(&de, 0, sizeof(de));
    if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("unlink: writei");

    // 减少文件的链接数
    ip->nlink--;
    iupdate(ip); // 写回磁盘

    iunlockput(dp);

    // 如果 nlink 为 0，释放 inode 和数据块
    if(ip->nlink == 0) {
        itrunc(ip);
        ip->type = 0;
        iupdate(ip);
    }

    iunlockput(ip);
    
    // 提交事务
    end_op();
    return 0;
}