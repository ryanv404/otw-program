#ifndef _MY_SSH_H
#define _MY_SSH_H

#include "lssh2/config.h"

int libssh2_connect(level_t *level, level_t **all_levels);

#endif /* _MY_SSH_H */
