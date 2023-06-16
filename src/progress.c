/* progress.c - OTW program */

#include "project/progress.h"

#include <stdio.h>
#include <stdlib.h>

#include "project/typedefs.h"
#include "project/datautils.h"
#include "project/validate.h"
#include "project/messages.h"
#include "project/utils.h"

int
mark_level_complete(level_t *level, level_t **leveldata)
{
	int idx;

	if ((idx = is_valid_level(level, leveldata)) == -1) {
		free(level);
		free_levels(leveldata);
		quit(ERR_BAD_LEVEL_ARG);
	}
	
	leveldata[idx]->is_level_complete = 1;
    printf("[+] Level %s is now marked as complete.\n", leveldata[idx]->levelname);

	save_data(leveldata);
	return 0;
}

int
mark_game_complete(level_t *level, level_t **leveldata)
{
	int idx;

	if ((idx = is_valid_level(level, leveldata)) == -1) {
		free(level);
		free_levels(leveldata);
		quit(ERR_BAD_LEVEL_ARG);
	}
	
	leveldata[idx]->is_game_complete = 1;
    printf("[+] The %s wargame is now marked as complete.\n", level->gamename);

	save_data(leveldata);
	return 0;
}

int
show_progress(void)
{
	puts("[Progress] You're doin great bud!");
	exit(EXIT_SUCCESS);
}
