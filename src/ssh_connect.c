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
#define KNOWNHOSTS		"data/known_hosts"

int
connect_to_game(level_t *level)
{
	int					s, i, rc;
	struct addrinfo 	hints;
	struct addrinfo    *res;
	struct sockaddr_in 	serveraddr;
	libssh2_socket_t 	sock;
	char 			   *userauthlist;
	const char 		   *fingerprint;

	char hostname[MAX_ADDR_WIDTH] = "";
	char addrbuf[INET_ADDRSTRLEN] = "";

	//LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;

	rc = libssh2_init(0);
	if (rc != 0) {
		fprintf(stderr, "[-] Could not initialize libssh2.\n");
		freeaddrinfo(res);
		return -1;
	}
	printf("[*] libssh2 initialized.\n");

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[-] Failed to a create socket.\n");
		freeaddrinfo(res);
		libssh2_exit();
		return -1;
	}
	printf("[*] Socket created.\n");

	sprintf(hostname, "%s.labs.overthewire.org", level->gamename);

	/* Obtain IPv4 address matching host/port */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family 	= AF_INET; 		/* Allow IPv4 */
	hints.ai_socktype 	= SOCK_STREAM; 	/* TCP socket */
	hints.ai_protocol 	= 0; 			/* Any protocol */
	hints.ai_flags		= 0;
	hints.ai_canonname 	= NULL;
	hints.ai_addr 		= NULL;
	hints.ai_next 		= NULL;
	
	s = getaddrinfo(hostname, level->port, &hints, &res);
	if (s != 0) {
		fprintf(stderr, "[-] getaddrinfo error: %s\n", gai_strerror(s));
		return -1;
	}

	memcpy(&serveraddr, res->ai_addr, sizeof(serveraddr));

	fprintf(stderr, "[*] Connecting to server...\n");
	if (connect(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
		fprintf(stderr, "[-] Failed to connect.\n");
		goto shutdown;
	}

	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("hostname.............%s\n", hostname);
	printf("ai_addr->sa_family...%s\n", (res->ai_addr->sa_family == 2) ? "IPv4" : "NOT IPv4");
	printf("ai_socktype..........%s\n",	(res->ai_socktype == 1) ? "SOCK_STREAM" : "NOT SOCK_STREAM");
	printf("ai_protocol..........%d\n",	res->ai_protocol);
	printf("ai_addr->sa_data.....%s\n", addrbuf);
	printf("sin_addr.............%s\n", inet_ntoa(serveraddr.sin_addr));
	printf("level->port..........%s\n", level->port);
	printf("sin_port.............%d\n", ntohs(serveraddr.sin_port));
	printf("ai_flags.............%d\n", res->ai_flags);
	printf("++++++++++++++++++++++++++++++++++++++++++++++++\n");
	print_level(level);

	freeaddrinfo(res);

	fprintf(stderr, "[+] Successfully connected to %s@%s:%d.\n", level->levelname, hostname,
			ntohs(serveraddr.sin_port));

	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[-] Could not initialize an SSH session.\n");
		goto shutdown;
	}

	rc = libssh2_session_handshake(session, sock);
	if (rc) {
		fprintf(stderr, "[-] Failure to establish an SSH session: %d\n", rc);
		goto shutdown;
	}

	/* Check the server's fingerprint against our known hosts. */
	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
	
	fprintf(stderr, "[*] Server fingerprint:\n");
	for (i = 0; i < 20; i++) {
		fprintf(stderr, "%02X ", (unsigned char) fingerprint[i]);
	}
	fprintf(stderr, "\n");

	if (access(KNOWNHOSTS, F_OK) != 0) {
		puts("[*] OTW known_hosts file does not exist.");
	} else {
		puts("[*] Checking for server's fingerprint in the OTW known_hosts file. ");
	}

	/* Check what authentication methods are available */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));
	
	if (userauthlist) {
		fprintf(stderr, "[+] Authentication methods: %s.\n", userauthlist);
	}

shutdown:
	if (session) {
		libssh2_session_disconnect(session, "Normal Shutdown");
		libssh2_session_free(session);
	}

	if (sock != LIBSSH2_INVALID_SOCKET) {
		shutdown(sock, 2);
		close(sock);
	}

	fprintf(stderr, "[+] All done. Exiting now.\n");
	libssh2_exit();
	return 0;
}

