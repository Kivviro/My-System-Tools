#ifndef FIGGER_MONITOR_H
#define FIGGER_MONITOR_H

#define _GNU_SOURCE

#include <limits.h>
#include <signal.h>
#include <stdio.h>
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

extern volatile sig_atomic_t g_stop;

// logger
int logger_open(struct monitor *m, const char *requested_log);
void logger_close(struct monitor *m);
void log_message(struct monitor *m, const char *format, ...)
    __attribute__((format(printf, 2, 3)));

int is_log_path(const struct monitor *m, const char *path);

// inotify module
int inotify_monitor_init(struct monitor *m);
int inotify_add_tree(struct monitor *m, const char *path);
void inotify_monitor_close(struct monitor *m);
void inotify_handle_events(struct monitor *m);

// fanotify module
int fanotify_monitor_init(struct monitor *m);
int fanotify_mark_directory(struct monitor *m, const char *path);
void fanotify_monitor_close(struct monitor *m);
void fanotify_handle_events(struct monitor *m);

// other utils
int path_is_within(const char *root, const char *path);
int path_join(char *out, size_t out_size, const char *parent, const char *name);
const char *event_kind_name(uint32_t mask);
void process_name(pid_t pid, char *out, size_t out_size);

#endif