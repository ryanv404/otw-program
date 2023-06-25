/* libssh_client.c - OTW program */

#include "lssh/ssh.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

#include <sys/time.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <fcntl.h>
#include <termios.h>

#include <libssh/libssh.h>

#include "lssh/config.h"

static int signal_delayed = 0;
static struct termios terminal = {0};

static void do_cleanup(int i);
static void do_exit(int i);
static void sigwindowchanged(int i);
static void setsignal(void);
static void sizechanged(ssh_channel chan);
static void select_loop(ssh_session session, ssh_channel channel);
static int  shell(ssh_session session);
static int  client(ssh_session session, level_t *level);
static int  verify_knownhost(ssh_session session);
static int  do_password_authentication(ssh_session session, level_t *level);

int
libssh_connect(level_t *level)
{
	int rc;
	ssh_session session;

	/* Initialize libssh functions */
	rc = ssh_init();
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Failed to initialize libssh.\n");
		return SSH_ERROR;
	}

	/* Allocate a new ssh session */
	session = ssh_new();
	if (session == NULL) {
		fprintf(stderr, "[-] Failed to create a new ssh session.\n");
		return SSH_ERROR;
	}

	signal(SIGTERM, do_exit);

	/* Launch ssh client */
	client(session, level);

	if (session) {
		ssh_disconnect(session);
		ssh_free(session);
	}

	/* Necessary when ssh_init is explicitly called */
	rc = ssh_finalize();
	if (rc != SSH_OK) fprintf(stderr, "[-] Error while deinitializing libssh.\n");

	return SSH_OK;
}

static void
do_cleanup(int i)
{
	/* unused variable */
	(void) i;

	/* Restore local terminal to its original state */
	tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
	return;
}

static void
do_exit(int i)
{
	/* unused variable */
	(void) i;
	do_cleanup(0);
	return;
}

static void
sigwindowchanged(int i)
{
	(void) i;
	signal_delayed = 1;
	return;
}

static void
setsignal(void)
{
	signal(SIGWINCH, sigwindowchanged);
	signal_delayed = 0;
	return;
}

static void
sizechanged(ssh_channel chan)
{
	int rc;

	struct winsize win = {
		.ws_row = 0,
		.ws_col = 0
	};

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	rc = ssh_channel_change_pty_size(chan, win.ws_col, win.ws_row);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Unable to resize the pty.\n");
	}

	setsignal();
	return;
}

static void
select_loop(ssh_session session, ssh_channel channel)
{
	int rc;
	ssh_connector connector_in, connector_out, connector_err;

	ssh_event event = ssh_event_new();

	connector_in  = ssh_connector_new(session);
	connector_out = ssh_connector_new(session);
	connector_err = ssh_connector_new(session);

	/* stdin */
	ssh_connector_set_out_channel(connector_in, channel, SSH_CONNECTOR_STDINOUT);
	ssh_connector_set_in_fd(connector_in, STDIN_FILENO);
	ssh_event_add_connector(event, connector_in);

	/* stdout */
	ssh_connector_set_out_fd(connector_out, STDOUT_FILENO);
	ssh_connector_set_in_channel(connector_out, channel, SSH_CONNECTOR_STDINOUT);
	ssh_event_add_connector(event, connector_out);

	/* stderr */
	ssh_connector_set_out_fd(connector_err, STDERR_FILENO);
	ssh_connector_set_in_channel(connector_err, channel, SSH_CONNECTOR_STDERR);
	ssh_event_add_connector(event, connector_err);

	while (ssh_channel_is_open(channel)) {
		if (signal_delayed) {
			sizechanged(channel);
		}

		rc = ssh_event_dopoll(event, 60000);
		if (rc == SSH_ERROR) {
			fprintf(stderr, "[-] Error in ssh_event_dopoll.\n");
			break;
		}
	}
	
	ssh_event_remove_connector(event, connector_in);
	ssh_event_remove_connector(event, connector_out);
	ssh_event_remove_connector(event, connector_err);

	ssh_connector_free(connector_in);
	ssh_connector_free(connector_out);
	ssh_connector_free(connector_err);

	ssh_event_free(event);
	return;
}

