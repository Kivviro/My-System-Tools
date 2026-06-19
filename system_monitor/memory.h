#ifndef MEMORY_H
#define MEMORY_H

typedef struct
{
    unsigned long total;
    unsigned long free;
    unsigned long available;

    unsigned long swap_total;
    unsigned long swap_free;
} MemoryInfo;

int read_memory(MemoryInfo *ram);

#endif
