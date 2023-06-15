/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "constants.h" 		/* various global vals */
#include "datautils.h" 		/* load_saved_data, free_levels */
#include "messages.h" 		/* ERR_BAD_LEVEL_ARG */
#include "parse_opts.h" 	/* parse_opts */
#include "ssh_connect.h" 	/* connect_to_game */
#include "typedefs.h" 		/* struct typedefs */
#include "utils.h" 			/* show_usage */
#include "validate.h" 		/* is_valid_level */

int
main(int argc, char **argv)
{
	if (argc < 2 || argc > 4) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	/* Load saved data; leveldata free'd in is_valid_level */
	level_t **leveldata = load_saved_data();
	level_t *level = malloc(sizeof(level_t));

	if (level == NULL) {
		quit(ERR_BAD_MALLOC);
	}

	/* Handle command line options */
	parse_opts(argc, argv, level, leveldata);

	/* Validate user provided level */
	if (is_valid_level(level, leveldata) != 0) {
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Connect to the level */
	connect_to_game(level);

	exit(EXIT_SUCCESS);
}
