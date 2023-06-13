#ifndef _MY_UTILS_H
#define _MY_UTILS_H

#include "typedefs.h"

int quit(char *msg);
int show_usage(void);
int show_help(void);
int get_timestamp(char *buf, int bufsize);
int print_game(game_t *game);
int print_level(level_t *level);

#endif /* _MY_UTILS_H */
