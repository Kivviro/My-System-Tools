#define _POSIX_C_SOURCE 200809L

#include "completion.h"

static int common_prefix_len(char **items, size_t count)
{
    if (count == 0)
        return 0;

    size_t n = strlen(items[0]);

    for (size_t i = 1; i < count; ++i)
    {
        size_t j = 0;

        while (j < n && j < strlen(items[i]) && items[0][j] == items[i][j])
        {
            ++j;
        }

        n = j;
    }

    return (int)n;
}

int completion_complete(char *buf, size_t *len, size_t *cursor)
{
    size_t start = *cursor;

    while (start > 0 && buf[start - 1] != ' ' && buf[start - 1] != '\t')
    {
        --start;
    }

    char token[PATH_MAX];

    size_t token_len = *cursor - start;

    if (token_len >= sizeof(token))
        return 0;

    memcpy(token, buf + start, token_len);

    token[token_len] = '\0';

    char dir[PATH_MAX];
    char prefix[PATH_MAX];

    const char *slash = strrchr(token, '/');

    if (slash != NULL)
    {
        size_t dir_len = (size_t)(slash - token) + 1;

        if (dir_len >= sizeof(dir))
            return 0;
        
        memcpy(dir, token, dir_len);

        dir[dir_len] = '\0';

        snprintf(prefix, sizeof(prefix), "%s", slash + 1);
    }
    else 
    {
        snprintf(dir, sizeof(dir), "./");

        snprintf(prefix, sizeof(prefix), "%s", token);
    }

    DIR *d = opendir(dir);

    if (d == NULL)
        return 0;

    char *matches[256];
    size_t count = 0;

    struct dirent *ent;

    size_t prefix_len = strlen(prefix);

    while ((ent = readdir(d)) != NULL && count < 256)
    {
        if (strncmp(ent->d_name, prefix, prefix_len) != 0)
            continue;

        if (ent->d_name[0] == '.' && prefix[0] != '.')
            continue;

        matches[count] = strdup(ent->d_name);

        if (matches[count] != NULL)
            ++count;
    }

    closedir(d);

    if (count == 0)
        return 0;

    // one match

    if (count == 1)
    {
        const char *name = matches[0];

        size_t name_len = strlen(name);
        size_t add = name_len - prefix_len;

        int is_dir = 0;

        char full[PATH_MAX];

        if (strcmp(dir, "./") == 0)
            snprintf(full, sizeof(full), "%s", name);
        else
            snprintf(full, sizeof(full), "%s%s", dir, name);

        struct stat st;

        if (stat(full, &st) == 0 && S_ISDIR(st.st_mode))
        {
            is_dir = 1;
        }

        size_t extra = add + (is_dir ? 1 : 0);

        if (*len + extra < KSH_MAX_LINE)
        {
            memmove(buf + *cursor + extra, buf + *cursor, *len - *cursor + 1);
            memcpy(buf + *cursor, name + prefix_len, add);

            *cursor += add;
            *len += add;

            if (is_dir)
            {
                buf[*cursor] = '/';

                ++*cursor;
                ++*len;
            }
        }

        free(matches[0]);

        return 1;
    }

    int common = common_prefix_len(matches, count);

    if ((size_t)common > prefix_len)
    {
        size_t add = (size_t)common - prefix_len;
        
        if (*len + add < KSH_MAX_LINE)
        {
            memmove(buf + *cursor + add, buf + *cursor, *len - *cursor + 1);
            memcpy(buf + *cursor, matches[0] + prefix_len, add);

            *cursor += add;
            *len += add;
        }
    }
    else
    {
        printf("\a\n");

        for (size_t i = 0; i < count; ++i)
            printf("%s ", matches[i]);

        printf("\n");
    }

    for (size_t i = 0; i < count; ++i)
        free(matches[i]);
    
    return 1;
}
