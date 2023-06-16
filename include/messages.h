#ifndef _MY_MESSAGES_H
#define _MY_MESSAGES_H

#define DEFAULT_PROGNAME 	"otw"

/* Error messages */
#define ERR_BAD_LEVEL_ARG	"[Error] invalid level argument."
#define ERR_MISSING_OPTARG 	"[Error] option -%c is missing a required argument.\n\n"
#define ERR_BAD_MALLOC		"[Error] memory allocation error.\n"
#define ERR_FOPEN_FAILED	"[Error] could not open the data file.\n"

/* Usage/help messages */
#define USAGE_MSG																\
	"usage: %s LEVEL\n"															\
	"       %s [-h] [-p] [-c LEVEL]\n"											\
	"       %s -s PASSWORD LEVEL\n\n"											\
	"Use `%s -h` for more help.\n"
#define HELP_MSG																\
	"Usage: %s LEVEL\n" 														\
	"       %s [-h|-p] [-c LEVEL]\n"											\
	"       %s -s PASSWORD LEVEL\n"												\
	"\n" 																		\
	"Connect to the OTW level LEVEL with `%s LEVEL`.\n"							\
	"\n" 																		\
	"    LEVEL\n"		 														\
	"        A single word composed of the wargame's name (e.g. \"bandit\")\n"	\
	"        and the level number (e.g. \"2\").\n"								\
	"        For example, `%s bandit2`, `%s vortex12`, etc.\n"					\
	"\n" 																		\
	"    -c, --complete LEVEL\n"												\
	"        Mark the level LEVEL as complete. Note: storing a password for a\n"\
	"        level automatically marks the level before it complete.\n"			\
	"\n" 																		\
	"    -h, --help\n" 															\
	"        Show this help information.\n"										\
	"\n" 																		\
	"    -p, --progress\n"														\
	"        Display which wargames you've completed and your overall progress.\n"	\
	"\n" 																		\
	"    -s, --store PASSWORD LEVEL\n"											\
	"        Store password PASSWORD for the level LEVEL in a local file to\n"	\
	"        make connecting to the level more convenient. Note: this will also\n"	\
	"        mark the level before it (where the password was obtained) complete.\n"

#endif /* _MY_MESSAGES_H */
