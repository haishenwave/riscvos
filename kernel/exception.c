#include "riscv.h"
#include "printf.h"

//异常分发
void handle_exception(void){
    uint64_t scause = r_scause(); //读取scause寄存器，判断异常类型
    uint64_t sepc = r_sepc();     //读取sepc寄存器，获取异常发生时的指令地址
    uint64_t stval = r_stval();   //读取stval寄存器，获取异常相关的附加信息

    printf("[exception] scause=0x%x, sepc=0x%x, stval=0x%x\n", scause, sepc, stval);
    
    //根据scause的值进行异常类型判断和处理
    switch(scause){
        case 8: //环境调用 from U-mode
            printf("[exception] Environment call from U-mode at sepc=0x%x\n", sepc);
            break;
        case 9: //环境调用 from S-mode
            printf("[exception] Environment call from S-mode at sepc=0x%x\n", sepc);
            break;
        case 12: //指令页错误
            printf("[exception] Instruction page fault at va=0x%x\n", stval);
            break;
        case 13: //加载页错误
            printf("[exception] Load page fault at va=0x%x\n", stval);
            break;
        case 15: //存储/AMO页错误
            printf("[exception] Store/AMO page fault at va=0x%x\n", stval);
            break;
        default:
            printf("[exception] Unknown exception scause=0x%x at sepc=0x%x\n", scause, sepc);
            break;
    }
    //死循环（遇到错误只能死机处理）
    for(;;)asm volatile("wfi"); 
}