/* data.c - OTW program */

#include "lssh2/data.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

#include "lssh2/config.h"
#include "lssh2/validate.h"
#include "lssh2/error_msgs.h"
#include "lssh2/misc.h"

static level_t **
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

static level_t **
create_new_datafile(void)
{
	int level_index;
	char namebuf[100];
	struct stat sb;

	/* Default info for each level */
	init_level_t init_levels[TOTAL_GAMES] = {
	    {"bandit",    "2220", 34}, {"natas",         "0", 34},
	    {"leviathan", "2223",  7}, {"krypton",    "2231",  7},
	    {"narnia",    "2226",  9}, {"behemoth",   "2221",  8},
	    {"utumno",    "2227",  8}, {"maze",       "2225",  9},
	    {"vortex",    "2228", 27}, {"manpage",    "2224",  7},
	    {"drifter",   "2230", 15}, {"formulaone", "2223",  6}
	};

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
			strncpy(all_levels[level_index]->port, init_levels[i].port, 6);
			/* Is password saved */
			all_levels[level_index]->is_pass_saved = 0;
			/* Is level complete */
			all_levels[level_index]->is_level_complete = 0;
			level_index++;
		}
	}

	save_data(all_levels);
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

void
get_level_pass(level_t *level)
{
	size_t len;
	char passbuf[MAX_PASS_WIDTH] = {0};

	if (level->is_pass_saved) {
		return;
	}

	fprintf(stderr, "[!] Enter password: ");
	fgets(passbuf, MAX_PASS_WIDTH, stdin);

	len = strlen(passbuf);
	
	if ((len > 0) && (passbuf[len - 1] == '\n')) {
		passbuf[len - 1] = '\0';
	}

	strncpy(level->pass, passbuf, MAX_PASS_WIDTH);
	return;
}

int
store_pass(char *pass, char *levelname, level_t *level, level_t **all_levels)
{
	int namelen, passlen, idx;

	if (strlen(level->levelname) == 0) {
		namelen = (int) strnlen(levelname, MAX_NAME_WIDTH);
		memcpy(level->levelname, levelname, namelen);
		level->levelname[MAX_NAME_WIDTH - 1] = '\0';
	}

	/* Validate user provided level; returns index of matched level in all_levels array */
	if ((idx = is_valid_level(level, all_levels)) == -1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}

	if (pass == NULL) {
		/* Clear bad stored password */
		memcpy(all_levels[idx]->pass, "?", 2);
		all_levels[idx]->is_pass_saved = 0;
	} else {
		passlen = (int) strnlen(pass, MAX_PASS_WIDTH);
		memcpy(all_levels[idx]->pass, pass, passlen);
		all_levels[idx]->pass[MAX_PASS_WIDTH - 1] = '\0';
		all_levels[idx]->is_pass_saved = 1;
	}

	/* Must beat level prior to this one to get this password so mark it as complete */
	// all_levels[idx - 1]->is_level_complete = 1; /* TODO but not on the 0th level! */ 

	/* Write pw to data file */
	save_data(all_levels);

	fprintf(stderr, "[*] Password saved.\n");
	return 0;
}
