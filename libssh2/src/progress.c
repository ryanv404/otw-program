/* progress.c - OTW program */

#include "lssh2/progress.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lssh2/config.h"
#include "lssh2/misc.h"
#include "lssh2/error_msgs.h"
#include "lssh2/data.h"
#include "lssh2/validate.h"

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
    printf("[+] %s is now marked complete.\n", all_levels[idx]->levelname);

	save_data(all_levels);
	return 0;
}

void
show_progress(level_t **all_levels)
{
	float perc;
	int tot_num_complete, num_complete, total_levels, width;

	int levels_index = 0;

	/* Default info for each level */
	init_level_t init_levels[TOTAL_GAMES] = {
	    {"bandit",    "2220", 34}, {"natas",         "0", 34},
	    {"leviathan", "2223",  7}, {"krypton",    "2231",  7},
	    {"narnia",    "2226",  9}, {"behemoth",   "2221",  8},
	    {"utumno",    "2227",  8}, {"maze",       "2225",  9},
	    {"vortex",    "2228", 27}, {"manpage",    "2224",  7},
	    {"drifter",   "2230", 15}, {"formulaone", "2232",  6}
	};

	printf("\n"
	       "*********************"
	       "*   Your progress   *"
	       "*********************"
	       "\n");

	tot_num_complete = 0;
	for (int i = 0; i < TOTAL_GAMES; i++) {
		num_complete = 0;
		total_levels = init_levels[i].maxlevel + 1;
		for (int x = 0; x < total_levels; x++) {
			if (all_levels[levels_index]->is_level_complete) {
				tot_num_complete++;
				num_complete++;
			}
			levels_index++;
		}
		
		printf("[%s] ", init_levels[i].gamename);
		if (num_complete == total_levels) {
			width = 12 - strlen(init_levels[i].gamename);

			printf("%*s%s", width, "[", ":+ COMPLETE +:] \\o/");
		} else {
			width = 12 - strlen(init_levels[i].gamename);
			perc = (((float) num_complete / (float) total_levels) * 100);

			printf("%*d/%-2d levels completed (%.0f%% complete).",
					width, num_complete, total_levels, perc);
		}
		printf("\n");
	}

	if (tot_num_complete == TOTAL_LEVELS) {
		printf("\n[[[[!!!::+ FINISHED +::!!!]]]] :-D\n");
	} else {
		printf("\n[+ OVERALL +]\n");
		perc =  (((float) tot_num_complete / (float) TOTAL_LEVELS) * 100);
		printf("You've completed %d %s and have %d more to go (%.1f%% complete).\n",
				tot_num_complete, tot_num_complete == 1 ? "level" : "levels",
				TOTAL_LEVELS - tot_num_complete, perc);
	}
	printf("\n");

	return;
}
