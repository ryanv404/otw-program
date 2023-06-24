/* libssh2_client.c */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <inttypes.h>
#include <fcntl.h>
#include <poll.h>
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

#define KNOWNHOSTS			"data/otw_knownhosts"
#define MAX_ADDR_WIDTH		60
#define SHA256_FP_WIDTH		32
#define BUFSIZE 			32000

#define errExit(msg)	 do { 	perror(msg);		 \
								return EXIT_FAILURE; \
							} while (0)
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
static int waitsocket(libssh2_socket_t socket_fd, LIBSSH2_SESSION *session);
static int ssh2_echo(LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel, libssh2_socket_t sock);
int poll_example(libssh2_socket_t sock);

int
main(int argc, char **argv)
{

	(void) argc;
	(void) argv;

	int s, rc;
	libssh2_socket_t sock;
	struct addrinfo hints;
	char hostname[MAX_ADDR_WIDTH];

	char *userauthlist		 = NULL;
	const char *fingerprint	 = NULL;
	struct addrinfo *res	 = NULL;
	LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;

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
	if (check_for_known_host(fingerprint, level) != 0) {
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
	if (libssh2_userauth_password(session, level->levelname, level->pass) != 0) {
		fprintf(stderr, "[-] Password authentication failed.\n");
		if (level->is_pass_saved) {
			/* Don't keep an incorrect password stored */
			clear_bad_pass(level, all_levels);
		}
		rc = 1;
		goto shutdown;
	}

	/* Password was correct; now we should store it if it's not already stored */
	if (!level->is_pass_saved) {
		save_correct_pass(level, all_levels);
	}

    /* Exec non-blocking on the remote host */
	do {
		channel = libssh2_channel_open_session(session);
		if ((channel) || (libssh2_session_last_error(session, NULL, NULL, 0) != LIBSSH2_ERROR_EAGAIN)) {
			break;
		}

		waitsocket(sock, session);
	} while (1);

	if (!channel) {
		fprintf(stderr, "[-] Unable to open a session channel.\n");
	}

	/* Allocate a channel to exchange data with the server */
//	channel = libssh2_channel_open_session(session);
//	if (!channel) {
//		fprintf(stderr, "[-] Unable to open a session channel.\n");
//		rc = 1;
//		goto shutdown;
//	}

//	/* Request a remote pty */
//	if (libssh2_channel_request_pty(channel, "xterm-256color")) {
//		fprintf(stderr, "[-] Request for remote pseudoterminal failed.\n");
//		rc = 1;
//	 	goto shutdown;
//	}
//
//	/* Request a remote shell */
//	if (libssh2_channel_shell(channel)) {
//		fprintf(stderr, "[-] Request for remote shell failed.\n");
//		rc = 1;
//	 	goto shutdown;
//	}

	/* Main loop starts */

	puts("[+] Got a remote shell. Starting main loop.");

	ssh2_echo(session, channel, sock);

	/* Main loop ends */

//	rc = libssh2_channel_get_exit_status(channel);
//
//	if (libssh2_channel_close(channel))
//		fprintf(stderr, "[-] Unable to close channel.\n");

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

	fprintf(stderr, "correct pass saved.\n");

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

	fprintf(stderr, "bad pass was cleared.\n");

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

static int
waitsocket(libssh2_socket_t socket_fd, LIBSSH2_SESSION *session)
{
	int rc, dir;
	fd_set fd;
	struct timeval timeout;

	fd_set *writefd = NULL;
	fd_set *readfd  = NULL;

	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;

	FD_ZERO(&fd);
	FD_SET(socket_fd, &fd);

	/* Now make sure we wait in the correct direction */
	dir = libssh2_session_block_directions(session);

	if (dir & LIBSSH2_SESSION_BLOCK_INBOUND)  readfd  = &fd;
	if (dir & LIBSSH2_SESSION_BLOCK_OUTBOUND) writefd = &fd;

	rc = select((int)(socket_fd + 1), readfd, writefd, NULL, &timeout);

	return rc;
}

static int
ssh2_echo(LIBSSH2_SESSION *session, LIBSSH2_CHANNEL *channel, libssh2_socket_t sock)
{
	int rc;

	int   exitcode = 0;
	char  commandline[] = "id";
	char *exitsignal = "none";

	while ((rc = libssh2_channel_exec(channel, commandline)) == LIBSSH2_ERROR_EAGAIN) {
		waitsocket(sock, session);
	}

	if (rc) {
		fprintf(stderr, "Exec error.\n");
		return 1;
	} else {
		int i;
		char buffer[BUFSIZE];

		int running  = 1;
		int rereads  = 0;
		int rewrites = 0;
		ssize_t bufsize    = BUFSIZE;
		ssize_t totread    = 0;
		ssize_t totwritten = 0;
		ssize_t totsize    = 1500000;
		LIBSSH2_POLLFD *fds = NULL;

		for (i = 0; i < BUFSIZE; i++) {
			buffer[i] = 'A';
		}

		fds = (LIBSSH2_POLLFD *) malloc(sizeof(LIBSSH2_POLLFD));
		if (!fds) {
			fprintf(stderr, "Malloc failed.\n");
			return 1;
		}

		fds[0].type = LIBSSH2_POLLFD_CHANNEL;
		fds[0].fd.channel = channel;
		fds[0].events = LIBSSH2_POLLFD_POLLIN | LIBSSH2_POLLFD_POLLOUT;

		do {
			int act = 0;

			rc = libssh2_poll(fds, 1, 10);
			if (rc < 1) continue;

			if (fds[0].revents & LIBSSH2_POLLFD_POLLIN) {
				ssize_t n = libssh2_channel_read(channel, buffer, sizeof(buffer));
				act++;

				if (n == LIBSSH2_ERROR_EAGAIN) {
					rereads++;
					fprintf(stderr, "Will read again.\n");
				} else if (n < 0) {
					fprintf(stderr, "Read failed.\n");
					return 1;
				} else {
					totread += n;
					fprintf(stderr, "Read %d bytes (%d in total).\n", (int) n, (int) totread);
				}
			}

			if (fds[0].revents & LIBSSH2_POLLFD_POLLOUT) {
				act++;

				if (totwritten < totsize) {
					/* we have not written all data yet */
					ssize_t left = totsize - totwritten;
					ssize_t size = (left < bufsize) ? left : bufsize;
					ssize_t n    = libssh2_channel_write_ex(channel, 0, buffer, size);

					if (n == LIBSSH2_ERROR_EAGAIN) {
						rewrites++;
						fprintf(stderr, "Will write again.\n");
					} else if (n < 0) {
						fprintf(stderr, "Write failed.\n");
						return 1;
					} else {
						totwritten += n;
						fprintf(stderr, "Wrote %d bytes (%d in total).", (int) n, (int) totwritten);
						if ((left >= bufsize) && (n != bufsize)) {
							fprintf(stderr, " PARTIAL");
						}
						fprintf(stderr, "\n");
					}
				} else {
					/* All data has been written, send EOF */
					rc = libssh2_channel_send_eof(channel);

					if (rc == LIBSSH2_ERROR_EAGAIN) {
						fprintf(stderr, "Will send eof again.\n");
					} else if (rc < 0) {
						fprintf(stderr, "Send eof failed.\n");
						return 1;
					} else {
						fprintf(stderr, "Sent eof.\n");
						/* we're done writing, stop listening for OUT events */
						fds[0].events &= ~LIBSSH2_POLLFD_POLLOUT;
					}
				}
			}

			if (fds[0].revents & LIBSSH2_POLLFD_CHANNEL_CLOSED) {
				/* Don't leave loop until we have read all data */
				if (!act) running = 0;
			}
		} while (running);

		exitcode = 127;

		while ((rc = libssh2_channel_close(channel)) == LIBSSH2_ERROR_EAGAIN) {
			waitsocket(sock, session);
		}

		if (rc == 0) {
			exitcode = libssh2_channel_get_exit_status(channel);
			libssh2_channel_get_exit_signal(channel, &exitsignal, NULL, NULL, NULL, NULL, NULL);
		}

		if (exitsignal) {
			fprintf(stderr, "\nGot signal: %s\n", exitsignal);
		}

		libssh2_channel_free(channel);
		channel = NULL;

		fprintf(stderr, "\nrereads: %d, rewrites: %d, totwritten %d\n", rereads, rewrites, (int) totwritten);

//		if (totwritten != totread) {
//			fprintf(stderr, "\n*** FAIL bytes written: %d, bytes read: %d ***\n", (int) totwritten, (int) totread);
//			return 1;
//		}
	}

	return exitcode;
}

int
poll_example(libssh2_socket_t sock)
{
	(void) sock;

#if 0

	int ready;
	char buf[10];
	ssize_t s;
	nfds_t j;

	nfds_t nfds = 1;
	nfds_t num_open_fds = 1;
	struct pollfd *pfds = NULL;

	pfds = (struct pollfd *) calloc(nfds, sizeof(struct pollfd));
	if (pfds == NULL) errExit("calloc");

	/* Add fd to 'pfds' array. */
	pfds[0].fd = sock;
	pfds[0].events = POLLIN | POLLOUT;

	/* Keep calling poll as long as at least one file descriptor is open. */
	while (num_open_fds > 0) {
		fprintf(stderr, "About to poll.\n");

		ready = poll(pfds, nfds, -1);
		if (ready == -1) errExit("poll");

		fprintf(stderr, "%d file %s ready.\n", ready, (ready == 1) ? "descriptor" : "descriptors");

		/* Deal with array returned by poll(). */
		for (j = 0; j < nfds; j++) {
			if (pfds[j].revents != 0) {
				fprintf(stderr, "  fd: %d; events: %s%s%s\n", pfds[j].fd,
						(pfds[j].revents & POLLIN)  ? "POLLIN "  : "",
						(pfds[j].revents & POLLHUP) ? "POLLHUP " : "",
						(pfds[j].revents & POLLERR) ? "POLLERR " : "");

				if (pfds[j].revents & POLLIN) {
					s = read(pfds[j].fd, buf, sizeof(buf));
					if (s == -1) errExit("read");

					fprintf(stderr, "    read %zd bytes: %.*s\n", s, (int) s, buf);
				} else {                /* POLLERR | POLLHUP */
					fprintf(stderr, "    closing fd %d\n", pfds[j].fd);

					if (close(pfds[j].fd) == -1) errExit("close");
					num_open_fds--;
				}
			}
		}
	}

	fprintf(stderr, "All file descriptors closed; bye.\n");

#endif

	return EXIT_SUCCESS;
}
