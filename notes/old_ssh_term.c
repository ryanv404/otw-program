/* term.c - Sets up shell on remote pseudoterminal */

#include "project/term.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <libssh/libssh.h>

void print_session_struct(struct ssh_session_struct *ses);
void print_channel_struct(struct ssh_channel_struct *chan);
void print_socket_struct(struct ssh_socket_struct *sock);
void print_buffer_struct(struct ssh_buffer_struct *buf);

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

	if (isatty(ssh_get_fd(session))) {
		printf("socket fd is connected to a tty.\n");
	} else {
		printf("socket fd is NOT connected to a tty.\n");
	}

	print_session_struct(session);
	print_channel_struct(channel);
	print_socket_struct(session->socket);

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

void print_session_struct(struct ssh_session_struct *ses)
{
	// ssh_session_struct:
	// struct ssh_session_struct {
	// 	struct ssh_common_struct common;
	// 	struct ssh_socket_struct *socket;
	// 	char *serverbanner;
	// 	char *clientbanner;
	// 	int protoversion;
	// 	int server;
	// 	int client;
	// 	int openssh;
	// 	uint32_t send_seq;
	// 	uint32_t recv_seq;
	// 	struct ssh_timestamp last_rekey_time;
	
	// 	int connected;
	// 	/* !=0 when the user got a session handle */
	// 	int alive;
	// 	/* two previous are deprecated */
	// 	/* int auth_service_asked; */
	
	// 	/* session flags (SSH_SESSION_FLAG_*) */
	// 	int flags;
	
	// 	/* Extensions negotiated using RFC 8308 */
	// 	uint32_t extensions;
	
	// 	ssh_string banner; /* that's the issue banner from
	// 						the server */
	// 	char *discon_msg; /* disconnect message from
	// 						the remote host */
	// 	ssh_buffer in_buffer;
	// 	PACKET in_packet;
	// 	ssh_buffer out_buffer;
	// 	struct ssh_list *out_queue; /* This list is used for delaying packets
	// 									when rekeying is required */
	
	// 	/* the states are used by the nonblocking stuff to remember */
	// 	/* where it was before being interrupted */
	// 	enum ssh_pending_call_e pending_call_state;
	// 	enum ssh_session_state_e session_state;
	// 	enum ssh_packet_state_e packet_state;
	// 	enum ssh_dh_state_e dh_handshake_state;
	// 	enum ssh_channel_request_state_e global_req_state;
	// 	struct ssh_agent_state_struct *agent_state;
	
	// 	struct {
	// 		struct ssh_auth_auto_state_struct *auto_state;
	// 		enum ssh_auth_service_state_e service_state;
	// 		enum ssh_auth_state_e state;
	// 		uint32_t supported_methods;
	// 		uint32_t current_method;
	// 	} auth;
	
	// 	/*
	// 	* RFC 4253, 7.1: if the first_kex_packet_follows flag was set in
	// 	* the received SSH_MSG_KEXINIT, but the guess was wrong, this
	// 	* field will be set such that the following guessed packet will
	// 	* be ignored.  Once that packet has been received and ignored,
	// 	* this field is cleared.
	// 	*/
	// 	int first_kex_follows_guess_wrong;
	
	// 	ssh_buffer in_hashbuf;
	// 	ssh_buffer out_hashbuf;
	// 	struct ssh_crypto_struct *current_crypto;
	// 	struct ssh_crypto_struct *next_crypto;  /* next_crypto is going to be used after a SSH2_MSG_NEWKEYS */
	
	// 	struct ssh_list *channels; /* linked list of channels */
	// 	int maxchannel;
	// 	ssh_agent agent; /* ssh agent */
	
	// /* keyb interactive data */
	// 	struct ssh_kbdint_struct *kbdint;
	// 	struct ssh_gssapi_struct *gssapi;
	
	// 	/* server host keys */
	// 	struct {
	// 		ssh_key rsa_key;
	// 		ssh_key dsa_key;
	// 		ssh_key ecdsa_key;
	// 		ssh_key ed25519_key;
	// 		/* The type of host key wanted by client */
	// 		enum ssh_keytypes_e hostkey;
	// 		enum ssh_digest_e hostkey_digest;
	// 	} srv;
	
	// 	/* auths accepted by server */
	// 	struct ssh_list *ssh_message_list; /* list of delayed SSH messages */
	// 	int (*ssh_message_callback)( struct ssh_session_struct *session, ssh_message msg, void *userdata);
	// 	void *ssh_message_callback_data;
	// 	ssh_server_callbacks server_callbacks;
	// 	void (*ssh_connection_callback)( struct ssh_session_struct *session);
	// 	struct ssh_packet_callbacks_struct default_packet_callbacks;
	// 	struct ssh_list *packet_callbacks;
	// 	struct ssh_socket_callbacks_struct socket_callbacks;
	// 	ssh_poll_ctx default_poll_ctx;
	// 	/* options */
	// #ifdef WITH_PCAP
	// 	ssh_pcap_context pcap_ctx; /* pcap debugging context */
	// #endif
	// 	struct {
	// 		struct ssh_list *identity;
	// 		char *username;
	// 		char *host;
	// 		char *bindaddr; /* bind the client to an ip addr */
	// 		char *sshdir;
	// 		char *knownhosts;
	// 		char *global_knownhosts;
	// 		char *wanted_methods[SSH_KEX_METHODS];
	// 		char *pubkey_accepted_types;
	// 		char *ProxyCommand;
	// 		char *custombanner;
	// 		unsigned long timeout; /* seconds */
	// 		unsigned long timeout_usec;
	// 		unsigned int port;
	// 		socket_t fd;
	// 		int StrictHostKeyChecking;
	// 		char compressionlevel;
	// 		char *gss_server_identity;
	// 		char *gss_client_identity;
	// 		int gss_delegate_creds;
	// 		int flags;
	// 		int nodelay;
	// 		bool config_processed;
	// 		uint8_t options_seen[SOC_MAX];
	// 		uint64_t rekey_data;
	// 		uint32_t rekey_time;
	// 	} opts;
	// 	/* counters */
	// 	ssh_counter socket_counter;
	// 	ssh_counter raw_counter;
	// };

	// 	enum ssh_session_state_e {
	// 		SSH_SESSION_STATE_NONE=0,
	// 		SSH_SESSION_STATE_CONNECTING,
	// 		SSH_SESSION_STATE_SOCKET_CONNECTED,
	// 		SSH_SESSION_STATE_BANNER_RECEIVED,
	// 		SSH_SESSION_STATE_INITIAL_KEX,
	// 		SSH_SESSION_STATE_KEXINIT_RECEIVED,
	// 		SSH_SESSION_STATE_DH,
	// 		SSH_SESSION_STATE_AUTHENTICATING,
	// 		SSH_SESSION_STATE_AUTHENTICATED,
	// 		SSH_SESSION_STATE_ERROR,
	// 		SSH_SESSION_STATE_DISCONNECTED
	// };

	// struct ssh_common_struct:
	// 		struct error_struct error;
	// 		ssh_callbacks callbacks;
	// 		int log_verbosity;

	// struct error_struct:
	// 		int error_code
	// 		char error_buffer [ERROR_BUFFERLEN]

	return;
}

