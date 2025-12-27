#include "riscv.h"
#include "fs.h"
#include "buf.h"
#include "printf.h"
#include "proc.h"
#include "klog.h"

extern struct buf* bread(uint32_t, uint32_t);
extern void brelse(struct buf*);
extern void bwrite(struct buf*);
extern void log_write(struct buf*);
extern void panic(const char*);

// 全局超级块
struct superblock sb;

// inode 缓存
struct {
    struct inode inode[NINODE];
} icache;

// 读取超级块
void readsb(int dev, struct superblock *sb) {
    struct buf *bp = bread(dev, 1);
    char *p = (char*)sb;
    char *data = (char*)bp->data;
    for(int i = 0; i < (int)sizeof(struct superblock); i++)
        p[i] = data[i];
    brelse(bp);
}

// 初始化文件系统
void fsinit(int dev) {
    readsb(dev, &sb);
    
    if(sb.magic != FSMAGIC) {
        LOG_W("Invalid magic number %x (expected %x)\n", sb.magic, FSMAGIC);
    } else {
        LOG_I("File system mounted: %d blocks, %d inodes\n", sb.size, sb.ninodes);
        //LOG_I("  logstart=%d, inodestart=%d, bmapstart=%d\n", 
        //      sb.logstart, sb.inodestart, sb.bmapstart);
    }
    

    // 初始化 inode 缓存
    for(int i = 0; i < NINODE; i++) {
        icache.inode[i].ref = 0;
        icache.inode[i].valid = 0;
    }
}

// 分配一个磁盘块
static uint32_t balloc(uint32_t dev) {
    int b, bi, m;
    struct buf *bp;

    for(b = 0; b < (int)sb.size; b += BPB) {
        bp = bread(dev, BBLOCK(b, sb));
        for(bi = 0; bi < BPB && b + bi < (int)sb.size; bi++) {
            m = 1 << (bi % 8);
            if((bp->data[bi/8] & m) == 0) {  // 块空闲？
                bp->data[bi/8] |= m;  // 标记为已使用
                log_write(bp);
                brelse(bp);
                
                // 清零新分配的块 
                // 这保证了新文件不会包含之前的垃圾数据
                bp = bread(dev, b + bi);
                // 手动清零，避免依赖 memset
                for(int i = 0; i < BSIZE; i++) {
                    bp->data[i] = 0;
                }
                log_write(bp);
                brelse(bp);
                
                return b + bi;
            }
        }
        brelse(bp);
    }
    panic("balloc: out of blocks");
    return 0;
}

// 释放一个磁盘块
void bfree(int dev, uint32_t b) {
    struct buf *bp;
    int bi, m;

    bp = bread(dev, BBLOCK(b, sb));
    bi = b % BPB;
    m = 1 << (bi % 8);
    if((bp->data[bi/8] & m) == 0)
        panic("freeing free block");
    bp->data[bi/8] &= ~m;
    log_write(bp);
    brelse(bp);
}

// 获取 inode (不读取磁盘内容)
struct inode* iget(uint32_t dev, uint32_t inum) {
    struct inode *ip, *empty = 0;

    // 查找缓存
    for(ip = &icache.inode[0]; ip < &icache.inode[NINODE]; ip++) {
        if(ip->ref > 0 && ip->dev == dev && ip->inum == inum) {
            ip->ref++;
            return ip;
        }
        if(empty == 0 && ip->ref == 0)
            empty = ip;
    }

    // 分配新的缓存槽
    if(empty == 0)
        panic("iget: no inodes");

    ip = empty;
    ip->dev = dev;
    ip->inum = inum;
    ip->ref = 1;
    ip->valid = 0;
    return ip;
}

// 从磁盘读取 inode 内容
void ilock(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    if(ip == 0 || ip->ref < 1)
        panic("ilock");

    if(ip->valid == 0) {
        bp = bread(ip->dev, IBLOCK(ip->inum, sb));
        dip = (struct dinode*)bp->data + ip->inum % IPB;
        ip->type = dip->type;
        ip->major = dip->major;
        ip->minor = dip->minor;
        ip->nlink = dip->nlink;
        ip->size = dip->size;
        for(int i = 0; i < NDIRECT + 1; i++)
            ip->addrs[i] = dip->addrs[i];
        brelse(bp);
        ip->valid = 1;
        if(ip->type == 0)
            panic("ilock: no type");
    }
}

// 解锁 inode (目前只是占位)
void iunlock(struct inode *ip) {
    if(ip == 0 || ip->ref < 1)
        panic("iunlock");
}

