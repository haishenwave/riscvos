#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include <stdint.h>

#define BSIZE 1024
#define FSSIZE 1000
#define FSMAGIC 0x10203040
#define ROOTINO 1
#define NDIRECT 12
#define DIRSIZ 14
#define T_DIR 1
#define T_FILE 2

#define IPB (BSIZE / sizeof(struct dinode))
#define BPB (BSIZE * 8)

struct superblock {
    uint32_t magic;
    uint32_t size;
    uint32_t nblocks;
    uint32_t ninodes;
    uint32_t nlog;
    uint32_t logstart;
    uint32_t inodestart;
    uint32_t bmapstart;
};

struct dinode {
    short type;
    short major;
    short minor;
    short nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};

struct dirent {
    unsigned short inum;
    char name[DIRSIZ];
};

int fsfd;
struct superblock sb;
uint32_t freeinode = 1;
uint32_t freeblock;

// 位图缓存
char bitmap[FSSIZE / 8 + 1];

void wsect(uint32_t sec, void *buf) {
    if(lseek(fsfd, sec * BSIZE, SEEK_SET) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }
    if(write(fsfd, buf, BSIZE) != BSIZE) {
        perror("write");
        exit(1);
    }
}

void rsect(uint32_t sec, void *buf) {
    if(lseek(fsfd, sec * BSIZE, SEEK_SET) != sec * BSIZE) {
        perror("lseek");
        exit(1);
    }
    if(read(fsfd, buf, BSIZE) != BSIZE) {
        perror("read");
        exit(1);
    }
}

void winode(uint32_t inum, struct dinode *ip) {
    char buf[BSIZE];
    uint32_t bn = (inum / IPB) + sb.inodestart;
    rsect(bn, buf);
    struct dinode *dip = ((struct dinode*)buf) + (inum % IPB);
    *dip = *ip;
    wsect(bn, buf);
}

void rinode(uint32_t inum, struct dinode *ip) {
    char buf[BSIZE];
    uint32_t bn = (inum / IPB) + sb.inodestart;
    rsect(bn, buf);
    *ip = *((struct dinode*)buf + inum % IPB);
}

uint32_t ialloc(unsigned short type) {
    uint32_t inum = freeinode++;
    struct dinode din;
    memset(&din, 0, sizeof(din));
    din.type = type;
    din.nlink = 1;
    din.size = 0;
    winode(inum, &din);
    return inum;
}

// 标记块为已使用
void balloc_mark(uint32_t b) {
    bitmap[b / 8] |= (1 << (b % 8));
}

// 分配一个块并标记
uint32_t balloc(void) {
    uint32_t b = freeblock++;
    balloc_mark(b);
    return b;
}

// 向 inode 追加数据
void iappend(uint32_t inum, void *data, int n) {
    char buf[BSIZE];
    struct dinode din;
    rinode(inum, &din);

    uint32_t off = din.size;
    while(n > 0) {
        uint32_t bn = off / BSIZE;
        if(bn >= NDIRECT) {
            fprintf(stderr, "iappend: file too large\n");
            exit(1);
        }
        if(din.addrs[bn] == 0) {
            din.addrs[bn] = balloc();  // 使用 balloc 分配并标记
        }
        rsect(din.addrs[bn], buf);
        int m = BSIZE - off % BSIZE;
        if(m > n) m = n;
        memcpy(buf + off % BSIZE, data, m);
        wsect(din.addrs[bn], buf);
        off += m;
        data = (char*)data + m;
        n -= m;
    }
    din.size = off;
    winode(inum, &din);
}

// 写入位图到磁盘
void write_bitmap(void) {
    char buf[BSIZE];
    int nbitmap = (FSSIZE / BPB) + 1;
    
    for(int i = 0; i < nbitmap; i++) {
        memset(buf, 0, BSIZE);
        // 复制对应的位图数据
        int start = i * BSIZE;
        int end = start + BSIZE;
        if(end > (FSSIZE / 8 + 1)) end = FSSIZE / 8 + 1;
        for(int j = start; j < end; j++) {
            buf[j - start] = bitmap[j];
        }
        wsect(sb.bmapstart + i, buf);
    }
}

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "Usage: mkfs fs.img\n");
        exit(1);
    }

    fsfd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0666);
    if(fsfd < 0) {
        perror(argv[1]);
        exit(1);
    }

    // 初始化磁盘（全部填 0）
    char zero[BSIZE];
    memset(zero, 0, BSIZE);
    for(int i = 0; i < FSSIZE; i++) {
        if(write(fsfd, zero, BSIZE) != BSIZE) {
            perror("write init");
            exit(1);
        }
    }
    lseek(fsfd, 0, SEEK_SET);

    // 初始化位图缓存
    memset(bitmap, 0, sizeof(bitmap));

    // 规划磁盘布局
    sb.magic = FSMAGIC;
    sb.size = FSSIZE;
    sb.nlog = 30;
    sb.logstart = 2;
    
    int ninodes = 200;
    sb.ninodes = ninodes;
    sb.inodestart = sb.logstart + sb.nlog;
    
    int inodeblocks = (ninodes / IPB) + 1;
    sb.bmapstart = sb.inodestart + inodeblocks;
    
    int nbitmap = (FSSIZE / BPB) + 1;
    freeblock = sb.bmapstart + nbitmap;
    sb.nblocks = FSSIZE - freeblock;

    printf("mkfs: layout - log[%d-%d] inode[%d-%d] bitmap[%d-%d] data[%d-%d]\n",
           sb.logstart, sb.logstart + sb.nlog - 1,
           sb.inodestart, sb.inodestart + inodeblocks - 1,
           sb.bmapstart, sb.bmapstart + nbitmap - 1,
           freeblock, FSSIZE - 1);

    // 标记元数据块为已使用（0 到 freeblock-1）
    for(uint32_t i = 0; i < freeblock; i++) {
        balloc_mark(i);
    }

    // 写入超级块
    char buf[BSIZE];
    memset(buf, 0, BSIZE);
    memcpy(buf, &sb, sizeof(sb));
    wsect(1, buf);

    // 创建根目录
    uint32_t rootino = ialloc(T_DIR);
    printf("mkfs: root inode is %d\n", rootino);

    // 添加 "." 和 ".." 目录项
    struct dirent de;
    memset(&de, 0, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, ".");
    iappend(rootino, &de, sizeof(de));

    memset(&de, 0, sizeof(de));
    de.inum = rootino;
    strcpy(de.name, "..");
    iappend(rootino, &de, sizeof(de));

    // 写入位图
    write_bitmap();

    close(fsfd);
    printf("mkfs: fs.img created successfully, first free block = %d\n", freeblock);
    return 0;
}