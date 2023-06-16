#ifndef _MY_DATAUTILS_H
#define _MY_DATAUTILS_H

#include "project/typedefs.h"

level_t **load_data(void);
void 	  save_data(level_t **levels);
int 	  store_pass(char *pass, char *levelname, level_t *level, level_t **leveldata);
void 	  free_levels(level_t **levels);

#endif /* _MY_DATAUTILS_H */
