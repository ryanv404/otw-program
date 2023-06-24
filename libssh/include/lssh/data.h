#ifndef _MY_DATA_H
#define _MY_DATA_H

#include "lssh/config.h"

level_t **load_data(void);
void save_data(level_t **all_levels);
int  store_pass(char *pass, char *levelname, level_t *level, level_t **all_levels);
void free_levels(level_t **all_levels);
void get_level_pass(level_t *level);

#endif /* _MY_DATA_H */
