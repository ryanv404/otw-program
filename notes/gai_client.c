/* Client program */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

#define BUF_SIZE 500

int
main(int argc, char *argv[])
{
	/* Usage: ./gai_client host port msg... */
	
	int					sfd, s, j;
	char				buf[BUF_SIZE];
	size_t				len;
	ssize_t				nread;
	struct addrinfo 	hints;
	struct addrinfo    *result, *rp;


	/* Obtain address(es) matching host/port */
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family 	= AF_UNSPEC; 	/* Allow IPv4 or IPv6 */
	hints.ai_socktype 	= SOCK_DGRAM; 	/* Datagram socket */
	hints.ai_flags		= 0;
	hints.ai_protocol 	= 0; 			/* Any protocol */

	s = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (s != 0) {
		fprintf(stderr, "[-] getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}

	for (rp = result; rp != NULL; rp = rp->ai_next) {
		sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if (sfd == -1) {
			continue;
		}
		
		if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
			break; /* Success */
		}

		close(sfd);
	}

	if (rp == NULL) {
		fprintf(stderr, "[-] Could not connect.\n");
		exit(EXIT_FAILURE);
	}

	freeaddrinfo(result);

	/* Send remaining command-line arguments as separate
	datagrams, and read responses from server */
	for (j = 3; j < argc; j++) {
		len = strlen(argv[j]) + 1; /* +1 for terminating null byte */

		if (len + 1 > BUF_SIZE) {
			fprintf(stderr, "[-] Ignoring long message in argument %d.\n", j);
			continue;
		}

		if (write(sfd, argv[j], len) != len) {
			fprintf(stderr, "[-] Partial/failed write.\n");
			exit(EXIT_FAILURE);
		}

		nread = read(sfd, buf, BUF_SIZE);
		if (nread == -1) {
			perror("read");
			exit(EXIT_FAILURE);
		}

		printf("Received %ld bytes: %s\n", (long) nread, buf);
	}

	exit(EXIT_SUCCESS);
}