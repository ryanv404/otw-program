#ifndef _MY_VALIDATE_H
#define _MY_VALIDATE_H

#include <libssh/libssh.h>
#include "lssh/config.h"

int is_valid_level(level_t *level, level_t **leveldata);
int verify_knownhost(ssh_session session);
int authenticate_password(ssh_session session, level_t *level);
int check_for_known_host(const char *fingerprint, level_t *level);

#endif /* _MY_VALIDATE_H */
