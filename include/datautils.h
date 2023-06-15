#ifndef MY_DATAUTILS_H
#define MY_DATAUTILS_H

#include "typedefs.h"

void create_new_datafile(void);
void read_datafile(level_t **levels);
void write_to_datafile(level_t **levels);
void free_levels(level_t **levels);
int store_pass(level_t *level, level_t **leveldata);
level_t **load_saved_data(void);

#endif /* MY_DATAUTILS_H */
