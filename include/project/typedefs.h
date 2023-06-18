#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include <stdint.h>

#define MAX_NAME_WIDTH			16 	/* Longest level name is 12 characters */
#define MAX_PASS_WIDTH			52 	/* Unclear what the max length should be */
#define MAX_PORT_WIDTH			8
#define MD5_FINGERPRINT_WIDTH	16

typedef struct level_t {
    char 		levelname[MAX_NAME_WIDTH];
    char 		gamename[MAX_NAME_WIDTH];
    char 		pass[MAX_PASS_WIDTH];
    char	 	port[MAX_PORT_WIDTH];
	uint8_t		is_pass_saved;
    uint8_t		is_level_complete;
} level_t;

typedef struct init_level_t {
    char		gamename[MAX_NAME_WIDTH];
    char		port[MAX_PORT_WIDTH];
    int			maxlevel;
} init_level_t;

typedef struct fingerprint_t {
    char		gamename[MAX_NAME_WIDTH];
    char		fingerprint[MD5_FINGERPRINT_WIDTH];
} fingerprint_t;

#endif /* _MY_TYPEDEFS_H */
