#ifndef _KLOG_H
#define _KLOG_H

// 日志级别定义
#define LOG_DEBUG   0
#define LOG_INFO    1
#define LOG_WARN    2
#define LOG_ERROR   3

// 当前日志级别（低于此级别的日志不输出）
#define LOG_LEVEL   LOG_DEBUG

// 日志缓冲区大小
#define LOG_BUFSIZE 4096

// 日志条目最大长度
#define LOG_MAXLEN  256

// 日志系统初始化
void log_init(void);

// 核心日志函数
void klog(int level, const char *fmt, ...);

// 便捷宏
#define LOG_D(fmt, ...) klog(LOG_DEBUG, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) klog(LOG_INFO,  fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) klog(LOG_WARN,  fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) klog(LOG_ERROR, fmt, ##__VA_ARGS__)

// 显示日志缓冲区内容
void log_dump(void);

// 清空日志缓冲区
void log_clear(void);

// 获取日志统计信息
void log_stats(void);

#endif