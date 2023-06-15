/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "typedefs.h" 		/* struct typedefs */
#include "constants.h" 		/* various global vals */
#include "parse_opts.h" 	/* parse_opts */
#include "utils.h" 			/* show_usage */
#include "datautils.h" 		/* load_saved_data */
#include "ssh_connect.h" 	/* connect_to_game */
#include "messages.h" 		/* ERR_BAD_LEVEL_ARG */

int
main(int argc, char **argv)
{
	if (argc < 2) show_usage();

	level_t level;
	level_t **leveldata = load_saved_data();

	/* Handle command line options */
	parse_opts(argc, argv, levels);

	/* Validate user provided level */
	if (is_valid_level(&level, leveldata) != 0) quit(ERR_BAD_LEVEL_ARG);

	/* Load level info */
	setup_level(&level);

	/* Connect to level */
	//connect_to_game(&level);
	puts("ssh_connect is called here.");

	free_levels(levels);
	exit(EXIT_SUCCESS);
}
