/* datautils.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "datautils.h"
#include "typedefs.h"
#include "constants.h"
#include "validate.h"
#include "messages.h"
#include "utils.h"
#include "progress.h"

level_t **create_new_datafile(void);
level_t **init_data_structs(void);

int
store_pass(char *pass, char *levelname, level_t *level, level_t **leveldata)
{
	int namelen, passlen;

	namelen = (int) strlen(levelname) + 1;
	passlen = (int) strlen(pass) + 1;
	namelen = (namelen > LVLNAME_MAX) ? LVLNAME_MAX : namelen;
	passlen = (passlen > LVLPASS_MAX) ? LVLPASS_MAX : passlen;

	printf("namelen: %d, passlen: %d\n", namelen, passlen);
	memcpy(level->levelname, levelname, namelen);
	memcpy(level->pass, pass, passlen);
	
	/* Ensure final byte of buffer is a null byte */
	level->levelname[LVLNAME_MAX - 1] = '\0';
	level->pass[LVLPASS_MAX - 1] = '\0';

	/* Validate user provided level */
	if (is_valid_level(level, leveldata) != 0) {
		free(level);
		free_levels(leveldata);
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Write pw to data file and mark level as complete */
	mark_level_complete(level->levelname);

	printf("[+] Successfully stored password for %s.", level->levelname);
	print_level(level);

	return 0;
}

level_t **
init_data_structs(void)
{
	level_t **levels = (level_t **) calloc(NUM_LEVELS, sizeof(level_t *));

	if (levels == NULL) {
		fprintf(stderr, ERR_BAD_MALLOC);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		levels[i] = (level_t *) malloc(sizeof(level_t));
		if (levels[i] == NULL) {
			fprintf(stderr, ERR_BAD_MALLOC);
			free_levels(levels);
			exit(EXIT_FAILURE);
		}
	}

	return levels;
}

level_t **
load_data(void)
{
	level_t **levels = NULL;

	if (access(DATAFILE, F_OK) != 0) {
		levels = create_new_datafile();
	} else {
		levels = init_data_structs();
	}

	FILE *infile = fopen(DATAFILE, "rb");
	if (infile == NULL) {
		fprintf(stderr, ERR_FOPEN_FAILED);
		free_levels(levels);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fread(levels[i], sizeof(level_t), 1, infile);
	}

	fclose(infile);
	return levels;
}

void
save_data(level_t **levels)
{
	FILE *outfile = fopen(TEMP_DATAFILE, "wb");

	if (outfile == NULL) {
		fprintf(stderr, ERR_FOPEN_FAILED);
		free_levels(levels);
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fwrite(levels[i], sizeof(level_t), 1, outfile);
	}

	printf("[+] Data saved.\n");
	fclose(outfile);

	rename(TEMP_DATAFILE, DATAFILE);
	return;
}

level_t **
create_new_datafile(void)
{
	char *tok;
	char inbuf[256];

	if (access(CSV_DATAFILE, F_OK) != 0) {
		fprintf(stderr, "[Error] unable to located local data files.\n");
		exit(EXIT_FAILURE);
	}

	FILE *infile = fopen(CSV_DATAFILE, "r");
	FILE *outfile = fopen(DATAFILE, "wb");

	if ((infile == NULL) || (outfile == NULL)) {
		fprintf(stderr, ERR_FOPEN_FAILED);
		exit(EXIT_FAILURE);
	}

	level_t **levels = init_data_structs();

	for (int i = 0; i < NUM_LEVELS; i++) {
		fgets(inbuf, 256, infile);

		tok = strtok(inbuf, ",");
		strncpy(levels[i]->levelname, tok, LVLNAME_MAX);

		tok = strtok(NULL, ",\n");
		strncpy(levels[i]->gamename, tok, LVLNAME_MAX);

		tok = strtok(NULL, ",\n");
		strncpy(levels[i]->pass, tok, LVLPASS_MAX);

		levels[i]->is_pass_saved = atoi(strtok(NULL, ",\n"));

		tok = strtok(NULL, ",\n");
		strncpy(levels[i]->hostaddr, tok, LVLADDR_MAX);

		levels[i]->port = atoi(strtok(NULL, ",\n"));
		levels[i]->maxlevel = atoi(strtok(NULL, ",\n"));
		levels[i]->is_level_completed = atoi(strtok(NULL, ",\n"));
		levels[i]->is_game_completed = atoi(strtok(NULL, ",\n"));
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fwrite(&levels[i], sizeof(level_t), 1, outfile);
	}

	printf("[+] New data file created.\n");

	fclose(infile);
	fclose(outfile);

	return levels;
}

void
free_levels(level_t **levels)
{
	for (int i = 0; i < NUM_LEVELS; i++) {
		free(levels[i]);
	}
	free(levels);
	return;
}
