#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "typedefs.h"
#include "constants.h"
#include "validate.h"

void
setup_level(level_t *level, level_t **leveldata)
{


	return;
}

int
store_pw(level_t *level)
{
    /* Validate user provided level */
    if (is_valid_level(level) != 0) quit(ERR_BAD_LEVEL_ARG);

    /* Load level info */
    setup_level(level);

    /* Store the pw */
    puts("Successfully stored password for %s.", level->levelname);
    exit(EXIT_SUCCESS);
}

void
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
		strncpy(levels[i].levelname, tok, 32);

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].pass, tok, 32);

		levels[i].is_level_completed = atoi(strtok(NULL, ",\n"));

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].gamename, tok, 32);

		levels[i].port = atoi(strtok(NULL, ",\n"));
		levels[i].maxlevel = atoi(strtok(NULL, ",\n"));
		levels[i].is_game_completed = atoi(strtok(NULL, ",\n"));

		tok = strtok(NULL, ",\n");
		strncpy(levels[i].hostaddr, tok, 52);
	}

	for (int i = 0; i < NUM_LEVELS; i++) {
		fwrite(&levels[i], sizeof(level_t), 1, outfile);
	}

	fclose(infile);
	fclose(outfile);
	return;
}
