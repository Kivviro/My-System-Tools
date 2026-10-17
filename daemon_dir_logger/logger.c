/* event logging module to a .log file */

#define _GNU_SOURCE
#include "monitor.h"

#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

int logger_open(struct monitor *m, const char *requested_log)
{
    if (requested_log)
    {
        if (strlen(requested_log) >= sizeof(m->log_path))
        {
            errno = ENAMETOOLONG;
            return -1;
        }

        if (!realpath(requested_log, m->log_path))
        {
            char copy[PATH_MAX];
            char parent[PATH_MAX];
            char *slash;

            if (strlen(requested_log) >= sizeof(copy))
            {
                errno = ENAMETOOLONG;
                return -1;
            }

            strcpy(copy, requested_log);
            slash = strrchr(copy, '/');
            
            if (!slash)
            {
                if (!getcwd(parent, sizeof(parent)))
                    return -1;
                if (snprintf(m->log_path, sizeof(m->log_path), "%s/%s", parent, copy) >= 
                    (int)sizeof(m->log_path))
                {
                    errno = ENAMETOOLONG;
                    return -1;
                }
            }
            else
            {
                char basename[PATH_MAX];
                strcpy(basename, slash + 1);

                if (slash == copy)
                    slash[1] = '\0';
                else
                    *slash = '\0';

                if (!realpath(copy, parent))
                    return -1;
                if (snprintf(m->log_path, sizeof(m->log_path), "%s/%s", parent, basename) >=
                    (int)sizeof(m->log_path))
                {
                    errno = ENAMETOOLONG;
                    return -1;
                }
            }
        }
    }
    else
    {
        if (snprintf(m->log_path, sizeof(m->log_path), "%s/figger.log", m->root) >= 
            (int)sizeof(m->log_path))
        {
            errno = ENAMETOOLONG;
            return -1;
        }
    }

    m->log_file = fopen(m->log_path, "a");
    if (!m->log_file)
        return -1;

    setvbuf(m->log_file, NULL, _IOLBF, 0);
    return 0;
}

void logger_close(struct monitor *m)
{
    if (m->log_file)
    {
        fclose(m->log_file);
        m->log_file = NULL;
    }
}

// function to write a log line to a file
void log_message(struct monitor *m, const char *format, ...)
{
    if (!m || !m->log_file)
        return;

        time_t now = time(NULL);
        struct tm tm_now;
        char timestamp[32] = "unknown-time";

        if (localtime_r(&now, &tm_now))
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &tm_now);

        fprintf(m->log_file, "[%s] ", timestamp);
        va_list ap;
        va_start(ap, format);
        vfprintf(m->log_file, format, ap);
        va_end(ap);
        fputc('\n', m->log_file);
}

int is_log_path(const struct monitor *m, const char *path)
{
    return m && path && m->log_path[0] && strcmp(m->log_path, path) == 0;
}