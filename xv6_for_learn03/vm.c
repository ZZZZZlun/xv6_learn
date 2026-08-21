#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "memlayout.h"
#include "proc.h"

pagetable_t kernel_pagetable;
 extern char etext[];   //kernel.ld sets this to end of kernel code.

 extern char trampoline[];  //trampoline.S

 // Make a direct-map page table for the kernel
 pagetable_t
 kvmmake(void)
 {
    pagetable_t kpgtbl;

    kpgtbl = (pagetable_t) kalloc();
    memset(kpgtbl, 0, PGSIZE);

    // uart registers
    kvmmap(kpgtbl, UART0, UART0, PGSIZE, PTE_R | PTE_W); 

    // virtio mmio disk interface
    kvmmap(kpgtbl,VIRTIO0, VIRTIO0, PGSIZE, PTE_R | PTE_W);

    // PLIC 
    kvmmap(kpgtbl,PLIC,PLIC,0x4000000, PTE_R | PTE_W);

    // map kernel text executable and read-only.
    kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)etext-KERNBASE, PTE_R | PTE_X);

    // map kernel data and the physical RAM we'll make use of.
    kvmmap(kpgtbl, (uint64)etext, (uint64)etext, PHYSTOP-(uint64)etext, PTE_R | PTE_W);
    
    // allocate and map a kernel stack for process.
   proc_mapstacks(kpgtbl);

    return kpgtbl;
 }

// add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void
kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm)
{
   if(mappages(kpgtbl, va, sz, pa, perm) != 0)
   {
      //panic("kvmmap");
   }
}

//Initialize the kernel_pagetable,shared by all CPUs.
void
kvminit(void)
{
   kernel_pagetable = kvmmake();
}



//Return the address of the PTE in page table pagetable
//that corresponds to virtual address va. If alloc != 0,
//create any required page-table pages.
//
pte_t*
walk(pagetable_t pagetable, uint64 va, int alloc)
{
   if(va >= MAXVA)
      //panic("walk");

   for(int level = 2; level >0; level--){
      pte_t *pte = &pagetable[PX(level,va)];
      if(*pte &PTE_V){
         pagetable = (pagetable_t)PTE2PA(*pte);
      } else {
         if(!alloc || (pagetable = (pde_t*)kalloc()) == 0)
            return 0;
         memset(pagetable, 0, PGSIZE);
         *pte = PA2PTE(pagetable) | PTE_V; 
      }
   }
   return &pagetable[PX(0,va)];
}


// Look up a virtual address, return the physical address,
// or 0 if not mapped.
// Can only be used to look up user pages.
uint64
walkaddr(pagetable_t pagetable, uint64 va)
{
   pte_t *pte;
   uint64 pa;

   if(va >= MAXVA)
      return 0;

   pte = walk(pagetable, va, 0);
   if(pte == 0)
      return 0;
   if((*pte & PTE_V) == 0)
      return 0;
   if((*pte & PTE_U) == 0)
      return 0;
   pa = PTE2PA(*pte);
   return pa; 
}





//Create PTEs for virtual address starting at va that refer to
//physical address starting ai pa.
//va and size MUST be page-aligned.
//Returns 0 on success, -1 if walk() couldn't
//allocate a needed page-table page.
int
mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm)
{
   uint64 a ,last;
   pte_t *pte;

   if((va % PGSIZE) != 0)
      //panic("mappages: va not aligned");

   if((size % PGSIZE) != 0)
      //panic("mappages: size not aligned");

   if(size == 0)
      //panic("mappages: size");

   a = va;
   last = va + size - PGSIZE;
   for(;;){
      if((pte = walk(pagetable,a,1)) == 0)
         return -1;
      if(*pte & PTE_V)
         //panic("mappages: remap");
      *pte = PA2PTE(pa) | perm | PTE_V;
      if(a == last)
         break;
      a += PGSIZE;
      pa += PGSIZE;
   }
   return 0;
}

// create an empty user page table.
// returns 0 if out of memory. 
pagetable_t
uvmcreate()
{
   pagetable_t pagetable;
   pagetable = (pagetable_t) kalloc();
   if(pagetable == 0)
      return 0;
   memset(pagetable, 0, PGSIZE);
   return pagetable;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. It's OK if the mapping don't exist.
// Optionally free the physical memory.
void // 解除映射
uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free)
{
   uint64 a;
   pte_t *pte;

   if((va % PGSIZE) != 0);
      //panic("uvmunmap: not aligned");

   for(a = va; a < va + npages*PGSIZE; a += PGSIZE){
      if((pte = walk(pagetable, a, 0)) == 0)
         continue;
      if((*pte & PTE_V) == 0) // has physical page been allocated?
         continue;
      if(do_free){
         uint64 pa = PTE2PA(*pte);
         kfree((void*)pa);
      }
      *pte = 0;
   }
}

