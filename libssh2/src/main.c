/* main.c - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lssh2/data.h"
#include "lssh2/config.h"
#include "lssh2/error_msgs.h"
#include "lssh2/opts.h"
#include "lssh2/ssh.h"
#include "lssh2/misc.h"
#include "lssh2/validate.h"

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
	if (strncmp(level->gamename, "natas", MAX_NAME_WIDTH) != 0) {
		libssh2_connect(level, all_levels);
	} else {
		printf("[*] Connect to natas levels using your web browser.\n"
		       "    Username...%s\n"
		       "    URL........http://%s.natas.labs.overthewire.org\n\n",
			   level->levelname, level->levelname);
	}

	free(level);
	free_levels(all_levels);
	exit(EXIT_SUCCESS);
}
