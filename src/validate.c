#include "validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "typedefs.h"

const game_t games[NUM_GAMES] = {
	{"bandit", 0, BANDIT_MAX_LEVEL},
	{"natas", 0, BANDIT_MAX_LEVEL},
	{"leviathan", 0, BANDIT_MAX_LEVEL},
	{"krypton", 0, BANDIT_MAX_LEVEL},
	{"narnia", 0, BANDIT_MAX_LEVEL},
	{"behemoth", 0, BANDIT_MAX_LEVEL},
	{"utumno", 0, BANDIT_MAX_LEVEL},
	{"maze", 0, BANDIT_MAX_LEVEL},
	{"vortex", 0, BANDIT_MAX_LEVEL},
	{"manpage", 0, BANDIT_MAX_LEVEL},
	{"drifter", 0, BANDIT_MAX_LEVEL},
	{"formulaone", 0, BANDIT_MAX_LEVEL}
};

int is_valid_levelname(char *level_name)
{
	printf("%s == test\n", level_name);
	return 1;
}

int is_valid_levelnum(int level_num)
{
	level_num = 12;
	printf("%d == test\n", level_num);
	return 1;
}

int
is_valid_level(arg_t *split_arg)
{
	/* Returns 1 for a valid level, else returns 0 */
	if (!is_valid_levelname(split_arg->level_name)) {
		puts("Invalid level name.");
		return 0;
	}
	if (!is_valid_levelnum(split_arg->level_num)) {
		puts("Invalid level number.");
		return 0;
	}
	return 1;
}

int
get_level_info(arg_t *split_arg, level_t *level)
{
	memcpy(level->level_name, split_arg->level_name, LVLNAME_MAX);
	/* Ensure that the level name is a null terminated string */
	level->level_name[LVLNAME_MAX - 1] = '\0';
	level->level_num = split_arg->level_num;
	return 0;
}
