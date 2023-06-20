#ifndef _MY_TERM_H
#define _MY_TERM_H

#include <libssh/libssh.h>
#include <termios.h>

#define MAX_BUFSIZE		1024

int interactive_shell_session(ssh_session session);
int disable_echo(struct termios *flags);

#endif /* _MY_TERM_H */