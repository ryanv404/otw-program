/* ssh_connect - OTW program */

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <libssh/libssh.h>

 #include "project/typedefs.h"
#include "project/utils.h"
#include "project/validate.h"
#include "project/datautils.h"

#define MAX_ADDR_WIDTH	60
#define KNOWNHOSTS		"data/known_hosts.dat"

int show_remote_processes(ssh_session session);
int verify_knownhost(ssh_session session);
int authenticate_password(ssh_session session);
int test_several_auth_methods(ssh_session session);
int display_banner(ssh_session session);

int main()
{
  ssh_session my_ssh_session;
  int rc;
  char *password;
   int verbosity = SSH_LOG_PROTOCOL;
  int port = 22;
  char *user;

  // Open session and set options
  my_ssh_session = ssh_new();
  if (my_ssh_session == NULL)
    exit(-1);

   ssh_options_set(my_ssh_session, SSH_OPTIONS_HOST, "hostname");
  ssh_options_set(my_ssh_session, SSH_OPTIONS_PORT, port);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_USER, user);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_LOG_VERBOSITY, &verbosity);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_SSH_DIR, NULL);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_KNOWNHOSTS, NULL);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_GLOBAL_KNOWNHOSTS, NULL);
  ssh_options_set(my_ssh_session, SSH_OPTIONS_PROCESS_CONFIG, NULL);

  // Connect to server
  rc = ssh_connect(my_ssh_session);
  if (rc != SSH_OK)
  {
    fprintf(stderr, "Error connecting to localhost: %s\n",
            ssh_get_error(my_ssh_session));
    ssh_free(my_ssh_session);
    exit(-1);
  }
 
  // Verify the server's identity
  // For the source code of verify_knownhost(), check previous example
  if (verify_knownhost(my_ssh_session) < 0)
  {
    ssh_disconnect(my_ssh_session);
    ssh_free(my_ssh_session);
    exit(-1);
  }
 
  // Authenticate ourselves
  authenticate_password(my_ssh_session);
  

  ssh_disconnect(my_ssh_session);
  ssh_free(my_ssh_session);
}

int
connect_to_level(level_t *level, level_t **all_levels)
{
	free(level);
	free_levels(all_levels);
	return 0;
}


// can use raw mode with cfmakeraw(struct termios *termios_p)
// for better terminal emulation.

int shell_session(ssh_session session)
{
  ssh_channel channel;
  int rc;
 
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;
 
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
 
//   ...
 
  ssh_channel_close(channel);
  ssh_channel_send_eof(channel);
  ssh_channel_free(channel);
 
  return SSH_OK;
}

int interactive_shell_session(ssh_session session, ssh_channel channel)
{
  /* Session and terminal initialization skipped */
 
  char buffer[256];
  int nbytes, nwritten;
  int rc;
 
  rc = ssh_channel_request_pty(channel);
  if (rc != SSH_OK) return rc;
 
  rc = ssh_channel_change_pty_size(channel, 80, 24);
  if (rc != SSH_OK) return rc;
 
  rc = ssh_channel_request_shell(channel);
  if (rc != SSH_OK) return rc;

  while (ssh_channel_is_open(channel) &&
         !ssh_channel_is_eof(channel))
  {
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
 
    if (out_channels[0] != NULL)
    {
      nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
      if (nbytes < 0) return SSH_ERROR;
      if (nbytes > 0)
      {
        nwritten = write(1, buffer, nbytes);
        if (nwritten != nbytes) return SSH_ERROR;
      }
    }
 
    if (FD_ISSET(0, &fds))
    {
      nbytes = read(0, buffer, sizeof(buffer));
      if (nbytes < 0) return SSH_ERROR;
      if (nbytes > 0)
      {
        nwritten = ssh_channel_write(channel, buffer, nbytes);
        if (nbytes != nwritten) return SSH_ERROR;
      }
    }
  }
 
  return rc;
}

int display_banner(ssh_session session)
{
  int rc;
  char *banner;
 
/*
   * Does not work without calling ssh_userauth_none() first ***
   * That will be fixed ***
*/
  rc = ssh_userauth_none(session, NULL);
  if (rc == SSH_AUTH_ERROR)
    return rc;
 
  banner = ssh_get_issue_banner(session);
  if (banner)
  {
    printf("%s\n", banner);
    free(banner);
  }
 
  return rc;
}

