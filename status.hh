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

#ifndef STATUS_H
#define STATUS_H

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#include <functional>
#include <memory>
#include <vector>

#define ESTART(e) enum status_ ## e {
#define EV(e, k, v) e ## _ ## k = v,
#define EEND(e) }; typedef enum status_ ## e status_ ## e ## _t;

#include "status_enums.h"

#undef ESTART
#undef EV
#undef EEND

#define STATUS_UNKNOWN -2

struct backend_output;
struct status;

typedef int status_int_t;
typedef std::string status_string_t;

class backend_ptr;

class status_notify_token
{
	struct status_notify_info &ptr;
	friend struct status;
public:
	status_notify_token(struct status_notify_info &ptr);
	~status_notify_token();
};

class status_ptr
{
	std::unique_ptr<status> status;
	friend struct status;

public:
	typedef std::function<void(const std::string &code, const std::string &val)> notify_cb;
	typedef std::function<struct status *(backend_ptr&)> creator;

	status_ptr(backend_ptr &bdev, const creator &creator);
	~status_ptr();

	int query_command(const std::string &code) const;
	int query_status(const std::string &code) const;

	int query(const std::string &code, std::string &out_buf);

	std::unique_ptr<status_notify_token> start_notify(const std::string &code, notify_cb cb);

	void status_setup();

	const char *packet_separators() const;
	void update_status(const std::string &packet, const struct backend_output *inptr);

	int send_status_request(const std::string &code);
	void send_command(const std::string &cmd, const std::vector<int32_t> &args);
};

#endif /*STATUS_H*/
