#include "memory.h"

#include <stdio.h>
#include <string.h>

int read_memory(MemoryInfo *ram)
{
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
    }

    // char key[64];
    // unsigned long value;

    // while (fscanf(fp, "%63s %lu", key, &value) == 2)
    // {
    //     if (strcmp(key, "MemTotal:") == 0)
    //         ram->total = value;
    //     else if (strcmp(key, "MemFree:") == 0)
    //         ram->free = value;
    //     else if (strcmp(key, "MemAvailable:") == 0)
    //         ram->available = value;
    // }

    fclose(fp);

    return 0;
}