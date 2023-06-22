#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <libssh/libssh.h>

#include "examples_common.h"

int
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
