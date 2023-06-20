/* term.c - Sets up shell on remote pseudoterminal */

#include "project/term.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <libssh/libssh.h>

int
interactive_shell_session(ssh_session session)
{
	fd_set fds;
	int nbytes, nwritten, maxfd, rc;
	struct winsize winsz;
	struct timeval timeout;
	ssh_channel channel;
	ssh_channel in_channels[2], out_channels[2];

	char inbuffer[MAX_BUFSIZE]  = {0};
	char outbuffer[MAX_BUFSIZE] = {0};
	// struct termios *old_flags = NULL;

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

	/* Set timeout */
	timeout.tv_sec = 30;
	timeout.tv_usec = 0;

	/* Set up terminal */
	// old_flags = (struct termios *) calloc(1, sizeof(struct termios));

	//disable_echo(old_flags);

	while (ssh_channel_is_open(channel) && !ssh_channel_is_eof(channel)) {
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
				// tcsetattr(fileno(stdin), TCSANOW, old_flags);
				goto shutdown;

			}

			if (nbytes > 0) {
				nwritten = write(1, outbuffer, nbytes);
				if (nwritten != nbytes) {
					rc = SSH_ERROR;
					// tcsetattr(fileno(stdin), TCSANOW, old_flags);
					goto shutdown;
				}
			}
		}

		if (FD_ISSET(0, &fds)) {
			nbytes = read(0, inbuffer, sizeof(inbuffer));
			if (nbytes < 0) {
				rc = SSH_ERROR;
				// tcsetattr(fileno(stdin), TCSANOW, old_flags);
				goto shutdown;
			}

			if (nbytes > 0) {
				nwritten = ssh_channel_write(channel, inbuffer, nbytes);
				if (nbytes != nwritten) {
					rc = SSH_ERROR;
					// tcsetattr(fileno(stdin), TCSANOW, old_flags);
					goto shutdown;
				}
			}
		}
	}

	/* Restore original terminal settings */
	// tcsetattr(fileno(stdin), TCSANOW, old_flags);

shutdown:

	if (channel && ssh_channel_is_open(channel)) {
		ssh_channel_close(channel);
	}

	if (channel) {
		ssh_channel_free(channel);
	}

	// free(old_flags);
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