// 释放 inode 引用
void iput(struct inode *ip) {
    if(ip->ref == 1 && ip->valid && ip->nlink == 0) {
        // inode 没有链接了，释放它
        // itrunc(ip);  // 释放数据块 (暂不实现)
        ip->type = 0;
        // iupdate(ip); // 写回磁盘 (暂不实现)
        ip->valid = 0;
    }
    ip->ref--;
}

// 分配一个新的 inode
struct inode* ialloc(uint32_t dev, short type) {
    int inum;
    struct buf *bp;
    struct dinode *dip;

    for(inum = 1; inum < (int)sb.ninodes; inum++) {
        bp = bread(dev, IBLOCK(inum, sb));
        dip = (struct dinode*)bp->data + inum % IPB;
        if(dip->type == 0) {  // 空闲 inode
            // 清零
            char *p = (char*)dip;
            for(int i = 0; i < (int)sizeof(*dip); i++)
                p[i] = 0;
            dip->type = type;
            log_write(bp);
            brelse(bp);
            return iget(dev, inum);
        }
        brelse(bp);
    }
    panic("ialloc: no inodes");
    return 0;
}

// 修改 bmap 原型：增加 create 参数
// create=1: 不存在则分配
// create=0: 不存在则返回 0
static uint32_t bmap(struct inode *ip, uint32_t bn, int create) {
    uint32_t addr, *a;
    struct buf *bp;

    if(bn < NDIRECT) {
        if((addr = ip->addrs[bn]) == 0) {
            if(!create) return 0; // 如果不是创建模式，直接返回0
            ip->addrs[bn] = addr = balloc(ip->dev);
        }
        return addr;
    }
    bn -= NDIRECT;

    if(bn < NINDIRECT) {
        // 间接索引块
        if((addr = ip->addrs[NDIRECT]) == 0) {
            if(!create) return 0;
            ip->addrs[NDIRECT] = addr = balloc(ip->dev);
        }
        
        bp = bread(ip->dev, addr);
        a = (uint32_t*)bp->data;
        if((addr = a[bn]) == 0) {
            if(create) {
                a[bn] = addr = balloc(ip->dev);
                log_write(bp);
            }
        }
        brelse(bp);
        return addr;
    }

    panic("bmap: out of range");
    return 0;
}

// 修改 readi：调用 bmap 时 create=0
int readi(struct inode *ip, char *dst, uint32_t off, uint32_t n) {
    uint32_t tot, m;
    struct buf *bp;

    if(off > ip->size || off + n < off)
        return -1;
    if(off + n > ip->size)
        n = ip->size - off;

    for(tot = 0; tot < n; tot += m, off += m, dst += m) {
        // 关键修改：create=0
        uint32_t addr = bmap(ip, off / BSIZE, 0);
        
        m = BSIZE - off % BSIZE;
        if(m > n - tot)
            m = n - tot;

        if(addr == 0) {
            // 如果是稀疏文件（空洞），填充0
            // 这是标准文件系统行为，且不会分配磁盘块
            for(int i = 0; i < m; i++) dst[i] = 0;
        } else {
            bp = bread(ip->dev, addr);
            char *src = (char*)bp->data + off % BSIZE;
            for(uint32_t i = 0; i < m; i++)
                dst[i] = src[i];
            brelse(bp);
        }
    }
    return n;
}

// 修改 writei：调用 bmap 时 create=1
int writei(struct inode *ip, char *src, uint32_t off, uint32_t n) {
    uint32_t tot, m;
    struct buf *bp;

    if(off > ip->size || off + n < off)
        return -1;

    for(tot = 0; tot < n; tot += m, off += m, src += m) {
        // 关键修改：create=1
        uint32_t addr = bmap(ip, off / BSIZE, 1);
        if(addr == 0) return -1; // 分配失败

        bp = bread(ip->dev, addr);
        m = BSIZE - off % BSIZE;
        if(m > n - tot)
            m = n - tot;
        char *dst = (char*)bp->data + off % BSIZE;
        for(uint32_t i = 0; i < m; i++)
            dst[i] = src[i];
        log_write(bp);
        brelse(bp);
    }

    if(n > 0) {
        if(off > ip->size)
            ip->size = off;
        // 确保元数据（包括新分配的块号）写回磁盘
        iupdate(ip);
    }
    return n;
}
// 在目录中查找文件
struct inode* dirlookup(struct inode *dp, char *name, uint32_t *poff) {
    uint32_t off;
    struct dirent de;

    if(dp->type != T_DIR)
        panic("dirlookup not DIR");

