#define __USE_MISC
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <inttypes.h>
#include <termios.h>
#include <sys/ioctl.h>

void get_iflags(struct termios *tattrs, int short_desc);
void get_oflags(struct termios *tattrs, int short_desc);
void get_cflags(struct termios *tattrs, int short_desc);
void get_lflags(struct termios *tattrs, int short_desc);
void get_ctrlchars(struct termios *tattrs, int short_desc);
void get_shortflags(struct termios *in_tattrs, struct termios *out_tattrs, struct termios *err_tattrs);

int
main (int argc, char **argv)
{
	if (argc > 2) {
		fprintf(stderr, "usage: %s        (short descriptions)\n", argv[0]);
		fprintf(stderr, "       %s -l     (long descriptions)\n", argv[0]);
	}

	struct termios in_tattrs, out_tattrs, err_tattrs;

	if (tcgetattr(STDIN_FILENO, &in_tattrs) == -1) {
		perror("tcgetattr on stdin");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(STDOUT_FILENO, &out_tattrs) == -1) {
		perror("tcgetattr on stdout");
		exit(EXIT_FAILURE);
	}

	if (tcgetattr(STDERR_FILENO, &err_tattrs) == -1) {
		perror("tcgetattr on stderr");
		exit(EXIT_FAILURE);
	}

	printf("---------------------------\n");
	printf("TERMINAL ATTRIBUTES:\n");
	printf("---------------------------\n");

	if (argc == 1) {
		get_shortflags(&in_tattrs, &out_tattrs, &err_tattrs);
		return 0;
	}

	printf("**************************************\n");
	printf("STDIN:\n");
	printf("--------------------------\n");
	get_iflags(&in_tattrs, 0);
	get_oflags(&in_tattrs, 0);
	get_cflags(&in_tattrs, 0);
	get_lflags(&in_tattrs, 0);
	get_ctrlchars(&in_tattrs, 0);
	printf("\n**************************************\n");
	printf("STDOUT:\n");
	printf("---------------------------\n");
	get_iflags(&out_tattrs, 0);
	get_oflags(&out_tattrs, 0);
	get_cflags(&out_tattrs, 0);
	get_lflags(&out_tattrs, 0);
	get_ctrlchars(&out_tattrs, 0);
	printf("\n**************************************\n");
	printf("STDERR:\n");
	printf("---------------------------\n");
	get_iflags(&err_tattrs, 0);
	get_oflags(&err_tattrs, 0);
	get_cflags(&err_tattrs, 0);
	get_lflags(&err_tattrs, 0);
	get_ctrlchars(&err_tattrs, 0);
	printf("**************************************\n");
	return 0;
}

void
get_shortflags(struct termios *in_tattrs, struct termios *out_tattrs, struct termios *err_tattrs)
{
	printf("[*] termios.c_iflags:\n");
	printf("STDIN : ");
	get_iflags(in_tattrs, 1);
	printf("STDOUT: ");
	get_iflags(out_tattrs, 1);
	printf("STDERR: ");
	get_iflags(err_tattrs, 1);

	printf("[*] termios.c_oflags:\n");
	printf("STDIN : ");
	get_oflags(in_tattrs, 1);
	printf("STDOUT: ");
	get_oflags(out_tattrs, 1);
	printf("STDERR: ");
	get_oflags(err_tattrs, 1);

	printf("[*] termios.c_lflags:\n");
	printf("STDIN : ");
	get_lflags(in_tattrs, 1);
	printf("STDOUT: ");
	get_lflags(out_tattrs, 1);
	printf("STDERR: ");
	get_lflags(err_tattrs, 1);

	printf("[*] termios.c_cflags:\n");
	printf("STDIN : ");
	get_cflags(in_tattrs, 1);
	printf("STDOUT: ");
	get_cflags(out_tattrs, 1);
	printf("STDERR: ");
	get_cflags(err_tattrs, 1);

	printf("[*] termios.c_cc[]:\n");
	printf("STDIN : ");
	get_ctrlchars(in_tattrs, 1);
	printf("STDOUT: ");
	get_ctrlchars(out_tattrs, 1);
	printf("STDERR: ");
	get_ctrlchars(err_tattrs, 1);
	return;
}

