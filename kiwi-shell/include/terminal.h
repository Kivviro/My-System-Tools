#ifndef KSH_TERMINAL_H
#define KSH_TERMINAL_H

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <unistd.h>

int terminal_enable_raw_mode(void);
void terminal_disable_raw_mode(void);
void terminal_restore_at_exit(void);

#endif
