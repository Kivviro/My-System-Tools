#define _GNU_SOURCE
#include "monitor.h"

#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

volatile sig_atomic_t g_stop = 0;

static void handle_signal(int signo)
{

    (void)signo;
    g_stop = 1;
}

static void usage(const char *program)
{
    fprintf(stderr, "Usage: %s DIRECTORY [LOGFILE]\n", program);
    fprintf(stderr, "Example: %s ./monitored-dir ./figger.log\n", program);
}

int main(int argc, char **argv)
{
    struct monitor m;
    struct stat st;
    char resolved[PATH_MAX];
    struct sigaction sa;

    if (argc < 2 || argc > 3)
    {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    memset(&m, 0, sizeof(m));
    m.inotify_fd = -1;
    m.fanotify_fd = -1;

    if (!realpath(argv[1], resolved))
    {
        perror("realpath directory");
        return EXIT_FAILURE;
    }

    if (stat(resolved, &st) < 0)
    {
        perror("stat directory");
        return EXIT_FAILURE;
    }

    if (!S_ISDIR(st.st_mode))
    {
        fprintf(stderr, "figger: not a directory: %s\n", resolved);
        return EXIT_FAILURE;
    }

    if (strlen(resolved) >= sizeof(m.root))
    {
        fprintf(stderr, "figger: directory parth is too long\n");
        return EXIT_FAILURE;
    }
    strcpy(m.root, resolved);

    if (logger_open(&m, argc == 3 ? argv[2] : NULL) < 0)
    {
        perror("open log file");
        return EXIT_FAILURE;
    }

    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_signal;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGINT, &sa, NULL) < 0 || sigaction(SIGTERM, &sa, NULL) < 0)
    {
        perror("sigaction");
        logger_close(&m);
        return EXIT_FAILURE;
    }

    if (inotify_monitor_init(&m) < 0)
    {
        perror("inotify initialization");
        logger_close(&m);
        return EXIT_FAILURE;
    }

    if (fanotify_monitor_init(&m) < 0)
    {
        perror("fanotify initialization");
        inotify_monitor_close(&m);
        logger_close(&m);
        return EXIT_FAILURE;
    }

    if (inotify_add_tree(&m, m.root) < 0)
    {
        perror("initial recursive watch setup");
        fanotify_monitor_close(&m);
        inotify_monitor_close(&m);
        logger_close(&m);
        return EXIT_FAILURE;
    }

    log_message(&m, "START root=%s log=%s", m.root, m.log_path);
    printf("Monitoring: %s\nLog: 5s\nPress CTRL-C to stop\n", m.root, m.log_path);

    while (!g_stop)
    {
        struct pollfd fds[2] = {
            { .fd = m.inotify_fd, .events = POLLIN },
            { .fd = m.fanotify_fd, .events = POLLIN }
        };
        int rc = poll(fds, 2, -1);

        if (rc < 0)
        {
            if (errno == EINTR)
                continue;
            perror("poll");
            log_message(&m, "ERROR poll: %s", strerror(errno));
            break;
        }

        if (fds[0].revents & POLLIN)
            inotify_handle_events(&m);
        if (fds[1].revents & POLLIN)
            fanotify_handle_events(&m);

        if ((fds[0].revents | fds[1].revents) & (POLLERR | POLLHUP | POLLNVAL))
        {
            log_message(&m, "ERROR poll reported an invalid or closed descriptor");
            break;
        }
    }

    log_message(&m, "STOP");
    fanotify_monitor_close(&m);
    inotify_monitor_close(&m);
    logger_close(&m);
    return EXIT_SUCCESS;
}