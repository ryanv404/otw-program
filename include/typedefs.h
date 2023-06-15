#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include "constants.h"

typedef struct arg_t {
	char	level_name[LVLNAME_MAX];
	int		level_num;
} arg_t;

typedef struct level_t {
	char	name[LVLNAME_MAX];
	int		number;
	char	ssh_addr[LVLADDR_MAX];
	char	pw[LVLPW_MAX];
	int		is_completed;
} level_t;

typedef struct game_t {
	char		name[LVLNAME_MAX];
	int			max_level;
	int			port;
	int 	 	is_completed;
	level_t		**levels;
} game_t;

typedef struct GameTable_t {
	int			size;
	game_t		**games;
} GameTable_t;

#endif /* _MY_TYPEDEFS_H */
