#ifndef _MY_CONFIG_H
#define _MY_CONFIG_H

#define DEFAULT_PROGNAME	"otw"
#define KNOWNHOSTS			"data/otw_knownhosts"
#define DATADIR				"data"
#define DATAFILE			"data/otw_data.dat"
#define TEMP_DATAFILE		DATAFILE	"~"
#define OPTSTR				":al:c:hps:"
#define SHA256_FP_WIDTH     32
#define MAX_ADDR_WIDTH		60
#define MAX_NAME_WIDTH		16  /* Longest level name is 12 characters */
#define MAX_PASS_WIDTH		48  /* Unclear what the max length should be */
#define TOTAL_GAMES			12
#define TOTAL_LEVELS		183
#define BUFSIZE				1024
#define MAX_BUFSIZE			1024

typedef struct level_t {
    char 		levelname[MAX_NAME_WIDTH];
    char 		gamename[MAX_NAME_WIDTH];
    char 		pass[MAX_PASS_WIDTH];
    char		port[6];
	int			is_pass_saved;
    int			is_level_complete;
} level_t;

typedef struct init_level_t {
    char		gamename[MAX_NAME_WIDTH];
    char		port[6];
    int			maxlevel;
} init_level_t;

#endif /* _MY_CONFIG_H */
