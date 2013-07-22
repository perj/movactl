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

#include "status.hh"
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

status_notify_token::status_notify_token(struct status_notify_info &ptr)
	: ptr(ptr)
{
}

status_notify_token::~status_notify_token()
{
	ptr.status.stop_notify(ptr);
}

status::status(backend_ptr &ptr, std::string name, std::string line, std::string client, int throttle)
	: backend_device(ptr, std::move(name), std::move(line), std::move(client), throttle)
{
}

status::~status()
{
}

std::unique_ptr<status_notify_token>
status::start_notify(const std::string &code, backend_ptr::notify_cb cb)
{
	auto iter = notify_chain.emplace_after(notify_chain.before_begin(), *this, code, std::move(cb));

	return std::unique_ptr<status_notify_token>(new status_notify_token(*iter));
}

void
status::stop_notify (struct status_notify_info &ptr)
{
	notify_chain.remove(ptr);
}

void
status::notify(const std::string &code, int val)
{
	char v[4];

	base64_int24(v, val);
	for (auto &notify : notify_chain) {
		if (code == notify.code)
			notify.cb(notify.code, v);
	}
}

void
status::notify(const std::string &code, const std::string &val)
{
	for (auto &notify : notify_chain) {
		if (code == notify.code)
			notify.cb(notify.code, val);
	}
}

