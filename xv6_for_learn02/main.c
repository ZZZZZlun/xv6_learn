#include "types.h"
#include "riscv.h"
#include "defs.h"


volatile static int started = 0;

void 
main()
{
    if(cpuid() == 0){
        kinit();
        kvminit();
        

    }
}