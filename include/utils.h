#ifndef _MY_UTILS_H
#define _MY_UTILS_H

#include "typedefs.h"

int quit(char *msg);
int show_usage(void);
int show_help(void);
void print_leveldata(level_t **level);
void print_level(level_t *level);

#endif /* _MY_UTILS_H */
