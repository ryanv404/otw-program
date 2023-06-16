#ifndef _MY_UTILS_H
#define _MY_UTILS_H

#include "project/typedefs.h"

int quit(char *msg);
void show_usage(void);
void show_help(void);
void print_all_levels(level_t **all_levels);
void print_level(level_t *level);

#endif /* _MY_UTILS_H */
