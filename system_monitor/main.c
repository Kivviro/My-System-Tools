#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "memory.h"

/*
/proc/stat - CPU
/proc/meminfo - RAM
/proc/cpuinfo - CPU cores
*/

/*
add later:
    - count of tasks
    - uptime
    - processes
*/

static void clear_screen(void)
{
    printf("\033[H\033[J");
}

static void draw_bar(double percent)
{
    int width = 40;

    int filled = (int)(percent * width / 100.0);

    printf("[");

    for (int i = 0; i < width; i++)
    {
        if (i < filled)
            putchar('#');
        else
            putchar(' ');
    }

    printf("] %5.1f%%", percent);
}

int main(void)
{
    int cores = sysconf(_SC_NPROCESSORS_ONLN);

    CPUStats total_old;
    CPUStats total_new;

    CPUStats *cores_old = malloc(sizeof(CPUStats) * cores);
    CPUStats *cores_new = malloc(sizeof(CPUStats) * cores);

    if (!cores_old || !cores_new)
    {
        fprintf(stderr, "malloc errod\n"); // change to errno lib
        return 1;
    }

    read_total_cpu(&total_old);
    read_all_cores(cores_old, cores);

    while (1)
    {
        sleep(1);

        read_total_cpu(&total_new);
        read_all_cores(cores_new, cores);

        MemoryInfo ram;

        read_memory(&ram);

        if (read_memory(&ram) != 0)
        {
            fprintf(stderr, "Cannot read /proc/meminfo\n");
        }

        clear_screen();

        printf("                   === KMONITOR ===\n\n");

        double total_usage = cpu_usage(&total_old, &total_new);

        printf("CPU TOTAL\n");

        draw_bar(total_usage);

        printf("\n\n");
        
        printf("CPU CORES\n\n");

        for (int i = 0; i < cores; i++)
        {
            double usage = cpu_usage(&cores_old[i], &cores_new[i]);

            printf("Core %-2d ", i);
            draw_bar(usage);
            printf("\n");
        }

        printf("\n");

        unsigned long total_mb = ram.total / 1024;
        unsigned long free_mb = ram.available / 1024;
        unsigned long used_mb = total_mb - free_mb;

        printf("RAM\n\n");
        printf("Total : %lu MB\n", total_mb);
        printf("Used : %lu MB\n", used_mb);
        printf("Free : %lu MB\n", free_mb);

        total_old = total_new;

        for (int i = 0; i < cores; i++)
            cores_old[i] = cores_new[i];
    }

    free(cores_old);
    free(cores_new);

    return 0;
}