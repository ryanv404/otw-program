/* ssh_connect - OTW program */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <libssh/libssh.h>

#include "project/typedefs.h"

#define MAX_ADDR_WIDTH		60
#define DATADIR				"data"
#define KNOWNHOSTS_FILE		"data/known_hosts"

int verify_knownhost(ssh_session session);
int get_server_auth_methods(ssh_session session);
int authenticate_password(ssh_session session, level_t *level);
int run_remote_cmd(ssh_session session, char *cmd);
int interactive_shell_session(ssh_session session, ssh_channel channel);

int
connect_to_level(level_t *level)
{
	int rc;
	char hostname[MAX_ADDR_WIDTH];
	ssh_channel channel;
	ssh_session my_ssh_session;

	/* Open an ssh session */
	my_ssh_session = ssh_new();
	if (my_ssh_session == NULL) {
		fprintf(stderr, "[-] Error creating new ssh session.\n");
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

	channel = ssh_channel_new(my_ssh_session);
	if (channel == NULL) {
		rc = SSH_ERROR;
		fprintf(stderr, "[-] Could not allocate a new ssh channel.\n");
		goto shutdown;
	}

	/* Open channel to the ssh session */
	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK) {
		ssh_channel_free(channel);
		fprintf(stderr, "[-] Could not open an ssh session channel.\n");
		goto shutdown;
	}
	
	/* Get a pseudoterminal and start a shell on the remote machine */
	if (interactive_shell_session(my_ssh_session, channel) > 5) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
	}

shutdown:
	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);

	fprintf(stderr, "[*] Exiting now.\n");
	return rc;
}

int
verify_knownhost(ssh_session session)
{
	int rc;
	size_t hlen;
	char *hexa;
	char buf[10];
	enum ssh_known_hosts_e state;

	ssh_key srv_pubkey = NULL;
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
		break;

	case SSH_KNOWN_HOSTS_CHANGED:
		hexa = ssh_get_hexa(hash, hlen);
		fprintf(stderr, "[!] The public key for this server has changed!\n");
		fprintf(stderr, "    It is now: %s\n", hexa);
		fprintf(stderr, "    For security reasons, the connection will be closed.\n");
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
		hexa = ssh_get_hexa(hash, hlen);
		fprintf(stderr, "[*] Server's public key hash:\n%s\n\n", hexa);
		ssh_string_free_char(hexa);

		fprintf(stderr, "[*] This server is currently unknown. Do you want to trust it?\n");
		fprintf(stderr, "    (yes|no): ");
		scanf("%9s", buf);

		while (1) {
			if (strncasecmp(buf, "yes", 3) == 0) {
				break;
			}

			if (strncasecmp(buf, "no", 2) == 0) {
				ssh_clean_pubkey_hash(&hash);
				exit(EXIT_SUCCESS);
			}

			fprintf(stderr, "[*] Do you want to trust this server?\n");
			fprintf(stderr, "    (yes|no): ");
			scanf("%9s", buf);
		}

		rc = ssh_session_update_known_hosts(session);
		if (rc < 0) {
			ssh_clean_pubkey_hash(&hash);
			fprintf(stderr, "[-] Error while updating the known hosts file: %s\n", strerror(errno));
			return -1;
		}
		break;

	case SSH_KNOWN_HOSTS_ERROR:
		fprintf(stderr, "[-] Error while processing the server's public key hash: %s\n", ssh_get_error(session));
		ssh_clean_pubkey_hash(&hash);
		return -1;
	}

	ssh_clean_pubkey_hash(&hash);
	return 0;
}

int
get_server_auth_methods(ssh_session session)
{
	int method;

	int len = 0;
	char methods[4][20] = {{0}, {0}, {0}, {0}};

	ssh_userauth_none(session, NULL);
	method = ssh_userauth_list(session, NULL);

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

	fprintf(stderr, "[*] Accepted authentication methods: ");
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

int
authenticate_password(ssh_session session, level_t *level)
{
	int rc;
	char pass[MAX_PASS_WIDTH];

	if ((strlen(level->pass) == 1) && (strncmp(level->pass, "?", 1) == 0)) {
		fprintf(stderr, "[*] Enter %s's password: ", level->levelname);
		scanf("%46s", pass);
	} else {
		strncpy(pass, level->pass, MAX_PASS_WIDTH);
	}

	rc = ssh_userauth_password(session, NULL, pass);
	if (rc == SSH_AUTH_DENIED) {
		fprintf(stderr, "[-] Authentication denied.\n");
	} else if (rc == SSH_AUTH_SUCCESS) {
		fprintf(stderr, "[+] Authenticated as %s.\n\n", level->levelname);
	} else if (rc == SSH_AUTH_ERROR) {
		fprintf(stderr, "[-] Authentication error: %s\n", ssh_get_error(session));
	} else {
		fprintf(stderr, "[-] Unknown response from ssh_userauth_password.\n");
	}

	return rc;
}

int
interactive_shell_session(ssh_session session, ssh_channel channel)
{
	int rc;
	int nbytes, nwritten;
	char buffer[256];

	rc = ssh_channel_request_pty(channel);
	if (rc != SSH_OK) return -1;

	rc = ssh_channel_change_pty_size(channel, 80, 24);
	if (rc != SSH_OK) return -1;

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) return -1;

	while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
		struct timeval timeout;
		ssh_channel in_channels[2], out_channels[2];
		fd_set fds;
		int maxfd;

		timeout.tv_sec = 30;
		timeout.tv_usec = 0;

		in_channels[0] = channel;
		in_channels[1] = NULL;

		FD_ZERO(&fds);
		FD_SET(0, &fds);
		FD_SET(ssh_get_fd(session), &fds);
		maxfd = ssh_get_fd(session) + 1;

		ssh_select(in_channels, out_channels, maxfd, &fds, &timeout);

		if (out_channels[0] != NULL) {
			nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
			if (nbytes < 0) return 6;
			if (nbytes > 0) {
				nwritten = write(1, buffer, nbytes);
				if (nwritten != nbytes) return 6;
			}
		}

		if (FD_ISSET(0, &fds)) {
			nbytes = read(0, buffer, sizeof(buffer));
			if (nbytes < 0) return 7;
			if (nbytes > 0) {
				nwritten = ssh_channel_write(channel, buffer, nbytes);
				if (nbytes != nwritten) return 7;
			}
		}
	}

	return 8;
}

int
run_remote_cmd(ssh_session session, char *cmd)
{
	int rc, nbytes;
	char buffer[256];
	ssh_channel channel;

	channel = ssh_channel_new(session);
	if (channel == NULL) {
		fprintf(stderr, "[-] Error while executing remote command.\n");
		return SSH_ERROR;
	}

	rc = ssh_channel_open_session(channel);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Error while executing remote command.\n");
		ssh_channel_free(channel);
		return rc;
	}

	rc = ssh_channel_request_exec(channel, cmd);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Error while executing remote command.\n");
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return rc;
	}

	nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
	while (nbytes > 0) {
		if (write(1, buffer, nbytes) != (unsigned int) nbytes) {
			ssh_channel_close(channel);
			ssh_channel_free(channel);
			return SSH_ERROR;
		}
		nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
	}

	if (nbytes < 0) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	ssh_channel_send_eof(channel);
	ssh_channel_close(channel);
	ssh_channel_free(channel);

	return SSH_OK;
}
