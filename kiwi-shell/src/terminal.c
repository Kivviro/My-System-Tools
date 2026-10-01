#define _POSIX_C_SOURCE 200809L

#include "terminal.h"

static struct termios original_termios;
static int raw_enabled = 0;

int terminal_enable_raw_mode(void)
{
    if (!isatty(STDIN_FILENO))
        return -1;

    if (tcgetattr(STDIN_FILENO, &original_termios) == -1)
        return -1;

    struct termios raw = original_termios;

    raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    raw.c_iflag &= ~(IXON | ICRNL | BRKINT | INPCK | ISTRIP);
    raw.c_oflag &= ~(OPOST);
    raw.c_cflag |= CS8;
    raw.c_cc[VMIN] = 1;
    raw.c_cc[VTIME] = 0;

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        return -1;

    raw_enabled = 1;

    return 0;
}

void terminal_disable_raw_mode(void)
{
    if (raw_enabled)
    {
        tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios);
        raw_enabled = 0;
    }
}

void terminal_restore_at_exit(void)
{
    terminal_disable_raw_mode();
}
