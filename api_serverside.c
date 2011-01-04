/*
 * Copyright (c) 2011 Pelle Johansson
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

#include "api_serverside.h"
#include "backend.h"
#include "status.h"

#include <sys/queue.h>
#include <sys/socket.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netdb.h>

struct serverside {
	TAILQ_ENTRY(serverside) link;

	int fd;
	struct backend_device *bdev;
	
	struct sockaddr_storage addr;
	socklen_t addrlen;
	int should_unlink;

	struct event ev;
};

struct ss_notify_code {
	char *code;
	status_notify_token_t token;
};


struct api_ss_conn {
	int fd;
	struct bufferevent *be;

	struct backend_device *bdev;

	struct ss_notify_code *codes;
	size_t num_codes;
	size_t alloced_codes;
};

TAILQ_HEAD(, serverside) serversides = TAILQ_HEAD_INITIALIZER(serversides);

void
ss_query_commands(struct api_ss_conn *conn, const char *arg, size_t len) {
	bufferevent_write(conn->be, "QCMD", 4);
#if 0
	while (len >= 4) {
		if (conn->bdev->status.query_command(conn->bdev, arg)) {
			bufferevent_write(conn->be, arg, 4);
		}
		arg += 4;
		len -= 4;
	}
#endif
	bufferevent_write(conn->be, "\n", 1);
}

static const char debase64[] =
	"\76" /* + */
	"\0\0\0"
	"\77\64\65\66\67\70\71\72\73\74\75" /* /, 0 - 9 */
	"\0\0\0"
	"\0" /* = */
	"\0\0\0"
	"\0\1\2\3\4\5\6\7\10\11\12\13\14\15\16\17\20\21\22\23\24\25\26\27\30\31" /* A - Z */
	"\0\0\0\0\0\0"
	"\32\33\34\35\36\37\40\41\42\43\44\45\46\47\50\51\52\53\54\55\56\57\60\61\62\63"; /* a - z */

void
ss_send_command(struct api_ss_conn *conn, const char *arg, size_t len) {
	char cmd[5];
	int narg = len / 4 - 1;
	int args[10];
	int i, j;
	const char *a;

	if (narg < 0) {
		warnx("Short line: %s", arg);
		return;
	}
	if (narg > 10) {
		warnx("Long line: %s", arg);
		return;
	}

	memcpy(cmd, arg, 4);
	cmd[4] = '\0';

	a = arg + 4;
	for (i = 0 ; i < narg ; i++) {
		args[i] = 0;
		for (j = 0 ; j < 4 ; j++) {
			if (*a < '+' || *a > 'z' || !debase64[*a - '+']) {
				warnx("Invalid line: %s", arg);
				return;
			}
			args[i] = args[i] << 6 | debase64[*a - '+'];
		}
	}

	backend_send_command(conn->bdev, cmd, narg, args);
}

#include "api_serverside_command.h"

void
serverside_handle(struct api_ss_conn *conn, const char *line, size_t len) {
	const struct api_serverside_command *cmd;

	if (len < 4) {
		warnx ("Short line: %s", line);
		return;
	}

	cmd = api_serverside_command (line, 4);
	if (cmd)
		cmd->handler (conn, line + 4, len - 4);
	else
		warnx ("Unknown command: %s", line);
}

static void
ss_conn_read (struct bufferevent *be, void *arg) {
	struct api_ss_conn *conn = arg;
	char *line;

	while ((line = evbuffer_readline(be->input))) {
		serverside_handle (conn, line, strlen(line));
		free (line);
	}
}

static void
ss_conn_write_done (struct bufferevent *be, void *arg) {
	bufferevent_disable (be, EV_WRITE);
}

static void
ss_conn_error (struct bufferevent *be, short what, void *arg) {
	struct api_ss_conn *conn = arg;
	struct ss_notify_code *code_note;

	if (what != (EVBUFFER_READ | EVBUFFER_EOF)) {
		/* Presume errno to still be up to date. */
		warn ("backend: read error: %d", what);
	}

	ss_conn_read (be, arg);
	if (EVBUFFER_LENGTH(be->input))
		serverside_handle(conn, (const char*)EVBUFFER_DATA(be->input), EVBUFFER_LENGTH(be->input));

	bufferevent_free (be);
	close (conn->fd);

	for (code_note = conn->codes; code_note < conn->codes + conn->num_codes; code_note++) {
		free (code_note->code);
		status_stop_notify (code_note->token);
	}
	free (conn);
}

