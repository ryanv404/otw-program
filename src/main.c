/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>

#include "typedefs.h" 		/* struct typedefs */
#include "datautils.h" 		/* load_data, free_levels */
#include "messages.h" 		/* ERR_BAD_LEVEL_ARG, ERR_BAD_MALLOC */
#include "parse_opts.h" 	/* parse_opts */
#include "ssh_connect.h" 	/* connect_to_game */
#include "utils.h" 			/* show_usage, quit */
#include "validate.h" 		/* is_valid_level */

int
main(int argc, char **argv)
{
	if (argc < 2 || argc > 4) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	/* Load saved data */
	level_t **all_leveldata = load_data();

	level_t *level = malloc(sizeof(level_t));
	if (level == NULL) {
		free_levels(all_leveldata);
		quit(ERR_BAD_MALLOC);
	}

	/* Handle command line options */
	parse_opts(argc, argv, level, all_leveldata);

	/* Validate user provided level */
	if (is_valid_level(level, all_leveldata) != 0) {
		free(level);
		free_levels(all_leveldata);
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Release unneeded memory */
	free_levels(all_leveldata);

	/* Connect to the level */
	connect_to_game(level);

	free(level);
	exit(EXIT_SUCCESS);
}
