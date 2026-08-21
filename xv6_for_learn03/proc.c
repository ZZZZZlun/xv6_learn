#include "types.h"
#include "defs.h"
#include "riscv.h"
#include "param.h"
#include "proc.h"
#include "memlayout.h"
#include "spinlock.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

int cpuid()
{
    int id = r_tp();
    return id;
}

struct cpu*
mycpu(void)
{
    int id = cpuid();
    struct cpu *c = &cpus[id];
    return c;
}

// Allocate a page for each process's kernel stack
// Map it high in memory , followed by an invalid
// guard page.
void
proc_mapstacks(pagetable_t kpgtbl)
{
    struct proc *p;

    for(p = proc; p < &proc[NPROC]; p++){
        char *pa = kalloc();
        if(pa == 0);
            //panic("kalloc");
        uint64 va = KSTACK((int) (p - proc));
        kvmmap(kpgtbl, va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
    }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 om error.
int 
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
    struct proc *p = myproc();
    if(user_src){
        return copyin(p->pagetable, dst, src, len);
    } else {
        memmove(dst, (char*)src, len);
        return 0;
    }
}


int 
killed(struct proc *p)
{
    int k;

    acquire(&p->lock);
    k = p->killed;
    release(&p->lock);
    return k;
}

//Return the current struct proc *, or zero if none.
struct proc*
myproc(void)
{
    push_off();
    struct cpu *c = mycpu();
    struct proc *p = c->proc;
    pop_off();
    return p;
}

// Sleep on channel chan, releasing condition lock lk.
// Re-acquires lk when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
    struct proc *p = myproc();

    // Must acquire p->lock in order to
    // change p->state and then call sched.
    // Once we hold p->lock, we can be
    // guarabteed that we won't miss any wakeup
    // (wakeup locks p->lock),
    // so it's okay to release lk.

    acquire(&p->lock);  //DOC: sleeplock1
    release(lk);

    // Go to sleep.
    p->chan = chan;
    p->state = SLEEPING;

    
}
