#ifndef _MY_PROGRESS_H
#define _MY_PROGRESS_H

#include "lssh/config.h"

void show_progress(level_t **all_levels);
int  mark_level_complete(level_t *level, level_t **all_levels);

#endif /* _MY_PROGRESS_H */