void
get_cflags(struct termios *tattrs, int short_desc)
{
	/* Get c_cflag bits that are set */
	if (short_desc) {
		printf("(");
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
		if (tattrs->c_cflag & CRTSCTS) 	printf("CRTSCTS");
		if (tattrs->c_cflag & CSIZE) {
			printf("|");
			if (tattrs->c_cflag & CS5) printf("CS5|");
			if (tattrs->c_cflag & CS6) printf("CS6|");
			if (tattrs->c_cflag & CS7) printf("CS7|");
			if (tattrs->c_cflag & CS8) printf("CS8");
		}
		printf(")\n");
		return;
	}

	printf("[*] c_cflag values (control mode flags):\n");
	if (tattrs->c_cflag & CSTOPB) 	printf("CSTOPB: Send two stop bits else one.\n");
	if (tattrs->c_cflag & CREAD) 	printf("CREAD: Enable receiver.\n");
	if (tattrs->c_cflag & PARENB) 	printf("PARENB: Parity enable.\n");
	if (tattrs->c_cflag & PARODD) 	printf("PARODD: Odd parity else even.\n");
	if (tattrs->c_cflag & HUPCL) 	printf("HUPCL: Hang up on last close.\n");
	if (tattrs->c_cflag & CLOCAL) 	printf("CLOCAL: Ignore modem status lines.\n");
	if (tattrs->c_cflag & CBAUD) 	printf("CBAUD: Baud speed mask (not in POSIX).\n");
	if (tattrs->c_cflag & CBAUDEX) 	printf("CBAUDEX: Extra baud speed mask, included in CBAUD (not in POSIX).\n");
	if (tattrs->c_cflag & CIBAUD) 	printf("CIBAUD: Input baud rate (not used).\n");
	if (tattrs->c_cflag & CMSPAR) 	printf("CMSPAR: Mark or space (stick) parity (__USE_MISC).\n");
	if (tattrs->c_cflag & CRTSCTS) 	printf("CRTSCTS: Flow control (__USE_MISC).\n");
	if (tattrs->c_cflag & CSIZE) {
		printf("Number of bits per byte: ");
		if (tattrs->c_cflag & CS5) printf("CS5 ");
		if (tattrs->c_cflag & CS6) printf("CS6 ");
		if (tattrs->c_cflag & CS7) printf("CS7 ");
		if (tattrs->c_cflag & CS8) printf("CS8");
		printf("\n");
	}
	return;
}

void
get_ctrlchars(struct termios *tattrs, int short_desc)
{
	/* Get c_cc characters that are defined */
	if (short_desc) {
		printf("(");
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
					printf("VEOL2");
					break;
				}
			}
		}
		printf(")\n");
		return;
	}


	printf("[*] c_cc[32] values (control characters):\n");
	for (int i = 0; i < 17; i++) {
		if (tattrs->c_cc[i]) {
			printf("termios.c_cc[%2d]: %o\n", i, tattrs->c_cc[i]);
			switch (tattrs->c_cc[i]) {
			case VINTR: 
				printf("[%d] VINTR: Generates a SIGINT signal to all processes in the\n", i);
				printf("            ctty's process group (ISIG). [default: Ctrl-C]\n");
				break;
			case VQUIT:
				printf("[%d] VQUIT: Generates a SIGQUIT signal to all processes in the\n", i);
				printf("            ctty's process group (ISIG). [default: Ctrl-\\]\n");
				break;
			case VERASE:
				printf("[%d] VERASE: Erases the last character in the current line (requires\n", i);
				printf("             ICANON). [default: BS]\n");
				break;
			case VKILL:
				printf("[%d] VKILL: Deletes the entire line as delimited by the NL, EOF or EOL\n", i);
				printf("            character (ICANON). [default: Ctrl-U]\n");
				break;
			case VEOF:
				printf("[%d] VEOF: All of the bytes waiting to be read are immediately passed to\n", i);
				printf("           the process (ICANON). [default: Ctrl-D]\n");
				break;
			case VTIME:
				printf("[%d] VTIME: Timeout in deciseconds for noncanonical read.\n", i);
				break;
			case VMIN:
				printf("[%d] VMIN: Minimum number of characters for noncanonical read.\n", i);
				break;
			case VSTART:
				printf("[%d] VSTART: Restarts output stopped by the Stop character (IXON or\n", i);
				printf("             IXOFF). [default: Ctrl-Q]\n");
				break;
			case VSTOP:
				printf("[%d] VSTOP: Stop output until Start character typed (IXON or IXOFF).\n", i);
				printf("            [default: Ctrl-S]\n");
				break;
			case VSUSP:
				printf("[%d] VSUSP: Sends SIGTSTP signal (ISIG).\n", i);
				printf("            [default: Ctrl-Z]\n");
				break;
			case VEOL:
				printf("[%d] VEOL: Additional line delimiter like NL (ICANON).\n", i);
				break;
			case VREPRINT:
				printf("[%d] VREPRINT: Reprint unread characters (not in POSIX).\n", i);
				printf("               [default: Ctrl-R]\n");
				break;
			case VWERASE:
				printf("[%d] VWERASE: Word erase (not in POSIX).\n", i);
				printf("              [default: Ctrl-W]\n");
				break;
			case VLNEXT:
				printf("[%d] VLNEXT: Quotes the next input character (not in POSIX).\n", i);
				printf("             [default: Ctrl-V]\n");
				break;
			case VEOL2:
				printf("[%d] VEOL2: Yet another end-of-line character (not in POSIX).\n", i);
				break;
			}
		}
	}
	return;
}

