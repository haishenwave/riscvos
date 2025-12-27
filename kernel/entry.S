.section .text.entry
.global _start

_start:
        li t0, 0x10000000
        li t1, 'S'
        sb t1, 0(t0)

        .option push
        .option norelax
        la gp, __global_pointer$
        .option pop
        la sp, stack_top

        call kmain

        wfi
        j .

