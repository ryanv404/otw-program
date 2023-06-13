#ifndef _MY_CONSTANTS_H
#define _MY_CONSTANTS_H

/* Total number of OTW wargames */
#define NUM_GAMES				12

/* Longest level name is 'formulaone6\0' (12 characters) */
# define LVLNAME_MAX		24
/* Longest ssh address is 'formulaone6@formulaone.labs.overthewire.org\0' (44 characters) */
# define LVLADDR_MAX		44
/* Unclear what the max length should be */
# define LVLPW_MAX			24

/* Highest level available for each game */
#define BANDIT_MAX_LEVEL		34
#define NATAS_MAX_LEVEL			34
#define LEVIATHAN_MAX_LEVEL		7
#define KRYPTON_MAX_LEVEL		7
#define NARNIA_MAX_LEVEL		9
#define BEHEMOTH_MAX_LEVEL		8
#define UTUMNO_MAX_LEVEL		8
#define MAZE_MAX_LEVEL			9
#define VORTEX_MAX_LEVEL		27
#define MANPAGE_MAX_LEVEL		7
#define DRIFTER_MAX_LEVEL		15
#define FORMULAONE_MAX_LEVEL	6

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

#define OPTSTR		":c:dhs:"

/* Read 1 less than LVLNAME_MAX non-digit chars and an int */
#define SCAN_FMTSTR	"%23[^0-9]%d"

#endif /* _MY_CONSTANTS_H */
