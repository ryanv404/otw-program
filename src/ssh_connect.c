/* ssh_connect.c - OTW program */

#include "project/ssh_connect.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <libssh2.h>

#include "project/typedefs.h"
#include "project/utils.h"

#define MAX_ADDR_WIDTH	60
#define KNOWNHOSTS		"data/known_otw_hosts.dat"

int check_for_known_host(const char *fingerprint, level_t *level);

int
connect_to_game(level_t *level)
{
	int					s, rc;
	char 			   *userauthlist;
	const char 		   *fingerprint;
	struct addrinfo 	hints;
	struct addrinfo    *res;
	struct sockaddr_in 	serveraddr;
	libssh2_socket_t 	sock;

	char hostname[MAX_ADDR_WIDTH] = "";

	//LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;

	rc = libssh2_init(0);
	if (rc != 0) {
		fprintf(stderr, "[-] Could not initialize libssh2.\n");
		freeaddrinfo(res);
		return -1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[-] Failed to a create socket.\n");
		freeaddrinfo(res);
		libssh2_exit();
		return -1;
	}

	/* Obtain IPv4 address matching host/port */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family 	= AF_INET; 		/* Allow IPv4 */
	hints.ai_socktype 	= SOCK_STREAM; 	/* TCP socket */
	hints.ai_protocol 	= 0; 			/* Any protocol */
	hints.ai_flags		= 0;
	hints.ai_canonname 	= NULL;
	hints.ai_addr 		= NULL;
	hints.ai_next 		= NULL;
	
	sprintf(hostname, "%s.labs.overthewire.org", level->gamename);

	s = getaddrinfo(hostname, level->port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "[-] getaddrinfo error: %s\n", gai_strerror(s));
		return -1;
	}

	memcpy(&serveraddr, res->ai_addr, sizeof(serveraddr));

	if (connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
		fprintf(stderr, "[-] Failed to connect.\n");
		goto shutdown;
	}

	// printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// printf("hostname.............%s\n", hostname);
	// printf("ai_addr->sa_family...%s\n", (res->ai_addr->sa_family == 2) ? "IPv4" : "NOT IPv4");
	// printf("ai_socktype..........%s\n",	(res->ai_socktype == 1) ? "SOCK_STREAM" : "NOT SOCK_STREAM");
	// printf("ai_protocol..........%d\n",	res->ai_protocol);
	// printf("sin_addr.............%s\n", inet_ntoa(serveraddr.sin_addr));
	// printf("level->port..........%s\n", level->port);
	// printf("sin_port.............%d\n", ntohs(serveraddr.sin_port));
	// printf("ai_flags.............%d\n", res->ai_flags);
	// printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	// print_level(level);

	freeaddrinfo(res);

	fprintf(stderr, "[+] Connected to %s:%d.\n", hostname, ntohs(serveraddr.sin_port));

	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[-] Could not initialize an SSH session.\n");
		goto shutdown;
	}

	rc = libssh2_session_handshake(session, sock);
	if (rc != 0) {
		fprintf(stderr, "[-] Failed to establish an SSH session.\n");
		goto shutdown;
	}

	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_MD5);
	
	if (fingerprint == NULL) {
		fprintf(stderr, "[-] Could not retrieve a fingerprint from the server. Exiting now.\n");
		goto shutdown;
	}

	/* Check the server's fingerprint against our known hosts */
	check_for_known_host(fingerprint, level);

	/* Ensure that the server accepts password authentication */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));
	
	if (!userauthlist || (strstr(userauthlist, "password") == NULL)) {
		puts("[-] This server does not accept password authentication.");
		goto shutdown;
	}

	/* Authenticate with a password */
	if (libssh2_userauth_password(session, level->levelname, level->pass)) {
		fprintf(stderr, "[-] Password authentication failed.\n");
		goto shutdown;
	} else {
		fprintf(stderr, "[+] Authenticated as %s.\n", level->levelname);
	}

// /* Request a session channel on which to run a shell */
// channel = libssh2_channel_open_session(session);
// if (!channel) {
// 	fprintf(stderr, "[-] Unable to open a session channel.\n");
// 	goto shutdown;
// }

// /* Request a terminal with 'vanilla' terminal emulation
// 	* See /etc/termcap for more options. This is useful when opening
// 	* an interactive shell.
// 	*/

