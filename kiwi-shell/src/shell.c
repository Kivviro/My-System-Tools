#define _POSIX_C_SOURCE 200809L

#include "shell.h"
#include "completion.h"
#include "history.h"
#include "parser.h"
#include "terminal.h"
#include "executor.h"

static void refresh_line(const char *prompt, const char *buf, size_t cursor)
{
    printf("\r\033[2K");
    printf("%s%s", prompt, buf);
    printf("\r\033[%zuC", strlen(prompt) + cursor);

    fflush(stdout);
}

static char *read_line(const char *prompt)
{
    char buf[KSH_MAX_LINE];

    size_t len = 0;
    size_t cursor = 0;

    buf[0] = '\0';

    int history_pos = (int)history_count();

    if (terminal_enable_raw_mode() == -1)
    {
        char *line = NULL;
        size_t capacity = 0;

        printf("%s", prompt);
        fflush(stdout);

        size_t n = getline(&line, &capacity, stdin);

        if (n > 0 && line[n - 1] == '\n')
        {
            line[n - 1] = '\0';
        }

        if (n < 0)
        {
            free(line);
            return NULL;
        }

        return line;
    }

    printf("%s", prompt);
    fflush(stdout);

    for (;;)
    {
        unsigned char c;

        ssize_t n = read(STDIN_FILENO, &c, 1);

        if (n <= 0)
        {
            terminal_disable_raw_mode();

            return NULL;
        }

        // Enter
        if (c == '\r' || c == '\n')
        {
            buf[len] = '\0';
            printf("\r\n");
            fflush(stdout);
            terminal_disable_raw_mode();
            
            return strdup(buf);
        }

        // ctrl+D
        if (c == 4)
        {
            if (len == 0)
            {
                putchar('\n');
                terminal_disable_raw_mode();

                return NULL;
            }

            continue;
        }

        // ctrl+C
        if (c == 3)
        {
            printf("^C\n");
            terminal_disable_raw_mode();

            return strdup("");
        }

        // tab
        if (c == '\t')
        {
            completion_complete(buf, &len, &cursor);
            refresh_line(prompt, buf, cursor);

            continue;
        }

        // backspace
        if (c == 127 || c == 8)
        {
            if (cursor > 0)
            {
                memmove(buf + cursor - 1, buf + cursor, len - cursor + 1);

                --cursor;
                --len;

                refresh_line(prompt, buf, cursor);
            }

            continue;
        }

        //esc
        if (c == 27)
        {
            unsigned char seq[2];

            if (read(STDIN_FILENO, &seq[0], 1) != 1)
            {
                continue;
            }

            if (seq[0] != '[')
                continue;

            if (read(STDIN_FILENO, &seq[1], 1) != 1)
                continue;

            // left 
            if (seq[1] == 'D')
            {
                if (cursor > 0)
                    --cursor;

                refresh_line(prompt, buf, cursor);
            }

            // right
            else if(seq[1] == 'C')
            {
                if (cursor < len)
                    ++cursor;

                refresh_line(prompt, buf, cursor);
            }

            // up
            else if (seq[1] == 'A')
            {
                if (history_count() > 0 && history_pos > 0)
                {
                    --history_pos;

                    const char *item = history_get(history_pos);
                    strncpy(buf, item, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                    len = strlen(buf);
                    cursor = len;
                    refresh_line(prompt, buf, cursor);
                }
            }
            // down
            else if (seq[1] == 'B')
            {
                if (history_pos < (int)history_count() -1)
                {
                    ++history_pos;

                    const char *item = history_get(history_pos);
                    strncpy(buf, item, sizeof(buf) - 1);
                    buf[sizeof(buf) - 1] = '\0';
                    len = strlen(buf);
                    cursor = len;

                    refresh_line(prompt, buf, cursor);
                }
                // return to empty line
                else if (history_pos == (int)history_count() - 1)
                {
                    ++history_pos;
                    len = 0;
                    cursor = 0;

                    buf[0] = '\0';

                    refresh_line(prompt, buf, cursor);
                }
            }

            continue;
        }

        // printable char
        if (isprint((unsigned char)c))
        {
            if (len + 1 < sizeof(buf))
            {
                memmove(buf + cursor + 1, buf + cursor, len - cursor + 1);
                buf[cursor] = (char)c;

                ++cursor;
                ++len;

                refresh_line(prompt, buf, cursor);
            }
        }
    }
}

static void build_prompt(char *prompt, size_t size)
{
    char cwd[PATH_MAX];

    if (!getcwd(cwd, sizeof(cwd)))
            strcpy(cwd, "?");

    const char *home = getenv("HOME");
    const char *user = getenv("USER");

    if (home != NULL && strcmp(cwd, home) == 0)
    {
        snprintf(prompt, size, "%s:~$ ", user ? user : "ksh");
    }
    else if (home != NULL && strncmp(cwd, home, strlen(home)) == 0)
    {
        snprintf(prompt, size, "%s:~%s$ ", user ? user : "ksh", cwd + strlen(home));
    }
    else
    {
        snprintf(prompt, size, "%s:%s$ ", user ? user : "ksh", cwd);
    }
}

int shell_run(void)
{
    for (;;)
    {
        char prompt[PATH_MAX + 64];

        build_prompt(prompt, sizeof(prompt));
        char *line = read_line(prompt);

        if (line == NULL)
            break;

        if (*line == '\0')
        {
            free(line);
            continue;
        }

        history_add(line);

        char *argv[KSH_MAX_ARGS];
        parse_line(line, argv, KSH_MAX_ARGS);

        executor_execute(argv);

        free(line);
    }

    return 0;
}
