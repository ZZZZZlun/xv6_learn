#include "types.h"
#include "riscv.h"
#include "defs.h"


volatile static int started = 0;

#define UART0 0x10000000

static inline void raw_putc(char c) {
    volatile char *thr = (volatile char *)(UART0 + 0);
    *thr = c;
}

static inline void raw_puts(char *s) {
    while (*s) {
        if (*s == '\n')
            raw_putc('\r');
        raw_putc(*s++);
    }
}

void 
main()
{
    

        uartinit();
        char msg[] = "Hello, UART!\n";
        char msg1[] = "Hello, UART1s!\n";
        char msg2[] = "Hello, UARTbr//?~_+=^ &%!\n";
        uartwrite(msg, sizeof(msg)-1);
        // kinit();
        
        uartwrite(msg1, sizeof(msg1)-1);
        // kvminit();
        
        
        uartwrite(msg2, sizeof(msg2)-1);

    
}