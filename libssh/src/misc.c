/* misc.c - OTW program */

#include "lssh/misc.h"

#include <stdio.h>
#include <stdlib.h>

#include "lssh/config.h"

void
show_usage(void)
{
	fprintf(stderr, "usage: %s LEVEL\n", DEFAULT_PROGNAME);
	fprintf(stderr, "       %s [-h|--help] [-p|--progress] [-c|--complete LEVEL]\n", DEFAULT_PROGNAME);
	fprintf(stderr, "           [-s|--store PASSWORD LEVEL]\n\n");
	fprintf(stderr, "use `%s -h` or `%s --help` for more help.\n", DEFAULT_PROGNAME, DEFAULT_PROGNAME);
	return;
}

void
show_help(void)
{
	fprintf(stderr, "Usage: %s LEVEL\n", DEFAULT_PROGNAME);
	fprintf(stderr, "       %s [-h|--help] [-p|--progress] [-c|--complete LEVEL]\n", DEFAULT_PROGNAME);
	fprintf(stderr, "           [-s|--store PASSWORD LEVEL]\n\n");
	fprintf(stderr, "Connect to the OTW level LEVEL with `%s LEVEL`.\n\n", DEFAULT_PROGNAME);
	fprintf(stderr, "    LEVEL\n");
	fprintf(stderr, "        A single word composed of the wargame's name (e.g. \"bandit\")\n");
	fprintf(stderr, "        and the level number (e.g. \"2\").\n");
	fprintf(stderr, "        For example, `%s bandit2`, `%s vortex12`, etc.\n\n", DEFAULT_PROGNAME, DEFAULT_PROGNAME);
	fprintf(stderr, "    -c, --complete LEVEL\n");
	fprintf(stderr, "        Mark the level LEVEL as complete. Note: storing a password for a\n");
	fprintf(stderr, "        level automatically marks the level before it complete.\n\n");
	fprintf(stderr, "    -h, --help\n");
	fprintf(stderr, "        Show this help information.\n\n");
	fprintf(stderr, "    -p, --progress\n");
	fprintf(stderr, "        Display which wargames you've completed and your overall progress.\n\n");
	fprintf(stderr, "    -s, --store PASSWORD LEVEL\n");
	fprintf(stderr, "        Store password PASSWORD for the level LEVEL in a local file to\n");
	fprintf(stderr, "        make connecting to the level more convenient. Note: this will also\n");
	fprintf(stderr, "        mark the level before it (where the password was obtained) complete.\n\n");
	return;
}

int
quit(char *msg)
{
	fprintf(stderr, "%s", msg);
	exit(EXIT_FAILURE);
}

void
print_level(level_t *level)
{
	char hostname[MAX_ADDR_WIDTH];
	sprintf(hostname, "%s@%s.labs.overthewire.org", level->levelname, level->gamename);
	printf("level_name.................%s\n", level->levelname);
	printf("game_name..................%s\n", level->gamename);
	printf("password...................%s\n", level->pass);
	printf("hostname...................%s\n", hostname);
	printf("port.......................%s\n", level->port);
	printf("is_password_saved..........%s\n", level->is_pass_saved ? "YES" : "NO");
	printf("is_level_complete..........%s\n", level->is_level_complete ? "YES" : "NO");
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	return;
}

void
print_all_levels(level_t **all_levels)
{
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		print_level(all_levels[i]);
	}
	return;
}

void
print_hex_fingerprint(const char *buf, int len)
{
	for (int i = 0; i < len; i++) {
		if (i == 0) {
			fprintf(stderr, "    ");
		} else if ((i != 0) && (i % 16 == 0)) {
			fprintf(stderr, "\n    ");
		}

		fprintf(stderr, "%02X", (unsigned char) buf[i]);
	}

	fprintf(stderr, "\n");
	return;
}
