/* progress.c - OTW program */

#include "project/progress.h"

#include <stdio.h>
#include <stdlib.h>

#include "project/typedefs.h"
#include "project/datautils.h"
#include "project/validate.h"
#include "project/error_msgs.h"
#include "project/utils.h"

int
mark_level_complete(level_t *level, level_t **all_levels)
{
	int idx;

	if ((idx = is_valid_level(level, all_levels)) == -1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}
	
	all_levels[idx]->is_level_complete = 1;
    printf("[+] Level %s is now marked as complete.\n", all_levels[idx]->levelname);

	save_data(all_levels);
	return 0;
}

int
show_progress(void)
{
	puts("[Progress] You're doin great bud!");
	exit(EXIT_SUCCESS);
}
