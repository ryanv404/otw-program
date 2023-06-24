#ifndef _MY_VALIDATE_H
#define _MY_VALIDATE_H

#include "lssh2/config.h"

int is_valid_level(level_t *level, level_t **leveldata);
int check_for_known_host(const char *fingerprint, level_t *level);

#endif /* _MY_VALIDATE_H */
