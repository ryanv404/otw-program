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
#include "project/validate.h"
#include "project/datautils.h"

#define MAX_ADDR_WIDTH	60
#define KNOWNHOSTS		"data/known_otw_hosts.dat"

int save_correct_pw(level_t *level, level_t **all_levels);
int clear_bad_pw(level_t *level, level_t **all_levels);
int check_for_known_host(const char *fingerprint, level_t *level);

int
connect_to_game(level_t *level, level_t **all_levels)
{
	int					s, rc;
	char 			   *userauthlist;
	const char 		   *fingerprint;
	struct addrinfo 	hints;
	struct sockaddr_in 	serveraddr;
	libssh2_socket_t 	sock;

	struct addrinfo    *res  = NULL;
	LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;
	char hostname[MAX_ADDR_WIDTH] = "";

	rc = libssh2_init(0);
	if (rc != 0) {
		fprintf(stderr, "[-] Could not initialize libssh2.\n");
		return -1;
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[-] Failed to a create socket.\n");
		rc = 1;
		goto shutdown;
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
		rc = 2;
		goto shutdown;
	}

	memcpy(&serveraddr, res->ai_addr, sizeof(serveraddr));

	fprintf(stderr, "[*] Connecting to %s on port %d as user %s.\n", hostname, ntohs(serveraddr.sin_port),
			level->levelname);

	if (connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
		fprintf(stderr, "[-] Failed to connect.\n");
		rc = 3;
		goto shutdown;
	}

	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[-] Could not initialize an SSH session.\n");
		rc = 4;
		goto shutdown;
	}

	rc = libssh2_session_handshake(session, sock);
	if (rc != 0) {
		fprintf(stderr, "[-] Failed to establish an SSH session.\n");
		rc = 5;
		goto shutdown;
	}

	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_MD5);
	if (fingerprint == NULL) {
		fprintf(stderr, "[-] Could not retrieve a fingerprint from the server.\n");
		rc = 6;
		goto shutdown;
	}

	/* Check the server's fingerprint against our known hosts */
	check_for_known_host(fingerprint, level);

	/* Ensure that the server accepts password authentication */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));
	if (!userauthlist || (strstr(userauthlist, "password") == NULL)) {
		fprintf(stderr, "[-] This server does not accept password authentication.\n");
		rc = 6;
		goto shutdown;
	}

	if (!level->is_pass_saved) {
		char passbuf[MAX_PASS_WIDTH];
		printf("[!] Enter %s's password: ", level->levelname);
		scanf("%51s", passbuf);
		strncpy(level->pass, passbuf, MAX_PASS_WIDTH);
	}

	/* Authenticate with the password */
	if (libssh2_userauth_password(session, level->levelname, level->pass)) {
		fprintf(stderr, "[-] Password authentication failed.\n");
		if (level->is_pass_saved) {
			/* Don't keep an incorrect password stored */
			clear_bad_pw(level, all_levels);
		}
		rc = 7;
		goto shutdown;
	}

	if (!level->is_pass_saved) {
		/* Password was correct so now we should store it */
		save_correct_pw(level, all_levels);
	}

	 /* Request a session channel on which to run a shell */
	 channel = libssh2_channel_open_session(session);
	 if (!channel) {
	 	fprintf(stderr, "[-] Unable to open a session channel.\n");
		rc = 8;
	 	goto shutdown;
	 }

	puts("[+] Channel opened to server.");

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

	if (libssh2_channel_close(channel)) {
		fprintf(stderr, "[-] Unable to close channel.\n");
	}

	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}

shutdown:
	free(level);
	free_levels(all_levels);
	
	if (res != NULL) {
		freeaddrinfo(res);
	}

	if (session) {
		libssh2_session_disconnect(session, "Normal shutdown");
		libssh2_session_free(session);
	}

	if (sock != LIBSSH2_INVALID_SOCKET) {
		shutdown(sock, 2);
		close(sock);
	}

	libssh2_exit();
	return rc;
}

int save_correct_pw(level_t *level, level_t **all_levels)
{
	int idx;

	idx = is_valid_level(level, all_levels);
	printf("saving pass for all_levels[%d]->levelname: %s\n", idx, all_levels[idx]->levelname);
	memcpy(all_levels[idx]->pass, level->pass, MAX_PASS_WIDTH - 1);
	all_levels[idx]->is_pass_saved = (uint8_t) 1;

	save_data(all_levels);
	printf("[+] Saved %s's password.\n", all_levels[idx]->levelname);
	return 0;
}

int clear_bad_pw(level_t *level, level_t **all_levels)
{
	int idx;

	idx = is_valid_level(level, all_levels);

	memcpy(all_levels[idx]->pass, "?", 2);
	all_levels[idx]->is_pass_saved = (uint8_t) 0;

	save_data(all_levels);
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

	memcpy(fprint.fingerprint, fingerprint, MD5_FINGERPRINT_WIDTH);
	strncpy(fprint.gamename, level->gamename, MAX_NAME_WIDTH);

	fprintf(stderr, "[*] %s's host fingerprint is:\n    ", level->levelname);
	for (int i = 0; i < 16; i++) {
		if ((i != 0) && (i % 4 == 0)) {
			fprintf(stderr, " ");
		}
		fprintf(stderr, "%02X", (unsigned char) fingerprint[i]);
	}

	if (access(KNOWNHOSTS, F_OK) == 0) {
		fp = fopen(KNOWNHOSTS, "rb+");
		/* If fp is NULL, we try to create a new file below */
		if (fp != NULL) {
			/* Check if host's fingerprint is in the known hosts file */
			while (fread(&fprint, sizeof(fprint), 1, fp) == 1) {
				if (strncmp(fingerprint, fprint.fingerprint, MD5_FINGERPRINT_WIDTH) == 0) {
					fprintf(stderr, " [KNOWN HOST]\n");
					fclose(fp);
					return 0;
				}
			}

			fprintf(stderr, " [UNKNOWN HOST]\n");

			/* Ensure that we're appending and clear any EOF */
			fseek(fp, 0, SEEK_END);

			/* Save the host's fingerprint to the known hosts file */
			fwrite(&fprint, sizeof(fprint), 1, fp);
			fclose(fp);
			return 0;
		}
	}

	fprintf(stderr, " [UNKNOWN HOST]\n");

	/* Create new known hosts file */
	fp = fopen(KNOWNHOSTS, "wb");
	if (fp == NULL) {
		fprintf(stderr, "[-] Unable to create an OTW known hosts file.\n");
		return 1;
	}

	/* Save the host's fingerprint to the known hosts file */
	fwrite(&fprint, sizeof(fprint), 1, fp);
	fclose(fp);

	return 0;
}
