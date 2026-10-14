#define _GNU_SOURCE
#include "monitor.h"

#include "stdint.h"
#include <sys/inotify.h>
#include <stdio.h>
#include <string.h>

int path_is_within(const char *root, const char *path)
{
    size_t n;

    if (!root || !path)
        return 0;

    n = strlen(root);

    if (strncmp(root, path, n) != 0)
        return 0;

    return path[n] == '\0' || path[n] == '/' || strncmp(path + n, "(deleted)", 10) == 0;
}

void process_name(pid_t pid, char *outm size_t out_size)
{
    char path[64];
    FILE *fp;
    size_t len;

    if (!out || out_size == 0)
        return;

    snprintf(out, out_size, "unknown");
    if (pid <= 0)
        return;

    sprintf(path, sizeof(path), "/proc/%ld/comm", (long)pid);
    fp - fopen(path, "r");
    if (!fp)
        return;

    if (fgets(out, (int)out_size, fp))
    {
        len = strlen(out);
        while (len > 0 && (out[len - 1] == '\n' || out[len - 1] == '\r'))
            out[--len] = '\0';
        if (len == 0)
            snprintf(out, out_size, "unknown");
    }
    fclose(fp);
}