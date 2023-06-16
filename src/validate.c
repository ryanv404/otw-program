/* validate.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "validate.h"
#include "constants.h"
#include "typedefs.h"

int
is_valid_level(level_t *level, level_t **leveldata)
{
	/* Validate user provided level argument */
	for (int i = 0; i < NUM_LEVELS; i++) {
		if (strncmp(leveldata[i]->levelname, level->levelname, LVLNAME_MAX) == 0) {
			/* Level arg is now validated */
			memcpy(level, leveldata[i], sizeof(level_t));
			return 0;
		}
	}
	return 1;
}
