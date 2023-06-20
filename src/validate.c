/* validate.c - OTW program */

#include "project/validate.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <libssh/libssh.h>

#include "project/typedefs.h"

#define TOTAL_LEVELS	183

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
