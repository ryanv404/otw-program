/* main.c */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "typedefs.h" /* struct typedefs */
#include "constants.h" /* various global vals */
#include "parse_opts.h" /* parse_opts */
#include "validate.h" /* is_valid_level, get_level_info */
#include "utils.h" /* show_usage */

int
main(int argc, char **argv)
{
	arg_t split_arg;
	level_t level;

	if (argc < 2) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	/* Handle CLI options and parse level into name and number */
	parse_opts(argc, argv, &split_arg);

	/* Ensure the level argument is valid */
	if (!is_valid_level(&split_arg)) {
		show_usage();
		exit(EXIT_FAILURE);
	}

	/* Load level-specific connection info into level */
	get_level_info(&split_arg, &level);

	//ssh_connect(&level);
	puts("ssh_connect is called here.");

	exit(EXIT_SUCCESS);
}
