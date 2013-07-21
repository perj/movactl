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
#include "status_private.hh"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "line.h"
#include "base64.h"

#include <forward_list>
#include <string>

status_ptr::status_ptr(backend_device &bdev, const creator &creator)
	: status(creator(bdev))
{
}

status::status(backend_device &bdev)
	: bdev(bdev)
{
}

status::~status()
{
	/* XXX free notify chain */
}

int
status_ptr::query_command(const std::string &code)
const
{
	return status->query_command(code);
}

int
status_ptr::query_status(const std::string &code)
const
{
	return status->query_status(code);
}

int
status_ptr::query(const std::string &code, std::string &out_buf)
{
	return status->query(code, out_buf);
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
status::notify(const std::string &code, int val)
{
	char v[4];

	base64_int24(v, val);
	for (auto &notify : notify_chain) {
		if (code == notify.code)
			notify.cb(&notify, notify.code, v);
	}
}

void
status::notify(const std::string &code, const std::string &val)
{
	for (auto &notify : notify_chain) {
		if (code == notify.code)
			notify.cb(&notify, notify.code, val);
	}
}

status_ptr::~status_ptr()
{
}

void
status_ptr::status_setup()
{
	status->status_setup();
}

const char *
status_ptr::packet_separators()
const
{
	return status->packet_separators();
}

void
status_ptr::update_status(const std::string &packet, const struct backend_output *inptr)
{
	status->update_status(packet, inptr);
}

int
status_ptr::send_status_request(const std::string &code)
{
	return status->send_status_request(code);
}

void
status_ptr::send_command(const std::string &cmd, const std::vector<int32_t> &args)
{
	status->send_command(cmd, args);
}

