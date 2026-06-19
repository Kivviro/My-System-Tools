#define _POSIX_C_SOURCE 200809L

#include "tasks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

static int is_pid_dir(const char *name)
{
    while (*name)
    {
        if (!isdigit((unsigned char)*name))
            return 0;
        
        name++;
    }

    return 1;
}

int read_tasks(TaskInfo *info)
{
    info->tasks = 0;
    info->threads = 0;
    info->kernel_threads = 0;
    info->running = 0;

    DIR *proc;
    struct dirent *entry;

    proc = opendir("/proc");

    if (!proc)
        return -1;

    while ((entry = readdir(proc)) != NULL)
    {
        if (!is_pid_dir(entry->d_name))
            continue;

        info->tasks++;

        char path[256];

        snprintf(path, sizeof(path), "/proc/%s/status", entry->d_name);

        FILE *fp = fopen(path, "r");

        if (!fp)
            continue;

        char line[256];
        int threads = 0;
        char state = '?';

        while (fgets(line, sizeof(line), fp))
        {
            if (sscanf(line, "Threads: %c", &threads) == 1)
                continue;

            if (sscanf(line, "State: %c", &state) == 1)
                continue;
        }

        fclose(fp);

        info->threads += threads;

        if (state == 'R')
            info->running++;

        snprintf(path, sizeof(path), "/proc/%s/exe", entry->d_name);

        if (access(path, F_OK) == 0)
        {
            char exe[512];

            ssize_t len = readlink(path, exe, sizeof(exe) - 1);

            if (len == -1)
                info->kernel_threads++;
        }
    }

    closedir(proc);

    return 0;
}