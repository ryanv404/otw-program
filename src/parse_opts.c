/* parse_opts.c - OTW program */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#include "parse_opts.h"
#include "constants.h"
#include "typedefs.h"
#include "messages.h"
#include "utils.h"
#include "datautils.h"
#include "progress.h"

extern char *optarg;
extern int optind, optopt;

int
parse_opts(int argcount, char **args, level_t *level, level_t **all_levelinfo)
{
	int c, nargs, namelen;

	while (1) {
		static struct option longopts[] = {
			{"help",	 no_argument, 		0, 	'h'},
			{"progress", no_argument, 		0, 	'p'},
			{"complete", required_argument, 0, 	'c'},
			{"store", 	 required_argument, 0, 	's'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argcount, args, OPTSTR, longopts, NULL);
		
		/* Detect the end of the provided options */
		if (c == -1) break;

		switch (c) {
		case 'c':
			/* Mark a level as completed */
			mark_level_complete(optarg);
			free(level);
			free_levels(all_levelinfo);
			exit(EXIT_SUCCESS);
			break;

		case 'h':
			/* Display the help message */
			show_help();
			free(level);
			free_levels(all_levelinfo);
			exit(EXIT_SUCCESS);
			break;

		case 'p':
			/* Display the user's progress */
			show_progress();
			free(level);
			free_levels(all_levelinfo);
			exit(EXIT_SUCCESS);
			break;

		case 's':
			/* Store the level's password */
			printf("optind: %d, argc: %d\n", optind, argcount);
			printf("argv[optind]: %s, argv[argc-1]: %s, optarg: %s\n", args[optind], args[argcount-1], optarg);

			store_pass(optarg, args[argcount - 1], level, all_levelinfo);
			free(level);
			free_levels(all_levelinfo);
			exit(EXIT_SUCCESS);
			break;

		case ':':
			fprintf(stderr, ERR_MISSING_OPTARG, (char) optopt);
			show_usage();
			free(level);
			free_levels(all_levelinfo);
			exit(EXIT_FAILURE);
			break;

		case '?':
		default:
			free(level);
			free_levels(all_levelinfo);
			quit("[Error] unknown option.\n");
		}
	}

	/* Only 1 argument should remain: the level argument */
	nargs = argcount - optind;
	if (nargs != 1) {
		free(level);
		free_levels(all_levelinfo);
		quit(ERR_BAD_LEVEL_ARG);
	}

	namelen = (int) strlen(args[optind]) + 1;
	namelen = (namelen > LVLNAME_MAX) ? LVLNAME_MAX : namelen;
	memcpy(level->levelname, args[optind], namelen);

	/* Ensure final byte of buffer is a null byte */
	level->levelname[LVLNAME_MAX - 1] = '\0';

	return 0;
}
