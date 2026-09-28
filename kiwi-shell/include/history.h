#ifndef KIWI_TERMINAL_H
#define KIWI_TERMINAL_H

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#define MAX_HISTORY 100

void history_add(const char *line);
void history_free(void);

size_t history_count(void);
const char *history_get(size_t index);

#endif
