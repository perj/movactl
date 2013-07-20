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

struct backend_device;
struct backend_output;
struct status;

#define ESTART(e) enum status_ ## e {
#define EV(e, k, v) e ## _ ## k = v,
#define EEND(e) }; typedef enum status_ ## e status_ ## e ## _t;

#include "status_enums.h"

#undef ESTART
#undef EV
#undef EEND

#define STATUS_UNKNOWN -2

typedef int status_int_t;
typedef char *status_string_t;

struct status_dispatch {
	void (*status_setup)(struct backend_device *bdev, struct status *status);
	const char *packet_separators;
	void (*update_status)(struct backend_device *bdev, struct status *status, const char *packet, const struct backend_output *inptr);
	int (*send_status_request)(struct backend_device *bdev, const char *code);
	int (*query_command)(const struct status *status, const char *code);
	int (*query_status)(const struct status *status, const char *code);
	int (*query)(struct status *status, const char *code, void *buf, size_t *buflen);
	void (*send_command)(struct backend_device *bdev, const char *cmd, int narg, const int32_t *args);
};

typedef struct status_notify_info *status_notify_token_t;

#ifdef __cplusplus
extern "C" {
#endif

const struct status_dispatch *status_dispatch(const struct status *status);
void *status_device_specific(const struct status *status);
void status_set_device_specific(struct status *status, void *v);

void status_stop_notify (status_notify_token_t token);

void status_notify_int (struct status *status, const char *code, int val);
void status_notify_str (struct status *status, const char *code, const char *val, size_t len);

#ifdef __cplusplus
}

#include <functional>
#include <memory>

class status_ptr
{
	std::unique_ptr<status> status;
	friend struct status;

public:
	typedef std::function<void(status_notify_token_t token, const std::string &code, const std::string &val)> notify_cb;

	status_ptr(const struct status_dispatch *dispatch);
	~status_ptr();

	const struct status_dispatch *dispatch();
	struct status *get();
	const struct status *get() const;

	int query_command(const std::string &code) const;
	int query_status(const std::string &code) const;

	int query(const std::string &code, std::string &out_buf);

	status_notify_token_t start_notify(const std::string &code, notify_cb cb);
};

#endif

#endif /*STATUS_H*/
