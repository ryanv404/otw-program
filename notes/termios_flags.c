#include "project/termios_flags.h"

#define __USE_MISC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <termios.h>
#include <sys/ioctl.h>

void get_iflags(struct termios *tattrs);
void get_oflags(struct termios *tattrs);
void get_cflags(struct termios *tattrs);
void get_lflags(struct termios *tattrs);
void get_ctrlchars(struct termios *tattrs);
void get_shortflags(struct termios *in_tattrs);

int
get_termios_flags(int fd)
{
	struct termios tattrs;

	if (tcgetattr(fd, &tattrs) == -1) {
		perror("tcgetattr");
		exit(EXIT_FAILURE);
	}

	get_shortflags(&tattrs);
	return 0;
}

void
get_shortflags(struct termios *in_tattrs)
{
	printf("+++++++++++++++++++++++++\n");
	printf("+  TERMINAL ATTRIBUTES  +\n");
	printf("+++++++++++++++++++++++++\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[*]    input flags     [*]\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	get_iflags(in_tattrs);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[*]    output flags    [*]\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	get_oflags(in_tattrs);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[*]    local flags     [*]\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	get_lflags(in_tattrs);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[*]   control flags    [*]\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	get_cflags(in_tattrs);
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	printf("[*] control characters [*]\n");
	printf("~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
	get_ctrlchars(in_tattrs);
	return;
}

void
get_cflags(struct termios *tattrs)
{
	/* Get c_cflag bits that are set */
	printf("|");
	if (tattrs->c_cflag & CSTOPB) 	printf("CSTOPB|");
	if (tattrs->c_cflag & CREAD) 	printf("CREAD|");
	if (tattrs->c_cflag & PARENB) 	printf("PARENB|");
	if (tattrs->c_cflag & PARODD) 	printf("PARODD|");
	if (tattrs->c_cflag & HUPCL) 	printf("HUPCL|");
	if (tattrs->c_cflag & CLOCAL) 	printf("CLOCAL|");
	if (tattrs->c_cflag & CBAUD) 	printf("CBAUD|");
	if (tattrs->c_cflag & CBAUDEX) 	printf("CBAUDEX|");
	if (tattrs->c_cflag & CIBAUD) 	printf("CIBAUD|");
	if (tattrs->c_cflag & CMSPAR) 	printf("CMSPAR|");
	if (tattrs->c_cflag & CRTSCTS) 	printf("CRTSCTS|");
	if (tattrs->c_cflag & CSIZE) {
		if (tattrs->c_cflag & CS5) printf("CS5|");
		if (tattrs->c_cflag & CS6) printf("CS6|");
		if (tattrs->c_cflag & CS7) printf("CS7|");
		if (tattrs->c_cflag & CS8) printf("CS8|");
	}
	printf("\n");
	return;
}

void
get_ctrlchars(struct termios *tattrs)
{
	/* Get c_cc characters that are defined */
	printf("|");
	for (int i = 0; i < 17; i++) {
		if (tattrs->c_cc[i]) {
			switch (i) {
			case VINTR: 
				printf("VINTR|");
				break;
			case VQUIT:
				printf("VQUIT|");
				break;
			case VERASE:
				printf("VERASE|");
				break;
			case VKILL:
				printf("VKILL|");
				break;
			case VEOF:
				printf("VEOF|");
				break;
			case VTIME:
				printf("VTIME|");
				break;
			case VMIN:
				printf("VMIN|");
				break;
			case VSTART:
				printf("VSTART|");
				break;
			case VSTOP:
				printf("VSTOP|");
				break;
			case VSUSP:
				printf("VSUSP|");
				break;
			case VEOL:
				printf("VEOL|");
				break;
			case VREPRINT:
				printf("VREPRINT|");
				break;
			case VWERASE:
				printf("VWERASE|");
				break;
			case VLNEXT:
				printf("VLNEXT|");
				break;
			case VEOL2:
				printf("VEOL2|");
				break;
			}
		}
	}
	printf("\n");
	return;
}

void
get_iflags(struct termios *tattrs)
{
	/* Get c_iflag bits that are set */
	printf("|");
	if (tattrs->c_iflag & IGNBRK)	printf("IGNBRK|");
	if (tattrs->c_iflag & BRKINT) 	printf("BRKINT|");
	if (tattrs->c_iflag & IGNPAR) 	printf("IGNPAR|");
	if (tattrs->c_iflag & PARMRK) 	printf("PARMRK|");
	if (tattrs->c_iflag & INPCK) 	printf("INPCK|");
	if (tattrs->c_iflag & ISTRIP) 	printf("ISTRIP|");
	if (tattrs->c_iflag & INLCR) 	printf("INLCR|");
	if (tattrs->c_iflag & IGNCR) 	printf("IGNCR|");
	if (tattrs->c_iflag & ICRNL) 	printf("ICRNL|");
	if (tattrs->c_iflag & IUCLC) 	printf("IUCLC|");
	if (tattrs->c_iflag & IXON) 	printf("IXON|");
	if (tattrs->c_iflag & IXANY) 	printf("IXANY|");
	if (tattrs->c_iflag & IXOFF) 	printf("IXOFF|");
	if (tattrs->c_iflag & IMAXBEL) 	printf("IMAXBEL|");
	if (tattrs->c_iflag & IUTF8) 	printf("IUTF8|");
	printf("\n");
	return;
}

void
get_oflags(struct termios *tattrs)
{
	/* Get c_oflag bits that are set */
	printf("|");
	if (tattrs->c_oflag & OPOST) 	printf("OPOST|");
	if (tattrs->c_oflag & OLCUC) 	printf("OLCUC|");
	if (tattrs->c_oflag & ONLCR) 	printf("ONLCR|");
	if (tattrs->c_oflag & OCRNL) 	printf("OCRNL|");
	if (tattrs->c_oflag & ONOCR) 	printf("ONOCR|");
	if (tattrs->c_oflag & ONLRET) 	printf("ONLRET|");
	if (tattrs->c_oflag & OFILL) 	printf("OFILL|");
	if (tattrs->c_oflag & OFDEL) 	printf("OFDEL|");
	if (tattrs->c_oflag & NLDLY) {
		if (tattrs->c_oflag & NL0) printf("NL0|");
		if (tattrs->c_oflag & NL1) printf("NL1|");
	}
	if (tattrs->c_oflag & CRDLY) {
		if (tattrs->c_oflag & CR0) printf("CR0|");
		if (tattrs->c_oflag & CR1) printf("CR1|");
		if (tattrs->c_oflag & CR2) printf("CR2|");
		if (tattrs->c_oflag & CR3) printf("CR3|");
	}
	if (tattrs->c_oflag & TABDLY) {
		if (tattrs->c_oflag & TAB0) printf("TAB0|");
		if (tattrs->c_oflag & TAB1) printf("TAB1|");
		if (tattrs->c_oflag & TAB2) printf("TAB2|");
		if (tattrs->c_oflag & TAB3) printf("TAB3|");
	}
	if (tattrs->c_oflag & BSDLY) {
		if (tattrs->c_oflag & BS0) printf("BS0|");
		if (tattrs->c_oflag & BS1) printf("BS1|");
	}
	if (tattrs->c_oflag & FFDLY) {
		if (tattrs->c_oflag & FF0) printf("FF0|");
		if (tattrs->c_oflag & FF1) printf("FF1|");
	}
	if (tattrs->c_oflag & VTDLY) {
		if (tattrs->c_oflag & VT0) printf("VT0|");
		if (tattrs->c_oflag & VT1) printf("VT1|");
	}
	printf("\n");
	return;
}

void
get_lflags(struct termios *tattrs)
{
	/* Get c_lflag bits that are set */
	printf("|");
	if (tattrs->c_lflag & ISIG) 	printf("ISIG|");
	if (tattrs->c_lflag & ICANON) 	printf("ICANON|");
	if (tattrs->c_lflag & XCASE) 	printf("XCASE|");
	if (tattrs->c_lflag & ECHO) 	printf("ECHO|");
	if (tattrs->c_lflag & ECHOE) 	printf("ECHOE|");
	if (tattrs->c_lflag & ECHOK) 	printf("ECHOK|");
	if (tattrs->c_lflag & ECHONL) 	printf("ECHONL|");
	if (tattrs->c_lflag & NOFLSH) 	printf("NOFLSH|");
	if (tattrs->c_lflag & TOSTOP) 	printf("TOSTOP|");
	if (tattrs->c_lflag & ECHOCTL) 	printf("ECHOCTL|");
	if (tattrs->c_lflag & ECHOPRT) 	printf("ECHOPRT|");
	if (tattrs->c_lflag & ECHOKE) 	printf("ECHOKE|");
	if (tattrs->c_lflag & FLUSHO) 	printf("FLUSHO|");
	if (tattrs->c_lflag & PENDIN) 	printf("PENDIN|");
	if (tattrs->c_lflag & IEXTEN) 	printf("IEXTEN|");
	if (tattrs->c_lflag & EXTPROC) 	printf("EXTPROC|");
	printf("\n");
	return;
}
