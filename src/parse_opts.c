/* parse_opts.c - OTW program */

#include "project/parse_opts.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "project/typedefs.h"
#include "project/error_msgs.h"
#include "project/utils.h"
#include "project/datautils.h"
#include "project/validate.h"

#define OPTSTR		":al:c:hps:"

extern char *optarg;
extern int optind, optopt;

int
parse_opts(int argcount, char **args, level_t *level, level_t **all_levels)
{
	int c, nargs, namelen;

	struct option longopts[] = {
		{"help",	 no_argument, 		0, 	'h'},
		{"progress", no_argument, 		0, 	'p'},
		{"complete", required_argument, 0, 	'c'},
		{"store", 	 required_argument, 0, 	's'},
		/* DEBUG OPTIONS */
		{"all", 	 no_argument, 		0, 	'a'},
		{"level", 	 required_argument, 0, 	'l'},
		{0, 0, 0, 0}
	};

	while (1) {

		c = getopt_long(argcount, args, OPTSTR, longopts, NULL);
		
		/* Detect the end of the provided options */
		if (c == -1) break;

		switch (c) {
		/* START OF DEBUG OPTIONS */
		case 'a':
			/* DEBUG: Print stored data for all levels */
			print_all_levels(all_levels);
			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
		case 'l':
			/* DEBUG: Print stored data for the specified level */
			strncpy(level->levelname, optarg, MAX_NAME_WIDTH);
			if (is_valid_level(level, all_levels) == -1) {
				fprintf(stderr, "[-] Invalid level argument.\n");
				free(level);
				free_levels(all_levels);
				exit(EXIT_FAILURE);
			}
			print_level(level);
			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
		/* END OF DEBUG OPTIONS */
		
		case 'c':
			/* Mark a level as completed */
			namelen = (int) strlen(optarg) + 1;
			namelen = (namelen > MAX_NAME_WIDTH) ? MAX_NAME_WIDTH : namelen;
			memcpy(level->levelname, optarg, namelen);
			level->levelname[MAX_NAME_WIDTH - 1] = '\0';

    		mark_level_complete(level, all_levels);
			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
			break;

		case 'h':
			/* Display the help message */
			show_help();
			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
			break;

		case 'p':
			/* Display the user's progress */
			show_progress(all_levels);
			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
			break;

		case 's':
			/* Store the level's password */
			if (optind != argcount) {
				store_pass(optarg, args[argcount - 1], level, all_levels);
			} else {
				free(level);
				free_levels(all_levels);
				quit("[-] Level argument is missing.\n");
			}

			free(level);
			free_levels(all_levels);
			exit(EXIT_SUCCESS);
			break;

		case ':':
			fprintf(stderr, ERR_MISSING_OPTARG, (char) optopt);
			show_usage();
			free(level);
			free_levels(all_levels);
			exit(EXIT_FAILURE);
			break;

		case '?':
		default:
			free(level);
			free_levels(all_levels);
			quit("[-] Unknown option.\n");
		}
	}

	/* Only 1 argument should remain: the level argument */
	nargs = argcount - optind;
	if (nargs != 1) {
		free(level);
		free_levels(all_levels);
		quit(ERR_BAD_LEVEL_ARG);
	}

	namelen = (int) strlen(args[optind]) + 1;
	namelen = (namelen > MAX_NAME_WIDTH) ? MAX_NAME_WIDTH : namelen;
	memcpy(level->levelname, args[optind], namelen);

	/* Ensure final byte of buffer is a null byte */
	level->levelname[MAX_NAME_WIDTH - 1] = '\0';

	return 0;
}
