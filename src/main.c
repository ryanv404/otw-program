/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "typedefs.h" 		/* struct typedefs */
#include "constants.h" 		/* various global vals */
#include "parse_opts.h" 	/* parse_opts */
#include "utils.h" 			/* show_usage */
#include "datautils.h" 		/* load_saved_data, free_levels */
#include "ssh_connect.h" 	/* connect_to_game */
#include "messages.h" 		/* ERR_BAD_LEVEL_ARG */
#include "validate.h" 		/* is_valid_level */

int
main(int argc, char **argv)
{
	if (argc < 2) show_usage();

	/* Load saved data; leveldata free'd in is_valid_level */
	level_t **leveldata = load_saved_data();
	level_t *level = malloc(sizeof(level_t));

	if (level == NULL) quit("Memory allocation error");

	/* Handle command line options */
	parse_opts(argc, argv, level, leveldata);

	/* Validate user provided level */
	if (is_valid_level(level, leveldata) != 0) quit(ERR_BAD_LEVEL_ARG);

	/* Connect to the level */
	connect_to_game(level);

	exit(EXIT_SUCCESS);
}
