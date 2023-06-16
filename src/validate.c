/* validate.c - OTW program */

#include "project/validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "project/typedefs.h"

#define TOTAL_LEVELS	183

int
is_valid_level(level_t *level, level_t **all_levels)
{
	/* Validate user provided level argument */
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		if (strncmp(all_levels[i]->levelname, level->levelname, MAX_NAME_WIDTH) == 0) {
			/* Level arg is now validated */
			memcpy(level, all_levels[i], sizeof(level_t));
			return i;
		}
	}
	return -1;
}
