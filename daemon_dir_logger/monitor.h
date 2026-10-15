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



// logger
int logger_open(struct monitor *m, const char *requested_log);
void logger_close(struct monitor *m);
void log_message(struct monitor *m, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

int is_log_path(const struct monitor *m, const char *path);

// fanotify module
int fanotify_monitor_init(struct monitor *m);
int fanotify_mark_directory(struct monitor *m, const char *path);
void fanotify_monitor_close(struct monitor *m);
void fanotify_handle_events(struct monitor *m);


#endif