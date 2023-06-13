#include "parse_opts.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

#include "constants.h"
#include "typedefs.h"
#include "utils.h"
#include "data_utils.h"
#include "progress.h"

#define OPTSTR		":c:dhs:"

int
parse_opts(int argcount, char **args, arg_t *split_arg)
{
	int opt, nargs;
	const char *short_opts = OPTSTR;
	struct option long_opts[] = {
		{"help",	 no_argument, 			NULL, 	'h'},
		{"display",  no_argument, 			NULL, 	'd'},
		{"complete", required_argument, 	NULL, 	'c'},
		{"store", 	 required_argument, 	NULL, 	's'},
		{NULL, 		 0, 					NULL, 	0}
	};

	while (1) {
		opt = getopt_long(argcount, args, short_opts, long_opts, NULL);
		/* Process all command line options */
		if (opt == -1) break;

		switch (opt) {
		case 'h':
			/* Display the help message */
			show_help();
			break;
		case 'd':
			/* Display the user's progress */
			show_progress();
			break;
		case 'c':
			/* Mark a level as completed */
			mark_level_complete(optarg);
			break;
		case 's':
			/* Store the level's password */
			store_pw(optarg, args[argcount - 1]);
			break;
		case ':':
			fprintf(stderr, "Option -%c is missing a required argument.\n\n", (char) optopt);
			show_usage();
			break;
		default:
			quit("Unknown option");
		}
	}

	/* Only the level argument should follow the options */
	nargs = argcount - optind;
	if (nargs != 1) {
		quit("Did not receive the correct number of arguments");
	}

	/* Split the remaining argument into name and number parts */
	sscanf(args[optind], "%[a-zA-Z]%d", split_arg->level_name, &split_arg->level_num);

	return 0;
}
