/* libssh2_client.c */

#include "libssh2_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libssh2.h>

// #include "project/datautils.h"
#include "project/typedefs.h"
// #include "project/utils.h"
// #include "project/validate.h"

#define KNOWNHOSTS			"data/otw_knownhosts.txt"

#define MAX_ADDR_WIDTH		60
#define SHA256_FP_WIDTH		32

int  check_for_known_host(char *fingerprint, level_t *level);
void get_level_pass(level_t *level);
// void save_correct_pass(level_t *level, level_t **all_levels);
// void clear_bad_pass(level_t *level, level_t **all_levels);
void print_fingerprint(char *buf, int len);
void convert_bin2txt(const char *binbuf, char *txtbuf, int binlen);

static level_t lvl = {
	.levelname = "bandit0",
	.gamename = "bandit",
	.pass = "?",
	.port = "2220",
	.is_pass_saved = 0,
	.is_level_complete = 0
};

int
main(int argc, char **argv)
{

	(void) argc;
	(void) argv;

	level_t *level = &lvl;

	int s, rc;
	libssh2_socket_t sock;
	struct addrinfo hints;
	char hostname[MAX_ADDR_WIDTH];

	char fpbuf[100] 		 = {0};
	char *userauthlist		 = NULL;
	const char *fingerprint	 = NULL;
	struct addrinfo *res	 = NULL;
	LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;

	/* Init libssh2 functions */
	rc = libssh2_init(0);
	if (rc != 0) {
		fprintf(stderr, "[-] Could not initialize libssh2.\n");
		return -1;
	}

	/* Open a TCP IPv4 socket */
	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[-] Failed to a create socket.\n");
		rc = 1;
		goto shutdown;
	}

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family	  = AF_INET; 		/* Allow IPv4 */
	hints.ai_socktype = SOCK_STREAM; 	/* TCP socket */
	hints.ai_protocol = 0;				/* Any protocol */
	
	sprintf(hostname, "%s.labs.overthewire.org", level->gamename);

	/* Get TCP IPv4 address matching host:port */
	s = getaddrinfo(hostname, level->port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "[-] getaddrinfo error: %s\n", gai_strerror(s));
		rc = 1;
		goto shutdown;
	}

	fprintf(stderr, "[*] Connecting to %s:%s as user %s.\n", hostname,
			level->port, level->levelname);

	/* Connect to remote server */
	if (connect(sock, res->ai_addr, res->ai_addrlen)) {
		fprintf(stderr, "[-] Failed to connect.\n");
		rc = 1;
		goto shutdown;
	}

	/* Init SSH session object */
	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[-] Could not initialize an SSH session.\n");
		rc = 1;
		goto shutdown;
	}

	/* Begin transport layer protocol negotiation */
	rc = libssh2_session_handshake(session, sock);
	if (rc != 0) {
		fprintf(stderr, "[-] Failed to establish an SSH session.\n");
		goto shutdown;
	}

	/* Get SHA-256 digest of remote server's hostkey */
	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA256);
	if (fingerprint == NULL) {
		fprintf(stderr, "[-] Could not retrieve remote server's hostkey.\n");
		rc = 1;
		goto shutdown;
	}

	/* Check the server's hostkey against our saved known hosts */
	convert_bin2txt(fingerprint, fpbuf, SHA256_FP_WIDTH);
	if (check_for_known_host(fpbuf, level) != 0) {
		rc = 1;
		goto shutdown;
	}

	/* Ensure that the server accepts password authentication */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));

	if (!userauthlist || (strstr(userauthlist, "password") == NULL)) {
		fprintf(stderr, "[-] This server does not appear to accept password authentication.\n");
		rc = 1;
		goto shutdown;
	}

	/* Get pass from user if it's not stored */
	if (!level->is_pass_saved) get_level_pass(level);

	/* Authenticate with password */
	if (libssh2_userauth_password(session, level->levelname, 
		level->pass) != 0) {
		fprintf(stderr, "[-] Password authentication failed.\n");
		if (level->is_pass_saved) {
			/* Don't keep an incorrect password stored */
			//clear_bad_pass(level, all_levels);
			fprintf(stderr, "bad pass was cleared.\n");
		}
		rc = 1;
		goto shutdown;
	}

	/* Password was correct; now we should store it if it's not already stored */
	if (!level->is_pass_saved) {
		fprintf(stderr, "correct pass saved.\n");
		//save_correct_pass(level, all_levels);
	}

	 /* Allocate a channel to exchange data with the server */
	 channel = libssh2_channel_open_session(session);
	 if (!channel) {
	 	fprintf(stderr, "[-] Unable to open a session channel.\n");
		rc = 1;
	 	goto shutdown;
	 }

	/* Request a remote pty */
	if (libssh2_channel_request_pty(channel, "xterm-256color")) {
		fprintf(stderr, "[-] Request for remote pseudoterminal failed.\n");
		rc = 1;
	 	goto shutdown;
	}

	/* Request a remote shell */
	if (libssh2_channel_shell(channel)) {
		fprintf(stderr, "[-] Request for remote shell failed.\n");
		rc = 1;
	 	goto shutdown;
	}

	puts("[+] Got a remote shell.");

	/* Main loop here */

	rc = libssh2_channel_get_exit_status(channel);

	if (libssh2_channel_close(channel))
		fprintf(stderr, "Unable to close channel.\n");

	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}