static int
shell(ssh_session session)
{
	int rc;
	ssh_channel channel;
	struct termios terminal_local;

	struct winsize win = {
		.ws_row = 0,
		.ws_col = 0
	};
	
	/* Allocate a new channel */
	channel = ssh_channel_new(session);
	if (channel == NULL) {
		fprintf(stderr, "[-] Error while creating a channel.\n");
		return SSH_ERROR;
	}

	/* Open a channel to exchange data with the remote server */
	if (ssh_channel_open_session(channel)) {
		fprintf(stderr, "[-] Error opening channel.\n");
		ssh_channel_free(channel);
		return SSH_ERROR;
	}
	
	/* Save current terminal state */
	tcgetattr(STDIN_FILENO, &terminal_local);
	memcpy(&terminal, &terminal_local, sizeof(struct termios));

	/*  Request a remote pty */
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	rc = ssh_channel_request_pty_size(channel, "xterm-256color", win.ws_col, win.ws_row);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Request for remote pty failed.\n");
		do_cleanup(0);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	rc = ssh_channel_request_shell(channel);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Request for remote shell failed.\n");
		do_cleanup(0);
		ssh_channel_free(channel);
		return SSH_ERROR;
	}

	/* Set raw mode for the local terminal */
	cfmakeraw(&terminal_local);
	tcsetattr(STDIN_FILENO, TCSANOW, &terminal_local);
	setsignal();
	
	signal(SIGTERM, do_cleanup);

	/* Start polling with I/O select loop */
	select_loop(session, channel);

	do_cleanup(0);
	ssh_channel_free(channel);

	return SSH_OK;
}

static int
client(ssh_session session, level_t *level)
{
	int rc;

	int   auth = 0;
	int   port = atoi(level->port);
	char *banner = NULL;
	char  hostname[MAX_ADDR_WIDTH] = {0};

	sprintf(hostname, "%s.labs.overthewire.org", level->gamename);
	
	/* Configure session to connect to the level and to use the local known hosts file */
	if ((ssh_options_set(session, SSH_OPTIONS_HOST, hostname) < 0)         ||
	    (ssh_options_set(session, SSH_OPTIONS_PORT, &port) < 0)            ||
	    (ssh_options_set(session, SSH_OPTIONS_USER, level->levelname) < 0) ||
	    (ssh_options_set(session, SSH_OPTIONS_SSH_DIR, DATADIR) < 0)       ||
	    (ssh_options_set(session, SSH_OPTIONS_KNOWNHOSTS, KNOWNHOSTS) < 0) ||
	    (ssh_options_set(session, SSH_OPTIONS_GLOBAL_KNOWNHOSTS, KNOWNHOSTS) < 0)) {
		fprintf(stderr, "[-] Error while setting ssh options.\n");
		ssh_free(session);
		return SSH_ERROR;
	}

	/* Connect to the server */
	rc = ssh_connect(session);
	if (rc != SSH_OK) {
		fprintf(stderr, "[-] Error connecting to %s: %s\n", hostname, ssh_get_error(session));
		ssh_free(session);
		return SSH_ERROR;
	}

	/* Verify the server's identity */
	if (verify_knownhost(session) < 0) {
		return SSH_ERROR;
	}

	/* Get server's banner */
	ssh_userauth_none(session, NULL);
	banner = ssh_get_issue_banner(session);
	if (banner) {
		printf("%s\n", banner);
		free(banner);
	}

	/* Authenticate with password */
	auth = do_password_authentication(session, level);
	if (auth != SSH_AUTH_SUCCESS) {
		return SSH_ERROR;
	}
	
	/* Run remote shell */
	shell(session);

	return SSH_OK;
}

