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
#include "marantz_status.h"

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

#include "line.h"
#include "base64.h"

struct status_notify_info
{
	void *status;
	char *code;
	status_notify_cb_t cb;
	void *cbarg;

	struct status_notify_info *next;
};

int
status_query(struct status *status, const char *code, char *buf, size_t *len) {
	return status->dispatch->query(status, code, buf, len);
}

status_notify_token_t
status_start_notify (struct status *status, const char *code,
		status_notify_cb_t cb,
		void *cbarg) {
	struct status_notify_info *info = malloc (sizeof (*info));

	if (!info)
		return NULL;
	info->code = strdup (code);
	if (!info->code) {
		free (info);
		return NULL;
	}
	info->status = status;
	info->cb = cb;
	info->cbarg = cbarg;
	info->next = status->notify_chain;
	status->notify_chain = info;

	return info;
}

void
status_stop_notify (status_notify_token_t token) {
	struct status_notify_info *info = token;
	struct status_notify_info *pinfo;
	struct status *status = info->status;

	if (status->notify_chain == info)
		status->notify_chain = info->next;
	else {
		for (pinfo = status->notify_chain; pinfo && pinfo->next != info; pinfo = pinfo->next)
			;
		if (!pinfo)
			return;
		pinfo->next = info->next;
	}
	free (info->code);
	free (info);
}

void
status_notify_int (struct status *status, const char *code, int val) {
	char v[4];
	struct status_notify_info *notify;

	base64_int24(v, val);
	for (notify = status->notify_chain; notify; notify = notify->next) {
		if (strcmp (code, notify->code) == 0) {
			notify->cb (status, notify, notify->code, notify->cbarg, v, 4);
		}
	}
}

void
status_notify_str (struct status *status, const char *code, const char *val, size_t len) {
	struct status_notify_info *notify;

	for (notify = status->notify_chain; notify; notify = notify->next) {
		if (strcmp (code, notify->code) == 0) {
			notify->cb (status, notify, notify->code, notify->cbarg, val, len);
		}
	}
}
