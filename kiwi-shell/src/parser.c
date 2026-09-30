#include "../include/parser.h"

int parse_line(char *line, char **argv, size_t max_args)
{
    size_t argc = 0;
    char *p = line;

    while (*p && argc + 1 < max_args)
    {
        while (isspace((unsigned char)*p))
            ++p;
        
        if (!*p)
            break;

        argv[argc++] = p;

        char *out = p;
        char quote = 0;

        while (*p)
        {
            if (quote)
            {
                if (*p == quote)
                {
                    quote = 0;
                    ++p;
                }
                else if (*p == '\\' && p[1])
                {
                    ++p;
                    *out++ = *p++;
                }
                else
                {
                    *out++ = *p++;
                }

                continue;
            }

            // outside quotes
            if (*p == '\'' || *p == '"')
            {
                quote = *p++;
            }
            // escape
            else if (*p == '\\' && p[1])
            {
                ++p;
                *out++ = *p++;
            }
            // arg separator
            else if (isspace((unsigned char)*p))
            {
                ++p;
                break;
            }
            else
            {
                *out++ = *p++;
            }
        }

        *out = '\0';
    }

    argv[argc] = NULL;

    return (int)argc;
}