    for(off = 0; off < dp->size; off += sizeof(de)) {
        if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            panic("dirlookup read");
        if(de.inum == 0)
            continue;

        // 比较文件名
        int match = 1;
        for(int i = 0; i < DIRSIZ; i++) {
            if(name[i] != de.name[i]) {
                match = 0;
                break;
            }
            if(name[i] == 0)
                break;
        }
        if(match) {
            if(poff)
                *poff = off;
            return iget(dp->dev, de.inum);
        }
    }
    return 0;
}

// 在目录中添加新条目
int dirlink(struct inode *dp, char *name, uint32_t inum) {
    int off;
    struct dirent de;
    struct inode *ip;

    // 检查是否已存在
    if((ip = dirlookup(dp, name, 0)) != 0) {
        iput(ip);
        return -1;
    }

    // 查找空闲条目
    for(off = 0; off < (int)dp->size; off += sizeof(de)) {
        if(readi(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
            panic("dirlink read");
        if(de.inum == 0)
            break;
    }

    // 填充条目
    de.inum = inum;
    for(int i = 0; i < DIRSIZ; i++) {
        de.name[i] = (i < DIRSIZ && name[i]) ? name[i] : 0;
    }

    if(writei(dp, (char*)&de, off, sizeof(de)) != sizeof(de))
        panic("dirlink");

    return 0;
}

// 路径解析辅助函数
static char* skipelem(char *path, char *name) {
    while(*path == '/')
        path++;
    if(*path == 0)
        return 0;
    char *s = path;
    while(*path != '/' && *path != 0)
        path++;
    int len = path - s;
    if(len >= DIRSIZ) {
        for(int i = 0; i < DIRSIZ; i++)
            name[i] = s[i];
    } else {
        for(int i = 0; i < len; i++)
            name[i] = s[i];
        name[len] = 0;
    }
    while(*path == '/')
        path++;
    return path;
}

// 路径解析核心
static struct inode* namex(char *path, int nameiparent, char *name) {
    struct inode *ip, *next;

    if(*path == '/')
        ip = iget(ROOTDEV, ROOTINO);
    else
        ip = iget(ROOTDEV, ROOTINO); // 暂时不支持相对路径

    while((path = skipelem(path, name)) != 0) {
        ilock(ip);
        if(ip->type != T_DIR) {
            iunlock(ip);
            iput(ip);
            return 0;
        }
        if(nameiparent && *path == '\0') {
            iunlock(ip);
            return ip;
        }
        if((next = dirlookup(ip, name, 0)) == 0) {
            iunlock(ip);
            iput(ip);
            return 0;
        }
        iunlock(ip);
        iput(ip);
        ip = next;
    }
    if(nameiparent) {
        iput(ip);
        return 0;
    }
    return ip;
}

struct inode* namei(char *path) {
    char name[DIRSIZ];
    return namex(path, 0, name);
}

struct inode* nameiparent(char *path, char *name) {
    return namex(path, 1, name);
}

void iupdate(struct inode *ip) {
    struct buf *bp;
    struct dinode *dip;

    bp = bread(ip->dev, IBLOCK(ip->inum, sb));
    dip = (struct dinode*)bp->data + ip->inum % IPB;
    
    dip->type = ip->type;
    dip->major = ip->major;
    dip->minor = ip->minor;
    dip->nlink = ip->nlink;
    dip->size = ip->size;
    for(int i = 0; i < NDIRECT + 1; i++)
        dip->addrs[i] = ip->addrs[i];
    
    log_write(bp);
    brelse(bp);
}

// 解锁并释放 inode
void iunlockput(struct inode *ip) {
    iunlock(ip);
    iput(ip);
}

// 清空 inode 数据块（截断文件）
void itrunc(struct inode *ip) {
    struct buf *bp;
    uint32_t *a;

    // 释放直接块
    for(int i = 0; i < NDIRECT; i++) {
        if(ip->addrs[i]) {
            bfree(ip->dev, ip->addrs[i]);
            ip->addrs[i] = 0;
        }
    }

    // 释放间接块
    if(ip->addrs[NDIRECT]) {
        bp = bread(ip->dev, ip->addrs[NDIRECT]);
        a = (uint32_t*)bp->data;
        for(int j = 0; j < NINDIRECT; j++) {
            if(a[j])
                bfree(ip->dev, a[j]);
        }
        brelse(bp);
        bfree(ip->dev, ip->addrs[NDIRECT]);
        ip->addrs[NDIRECT] = 0;
    }

    ip->size = 0;
    iupdate(ip);
}