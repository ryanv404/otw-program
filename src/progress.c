#include "progress.h"

#include <stdio.h>
#include <stdlib.h>

#include "typedefs.h"

int
mark_level_complete(char *level)
{
    printf("%s is now marked as completed.\n", level);
    exit(EXIT_SUCCESS);
}

int
mark_game_complete(char *game)
{
    printf("%s is now marked as completed.\n", game);
    exit(EXIT_SUCCESS);
}

int
show_progress(void)
{
	puts("[Progress] You're doin great bud!");
	exit(EXIT_SUCCESS);
}
