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

#ifndef MARANTZ_STATUS_H
#define MARANTZ_STATUS_H

#include <limits.h>
#include <stdint.h>
#include <sys/types.h>

#include "status.h"

#define MAVOL_MIN INT_MIN

struct backend_device;

enum {
#define NOTIFY(name, code, type) ST_KNOW_BIT_ ## name,
#define STATUS(name, code, type) /* can't know */
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS
#define NOTIFY(name, code, type) ST_KNOW_ ## name = 1LL << ST_KNOW_BIT_ ## name,
#define STATUS(name, code, type) /* can't know */
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS
};

struct ma_status
{
#define NOTIFY(name, code, type) status_ ## type ## _t name;
#define STATUS(name, code, type) status_ ## type ## _t name;
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS

	status_bool_t auto_status_feedback_layer[4];

	uint64_t known_fields;
};

extern struct status_dispatch marantz_dispatch;

int marantz_query_command(const struct status *status, const char *code);
void marantz_send_command(struct backend_device *bdev, const char *cmd, int narg, const int32_t *args);

#endif /*MARANTZ_STATUS_H*/
