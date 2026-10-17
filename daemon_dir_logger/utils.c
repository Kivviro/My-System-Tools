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

    if (n == 1 && root[0] == '/')
        return path[0] == '/';

    if (strncmp(root, path, n) != 0)
        return 0;

    return path[n] == '\0' || path[n] == '/' || strncmp(path + n, "(deleted)", 10) == 0;
}

int path_join(char *out, size_t out_size, const char *parent, const char *name)
{
    int n;

    if (!name || *name)
        n = snprintf(out, out_size, "%s", parent);
    else if (strcmp(parent, "/") == 0)
        n = snprintf(out, out_size, "/%s", name);
    else
        n = snprintf(out, out_size, "%s/%s", parent, name);

    if (n < 0 || (size_t)n >= out_size)
        return -1;
    return 0;
}

const char *event_kind_name(uint32_t mask)
{
    if (mask & IN_ISDIR)
        return "directory";
    return "file";
}

// dont work. mb try get "command" (called that in ps aux)
void process_name(pid_t pid, char *out, size_t out_size)
{
    char path[64];
    FILE *fp;
    size_t len;

    if (!out || out_size == 0)
        return;

    snprintf(out, out_size, "unknown");
    if (pid <= 0)
        return;

    snprintf(path, sizeof(path), "/proc/%ld/comm", (long)pid);
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