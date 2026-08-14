// Mutual exclusion lock
typedef unsigned long uint64;

struct spinlock{
    uint64 locked;

    // For debugging:
    char *name;         // Name of lock.
    struct cpu *cpu;    // The cpu holding the lock
};