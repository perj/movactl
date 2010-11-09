/*
 * Copyright (c) 2008 Pelle Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "line.h"

#include <termios.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <string.h>
#include <stdio.h>
#define TTYDEFCHARS
#include <sys/ttydefaults.h>

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
	struct termios curattr;

	if (fd < 0)
		return -1;

	tattr.c_iflag = IGNBRK;
	tattr.c_oflag = 0;
	tattr.c_cflag = MA_DATABITS | MA_STOPFLAG | CREAD | MA_PARITY | CLOCAL;
	tattr.c_lflag = 0;

	memcpy (tattr.c_cc, ttydefchars, sizeof (ttydefchars));

	tattr.c_ispeed = tattr.c_ospeed = MA_SPEED;

	if (tcgetattr (fd, &curattr)) {
		close (fd);
		return -1;
	}

	if (memcmp (&curattr, &tattr, sizeof (tattr)) != 0) {
		/* Only change attrs if needed. */
		if (tcsetattr (fd, TCSANOW, &tattr)) {
			close (fd);
			return -1;
		}
	}
	
	/* Reenable blocking
	 * Testing to skip this to see if it help with the stuck daemon problem.
	 */
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

		if (tot == 1 && (buf[0] == '\r' || buf[0] == '\n')) {
			/* Skip empty lines. */
			tot = 0;
			continue;
		}

	} while (tot < buflen - 1 && buf[tot - 1] != '\r' && buf[tot - 1] != '\n');

	if (buf[tot - 1] == '\r' || buf[tot - 1] == '\n') {
		buf[tot - 1] = '\0';
		return tot - 1;
	}

	buf[tot] = '\0';
	return tot;
}

int
send_command (int fd, const char *command, const char *arg) {
	struct iovec vecs[5];

	if (isatty (fd)) {
		vecs[0].iov_base = (char*)"@";
		vecs[0].iov_len = 1;
	} else {
		/* Assume proxy socket. */
		vecs[0].iov_base = (char*)"send ";
		vecs[0].iov_len = 5;
	}
	vecs[1].iov_base = (char*)command;
	vecs[1].iov_len = strlen (command);
	vecs[2].iov_base = (char*)":";
	vecs[2].iov_len = 1;
	vecs[3].iov_base = (char*)arg;
	vecs[3].iov_len = strlen (arg);
	vecs[4].iov_base = (char*)"\r";
	vecs[4].iov_len = 1;

	fprintf (stderr, "Sending command %s:%s\n", command, arg);

	return writev(fd, vecs, 5);
}

int
send_status_request (int fd, const char *status) {
	struct iovec vecs[3];

	vecs[0].iov_base = (char*)"@";
	vecs[0].iov_len = 1;
	vecs[1].iov_base = (char*)status;
	vecs[1].iov_len = strlen (status);
	vecs[2].iov_base = (char*)":?\r";
	vecs[2].iov_len = 3;

	return writev(fd, vecs, 3);
}

