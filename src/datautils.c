/* datautils.c - OTW program */

#include "project/datautils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>

#include "project/typedefs.h"
#include "project/validate.h"
#include "project/error_msgs.h"
#include "project/utils.h"
#include "project/progress.h"

#define DATAFILE        "data/otw-data.dat"
#define TEMP_DATAFILE   DATAFILE	"~"
#define TOTAL_GAMES		12
#define TOTAL_LEVELS	183

level_t **create_new_datafile(void);
level_t **init_data_structs(void);

const init_level_t init_levels[TOTAL_GAMES] = {
	{"bandit",  2220, 34}, {"natas",      0, 34}, {"leviathan",  2223, 7},
	{"krypton", 2231, 7},  {"narnia",  2226, 9},  {"behemoth",   2221, 8},
	{"utumno",  2227, 8},  {"maze",    2225, 9},  {"vortex",     2228, 27},
	{"manpage", 2224, 7},  {"drifter", 2230, 15}, {"formulaone", 2223, 6}
};

level_t **
create_new_datafile(void)
{
	int level_index;
	char namebuf[100];

	level_t **all_levels = init_data_structs();

	level_index = 0;
	for (int i = 0; i < TOTAL_GAMES; i++) {
		for (int j = 0; j < init_levels[i].maxlevel; j++) {
			sprintf(namebuf, "%s%d", init_levels[i].gamename, j);
			strncpy(all_levels[level_index]->levelname, namebuf, MAX_NAME_WIDTH);
			strncpy(all_levels[level_index]->gamename, init_levels[i].gamename, MAX_NAME_WIDTH);
			strncpy(all_levels[level_index]->pass, "?", 2);
			all_levels[level_index]->port = (uint16_t) init_levels[i].port;
			all_levels[level_index]->is_pass_saved = (uint8_t) 0;
			all_levels[level_index]->is_level_complete = (uint8_t) 0;
			level_index++;
		}
	}

	save_data(all_levels);
	printf("[+] Created a local data file.\n");
	return all_levels;
}

int
store_pass(char *pass, char *levelname, level_t *level, level_t **all_levels)
{
	int namelen, passlen, idx;

	namelen = (int) strlen(levelname) + 1;
	passlen = (int) strlen(pass) + 1;
	namelen = (namelen > MAX_NAME_WIDTH) ? MAX_NAME_WIDTH : namelen;
	passlen = (passlen > MAX_PASS_WIDTH) ? MAX_PASS_WIDTH : passlen;
	memcpy(level->levelname, levelname, namelen);
	memcpy(level->pass, pass, passlen);
	level->levelname[MAX_NAME_WIDTH - 1] = '\0';
	level->pass[MAX_PASS_WIDTH - 1] = '\0';

	/* Validate user provided level; returns index of matched level in all_levels array */
	if ((idx = is_valid_level(level, all_levels)) == -1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Must beat level prior to this one to get this password so mark it as complete */
	all_levels[idx - 1]->is_level_complete = 1;

	/* Write pw to data file */
	memcpy(all_levels[idx]->pass, pass, passlen);
	all_levels[idx]->is_pass_saved = (uint8_t) 1;
	save_data(all_levels);

	printf("[+] Successfully stored password for %s.\n", level->levelname);
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
		all_levels[i] = (level_t *) malloc(sizeof(level_t));
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

	fclose(outfile);
	rename(TEMP_DATAFILE, DATAFILE);

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
