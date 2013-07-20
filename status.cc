/*
 * Copyright (c) 2008, 2011 Pelle Johansson
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

#include "status.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "line.h"
#include "base64.h"

#include <forward_list>
#include <string>

struct status_notify_info
{
	status &status;
	std::string code;
	status_ptr::notify_cb cb;

	status_notify_info(struct status &status, std::string code, status_ptr::notify_cb cb)
		: status(status), code(std::move(code)), cb(std::move(cb))
	{
	}

	bool
	operator == (const status_notify_info &r)
	{
		return this == &r;
	}
};

struct status
{
	backend_device &bdev;
	std::forward_list<status_notify_info> notify_chain;

	const struct status_dispatch *dispatch;

	void *device_specific = NULL;

public:
	status(backend_device &bdev, const struct status_dispatch *dispatch);
	~status();

	status_notify_token_t start_notify(const std::string &code, status_ptr::notify_cb cb);
	void stop_notify(status_notify_token_t token);
};

status_ptr::status_ptr(backend_device &bdev, const struct status_dispatch *dispatch)
	: status(new struct status(bdev, dispatch))
{
}

status::status(backend_device &bdev, const struct status_dispatch *dispatch)
	: bdev(bdev), dispatch(dispatch)
{
}

status::~status()
{
	/* XXX free notify chain */
}

void *
status_device_specific(const struct status *status)
{
	return status->device_specific;
}

void
status_set_device_specific(struct status *status, void *v)
{
	status->device_specific = v;
}

int
status_ptr::query_command(const std::string &code)
const
{
	return status->dispatch->query_command(&*status, code.c_str());
}

int
status_ptr::query_status(const std::string &code)
const
{
	return status->dispatch->query_status(&*status, code.c_str());
}

int
status_ptr::query(const std::string &code, std::string &out_buf)
{
	char buf[100];
	size_t len = sizeof(buf);

	int res = status->dispatch->query(&*status, code.c_str(), buf, &len);

	if (res == 0)
		out_buf = std::string(buf, len);
	return res;
}

status_notify_token_t
status::start_notify(const std::string &code, status_ptr::notify_cb cb)
{
	auto iter = notify_chain.emplace_after(notify_chain.before_begin(), *this, code, std::move(cb));

	return &*iter;
}

status_notify_token_t
status_ptr::start_notify(const std::string &code, notify_cb cb)
{
	return status->start_notify(code, std::move(cb));
}

void
status::stop_notify (status_notify_token_t token)
{
	notify_chain.remove(*token);
}

void
status_stop_notify(status_notify_token_t token)
{
	token->status.stop_notify(token);
}

void
status_notify_int (struct status *status, const char *code, int val) {
	char v[4];

	base64_int24(v, val);
	for (auto &notify : status->notify_chain) {
		if (code == notify.code)
			notify.cb(&notify, notify.code, v);
	}
}

void
status_notify_str (struct status *status, const char *code, const char *val, size_t len) {
	for (auto &notify : status->notify_chain) {
		if (code == notify.code)
			notify.cb(&notify, notify.code, std::string(val, len));
	}
}

status_ptr::~status_ptr()
{
}

void
status_ptr::status_setup()
{
	status->dispatch->status_setup(&status->bdev, &*status);
}

const char *
status_ptr::packet_separators()
const
{
	return status->dispatch->packet_separators;
}

void
status_ptr::update_status(const std::string &packet, const struct backend_output *inptr)
{
	status->dispatch->update_status(&status->bdev, &*status, packet.c_str(), inptr);
}

int
status_ptr::send_status_request(const std::string &code)
{
	return status->dispatch->send_status_request(&status->bdev, code.c_str());
}

void
status_ptr::send_command(const std::string &cmd, const std::vector<int32_t> &args)
{
	status->dispatch->send_command(&status->bdev, cmd.c_str(), args.size(), args.data());
}

