#ifndef KSH_COMPLETION_H
#define KSH_COMPLETION_H

#include <stddef.h>
#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "shell.h"

int completion_complete(char *buf, size_t *len, size_t *cursos);

#endif
