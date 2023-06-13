#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include <stdint.h>

#ifndef _MY_CONSTANTS_H
/* Longest level name is 'formulaone6\0' (12 characters) */
# define LVLNAME_MAX		24
/* Longest ssh address is 'formulaone6@formulaone.labs.overthewire.org\0' (44 characters) */
# define LVLADDR_MAX		44
# define LVLPW_MAX			44
#endif /* _MY_CONSTANTS_H */

typedef struct game_t {
	char	name[LVLNAME_MAX];
	int		min_level;
	int		max_level;
} game_t;

typedef struct arg_t {
	char	level_name[LVLNAME_MAX];
	int		level_num;
} arg_t;

typedef struct level_t {
	char	 level_name[LVLNAME_MAX];
	char	 game_name[LVLNAME_MAX];
	int		 level_num;
	char	 ssh_addr[LVLADDR_MAX];
	uint16_t port;
	char	 pw[LVLPW_MAX];
	int 	 is_completed;
} level_t;

#endif /* _MY_TYPEDEFS_H */
