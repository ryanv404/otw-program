#ifndef _MY_TERM_H
#define _MY_TERM_H

#include <libssh/libssh.h>

#define MAX_BUFSIZE		1024

int interactive_shell_session(ssh_session session);

#endif /* _MY_TERM_H */