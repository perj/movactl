
#include "line.h"

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <string.h>
#include <stdio.h>

#define MA_SPEED B9600
#define MA_DATABITS CS8
#define MA_STOPBITS 1 
#define MA_PARITY   0

#if MA_STOPBITS == 2
#define MA_STOPFLAG CSTOPB
#else
#define MA_STOPFLAG 0
#endif

int
open_line (const char *path, int mode) {
	int fd = open (path, mode | O_NONBLOCK);
	struct termios tattr = {0};

	if (fd < 0)
		return -1;

	tattr.c_iflag = IGNBRK | ICANON;
	tattr.c_oflag = 0;
	tattr.c_cflag = MA_DATABITS | MA_STOPFLAG | CREAD | MA_PARITY | CLOCAL;
	tattr.c_lflag = 0;

	tattr.c_cc[VEOL] = '\r';

	tattr.c_ispeed = tattr.c_ospeed = MA_SPEED;

	if (tcsetattr (fd, TCSANOW, &tattr)) {
		close (fd);
		return -1;
	}
	
	/* Reenable blocking */
	if (fcntl (fd, F_SETFL, 0)) {
		close (fd);
		return -1;
	}

	/* Write a CR for flushing */
	write (fd, "\r", 1);

	return fd;
}

int
read_line (int fd, char *buf, int buflen) {
	int tot = 0;

	do {
		int res = read (fd, buf + tot, buflen - tot - 1);

		if (res < 0)
			return -1;

		if (res == 0)
			return 0;

		tot += res;
	} while (tot < buflen - 1 && buf[tot - 1] != '\r');

	if (buf[tot - 1] == '\r') {
		buf[tot - 1] = '\0';
		return tot - 1;
	}

	buf[tot] = '\0';
	return tot;
}

int
send_command (int fd, const char *command, const char *arg) {
	struct iovec vecs[5];

	vecs[0].iov_base = "@";
	vecs[0].iov_len = 1;
	vecs[1].iov_base = (char*)command;
	vecs[1].iov_len = strlen (command);
	vecs[2].iov_base = ":";
	vecs[2].iov_len = 1;
	vecs[3].iov_base = (char*)arg;
	vecs[3].iov_len = strlen (arg);
	vecs[4].iov_base = "\r";
	vecs[4].iov_len = 1;

	fprintf (stderr, "Sending command %s:%s\n", command, arg);

	return writev(fd, vecs, 5);
}

int
send_status_request (int fd, const char *status) {
	struct iovec vecs[3];

	vecs[0].iov_base = "@";
	vecs[0].iov_len = 1;
	vecs[1].iov_base = (char*)status;
	vecs[1].iov_len = strlen (status);
	vecs[2].iov_base = ":?\r";
	vecs[2].iov_len = 3;

	return writev(fd, vecs, 3);
}

