#define _POSIX_C_SOURCE 200809L

#include "history.h"

static char *history[MAX_HISTORY];
static size_t history_len = 0;

void history_add(const char *line)
{
    if (line == NULL || *line == '\0')
        return;

    if (history_len > 0 && strcmp(history[history_len - 1], line) == 0)
        return;

    if (history_len == MAX_HISTORY)
    {
        free(history[0]);
        memmove(history, history + 1, sizeof(history[0]) * (MAX_HISTORY - 1));
        history_len--;
    }

    history[history_len] = strdup(line);

    if (history[history_len] != NULL)
        history_len++;
}

void history_free(void)
{
    for (size_t i = 0; i < history_len; ++i)
        free(history[i]);

    history_len = 0;
}

size_t history_count(void)
{
    return history_len;
}

const char *history_get(size_t index)
{
    if (index >= history_len)
        return NULL;

    return history[index];
}
