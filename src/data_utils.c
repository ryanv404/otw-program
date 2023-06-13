#include "data_utils.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "constants.h"
#include "typedefs.h"
#include "validate.h"
#include "utils.h"
#include "messages.h"

int
get_pw_if_stored(level_t *level)
{
	typedef struct {
		char level_name[LVLNAME_MAX];
		char pw[LVLPW_MAX];
	} entry_t;

	entry_t entries[3] = {
		{"bandit1", "stored password1"},
		{"bandit2", "stored password2"},
		{"bandit3", "stored password3"}
	};

	/* Handle case where we want to overwrite the old pw */
	if (level->pw != NULL) return 1;

	for (int i = 0; i < 3; i++) {
		if (strcmp(level->level_name, entries[i].level_name) == 0) {
			memcpy(level->pw, entries[i].pw, LVLPW_MAX);
			return 0;
		}
	}

	return 1;
}

int
setup_level(level_t *level)
{
	/* 
	 *	At this point, level should have validated info in the following:
	 *	level->level_name
	 *	level->game_name
	 *	level->level_num
	 *	level->port
	 *	level->is_completed
	 */

	sprintf(level->ssh_addr, "%s@%s.labs.overthewire.org",
			level->level_name, level->game_name);

	if (get_pw_if_stored(level) == 1) {
		/* No stored password */
		return 1;
	}

	/* Found stored password */
	return 0;
}

int
store_pw(level_t *level)
{
	/* Split level arg into name and number and determine if it's valid. */
	if (!is_valid_level(level)) {
		quit("Invalid level input");
	}

	/* Validate user provided level */
	if (is_valid_level(level) != 0) quit(ERR_BAD_LEVEL_ARG);

	/* Load level info */
	setup_level(level);

	/* Store the pw */
	puts("new password was written to file.");

	print_level(level);

	exit(EXIT_SUCCESS);
}
