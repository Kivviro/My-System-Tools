#define _POSIX_C_SOURCE 200809L

#include "terminal.h"
#include "history.h"
#include "shell.h"

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

int main(void)
{
    atexit(terminal_restore_at_exit);

    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    int result = shell_run();

    history_free();

    return result;
}
