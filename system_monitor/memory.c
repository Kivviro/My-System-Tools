#include "memory.h"

#include <stdio.h>
#include <string.h>

int read_memory(MemoryInfo *ram)
{
    ram->total = 0;
    ram->free = 0;
    ram->available = 0;

    ram->swap_total = 0;
    ram->swap_free = 0;

    FILE *fp = fopen("/proc/meminfo", "r");

    if (!fp)
        return -1;

    char line[256];

    while (fgets(line, sizeof(line), fp))
    {
        if (sscanf(line, "MemTotal: %lu", &ram->total) == 1)
            continue;
        if (sscanf(line, "MemFree: %lu", &ram->free) == 1)
            continue;
        if (sscanf(line, "MemAvailable: %lu", &ram->available) == 1)
            continue;
        if (sscanf(line, "SwapTotal: %lu", &ram->swap_total) == 1)
            continue;
        if (sscanf(line, "SwapFree: %lu", &ram->swap_free))
            continue;
    }

    fclose(fp);

    return 0;
}