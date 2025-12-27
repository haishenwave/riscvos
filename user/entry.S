.section .text.entry
.global _start

_start:
    # 调用 main(argc, argv)
    # 目前我们没有参数，a0=0, a1=0
    li a0, 0
    li a1, 0
    call main
    
    # main 返回后调用 exit(retval)
    # a0 已经是 main 的返回值
    li a7, 2    # SYS_exit
    ecall
    
    # 死循环
1:  j 1b