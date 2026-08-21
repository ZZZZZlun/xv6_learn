#include "types.h"
#include "riscv.h"

#define NCPU 4
void main();
void timerinit();

__attribute__ ((aligned (16))) char stack0[4096 * NCPU];//把该变量的地址强行放到16的整数倍上

#define UART0 0x10000000

static inline void raw_putc(char c) {
    volatile char *thr = (volatile char *)(UART0 + 0);
    *thr = c;
}

static inline void raw_puts(char *s) {
    while (*s) {
        if (*s == '\n') raw_putc('\r');
        raw_putc(*s++);
    }
}




void start()
{
    raw_puts("start: before mret\n");
    //set M Previous Privilege mode to Supervisor , for mret.
    unsigned long x = r_mstatus();
    x &= ~MSTATUS_MPP_MASK;
    x |= MSTATUS_MPP_S;
    w_mstatus(x);

    //set M Exception Program Counter to main, for mret.
    //reqtures gcc -mcmodel=medany
    w_mepc((uint64)main);     

    //disable paging for now
    w_satp(0);

    //delegate all interrupts and exceptions to supervisor mode.
    w_medeleg(0xffff);//机器异常代理寄存器
    w_mideleg(0xffff);//机器中断代理寄存器
    //监管中断寄存器
    w_sie(r_sie() | SIE_SEIE | SIE_STIE);

    // configure Physical Memory Protection to give supervisor mode
    // access to all of physical memory
    //物理内存保护地址寄存器
    w_pmpaddr0(0x3fffffffffffffull);
    //物理内存保护配置
    w_pmpcfg0(0xf);

    // ask for clock interrupts
    timerinit();

    // keep each CPU's hartid in its tp register, for cpuid().
    int id = r_mhartid();
    w_tp(id);

    //switch to supervisor mode and jump to main().
    asm volatile("mret"); //s模式的切换指令PC跳转+特权级切换

}

//ask each hart tp generate timer interrupts.
void
timerinit()
{
    //enable supervisor-mode timer interrupts
    w_mie(r_mie() | MIE_STIE);

    //eanble the sstc extension (i.e. stimecmp).
    w_menvcfg(r_menvcfg() | (1L << 63));

    //allow supervisor to use stimecmp and time.
    w_mcounteren(r_mcounteren() | 2);

    //ask for the very first timer interrupt.
    w_stimecmp(r_time() + 1000000);

}

