#ifndef KSH_EXECUTOR_H
#define KSH_EXECUTOR_H

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

void executor_execute(char **argv);

#endif