// #if 0
// if (libssh2_channel_request_pty(channel, "vanilla")) {
// 	fprintf(stderr, "[-] Failed requesting pty.\n");
// }
// #endif

// if (libssh2_channel_shell(channel)) {
// 	fprintf(stderr, "[-] Unable to request a shell on the allocated pty.\n");
// 	goto shutdown;
// }

// /* At this point the shell can be interacted with using
// 	* libssh2_channel_read()
// 	* libssh2_channel_read_stderr()
// 	* libssh2_channel_write()
// 	* libssh2_channel_write_stderr()
// 	*
// 	* Blocking mode may be (en|dis)abled with:
// 	*    libssh2_channel_set_blocking()
// 	* If the server send EOF, libssh2_channel_eof() will return non-0
// 	* To send EOF to the server use: libssh2_channel_send_eof()
// 	* A channel can be closed with: libssh2_channel_close()
// 	* A channel can be freed with: libssh2_channel_free()
// 	*/

// /* Read and display all the data received on stdout (ignoring stderr)
// 	* until the channel closes. This will eventually block if the command
// 	* produces too much data on stderr; the loop must be rewritten to use
// 	* non-blocking mode and include interspersed calls to
// 	* libssh2_channel_read_stderr() to avoid this. See ssh2_echo.c for
// 	* an idea of how such a loop might look.
// 	*/

// while (!libssh2_channel_eof(channel)) {
// 	char buf[1024];
// 	ssize_t err = libssh2_channel_read(channel, buf, sizeof(buf));
// 	if (err < 0) {
// 		fprintf(stderr, "[-] Unable to read response: %d\n", (int) err);
// 	} else {
// 		fwrite(buf, 1, err, stdout);
// 	}
// }

// rc = libssh2_channel_get_exit_status(channel);

// if (libssh2_channel_close(channel)) {
// 	fprintf(stderr, "[-] Unable to close channel.\n");
// }

// if (channel) {
// 	libssh2_channel_free(channel);
// 	channel = NULL;
// }

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
	return 0;
}


int
check_for_known_host(const char *fingerprint, level_t *level)
{
	FILE *fp;
	fingerprint_t fprint = {
		.fingerprint = {0},
		.gamename = {0}
	};

	fprintf(stderr, "[*] Server's fingerprint:");
	for (int i = 0; i < 16; i++) {
		if (i % 4 == 0) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "%02x", (unsigned char) fingerprint[i]);
	}
	fprintf(stderr, "\n");

	if (access(KNOWNHOSTS, F_OK) == 0) {
		fp = fopen(KNOWNHOSTS, "rb+");
		if (fp == NULL) {
			fprintf(stderr, "[-] Unable to open the OTW known hosts file.\n");
		} else {
			/* Check if host's fingerprint is in the known hosts file */
			while (fread(&fprint, sizeof(fprint), 1, fp) == 1) {
				if (strncmp(fingerprint, fprint.fingerprint, MD5_FINGERPRINT_WIDTH) == 0) {
					puts("[*] This server is known.");
					fclose(fp);
					return 0;
				}
			}
			/* Save the host's fingerprint to the known hosts file */
			memcpy(fprint.fingerprint, fingerprint, MD5_FINGERPRINT_WIDTH);
			strncpy(fprint.gamename, level->gamename, MAX_NAME_WIDTH);

			/* Ensure that we're appending and clear any EOF */
			fseek(fp, 0, SEEK_END);

			fwrite(&fprint, sizeof(fprint), 1, fp);
			fprintf(stderr, "[*] Server's fingerprint has been saved.\n");
			fclose(fp);
		}
	} else {
		fp = fopen(KNOWNHOSTS, "wb");
		if (fp == NULL) {
			fprintf(stderr, "[-] Unable to create an OTW known hosts file.\n");
		} else {
			/* Save the host's fingerprint to the known hosts file */
			memcpy(fprint.fingerprint, fingerprint, MD5_FINGERPRINT_WIDTH);
			strncpy(fprint.gamename, level->gamename, MAX_NAME_WIDTH);

			fwrite(&fprint, sizeof(fprint), 1, fp);
			fprintf(stderr, "[*] This server has been saved to the known hosts file.\n");
			fclose(fp);
		}
	}
	return 0;
}
