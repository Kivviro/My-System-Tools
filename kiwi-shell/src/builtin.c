#define _POSIX_C_SOURCE 200809L

#include "builtin.h"

#include "history.h"
#include "terminal.h"

static int builtin_cd(char **argv)
{
    const char *dir = argv[1];

    if (dir == NULL)
        dir = getenv("HOME");

    if (dir == NULL)
    {
        fprintf(stderr, "ksh: cd: HOME is not set\n");
        
        return 1;
    }

    if (chdir(dir) == -1)
    {
        fprintf(stderr, "ksh: cd: %s: failed\n", dir);

        return 1;
    }

    return 0;
}

static int builtin_pwd(void)
{
    char cwd[PATH_MAX];

    if (getcwd(cwd, sizeof(cwd)))
    {
        puts(cwd);
        return 0;
    }

    perror("ksh: pwd");

    return 1;
}

static int builtin_history(void)
{
    for (size_t i = 0; i < history_count(); ++i)
    {
        printf("%4zu %s\n", i + 1, history_get(i));
    }

    return 0;
}

static int builtin_exit(char **argv)
{
    int status = 0;

    if (argv[1] != NULL)
        status = atoi(argv[1]);

    terminal_disable_raw_mode();
    history_free();

    exit(status);
}

int builtin_execute(char **argv)
{
    if (argv == NULL || argv[0] == NULL)
    {
        return 0;
    }

    if (strcmp(argv[0], "cd") == 0)
        return builtin_cd(argv);

    if (strcmp(argv[0], "pwd") == 0)
        return builtin_pwd();

    if (strcmp(argv[0], "history") == 0)
        return builtin_history();

    if (strcmp(argv[0], "exit") == 0)
        return builtin_exit(argv);

    return -1;
}
