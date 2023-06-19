/* ssh_connect - OTW program */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#include <libssh/libssh.h>
#include <sys/ioctl.h>

#include "project/typedefs.h"

#define MAX_ADDR_WIDTH		60
#define MAX_BUFSIZE			1024
#define DATADIR				"data"
#define KNOWNHOSTS_FILE		"data/known_hosts"

int verify_knownhost(ssh_session session);
int get_server_auth_methods(ssh_session session);
int authenticate_password(ssh_session session, level_t *level);
int run_remote_cmd(ssh_session session, char *cmd);
int interactive_shell_session(ssh_session session);

int
connect_to_level(level_t *level)
{
	int rc;
	char hostname[MAX_ADDR_WIDTH];
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

	/* Get a pty and start an interactive shell on the remote machine */
	if (interactive_shell_session(my_ssh_session) != SSH_OK) {
		fprintf(stderr, "[-] Received an error from the remote shell.\n");
	}

shutdown:
	ssh_disconnect(my_ssh_session);
	ssh_free(my_ssh_session);

	fprintf(stderr, "\n[*] Exiting now.\n");
	return rc;
}

int
verify_knownhost(ssh_session session)
{
	int rc, n;
	size_t hlen;
	char *hexa, *hptr;
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
		n = 0;
		hexa = ssh_get_hexa(hash, hlen);
		fprintf(stderr, "[!] The public key for this server has changed! It is now:\n");
		hptr = hexa;
		while (*hptr != '\0') {
			if ((n != 0) && (n % 30 == 0)) {
				fprintf(stderr, "\n");
			}

			if ((n != 0) && (n % 2 == 0) && (n % 30 != 0)) {
				fprintf(stderr, ":");
			}			
			fprintf(stderr, "%02X", *hptr);
			hptr++;
			n++;
		}
		fprintf(stderr, "\n\n");
		fprintf(stderr, "For security reasons, the connection will be closed.\n");
		ssh_string_free_char(hexa);
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
		n = 0;
		hexa = ssh_get_hexa(hash, hlen);
		hptr = hexa;
		fprintf(stderr, "[*] Server's public key hash:\n");
		while (*hptr != '\0') {
			if ((n != 0) && (n % 30 == 0)) {
				fprintf(stderr, "\n");
			}

			if ((n != 0) && (n % 2 == 0) && (n % 30 != 0)) {
				fprintf(stderr, ":");
			}			
			fprintf(stderr, "%02X", *hptr);
			hptr++;
			n++;
		}

		fprintf(stderr, "\n\n");
		ssh_string_free_char(hexa);
		ssh_clean_pubkey_hash(&hash);

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
		fprintf(stderr, "[-] Authentication did not succeed.\n");
	}

	return rc;
}

int
interactive_shell_session(ssh_session session)
{
	fd_set fds;
	int nbytes, nwritten, maxfd;
	struct timeval timeout;
	ssh_channel channel;
	ssh_channel in_channels[2], out_channels[2];
	struct winsize sz;

	char inbuffer[MAX_BUFSIZE]  = {0};
	char outbuffer[MAX_BUFSIZE] = {0};

	/* Open channel to the ssh connection */
	if ((channel = ssh_channel_new(session)) == NULL) {
		return SSH_ERROR;
	}

	if (ssh_channel_open_session(channel) != SSH_OK) {
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	/* Get the current window size: columns & rows */
	ioctl(0, TIOCGWINSZ, &sz);

	/* Set up remote pseudo-tty and request a shell */
	if (ssh_channel_request_pty_size(channel, "xterm-256color", sz.ws_col, sz.ws_row) != SSH_OK) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	if (ssh_channel_request_shell(channel) != SSH_OK) {
		ssh_channel_close(channel);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
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
			nbytes = ssh_channel_read(channel, outbuffer, sizeof(outbuffer), 0);
			if (nbytes < 0) {
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				return SSH_ERROR;
			}

			if (nbytes > 0) {
				nwritten = write(1, outbuffer, nbytes);
				if (nwritten != nbytes) {
					ssh_channel_close(channel);
					ssh_channel_free(channel);
					return SSH_ERROR;
				}
			}
		}

		if (FD_ISSET(0, &fds)) {
			nbytes = read(0, inbuffer, sizeof(inbuffer));
			if (nbytes < 0) {
				ssh_channel_close(channel);
				ssh_channel_free(channel);
				return SSH_ERROR;
			}

			if (nbytes > 0) {
				nwritten = ssh_channel_write(channel, inbuffer, nbytes);
				if (nbytes != nwritten) {
					ssh_channel_close(channel);
					ssh_channel_free(channel);
					return SSH_ERROR;
				}
			}
		}
	}

	if (ssh_channel_is_open(channel)) {
		ssh_channel_close(channel);
	}

	ssh_channel_free(channel);
	return SSH_OK;
}