void
get_iflags(struct termios *tattrs, int short_desc)
{
	/* Get c_iflag bits that are set */
	if (short_desc) {
		printf("(");
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
		if (tattrs->c_iflag & IUTF8) 	printf("IUTF8");
		printf(")\n");
		return;
	}

	printf("[*] c_iflag values (input mode flags):\n");
	if (tattrs->c_iflag & IGNBRK)	printf("IGNBRK: Ignore break condition.\n");
	if (tattrs->c_iflag & BRKINT) 	printf("BRKINT: Signal interrupt on break.\n");
	if (tattrs->c_iflag & IGNPAR) 	printf("IGNPAR: Ignore characters with parity errors.\n");
	if (tattrs->c_iflag & PARMRK) 	printf("PARMRK: Mark parity and framing errors.\n");
	if (tattrs->c_iflag & INPCK) 	printf("INPCK: Enable input parity check.\n");
	if (tattrs->c_iflag & ISTRIP) 	printf("ISTRIP: Strip 8th bit off characters.\n");
	if (tattrs->c_iflag & INLCR) 	printf("INLCR: Map NL to CR on input.\n");
	if (tattrs->c_iflag & IGNCR) 	printf("IGNCR: Ignore CR.\n");
	if (tattrs->c_iflag & ICRNL) 	printf("ICRNL: Map CR to NL on input.\n");
	if (tattrs->c_iflag & IUCLC) 	printf("IUCLC: Map uppercase characters to lowercase on input (not in POSIX).\n");
	if (tattrs->c_iflag & IXON) 	printf("IXON: Enable start/stop output control.\n");
	if (tattrs->c_iflag & IXANY) 	printf("IXANY: Enable any character to restart output.\n");
	if (tattrs->c_iflag & IXOFF) 	printf("IXOFF: Enable start/stop input control.\n");
	if (tattrs->c_iflag & IMAXBEL) 	printf("IMAXBEL: Ring bell when input queue is full (not in POSIX).\n");
	if (tattrs->c_iflag & IUTF8) 	printf("IUTF8: Input is UTF8 (not in POSIX).\n");
	return;
}

