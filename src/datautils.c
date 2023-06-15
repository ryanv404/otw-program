#include "datautils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedefs.h"
#include "constants.h"
#include "validate.h"
#include "messages.h"
#include "utils.h"

int
store_pass(level_t *level, level_t **leveldata)
{
    /* Validate user provided level */
    if (is_valid_level(level, leveldata) != 0) quit(ERR_BAD_LEVEL_ARG);

    /* Store the pw */
    printf("Successfully stored password for %s.", level->levelname);
    exit(EXIT_SUCCESS);
}

level_t **
load_saved_data(void)
{
	level_t **levels = (level_t **) calloc(NUM_LEVELS, sizeof(level_t *));

	if (levels == NULL) {
		fprintf(stderr, "Memory allocation error.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		levels[i] = (level_t *) malloc(sizeof(level_t));
		if (levels[i] == NULL) {
			fprintf(stderr, "Memory allocation error.\n");
			exit(EXIT_FAILURE);
		}
	}

	read_datafile(levels);
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


void
read_datafile(level_t **levels)
{
	FILE *infile = fopen(TEMP_DATAFILE, "rb");

	if (infile == NULL) {
		fprintf(stderr, "Could not open the data file.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fread(levels[i], sizeof(level_t), 1, infile);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		printf("%s - %s - %d\n", levels[i]->levelname, levels[i]->pass, levels[i]->is_game_completed);
	}

	fclose(infile);
	return;
}

void
write_to_datafile(level_t **levels)
{
	FILE *outfile = fopen(TEMP_DATAFILE, "wb");

	if (outfile == NULL) {
		fprintf(stderr, "Could not open the data file.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fwrite(levels[i], sizeof(level_t), 1, outfile);
	}

	printf("Data saved.\n");
	fclose(outfile);
	rename(TEMP_DATAFILE, DATAFILE);
	return;
}

void
create_new_datafile(void)
{
	char *tok;
	char inbuf[256];
	level_t levels[183];

	FILE *infile = fopen(CSV_DATAFILE, "r");
	FILE *outfile = fopen(DATAFILE, "wb");

	if ((infile == NULL) || (outfile == NULL)) {
		fprintf(stderr, "Could not open one of the data files.\n");
		exit(EXIT_FAILURE);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fgets(inbuf, 256, infile);

		tok = strtok(inbuf, ",");
		strncpy(levels[i].levelname, tok, LVLNAME_MAX);

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].gamename, tok, LVLNAME_MAX);

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].pass, tok, LVLPASS_MAX);

		levels[i].is_pass_saved = atoi(strtok(NULL, ",\n"));

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].hostaddr, tok, LVLADDR_MAX);

		levels[i].port = atoi(strtok(NULL, ",\n"));
		levels[i].maxlevel = atoi(strtok(NULL, ",\n"));
		levels[i].is_level_completed = atoi(strtok(NULL, ",\n"));
		levels[i].is_game_completed = atoi(strtok(NULL, ",\n"));
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fwrite(&levels[i], sizeof(level_t), 1, outfile);
	}

	fclose(infile);
	fclose(outfile);
	return;
}
