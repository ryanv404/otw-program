#include "ssh_connect.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <libssh2.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "typedefs.h"

static const char *pubkey  = ".ssh/id_ed25519.pub";
static const char *privkey = ".ssh/id_ed25519";
static const char *knownhosts = ".ssh/known_hosts";

int
ssh_connect(level_t *level)
{
    int i, rc;
    uint32_t hostaddr;
    char *userauthlist;
    const char *fingerprint;
    struct sockaddr_in sin;
    LIBSSH2_CHANNEL *channel;
    libssh2_socket_t sock;

    int auth_pw = 0;
    LIBSSH2_SESSION *session = NULL;

    hostaddr = inet_addr(host);

    rc = libssh2_init(0);
    if (rc) quit("libssh2 initialization failed");

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == LIBSSH2_INVALID_SOCKET) {
        fprintf(stderr, "failed to create socket.\n");
        rc = 1;
        goto shutdown;
    }

    sin.sin_family = AF_INET;
    sin.sin_port = htons(port);
    sin.sin_addr.s_addr = hostaddr;

    fprintf(stderr, "Connecting to %s:%d as user %s\n",
            inet_ntoa(sin.sin_addr), ntohs(sin.sin_port), username);

    if (connect(sock, (struct sockaddr *)(&sin), sizeof(struct sockaddr_in))) {
        fprintf(stderr, "failed to connect.\n");
        goto shutdown;
    }

    /* Create a session instance and start it up. This will trade welcome
     * banners, exchange keys, and setup crypto, compression, and MAC layers
     */

    session = libssh2_session_init();
    if (!session) {
        fprintf(stderr, "Could not initialize SSH session.\n");
        goto shutdown;
    }

    rc = libssh2_session_handshake(session, sock);
    if (rc) {
        fprintf(stderr, "Failure establishing SSH session: %d\n", rc);
        goto shutdown;
    }
    rc = 1;

    /* At this point we have not yet authenticated.  The first thing to do
     * is check the hostkey's fingerprint against our known hosts Your app
     * may have it hard coded, may go to a file, may present it to the
     * user, that's your call
     */

    fingerprint = libssh2_hostkey_hash(session, LIBSSH2_HOSTKEY_HASH_SHA1);
    fprintf(stderr, "Fingerprint: ");
    for (i = 0; i < 20; i++) {
        fprintf(stderr, "%02X ", (unsigned char) fingerprint[i]);
    }
    fprintf(stderr, "\n");

    /* check what authentication methods are available */
    userauthlist = libssh2_userauth_list(session, username,
                                         (unsigned int) strlen(username));
    if (userauthlist) {
        fprintf(stderr, "Authentication methods: %s\n", userauthlist);
        if (strstr(userauthlist, "password")) {
            auth_pw |= 1;
        }
        if (strstr(userauthlist, "keyboard-interactive")) {
            auth_pw |= 2;
        }
        if (strstr(userauthlist, "publickey")) {
            auth_pw |= 4;
        }

        /* check for options */
        if (argc > 4) {
            if ((auth_pw & 1) && !strcmp(argv[4], "-p")) {
                auth_pw = 1;
            }
            if ((auth_pw & 2) && !strcmp(argv[4], "-i")) {
                auth_pw = 2;
            }
            if ((auth_pw & 4) && !strcmp(argv[4], "-k")) {
                auth_pw = 4;
            }
        }

        if (auth_pw & 1) {
            /* We could authenticate via password */
            if (libssh2_userauth_password(session, username, password)) {
                fprintf(stderr, "Authentication by password failed.\n");
                goto shutdown;
            } else {
                fprintf(stderr, "Authentication by password succeeded.\n");
            }
        } else if(auth_pw & 2) {
            /* Or via keyboard-interactive */
            if(libssh2_userauth_keyboard_interactive(session, username,
                                                     &kbd_callback) ) {
                fprintf(stderr,
                        "Authentication by keyboard-interactive failed.\n");
                goto shutdown;
            } else {
                fprintf(stderr,
                        "Authentication by keyboard-interactive succeeded.\n");
            }
        } else if(auth_pw & 4) {
            /* Or by public key */
            size_t fn1sz, fn2sz;
            char *fn1, *fn2;
            char const *h = getenv("HOME");

            if (!h || !*h) h = ".";
            fn1sz = strlen(h) + strlen(pubkey) + 2;
            fn2sz = strlen(h) + strlen(privkey) + 2;
            fn1 = malloc(fn1sz);
            fn2 = malloc(fn2sz);

            if (!fn1 || !fn2) {
                free(fn2);
                free(fn1);
                fprintf(stderr, "out of memory\n");
                goto shutdown;
            }

            snprintf(fn1, fn1sz, "%s/%s", h, pubkey);
            snprintf(fn2, fn2sz, "%s/%s", h, privkey);

            if (libssh2_userauth_publickey_fromfile(session, username,
                                                   fn1, fn2,
                                                   password)) {
                fprintf(stderr, "Authentication by public key failed.\n");
                free(fn2);
                free(fn1);
                goto shutdown;
            } else {
                fprintf(stderr, "Authentication by public key succeeded.\n");
            }
            free(fn2);
            free(fn1);
        } else {
            fprintf(stderr, "No supported authentication methods found.\n");
            goto shutdown;
        }
    }

    /* Request a session channel on which to run a shell */
    channel = libssh2_channel_open_session(session);
    if (!channel) {
        fprintf(stderr, "Unable to open a session\n");
        goto shutdown;
    }

    /* Request a terminal with 'vanilla' terminal emulation
     * See /etc/termcap for more options. This is useful when opening
     * an interactive shell.
     */

    #if 0
    if(libssh2_channel_request_pty(channel, "vanilla")) {
        fprintf(stderr, "Failed requesting pty\n");
    }
    #endif

        if(libssh2_channel_shell(channel)) {
            fprintf(stderr, "Unable to request shell on allocated pty\n");
            goto shutdown;
        }

        /* At this point the shell can be interacted with using
         * libssh2_channel_read()
         * libssh2_channel_read_stderr()
         * libssh2_channel_write()
         * libssh2_channel_write_stderr()
         *
         * Blocking mode may be (en|dis)abled with:
         *    libssh2_channel_set_blocking()
         * If the server send EOF, libssh2_channel_eof() will return non-0
         * To send EOF to the server use: libssh2_channel_send_eof()
         * A channel can be closed with: libssh2_channel_close()
         * A channel can be freed with: libssh2_channel_free()
         */

        /* Read and display all the data received on stdout (ignoring stderr)
         * until the channel closes. This will eventually block if the command
         * produces too much data on stderr; the loop must be rewritten to use
         * non-blocking mode and include interspersed calls to
         * libssh2_channel_read_stderr() to avoid this. See ssh2_echo.c for
         * an idea of how such a loop might look.
         */
        while (!libssh2_channel_eof(channel)) {
            char buf[1024];
            ssize_t err = libssh2_channel_read(channel, buf, sizeof(buf));
            if (err < 0) {
                fprintf(stderr, "Unable to read response: %d\n", (int)err);
            } else {
                fwrite(buf, 1, err, stdout);
            }
        }
    }

    rc = libssh2_channel_get_exit_status(channel);

    if (libssh2_channel_close(channel)) fprintf(stderr, "Unable to close channel\n");

    if (channel) {
        libssh2_channel_free(channel);
        channel = NULL;
    }

shutdown:
    if (session) {
        libssh2_session_disconnect(session, "Normal Shutdown");
        libssh2_session_free(session);
    }

    if (sock != LIBSSH2_INVALID_SOCKET) {
        shutdown(sock, 2);
        close(sock);
    }

    fprintf(stderr, "all done\n");
    libssh2_exit();
    return rc;
}