int test_several_auth_methods(ssh_session session)
{
  int method, rc;
 
  rc = ssh_userauth_none(session, NULL);
  if (rc == SSH_AUTH_SUCCESS || rc == SSH_AUTH_ERROR) {
      return rc;
  }
 
  method = ssh_userauth_list(session, NULL);
 
  if (method & SSH_AUTH_METHOD_NONE)
  { // For the source code of function authenticate_none(),
    // refer to the corresponding example
    rc = authenticate_none(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_PUBLICKEY)
  { // For the source code of function authenticate_pubkey(),
    // refer to the corresponding example
    rc = authenticate_pubkey(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_INTERACTIVE)
  { // For the source code of function authenticate_kbdint(),
    // refer to the corresponding example
    rc = authenticate_kbdint(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  if (method & SSH_AUTH_METHOD_PASSWORD)
  { // For the source code of function authenticate_password(),
    // refer to the corresponding example
    rc = authenticate_password(session);
    if (rc == SSH_AUTH_SUCCESS) return rc;
  }
  return SSH_AUTH_ERROR;
}

int authenticate_password(ssh_session session)
{
  char *password;
  int rc;
 
  password = getpass("Enter your password: ");
  rc = ssh_userauth_password(session, NULL, password);
  if (rc == SSH_AUTH_ERROR)
  {
     fprintf(stderr, "Authentication failed: %s\n",
       ssh_get_error(session));
     return SSH_AUTH_ERROR;
  }
 
  return rc;
}

int verify_knownhost(ssh_session session)
{
    enum ssh_known_hosts_e state;
    unsigned char *hash = NULL;
    ssh_key srv_pubkey = NULL;
    size_t hlen;
    char buf[10];
    char *hexa;
    char *p;
    int cmp;
    int rc;
 
    rc = ssh_get_server_publickey(session, &srv_pubkey);
    if (rc < 0) {
        return -1;
    }
 
    rc = ssh_get_publickey_hash(srv_pubkey,
                                SSH_PUBLICKEY_HASH_SHA1,
                                &hash,
                                &hlen);
    ssh_key_free(srv_pubkey);
    if (rc < 0) {
        return -1;
    }
 
    state = ssh_session_is_known_server(session);
    switch (state) {
        case SSH_KNOWN_HOSTS_OK:
            /* OK */
 
            break;
        case SSH_KNOWN_HOSTS_CHANGED:
            fprintf(stderr, "Host key for server changed: it is now:\n");
            ssh_print_hexa("Public key hash", hash, hlen);
            fprintf(stderr, "For security reasons, connection will be stopped\n");
            ssh_clean_pubkey_hash(&hash);
 
            return -1;
        case SSH_KNOWN_HOSTS_OTHER:
            fprintf(stderr, "The host key for this server was not found but an other"
                    "type of key exists.\n");
            fprintf(stderr, "An attacker might change the default server key to"
                    "confuse your client into thinking the key does not exist\n");
            ssh_clean_pubkey_hash(&hash);
 
            return -1;
        case SSH_KNOWN_HOSTS_NOT_FOUND:
            fprintf(stderr, "Could not find known host file.\n");
            fprintf(stderr, "If you accept the host key here, the file will be"
                    "automatically created.\n");
 
            /* FALL THROUGH to SSH_SERVER_NOT_KNOWN behavior */
 
        case SSH_KNOWN_HOSTS_UNKNOWN:
            hexa = ssh_get_hexa(hash, hlen);
            fprintf(stderr,"The server is unknown. Do you trust the host key?\n");
            fprintf(stderr, "Public key hash: %s\n", hexa);
            ssh_string_free_char(hexa);
            ssh_clean_pubkey_hash(&hash);
            p = fgets(buf, sizeof(buf), stdin);
            if (p == NULL) {
                return -1;
            }
 
            cmp = strncasecmp(buf, "yes", 3);
            if (cmp != 0) {
                return -1;
            }
 
            rc = ssh_session_update_known_hosts(session);
            if (rc < 0) {
                fprintf(stderr, "Error %s\n", strerror(errno));
                return -1;
            }
 
            break;
        case SSH_KNOWN_HOSTS_ERROR:
            fprintf(stderr, "Error %s", ssh_get_error(session));
            ssh_clean_pubkey_hash(&hash);
            return -1;
    }
 
    ssh_clean_pubkey_hash(&hash);
    return 0;
}

int show_remote_processes(ssh_session session)
{
  ssh_channel channel;
  int rc;
  char buffer[256];
  int nbytes;
 
  channel = ssh_channel_new(session);
  if (channel == NULL)
    return SSH_ERROR;
 
  rc = ssh_channel_open_session(channel);
  if (rc != SSH_OK)
  {
    ssh_channel_free(channel);
    return rc;
  }
 
  rc = ssh_channel_request_exec(channel, "ps aux");
  if (rc != SSH_OK)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return rc;
  }
 
  nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  while (nbytes > 0)
  {
    if (write(1, buffer, nbytes) != (unsigned int) nbytes)
    {
      ssh_channel_close(channel);
      ssh_channel_free(channel);
      return SSH_ERROR;
    }
    nbytes = ssh_channel_read(channel, buffer, sizeof(buffer), 0);
  }
 
  if (nbytes < 0)
  {
    ssh_channel_close(channel);
    ssh_channel_free(channel);
    return SSH_ERROR;
  }
 
  ssh_channel_send_eof(channel);
  ssh_channel_close(channel);
  ssh_channel_free(channel);
 
  return SSH_OK;
}