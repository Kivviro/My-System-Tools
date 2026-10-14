#ifndef FIGGER_MONITOR_H
#define FIGGER_MONITOR_H

#define _GNU_SOURCE

#include <limits.h>
#include <signal.h>
#include <sys/types.h>
#include <stdint.h>
#include <sys/inotify.h>

struct watch_entry {
    int wd;
    char *path;
    struct watch_entry *next;
};

struct monitor {
    int inotify_fd;
    int fanotify_fd;
    char root[PATH_MAX];
    char log_path[PATH_MAX];
    FILE *log_file;
    struct watch_entry *watches;
};

#endif