static void
ss_accept_connection (int fd, short what, void *cbarg) {
	struct serverside *ss = cbarg;
	struct sockaddr_storage addr;
	socklen_t al = sizeof (addr);
	int cfd = accept (fd, (struct sockaddr*)&addr, &al);
	struct api_ss_conn *conn;

	if (cfd < 0) {
		warn ("ss_accept_connection");
		return;
	}

	conn = calloc (1, sizeof (*conn));
	if (!conn) {
		warn ("ss_accept_connection");
		close (cfd);
		return;
	}

	conn->bdev = ss->bdev;

	conn->fd = cfd;
	conn->be = bufferevent_new (cfd, ss_conn_read, ss_conn_write_done, ss_conn_error, conn);
	if (!conn->be) {
		warn ("ss_accept_connection");
		free (conn);
		close (cfd);
		return;
	}
	bufferevent_enable (conn->be, EV_READ);
}

static void
serverside_add (struct serverside *ss, int fd) {
	event_set (&ss->ev, fd, EV_READ | EV_PERSIST, ss_accept_connection, ss);
	if (event_add (&ss->ev, NULL)) {
		err(1, "event_add(%d)", fd);
	}
	ss->fd = fd;

	TAILQ_INSERT_TAIL(&serversides, ss, link);
}

void
serverside_listen_fd (struct backend_device *bdev, int fd) {
	struct serverside *ss = malloc(sizeof (*ss));

	if (!ss)
		err(1, "malloc");

	ss->bdev = bdev;
	ss->addrlen = sizeof(ss->addr);

	if (getsockname(fd, (struct sockaddr*)&ss->addr, &ss->addrlen))
		err(1, "getsockname");

	ss->should_unlink = 0;

	serverside_add(ss, fd);

}

void
serverside_listen_local (struct backend_device *bdev, const char *path) {
	struct serverside *ss = malloc(sizeof (*ss));
	struct sockaddr_un *sun = (struct sockaddr_un*)&ss->addr;
	int s;
	struct stat st;

	if (!ss)
		err(1, "malloc");

	if (!lstat(path, &st)) {
		if (!S_ISSOCK(st.st_mode))
			errx(1, "listen_local: %s exists but not socket", path);
		if (unlink(path))
			err(1, "unlink");
	}

	ss->bdev = bdev;
	ss->addrlen = sizeof (*sun);

	sun->sun_family = AF_UNIX;
	strlcpy(sun->sun_path, path, sizeof(sun->sun_path));

	ss->should_unlink = 1;

	s = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (s < 0)
		err(1, "socket(PF_LOCAL)");

	if (bind(s, (struct sockaddr*)&ss->addr, ss->addrlen))
		err(1, "bind");

	if (listen(s, 128))
		err(1, "listen");

	serverside_add(ss, s);
}

void
serverside_listen_tcp (struct backend_device *bdev, const char *serv)
{
	const struct addrinfo hints = { .ai_flags = AI_PASSIVE, .ai_socktype = SOCK_STREAM };
	struct addrinfo *res = NULL, *curr;
	int r;

	if ((r = getaddrinfo(NULL, serv, &hints, &res)))
		errx(1, "getaddrinfo(%s): %s", serv, gai_strerror(r));

	for (curr = res ; curr ; curr = curr->ai_next) {
		struct serverside *ss = malloc (sizeof (*ss));
		int s;
		const int one = 1;

		ss->bdev = bdev;
		ss->addrlen = curr->ai_addrlen;
		memcpy(&ss->addr, curr->ai_addr, curr->ai_addrlen);

		ss->should_unlink = 0;

		s = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
		if (s < 0)
			err(1, "socket(%s, %d)", serv, curr->ai_family);

		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (bind(s, curr->ai_addr, curr->ai_addrlen))
			err(1, "bind(%s, %d)", serv, curr->ai_family);

		if (listen(s, 128))
			err(1, "listen(%s, %d)", serv, curr->ai_family);

		serverside_add(ss, s);
	}

	freeaddrinfo(res);
}

void
serverside_close_all (void)
{
	struct serverside *ss;

	while ((ss = TAILQ_FIRST(&serversides))) {
		close(ss->fd);
		if (ss->should_unlink && ss->addr.ss_family == AF_UNIX) {
			struct sockaddr_un *sun = (struct sockaddr_un*)&ss->addr;

			unlink(sun->sun_path);
		}
	}
}
