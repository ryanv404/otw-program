#include "utils.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "constants.h"
#include "messages.h"

int
show_usage(void)
{
	puts(USAGE_MSG);
	exit(EXIT_FAILURE);
}

int
show_help(void)
{
	puts(HELP_MSG);
	exit(EXIT_SUCCESS);
}

int
quit(char *msg)
{
	fprintf(stderr, "[Error] %s.\n", msg);
	exit(EXIT_FAILURE);
}

int
get_timestamp(char *buf, int bufsize)
{
	/* Return string representation of epoch time in buf */
	time_t result;
	char tstamp[32];

	if ((buf == NULL) || (bufsize < 1)) {
		fprintf(stderr, "[Error] Bad input to getTimestamp.\n");
		return 1;
	}
	if((result = time(NULL)) == (time_t)(-1)) {
		fprintf(stderr, "[Error] Could not get timestamp.\n");
		return 1;
	}
	sprintf(tstamp, "%jd", (intmax_t) result);
	strncpy(buf, tstamp, bufsize);
	return 0;
}
