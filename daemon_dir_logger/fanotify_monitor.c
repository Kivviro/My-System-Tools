/*
Use for monitoring processes actions. Check pid and name of process
and path to the file/directory with which the process is interacting.
*/

#define _GNU_SOURCE
#include "monitor.h"

#include <errno.h>
#include <fcntl.h>
#include <linux/fanotify.h>
#include <stdio.h>
#include <string.h>
#include <sys/fanotify.h>
#include <unistd.h>

int fanotify_monitor_init(struct monitor *m)
{
    m->fanotify_fd = fanotify_init(FAN_CLASS_NOTIF | FAN_CLOEXEC, 
        O_RDONLY | O_CLOEXEC | O_LARGEFILE);
    
    return m->fanotify_fd < 0 ? -1 : 0;
}

int fanotify_mark_directory(struct monitor *m, const char *path)
{
    uint64_t mask = FAN_EVENT_ON_CHILD | FAN_OPEN | FAN_MODIFY | FAN_CLOSE_WRITE;

    return fanotify_mark(m->fanotify_fd, FAN_MARK_ADD, mask, AT_FDCWD, path);
}

static void fanotify_event_names(uint64_t mask, char *out, size_t out_size)
{
    out[0] = '\0';

    #define APPEND_FLAG(flag, label) do { \
        if (mask & (flag)) { \
            size_t used = strlen(out); \
            snprintf(out + used, out_size > used ? out_size - used : 0, \
                    "%s%s", used ? "|" : "", (label)); \
        } \
    } while (0)
    
        APPEND_FLAG(FAN_OPEN, "OPEN");
        APPEND_FLAG(FAN_MODIFY, "MODIFY");
        APPEND_FLAG(FAN_CLOSE_WRITE, "CLOSE_WRITE");
    #undef APPEND_FLAG

    if (!out[0])
        snprintf(out, out_size, "OTHER");
}

void fanotify_handle_events(struct monitor *m)
{
    char buffer[64 *1024] __attribute__((aligned(__alignof__(struct fanotify_event_metadata))));
    size_t n = read(m->fanotify_fd, buffer, sizeof(buffer));

    if (n < 0)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            return;

        log_message(m, "ERROR fanotift read: %s", strerror(errno));
        return;
    }
    if (n == 0)
        return;

    struct fanotify_event_metadata *meta = (struct fanotify_event_metadata *)(void *)buffer;
    ssize_t remaining = n;

    while (FAN_EVENT_OK(meta, remaining))
    {
        char proc_path[64];
        char object_path[PATH_MAX];
        char pname[256];
        char event_names[128];
        ssize_t path_len;

        if (meta->vers != FANOTIFY_METADATA_VERSION)
        {
            log_message(m, "ERROR fanotify metadata version mismatch");
            break;
        }

        if (meta->fd == FAN_NOFD)
        {
            if (meta->mask & FAN_Q_OVERFLOW)
                log_message(m, "WARN fanotify queue overflow: some events were lost");
            else
                log_message(m, "WARN fanotify event has no object fd mask=0x%llx");
        }
        else if (meta->fd >= 0)
        {
            snprintf(proc_path, sizeof(proc_path), "/proc/self/fd/%d", meta->fd);
            path_len = readlink(proc_path, object_path, sizeof(object_path) - 1);

            if (path_len >= 0)
            {
                object_path[path_len] = '\0';

                // unfull if
                if (path_is_within(m->root, object_path) && !is_log_path(m, object_path))
                {
                    process_name((pid_t)meta->pid, pname, sizeof(pname));
                    fanotify_event_names(meta->mask, event_names, sizeof(event_names));
                    log_message(m, "PROSECC action=%s pid=%ld name=%s path=%s",
                        event_names, (long)meta->pid, pname, object_path);
                }
            }
            else
            {
                log_message(m, "WARN cannot resolve fanotify object fd=%d: %s", 
                    meta->fd, strerror(errno));
            }

            close(meta->fd);
        }

        if (meta->event_len == 0)
            break;

        meta = FAN_EVENT_NEXT(meta, remaining);
    }
}

void fanotify_monitor_close(struct monitor *m)
{
    if (m->fanotify_fd >= 0)
    {
        close(m->fanotify_fd);
        m->fanotify_fd = -1;
    }
}