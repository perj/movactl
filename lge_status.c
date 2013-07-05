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

struct lge_notify
{
	const char *code;
	const char *cmd;
	void (*func)(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg);
};

#define UPDATE_FUNC_BOOL(field) \
	static void \
	update_ ## field(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg) { \
		if (ok) \
			status_notify_int(status, lgenot->code, atoi(arg)); \
		else \
			status_notify_int(status, lgenot->code, -1); \
	}

#define UPDATE_FUNC_INT(field) \
	static void \
	update_ ## field(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg) { \
		if (ok) \
			status_notify_int(status, lgenot->code, strtol(arg, NULL, 16)); \
		else \
			status_notify_int(status, lgenot->code, INT_MIN); \
	}

UPDATE_FUNC_BOOL(power);

UPDATE_FUNC_INT (aspect_ratio)
UPDATE_FUNC_INT (video_mute)
UPDATE_FUNC_BOOL(audio_mute);

UPDATE_FUNC_INT (volume)

UPDATE_FUNC_INT (contrast)
UPDATE_FUNC_INT (brightness)
UPDATE_FUNC_INT (colour)
UPDATE_FUNC_INT (tint)
UPDATE_FUNC_INT (sharpness)

UPDATE_FUNC_BOOL (osd)
UPDATE_FUNC_BOOL (remote_control_lock)

UPDATE_FUNC_INT (treble)
UPDATE_FUNC_INT (bass)
UPDATE_FUNC_INT (balance)

UPDATE_FUNC_INT (colour_temperature)

UPDATE_FUNC_INT (energy_saving)

static void
update_tv_band(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg) {
	if (ok)
		status_notify_int(status, lgenot->code, arg[4] - '0');
	else
		status_notify_int(status, lgenot->code, -1);
}

static void
update_tv_channel(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg) {
	if (ok) {
		int val = 0;
		int i;

		for (i = 0 ; i < 4 ; i++) {
			val <<= 4;
			if (arg[i] >= '0' && arg[i] <= '9')
				val += arg[i] - '0';
			else if (arg[i] >= 'A' && arg[i] <= 'F')
				val += arg[i] - 'A';
			else if (arg[i] >= 'a' && arg[i] <= 'f')
				val += arg[i] - 'a';
		}
		status_notify_int(status, lgenot->code, val);
	} else
		status_notify_int(status, lgenot->code, -1);
}

UPDATE_FUNC_BOOL (programme_add)

UPDATE_FUNC_INT (back_light)

static void
update_source(struct status *status, const struct lge_notify *lgenot, int ok, const char *arg) {
	if (ok) {
		int val = 0x100 + strtol(arg, NULL, 16);

		if (val >= 0x1A0 && val <= 0x1AF) {
			/* For some reason this shifts. */
			val -= 0x10;
		}
		status_notify_int(status, lgenot->code, val);
	} else
		status_notify_int(status, lgenot->code, -1);
}

const struct lge_notify lge_notifies[] = {
#define NOTIFY(name, code, cmd, type) { code, cmd, update_ ## name },
#include "lge_notify.h"
#undef NOTIFY
	{ NULL }
};

void
lge_update_status (struct backend_device *bdev, struct status *status, const char *line,
		struct backend_output **inptr, struct backend_output ***outptr) {
	const struct lge_notify *lgenot;
	char cmd[3];
	int ok;

	while (*inptr != **outptr && ((**inptr).len < 2 || (**inptr).data[1] != line[0]))
		backend_remove_output(bdev, inptr);
	if (*inptr == **outptr) {
		warnx("No output match for %s", line);
		return;
	}

	cmd[0] = (**inptr).data[0];
	backend_remove_output(bdev, inptr);

	warnx("Read packet %c%s", cmd[0], line);

	cmd[1] = line[0];
	cmd[2] = '\0';

	if (strlen(line) < sizeof ("x 01 ")) {
		warnx("Invalid packet");
		return;
	}

	line += sizeof ("x 01 ") - 1;
	if (strncmp(line, "OK", 2) == 0)
		ok = 1;
	else if (strncmp(line, "NG", 2) == 0)
		ok = 0;
	else {
		warnx("Invalid OK/NG");
		return;
	}
	line += 2;
	
	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (strncmp(cmd, lgenot->cmd, 2) == 0) {
			lgenot->func(status, lgenot, ok, line);
		}
	}
}

int
lge_send_status_request(struct backend_device *bdev, const char *code) {
	const struct lge_notify *lgenot;

	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (strncmp(code, lgenot->code, 4) == 0) {
			backend_send(bdev, "%s 00 FF\r", lgenot->cmd);
			return 0;
		}
	}
	warnx("send_status_request(%s)", code);
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

int
lge_query_command(struct status *status, const char *code) {
	const struct lge_command *lgecmd;

	for (lgecmd = lge_commands ; lgecmd->cmd ; lgecmd++) {
		if (strncmp(code, lgecmd->cmd, 4) == 0) {
			return 0;
		}
	}
	return -1;
}

int
lge_query_status(struct status *status, const char *code) {
	const struct lge_notify *lgenot;

	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (strncmp(code, lgenot->code, 4) == 0) {
			return 0;
		}
	}
	return -1;
}

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
	lge_query_command,
	lge_query_status,
	lge_query,
	lge_send_command,
};
