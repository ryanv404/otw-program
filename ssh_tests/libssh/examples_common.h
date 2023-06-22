#ifndef EXAMPLES_COMMON_H_
#define EXAMPLES_COMMON_H_

#include <libssh/libssh.h>

/** Zero a structure */
#define ZERO_STRUCT(x) memset((char *) &(x), 0, sizeof(x))

int authenticate_console(ssh_session session);
int verify_knownhost(ssh_session session);

#endif /* EXAMPLES_COMMON_H_ */
