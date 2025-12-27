//定义页表格式、CSR寄存器操作宏及内存管理接口
#pragma once
#include <stdint.h>

/* 基本常量定义 */
#define PGSIZE 4096              //一个物理页大小是4096字节（4KB）
#define PHYSTOP (0x88000000UL)   //物理内存的结束地址，假设内存只有128MB
#define BSIZE 1024              //块大小（文件系统相关）    

/* 
 *  页表项（PTE）的标志位
 *  RISC-V 的页表项低10位用于存储权限和状态
 */
#define PTE_V (1L<<0) /* Valid： 该页表项是否有效。0表示该页不存在，访问会报错 */
#define PTE_R (1L<<1) /* Read：是否可读 */
#define PTE_W (1L<<2) /* Write：是否可写 */
#define PTE_X (1L<<3) /* Exec：是否可执行（代码段需要这个） */
#define PTE_U (1L<<4) /* User：是否允许用户态访问（内核态通常设为0） */

/*
 *类型定义
 */
typedef uint64_t pte_t;       /* 页表项本身为 64 位的整数 */
typedef uint64_t* pagetable_t;/* 页表是指向pte_t数组的指针（一个页表含512项） */

/*
 *  地址转换宏
 *  RISC-V 的物理地址在PTE中是存储在第10-53位的，需要移位操作提取
 */
/* 从 PTE 中取出物理地址（去掉低 10 位标志并左移12位恢复成物理地址） */
#define PTE_PA(pte)  (((pte) >> 10) << 12)
/* 把物理地址打包成 PTE 中存储的格式（右移12位去掉页内偏移，再左移10位放到PTE中间） */
#define PA_PTE(pa)   ((((uint64_t)(pa)) >> 12 ) << 10)

/* 
 * 虚拟地址索引计算
 * S39 分页模式下，虚拟地址被分为：9位（L2）+9位（L1）+9位（L0）+12位（页内偏移）
 */
#define VPN_SHIFT(level) (12 + 9*(level)) //计算该级索引在虚拟地址中的偏移量
#define VPN_MASK(va, level) (((va) >> VPN_SHIFT(level)) & 0x1FF) //提取该级的9位索引值（0-511）

/* 
 *内存对齐工具
 */
//向上取整到页边界
#define PGGROUNDUP(sz)  (((sz)+PGSIZE-1) & ~(PGSIZE-1))
//向下取整到页边界
#define PGGROUNDDOWN(a) ((a) & ~(PGSIZE-1)) 

/* 
 * CSR（Control and Status Register） 读写操作
 * 使用内联汇编直接操作硬件寄存器
 */
//写 stvec（中断向量表基址）：告诉CPU发生中断跳转到那里
static inline void w_stvec(uint64_t x){asm volatile("csrw stvec, %0" : : "r"(x));}
//读 stvec
static inline uint64_t r_stvec(void){uint64_t x;asm volatile("csrr %0, stvec" : "=r"(x));return x;}
//读 scause（中断原因）：看看为什么发生了中断
static inline uint64_t r_scause(void){uint64_t x;asm volatile("csrr %0, scause" : "=r"(x));return x;}
//读 sepc（异常程序计数器）：中断发生时，CPU正在执行哪条指令的地址
static inline uint64_t r_sepc(void){uint64_t x;asm volatile("csrr %0, sepc" : "=r"(x));return x;}
//读 sstatus （状态寄存器）：包含中断开启位等重要状态
static inline uint64_t r_sstatus(void){uint64_t x;asm volatile("csrr %0, sstatus" : "=r"(x));return x;}
//写 sstatus
static inline void w_sstatus(uint64_t x){asm volatile("csrw sstatus, %0" : : "r"(x));}
//读 stval （异常值）：发生缺页时，这里存的是那个出错的虚拟地址
static inline uint64_t r_stval(void){uint64_t x;asm volatile("csrr %0, stval" : "=r"(x));return x;}
//读 satp （页表基址寄存器）：当前使用的页表物理地址
static inline uint64_t r_satp(void){uint64_t x;asm volatile("csrr %0, satp" : "=r"(x));return x;}
//写 sepc （设置下次返回用户态时的pc）
static inline void w_sepc(uint64_t x){asm volatile("csrw sepc, %0" : : "r"(x));}
//写 sscratch （内核栈指针寄存器）
static inline void w_sscratch(uint64_t x){asm volatile("csrw sscratch, %0" : : "r"(x));}
//开启中断 （设置sstatus的第1位SIE）
static inline void intr_on(void){asm volatile("csrs sstatus, 0x2");}
//关闭中断 （清除sstatus的第1位SIE）
static inline void intr_off(void){asm volatile("csrc sstatus, 0x2");}

//外部函数声明
void pmm_init(void);
void* alloc_page(void);
void free_page(void *pa);
pte_t* walk_lookup(pagetable_t pagetable, uint64_t va);
pte_t* walk_create(pagetable_t pagetable, uint64_t va);
int map_page(pagetable_t pagetable, uint64_t va, uint64_t pa, int perm);
