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
#include "base64.h"

#include <sys/queue.h>
#include <sys/socket.h>
#include <string.h>
#include <err.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <netdb.h>
#include <search.h>

#include <list>
#include <map>
#include <string>

#include "smart_event.hh"
#include "smart_fd.hh"

class serverside;

struct api_ss_conn {
	smart_fd fd;
	serverside &ss;
	std::unique_ptr<bufferevent, decltype(&bufferevent_free)> be;

	std::map<std::string, std::unique_ptr<status_notify_info, decltype(&status_stop_notify)>> codes;

	api_ss_conn(serverside &ss, int fd);

	bool operator == (const api_ss_conn &r) const
	{
		return fd.fd == r.fd.fd;
	}
	bool operator != (const api_ss_conn &r) const
	{
		return fd.fd != r.fd.fd;
	}

	typedef void command_function(const char *arg, size_t len);
	typedef void (api_ss_conn::*command_function_ptr)(const char *arg, size_t len);
	command_function query_commands;
	command_function query_status;
	command_function send_command;
	command_function query;
	command_function start;
	command_function stop;
	command_function enable_server;
	command_function disable_server;

	void start_notify(const char *code, status_notify_cb_t cb, int replace);
	void stop_notify(const char *code);

	void handle(const char *line, size_t len);
};

class serverside {
public:
	std::string name;

	smart_fd fd;
	backend_ptr &bdev;

	struct sockaddr_storage addr;
	socklen_t addrlen;
	bool should_unlink;
	bool disabled;

	smart_event ev;

	std::list<api_ss_conn> conns;

	serverside(const char *name, backend_ptr &bdev, const struct sockaddr *addr, socklen_t addrlen, bool should_unlink, int fd);
	~serverside();

	void accept_connection(int fd);
};

std::list<serverside> serversides;

void
api_ss_conn::query_commands(const char *arg, size_t len)
{
	bufferevent_write(&*be, "QCMD", 4);
	while (len >= 4) {
		if (!status_query_command(ss.bdev.status(), arg)) {
			bufferevent_write(&*be, arg, 4);
		}
		arg += 4;
		len -= 4;
	}
	bufferevent_write(&*be, "\n", 1);
	bufferevent_enable(&*be, EV_WRITE);
}

void
api_ss_conn::query_status(const char *arg, size_t len)
{
	bufferevent_write(&*be, "QSTS", 4);
	while (len >= 4) {
		if (!status_query_status(ss.bdev.status(), arg)) {
			bufferevent_write(&*be, arg, 4);
		}
		arg += 4;
		len -= 4;
	}
	bufferevent_write(&*be, "\n", 1);
	bufferevent_enable(&*be, EV_WRITE);
}

void
api_ss_conn::send_command(const char *arg, size_t len)
{
	char cmd[5];
	int narg = len / 4 - 1;
	int32_t args[10];
	int i;
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
		args[i] = debase64_int24(a);
		a += 4;
	}

	ss.bdev.send_command(cmd, narg, args);
}

void
api_ss_conn::start_notify(const char *code, status_notify_cb_t cb, int replace)
{
	auto token = codes.emplace(code, decltype(codes)::mapped_type( NULL, &status_stop_notify ));
	status_notify_token_t newtoken;

	if (!token.second && !replace)
		return;

	newtoken = status_start_notify(ss.bdev.status(), code, cb, this);
	if (!newtoken) {
		warn ("ss_start_notify: backend_start_notify");
		return;
	}

	token.first->second.reset(newtoken);
}

void
api_ss_conn::stop_notify(const char *code) {
	codes.erase(code);
}

static void
ss_query_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, const char *val, size_t len) {
	auto conn = static_cast<api_ss_conn *>(cbarg);

	bufferevent_write(&*conn->be, "STAT", 4);
	bufferevent_write(&*conn->be, code, 4);
	bufferevent_write(&*conn->be, val, len);
	bufferevent_write(&*conn->be, "\n", 1);

	bufferevent_enable(&*conn->be, EV_WRITE);
	conn->stop_notify(code);
}