void
get_oflags(struct termios *tattrs, int short_desc)
{
	/* Get c_oflag bits that are set */
	if (short_desc) {
		printf("(");
		if (tattrs->c_oflag & OPOST) 	printf("OPOST|");
		if (tattrs->c_oflag & OLCUC) 	printf("OLCUC|");
		if (tattrs->c_oflag & ONLCR) 	printf("ONLCR|");
		if (tattrs->c_oflag & OCRNL) 	printf("OCRNL|");
		if (tattrs->c_oflag & ONOCR) 	printf("ONOCR|");
		if (tattrs->c_oflag & ONLRET) 	printf("ONLRET|");
		if (tattrs->c_oflag & OFILL) 	printf("OFILL|");
		if (tattrs->c_oflag & OFDEL) 	printf("OFDEL|");
		if (tattrs->c_oflag & NLDLY) {
			printf("[");
			if (tattrs->c_oflag & NL0) printf("NL0|");
			if (tattrs->c_oflag & NL1) printf("NL1");
			printf("]");
		}
		if (tattrs->c_oflag & CRDLY) {
			printf("[");
			if (tattrs->c_oflag & CR0) printf("CR0|");
			if (tattrs->c_oflag & CR1) printf("CR1|");
			if (tattrs->c_oflag & CR2) printf("CR2|");
			if (tattrs->c_oflag & CR3) printf("CR3");
			printf("]");
		}
		if (tattrs->c_oflag & TABDLY) {
			printf("[");
			if (tattrs->c_oflag & TAB0) printf("TAB0|");
			if (tattrs->c_oflag & TAB1) printf("TAB1|");
			if (tattrs->c_oflag & TAB2) printf("TAB2|");
			if (tattrs->c_oflag & TAB3) printf("TAB3");
			printf("]");
		}
		if (tattrs->c_oflag & BSDLY) {
			printf("[");
			if (tattrs->c_oflag & BS0) printf("BS0|");
			if (tattrs->c_oflag & BS1) printf("BS1");
			printf("]");
		}
		if (tattrs->c_oflag & FFDLY) {
			printf("[");
			if (tattrs->c_oflag & FF0) printf("FF0|");
			if (tattrs->c_oflag & FF1) printf("FF1");
			printf("]");
		}
		if (tattrs->c_oflag & VTDLY) {
			printf("[");
			if (tattrs->c_oflag & VT0) printf("VT0|");
			if (tattrs->c_oflag & VT1) printf("VT1");
			printf("]");
		}
		printf(")\n");
		return;
	}

	printf("[*] c_oflag values (output mode flags):\n");
	if (tattrs->c_oflag & OPOST) 	printf("OPOST: Post-process output.\n");
	if (tattrs->c_oflag & OLCUC) 	printf("OLCUC: Map lowercase characters to uppercase on output (not in POSIX).\n");
	if (tattrs->c_oflag & ONLCR) 	printf("ONLCR: Map NL to CR-NL on output.\n");
	if (tattrs->c_oflag & OCRNL) 	printf("OCRNL: Map CR to NL on output.\n");
	if (tattrs->c_oflag & ONOCR) 	printf("ONOCR: No CR output at column 0.\n");
	if (tattrs->c_oflag & ONLRET) 	printf("ONLRET: NL performs CR function.\n");
	if (tattrs->c_oflag & OFILL) 	printf("OFILL: Use fill characters for delay.\n");
	if (tattrs->c_oflag & OFDEL) 	printf("OFDEL: Fill is DEL.\n");
	if (tattrs->c_oflag & NLDLY) {
		printf("NLDLY: Selected newline delays (__USE_MISC|__USE_XOPEN): ");
		if (tattrs->c_oflag & NL0) printf("NL0.");
		if (tattrs->c_oflag & NL1) printf("NL1.");
		printf("\n");
	}
	if (tattrs->c_oflag & CRDLY) {
		printf("CRDLY: Selected carriage-return delays (__USE_MISC|__USE_XOPEN): ");
		if (tattrs->c_oflag & CR0) printf("CR0.");
		if (tattrs->c_oflag & CR1) printf("CR1.");
		if (tattrs->c_oflag & CR2) printf("CR2.");
		if (tattrs->c_oflag & CR3) printf("CR3.");
		printf("\n");
	}
	if (tattrs->c_oflag & TABDLY) {
		printf("TABDLY: Selected horizontal-tab delays (__USE_MISC|__USE_XOPEN): ");
		if (tattrs->c_oflag & TAB0) printf("TAB0.");
		if (tattrs->c_oflag & TAB1) printf("TAB1.");
		if (tattrs->c_oflag & TAB2) printf("TAB2.");
		if (tattrs->c_oflag & TAB3) printf("TAB3 (Expand tabs to spaces).");
		printf("\n");
	}
	if (tattrs->c_oflag & BSDLY) {
		printf("BSDLY: Selected backspace delays (__USE_MISC|__USE_XOPEN): ");
		if (tattrs->c_oflag & BS0) printf("BS0.");
		if (tattrs->c_oflag & BS1) printf("BS1.");
		printf("\n");
	}
	if (tattrs->c_oflag & FFDLY) {
		printf("FFDLY: Selected form-feed delays (__USE_MISC|__USE_XOPEN): ");
		if (tattrs->c_oflag & FF0) printf("FF0.");
		if (tattrs->c_oflag & FF1) printf("FF1.");
		printf("\n");
	}
	if (tattrs->c_oflag & VTDLY) {
		printf("VTDLY: Selected vertical-tab delays: ");
		if (tattrs->c_oflag & VT0) printf("VT0.");
		if (tattrs->c_oflag & VT1) printf("VT1.");
		printf("\n");
	}
	return;
}

