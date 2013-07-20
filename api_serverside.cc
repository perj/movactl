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

#include "smart_bufferevent.hh"
#include "smart_event.hh"
#include "smart_fd.hh"

class serverside;

struct api_ss_conn {
	smart_fd fd;
	serverside &ss;
	smart_bufferevent be;

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

	typedef void command_function(const std::string &arg);
	typedef void (api_ss_conn::*command_function_ptr)(const std::string &arg);
	command_function query_commands;
	command_function query_status;
	command_function send_command;
	command_function query;
	command_function start;
	command_function stop;
	command_function enable_server;
	command_function disable_server;

	void start_notify(const std::string &code, status_notify_cb_t cb, int replace);
	void stop_notify(const std::string &code);

	void handle(const std::string &line);
	void readcb();
	void writecb();
	void errorcb(short what);
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

	serverside(std::string name, backend_ptr &bdev, const struct sockaddr *addr, socklen_t addrlen, bool should_unlink, int fd);
	~serverside();

	void accept_connection(int fd);
};

std::list<serverside> serversides;

void
api_ss_conn::query_commands(const std::string &arg)
{
	bufferevent_write(be, "QCMD", 4);
	for (size_t i = 0 ; i < arg.length() ; i += 4) {
		std::string cmd = arg.substr(i, 4);
		if (!status_query_command(ss.bdev.status(), cmd.c_str())) {
			bufferevent_write(be, cmd.c_str(), 4);
		}
	}
	bufferevent_write(be, "\n", 1);
	bufferevent_enable(be, EV_WRITE);
}

void
api_ss_conn::query_status(const std::string &arg)
{
	bufferevent_write(be, "QSTS", 4);
	for (size_t i = 0 ; i < arg.length() ; i += 4) {
		std::string cmd = arg.substr(i, 4);
		if (!status_query_status(ss.bdev.status(), cmd.c_str())) {
			bufferevent_write(be, cmd.c_str(), 4);
		}
	}
	bufferevent_write(be, "\n", 1);
	bufferevent_enable(be, EV_WRITE);
}

void
api_ss_conn::send_command(const std::string &arg)
{
	if (arg.length() < 4) {
		warnx("Short line: %s", arg.c_str());
		return;
	}

	std::string cmd = arg.substr(0, 4);

	std::vector<int32_t> args;
	for (size_t i = 4 ; i < arg.length() ; i += 4) {
		std::string a = arg.substr(i, 4);
		args.emplace_back(debase64_int24(a.c_str()));
	}

	ss.bdev.send_command(cmd, args);
}

void
api_ss_conn::start_notify(const std::string &code, status_notify_cb_t cb, int replace)
{
	auto token = codes.emplace(code, decltype(codes)::mapped_type( NULL, &status_stop_notify ));
	status_notify_token_t newtoken;

	if (!token.second && !replace)
		return;

	newtoken = status_start_notify(ss.bdev.status(), code.c_str(), cb, this);
	if (!newtoken) {
		warn ("ss_start_notify: backend_start_notify");
		return;
	}

	token.first->second.reset(newtoken);
}

void
api_ss_conn::stop_notify(const std::string &code)
{
	codes.erase(code);
}

static void
ss_query_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, const char *val, size_t len) {
	auto conn = static_cast<api_ss_conn *>(cbarg);

	bufferevent_write(conn->be, "STAT", 4);
	bufferevent_write(conn->be, code, 4);
	bufferevent_write(conn->be, val, len);
	bufferevent_write(conn->be, "\n", 1);

	bufferevent_enable(conn->be, EV_WRITE);
	conn->stop_notify(code);
}

static void
ss_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, const char *val, size_t len) {
	auto conn = static_cast<api_ss_conn *>(cbarg);

	bufferevent_write(conn->be, "STAT", 4);
	bufferevent_write(conn->be, code, 4);
	bufferevent_write(conn->be, val, len);
	bufferevent_write(conn->be, "\n", 1);

	bufferevent_enable(conn->be, EV_WRITE);
}

void
api_ss_conn::query(const std::string &arg)
{
	char buf[256];
	size_t l = sizeof(buf);

	if (arg.length() != 4) {
		warnx("ss_query: Invalid query %s", arg.c_str());
		return;
	}

	int res = status_query(ss.bdev.status(), arg.c_str(), buf, &l);

	if (!res) {
		be.write("STAT");
		be.write(arg);
		bufferevent_write(be, buf, l);
		be.write("\n");
		bufferevent_enable(be, EV_WRITE);
		return;
	}

	if (res != STATUS_UNKNOWN) {
		warn ("ss_query");
		return;
	}

	start_notify(arg, ss_query_notify_cb, 0);
}

void
api_ss_conn::start(const std::string &arg)
{
	if (arg.length() != 4)
		return;
	start_notify(arg, ss_notify_cb, 1);
}

void
api_ss_conn::stop(const std::string &arg)
{
	if (arg.length() != 4)
		return;
	stop_notify(arg);
}

void
api_ss_conn::enable_server(const std::string &arg)
{
	for (auto &ss : serversides) {
		if (ss.name == arg)
			ss.disabled = 0;
	}
}

