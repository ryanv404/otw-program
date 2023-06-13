/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "typedefs.h" 		/* struct typedefs */
#include "constants.h" 		/* various global vals */
#include "parse_opts.h" 	/* parse_opts */
#include "utils.h" 			/* show_usage */
#include "data_utils.h" 	/* validate_and_setup_level */
#include "ssh_connect.h" 	/* ssh_connect */
#include "messages.h" 		/* ERR_BAD_LEVEL_ARG */

int
main(int argc, char **argv)
{
	level_t level;

	if (argc < 2) show_usage();

	/* Handle command line options */
	parse_opts(argc, argv, &level);

	/* Validate user provided level */
	if (is_valid_level(&level) != 0) quit(ERR_BAD_LEVEL_ARG);

	/* Load level info */
	setup_level(&level);

	/* Connect to level */
	//ssh_connect(&level);
	puts("ssh_connect is called here.");

	exit(EXIT_SUCCESS);
}
