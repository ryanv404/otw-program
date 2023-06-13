#ifndef _MY_SSH_CONNECT_H
#define _MY_SSH_CONNECT_H

#include <stdint.h>

int ssh_connect(char *host, uint16_t port, char *username, char *password);

#endif /* _MY_SSH_CONNECT_H */
