/* datautils.c - OTW program */

#include "project/datautils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/stat.h>

#include "project/typedefs.h"
#include "project/validate.h"
#include "project/error_msgs.h"
#include "project/utils.h"

#define DATADIR			"data"
#define DATAFILE        "data/otw_data.dat"
#define TEMP_DATAFILE   DATAFILE	"~"
#define TOTAL_GAMES		12
#define TOTAL_LEVELS	183

level_t **create_new_datafile(void);
level_t **init_data_structs(void);

const init_level_t init_levels[TOTAL_GAMES] = {
	{"bandit",  2220, 34}, {"natas",      0, 34}, {"leviathan",  2223,  7},
	{"krypton", 2231,  7}, {"narnia",  2226,  9}, {"behemoth",   2221,  8},
	{"utumno",  2227,  8}, {"maze",    2225,  9}, {"vortex",     2228, 27},
	{"manpage", 2224,  7}, {"drifter", 2230, 15}, {"formulaone", 2223,  6}
};

level_t **
create_new_datafile(void)
{
	int level_index;
	char namebuf[100];
	struct stat sb;

	stat(DATADIR, &sb);

	if (!S_ISDIR(sb.st_mode)) {
		if (mkdir(DATADIR, 0755) == -1) {
			fprintf(stderr, "[-] Unable to create a data directory in the current working directory.\n");
			exit(EXIT_FAILURE);
		}
	}

	level_t **all_levels = init_data_structs();

	/* Assign default values to the fields of each level */
	level_index = 0;
	for (int i = 0; i < TOTAL_GAMES; i++) {
		for (int j = 0; j <= init_levels[i].maxlevel; j++) {
			/* Level name */
			sprintf(namebuf, "%s%d", init_levels[i].gamename, j);
			strncpy(all_levels[level_index]->levelname, namebuf, MAX_NAME_WIDTH);
			/* Game name */
			strncpy(all_levels[level_index]->gamename, init_levels[i].gamename, MAX_NAME_WIDTH);
			/* Password */
			strncpy(all_levels[level_index]->pass, "?", 2);
			/* Port */
			all_levels[level_index]->port = (uint32_t) init_levels[i].port;
			/* Is password saved */
			all_levels[level_index]->is_pass_saved = (uint8_t) 0;
			/* Is level complete */
			all_levels[level_index]->is_level_complete = (uint8_t) 0;
			level_index++;
		}
	}

	save_data(all_levels);
	return all_levels;
}

int
store_pass(char *pass, char *levelname, level_t *level, level_t **all_levels)
{
	int namelen, passlen, idx;

	/* Handle user-supplied level name string carefully */
	namelen = (int) strnlen(levelname, MAX_NAME_WIDTH);
	memcpy(level->levelname, levelname, namelen);
	level->levelname[MAX_NAME_WIDTH - 1] = '\0';

	/* Validate user provided level; returns index of matched level in all_levels array */
	if ((idx = is_valid_level(level, all_levels)) == -1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Handle user-supplied password string carefully */
	passlen = (int) strnlen(pass, MAX_PASS_WIDTH);
	memcpy(all_levels[idx]->pass, pass, passlen);
	all_levels[idx]->pass[MAX_PASS_WIDTH - 1] = '\0';
	all_levels[idx]->is_pass_saved = (uint8_t) 1;

	/* Must beat level prior to this one to get this password so mark it as complete */
	// all_levels[idx - 1]->is_level_complete = (uint8_t) 1; /* TODO but not on the 0th level! */ 

	/* Write pw to data file */
	save_data(all_levels);

	printf("[+] Saved %s's password.\n", all_levels[idx]->levelname);
	return 0;
}

level_t **
init_data_structs(void)
{
	level_t **all_levels = (level_t **) calloc(TOTAL_LEVELS, sizeof(level_t *));
	if (all_levels == NULL) {
		fprintf(stderr, ERR_BAD_MALLOC);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < TOTAL_LEVELS; i++) {
		all_levels[i] = (level_t *) calloc(1, sizeof(level_t));
		if (all_levels[i] == NULL) {
			fprintf(stderr, ERR_BAD_MALLOC);
			exit(EXIT_FAILURE);
		}
	}

	return all_levels;
}

level_t **
load_data(void)
{
	level_t **all_levels = NULL;

	if (access(DATAFILE, F_OK) != 0) {
		all_levels = create_new_datafile();
	} else {
		all_levels = init_data_structs();
	}

	FILE *infile = fopen(DATAFILE, "rb");
	if (infile == NULL) {
		fprintf(stderr, ERR_FOPEN_FAILED);
		free_levels(all_levels);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < TOTAL_LEVELS; i++) {
		fread(all_levels[i], sizeof(level_t), 1, infile);
	}

	fclose(infile);
	return all_levels;
}

void
save_data(level_t **all_levels)
{
	FILE *outfile = fopen(TEMP_DATAFILE, "wb");

	if (outfile == NULL) {
		fprintf(stderr, ERR_FOPEN_FAILED);
		free_levels(all_levels);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < TOTAL_LEVELS; i++) {
		fwrite(all_levels[i], sizeof(level_t), 1, outfile);
	}

	if (rename(TEMP_DATAFILE, DATAFILE)) {
		fprintf(stderr, "[-] Error while processing the data file.\n");
	}
	
	fclose(outfile);
	return;
}

void
free_levels(level_t **all_levels)
{
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		free(all_levels[i]);
	}
	free(all_levels);
	return;
}

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

int
show_progress(level_t **all_levels)
{
	int tot_num_complete, num_complete, total_levels, width;
	int levels_index = 0;
	float perc;

	puts("\n*********************");
	puts("*   Your progress   *");
	puts("*********************\n");

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
			printf("%*d/%-2d levels completed (%.0f%% complete).", width,
					num_complete, total_levels, perc);
		}
		printf("\n");
	}

	if (tot_num_complete == TOTAL_LEVELS) {
		printf("\n[[[[!!!::+ FINISHED +::!!!]]]] :-D\n");
	} else {
		printf("\n[+ OVERALL +]\n");
		perc =  (((float) tot_num_complete / (float) TOTAL_LEVELS) * 100);
		printf("You've completed %d %s and have %d more to go (%.1f%% complete).\n", tot_num_complete,
				tot_num_complete == 1 ? "level" : "levels", TOTAL_LEVELS - tot_num_complete, perc);
	}
	printf("\n");

	return 0;
}
