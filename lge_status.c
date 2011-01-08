/*
 * Copyright (c) 2011 Pelle Johansson
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

#include "lge_status.h"
#include "status.h"
#include "backend.h"

#include <stdlib.h>
#include <string.h>
#include <err.h>

void
lge_setup_status (struct backend_device *bdev, struct status *status) {
}

void
lge_update_status (struct backend_device *bdev, struct status *status, const char *line) {
}

int
lge_send_status_request(struct backend_device *bdev, const char *code) {
	return -1;
}

int
lge_query (struct status *status, const char *code, void *buf, size_t *buflen) {
	/* Not really supported (but could use a cache) */
	return STATUS_UNKNOWN;
}

const struct lge_command {
	const char *cmd;
	const char *fmt;
	int narg;
	int split;
} lge_commands[] = {
#define SIMPLE_COMMAND(name, cmd, code, arg) { cmd, code " 00 " arg "\r", 0, 0 },
#define UINT_COMMAND(name, cmd, code) { cmd, code " 00 %02X\r", 1, 0 },
#define UINT2_SUFF_COMMAND(name, cmd, code, suff) { cmd, code " 00 %02X %02X " suff "\r", 1, 1 },
#include "lge_command.h"
	{ NULL }
};

void
lge_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args) {
	const struct lge_command *lgecmd;

	for (lgecmd = lge_commands ; lgecmd->cmd ; lgecmd++) {
		if (strcmp(cmd, lgecmd->cmd) == 0) {
			break;
		}
	}
	if (!lgecmd->cmd) {
		warnx("No such command: %s", cmd);
		return;
	}
	if (narg != lgecmd->narg) {
		warnx("Mismatch number of arguments %d <> %d", narg, lgecmd->narg);
		return;
	}
	if (narg == 0)
		backend_send(bdev, lgecmd->fmt, "" /* Suppress warning */);
	else if (lgecmd->split)
		backend_send(bdev, lgecmd->fmt, (int)args[0] / 256, (int)args[0] & 255);
	else
		backend_send(bdev, lgecmd->fmt, (int)args[0]);
}

struct status_dispatch lge_dispatch = {
	lge_setup_status,
	"x",
	lge_update_status,
	lge_send_status_request,
	lge_query,
	lge_send_command,
};
