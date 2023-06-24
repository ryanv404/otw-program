/* libssh_client.c */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/select.h>
#include <sys/time.h>
#include <termios.h>
#include <unistd.h>
#include <pty.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

#include <libssh/libssh.h>

static struct termios terminal;

static int   signal_delayed = 0;
static char *host = NULL;
static char *user = NULL;

static void error(ssh_session session);
static void do_cleanup(int i);
static void do_exit(int i);
static void sigwindowchanged(int i);
static void setsignal(void);
static void sizechanged(ssh_channel chan);
static void select_loop(ssh_session session, ssh_channel channel);
static void shell(ssh_session session);
static int  client(ssh_session session);
static int  verify_knownhost(ssh_session session);
static int  authenticate_console(ssh_session session);

int
main(int argc, char **argv)
{
	ssh_session session;

	ssh_init();
	session = ssh_new();

	signal(SIGTERM, do_exit);

	client(session);

	ssh_disconnect(session);
	ssh_free(session);

	ssh_finalize();
	return EXIT_SUCCESS;
}

static void
do_cleanup(int i)
{
  /* unused variable */
  (void) i;
  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
  return;
}

static void
do_exit(int i)
{
	/* unused variable */
	(void) i;
	do_cleanup(0);
	exit(EXIT_SUCCESS);
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
	struct winsize win = {
		.ws_row = 0,
		.ws_col = 0
	};

	ioctl(STDOUT_FILENO, TIOCGWINSZ, &win);
	ssh_channel_request_pty_size(chan, "xterm-256color", win.ws_col, win.ws_row);
	
	setsignal();
	return;
}

