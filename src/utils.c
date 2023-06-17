/* utils.c - OTW program */

#include "project/utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "project/typedefs.h"

#define DEFAULT_PROGNAME 	"otw"
#define TOTAL_LEVELS		183

void
show_usage(void)
{
	fprintf(stderr, "usage: %s LEVEL\n", DEFAULT_PROGNAME);
	fprintf(stderr, "       %s [-h|--help] [-p|--progress] [-c|--complete LEVEL] [-s|--store PASSWORD LEVEL]\n\n",
			DEFAULT_PROGNAME);
	fprintf(stderr, "use `%s -h` or `%s --help` for more help.\n", DEFAULT_PROGNAME, DEFAULT_PROGNAME);
	return;
}

void
show_help(void)
{
	fprintf(stderr, "Usage: %s LEVEL\n", DEFAULT_PROGNAME);
	fprintf(stderr, "       %s [-h|--help] [-p|--progress] [-c|--complete LEVEL] [-s|--store PASSWORD LEVEL]\n\n",
			DEFAULT_PROGNAME);
	fprintf(stderr, "Connect to the OTW level LEVEL with `%s LEVEL`.\n\n", DEFAULT_PROGNAME);
	fprintf(stderr, "    LEVEL\n");
	fprintf(stderr, "        A single word composed of the wargame's name (e.g. \"bandit\")\n");
	fprintf(stderr, "        and the level number (e.g. \"2\").\n");
	fprintf(stderr, "        For example, `%s bandit2`, `%s vortex12`, etc.\n\n", DEFAULT_PROGNAME, DEFAULT_PROGNAME);
	fprintf(stderr, "    -c, --complete LEVEL\n");
	fprintf(stderr, "        Mark the level LEVEL as complete. Note: storing a password for a\n");
	fprintf(stderr, "        level automatically marks the level before it complete.\n\n");
	fprintf(stderr, "    -h, --help\n");
	fprintf(stderr, "        Show this help information.\n\n");
	fprintf(stderr, "    -p, --progress\n");
	fprintf(stderr, "        Display which wargames you've completed and your overall progress.\n\n");
	fprintf(stderr, "    -s, --store PASSWORD LEVEL\n");
	fprintf(stderr, "        Store password PASSWORD for the level LEVEL in a local file to\n");
	fprintf(stderr, "        make connecting to the level more convenient. Note: this will also\n");
	fprintf(stderr, "        mark the level before it (where the password was obtained) complete.\n\n");
	return;
}

int
quit(char *msg)
{
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}

void
print_level(level_t *level)
{
	printf("level name.................%s\n", level->levelname);
	printf("game name..................%s\n", level->gamename);
	printf("password...................%s\n", level->pass);
	printf("port.......................%"PRIu16"\n", level->port);
	printf("is password saved..........%s\n", level->is_pass_saved ? "YES" : "NO");
	printf("is level complete..........%s\n", level->is_level_complete ? "YES" : "NO");
	return;
}

void
print_all_levels(level_t **all_levels)
{
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		printf("Level %d:\n", i);
		print_level(all_levels[i]);
		printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	}
	return;
}
