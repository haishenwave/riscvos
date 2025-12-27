#include "riscv.h"
#include "printf.h"
/* 外部页分配/释放函数 */
extern void* alloc_page(void);
extern void  free_page(void*);

/* 
 * walk_create ：找路并修路
 * 给定虚拟地址 va，找到对应的第0级页表项PTE的指针
 * 如果中间的页表不存在，就顺手申请新页创建出来
 */
pte_t* walk_create(pagetable_t pt, uint64_t va){
    /* 从最高级（level=2）向下遍历到 level>0（不包含第 0 级的最终 PTE） */
    for(int level=2; level>0; level--){
        int idx = VPN_MASK(va, level); /* 计算在该级的索引 */
        if(pt[idx] & PTE_V){
            /* 如果该项有效，取出下一级页表的物理地址 */
            pt = (pagetable_t)PTE_PA(pt[idx]);
        } else {
            /* 不存在，分配一个新页作为下一级页表 */
            pagetable_t newpt = alloc_page();
            //新页表清零
            for(int i=0;i<512;i++) newpt[i]=0;
            /* 将新页表的地址填入当前的PTE，并标记有效 */
            pt[idx] = PA_PTE(newpt) | PTE_V;
            pt =newpt; /* 进入新建的子页表继续 */
        }
    }
    /* 返回第 0 级对应的 PTE 指针 */
    return &pt[VPN_MASK(va,0)];
}

/* 
 * walk_lookup - 查找虚拟地址 va 在页表 pt 中对应的第 0 级 PTE（不创建新页表）。
 * 找不到则返回 0。
 */
pte_t* walk_lookup(pagetable_t pt, uint64_t va){
    for(int level=2; level>0; level--){
        int idx = VPN_MASK(va, level);
        if(!(pt[idx] & PTE_V)) return 0; /* 中间级不存在映射，立即返回未映射 */
        pt = (pagetable_t)PTE_PA(pt[idx]);
    }
    return &pt[VPN_MASK(va,0)];
}

/* 
 * create_pagetable - 创建一个全新的空页表（根页表）
 * 返回新页表的基指针。
 */
pagetable_t create_pagetable(void){
    pagetable_t pt = alloc_page();
    for(int i=0;i<512;i++) pt[i]=0; /* 清空 512 个 PTE 条目 */
    return pt;
}

/* 
 * map_page - 建立映射
 * 把虚拟地址va，映射到物理地址pa，权限是perm
 */
int map_page(pagetable_t pt, uint64_t va, uint64_t pa, int perm){
    pte_t* pte = walk_create(pt, va); /* 找到va对应的PTE指针（自动建立中间映射） */
    if(!pte) return -1;
    *pte = PA_PTE(pa) | perm | PTE_V; /* 写入物理地址、权限及有效位 */
    return 0;
}

/* 
 * permstr - 将 PTE 权限位转换为可读的短字符串（用于调试打印）。
 * 返回静态缓冲区地址（非线程安全）。
 */
static const char* permstr(int pte){
    static char buf[8];
    buf[0] = (pte & PTE_V) ? 'v' : '-';
    buf[1] = (pte & PTE_R) ? 'r' : '-';
    buf[2] = (pte & PTE_W) ? 'w' : '-';
    buf[3] = (pte & PTE_X) ? 'x' : '-';
    buf[4] = (pte & PTE_U) ? 'u' : '-';
    buf[5] = 0;
    return buf;
}

/* 
 * dump_pagetable - 递归打印页表结构（调试用）。
 * pagetable: 当前级页表指针
 * level: 当前级别（2 = 最高级）
 * va_base: 当前页表覆盖的虚拟地址基址
 */
void dump_pagetable(pagetable_t pagetable, int level, uint64_t va_base){
    for(int i=0; i<512; i++){
        pte_t pte = pagetable[i];
        if(pte & PTE_V){
            uint64_t pa = PTE_PA(pte);
            uint64_t va = va_base | ((uint64_t)i << (12 + 9*level));

            /* 根据层级缩进，便于查看层次结构 */
            for(int j=0; j<(2-level)*2; j++) printf(" ");

            /* 打印层级、虚拟地址、物理地址和权限 */
            printf("L%d  VA:0x%x -> PA:0x%x %s\n",
                    level, va, pa, permstr(pte));
            
            /* 如果该 PTE 既没有 R/W/X 位，则视为指向子页表（中间节点），递归打印 */
            int flags = pte & (PTE_R|PTE_W|PTE_X);
            if(flags == 0 && level > 0){
                pagetable_t child = (pagetable_t)pa; /* 假定 pa 可直接作为虚拟地址访问 */
                dump_pagetable(child, level - 1, va);
            }
        }
    }
}

/* 
 * destroy_pagetable - 递归释放页表占用的页（未实现）。
 * TODO: 遍历每个 PTE，若为中间节点则递归释放子页表，最后 free_page(self)。
 */
void destroy_pagetable(pagetable_t pt){
    //free
}


/*
 * 从用户空间复制数据到内核
 * 必须检查 PTE_U 确保这是用户内存
 */
int copyin(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t len){
    uint64_t n, va0, pa0;
    pte_t *pte;

    while(len > 0){
        va0 = PGGROUNDDOWN(srcva);
        pte = walk_lookup(pagetable, va0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;
        pa0 = PTE_PA(*pte);
        n = PGSIZE - (srcva - va0);
        if(n > len) n = len;
        char *src = (char *)(pa0 + (srcva - va0));
        for(uint64_t i = 0; i < n; i++) dst[i] = src[i];
        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }
    return 0;
}

//从用户空间复制字符串
int copyinstr(pagetable_t pagetable, char *dst, uint64_t srcva, uint64_t max){
    uint64_t n, va0, pa0;
    int got_null = 0;
    pte_t *pte;

    while(got_null == 0 && max > 0){
        va0 = PGGROUNDDOWN(srcva);
        pte = walk_lookup(pagetable, va0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;
        pa0 = PTE_PA(*pte);
        n = PGSIZE - (srcva - va0);
        if(n > max) n = max;
        char *p = (char *)(pa0 + (srcva - va0));
        while(n > 0){
            if(*p == '\0'){
                *dst = '\0';
                got_null = 1;
                break;
            }
            else{
                *dst = *p;
            }
            --n;
            --max;
            p++;
            dst++;
        }
        srcva = va0 + PGSIZE;
    }
    if(got_null) return 0;
    return -1;
}

// 从内核复制数据到用户空间
int copyout(pagetable_t pagetable, uint64_t dstva, char *src, uint64_t len) {
    uint64_t n, va0, pa0;
    pte_t *pte;

    while(len > 0) {
        va0 = PGGROUNDDOWN(dstva);
        pte = walk_lookup(pagetable, va0);
        if(pte == 0 || (*pte & PTE_V) == 0 || (*pte & PTE_U) == 0)
            return -1;
        pa0 = PTE_PA(*pte);
        n = PGSIZE - (dstva - va0);
        if(n > len) n = len;
        char *dst = (char*)(pa0 + (dstva - va0));
        for(uint64_t i = 0; i < n; i++)
            dst[i] = src[i];
        len -= n;
        src += n;
        dstva = va0 + PGSIZE;
    }
    return 0;
}