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

#include <libssh/sftp.h>
#include <libssh/libssh.h>
#include <libssh/callbacks.h>

#include "examples_common.h"

#define MAXCMD 		10

static char *cmds[MAXCMD];
static struct termios terminal;
static ssh_pcap_file pcap;

static char *host = NULL;
static char *user = NULL;
static char *pcap_file = NULL;
static int signal_delayed = 0;
static char *proxycommand = NULL;

static void
usage(void)
{
	fprintf(stderr,
			"usage: ssh [options] [login@]hostname\n\n"
			"    -r          Use RSA to verify host public key.\n"
			"    -l user     Log in as user.\n"
			"    -p portnum  Connect to port `portnum`.\n"
			"    -P file     Create a pcap debugging file.\n"
			"    -T proxycommand\n"
			"                Command to execute as a socket proxy.\n\n"
			"sample SSH client - libssh-%s\n",
			ssh_version(0));

	exit(EXIT_SUCCESS);
}

static int
opts(int argc, char **argv)
{
	int i;

	while ((i = getopt(argc, argv, "T:P:")) != -1) {
		switch (i) {
		case 'P':
			pcap_file = optarg;
			break;
		case 'T':
			proxycommand = optarg;
			break;
		default:
			fprintf(stderr, "Unknown option %c\n", optopt);
			return -1;
		}
	}

	if (optind < argc) {
		host = argv[optind++];
	}

	if (host == NULL) {
		return -1;
	}

	return 0;
}

static void
do_cleanup(int i)
{
  /* unused variable */
  (void) i;

  tcsetattr(STDIN_FILENO, TCSANOW, &terminal);
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
}

static void
setsignal(void)
{
	signal(SIGWINCH, sigwindowchanged);
	signal_delayed = 0;
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
}

static void
select_loop(ssh_session session,ssh_channel channel)
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
}

static void
shell(ssh_session session)
{
	ssh_channel channel;
	struct termios terminal_local;
	
	int interactive = isatty(0);

	channel = ssh_channel_new(session);
	if (channel == NULL) {
		return;
	}

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
	
	if (interactive) {
		do_cleanup(0);
	}
	
	ssh_channel_free(channel);
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

	if (proxycommand != NULL) {
		if (ssh_options_set(session, SSH_OPTIONS_PROXYCOMMAND, proxycommand)) {
			return -1;
		}
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
set_pcap(ssh_session session)
{
	if (pcap_file == NULL) {
		return;
	}

	pcap = ssh_pcap_file_new();
	if (pcap == NULL) {
		return;
	}

	if (ssh_pcap_file_open(pcap, pcap_file) == SSH_ERROR) {
		printf("Error opening pcap file\n");
		ssh_pcap_file_free(pcap);
		pcap = NULL;
		return;
	}

	ssh_set_pcap_file(session, pcap);
}

static void
cleanup_pcap(void)
{
	if (pcap != NULL) {
		ssh_pcap_file_free(pcap);
	}

	pcap = NULL;
}

int
main(int argc, char **argv)
{
	ssh_session session;

	ssh_init();
	session = ssh_new();

	if (ssh_options_getopt(session, &argc, argv) || opts(argc, argv)) {
		fprintf(stderr, "Error parsing the command line: %s\n",
				ssh_get_error(session));
		ssh_free(session);
		ssh_finalize();
		usage();
	}

	signal(SIGTERM, do_exit);

	set_pcap(session);
	client(session);

	ssh_disconnect(session);
	ssh_free(session);

	cleanup_pcap();
	ssh_finalize();

	return EXIT_SUCCESS;
}
