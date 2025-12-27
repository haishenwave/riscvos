#include "riscv.h"
#include "printf.h"
#include "klog.h"
#include <stdarg.h>

// 日志级别名称
static const char *level_names[] = {
    "DEBUG",
    "INFO ",
    "WARN ",
    "ERROR"
};

// 日志级别颜色（ANSI）
static const char *level_colors[] = {
    "\x1b[36m",  // DEBUG: 青色
    "\x1b[32m",  // INFO:  绿色
    "\x1b[33m",  // WARN:  黄色
    "\x1b[31m"   // ERROR: 红色
};
static const char *color_reset = "\x1b[0m";

// 环形日志缓冲区
static struct {
    char buffer[LOG_BUFSIZE];
    int head;           // 写入位置
    int tail;           // 读取位置
    int count;          // 当前条目数
    int total_logs;     // 总日志数
    int dropped;        // 丢弃的日志数
    int enabled;        // 是否启用
    int use_color;      // 是否使用颜色
} log_state;

// 获取系统时间（简化版，使用 tick 计数）
extern uint64_t get_time(void);

static uint64_t get_timestamp(void) {
    return get_time() / 10000;  // 简化的时间戳
}

// 初始化日志系统
void log_init(void) {
    log_state.head = 0;
    log_state.tail = 0;
    log_state.count = 0;
    log_state.total_logs = 0;
    log_state.dropped = 0;
    log_state.enabled = 1;
    log_state.use_color = 1;
    
    LOG_I("Kernel log system initialized\n");
}

// 向缓冲区写入一个字符
static void log_putc(char c) {
    log_state.buffer[log_state.head] = c;
    log_state.head = (log_state.head + 1) % LOG_BUFSIZE;
    
    // 如果缓冲区满了，覆盖最旧的数据
    if(log_state.head == log_state.tail) {
        log_state.tail = (log_state.tail + 1) % LOG_BUFSIZE;
        log_state.dropped++;
    }
}

// 向缓冲区写入字符串
static void log_puts(const char *s) {
    while(*s) {
        log_putc(*s++);
    }
}

// 数字转字符串（用于时间戳）
static void log_print_num(uint64_t n) {
    char buf[20];
    int i = 0;
    
    if(n == 0) {
        log_putc('0');
        return;
    }
    
    while(n > 0) {
        buf[i++] = '0' + (n % 10);
        n /= 10;
    }
    
    while(i > 0) {
        log_putc(buf[--i]);
    }
}

// 核心日志函数
void klog(int level, const char *fmt, ...) {
    if(!log_state.enabled)
        return;
    
    if(level < LOG_LEVEL)
        return;
    
    log_state.total_logs++;
    
    // 获取时间戳
    uint64_t ts = get_timestamp();
    
    // 格式化输出到控制台和缓冲区
    // [时间戳] [级别] 消息
    
    // 输出到控制台
    if(log_state.use_color) {
        printf("%s", level_colors[level]);
    }
    printf("[%d] [%s] ", (int)ts, level_names[level]);
    if(log_state.use_color) {
        printf("%s", color_reset);
    }
    
    // 写入缓冲区
    log_putc('[');
    log_print_num(ts);
    log_puts("] [");
    log_puts(level_names[level]);
    log_puts("] ");
    
    // 处理可变参数
    va_list ap;
    va_start(ap, fmt);
    
    for(; *fmt; fmt++) {
        if(*fmt != '%') {
            printf("%c", *fmt);
            log_putc(*fmt);
            continue;
        }
        
        char f = *++fmt;
        if(!f) break;
        
        switch(f) {
        case 'd': {
            int val = va_arg(ap, int);
            printf("%d", val);
            // 简化：数字写入缓冲区
            if(val < 0) {
                log_putc('-');
                val = -val;
            }
            char buf[16];
            int i = 0;
            if(val == 0) buf[i++] = '0';
            while(val > 0) {
                buf[i++] = '0' + (val % 10);
                val /= 10;
            }
            while(i > 0) log_putc(buf[--i]);
            break;
        }
        case 'x': {
            unsigned int val = va_arg(ap, unsigned int);
            printf("%x", val);
            // 简化处理
            break;
        }
        case 's': {
            const char *s = va_arg(ap, const char*);
            if(!s) s = "(null)";
            printf("%s", s);
            log_puts(s);
            break;
        }
        case 'c': {
            char c = (char)va_arg(ap, int);
            printf("%c", c);
            log_putc(c);
            break;
        }
        case '%':
            printf("%%");
            log_putc('%');
            break;
        default:
            printf("%%%c", f);
            log_putc('%');
            log_putc(f);
            break;
        }
    }
    
    va_end(ap);
    log_state.count++;
}

// 显示日志缓冲区内容
void log_dump(void) {
    printf("\n=== Log Buffer Dump ===\n");
    
    int pos = log_state.tail;
    while(pos != log_state.head) {
        char c = log_state.buffer[pos];
        printf("%c", c);
        pos = (pos + 1) % LOG_BUFSIZE;
    }
    
    printf("\n=== End of Log ===\n");
}

// 清空日志缓冲区
void log_clear(void) {
    log_state.head = 0;
    log_state.tail = 0;
    log_state.count = 0;
    LOG_I("Log buffer cleared\n");
}

// 获取日志统计信息
void log_stats(void) {
    printf("\n=== Log Statistics ===\n");
    printf("Total logs:   %d\n", log_state.total_logs);
    printf("Buffer count: %d\n", log_state.count);
    printf("Dropped:      %d\n", log_state.dropped);
    printf("Buffer usage: %d / %d bytes\n", 
           (log_state.head - log_state.tail + LOG_BUFSIZE) % LOG_BUFSIZE,
           LOG_BUFSIZE);
    printf("======================\n");
}