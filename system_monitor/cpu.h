#ifndef CPU_H
#define CPU_H

typedef struct
{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
} CPUStats;

int read_total_cpu(CPUStats *stats);
int read_all_cores(CPUStats *stats, int cores);

double cpu_usage(const CPUStats *old_stat, const CPUStats *new_stat);

#endif