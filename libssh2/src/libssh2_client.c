/* libssh2_client.c */

#include "lssh2/ssh.h"

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

#include "lssh2/config.h"
#include "lssh2/misc.h"
#include "lssh2/data.h"
#include "lssh2/validate.h"

int
libssh2_connect(level_t *level, level_t **all_levels)
{
	int rc, written, old_flags, new_flags, numfds;
	struct addrinfo hints;
	struct termios oldtc, tc;
	libssh2_socket_t sock;

	char inputbuf[BUFSIZE] = {0};
	char commandbuf[BUFSIZE] = {0};
	char hostname[MAX_ADDR_WIDTH] = {0};

	int termchanged 		 = 0;
	char *userauthlist		 = NULL;
	const char *fingerprint	 = NULL;
	struct addrinfo *res	 = NULL;
	LIBSSH2_SESSION *session = NULL;
	LIBSSH2_CHANNEL *channel = NULL;

	numfds = 2;
	struct pollfd pfds[numfds];

	/* Initialize libssh2 functions */
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
			store_pass(level->pass, level->levelname, level, all_levels);
		}
	} else {
		fprintf(stderr, "[-] Password authentication failed.\n");
		if (level->is_pass_saved) {
			/* Don't keep an incorrect password stored */
			store_pass(NULL, level->levelname, level, all_levels);
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

	/* De-initialize libssh2 functions */
	libssh2_exit();

	/* Restore local terminal to its original state */
	if (termchanged) {
		/* Reset stdin file status flags; re-enable blocking mode */
		fcntl(STDIN_FILENO, F_SETFL, old_flags);

		/* Reset to original local terminal settings */
		tcsetattr(STDIN_FILENO, TCSANOW, &oldtc);
	}

	return rc;
}
