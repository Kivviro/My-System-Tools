#include "cpu.h"

#include <stdio.h>
#include <string.h>

static unsigned long long total_time(const CPUStats *s)
{
        return s->user + s->nice + s->system + s->idle + s->iowait +
            s->irq + s->softirq + s->steal;
}

int read_total_cpu(CPUStats *stats)
{
    FILE *fp = fopen("/proc/stat", "r");

    if (!fp)
        return -1;

    fscanf (fp, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &stats->user, 
    &stats->nice, &stats->system, &stats->idle, &stats->iowait, &stats->irq, 
    &stats->softirq, &stats->steal);

    fclose(fp);

    return 0;
}

int read_all_cores(CPUStats *stats, int cores)
{
    FILE *fp = fopen("/proc/stat", "r");

    if (!fp)
        return -1;

    char line[256];
    fgets(line, sizeof(line), fp);

    for (int i = 0; i < cores; i++)
    {
        char label[16];

        if (!fgets(line, sizeof(line), fp))
        {
            fclose(fp);
            return -1;
        }

        sscanf(line, "%15s %llu %llu %llu %llu %llu %llu %llu %llu", label, 
            &stats[i].user, &stats[i].nice, &stats[i].system, &stats[i].idle, 
            &stats[i].iowait, &stats[i].irq, &stats[i].softirq, &stats[i].steal);
    }

    fclose(fp);

    return 0;
}

double cpu_usage(const CPUStats *old, const CPUStats *new)
{
    unsigned long long old_total = total_time(old);
    unsigned long long new_total = total_time(new);
    unsigned long long total_delta = new_total - old_total;
    unsigned long long idle_delta = (new->idle + new->iowait) - 
        (old->idle + old->iowait);

    if (total_delta == 0)
        return 0.0;

    return 100.0 * (total_delta - idle_delta) / total_delta;
}