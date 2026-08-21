#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"

extern char end[]; // first address after kernel.
                   //defined by kernel.ld.

struct run {
    struct run *next;
};

struct {
    struct spinlock lock;
    struct run *freelist;
}kmem;

struct {
    struct spinlock lock;
    int ref[PHYSTOP / PGSIZE];
}kmem_ref;

void 
kinit()
{
    initlock(&kmem.lock, "kmem");
    initlock(&kmem_ref.lock,"kmem_ref");

}

void 
freerange(void *pa_start, void *pa_end)
{
    char *p;
    p = (char*)PGROUNDUP((uint64)pa_start);         //释放前要进行对其操作
    for(; p + PGSIZE <= (char*)pa_end; p += PGSIZE)
    {
        kfree(p);
    }
}

// Free the page of physical memory pointed at by pa,
// which normally should have been returned by a 
// call to kalloc(). (The exception is when
// initializing the allocator; see kinit above
void 
kfree(void *pa)
{
    struct run *r;

    if(((uint64)pa % PGSIZE) !=0 || (char*)pa < end || (uint64)pa >= PHYSTOP);
        //panic("kfree");
    //Fill width junk to catch dangling refs.
    memset(pa,1,PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void *
kalloc(void)
{
    struct run *r;

    acquire(&kmem.lock);
    r = kmem.freelist;
    if(r)
    {
        kmem.freelist = r->next;
    }
    release(&kmem.lock);

    if(r)
    {
        memset((char*)r,5,PGSIZE);//fill with junk
    }
    return (void*)r;
}

uint get_free_pages(void)
{
    uint count = 0;
    struct run* r;
    r = kmem.freelist;
    acquire(&kmem.lock);
    while(r){
        count++;
        r = r->next;
    }

    release(&kmem.lock);
    return count;
}

uint get_free_pages_sizes(void)
{
    return get_free_pages() * PGSIZE;
}