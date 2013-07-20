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

#include "backend.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <event.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <search.h>
#include <stdio.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include <forward_list>
#include <functional>
#include <list>
#include <sstream>
#include <string>

#include "status.h"
#include "line.h"
#include "api_serverside.h"
#include "smart_fd.hh"
#include "smart_event.hh"
#include "smart_evbuffer.hh"
#include "event_unhandled_exception.hh"

#include "backend_type.h"

class output_list
{
	std::forward_list<backend_output> list;
	decltype(list)::iterator send_iter = list.end();
	decltype(list)::iterator insert_iter = list.before_begin();

public:
	decltype(send_iter) to_send()
	{
		if (send_iter == list.end())
			return send_iter;
		return send_iter++;
	}

	const backend_output *inptr()
	{
		if (list.empty() || send_iter == list.begin())
			return NULL;
		return &list.front();
	}

	void push(backend_output o)
	{
		insert_iter = list.insert_after(insert_iter, std::move(o));
		if (send_iter == list.end())
			send_iter = insert_iter;
	}

	void pop()
	{
		if (insert_iter == list.begin())
			insert_iter = list.before_begin();
		list.pop_front();
	}

	void clear()
	{
		list.clear();
		send_iter = list.end();
		insert_iter = list.before_begin();
	}

	decltype(list.end()) end()
	{
		return list.end();
	}
};

struct backend_device {
	backend_ptr &ptr;

	std::string name;

	std::string line;
	smart_fd line_fd;
	smart_event<event_unhandled_exception::handle> read_ev;
	smart_event<event_unhandled_exception::handle> write_ev;

	smart_evbuffer input;
	output_list output;
	struct timeval out_throttle;

	std::string client;

	std::unique_ptr<status, decltype(&status_free)> status;

	backend_device(backend_ptr &ptr, std::string name, const struct status_dispatch *dispatch,
			std::string line, std::string client, int throttle);

	void open();
	void close();

	void listen(int fd);
	void listen_to_client();

	void send(const struct timeval *throttle, const char *fmt, va_list ap);
	void remove_output(const struct backend_output **inptr);

	static backend_device &impl(backend_ptr &ptr);
	static void create(std::string name, const struct status_dispatch *dispatch,
			std::string line, std::string client, int throttle);
private:
	void readcb(short what);
	void writecb();
};

std::list<backend_ptr> backends;

backend_device &
backend_device::impl(backend_ptr &ptr)
{
	return *ptr.bdev;
}

template <>
backend_ptr::backend_ptr()
{
}

void
backend_device::create(std::string name, const struct status_dispatch *dispatch,
			std::string line, std::string client, int throttle)
{
	backends.push_back(backend_ptr());
	backend_ptr &ptr = backends.back();
	ptr.bdev.reset(new backend_device(ptr, std::move(name), dispatch, std::move(line), std::move(client), throttle));
}

void
add_backend_device(const char *str) {
	int ms = 0;
	const char *next;

	next = strchr(str, ':');

	if (!next)
		errx (1, "No type for backend: %s", str);

	std::string name(str, next - str);

	str = next + 1;
	next = strchr(str, ':');
	if (!next)
		errx (1, "No path for backend: %s:%s", name.c_str(), str);

	std::string type(str, next - str);
	str = next + 1;
	next = strchr(str, ':');

	std::string path, client;
	if (next) {
		path = std::string(str, next - str);

		str = next + 1;
		next = strchr(str, ':');
		if (next) {
			client = std::string(str, next - str);
			ms = atoi(next + 1);
		} else {
			client = std::string(str);
		}
	} else {
		path = std::string(str);
	}

	const struct backend_type *bt = backend_type(type.c_str(), type.length());
	if (!bt)
		errx (1, "Unknown device type: %s", type.c_str());

	backend_device::create(name, bt->dispatch, path, client, ms);
}

backend_device::backend_device(backend_ptr &ptr, std::string name, const struct status_dispatch *dispatch, std::string line,
		std::string client, int throttle)
	: ptr(ptr), name(std::move(name)), line(std::move(line)), client(std::move(client)),
	status(status_create(dispatch), status_free)
{
	out_throttle.tv_sec = throttle / 1000;
	out_throttle.tv_usec = (throttle % 1000) * 1000;
}

void
backend_device::readcb(short what) {
	size_t len;

	int res = input.read(line_fd, 1024);
	if (res < 0)
		err (1, "evbuffer_read");
	if (res == 0)
		event_loopexit (NULL);

	while ((len = input.length())) {
		unsigned char *data = input.data();
		size_t i;

		for (i = 0 ; i < len ; i++) {
			if (strchr(status_dispatch(&*status)->packet_separators, data[i]))
				break;
		}
		if (i == len)
			break;

		if (i > 0) {
			data[i] = '\0';
			status_dispatch(&*status)->update_status(this, &*status, (char*)data, output.inptr());
		}
		input.drain(i + 1);
	}
}

