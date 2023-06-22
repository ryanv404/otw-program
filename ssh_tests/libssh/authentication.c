#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libssh/libssh.h>

#include "examples_common.h"

static void
error(ssh_session session)
{
	fprintf(stderr, "Authentication failed: %s\n", ssh_get_error(session));
	return;
}

int
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
