#include "validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "typedefs.h"
#include "datautils.h"

int
is_valid_level(level_t *level, level_t **leveldata)
{
	/* Validate user provided level argument */
	for (int i = 0; i < NUM_LEVELS; i++) {
		if (strncmp(leveldata[i]->levelname, level->levelname, LVLNAME_MAX) == 0) {
			/* Level arg has been validated; copy saved data for this level into the remaining fields */
			memcpy(level, leveldata[idx]);
			free_levels(leveldata);
			return 0;
		}
	}
	free_levels(leveldata);
	return 1;
}