void
api_ss_conn::disable_server(const std::string &arg)
{
	for (auto &ss : serversides) {
		if (ss.name == arg)
			ss.disabled = 1;
	}
}

#include "api_serverside_command.h"

void
api_ss_conn::handle(const std::string &line)
{
	const struct api_serverside_command *cmd;

	if (ss.disabled && line.substr(0, 4) != "SENA") {
		be.write("EDIS\n");
		bufferevent_enable(be, EV_WRITE);
		return;
	}

	if (line.length() < 4) {
		warnx ("Short line: %s", line.c_str());
		return;
	}

	cmd = api_serverside_command(line.c_str(), 4);
	if (cmd)
		(this->*cmd->handler)(line.substr(4));
	else {
		warnx("Unknown command: %s", line.c_str());
		be.write("ECMD\n");
		bufferevent_enable(be, EV_WRITE);
	}
}

void
api_ss_conn::readcb()
{
	char *line;

	while ((line = evbuffer_readline(be->input))) {
		handle(line);
		free(line);
	}
}

void
api_ss_conn::writecb()
{
	bufferevent_disable (be, EV_WRITE);
}

void
api_ss_conn::errorcb(short what)
{
	if (what != (EVBUFFER_READ | EVBUFFER_EOF)) {
		/* Presume errno to still be up to date. */
		warn ("backend: read error: %d", what);
	}

	readcb();
	if (EVBUFFER_LENGTH(be->input))
		handle(std::string((const char*)EVBUFFER_DATA(be->input), EVBUFFER_LENGTH(be->input)));

	ss.conns.erase(std::find(ss.conns.begin(), ss.conns.end(), *this));
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
	: fd(fd), ss(ss), be(fd, std::bind(&api_ss_conn::readcb, this), std::bind(&api_ss_conn::writecb, this),
			std::bind(&api_ss_conn::errorcb, this, std::placeholders::_1))
{
	bufferevent_enable(be, EV_READ);
}

void
serverside_listen_fd(std::string name, backend_ptr &bdev, int fd)
{
	struct sockaddr_storage addr;
	socklen_t addrlen = sizeof(addr);

	if (getsockname(fd, (struct sockaddr*)&addr, &addrlen))
		err(1, "getsockname");

	serversides.emplace_back(std::move(name), bdev, (struct sockaddr*)&addr, addrlen, false, fd);
}

void
serverside_listen_local(std::string name, backend_ptr &bdev, const std::string &path) {
	struct sockaddr_storage addr;
	struct sockaddr_un *sun = (struct sockaddr_un *)&addr;
	int s;
	struct stat st;

	if (!lstat(path.c_str(), &st)) {
		if (!S_ISSOCK(st.st_mode))
			errx(1, "listen_local: %s exists but not socket", path.c_str());
		if (unlink(path.c_str()))
			err(1, "unlink");
	}

	sun->sun_family = AF_UNIX;
	strlcpy(sun->sun_path, path.c_str(), sizeof(sun->sun_path));

	s = socket(PF_LOCAL, SOCK_STREAM, 0);
	if (s < 0)
		err(1, "socket(PF_LOCAL)");

	if (bind(s, (struct sockaddr*)sun, sizeof(*sun)))
		err(1, "bind");
	/* Allow all by default. */
	chmod(path.c_str(), 0777);

	if (listen(s, 128))
		err(1, "listen");

	serversides.emplace_back(std::move(name), bdev, (struct sockaddr*)&addr, sizeof(*sun), true, s);
}

void
serverside_listen_tcp(std::string name, backend_ptr &bdev, const std::string &serv)
{
	const struct addrinfo hints = { .ai_flags = AI_PASSIVE, .ai_socktype = SOCK_STREAM };
	struct addrinfo *res = NULL, *curr;
	int r;

	if ((r = getaddrinfo(NULL, serv.c_str(), &hints, &res)))
		errx(1, "getaddrinfo(%s): %s", serv.c_str(), gai_strerror(r));

	for (curr = res ; curr ; curr = curr->ai_next) {
		int s;
		const int one = 1;

		s = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);
		if (s < 0)
			err(1, "socket(%s, %d)", serv.c_str(), curr->ai_family);

		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

		if (curr->ai_family == AF_INET6)
			setsockopt(s, IPPROTO_IPV6, IPV6_V6ONLY, &one, sizeof(one));

		if (bind(s, curr->ai_addr, curr->ai_addrlen))
			err(1, "bind(%s, %d)", serv.c_str(), curr->ai_family);

		if (listen(s, 128))
			err(1, "listen(%s, %d)", serv.c_str(), curr->ai_family);

		serversides.emplace_back(name, bdev, curr->ai_addr, curr->ai_addrlen, false, s);
	}

	freeaddrinfo(res);
}

serverside::serverside(std::string name, backend_ptr &bdev, const struct sockaddr *addr, socklen_t addrlen, bool should_unlink, int fd)
	: name(std::move(name)), fd(fd), bdev(bdev), addrlen(addrlen), should_unlink(should_unlink)
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
