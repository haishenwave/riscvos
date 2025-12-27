#include "riscv.h"
#include "virtio.h"
#include "fs.h"
#include "buf.h"
#include "printf.h"

// 磁盘接口函数声明
void virtio_disk_init(void);
void virtio_disk_rw(struct buf *b, int write);
void virtio_disk_intr(void);

// QEMU 的 virtio 描述符数量
#define NUM 8

static struct disk {
    // 内存中的页面，用于描述符环
    char pages[2*PGSIZE];
    
    // 描述符环的指针
    struct virtq_desc *desc;
    struct virtq_avail *avail;
    struct virtq_used *used;

    // 追踪正在进行的磁盘操作
    char free[NUM];  // 描述符是否空闲
    uint16_t used_idx; // 我们查看到了 used ring 的哪个位置

    // 正在等待磁盘操作完成的 buf 列表 (简化实现)
    struct buf *info[NUM]; 
    
    int init;
} disk;

void virtio_disk_init(void) {
    uint32_t status = 0;

    if(disk.init) return;

    // 1. 初始化步骤：Reset device
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS) = 0;

    // 2. Set ACKNOWLEDGE status bit
    status |= VIRTIO_CONFIG_S_ACKNOWLEDGE;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS) = status;

    // 3. Set DRIVER status bit
    status |= VIRTIO_CONFIG_S_DRIVER;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS) = status;

    // 4. Negotiate features (we don't strictly need any, so 0 is fine)
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_DEVICE_FEATURES) = 0;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_DRIVER_FEATURES) = 0; // we want nothing special
    
    // 5. Set FEATURES_OK status bit
    status |= VIRTIO_CONFIG_S_FEATURES_OK;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS) = status;

    // 6. Re-read status to ensure features accepted
    status = *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS);
    if(!(status & VIRTIO_CONFIG_S_FEATURES_OK))
        panic("virtio disk FEATURES_OK unset");

    // 7. Initialize queue 0
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_SEL) = 0;
    
    // Ensure queue is not ready yet
    if(*(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_READY))
        panic("virtio disk should not be ready");

    uint32_t max = *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_NUM_MAX);
    if(max == 0) panic("virtio disk has no queue 0");
    if(max < NUM) panic("virtio disk max queue too short");
    
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_NUM) = NUM;

    // Setup queue memory
    // page 0: descriptors + avail ring
    // page 1: used ring
    // 必须清零
    char *p = disk.pages;
    for(int i=0; i<2*PGSIZE; i++) p[i] = 0;
    
    disk.desc = (struct virtq_desc *) p;
    disk.avail = (struct virtq_avail *)(p + NUM*16);
    disk.used = (struct virtq_used *) (p + PGSIZE);

    // Tell device about queue addresses
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DESC_LOW) = (uint64_t)disk.desc;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DESC_HIGH) = (uint64_t)disk.desc >> 32;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DRIVER_LOW) = (uint64_t)disk.avail;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DRIVER_HIGH) = (uint64_t)disk.avail >> 32;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DEVICE_LOW) = (uint64_t)disk.used;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_DEVICE_HIGH) = (uint64_t)disk.used >> 32;

    // Queue is ready
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_READY) = 0x1;

    // 8. Set DRIVER_OK status bit
    status |= VIRTIO_CONFIG_S_DRIVER_OK;
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_STATUS) = status;

    disk.init = 1;
    for(int i = 0; i < NUM; i++) disk.free[i] = 1;
    
    printf("virtio_disk_init: disk initialized\n");
}

// 分配三个描述符
static int alloc3_desc(int *idx) {
    for(int i = 0; i < NUM; i++){
        if(disk.free[i]){
            disk.free[i] = 0;
            disk.free[(i+1)%NUM] = 0;
            disk.free[(i+2)%NUM] = 0;
            *idx = i;
            return 0;
        }
    }
    return -1;
}

static void free_chain(int i) {
    disk.free[i] = 1;
    disk.free[(i+1)%NUM] = 1;
    disk.free[(i+2)%NUM] = 1;
}

// 读写磁盘
// b->data 是数据缓冲区
// b->blockno 是块号
// write: 0=读, 1=写
void virtio_disk_rw(struct buf *b, int write) {
    uint64_t sector = b->blockno * (BSIZE / 512);

    // virtio disk 请求头
    struct virtio_blk_req {
        uint32_t type;
        uint32_t reserved;
        uint64_t sector;
    };
    static struct virtio_blk_req buf0;
    
    // 状态字节
    static uint8_t status0;

    if(!disk.init) panic("virtio_disk_rw: not initialized");

    int idx[3];
    while(1){
        if(alloc3_desc(&idx[0]) == 0) {
            idx[1] = (idx[0] + 1) % NUM;
            idx[2] = (idx[0] + 2) % NUM;
            break;
        }
        // 如果没有空闲描述符，应该 sleep 等待
        // 这里暂时用死循环轮询简化
        // yield(); 
    }

    // 1. 请求头
    buf0.type = write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN;
    buf0.reserved = 0;
    buf0.sector = sector;

    disk.desc[idx[0]].addr = (uint64_t) &buf0;
    disk.desc[idx[0]].len = sizeof(buf0);
    disk.desc[idx[0]].flags = VRING_DESC_F_NEXT;
    disk.desc[idx[0]].next = idx[1];

    // 2. 数据
    disk.desc[idx[1]].addr = (uint64_t) b->data;
    disk.desc[idx[1]].len = BSIZE;
    if(write)
        disk.desc[idx[1]].flags = 0; // device reads data
    else
        disk.desc[idx[1]].flags = VRING_DESC_F_WRITE; // device writes data
    disk.desc[idx[1]].flags |= VRING_DESC_F_NEXT;
    disk.desc[idx[1]].next = idx[2];

    // 3. 状态
    disk.desc[idx[2]].addr = (uint64_t) &status0;
    disk.desc[idx[2]].len = 1;
    disk.desc[idx[2]].flags = VRING_DESC_F_WRITE; // device writes status
    disk.desc[idx[2]].next = 0;

    // 记录这个链属于哪个 buf
    b->disk = 1; // 标记正在进行磁盘操作
    disk.info[idx[0]] = b;

    // 把链表头放入 avail ring
    disk.avail->ring[disk.avail->idx % NUM] = idx[0];
    
    // 内存屏障，确保 CPU 写完内存后再更新 idx
    __sync_synchronize();

    disk.avail->idx += 1;
    
    __sync_synchronize();

    // 通知设备
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_QUEUE_NOTIFY) = 0;

    // 等待操作完成 (自旋等待，简化版，不支持中断)
    // 真正的 OS 这里应该 sleep，然后在 virtio_disk_intr 中 wakeup
    while(b->disk == 1) {
        virtio_disk_intr(); // 轮询检查是否完成
    }
}

// 检查磁盘中断/完成情况
void virtio_disk_intr(void) {
    // 读取中断状态，并 ACK
    *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_INTERRUPT_ACK) = 
        *(volatile uint32_t*)(VIRTIO0 + VIRTIO_MMIO_INTERRUPT_STATUS) & 0x3;
    
    __sync_synchronize();

    // 检查 used ring 是否有新完成的任务
    while(disk.used_idx != disk.used->idx){
        __sync_synchronize();
        int id = disk.used->ring[disk.used_idx % NUM].id;
        
        if(disk.info[id]) {
            struct buf *b = disk.info[id];
            b->disk = 0; // 磁盘操作完成
            disk.info[id] = 0;
        }

        free_chain(id);
        disk.used_idx += 1;
    }
}