#ifndef _MY_MISC_H
#define _MY_MISC_H

#include "lssh/config.h"

int  quit(char *msg);
void show_usage(void);
void show_help(void);
void print_all_levels(level_t **all_levels);
void print_level(level_t *level);
void print_hex_fingerprint(const char *buf, int len);

#endif /* _MY_MISC_H */
