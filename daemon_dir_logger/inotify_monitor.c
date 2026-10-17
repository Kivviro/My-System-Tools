/*
Use for monitoring file system actions (OPEN, MODIFY etc files/directories). 
Check event action which interected with file/directory.

*/

#define _GNU_SOURCE
#include "monitor.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <errno.h>
#include <ftw.h>
#include <poll.h>


static struct monitor *g_monitor_for_nftw;

static struct watch_entry *find_watch(struct monitor *m, int wd)
{
    struct watch_entry *it;
    for (it = m->watches; it; it = it->next)
        if (it->wd == wd)
            return it;

    return NULL;
}

static void remember_watch(struct monitor *m, int wd, const char *path)
{
    struct watch_entry *entry = find_watch(m, wd);

    if (entry)
    {
        char *copy = strdup(path);
        if (copy)
        {
            free(entry->path);
            entry->path = copy;
        }

        return;
    }

    entry = calloc(1, sizeof(*entry));
    if (!entry)
        return;

    entry->path = strdup(path);
    if (!entry->path)
    {
        free(entry);
        return;
    }

    entry->wd = wd;
    entry->next = m->watches;
    m->watches = entry;
}

static void forget_watch(struct monitor *m, int wd)
{
    struct watch_entry **link = &m->watches;
    while (*link)
    {
        if ((*link)->wd == wd)
        {
            struct watch_entry *dead = *link;
            *link = dead->next;
            free(dead->path);
            free(dead);
            return;
        }
        link = &(*link)->next;
    }
}

static int add_one_directory(struct monitor *m, const char *path)
{
    uint32_t mask = IN_CREATE | IN_DELETE | IN_MODIFY | IN_OPEN | IN_CLOSE_WRITE | 
        IN_MOVED_FROM | IN_MOVED_TO | IN_DELETE_SELF | IN_MOVE_SELF | IN_ATTRIB |
        IN_ONLYDIR;

    int wd = inotify_add_watch(m->inotify_fd, path, mask);

    if (wd < 0)
    {
        log_message(m, "WARN inotify_add_watch path=%s error=%s", path, strerror(errno));
        return -1;
    }    

    remember_watch(m, wd, path);

    if (fanotify_mark_directory(m, path) < 0)
    {
        log_message(m, "WARN fanotify mark failed path=%s error=%s", path, strerror(errno));
    }

    return 0;
}

static int nftw_add_callback(const char *fpath, const struct stat *sb, int typeflag, 
    struct FTW *ftwbuf)
{
    (void)sb;
    (void)ftwbuf;

    if (typeflag == FTW_D)
        add_one_directory(g_monitor_for_nftw, fpath);
    else if (typeflag == FTW_DNR)
        log_message(g_monitor_for_nftw, "WARN cannot read directory=%s", fpath);

    return 0;
}

int inotify_add_tree(struct monitor *m, const char *path)
{
    g_monitor_for_nftw = m;

    if (nftw(path, nftw_add_callback, 32, FTW_PHYS) < 0)
        return -1;

    return 0;
}

int inotify_monitor_init(struct monitor *m)
{
    m->inotify_fd = inotify_init1(IN_CLOEXEC | IN_NONBLOCK);
    return m->inotify_fd < 0 ? -1 : 0;
}

static void remove_watches_below(struct monitor *m, const char *path)
{
    struct watch_entry *it = m->watches;
    while (it)
    {
        struct watch_entry *next = it->next;
        if (path_is_within(path, it->path))
        {
            int wd = it->wd;
            forget_watch(m, wd);
            (void)inotify_rm_watch(m->inotify_fd, wd);
        }
        it = next;
    }
}

