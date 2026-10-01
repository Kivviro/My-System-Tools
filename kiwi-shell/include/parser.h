#ifndef KSH_PARSER_H
#define KSH_PARSER_H

#include <stddef.h>
#include <ctype.h>

int parse_line(char *line, char **argv, size_t max_args);

#endif