int ssh_connect(level_t *level)
{
	int i, rc;
	uint32_t hostaddr;
	char *userauthlist;
	const char *fingerprint;
	struct sockaddr_in sin;
	libssh2_socket_t sock;

	LIBSSH2_CHANNEL *channel = NULL;
	LIBSSH2_SESSION *session = NULL;

	char hostname[MAX_ADDR_WIDTH] = {0};
	sprintf(hostname, "%s@%s.labs.overthewire.org", level->levelname, level->gamename);
	
	printf("[*] hostname = %s\n", hostname);
	hostaddr = inet_addr(hostname);
	printf("[*] hostaddr = 0x%"PRIx32"\n", hostaddr);

	print_level(level);

	rc = libssh2_init(0);
	if (rc) {
		quit("[-] Could not initialize libssh2.\n");
	}

	printf("[*] libssh2 initialized. rc = %d\n", rc);


	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[-] Failed to a create socket.\n");
		rc = 1;
		goto shutdown;
	}

	printf("[*] Socket created. sock = %d\n", sock);

	sin.sin_family = AF_INET;
	sin.sin_port = htons(2220);
	sin.sin_addr.s_addr = hostaddr;

	fprintf(stderr, "[+] Connecting to %s:%d as user %s.\n",
			inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), level->levelname);

	if (connect(sock, (struct sockaddr *)(&sin), sizeof(struct sockaddr_in))) {
		fprintf(stderr, "[-] Failed to connect.\n");
		goto shutdown;
	}

	/* Create a session instance and start it up. This will trade welcome
	 * banners, exchange keys, and setup crypto, compression, and MAC layers
	 */

	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[-] Could not initialize an SSH session.\n");
		goto shutdown;
	}

	rc = libssh2_session_handshake(session, sock);
	if (rc) {
		fprintf(stderr, "[-] Failure to establish an SSH session: %d\n", rc);
		goto shutdown;
	}

	/* Check the hostkey's fingerprint against our known hosts. */
	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
	
	fprintf(stderr, "[+] Fingerprint: ");
	for (i = 0; i < 20; i++) {
		fprintf(stderr, "%02X ", (unsigned char) fingerprint[i]);
	}
	fprintf(stderr, "\n");

	/* Check what authentication methods are available */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));
	
	if (userauthlist) {
		fprintf(stderr, "[+] Authentication methods: %s.\n", userauthlist);
	}
	
	/* Authenticate with a password */
	if (libssh2_userauth_password(session, level->levelname, level->pass)) {
		fprintf(stderr, "[-] Authentication by password failed.\n");
		goto shutdown;
	} else {
		fprintf(stderr, "[+] Authentication by password succeeded.\n");
	}

	/* Request a session channel on which to run a shell */
	channel = libssh2_channel_open_session(session);
	if (!channel) {
		fprintf(stderr, "[-] Unable to open a session channel.\n");
		goto shutdown;
	}

	/* Request a terminal with 'vanilla' terminal emulation
	 * See /etc/termcap for more options. This is useful when opening
	 * an interactive shell.
	 */

	#if 0
	if (libssh2_channel_request_pty(channel, "vanilla")) {
		fprintf(stderr, "[-] Failed requesting pty.\n");
	}
	#endif

	if (libssh2_channel_shell(channel)) {
		fprintf(stderr, "[-] Unable to request a shell on the allocated pty.\n");
		goto shutdown;
	}

	/* At this point the shell can be interacted with using
		* libssh2_channel_read()
		* libssh2_channel_read_stderr()
		* libssh2_channel_write()
		* libssh2_channel_write_stderr()
		*
		* Blocking mode may be (en|dis)abled with:
		*    libssh2_channel_set_blocking()
		* If the server send EOF, libssh2_channel_eof() will return non-0
		* To send EOF to the server use: libssh2_channel_send_eof()
		* A channel can be closed with: libssh2_channel_close()
		* A channel can be freed with: libssh2_channel_free()
		*/

	/* Read and display all the data received on stdout (ignoring stderr)
		* until the channel closes. This will eventually block if the command
		* produces too much data on stderr; the loop must be rewritten to use
		* non-blocking mode and include interspersed calls to
		* libssh2_channel_read_stderr() to avoid this. See ssh2_echo.c for
		* an idea of how such a loop might look.
		*/

	while (!libssh2_channel_eof(channel)) {
		char buf[1024];
		ssize_t err = libssh2_channel_read(channel, buf, sizeof(buf));
		if (err < 0) {
			fprintf(stderr, "[-] Unable to read response: %d\n", (int) err);
		} else {
			fwrite(buf, 1, err, stdout);
		}
	}

	rc = libssh2_channel_get_exit_status(channel);

	if (libssh2_channel_close(channel)) {
		fprintf(stderr, "[-] Unable to close channel.\n");
	}

	if (channel) {
		libssh2_channel_free(channel);
		channel = NULL;
	}

shutdown:
	if (session) {
		libssh2_session_disconnect(session, "Normal Shutdown");
		libssh2_session_free(session);
	}

	if (sock != LIBSSH2_INVALID_SOCKET) {
		shutdown(sock, 2);
		close(sock);
	}

	fprintf(stderr, "[+] All done. Exiting now.\n");
	libssh2_exit();
	return rc;
}
