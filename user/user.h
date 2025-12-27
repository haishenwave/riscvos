#ifndef _USER_H
#define _USER_H

// 系统调用声明
int fork(void);
void exit(int status) __attribute__((noreturn));
int wait(int *status);
int kill(int pid);
int getpid(void);
int sbrk(int n);
int write(int fd, const void *buf, int n);
int read(int fd, void *buf, int n);
int open(const char *path, int omode);
int close(int fd);
int uptime(void);
int un_link(const char *path);
int logdump(void);
int logstat(void);

// 用户库函数
int printf(const char *fmt, ...);
void puts(const char *s);
int strlen(const char *s);
void* memset(void *dst, int c, int n);

// === 添加文件打开标志 ===
#define O_RDONLY  0x000
#define O_WRONLY  0x001
#define O_RDWR    0x002
#define O_CREATE  0x200
#define O_TRUNC   0x400

#endif