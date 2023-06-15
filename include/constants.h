#ifndef _MY_CONSTANTS_H
#define _MY_CONSTANTS_H

#define NUM_LEVELS      183
#define CSV_DATAFILE    "data/leveldata.csv"
#define DATAFILE        "data/leveldata.dat"
#define TEMP_DATAFILE   DATAFILE    "~"
#define NUM_GAMES		12
#define NUM_LEVELS		183

/* Longest level name is 'formulaone6\0' (12 characters) */
#define LVLNAME_MAX		24
/* Longest ssh address is 'formulaone6@formulaone.labs.overthewire.org\0' (44 characters) */
#define LVLADDR_MAX		52
/* Unclear what the max length should be */
#define LVLPASS_MAX		100

/* Highest level available for each game */
#define BANDIT_MAX			34
#define NATAS_MAX			34
#define LEVIATHAN_MAX		7
#define KRYPTON_MAX			7
#define NARNIA_MAX			9
#define BEHEMOTH_MAX		8
#define UTUMNO_MAX			8
#define MAZE_MAX			9
#define VORTEX_MAX			27
#define MANPAGE_MAX			7
#define DRIFTER_MAX			15
#define FORMULAONE_MAX		6

/* Ports for each game*/
#define BANDIT_PORT			2220
#define LEVIATHAN_PORT		2223
#define KRYPTON_PORT		2231
#define NARNIA_PORT			2226
#define BEHEMOTH_PORT		2221
#define UTUMNO_PORT			2227
#define MAZE_PORT			2225
#define VORTEX_PORT			2228
#define MANPAGE_PORT		2224
#define DRIFTER_PORT		2230
#define FORMULAONE_PORT		2223

/* Reads 1 less than LVLNAME_MAX non-digit chars and an int */
#define SCAN_FMTSTR		"%23[^0-9]%d"
#define OPTSTR		":c:dhs:"

#endif /* _MY_CONSTANTS_H */




