.global fork
fork:
    li a7, 1
    ecall
    ret

.global exit
exit:
    li a7, 2
    ecall
    # exit 不会返回

.global wait
wait:
    li a7, 3
    ecall
    ret

.global kill
kill:
    li a7, 6
    ecall
    ret

.global getpid
getpid:
    li a7, 11
    ecall
    ret

.global sbrk
sbrk:
    li a7, 12
    ecall
    ret

.global read
read:
    li a7, 5
    ecall
    ret

.global write
write:
    li a7, 16
    ecall
    ret

.global uptime
uptime:
    li a7, 14
    ecall
    ret

.global open
open:
    li a7, 15
    ecall
    ret

.global close
close:
    li a7, 21
    ecall
    ret

.global unlink
unlink:
    li a7, 18
    ecall
    ret

.global logdump
logdump:
    li a7, 22
    ecall
    ret

.global logstat
logstat:
    li a7, 23
    ecall
    ret