void
get_lflags(struct termios *tattrs, int short_desc)
{
	/* Get c_lflag bits that are set */
	if (short_desc) {
		printf("(");
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
		if (tattrs->c_lflag & EXTPROC) 	printf("EXTPROC");
		printf(")\n");
		return;
	}

	printf("[*] c_lflag values (local mode flags):\n");
	if (tattrs->c_lflag & ISIG) 	printf("ISIG: Enable signals.\n");
	if (tattrs->c_lflag & ICANON) 	printf("ICANON: Canonical input (erase and kill processing).\n");
	if (tattrs->c_lflag & XCASE) 	printf("XCASE: If ICANON is also set, terminal is uppercase only (__USE_MISC).\n");
	if (tattrs->c_lflag & ECHO) 	printf("ECHO: Enable echo.\n");
	if (tattrs->c_lflag & ECHOE) 	printf("ECHOE: Echo erase character as error-correcting backspace.\n");
	if (tattrs->c_lflag & ECHOK) 	printf("ECHOK: Echo KILL.\n");
	if (tattrs->c_lflag & ECHONL) 	printf("ECHONL: Echo NL.\n");
	if (tattrs->c_lflag & NOFLSH) 	printf("NOFLSH: Disable flush after interrupt or quit.\n");
	if (tattrs->c_lflag & TOSTOP) 	printf("TOSTOP: Send SIGTTOU for background output.\n");
	if (tattrs->c_lflag & ECHOCTL) {
		printf("ECHOCTL: If ECHO is also set, terminal special characters other\n");
		printf("         than TAB, NL, START, and STOP are echoed as ^X (not in POSIX;\n");
		printf("         requires __USE_MISC).\n");
	}
	if (tattrs->c_lflag & ECHOPRT) {
		printf("ECHOPRT: If ICANON and ECHO are also set, characters are printed as\n");
		printf("         they are being erased (not in POSIX; requires __USE_MISC).\n");
	}
	if (tattrs->c_lflag & ECHOKE) {
		printf("ECHOKE: If ICANON is also set, KILL is echoed by erasing each\n");
		printf("        character on the line, as specified by ECHOE and ECHOPRT\n");
		printf("        (not in POSIX; requires __USE_MISC).\n");
	}
	if (tattrs->c_lflag & FLUSHO) {
		printf("FLUSHO: Output is being flushed. This flag is toggled by typing\n");
		printf("        the DISCARD character (not in POSIX; requires __USE_MISC).\n");
	}									
	if (tattrs->c_lflag & PENDIN) {
		printf("PENDIN: All characters in the input queue are reprinted when the\n");
		printf("        next character is read (not in POSIX; requires __USE_MISC).\n");
	}
	if (tattrs->c_lflag & IEXTEN) 	printf("IEXTEN: Enable implementation-defined input processing.\n");
	if (tattrs->c_lflag & EXTPROC) 	printf("EXTPROC: (__USE_MISC)\n");
	return;
}