void
backend_device::writecb() {
	auto out = output.to_send();

	if (out == output.end())
		return;

	if (write (line_fd, out->data, out->len) != out->len)
		err (1, "write");

	if (out->throttle.tv_sec > 0 || out->throttle.tv_usec > 0)
		write_ev.add(out->throttle);
	else
		write_ev.add(out_throttle);
}

void
backend_device::open()
{
	line_fd = open_line (line.c_str(), O_RDWR);
	if (!line_fd)
		err (1, "open_line");

	read_ev.set_fd(line_fd);
	read_ev.set(EV_READ | EV_PERSIST, std::bind(&backend_device::readcb, this, std::placeholders::_2));
	write_ev.set(EV_TIMEOUT, std::bind(&backend_device::writecb, this));

	if (read_ev.add())
		err (1, "event_add");
}

void
backend_device::close()
{
	read_ev.reset();
	write_ev.reset();
	input.reset();
	line_fd.close();

	output.clear();
}

void
backend_reopen_devices(void)
{
	for (auto &bdev : backends) {
		backend_device::impl(bdev).close();
		backend_device::impl(bdev).open();

		status_dispatch(bdev.status())->status_setup(&backend_device::impl(bdev), bdev.status());
	}
}

void
backend_device::listen(int fd)
{
	struct sockaddr_storage sst;
	socklen_t sstlen;
	std::string tag = name;

	sstlen = sizeof (sst);
	if (getsockname(fd, (struct sockaddr*)&sst, &sstlen) == 0) {
		if (sst.ss_family == AF_INET || sst.ss_family == AF_INET6) {
			char pbuf[NI_MAXSERV];

			getnameinfo((struct sockaddr*)&sst, sstlen, NULL, 0, pbuf, sizeof(pbuf), NI_NUMERICSERV);
			tag += ":";
			tag += pbuf;
		} else if (sst.ss_family == AF_UNIX) {
			struct sockaddr_un *sun = (struct sockaddr_un*)&sst;
			char path[256];

			if (sscanf(sun->sun_path, "/var/run/movactl.%s", path) == 1) {
				char *cp = strchr(path, '.');
				if (cp)
					*cp = '\0';
				tag = path;
			} else
				warnx("sscanf failed: %s", sun->sun_path);
		}
	}

	serverside_listen_fd(tag.c_str(), ptr, fd);
}

void
backend_listen_fd(const char *name, int fd)
{
	for (auto &bdev : backends) {
		if (name == backend_device::impl(bdev).name) {
			backend_device::impl(bdev).listen(fd);
			return;
		}
	}
	errx (1, "backend_listen: No matching device %s", name);
}

void
backend_device::listen_to_client()
{
	std::istringstream cst{client};
	std::string c;

	while (std::getline(cst, c, ',')) {
		size_t e;
		int p;

		if ((p = std::stoi(c, &e)) > 0 && p < 65536 && e == std::string::npos) {
			std::string tag = name + ":" + c;
			serverside_listen_tcp(tag.c_str(), ptr, c.c_str());
		} else {
			char path[256];
			std::string tag;

			if (sscanf(c.c_str(), "/var/run/movactl.%s.sock", path) == 1) {
				char *cp = strchr(path, '.');

				if (cp)
					*cp = '\0';
				tag = path;
			} else
				tag = name;
			serverside_listen_local(tag.c_str(), ptr, c.c_str());
		}
	}
}

void
backend_listen_all(void)
{
	for (auto &bdev : backends) {
		backend_device::impl(bdev).listen_to_client();
	}
}

void
backend_close_all(void)
{
	for (auto &bdev : backends) {
		backend_device::impl(bdev).close();
	}
}

void
backend_device::send(const struct timeval *throttle, const char *fmt, va_list ap)
{
	backend_output out;

	out.len = vasprintf(&out.data, fmt, ap);
	if (out.len < 0)
		err (1, "vasprintf");

	if (throttle)
		out.throttle = *throttle;
	else
		memset(&out.throttle, 0, sizeof (out.throttle));

	output.push(out);
}

void
backend_send(struct backend_device *bdev, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	bdev->send(NULL, fmt, ap);
	va_end(ap);
}

void
backend_send_throttle(struct backend_device *bdev, const struct timeval *throttle, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	bdev->send(throttle, fmt, ap);
	va_end(ap);
}

void
backend_device::remove_output(const struct backend_output **inptr)
{
	const struct backend_output *out = *inptr;

	if (out != output.inptr()) {
		if (out)
			throw 1;
		return;
	}

	free(out->data);
	output.pop();
	*inptr = output.inptr();
}

void
backend_remove_output(struct backend_device *bdev, const struct backend_output **inptr) {
	bdev->remove_output(inptr);
}

void
backend_ptr::send_command(const std::string &cmd, const std::vector<int32_t> &args)
{
	status_dispatch(status())->send_command(&*bdev, cmd.c_str(), args.size(), args.data());
}

struct status *
backend_ptr::status()
{
	return &*bdev->status;
}

void
backend_ptr::send_status_request(const std::string &code)
{
	 status_dispatch(status())->send_status_request(&*bdev, code.c_str());
}