static void
ss_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, const char *val, size_t len) {
	auto conn = static_cast<api_ss_conn *>(cbarg);

	bufferevent_write(&*conn->be, "STAT", 4);
	bufferevent_write(&*conn->be, code, 4);
	bufferevent_write(&*conn->be, val, len);
	bufferevent_write(&*conn->be, "\n", 1);

	bufferevent_enable(&*conn->be, EV_WRITE);
}

void
api_ss_conn::query(const char *arg, size_t len)
{
	char buf[256];
	size_t l = sizeof(buf);

	if (len != 4) {
		warnx("ss_query: Invalid query %s", arg);
		return;
	}

	int res = status_query(ss.bdev.status(), arg, buf, &l);

	if (!res) {
		bufferevent_write(&*be, "STAT", 4);
		bufferevent_write(&*be, arg, len);
		bufferevent_write(&*be, buf, l);
		bufferevent_write(&*be, "\n", 1);
		bufferevent_enable(&*be, EV_WRITE);
		return;
	}

	if (res != STATUS_UNKNOWN) {
		warn ("ss_query");
		return;
	}

	start_notify(arg, ss_query_notify_cb, 0);
}

void
api_ss_conn::start(const char *arg, size_t len) {
	if (len != 4)
		return;
	start_notify(arg, ss_notify_cb, 1);
}

void
api_ss_conn::stop(const char *arg, size_t len) {
	if (len != 4)
		return;
	stop_notify(arg);
}

void
api_ss_conn::enable_server(const char *arg, size_t len)
{
	for (auto &ss : serversides) {
		if (ss.name == arg)
			ss.disabled = 0;
	}
}

void
api_ss_conn::disable_server(const char *arg, size_t len)
{
	for (auto &ss : serversides) {
		if (ss.name == arg)
			ss.disabled = 1;
	}
}

#include "api_serverside_command.h"

void
api_ss_conn::handle(const char *line, size_t len)
{
	const struct api_serverside_command *cmd;

	if (ss.disabled && strncmp(line, "SENA", 4) != 0) {
		bufferevent_write(&*be, "EDIS\n", 5);
		bufferevent_enable(&*be, EV_WRITE);
		return;
	}

	if (len < 4) {
		warnx ("Short line: %s", line);
		return;
	}

	cmd = api_serverside_command (line, 4);
	if (cmd)
		(this->*cmd->handler)(line + 4, len - 4);
	else {
		warnx ("Unknown command: %s", line);
		bufferevent_write(&*be, "ECMD\n", 5);
		bufferevent_enable(&*be, EV_WRITE);
	}
}

static void
ss_conn_read (struct bufferevent *be, void *arg) {
	auto conn = static_cast<api_ss_conn *>(arg);
	char *line;

	while ((line = evbuffer_readline(be->input))) {
		conn->handle(line, strlen(line));
		free (line);
	}
}

static void
ss_conn_write_done (struct bufferevent *be, void *arg) {
	bufferevent_disable (be, EV_WRITE);
}

static void
ss_conn_error (struct bufferevent *be, short what, void *arg) {
	auto conn = static_cast<api_ss_conn *>(arg);

	if (what != (EVBUFFER_READ | EVBUFFER_EOF)) {
		/* Presume errno to still be up to date. */
		warn ("backend: read error: %d", what);
	}

	ss_conn_read (be, arg);
	if (EVBUFFER_LENGTH(be->input))
		conn->handle((const char*)EVBUFFER_DATA(be->input), EVBUFFER_LENGTH(be->input));

	conn->ss.conns.erase(std::find(conn->ss.conns.begin(), conn->ss.conns.end(), *conn));
}