void
print_socket_struct(struct ssh_socket_struct *sock)
{

	// ssh_socket_struct:
	// 		socket_t fd
	// 		int fd_is_socket
	// 		int last_errno
	// 		int read_wontblock
	// 		int write_wontblock
	// 		int data_except
	// 		enum ssh_socket_states_e state
	// 		ssh_buffer out_buffer
	// 		ssh_buffer in_buffer
	// 		ssh_session	session
	// 		ssh_socket_callbacks callbacks
	// 		ssh_poll_handle poll_handle
	// 		pid_t proxy_pid


	printf("[*] struct ssh_channel:\n");
	printf("socket_t fd = %d\n", session->socket->);
	printf("int fd_is_socket = %d\n", channel->fd_is_socket);
	printf("int last_errno = %d\n", channel->last_errno);
	printf("int read_wontblock = %d\n", channel->fd);
	printf("int write_wontblock = %d\n", channel->fd);
	printf("int data_except = %d\n", channel->fd);
	printf("enum ssh_socket_states_e state = %d\n", channel->fd);
	printf("ssh_buffer out_buffer = %d\n", channel->fd);
	printf("ssh_buffer in_buffer = %d\n", channel->fd);
	printf("ssh_session	session = %d\n", channel->fd);
	printf("ssh_socket_callbacks callbacks = %d\n", channel->fd);
	printf("ssh_poll_handle poll_handle = %d\n", channel->fd);
	printf("pid_t proxy_pid = %d\n\n", channel->fd);


	return;
}

void
print_channel_struct(struct ssh_channel_struct *chan)
{

	// struct ssh_channel_struct:
	// ssh_session session; /* SSH_SESSION pointer */
	// uint32_t local_channel;
	// uint32_t local_window;
	// int local_eof;
	// uint32_t local_maxpacket;
	// uint32_t remote_channel;
	// uint32_t remote_window;
	// int remote_eof; /* end of file received */
	// uint32_t remote_maxpacket;
	// enum ssh_channel_state_e state;
	// int delayed_close;
	// int flags;
	// ssh_buffer stdout_buffer;
	// ssh_buffer stderr_buffer;
	// void *userarg;
	// int exit_status;
	// enum ssh_channel_request_state_e request_state;
	// struct ssh_list *callbacks; /* list of ssh_channel_callbacks */
	// /* counters */
	// ssh_counter counter;

	// enum ssh_channel_type_e:
	//      SSH_CHANNEL_UNKNOWN=0,
	//      SSH_CHANNEL_SESSION,
	//      SSH_CHANNEL_DIRECT_TCPIP,
	//      SSH_CHANNEL_FORWARDED_TCPIP,
	//      SSH_CHANNEL_X11,
	//      SSH_CHANNEL_AUTH_AGENT

	return;
}

void
print_buffer_struct(struct ssh_buffer_struct *buf)
{
	// struct ssh_buffer_struct:
	// 		bool 	 secure
	// 		size_t 	 used
	// 		size_t 	 allocated
	// 		size_t 	 pos
	// 		uint8_t *data

	return;
}