shutdown:

	if (session) {
		libssh2_session_disconnect(session, "Normal shutdown");
		libssh2_session_free(session);
	}

	if (sock != LIBSSH2_INVALID_SOCKET) {
		shutdown(sock, 2);
		close(sock);
	}

	libssh2_exit();
	printf("libssh2_client return value: %d\n", rc);
	return rc;
}

int
check_for_known_host(char *fingerprint, level_t *level)
{
	int retries;

	FILE *fp = NULL;
	char linebuf[100] = {0};
	char namebuf[MAX_NAME_WIDTH] = {0};
	char fpbuf[SHA256_FP_WIDTH * 2] = {0};

	fp = fopen(KNOWNHOSTS, "r+");

	if (fp != NULL) {
		/* Scan known hosts file for a match */
		while (fgets(linebuf, sizeof(linebuf), fp) != NULL) {
			fprintf(stderr, "linebuf:\n%s\n", linebuf);

			sscanf(linebuf, "%[^:]:%[^\n]", namebuf, fpbuf);

			fprintf(stderr, "namebuf:\n%s\n", namebuf);

			fprintf(stderr, "fpbuf:\n");
			print_fingerprint(fpbuf, SHA256_FP_WIDTH * 2);

			fprintf(stderr, "fingerprint:\n");
			print_fingerprint(fingerprint, SHA256_FP_WIDTH * 2);
			
			if (strncmp(level->gamename, namebuf, MAX_NAME_WIDTH) == 0) {
				/* Host name match */
				if (strncmp(fingerprint, fpbuf, SHA256_FP_WIDTH) == 0) {
					/* Hostkey match */
					fclose(fp);
					return 0;
				} else {
					/* Hostkey has changed for this server */
					fprintf(stderr, 
							"[!] This server is in your known hosts file but it's hostkey\n"
							"    has changed. Its current key is:\n");
					print_fingerprint(fingerprint, SHA256_FP_WIDTH * 2);
					fprintf(stderr, "[!] For security reasons, this connection will be closed.\n");
					fclose(fp);
					return 1;
				}
			}
		}

		/* Distinguish between file stream error and EOF */
		if (feof(fp) && !ferror(fp)) {
			/* Ensure that we're appending and clear any EOF */
			fseek(fp, 0, SEEK_END);
		} else {
			fprintf(stderr, "[*] Error while processing the known hosts file.\n");
			fp = NULL;
		}
	} else {
		/* Create a known hosts file if one doesn't exist */
		fp = fopen(KNOWNHOSTS, "w");

		if (fp == NULL) {
			fprintf(stderr, "[*] Unable to create a known hosts file.\n");
		}
	}

	/* No match; known hosts file is open; writes will append */
	fprintf(stderr, "[*] SHA-256 digest of remote server's hostkey:\n");
	print_fingerprint(fingerprint, SHA256_FP_WIDTH * 2);

	retries = 0;
	do {
		memset(linebuf, 0, 100);
		fprintf(stderr, "[!] This server is currently unknown to you. Would you like to\n"
						"    trust it? (yes/no): ");

		fgets(linebuf, 100, stdin);

		if (strncasecmp(linebuf, "y", 1) == 0) {
			if (fp != NULL) {
				fprintf(fp, "%s:%s\n", level->gamename, fingerprint);
				fclose(fp);
			}
			return 0;
		} else if (strncasecmp(linebuf, "n", 1) == 0) {
			break;
		}
		retries++;
	} while (retries < 3);

	if (fp != NULL) {
		fclose(fp);
	}
	
	return 1;
}

void
get_level_pass(level_t *level)
{
	size_t len;
	char passbuf[MAX_PASS_WIDTH] = {0};

	fprintf(stderr, "[!] Enter password: ");
	fgets(passbuf, MAX_PASS_WIDTH, stdin);

	len = strlen(passbuf);
	
	if ((len > 0) && (passbuf[len - 1] == '\n')) {
		passbuf[len - 1] = '\0';
	}

	strncpy(level->pass, passbuf, MAX_PASS_WIDTH);
	return;
}

// void
// save_correct_pass(level_t *level, level_t **all_levels)
// {
// 	int idx = is_valid_level(level, all_levels);

// 	printf("saving pass for all_levels[%d]->levelname: %s\n", idx, all_levels[idx]->levelname);
// 	memcpy(all_levels[idx]->pass, level->pass, MAX_PASS_WIDTH - 1);
// 	all_levels[idx]->is_pass_saved = (uint8_t) 1;

// 	save_data(all_levels);
// 	printf("[+] Saved %s's password.\n", all_levels[idx]->levelname);

// 	return;
// }

// void
// clear_bad_pass(level_t *level, level_t **all_levels)
// {
// 	int idx = is_valid_level(level, all_levels);

// 	memcpy(all_levels[idx]->pass, "?", 2);
// 	all_levels[idx]->is_pass_saved = (uint8_t) 0;

// 	save_data(all_levels);
// 	return;
// }

void
convert_bin2txt(const char *binbuf, char *txtbuf, int binlen)
{
	for (int i = 0; i < binlen; i++) {
		sprintf(&txtbuf[strlen(txtbuf)], "%02X", (unsigned char) binbuf[i]);
	}
	return;
}

void
print_fingerprint(char *buf, int len)
{
	for (int i = 0; i < len; i++) {
		if (i == 0) {
			fprintf(stderr, "    ");
		} else if (i % 32 == 0) {
			fprintf(stderr, "\n    ");
		} else if (i % 8 == 0) {
			fprintf(stderr, " ");
		}

		fprintf(stderr, "%1s", (char *) &buf[i]);
	}

	fprintf(stderr, "\n\n");
	return;
}

