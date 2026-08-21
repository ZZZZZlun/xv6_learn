struct spinlock;
struct proc;
typedef uint64 *pagetable_t;
typedef uint64 pte_t;

//kalloc.c
void*           kalloc(void);
void            kfree(void *);
void            kinit(void);
uint            get_free_pages(void);
uint            get_free_pages_sizes(void);


// proc.c
int             cpuid(void);
struct cpu*     mycpu(void);
int             killed(struct proc *p);
void            proc_mapstacks(pagetable_t kpgtbl);
struct proc*    myproc();
int             either_copyout(int user_dst, uint64 dst, void *src, uint64 len);

//spinlock.c
void            acquire(struct spinlock*);
int             holding(struct spinlock*);
void            initlock(struct spinlock*, char* name);
void            release(struct spinlock*);
void            push_off(void);
void            pop_off(void);

//string.c
int             memcmp(const void*, const void*, uint);
void*           memmove(void*, const void*, uint);
void*           memset(void*, int, uint);
char*           safestrcpy(char*, const char*, uint);
int             strlen(const char*);
int             strncmp(const char*, const char*, uint);
char*           strncpy(char*, const char*, int);


//console.h
void consputc(int c);

//printf.c
int             printf(char*, ...);


// vm.c
void            kvminit(void);
void            kvmmap(pagetable_t, uint64, uint64, uint64, int perm);
int             mappages(pagetable_t, uint64, uint64, uint64, int perm);
pagetable_t     uvmcreate(void);
int             ismapped(pagetable_t, uint64);
uint64          uvmdealloc(pagetable_t, uint64, uint64);
uint64          vmfault(pagetable_t, uint64, int);
pte_t*          walk(pagetable_t, uint64, int);
uint64          walkaddr(pagetable_t, uint64);
int             copyout(pagetable_t, uint64, char *, uint64);
int             copyin(pagetable_t, char *, uint64, uint64);

// uart.c
void            uartinit(void);
void            uartintr(void);
void            uartwrite(char [], int);
void            uartputc_sync(int);
int             uartgetc(void);