#ifndef _FS_H
#define _FS_H

#include <stdint.h>

//==============================================================================
// 基本常量
//==============================================================================

#define ROOTDEV  0      // 根文件系统设备号
#define ROOTINO  1      // 根目录 inode 编号
#define FSSIZE   1000   // 文件系统总块数
#define FSMAGIC  0x10203040

//==============================================================================
// 磁盘布局
// [ boot | super | log | inode blocks | bitmap | data blocks ]
// [  0   |   1   | 2.. |     ...      |  ...   |     ...     ]
//==============================================================================

// 超级块
struct superblock {
    uint32_t magic;       // 魔数
    uint32_t size;        // 总块数
    uint32_t nblocks;     // 数据块数量
    uint32_t ninodes;     // inode 数量
    uint32_t nlog;        // 日志块数量
    uint32_t logstart;    // 日志区起始块号
    uint32_t inodestart;  // inode 区起始块号
    uint32_t bmapstart;   // 位图起始块号
};

//==============================================================================
// Inode
//==============================================================================

#define NDIRECT   12                          // 直接块数量
#define NINDIRECT (BSIZE / sizeof(uint32_t))  // 间接块数量
#define MAXFILE   (NDIRECT + NINDIRECT)       // 最大文件块数

// 磁盘上的 inode (Disk Inode)
struct dinode {
    short type;                   // 文件类型
    short major;                  // 主设备号
    short minor;                  // 次设备号
    short nlink;                  // 硬链接计数
    uint32_t size;                // 文件大小（字节）
    uint32_t addrs[NDIRECT + 1];  // 数据块地址
};

// 内存中的 inode
struct inode {
    uint32_t dev;                 // 设备号
    uint32_t inum;                // inode 号
    int ref;                      // 引用计数
    int valid;                    // 是否已从磁盘读取

    // 从磁盘拷贝的内容
    short type;
    short major;
    short minor;
    short nlink;
    uint32_t size;
    uint32_t addrs[NDIRECT + 1];
};

// 辅助宏
#define IPB        (BSIZE / sizeof(struct dinode))  // 每块 inode 数
#define IBLOCK(i, sb) ((i) / IPB + (sb).inodestart) // inode 所在块号

// 文件类型
#define T_DIR   1  // 目录
#define T_FILE  2  // 普通文件
#define T_DEV   3  // 设备

// inode 缓存
#define NINODE  50

//==============================================================================
// 位图
//==============================================================================

#define BPB         (BSIZE * 8)                   // 每块 bit 数
#define BBLOCK(b, sb) ((b) / BPB + (sb).bmapstart) // 块号对应的位图块

//==============================================================================
// 目录
//==============================================================================

#define DIRSIZ  14

struct dirent {
    unsigned short inum;
    char name[DIRSIZ];
};

//==============================================================================
// 文件描述符
//==============================================================================

#define NOFILE  16   // 每进程最大打开文件数
#define NFILE   100  // 系统最大打开文件数

#define FD_NONE   0
#define FD_INODE  1
#define FD_DEVICE 2

struct file {
    int type;             // FD_NONE, FD_INODE, FD_DEVICE
    int ref;              // 引用计数
    char readable;
    char writable;
    struct inode *ip;     // FD_INODE 时有效
    uint32_t off;         // 文件偏移
    short major;          // FD_DEVICE 时有效
};

// 打开文件标志
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#endif