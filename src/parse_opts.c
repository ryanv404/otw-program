#include "parse_opts.h"

#define _GNU_SOURCE /* getopt_long */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "constants.h"
#include "typedefs.h"
#include "utils.h"
#include "data_utils.h"
#include "progress.h"

extern char *optarg;
extern int optind, optopt;

int
parse_opts(int argcount, char **args, level_t *level)
{
	int c, nargs;
	const char *optstr = OPTSTR;

	while (1) {
		static struct option longopts[] = {
			{"help",	 no_argument, 		0, 	'h'},
			{"display",  no_argument, 		0, 	'd'},
			{"complete", required_argument, 0, 	'c'},
			{"store", 	 required_argument, 0, 	's'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argcount, args, optstr, longopts, NULL);
		
		/* Detect the end of the provided options */
		if (c == -1) break;

		switch (c) {
		case 'c':
			/* Mark a level as completed */
			mark_level_complete(optarg);
			break;
		
		case 'd':
			/* Display the user's progress */
			show_progress();
			break;

		case 'h':
			/* Display the help message */
			show_help();
			break;

		case 's':
			/* Store the level's password */
			memcpy(level->level_name, args[argcount - 1], LVLNAME_MAX);
			memcpy(level->pw, optarg, LVLPW_MAX);
			level->level_name[LVLNAME_MAX - 1] = '\0';
			level->pw[LVLPW_MAX - 1] = '\0';
			store_pw(level);
			break;

		case ':':
			fprintf(stderr, "Option -%c is missing a required argument.\n\n", (char) optopt);
			show_usage();
			break;

		case '?':
		default:
			quit("Unknown option");
		}
	}

	/* Only 1 argument should remain: the level argument */
	nargs = argcount - optind;
	if (nargs != 1) {
		quit("Did not receive valid input");
	}

	/* Ensure that the level_name field contains a null terminated string */
	memcpy(level->level_name, args[optind], LVLNAME_MAX);
	level->level_name[LVLNAME_MAX - 1] = '\0';

	return 0;
}
