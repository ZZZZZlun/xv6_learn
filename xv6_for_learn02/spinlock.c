// Mutual exclusion spin locks.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "proc.h"
#include "defs.h"

void
initlock(struct spinlock *lk, char *name)
{
    lk->name = name;
    lk->locked = 0;
    lk->cpu = 0;
}

//amoswap指令 加锁
static inline uint64
atomic_test_and_set(volatile uint64 *lock,uint64 new_val)
{
    uint64 old_val;
    
    __asm__ volatile (
        "amoswap.d.aq %0, %2, (%1)"
        : "=r"(old_val)
        : "r"(lock),
        "r"(new_val)
        : "memory"
    );
    return old_val;
}

//解锁
static inline void
atomic_lock_release(volatile uint64 *lock)
{
    __asm__ volatile(
        "amoswap.d.rl zero, zero, (%0)"
        :
        : "r"(lock)
        : "memory"
    );
}


// Acquire the lock.
// Loops (spins) until the lock is acquired.
void
acquire(struct spinlock *lk)
{
    push_off();             //disable interrupts to avoid deadlock.
    if(holding(lk));
        //panic("acquire");
    
    // On RISCV-V,sync_lock_test_and_set turns into an atomic swap:
    //  a5 = 1
    //  s1 = &lk->locked
    //  amoswap.w.aq a5, a5, (s1)
    while(atomic_test_and_set(&lk->locked, 1) != 0);

    // Tell the C compiler and the processor to not move loads or stores
    //past this point, to ensure that the critical section's memory
    //references happen strictly after the lock is acquired.//防cpu乱序执行，防编译器乱排代码
    //On RISCV-V, this emits a fence instruction.
    __sync_synchronize();

    // Record info about lock acquisition for holding() and debugging //for debugging
    lk->cpu = mycpu();
}

//Release the lock
void
release(struct spinlock *lk)
{
    if(!holding(lk))
    {
        //panic("release");
    }
    

    //Tell the C compiler and the CPU to not move loads or stores
    //past this point, to ensure that all the stores in the critical
    //section are visible to other CPUs before the lock is released,
    //and that loads in the critical section occur strictly before
    //the lock is released.
    // On RISCV-V, this emits a fence instruction.
    __sync_synchronize();

    lk->cpu = 0;

    // Release the lock, equivalent to lk->locked = 0
    atomic_lock_release(&lk->locked);   

    pop_off();
    
}

// Check whether this cpu is holding the lock.
// Interrupts must be off.
int 
holding(struct spinlock *lk)
{
    int r;
    r = (lk->locked && lk->cpu == mycpu());
    return r;
}


void
push_off(void)
{
    int old = intr_get();

    // disable interrupts to prevent an involuntary context
    // switch while using mycpu().
    intr_off();

    if(mycpu()->noff == 0)
    {
        mycpu()->intena = old;
    }
    mycpu()->noff += 1;
}

void 
pop_off(void)
{
    struct cpu *c = mycpu();
    if(intr_get());
        //panic("pop_off - interruptible");
    if(c->noff < 1);
        //panic("pop_off");
    c->noff -= 1;
    if(c->noff == 0 && c->intena)
        intr_on();
}