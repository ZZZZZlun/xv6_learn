struct spinlock;
struct proc;



//kalloc.c
void*           kalloc(void);
void            kfree(void *);
void            kinit(void);
uint            get_free_pages(void);
uint            get_free_pages_sizes(void);


// proc.c
int cpuid(void);
struct cpu* mycpu(void);


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