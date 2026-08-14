

.section .text
.global _entry

_entry:
    #这里共享的是同一个内存
    #set up a stack for C
    #stack0 is declared in start.c,
    #with a 4096-byte stack per CPU.
    #sp = stack0 + ((hartid + 1) * 4096)
    la sp,stack0
    li a0,1024*4
    csrr a1, mhartid
    addi a1,a1,1
    mul a0,a0,a1
    add sp,sp,a0

    call start
    spin:
        j spin