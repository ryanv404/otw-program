#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

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

void
print_level(level_t *level)
{
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("Level info:\n");
	printf("levelname..................%s\n", level->levelname);
	printf("gamename...................%s\n", level->gamename);
	printf("pass.......................%s\n", level->pass);
	printf("is_pass_saved..............%d\n", level->is_pass_saved);
	printf("hostaddr...................%s\n", level->hostaddr);
	printf("port.......................%d\n", level->port);
	printf("maxlevel...................%d\n", level->maxlevel);
	printf("is_level_completed.........%d\n", level->is_level_completed);
	printf("is_game_completed..........%d\n\n", level->is_game_completed);
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

void
print_leveldata(level_t **leveldata)
{
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	for (int i = 0; i < NUM_LEVELS; i++) {
		printf("Level %d:\n", i);
		printf("levelname..................%s\n", leveldata[i]->levelname);
		printf("gamename...................%s\n", leveldata[i]->gamename);
		printf("pass.......................%s\n", leveldata[i]->pass);
		printf("is_pass_saved..............%d\n", leveldata[i]->is_pass_saved);
		printf("hostaddr...................%s\n", leveldata[i]->hostaddr);
		printf("port.......................%d\n", leveldata[i]->port);
		printf("maxlevel...................%d\n", leveldata[i]->maxlevel);
		printf("is_level_completed.........%d\n", leveldata[i]->is_level_completed);
		printf("is_game_completed..........%d\n\n", leveldata[i]->is_game_completed);
		printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	}
	return;
}
