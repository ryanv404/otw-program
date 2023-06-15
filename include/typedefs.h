#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include "constants.h"

typedef struct level_t {
    char levelname[LVLNAME_MAX];
    char gamename[LVLNAME_MAX];
    char pass[LVLPASS_MAX];
	int is_pass_saved;
    char hostaddr[LVLADDR_MAX];
    int port;
    int maxlevel;
    int is_level_completed;
    int is_game_completed;
} level_t;

#endif /* _MY_TYPEDEFS_H */
