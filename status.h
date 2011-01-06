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

#ifndef STATUS_H
#define STATUS_H

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

struct backend_device;
struct status;

#define ESTART(e) enum status_ ## e {
#define EV(e, k, v) e ## _ ## k = v,
#define EEND(e) }; typedef enum status_ ## e status_ ## e ## _t;

#include "status_enums.h"

#undef ESTART
#undef EV
#undef EEND

typedef int status_int_t;
typedef char *status_string_t;

struct status_dispatch {
	void (*status_setup)(struct backend_device *bdev, struct status *status);
	const char *packet_separators;
	void (*update_status)(struct backend_device *bdev, struct status *status, const char *packet);
	int (*send_status_request)(struct backend_device *bdev, const char *code);
	int (*status_serialize)(struct status *status, const char *code, void *buf, size_t *buflen);
	void (*send_command)(struct backend_device *bdev, const char *cmd, int narg, int32_t *args);
};

struct status
{
	struct status_notify_info *notify_chain;

	const struct status_dispatch *dispatch;

	void *device_specific;
};

typedef struct status_notify_info *status_notify_token_t;

typedef void (*status_notify_cb_t)(struct status *status, status_notify_token_t token, const char *code, void *cbarg,
		void *data, size_t len);

status_notify_token_t status_notify (struct status *status, const char *code, status_notify_cb_t cb, void *cbarg);
void status_stop_notify (status_notify_token_t token);

#endif /*STATUS_H*/
