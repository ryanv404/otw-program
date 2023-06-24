/* libssh2_client.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <libssh2.h>

#include "project/typedefs.h"
// #include "project/datautils.h"
// #include "project/utils.h"
// #include "project/validate.h"

#define KNOWNHOSTS			"data/otw_knownhosts"
#define MAX_ADDR_WIDTH		60
#define SHA256_FP_WIDTH		32
#define BUFSIZE 			1024

static level_t lvl = {
	.levelname = "bandit0",
	.gamename  = "bandit",
	.pass      = "?",
	.port      = "2220",
	.is_pass_saved     = 0,
	.is_level_complete = 0
};

static int check_for_known_host(const char *fingerprint, level_t *level);
static void get_level_pass(level_t *level);
static void save_correct_pass(level_t *level, level_t **all_levels);
static void clear_bad_pass(level_t *level, level_t **all_levels);
static void print_hex_fingerprint(const char *buf, int len);

int
main(int argc, char **argv)
{

	(void) argc;
	(void) argv;

	int rc, written, old_flags, new_flags;
	libssh2_socket_t sock;
	struct addrinfo hints;
	struct termios oldtc, tc;
	char inputbuf[BUFSIZE];
	char commandbuf[BUFSIZE];
	char hostname[MAX_ADDR_WIDTH];

	char *userauthlist		 = NULL;
	const char *fingerprint	 = NULL;
	struct addrinfo *res	 = NULL;
	LIBSSH2_SESSION *session = NULL;
	LIBSSH2_CHANNEL *channel = NULL;

	
	int termchanged = 0;
	const char numfds = 2;
	struct pollfd pfds[numfds];

	level_t *level       = &lvl;
	level_t **all_levels = NULL;

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
	rc = getaddrinfo(hostname, level->port, &hints, &res);
	if (rc != 0) {
		fprintf(stderr, "[-] getaddrinfo error: %s\n", gai_strerror(rc));
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
	rc = check_for_known_host(fingerprint, level);
	if (rc != 0) {
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

	/* Authenticate with password */
	get_level_pass(level);

	if (libssh2_userauth_password(session, level->levelname, level->pass) == 0) {
		/* Store correct password if it's not already stored */
		fprintf(stderr, "[+] Logged in as %s.\n", level->levelname);
		if (!level->is_pass_saved) {
			save_correct_pass(level, all_levels);
		}
	} else {
		fprintf(stderr, "[-] Password authentication failed.\n");
		if (level->is_pass_saved) {
			/* Don't keep an incorrect password stored */
			clear_bad_pass(level, all_levels);
		}
		rc = 1;
		goto shutdown;
	}

	/* Allocate a channel to exchange data with the server */
	channel = libssh2_channel_open_session(session);
	if (!channel) {
		fprintf(stderr, "[-] Unable to open a session channel.\n");
		rc = 1;
		goto shutdown;
	}

	/* Set socket to non-blocking */
	rc = fcntl(sock, F_SETFL, O_NONBLOCK);
	if (rc == -1) {
		fprintf(stderr, "[-] Failed to set socket to non-blocking.\n");
		goto shutdown;
	}

	/* Set stdin to non-blocking */
	old_flags = fcntl(STDIN_FILENO, F_GETFL);
	new_flags = old_flags | O_NONBLOCK;
	rc = fcntl(STDIN_FILENO, F_SETFL, new_flags);

	termchanged = 1;
	if (rc == -1) {
		fprintf(stderr, "[-] Failed to set stdin to non-blocking.\n");
		goto shutdown;
	}

	/* Request a remote pty */
	rc = libssh2_channel_request_pty(channel, "xterm-256color");
	if (rc) {
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

	/* Set non-blocking mode on channel */
	libssh2_channel_set_blocking(channel, 0);

	/* Set stdin to raw mode */
	tcgetattr(STDIN_FILENO, &tc);
	memcpy(&oldtc, &tc, sizeof(struct termios));
	cfmakeraw(&tc);
	tcsetattr(STDIN_FILENO, TCSANOW, &tc);

	/* Prepare to use poll */
	memset(pfds, 0, sizeof(struct pollfd) * numfds);

	/* I/O polling loop starts */
	do {
		/* Declare that we need to wait while socket or stdin is not ready for reading */
		pfds[0].fd = sock;
		pfds[1].fd = STDIN_FILENO;
		pfds[0].events = POLLIN;
		pfds[1].events = POLLIN;
		pfds[0].revents = 0;
		pfds[1].revents = 0;

		/* Polling on socket and stdin while not ready to read from it */
		rc = poll(pfds, numfds, -1);
		if (rc < 0) {
			fprintf(stderr, "[-] Error while polling for I/O events.\n");
			rc = 1;
			goto shutdown;
		}

		if (pfds[0].revents & POLLIN) {
			/* Read output from remote side */
			do {
				rc = libssh2_channel_read(channel, inputbuf, BUFSIZE);
				printf("%s", inputbuf);
				fflush(stdout);
				memset(inputbuf, 0, BUFSIZE);
			} while ((rc != LIBSSH2_ERROR_EAGAIN) && (rc > 0));
		}

		if ((rc < 0) && (rc != LIBSSH2_ERROR_EAGAIN)) {
			fprintf(stderr, "[-] libssh2_channel_read error code %d\n", rc);
			goto shutdown;
		}

		if (pfds[1].revents & POLLIN) {
			/* Request for command input */
			fgets(commandbuf, BUFSIZE - 2, stdin);

			/* Write command to stdin of remote shell */
			written = 0;
			do {
				rc = libssh2_channel_write(channel, commandbuf, strlen(commandbuf));
				written += rc;
			} while ((rc != LIBSSH2_ERROR_EAGAIN) && (rc > 0) && (written != (int) strlen(commandbuf)));
		
			if ((rc < 0) && (rc != LIBSSH2_ERROR_EAGAIN)) {
				fprintf(stderr, "[-] libssh2_channel_write error code %d\n", rc);
				goto shutdown;
			}

			memset(commandbuf, 0, BUFSIZE);
		}
	} while (libssh2_channel_eof(channel) == 0);

shutdown:

	/* Do channel clean up */
	if (channel) {
		do {
			rc = libssh2_channel_close(channel);
		} while (rc == LIBSSH2_ERROR_EAGAIN);

		if (rc != 0) fprintf(stderr, "[-] Unable to close channel.\n");

		do {
			rc = libssh2_channel_free(channel);
		} while (rc == LIBSSH2_ERROR_EAGAIN);

		if (rc != 0) fprintf(stderr, "[-] Unable to free channel resources.\n");
		channel = NULL;

	}

	/* Do transport layer session clean up */
	if (session) {
		do {
			rc = libssh2_session_disconnect(session, "Normal shutdown");
		} while (rc == LIBSSH2_ERROR_EAGAIN);

		if (rc != 0) fprintf(stderr, "[-] Unable to terminate session.\n");
		
		do {
			rc = libssh2_session_free(session);
		} while (rc == LIBSSH2_ERROR_EAGAIN);

		if (rc != 0) fprintf(stderr, "[-] Unable to free session resources.\n");
	}

	/* Do socket clean up */
	if (sock != LIBSSH2_INVALID_SOCKET) {
		rc = shutdown(sock, SHUT_RDWR);
		if (rc != 0) fprintf(stderr, "[-] Error while shutting down the socket.\n");
		rc = close(sock);
		if (rc != 0) fprintf(stderr, "[-] Error while closing the socket.\n");
	}

	libssh2_exit();

	if (termchanged) {
		/* Reset stdin file status flags */
		fcntl(STDIN_FILENO, F_SETFL, old_flags);

		/* Revert to original local terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);
	}

	return rc;
}

static int
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

static void
get_level_pass(level_t *level)
{
	size_t len;
	char passbuf[MAX_PASS_WIDTH] = {0};

	if (level->is_pass_saved) {
		return;
	}

	fprintf(stderr, "[!] Enter password: ");
	fgets(passbuf, MAX_PASS_WIDTH, stdin);

	len = strlen(passbuf);
	
	if ((len > 0) && (passbuf[len - 1] == '\n')) {
		passbuf[len - 1] = '\0';
	}

	strncpy(level->pass, passbuf, MAX_PASS_WIDTH);
	return;
}

static void
save_correct_pass(level_t *level, level_t **all_levels)
{
	(void) level;
	(void) all_levels;

	fprintf(stderr, "[*] Password saved.\n");

//	int idx;
//	
//	idx = is_valid_level(level, all_levels);
//
//	printf("saving pass for all_levels[%d]->levelname: %s\n", idx, all_levels[idx]->levelname);
//	memcpy(all_levels[idx]->pass, level->pass, MAX_PASS_WIDTH - 1);
//	all_levels[idx]->is_pass_saved = 1;
//
//	save_data(all_levels);
//	printf("[+] Saved %s's password.\n", all_levels[idx]->levelname);
	return;
}

static void
clear_bad_pass(level_t *level, level_t **all_levels)
{
	(void) level;
	(void) all_levels;

	fprintf(stderr, "[*] Cleared stored password that did not work.\n");

//	int idx;
//	
//	idx = is_valid_level(level, all_levels);
//
//	memcpy(all_levels[idx]->pass, "?", 2);
//	all_levels[idx]->is_pass_saved = 0;
//
//	save_data(all_levels);
	return;
}

static void
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
