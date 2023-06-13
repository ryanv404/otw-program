#ifndef _MY_CONSTANTS_H
#define _MY_CONSTANTS_H

/* Total number of OTW wargames */
#define NUM_GAMES				12

/* Longest level name is 'formulaone6\0' (12 characters) */
# define LVLNAME_MAX		24
/* Longest ssh address is 'formulaone6@formulaone.labs.overthewire.org\0' (44 characters) */
# define LVLADDR_MAX		44
/* Unclear what the max length should be but 44 characters seems fair */
# define LVLPW_MAX			44

/* Level numbers always start at 0 */
#define MIN_LEVEL				0
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

#endif /* _MY_CONSTANTS_H */
