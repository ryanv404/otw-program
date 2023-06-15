#ifndef _MY_TYPEDEFS_H
#define _MY_TYPEDEFS_H

#include "constants.h"

#define NUM_LEVELS      183
#define CSV_DATAFILE    "data/leveldata.csv"
#define DATAFILE        "data/leveldata.dat"
#define TEMP_DATAFILE   DATAFILE    "~"



void read_datafile(level_t **levels);
void write_to_datafile(level_t **levels);
void create_new_datafile(void);
void free_levels(level_t **levels);
level_t **load_saved_data(void);


typedef struct level_t {
    char levelname[LVLNAME_MAX];
    char pass[LVLPASS_MAX];
    int is_level_completed;
    char gamename[LVLNAME_MAX];
    int port;
    int maxlevel;
    int is_game_completed;
    char hostaddr[LVLADDR_MAX];
} level_t;

#endif /* _MY_TYPEDEFS_H */
