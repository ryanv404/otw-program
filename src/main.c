/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>

#include "project/typedefs.h" 		/* struct typedefs */
#include "project/datautils.h" 		/* load_data, free_levels */
#include "project/error_msgs.h" 	/* ERR_BAD_LEVEL_ARG, ERR_BAD_MALLOC */
#include "project/parse_opts.h" 	/* parse_opts */
#include "project/ssh_connect.h" 	/* connect_to_game */
#include "project/utils.h" 			/* show_usage, quit */
#include "project/validate.h" 		/* is_valid_level */

int
main(int argc, char **argv)
{
	if (argc < 2 || argc > 4) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	/* Load saved data */
	level_t **all_levels = load_data();

	level_t *level = malloc(sizeof(level_t));
	if (level == NULL) {
		free_levels(all_levels);
		quit(ERR_BAD_MALLOC);
	}

	/* Handle command line options */
	parse_opts(argc, argv, level, all_levels);

	/* Validate user provided level */
	if (is_valid_level(level, all_levels) == -1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}

	/* Connect to the level */
	connect_to_level(level, all_levels);

	exit(EXIT_SUCCESS);
}