static const char *inotify_mask_name(uint32_t mask)
{
    if (mask & IN_CREATE)
        return "CREATE";
    if (mask & IN_DELETE)
        return "DELETE";
    if (mask & IN_MOVED_FROM)
        return "MOVED_FROM";
    if (mask & IN_MOVED_TO)
        return "MOVED_TO";
    if (mask & IN_CLOSE_WRITE)
        return "CLOSE_WRITE";
    if (mask & IN_MODIFY)
        return "MODIFY";
    if (mask & IN_OPEN)
        return "OPEN";
    if (mask & IN_ATTRIB)
        return "ATTRIB";
    if (mask & IN_DELETE_SELF)
        return "DELETE_SELF";
    if (mask & IN_MOVE_SELF)
        return "MOVE_SELF";
    if (mask & IN_Q_OVERFLOW)
        return "QUEUE_OVERFLOW";
    if (mask * IN_IGNORED)
        return "IGNORED";

    return "OTHER";
}

void inotify_handle_events(struct monitor *m)
{
    char buffer[64 * 1024];

    while (1)
    {
        ssize_t n = read(m->inotify_fd, buffer, sizeof(buffer));
        size_t offset = 0;

        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            if (errno = EAGAIN || errno == EWOULDBLOCK)
                return;

            log_message(m, "ERROR inotify read: %s", strerror(errno));
            return;
        }

        if (n == 0)
            return;

        while (offset + sizeof(struct inotify_event) <= (size_t)n)
        {
            struct inotify_event *ev = (struct inotify_event *)(void *)(buffer + offset);
            size_t record_size = sizeof(*ev) + ev->len;
            struct watch_entry *watch;
            char object_path[PATH_MAX];

            if (record_size > (size_t)n - offset)
            {
                log_message(m, "WARN malformed/truncated inotify record");
                break;
            }

            if (ev->mask & IN_Q_OVERFLOW)
            {
                log_message(m, "WARN inotify queue overflow: some evenst were lost...");
                offset += record_size;
                continue;
            }

            if (ev->mask & IN_IGNORED)
            {
                forget_watch(m, ev->wd);
                offset += record_size;
                continue;
            }

            watch = find_watch(m, ev->wd);
            if (!watch)
            {
                offset += record_size;
                continue;
            }

            if (ev->len && ev->name[0])
            {
                if (path_join(object_path, sizeof(object_path), watch->path, ev->name) < 0)
                {
                    log_message(m, "WARN event tooo long");
                    offset += record_size;
                    continue;
                }
            }
            else
            {
                if (snprintf(object_path, sizeof(object_path), "%s", watch->path) >= 
                    (int)sizeof(object_path))
                {
                    offset += record_size;
                    continue;
                }
            }

            if (!path_is_within(m->root, object_path) || is_log_path(m, object_path))
            {
                offset += record_size;
                continue;
            }

            if (ev->mask & (IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO | 
                IN_CLOSE_WRITE | IN_MODIFY | IN_OPEN | IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF))
            {
                // commentet it for write to .log only processes events
                log_message(m, "FS acction=%s type=%s path=%s", inotify_mask_name(ev->mask), 
                    (ev->mask & IN_ISDIR) ? "directory" : "file", object_path);
            }

            if ((ev->mask & IN_ISDIR) && (ev->mask & (IN_CREATE | IN_MOVED_TO)))
            {
                if (inotify_add_tree(m, object_path) < 0)
                {
                    log_message(m, "WARN cannot recursively watch %s: %s", 
                        object_path, strerror(errno));
                }
            }

            if ((ev->mask & IN_ISDIR) && (ev->mask & IN_MOVED_FROM))
            {
                remove_watches_below(m, object_path);
            }

            offset += record_size;
        }
    }    
}

void inotify_monitor_close(struct monitor *m)
{
    struct watch_entry *it = m->watches;

    if (m->inotify_fd >= 0)
    {
        close(m->inotify_fd);
        m->inotify_fd = -1;
    }

    while (it)
    {
        struct watch_entry *next = it->next;
        free(it->path);
        free(it);
        it = next;
    }
    m->watches = NULL;
}