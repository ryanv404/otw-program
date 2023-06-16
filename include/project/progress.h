#ifndef _MY_PROGRESS_H
#define _MY_PROGRESS_H

#include "typedefs.h"

int show_progress(void);
int mark_level_complete(level_t *level, level_t **all_levels);

#endif /* _MY_PROGRESS_H */
