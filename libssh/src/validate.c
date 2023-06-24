/* validate.c - OTW program */

#include "lssh/validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>

#include <libssh/libssh.h>

#include "lssh/config.h"
#include "lssh/misc.h"

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
verify_knownhost(ssh_session session)
{
	int rc, n;
	size_t hlen;
	char *hexa, *hptr;
	enum ssh_known_hosts_e state;

	char buf[6] = {0};
	ssh_key srv_pubkey  = NULL;
	unsigned char *hash = NULL;

	rc = ssh_get_server_publickey(session, &srv_pubkey);
	if (rc < 0) {
		fprintf(stderr, "[-] Error while obtaining server's public key hash.\n");
		return -1;
	}

	rc = ssh_get_publickey_hash(srv_pubkey, SSH_PUBLICKEY_HASH_SHA256, &hash, &hlen);
	ssh_key_free(srv_pubkey);
	if (rc < 0) {
		fprintf(stderr, "[-] Error while obtaining server's public key hash.\n");
		return -1;
	}

	state = ssh_session_is_known_server(session);

	switch (state) {
	case SSH_KNOWN_HOSTS_OK:
		/* This is a known host. */
		ssh_clean_pubkey_hash(&hash);
		break;

	case SSH_KNOWN_HOSTS_CHANGED:
		n = 0;
		hexa = ssh_get_hexa(hash, hlen);
		fprintf(stderr, "[!] The public key for this server has changed! It is now:\n");
		hptr = hexa;
		while (*hptr != '\0') {
			if ((n != 0) && (n % 30 == 0)) {
				fprintf(stderr, "\n");
			}

			if ((n != 0) && (n % 2 == 0) && (n % 30 != 0)) {
				fprintf(stderr, ":");
			}			
			fprintf(stderr, "%02X", *hptr);
			hptr++;
			n++;
		}
		fprintf(stderr, "\n\n");
		fprintf(stderr, "For security reasons, the connection will be closed.\n");
	
		ssh_string_free_char(hexa);
		ssh_clean_pubkey_hash(&hash);
		return -1;

	case SSH_KNOWN_HOSTS_OTHER:
		fprintf(stderr, "[!] The public key for this server was not found but another\n");
		fprintf(stderr, "    type of key exists. An attacker might change the default\n");
		fprintf(stderr, "    server key to confuse your client into thinking the key\n");
		fprintf(stderr, "    does not exist.\n");
		fprintf(stderr, "    For security reasons, the connection will be closed.\n");

		ssh_clean_pubkey_hash(&hash);
		return -1;

	case SSH_KNOWN_HOSTS_NOT_FOUND:
		fprintf(stderr, "[*] Could not find an OTW known hosts file. If you accept the\n");
		fprintf(stderr, "    public key here, the file will be automatically created.\n");
		// fall through

	case SSH_KNOWN_HOSTS_UNKNOWN:
		n = 0;
		hexa = ssh_get_hexa(hash, hlen);
		hptr = hexa;

		fprintf(stderr, "[*] Server's public key hash:\n");
		while (*hptr != '\0') {
			if ((n != 0) && (n % 24 == 0)) {
				fprintf(stderr, "\n");
			}

			if ((n != 0) && (n % 2 == 0) && (n % 24 != 0)) {
				fprintf(stderr, ":");
			}			
			fprintf(stderr, "%02X", *hptr);
			hptr++;
			n++;
		}

		fprintf(stderr, "\n\n");
		ssh_string_free_char(hexa);
		ssh_clean_pubkey_hash(&hash);

		fprintf(stderr, "[!] This server is currently unknown. Do you want to trust it?\n");
		fprintf(stderr, "    (yes|no): ");

		while (1) {
			fgets(buf, sizeof(buf), stdin);

			if (strncasecmp(buf, "yes", 3) == 0) {
				break;
			}

			if (strncasecmp(buf, "no", 2) == 0) {
				fprintf(stderr, "\n");
				return -1;
			}

			fprintf(stderr, "[!] Do you want to trust this server?\n");
			fprintf(stderr, "    (yes|no): ");
		}

		rc = ssh_session_update_known_hosts(session);
		if (rc < 0) {
			fprintf(stderr, "[-] Error while updating the known hosts file: %s\n", strerror(errno));
			return -1;
		}
		fprintf(stderr, "[+] Server saved to known hosts file.\n\n");
		break;

	case SSH_KNOWN_HOSTS_ERROR:
		fprintf(stderr, "[-] Error while processing the server's public key hash: %s\n", ssh_get_error(session));
		ssh_clean_pubkey_hash(&hash);
		return -1;
	}

	return 0;
}

int
authenticate_password(ssh_session session, level_t *level)
{
	int rc;
	size_t passlen;

	int attempts = 0;
	char pass[MAX_PASS_WIDTH] = {0};

	if ((strlen(level->pass) == 1) && (strncmp(level->pass, "?", 1) == 0)) {
		fprintf(stderr, "[!] Enter password: ");
		fgets(pass, sizeof(pass), stdin);
	
		passlen = strlen(pass);
		if ((passlen > 0) && (pass[passlen - 1] == '\n')) {
			pass[passlen - 1] = '\0';
		}
	
		rc = ssh_userauth_password(session, NULL, pass);
		attempts++;
	} else {
		rc = ssh_userauth_password(session, NULL, level->pass);
		attempts++;
	}

retry:

	if (rc == SSH_AUTH_DENIED) {
		fprintf(stderr, "[-] Incorrect password.\n\n");

		/* Give user three total attempts */
		if (attempts < 3) {
			memset(pass, 0, sizeof(pass));
			fprintf(stderr, "[!] Enter password: ");
			fgets(pass, sizeof(pass), stdin);
	
			passlen = strlen(pass);
			if ((passlen > 0) && (pass[passlen - 1] == '\n')) {
				pass[passlen - 1] = '\0';
			}
		
			rc = ssh_userauth_password(session, NULL, pass);
			attempts++;
			goto retry;
		}
		fprintf(stderr, "[-] Exceeded maximum number of attempts.\n");
	} else if (rc == SSH_AUTH_SUCCESS) {
		fprintf(stderr, "[+] You're now logged in as %s.\n\n", level->levelname);
	} else if (rc == SSH_AUTH_ERROR) {
		fprintf(stderr, "[-] Authentication error: %s\n", ssh_get_error(session));
	} else {
		fprintf(stderr, "[-] Authentication did not succeed.\n");
	}

	return rc;
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
