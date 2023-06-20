/* ssh_connect - OTW program */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <libssh/libssh.h>

#include "project/typedefs.h"
#include "project/validate.h"
#include "project/term.h"

#define MAX_ADDR_WIDTH		60
#define DATADIR				"data"
#define KNOWNHOSTS_FILE		"data/known_hosts"

int get_server_auth_methods(ssh_session session);

int
connect_to_level(level_t *level)
{
	int rc;
	char hostname[MAX_ADDR_WIDTH];
	ssh_session my_ssh_session;

	/* Initialize a new ssh session */
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL) {
		fprintf(stderr, "[-] Error initializing a new ssh session.\n");
		return -1;
	}

	/* Configure session to connect to the level and to use the local known hosts file */
	sprintf(hostname, "%s.labs.overthewire.org", level->gamename);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, 				hostname);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, 				&level->port);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, 				level->levelname);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_SSH_DIR, 			DATADIR);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_KNOWNHOSTS, 		KNOWNHOSTS_FILE);
	ssh_options_set(my_ssh_session, SSH_OPTIONS_GLOBAL_KNOWNHOSTS, 	KNOWNHOSTS_FILE);

	/* Connect to the server */
	rc = ssh_connect(my_ssh_session);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Error connecting to %s: %s\n", hostname, ssh_get_error(my_ssh_session));
		ssh_free(my_ssh_session);
		return rc;
	}

	/* Verify the server's identity */
	if (verify_knownhost(my_ssh_session) < 0) {
		rc = -1;
		goto shutdown;
	}

	/* Authenticate with password */
	if (authenticate_password(my_ssh_session, level) != SSH_AUTH_SUCCESS) {
		rc = -1;
		goto shutdown;
	}

	/* Get a pty and start an interactive shell on the remote machine */
	if (interactive_shell_session(my_ssh_session) != SSH_OK) {
		fprintf(stderr, "[-] Received an error from the remote shell.\n");
		rc = -1;
	}

shutdown:
	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);

	return rc;
}

int
get_server_auth_methods(ssh_session session)
{
	int method;

	int len = 0;
	char methods[6][20] = {{0}, {0}, {0}, {0}, {0}, {0}};

	ssh_userauth_none(session, NULL);
	method = ssh_userauth_list(session, NULL);

	if (method & SSH_AUTH_METHOD_NONE) {
		strcpy(methods[len], "none");
		len++;
	}

	if (method & SSH_AUTH_METHOD_PASSWORD) {
		strcpy(methods[len], "password");
		len++;
	}

	if (method & SSH_AUTH_METHOD_PUBLICKEY) {
		strcpy(methods[len], "publickey");
		len++;
	}

	if (method & SSH_AUTH_METHOD_HOSTBASED) {
		strcpy(methods[len], "hostbased");
		len++;
	}

	if (method & SSH_AUTH_METHOD_INTERACTIVE) {
		strcpy(methods[len], "kbd-interactive");
		len++;
	}

	if (method & SSH_AUTH_METHOD_GSSAPI_MIC) {
		strcpy(methods[len], "gssapi-with-mic");
		len++;
	}

	fprintf(stderr, "[*] Authentication methods accepted by server: ");
	if (len) {
		for (int i = 0; i < len; i++) {
			fprintf(stderr, "%s", methods[i]);
			if (i == (len - 1)) {
				fprintf(stderr, ".\n");
			} else {
				fprintf(stderr, ", ");
			}
		}
	} else {
		fprintf(stderr, "unknown.\n");
	}
	return 0;
}
