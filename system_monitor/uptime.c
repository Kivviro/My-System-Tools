#include "uptime.h"

#include <stdio.h>

double get_uptime(void)
{
    FILE *fp = fopen("/proc/uptime", "r");

    if (!fp)
        return 0.0;

    double uptime = 0.0;

    fscanf(fp, "%lf", &uptime);

    fclose(fp);

    return uptime;
}

void print_uptime(double uptime)
{
    int total = (int)uptime;
    int days = total / 86400;
    int hours = (total % 86400) / 3600;
    int minutes = (total % 3600) / 60;
    int seconds = total % 60;

    printf("%dd %dh %dm %ds", days, hours, minutes, seconds);
}