/* term.c - Sets up shell on remote pseudoterminal */

#include "project/term.h"

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <libssh/libssh.h>

#include "project/termios_flags.h"

void print_ssh_session(ssh_session ses);
void print_ssh_channel(ssh_channel chan);
void print_ssh_socket(ssh_session ses);

int
interactive_shell_session(ssh_session session)
{
	int nbytes, nwritten, rc;
	struct winsize winsz;
	ssh_channel channel;
	struct termios old_flags, new_flags;

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

	/* Get the current window size */
	if (ioctl(0, TIOCGWINSZ, &winsz) == -1) {
		/* If unable to get current window size; use defaults */
		winsz.ws_col = 80;
		winsz.ws_row = 24;
	}

	/* Request remote xterm-256color pty with current window size dimensions */
	rc = ssh_channel_request_pty_size(channel, "xterm-256color", winsz.ws_col, winsz.ws_row);
	if (rc != SSH_OK) {
		goto shutdown;
	}

	/* Open shell on remote pty */
	if ((rc = ssh_channel_request_shell(channel)) != SSH_OK) {
		goto shutdown;
	}

	print_ssh_session(ses);
	print_ssh_channel(chan);
	print_ssh_socket(ses);

	/* Set local stdin to raw so that input is directly passed to remote pty */
	tcgetattr(STDIN_FILENO, &old_flags);
	new_flags = old_flags;
	cfmakeraw(&new_flags);
	tcsetattr(STDIN_FILENO, TCSANOW, &new_flags);

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
			nbytes = ssh_channel_read(channel, outbuffer, sizeof(outbuffer), 0);
			if (nbytes < 0) {
				rc = SSH_ERROR;
				goto shutdown;

			}

			if (nbytes > 0) {
				nwritten = write(1, outbuffer, nbytes);
				if (nwritten != nbytes) {
					rc = SSH_ERROR;
					goto shutdown;
				}
			}
		}

		if (FD_ISSET(0, &fds)) {
			nbytes = read(0, inbuffer, sizeof(inbuffer));
			if (nbytes < 0) {
				rc = SSH_ERROR;
				goto shutdown;
			}

			if (nbytes > 0) {
				nwritten = ssh_channel_write(channel, inbuffer, nbytes);
				if (nbytes != nwritten) {
					rc = SSH_ERROR;
					goto shutdown;
				}
			}
		}
	}

shutdown:

	if (channel && ssh_channel_is_open(channel)) {
		ssh_channel_close(channel);
	}

	if (channel) {
		ssh_channel_free(channel);
	}

	/* Restore local stdin to original settings */
	tcsetattr(STDIN_FILENO, TCSANOW, &old_flags);
	return rc;
}

int
disable_echo(struct termios *flags)
{
	int retval;

	struct termios *new_flags = (struct termios *) calloc(1, sizeof(struct termios));

	/* Get the current terminal settings */
	tcgetattr(fileno(stdin), flags);
	memcpy(new_flags, flags, sizeof(struct termios));

	/* Turn off local echo, but pass the newlines through */
	new_flags->c_lflag &= ~ECHO;
	new_flags->c_lflag |=  ECHONL;

	retval = tcsetattr(fileno(stdin), TCSAFLUSH, new_flags);
	if (retval != 0) {
		free(new_flags);
		fprintf(stderr, "Failed to set attributes\n");
		return -1;
	}

	tcgetattr(fileno(stdin), new_flags);
	if (new_flags->c_lflag & ECHO) {
		free(new_flags);
		fprintf(stderr, "Failed to turn off ECHO\n");
		return -1;
	}

	if (!(new_flags->c_lflag & ECHONL)) {
		free(new_flags);
		fprintf(stderr, "Failed to turn on ECHONL\n");
		return -1;
	}

	free(new_flags);
	return 0;
}

void
print_ssh_session(ssh_session ses)
{

	printf("ssh session:\n");


	return;
}

void
print_ssh_channel(ssh_channel chan)
{

	printf("ssh channel:\n");


	return;
}

void
print_ssh_socket(ssh_session ses)
{

	printf("ssh socket:\n");


	return;
}
