#ifndef _MY_MESSAGES_H
#define _MY_MESSAGES_H

#define HELP_MSG																\
	"Usage: otw LEVEL\n" 														\
	"       otw [-h|-d]\n" 														\
	"       otw -s PASSWORD LEVEL\n"											\
	"       otw -c LEVEL\n"														\
	"\n" 																		\
	"Connect to the OTW wargame LEVEL.\n"										\
	"\n" 																		\
	"    LEVEL\n"		 														\
	"        A single word composed of the wargame's name (e.g. \"bandit\")\n"	\
	"        and the level number (e.g. \"2\").\n"								\
	"        For example, `otw bandit2`, `otw vortex12`, etc.\n"				\
	"\n" 																		\
	"    -c, --complete LEVEL\n"												\
	"        Mark level LEVEL as complete.\n"									\
	"\n" 																		\
	"    -d, --display\n"														\
	"        Display user's progress in completing all of the OTW wargames.\n"	\
	"\n" 																		\
	"    -h, --help\n" 															\
	"        Show this help information.\n"										\
	"\n" 																		\
	"    -s, --store PASSWORD\n"												\
	"        Store password PASSWORD for level LEVEL in the local database\n"	\
	"        to make connecting to the level quicker.\n"

#define DEFAULT_PROGNAME 	"otw"
#define USAGE_MSG  			"Usage: otw [-h] [-d] [-c LEVEL] [-s PASSWORD LEVEL]\n"	\
							"       otw LEVEL\n\nUse `otw -h` for more help.\n"
#define ERR_FOPEN_INPUT  	"fopen(input, r)"
#define ERR_FOPEN_OUTPUT 	"fopen(output, w)"
#define ERR_DO_THE_NEEDFUL 	"do_the_needful blew up"

#endif /* _MY_MESSAGES_H */
