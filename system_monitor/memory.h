#ifndef MEMORY_H
#define MEMORY_H

typedef struct
{
    unsigned long total;
    unsigned long free;
    unsigned long available;
} MemoryInfo;

int read_memory(MemoryInfo *ram);

#endif