static void
select_loop(ssh_session session, ssh_channel channel)
{
	int rc;
	ssh_connector connector_in, connector_out, connector_err;

	ssh_event event = ssh_event_new();

	/* stdin */
	connector_in = ssh_connector_new(session);
	ssh_connector_set_out_channel(connector_in, channel, SSH_CONNECTOR_STDINOUT);
	ssh_connector_set_in_fd(connector_in, STDIN_FILENO);
	ssh_event_add_connector(event, connector_in);

	/* stdout */
	connector_out = ssh_connector_new(session);
	ssh_connector_set_out_fd(connector_out, STDOUT_FILENO);
	ssh_connector_set_in_channel(connector_out, channel, SSH_CONNECTOR_STDINOUT);
	ssh_event_add_connector(event, connector_out);

	/* stderr */
	connector_err = ssh_connector_new(session);
	ssh_connector_set_out_fd(connector_err, STDERR_FILENO);
	ssh_connector_set_in_channel(connector_err, channel, SSH_CONNECTOR_STDERR);
	ssh_event_add_connector(event, connector_err);

	while (ssh_channel_is_open(channel)) {
		if (signal_delayed) {
			sizechanged(channel);
		}

		rc = ssh_event_dopoll(event, 60000);
		if (rc == SSH_ERROR) {
			fprintf(stderr, "Error in ssh_event_dopoll.\n");
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

static void
shell(ssh_session session)
{
	ssh_channel channel;
	struct termios terminal_local;
	
	int interactive = isatty(0);

	channel = ssh_channel_new(session);
	if (channel == NULL) return;

	if (interactive) {
		tcgetattr(STDIN_FILENO, &terminal_local);
		memcpy(&terminal, &terminal_local, sizeof(struct termios));
	}

	if (ssh_channel_open_session(channel)) {
		printf("Error opening channel: %s\n", ssh_get_error(session));
		ssh_channel_free(channel);
		return;
	}
	
	if (interactive) {
		ssh_channel_request_pty(channel);
		sizechanged(channel);
	}

	if (ssh_channel_request_shell(channel)) {
		printf("Requesting shell: %s\n", ssh_get_error(session));
		ssh_channel_free(channel);
		return;
	}

	if (interactive) {
		cfmakeraw(&terminal_local);
		tcsetattr(STDIN_FILENO, TCSANOW, &terminal_local);
		setsignal();
	}
	
	signal(SIGTERM, do_cleanup);
	select_loop(session, channel);
	
	if (interactive) do_cleanup(0);
	
	ssh_channel_free(channel);
	return;
}

static int
client(ssh_session session)
{
	int state;

	int auth = 0;
	char *banner = NULL;

	if (user) {
		if (ssh_options_set(session, SSH_OPTIONS_USER, user) < 0) {
			return -1;
		}
	}
	
	if (ssh_options_set(session, SSH_OPTIONS_HOST, host) < 0) {
		return -1;
	}

	if (ssh_connect(session)) {
		fprintf(stderr, "Connection failed: %s\n", ssh_get_error(session));
		return -1;
	}

	state = verify_knownhost(session);
	if (state != 0) {
		return -1;
	}

	ssh_userauth_none(session, NULL);
	banner = ssh_get_issue_banner(session);
	if (banner) {
		printf("%s\n", banner);
		free(banner);
	}

	auth = authenticate_console(session);
	if (auth != SSH_AUTH_SUCCESS) {
		return -1;
	}
	
	shell(session);

	return 0;
}

static void
error(ssh_session session)
{
	fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
	return;
}

static int
authenticate_console(ssh_session session)
{
	int rc;
	int method;

	char password[128] = {0};

	/* Try to authenticate */
	rc = ssh_userauth_none(session, NULL);
	if (rc == SSH_AUTH_ERROR) {
		error(session);
		return rc;
	}

	method = ssh_userauth_list(session, NULL);
	
	while (rc != SSH_AUTH_SUCCESS) {
		/* Try to authenticate with password */
		if (ssh_getpass("\nPassword: ", password, sizeof(password), 1, 0) < 0) {
			return SSH_AUTH_ERROR;
		}

		if (method & SSH_AUTH_METHOD_PASSWORD) {
			rc = ssh_userauth_password(session, NULL, password);
			if (rc == SSH_AUTH_ERROR) {
				error(session);
				return rc;
			} else if (rc == SSH_AUTH_SUCCESS) {
				fprintf(stderr, "Authentication succeeded.\n\n");
				break;
			}
		}
		memset(password, 0, sizeof(password));
	}

	return rc;
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
    if (rc < 0) {
        return -1;
    }

    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA256,
                                &hash,
                                &hlen);
    
	ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }

    state = ssh_session_is_known_server(session);

    switch(state) {
    case SSH_KNOWN_HOSTS_CHANGED:
        fprintf(stderr, "Host key for the server has changed. The server's host key now is:\n");
        ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr, "\nFor security reasons, the connection will be stopped.\n");
        return -1;

    case SSH_KNOWN_HOSTS_OTHER:
        fprintf(stderr, "The host key for this server was not found but an other type of key exists.\n");
        fprintf(stderr, "An attacker might change the default server key to confuse your client\n");
        fprintf(stderr, "into thinking the key does not exist.\n");
        fprintf(stderr, "We advise you to rerun the client with -d or -r for more safety.\n");
        return -1;

    case SSH_KNOWN_HOSTS_NOT_FOUND:
        fprintf(stderr, "Could not find a known host file. If you accept the host key here,\n");
        fprintf(stderr, "the file will be automatically created.\n");
        /* fall through */

    case SSH_SERVER_NOT_KNOWN:
        fprintf(stderr, "This server is currently unknown.\n");
        ssh_print_hash(SSH_PUBLICKEY_HASH_SHA256, hash, hlen);
		fprintf(stderr, "\nDo you trust the host key (yes/no)? ");

        if (fgets(buf, sizeof(buf), stdin) == NULL) {
            ssh_clean_pubkey_hash(&hash);
            return -1;
        }
        
		if (strncasecmp(buf, "yes", 3) != 0) {
            ssh_clean_pubkey_hash(&hash);
            return -1;
        }
        break;

    case SSH_KNOWN_HOSTS_ERROR:
        ssh_clean_pubkey_hash(&hash);
        fprintf(stderr, "%s", ssh_get_error(session));
        return -1;

    case SSH_KNOWN_HOSTS_OK:
        break; /* ok */
    }

    ssh_clean_pubkey_hash(&hash);
    return 0;
}
