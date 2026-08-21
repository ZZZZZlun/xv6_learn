// Long-term locks for processes
typedef unsigned long uint64;

struct sleeplock {
    uint64 locked;       // Is the lock held?
    struct spinlock lk;// spinlock protecting this sleep lock

    // For debugging:
    char *name;
    int pid;
};