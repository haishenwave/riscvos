    .text
    .global swtch
swtch:
    # Save the current context
    sd ra, 0(a0)          # Save return address
    sd sp, 8(a0)          # Save stack pointer
    sd s0, 16(a0)         # Save callee-saved registers
    sd s1, 24(a0)
    sd s2, 32(a0)
    sd s3, 40(a0)
    sd s4, 48(a0)
    sd s5, 56(a0)
    sd s6, 64(a0)
    sd s7, 72(a0)
    sd s8, 80(a0)
    sd s9, 88(a0)
    sd s10, 96(a0)
    sd s11, 104(a0)

    # Load the new context
    ld ra, 0(a1)          # Load return address
    ld sp, 8(a1)          # Load stack pointer
    ld s0, 16(a1)         # Load callee-saved registers
    ld s1, 24(a1)
    ld s2, 32(a1)
    ld s3, 40(a1)
    ld s4, 48(a1)
    ld s5, 56(a1)
    ld s6, 64(a1)
    ld s7, 72(a1)
    ld s8, 80(a1)
    ld s9, 88(a1)
    ld s10, 96(a1)
    ld s11, 104(a1)

    ret

    .global proc_start
#用于新进程的第一次启动。负责开启中断，然后跳去真正的入口
proc_start:
    csrs sstatus, 0x2    
    jalr s0