static int
do_password_authentication(ssh_session session, level_t *level)
{
	int rc, retries;
	int methods;

	char password[128] = {0};

	/* Ensure the remote server accepts password authentication */
	rc = ssh_userauth_none(session, NULL);
	methods = ssh_userauth_list(session, NULL);

	/* Authenticate with password */
	retries = 0;
	if (methods & SSH_AUTH_METHOD_PASSWORD) {
		while (retries < 3) {
			/* Try to authenticate with password */
			if ((level->is_pass_saved) && (retries == 0)) {
				rc = ssh_userauth_password(session, NULL, level->pass);
			} else {
				if (ssh_getpass("\nPassword: ", password, sizeof(password), 1, 0) < 0) {
					return SSH_AUTH_ERROR;
				}
				rc = ssh_userauth_password(session, NULL, password);
			}

			if (rc == SSH_AUTH_ERROR) {
				fprintf(stderr, "[-] Authentication failed.\n");
				return SSH_AUTH_ERROR;
			} else if (rc == SSH_AUTH_SUCCESS) {
				return SSH_AUTH_SUCCESS;
			} else {
				fprintf(stderr, "[-] Incorrect password.\n");
			}

			memset(password, 0, sizeof(password));
			retries++;
		}
	} else {
		fprintf(stderr, "[-] Server does not appear to accept password authentication.\n");
	}

	return SSH_AUTH_ERROR;
}

static int
verify_knownhost(ssh_session session)
{
	int rc;
	size_t hlen;
	ssh_key srv_pubkey;
	enum ssh_known_hosts_e state;

	char buf[10] = {0};
	unsigned char *hash = NULL;

	rc = ssh_get_server_publickey(session, &srv_pubkey);
	if (rc < 0) return SSH_ERROR;

	rc = ssh_get_publickey_hash(srv_pubkey,
								SSH_PUBLICKEY_HASH_SHA256,
								&hash,
								&hlen);

	ssh_key_free(srv_pubkey);
	if (rc < 0) return SSH_ERROR;

	state = ssh_session_is_known_server(session);

	switch (state) {
	case SSH_KNOWN_HOSTS_CHANGED:
		fprintf(stderr, "[-] Host key for the server has changed.\n"
						"    The server's current host key is:\n\n");
		ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
		ssh_clean_pubkey_hash(&hash);
		fprintf(stderr, "\n[-] For security reasons, the connection will be stopped.\n");
		return SSH_ERROR;

	case SSH_KNOWN_HOSTS_OTHER:
		fprintf(stderr, "[-] The host key for this server was not found but an other type of\n"
						"    key exists. An attacker might change the default server key to\n"
						"    confuse your client into thinking the key does not exist.\n"
						"    It is advised that you rerun the client with -d or -r for more safety.\n");
		return SSH_ERROR;

	case SSH_KNOWN_HOSTS_NOT_FOUND:
		fprintf(stderr, "[-] Could not find a known hosts file. If you accept the host key here,\n"
						"    the file will be automatically created.\n");
		/* fall through */

	case SSH_SERVER_NOT_KNOWN:
		fprintf(stderr, "[*] This server is currently unknown:\n");
		ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
		fprintf(stderr, "\n[*] Do you trust the host's key (yes/no)? ");

		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			ssh_clean_pubkey_hash(&hash);
			return SSH_ERROR;
		}

		if (strncasecmp(buf, "yes", 3) != 0) {
			ssh_clean_pubkey_hash(&hash);
			return SSH_ERROR;
		}
		break;

	case SSH_KNOWN_HOSTS_ERROR:
		ssh_clean_pubkey_hash(&hash);
		fprintf(stderr, "%s", ssh_get_error(session));
		return SSH_ERROR;

	case SSH_KNOWN_HOSTS_OK:
		break; /* ok */
	}

	ssh_clean_pubkey_hash(&hash);
	return SSH_OK;
}
