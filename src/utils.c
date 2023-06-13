#include "utils.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "typedefs.h"
#include "constants.h"
#include "messages.h"

int
show_usage(void)
{
	puts(USAGE_MSG);
	exit(EXIT_FAILURE);
}

int
show_help(void)
{
	puts(HELP_MSG);
	exit(EXIT_SUCCESS);
}

int
quit(char *msg)
{
	fprintf(stderr, "[Error] %s.\n", msg);
	exit(EXIT_FAILURE);
}

int
get_timestamp(char *buf, int bufsize)
{
	/* Return string representation of epoch time in buf */
	time_t result;
	char tstamp[32];

	if ((buf == NULL) || (bufsize < 1)) {
		fprintf(stderr, "[Error] Bad input to getTimestamp.\n");
		return 1;
	}
	if((result = time(NULL)) == (time_t)(-1)) {
		fprintf(stderr, "[Error] Could not get timestamp.\n");
		return 1;
	}
	sprintf(tstamp, "%jd", (intmax_t) result);
	strncpy(buf, tstamp, bufsize);
	return 0;
}

int
print_level(level_t *level)
{
	printf("Level info:\n");
	printf("level_name........%s\n", level->level_name);
	printf("game_name.........%s\n", level->game_name);
	printf("level_num.........%d\n", level->level_num);
	printf("ssh_addr..........%s\n", level->ssh_addr);
	printf("port..............%d\n", level->port);
	printf("pw................%s\n", level->pw);
	printf("is_completed......%d\n\n", level->is_completed);

	return 0;
}

int
print_game(game_t *game)
{
	printf("Game info:\n");
	printf("game_name..........%s\n", game->game_name);
	printf("max_level..........%d\n", game->max_level);
	printf("port...............%d\n", game->port);
	printf("is_completed.......%d\n\n", game->is_completed);

	return 0;
}