// Allocaqte PTEs and physical memory to grow a proccess from oldsz to
// newsz, which need not be page aligned. Returns new size or 0 on error.
uint64     //进程的堆？？
uvmalloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz, int xperm)
{
   char *mem;
   uint64 a;

   if(newsz < oldsz)
      return oldsz;
   //对齐操作
   oldsz = PGROUNDUP(oldsz);
   for(a = oldsz; a < newsz; a += PGSIZE){
      mem = kalloc();
      if(mem == 0){
         uvmdealloc(pagetable, a, oldsz);
         return 0;
      }
      memset(mem, 0, PGSIZE);
      if(mappages(pagetable, a, PGSIZE, (uint64)mem, PTE_R|PTE_U|xperm) != 0){
         kfree(mem);             //映射失败操作
         uvmdealloc(pagetable, a, oldsz);
         return 0;
      }
   }
   return newsz;
}


// Deallocate user pages to bring the process size from oldsz to 
// newsz. oldsz and newsz need not be page-aligned, nor does newsz
// need to be less than oldsz. oldsz can be larger than the actual
// process size. Returns the new process size.
uint64
uvmdealloc(pagetable_t pagetable, uint64 oldsz, uint64 newsz)
{
   if(newsz >= oldsz)
      return oldsz;
   
   if(PGROUNDUP(newsz) < PGROUNDUP(oldsz)){
      int npages = (PGROUNDUP(oldsz) - PGROUNDUP(newsz)) / PGSIZE;
      uvmunmap(pagetable, PGROUNDUP(newsz), npages, 1);
   }

   return newsz;
}

// Recursively free page-table pages.
// All leaf mappings must already have been removed.
void
freewalk(pagetable_t pagetable)
{
   // there are 2^9 = 512 PTEs in a page table.
}


// Copy drom kernel to user.
// Copy len bytes from src to virtual address dstva in a given page table.
// Return 0 on success, -1 on error.
int 
copyout(pagetable_t pagetable, uint64 dstva, char *src, uint64 len)
{
   uint64 n, va0, pa0;
   pte_t *pte;

   while(len > 0){
      va0 = PGROUNDDOWN(dstva);
      if(va0 >= MAXVA)
         return -1;

      pa0 = walkaddr(pagetable, va0);
      if(pa0 == 0){
         if((pa0 = vmfault(pagetable, va0, 0)) == 0){
            return -1;
         }
      }

      pte = walk(pagetable, va0, 0);
      // forbid copyout over read-only user text pages.
      if((*pte & PTE_W) == 0)
         return -1;

      n = PGSIZE - (dstva - va0);
      if(n > len)
         n = len;
      memmove((void *)(pa0 + (dstva - va0)), src, n);

      len -= n;
      src += n;
      dstva = va0 + PGSIZE;
   }
   return 0;
}

// Copy from user to kernel.
// Copy len bytes to dst from virtual address srcva in a given page table.
// Return 0 on success, -1 on error.
int 
copyin(pagetable_t pagetable, char *dst, uint64 srcva, uint64 len)
{
   uint64 n, va0, pa0;

   while(len > 0){
      va0 = PGROUNDDOWN(srcva);
      pa0 = walkaddr(pagetable, va0);
   
      if(pa0 == 0){
         if((pa0 = vmfault(pagetable, va0, 0)) == 0){
            return -1;
         }
      }
      n = PGSIZE - (srcva - va0);
      if(n > len)
         n = len;
      memmove(dst, (void *)(pa0 + (srcva - va0)), n);

      len -= n;
      dst += n;
      srcva = va0 + PGSIZE;
   }
   return 0;
}

// Copy a null-terminated string from user to kernel.
// Copy bytes to dst from virtual address srcva in a given page table,
// untill a '\0', or max.
// Return 0 on success, -1 on error.





// allocate and map user memory if process is referencing a page
// that was lazily allocated in sys_sbrk().
// returns 0 if va is invalid or already mapped, or if
// out of physical memory, and physical address if successful.
uint64
vmfault(pagetable_t pagetable, uint64 va, int read)
{
   uint64 mem;
   struct proc *p = myproc();

   if(va >= p->sz)
      return 0;
   va = PGROUNDDOWN(va);
   if(ismapped(pagetable, va)){
      return 0;
   }
   mem = (uint64) kalloc();
   if(mem == 0)
      return 0;
   memset((void *) mem, 0, PGSIZE);
   if(mappages(p->pagetable, va, PGSIZE, mem, PTE_W|PTE_U|PTE_R) != 0){
      kfree((void*)mem);
      return 0;
   }
   return mem;
}


int
ismapped(pagetable_t pagetable, uint64 va)
{
   pte_t *pte = walk(pagetable, va, 0);
   if(pte == 0){
      return 0;
   }
   if (*pte &PTE_V){
      return 1;
   }
   return 0;
}