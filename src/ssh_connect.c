/* ssh_connect.c - OTW program */

#include "project/ssh_connect.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libssh2.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "project/typedefs.h"
#include "project/utils.h"

const char *pubkey  = ".ssh/id_ed25519.pub";
const char *privkey = ".ssh/id_ed25519";
const char *knownhosts = ".ssh/known_hosts";

int
connect_to_game(level_t *level)
{
	puts("connect_to_game has been called.");
	print_level(level);
	return 0;
}

int
ssh_connect(level_t *level)
{
	int i, rc;
	uint32_t hostaddr;
	char *userauthlist;
	const char *fingerprint;
	struct sockaddr_in sin;
	LIBSSH2_CHANNEL *channel;
	libssh2_socket_t sock;

	LIBSSH2_SESSION *session = NULL;

	hostaddr = inet_addr(level->hostaddr);

	rc = libssh2_init(0);
	if (rc) {
		quit("[Error] libssh2 initialization failed.\n");
	}

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == LIBSSH2_INVALID_SOCKET) {
		fprintf(stderr, "[Error] failed to create socket.\n");
		rc = 1;
		goto shutdown;
	}

	sin.sin_family = AF_INET;
	sin.sin_port = htons(level->port);
	sin.sin_addr.s_addr = hostaddr;

	fprintf(stderr, "[SSH] connecting to %s:%d as user %s\n",
			inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), level->levelname);

	if (connect(sock, (struct sockaddr *)(&sin), sizeof(struct sockaddr_in))) {
		fprintf(stderr, "[Error] failed to connect.\n");
		goto shutdown;
	}

	/* Create a session instance and start it up. This will trade welcome
	 * banners, exchange keys, and setup crypto, compression, and MAC layers
	 */

	session = libssh2_session_init();
	if (!session) {
		fprintf(stderr, "[Error] could not initialize SSH session.\n");
		goto shutdown;
	}

	rc = libssh2_session_handshake(session, sock);
	if (rc) {
		fprintf(stderr, "[Error] failure establishing SSH session: %d\n", rc);
		goto shutdown;
	}
	rc = 1;

	/* At this point we have not yet authenticated.  The first thing to do
	 * is check the hostkey's fingerprint against our known hosts. Your app
	 * may have it hard coded, may go to a file, may present it to the
	 * user, that's your call
	 */

	fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
	fprintf(stderr, "[SSH] Fingerprint: ");
	for (i = 0; i < 20; i++) {
		fprintf(stderr, "%02X ", (unsigned char) fingerprint[i]);
	}
	fprintf(stderr, "\n");

	/* Check what authentication methods are available */
	userauthlist = libssh2_userauth_list(session, level->levelname,
										 (unsigned int) strlen(level->levelname));
	
	if (userauthlist) {
		fprintf(stderr, "[SSH] authentication methods: %s\n", userauthlist);
	}
	
	/* Authenticate with a password */
	if (libssh2_userauth_password(session, level->levelname, level->pass)) {
		fprintf(stderr, "[Error] authentication by password failed.\n");
		goto shutdown;
	} else {
		fprintf(stderr, "[SSH] authentication by password succeeded.\n");
	}

	/* Request a session channel on which to run a shell */
	channel = libssh2_channel_open_session(session);
	if (!channel) {
		fprintf(stderr, "[Error] unable to open a session\n");
		goto shutdown;
	}

	/* Request a terminal with 'vanilla' terminal emulation
	 * See /etc/termcap for more options. This is useful when opening
	 * an interactive shell.
	 */

	#if 0
	if(libssh2_channel_request_pty(channel, "vanilla")) {
		fprintf(stderr, "Failed requesting pty\n");
	}
	#endif

	if(libssh2_channel_shell(channel)) {
		fprintf(stderr, "[Error] unable to request shell on allocated pty\n");
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
			fprintf(stderr, "[Error] unable to read response: %d\n", (int)err);
		} else {
			fwrite(buf, 1, err, stdout);
		}
	}

	rc = libssh2_channel_get_exit_status(channel);

	if (libssh2_channel_close(channel)) {
		fprintf(stderr, "[Error] unable to close channel.\n");
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

	fprintf(stderr, "all done!\n");
	libssh2_exit();
	return rc;
}
