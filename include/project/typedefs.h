#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include <stdint.h>

/* Longest level name is 12 characters */
#define MAX_NAME_WIDTH		16
/* Unclear what the max length should be */
#define MAX_PASS_WIDTH		52

typedef struct level_t {
    char 		levelname[MAX_NAME_WIDTH];
    char 		gamename[MAX_NAME_WIDTH];
    char 		pass[MAX_PASS_WIDTH];
    uint16_t 	port;
	uint8_t		is_pass_saved;
    uint8_t		is_level_complete;
} level_t;

typedef struct init_level_t {
    char		gamename[MAX_NAME_WIDTH];
    int			port;
    int			maxlevel;
} init_level_t;


#endif /* _MY_TYPEDEFS_H */
