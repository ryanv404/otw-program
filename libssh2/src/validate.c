/* validate.c - OTW program */

#include "lssh2/validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include "lssh2/config.h"
#include "lssh2/misc.h"

int
is_valid_level(level_t *level, level_t **all_levels)
{
	/* Validate user provided level argument */
	for (int i = 0; i < TOTAL_LEVELS; i++) {
		if (strncmp(all_levels[i]->levelname, level->levelname, MAX_NAME_WIDTH) == 0) {
			/* Level arg is now validated */
			memcpy(level, all_levels[i], sizeof(level_t));
			return i;
		}
	}
	return -1;
}

int
check_for_known_host(const char *fingerprint, level_t *level)
{
	int retries, i, j, fp_txtbuf_len;

	fp_txtbuf_len = 2 * SHA256_FP_WIDTH + 8;

	char fp_txt[fp_txtbuf_len];
	char kh_fprint[fp_txtbuf_len];
	char kh_name[MAX_NAME_WIDTH];

	char resp[100] = {0};
	char kh_entry[100] = {0};

	memset(fp_txt, 0, fp_txtbuf_len);
	memset(kh_fprint, 0, fp_txtbuf_len);
	memset(kh_name, 0, MAX_NAME_WIDTH);

	/* Create a known hosts file if one doesn't exist */
	FILE *fp = fopen(KNOWNHOSTS, "r+");
	if (fp == NULL) {
		fp = fopen(KNOWNHOSTS, "w");
		if (fp == NULL) {
			fprintf(stderr, "[-] Could not create a known hosts file.\n");
			return 1;
		}
	}	

	/* Convert binary fingerprint to hex string */
	for (i = 0, j = 0; i < SHA256_FP_WIDTH; i++, j += 2) {
		sprintf(fp_txt + j, "%02X", fingerprint[i]);
	}

	fp_txt[j] = '\0';

	/* Scan known hosts file for a match */
	if (fp != NULL) {
		while (fgets(kh_entry, sizeof(kh_entry), fp) != NULL) {
			sscanf(kh_entry, "%[^:]:%[^\n]", kh_name, kh_fprint);

			/* Match host name first */
			if (strncmp(level->gamename, kh_name, MAX_NAME_WIDTH) == 0) {
				/* Match host key */
				if (strncmp(fp_txt, kh_fprint, SHA256_FP_WIDTH * 2) == 0) {
					fclose(fp);
					return 0;
				} else {
					/* Hostkey has changed for this server */
					fprintf(stderr,
							"[!] This server is in your known hosts file but it's hostkey\n"
							"    has changed. Its current key is:\n");
					print_hex_fingerprint(fingerprint, SHA256_FP_WIDTH);
					fprintf(stderr, "[!] For security reasons, this connection will be closed.\n");
					fclose(fp);
					return 1;
				}
			}
		}

		/* Ensure that we're appending and clear any EOF */
		fseek(fp, 0, SEEK_END);
	}
	
	/* No match; writes will append if file is open */
	fprintf(stderr, "[*] SHA-256 digest of remote server's hostkey:\n");
	print_hex_fingerprint(fingerprint, SHA256_FP_WIDTH);

	retries = 0;
	do {
		fprintf(stderr, "\n[!] This server is currently unknown to you. Would you like to\n"
						"    trust it? (yes/no): ");

		fgets(resp, 100, stdin);

		if (strncasecmp(resp, "y", 1) == 0) {
			/* Write host key to known hosts file */
			if (fp != NULL) {
				fprintf(fp, "%s:%s\n", level->gamename, fp_txt);
				fclose(fp);
			}
			return 0;
		} else if (strncasecmp(resp, "n", 1) == 0) {
			break;
		}

		retries++;
		memset(resp, 0, 100);
	} while (retries < 3);

	if (fp != NULL) {
		fclose(fp);
	}
	
	return 1;
}
