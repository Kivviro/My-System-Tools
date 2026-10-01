#ifndef KSH_SHELL_H
#define KSH_SHELL_H

#define KSH_MAX_LINE 4096
#define KSH_MAX_ARGS 128

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

int shell_run(void);

#endif
