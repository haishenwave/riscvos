#include "riscv.h"

/* 空闲链表节点，直接利用空闲页本身的空间存储下一个节点的地址 */
struct run {struct run *next; };
static struct run *freelist; /* 空闲链表的头指针 */

extern char end[]; /* 链接器定义的内核结束地址（内核可用内存起始于此之后） */

/* 
 * 释放物理页（free）
 * 把物理页pa插回到空闲链表头部
 */
void free_page(void *pa){
    struct run* r =(struct run*)pa;
    r ->next = freelist; //当前页指向旧的头
    freelist = r;        //头指针指向当前页
}

/* 
 *  初始化物理内存
 *  从内核结束地址（ end ）向上把每个页加入空闲链表，直到 物理内存上限（PHYSTOP）。
 *  使用 PGGROUNDUP 确保从页对齐地址开始。
 *  把每一页都free，这样就全放进了空闲链表。
 */
void pmm_init(void){
    uint64_t start = PGGROUNDUP((uint64_t)end);
    for(uint64_t pa=start; pa+PGSIZE <= PHYSTOP; pa+=PGSIZE){
        free_page((void*)pa);
    }
}

/* 
 * 分配物理页（alloc)
 * 从空闲链表头部摘下一个页返回
 */
void* alloc_page(void){
    if(!freelist) return 0; //没内存了
    struct run* r = freelist;
    freelist = r->next;
    return (void*)r;
}