void
serverside::accept_connection(int fd)
{
	struct sockaddr_storage addr;
	socklen_t al = sizeof (addr);
	int cfd = accept (fd, (struct sockaddr*)&addr, &al);

	if (cfd < 0) {
		warn ("accept_connection");
		return;
	}

	try {
		conns.emplace_back(*this, cfd);
	} catch(std::bad_alloc) {
		warn ("accept_connection");
		close(cfd);
	}
}

api_ss_conn::api_ss_conn(serverside &ss, int fd)
	: fd(fd), ss(ss), be(NULL, bufferevent_free)
{
	be.reset(bufferevent_new(fd, ss_conn_read, ss_conn_write_done, ss_conn_error, this));
	if (!be)
		throw std::bad_alloc();
	bufferevent_enable(&*be, EV_READ);
}

void
serverside_listen_fd(const char *name, backend_ptr &bdev, int fd)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	if (getsockname(fd, (struct sockaddr*)&addr, &addrlen))
		err(1, "getsockname");

	serversides.emplace_back(name, bdev, (struct sockaddr*)&addr, addrlen, false, fd);
}

void
serverside_listen_local(const char *name, backend_ptr &bdev, const char *path) {
	struct sockaddr_storage addr;
	struct sockaddr_un *sun = (struct sockaddr_un *)&addr;
	int s;
	struct stat st;

	if (!lstat(path, &st)) {
		if (!S_ISSOCK(st.st_mode))
			errx(1, "listen_local: %s exists but not socket", path);
		if (unlink(path))
			err(1, "unlink");
	}

	sun->sun_family = AF_UNIX;
	strlcpy(sun->sun_path, path, sizeof(sun->sun_path));

	s = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (s < 0)
		err(1, "socket(PF_LOCAL)");

	if (bind(s, (struct sockaddr*)sun, sizeof(*sun)))
		err(1, "bind");
	/* Allow all by default. */
	chmod(path, 0777);

	if (listen(s, 128))
		err(1, "listen");

	serversides.emplace_back(name, bdev, (struct sockaddr*)&addr, sizeof(*sun), true, s);
}

void
serverside_listen_tcp(const char *name, backend_ptr &bdev, const char *serv)
{
	const struct addrinfo hints = { .ai_flags = AI_PASSIVE, .ai_socktype = SOCK_STREAM };
	struct addrinfo *res = NULL, *curr;
	int r;

	if ((r = getaddrinfo(NULL, serv, &hints, &res)))
		errx(1, "getaddrinfo(%s): %s", serv, gai_strerror(r));

	for (curr = res ; curr ; curr = curr->ai_next) {
		int s;
		const int one = 1;

		s = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
		if (s < 0)
			err(1, "socket(%s, %d)", serv, curr->ai_family);

		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (curr->ai_family == AF_INET6)
			setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));

		if (bind(s, curr->ai_addr, curr->ai_addrlen))
			err(1, "bind(%s, %d)", serv, curr->ai_family);

		if (listen(s, 128))
			err(1, "listen(%s, %d)", serv, curr->ai_family);

		serversides.emplace_back(name, bdev, curr->ai_addr, curr->ai_addrlen, false, s);
	}

	freeaddrinfo(res);
}

serverside::serverside(const char *name, backend_ptr &bdev, const struct sockaddr *addr, socklen_t addrlen, bool should_unlink, int fd)
	: name(name), fd(fd), bdev(bdev), addrlen(addrlen), should_unlink(should_unlink)
{
	memcpy(&this->addr, addr, addrlen);

	ev.set_fd(fd);
	ev.set(EV_READ | EV_PERSIST, std::bind(&serverside::accept_connection, this, std::placeholders::_1));
	if (ev.add()) {
		err(1, "event_add(%d)", fd);
	}
}

serverside::~serverside()
{
	close(fd);
	if (should_unlink && addr.ss_family == AF_UNIX) {
		struct sockaddr_un *sun = (struct sockaddr_un*)&addr;

		unlink(sun->sun_path);
	}
}

void
serverside_close_all (void)
{
	serversides.clear();
}
