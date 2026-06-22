#define _POSIX_C_SOURCE 200809L

#include "tasks.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>

static int is_numeric(const char *s)
{
    if (*s == '\0')
        return 0;

    while (*s)
    {
        if (!isdigit((unsigned char)*s))
            return 0;

        s++;
    }

    return 1;
}

static int count_threads(const char *pid)
{
    char path[256];

    snprintf(path, sizeof(path), "/proc/%s/task", pid);

    DIR *task_dir = opendir(path);

    if (!task_dir)
        return 0;

    struct dirent *entry;

    int count = 0;

    while ((entry = readdir(task_dir)) != NULL)
    {
        if (is_numeric(entry->d_name))
            count++;
    }

    closedir(task_dir);

    return count;
}

static int is_kernel_thread(const char *pid)
{
    char path[256];

    snprintf(path, sizeof(path), "/proc/%s/exe", pid);

    char target[512];

    ssize_t len = readlink(path, target, sizeof(target) - 1);

    return (len == -1);
}

static char get_state(const char *pid)
{
    char path[256];

    snprintf(path, sizeof(path), "/proc/%s/status", pid);

    FILE *fp = fopen(path, "r");

    if (!fp)
        return '?';

    char line[256];
    char state = '?';

    while (fgets(line, sizeof(line), fp))
    {
        if (strncmp(line, "State:", 6) == 0)
        {
            sscanf(line, "State:\t%c", &state);
            break;
        }
    }

    fclose(fp);

    return state;
}

int read_tasks(TaskInfo *info)
{
    info->tasks = 0;
    info->threads = 0;
    info->kernel_threads = 0;
    info->running = 0;

    DIR *proc = opendir("/proc");

    if (!proc)
        return -1;

    struct dirent *entry;

    while ((entry = readdir(proc)) != NULL)
    {
        if (!is_numeric(entry->d_name))
            continue;

        const char *pid = entry->d_name;

        int kernel = is_kernel_thread(pid);

        char state = get_state(pid);

        if (state == 'R')
            info->running++;

        if (kernel)
        {
            info->kernel_threads++;
            continue;
        }

        info->tasks++;

        int threads = count_threads(pid);

        info->threads += threads;
    }

    closedir(proc);

    return 0;
}