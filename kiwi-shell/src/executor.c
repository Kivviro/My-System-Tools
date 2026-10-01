#define _POSIX_C_SOURCE 200809L

#include "executor.h"

#include "builtin.h"

void executor_execute(char **argv)
{
    if (argv == NULL || argv[0] == NULL)
    {
        return;
    }

    int builtin = builtin_execute(argv);

    if (builtin != -1)
        return;

    pid_t pid = fork();

    if (pid == -1)
    {
        perror("ksh: fork");

        return;
    }

    if (pid == 0)
    {
        signal(SIGINT, SIG_DFL);
        signal(SIGQUIT, SIG_DFL);

        execvp(argv[0], argv);

        fprintf(stderr, "ksh: %s: %s\n", argv[0], strerror(errno));

        _exit(127);
    }

    int status;

    do {
        if (waitpid(pid, &status, 0) == -1)
        {
            if (errno == EINTR)
                continue;

            perror("ksh: waitpid");

            break;
        }
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}
