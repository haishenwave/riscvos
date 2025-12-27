#include "riscv.h"
#include "proc.h"
#include "virtio.h" // 获取 VIRTIO0 宏

extern int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm);
extern pagetable_t create_pagetable(void);
extern char trampoline[]; //引用trampoline.S中的trampoline符号

/* 内核全局页表指针（由 kvminit 初始化） */
pagetable_t kernel_pagetable;

//辅助函数，批量映射一段连续区域
static void map_region(pagetable_t pt, uint64_t va, uint64_t pa, uint64_t size, int perm){
    uint64_t a, last;
    a = PGGROUNDDOWN(va);
    last = PGGROUNDUP(va + size - 1);
    for(; a<= last; a+=PGSIZE, pa+=PGSIZE){
        map_page(pt,a,pa,perm);
    }
}

/* 
 * kvminit - 构建内核页表
 * 这里做的是恒等映射，即虚拟地址 = 物理地址
 * 这样内核操作内存比较方便
 */
void kvminit(void){
    kernel_pagetable = create_pagetable();

    //映射内核数据段和自由内存（RAM区0x80200000 ～ END） -- 可读可写
    map_region(kernel_pagetable, 0x80200000, 0x80200000, PHYSTOP - 0x80200000, PTE_R|PTE_W|PTE_X); 
    //映射UART设备（串口0x1000000） -- 可读可写
    //如果不映射这个，开启分页以后无法使用printf
    map_page(kernel_pagetable, 0x10000000, 0x10000000, PTE_R|PTE_W);
    //映射内核代码段（文本段0x80000000 ～ 0x80200000） -- 可读可执行
    map_region(kernel_pagetable, 0x80000000, 0x80000000, 0x200000, PTE_R|PTE_X); //内核文本段
    //映射 TRAMPOLINE -- 可读可执行
    map_page(kernel_pagetable, TRAMPOLINE, (uint64_t)trampoline, PTE_R | PTE_X);
    // 映射 VIRTIO 磁盘设备 (RW)
    map_page(kernel_pagetable, VIRTIO0, VIRTIO0, PTE_R|PTE_W);
}

/* 
 * kvminithart - 激活页表
 * 把做好的页表地址写给硬件（satp寄存器）
 */
void kvminithart(void){
    //构造satp值
    //Mode=8（Sv39模式）
    //PPN = ——kernel_pagetable的物理页号
    uint64_t satp_val = ( 8ULL << 60) | (((uint64_t)kernel_pagetable) >> 12);

    //写入satp
    asm volatile("csrw satp, %0" : : "r"(satp_val));
    //刷新TLB，确保新页表立刻生效
    asm volatile("sfence.vma zero, zero");
}