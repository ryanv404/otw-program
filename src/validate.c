#include "validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "typedefs.h"
#include "messages.h"
#include "utils.h"

#include "data_utils.h"

const game_t games[NUM_GAMES] = {
	{"bandit", 		BANDIT_PORT, 	BANDIT_MAX_LEVEL, 		0},
	{"natas", 		0, 				NATAS_MAX_LEVEL, 		0},
	{"leviathan", 	LEVIATHAN_PORT, LEVIATHAN_MAX_LEVEL, 	0},
	{"krypton", 	KRYPTON_PORT, 	KRYPTON_MAX_LEVEL, 		0},
	{"narnia", 		NARNIA_PORT, 	NARNIA_MAX_LEVEL, 		0},
	{"behemoth", 	BEHEMOTH_PORT, 	BEHEMOTH_MAX_LEVEL, 	0},
	{"utumno", 		UTUMNO_PORT, 	UTUMNO_MAX_LEVEL, 		0},
	{"maze", 		MAZE_PORT, 		MAZE_MAX_LEVEL, 		0},
	{"vortex", 		VORTEX_PORT, 	VORTEX_MAX_LEVEL, 		0},
	{"manpage", 	MANPAGE_PORT, 	MANPAGE_MAX_LEVEL, 		0},
	{"drifter", 	DRIFTER_PORT, 	DRIFTER_MAX_LEVEL, 		0},
	{"formulaone", 	FORMULAONE_PORT, FORMULAONE_MAX_LEVEL, 	0}
};

int
is_valid_level(level_t *level)
{
	int ret, idx;

	/* Split the level argument into name and number parts */
	ret = sscanf(level->level_name, SCAN_FMTSTR,
				level->game_name, &level->level_num);
	
	if (ret != 2) quit(ERR_BAD_LEVEL_ARG);

	/* Validate level name */
	idx = -1;
	for (int i = 0; i < NUM_GAMES; i++) {
		if (strcmp(games[i].game_name, level->game_name) == 0) {
			idx = i;
			break;
		}
	}

	if (idx == -1) quit(ERR_BAD_LEVEL_ARG);

	/* Validate level number for chosen game */
	if (!(level->level_num >= 0) || !(level->level_num <= games[idx].max_level)) {
		quit(ERR_BAD_LEVEL_ARG);
	}

	level->is_completed = games[idx].is_completed;
	level->port = games[idx].port;

	return 0;
}
