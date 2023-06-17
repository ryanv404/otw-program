#ifndef _MY_DATAUTILS_H
#define _MY_DATAUTILS_H

#include "project/typedefs.h"

level_t **load_data(void);
void 	  save_data(level_t **all_levels);
int 	  store_pass(char *pass, char *levelname, level_t *level, level_t **all_levels);
void 	  free_levels(level_t **all_levels);
int       show_progress(level_t **all_levels);
int       mark_level_complete(level_t *level, level_t **all_levels);

#endif /* _MY_DATAUTILS_H */
