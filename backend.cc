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
#include <string>

#include "status.h"
#include "line.h"
#include "api_serverside.h"
#include "smart_fd.hh"
#include "smart_event.hh"
#include "smart_evbuffer.hh"

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
	std::string name;

	std::string line;
	smart_fd line_fd;
	smart_event read_ev;
	smart_event write_ev;

	smart_evbuffer input;
	output_list output;
	struct timeval out_throttle;

	std::string client;

	struct status status;

	backend_device(std::string name, const struct status_dispatch *dispatch, std::string line, std::string client, int throttle);

	void open();
	void close();

private:
	void readcb(short what);
	void writecb();
};

std::list<backend_device> backends;

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

	backends.emplace_back(name, bt->dispatch, path, client, ms);
}

backend_device::backend_device(std::string name, const struct status_dispatch *dispatch, std::string line,
		std::string client, int throttle)
	: name(std::move(name)), line(std::move(line)), client(std::move(client))
{
	status.dispatch = dispatch;
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
			if (strchr(status.dispatch->packet_separators, data[i]))
				break;
		}
		if (i == len)
			break;

		if (i > 0) {
			data[i] = '\0';
			status.dispatch->update_status(this, &status, (char*)data, output.inptr());
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
		bdev.close();
		bdev.open();

		bdev.status.dispatch->status_setup(&bdev, &bdev.status);
	}
}

void
backend_listen_fd (const char *name, int fd) {
	decltype(backends)::iterator bdev;
	struct sockaddr_storage sst;
	socklen_t sstlen;
	char tag[32];

	for (bdev = backends.begin() ; bdev != backends.end() ; bdev++) {
		if (name == bdev->name)
			break;
	}
	if (bdev == backends.end())
		errx (1, "backend_listen: No matching device %s", name);

	strlcpy(tag, bdev->name.c_str(), sizeof(tag));

	sstlen = sizeof (sst);
	if (getsockname(fd, (struct sockaddr*)&sst, &sstlen) == 0) {
		if (sst.ss_family == AF_INET || sst.ss_family == AF_INET6) {
			char pbuf[NI_MAXSERV];

			getnameinfo((struct sockaddr*)&sst, sstlen, NULL, 0, pbuf, sizeof(pbuf), NI_NUMERICSERV);
			strlcat(tag, ":", sizeof(tag));
			strlcat(tag, pbuf, sizeof(tag));
		} else if (sst.ss_family == AF_UNIX) {
			struct sockaddr_un *sun = (struct sockaddr_un*)&sst;
			char path[256];
			
			if (sscanf(sun->sun_path, "/var/run/movactl.%s", path) == 1) {
				char *cp = strchr(path, '.');
				if (cp)
					*cp = '\0';
				strlcpy(tag, path, sizeof(tag));
			} else
				warnx("sscanf failed: %s", sun->sun_path);
		}
	}

	serverside_listen_fd(tag, &*bdev, fd);
}

void
backend_listen_all (void)
{
	for (auto bdev = backends.begin() ; bdev != backends.end() ; bdev++) {
		char *e = NULL;
		int p;
		char *c, *client, *cl;

		if (bdev->client.empty())
			continue;

		cl = client = strdup(bdev->client.c_str());
		if (!client)
			err(1, "backend_listen_all");

		while ((c = strsep(&client, ","))) {
			if ((p = strtol(c, &e, 0)) > 0 && p < 65536 && e && *e == '\0') {
				char tag[32];

				snprintf(tag, sizeof(tag), "%s:%s", bdev->name.c_str(), c);
				serverside_listen_tcp(tag, &*bdev, c);
			} else {
				char path[256];
				char tag[32];

				if (sscanf(c, "/var/run/movactl.%s.sock", path) == 1) {
					char *cp = strchr(path, '.');

					if (cp)
						*cp = '\0';
					strlcpy(tag, path, sizeof(tag));
				} else
					strlcpy(tag, bdev->name.c_str(), sizeof(tag));
				serverside_listen_local(tag, &*bdev, c);
			}
		}
		free(cl);
	}
}

void
backend_close_all (void)
{
	for (auto bdev = backends.begin() ; bdev != backends.end() ; bdev++) {
		bdev->close();
	}
}

static void
backend_send_impl(struct backend_device *bdev, const struct timeval *throttle, const char *fmt, va_list ap) {
	backend_output out;

	out.len = vasprintf(&out.data, fmt, ap);
	if (out.len < 0)
		err (1, "vasprintf");

	if (throttle)
		out.throttle = *throttle;
	else
		memset(&out.throttle, 0, sizeof (out.throttle));

	bdev->output.push(out);
}

void
backend_send(struct backend_device *bdev, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	backend_send_impl(bdev, NULL, fmt, ap);
	va_end(ap);
}

void
backend_send_throttle(struct backend_device *bdev, const struct timeval *throttle, const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	backend_send_impl(bdev, throttle, fmt, ap);
	va_end(ap);
}

void
backend_remove_output(struct backend_device *bdev, const struct backend_output **inptr) {
	const struct backend_output *out = *inptr;

	if (out != bdev->output.inptr()) {
		if (out)
			throw 1;
		return;
	}

	free(out->data);
	bdev->output.pop();
	*inptr = bdev->output.inptr();
}

void
backend_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args) {
	bdev->status.dispatch->send_command(bdev, cmd, narg, args);
}

struct status *
backend_get_status(struct backend_device *bdev) {
	return &bdev->status;
}

void
backend_send_status_request(struct backend_device *bdev, const char *code) {
	 bdev->status.dispatch->send_status_request(bdev, code);
}
