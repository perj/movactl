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

#include "status.hh"
#include "status_private.hh"
#include "lge_status.hh"
#include "backend.h"

#include <stdlib.h>
#include <string.h>
#include <err.h>

class lge_status : public status
{
public:
	lge_status(backend_ptr &bdev);

	virtual void status_setup();
	virtual const char *packet_separators() const;
	virtual void update_status(const std::string &packet, const struct backend_output *inptr);
	virtual int send_status_request(const std::string &code);
	virtual int query_command(const std::string &code) const;
	virtual int query_status(const std::string &code) const;
	virtual int query(const std::string &code, std::string &out_buf);
	virtual void send_command(const std::string &cmd, const std::vector<int32_t> &args);

#define NOTIFY(name, code, cmd, type) void update_ ## name(const struct lge_notify *lgenot, int ok, const std::string &arg);
#include "lge_notify.h"
#undef NOTIFY
};

lge_status::lge_status(backend_ptr &bdev)
	: status(bdev)
{
}

const char *
lge_status::packet_separators()
const
{
	return "x";
}

void
lge_status::status_setup()
{
}

struct lge_notify
{
	const char *code;
	const char *cmd;
	void (lge_status::*func)(const struct lge_notify *lgenot, int ok, const std::string &arg);
};

#define UPDATE_FUNC_BOOL(field) \
	void \
	lge_status::update_ ## field(const struct lge_notify *lgenot, int ok, const std::string &arg) { \
		if (ok) \
			notify(lgenot->code, std::stoi(arg)); \
		else \
			notify(lgenot->code, -1); \
	}

#define UPDATE_FUNC_INT(field) \
	void \
	lge_status::update_ ## field(const struct lge_notify *lgenot, int ok, const std::string &arg) { \
		if (ok) \
			notify(lgenot->code, std::stoi(arg, NULL, 16)); \
		else \
			notify(lgenot->code, INT_MIN); \
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

void
lge_status::update_tv_band(const struct lge_notify *lgenot, int ok, const std::string &arg) {
	if (ok)
		notify(lgenot->code, arg[4] - '0');
	else
		notify(lgenot->code, -1);
}

void
lge_status::update_tv_channel(const struct lge_notify *lgenot, int ok, const std::string &arg) {
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
		notify(lgenot->code, val);
	} else
		notify(lgenot->code, -1);
}

UPDATE_FUNC_BOOL (programme_add)

UPDATE_FUNC_INT (back_light)

void
lge_status::update_source(const struct lge_notify *lgenot, int ok, const std::string &arg) {
	if (ok) {
		int val = 0x100 + std::stoi(arg, NULL, 16);

		if (val >= 0x1A0 && val <= 0x1AF) {
			/* For some reason this shifts. */
			val -= 0x10;
		}
		notify(lgenot->code, val);
	} else
		notify(lgenot->code, -1);
}

const struct lge_notify lge_notifies[] = {
#define NOTIFY(name, code, cmd, type) { code, cmd, &lge_status::update_ ## name },
#include "lge_notify.h"
#undef NOTIFY
	{ NULL }
};

void
lge_status::update_status(const std::string &line, const struct backend_output *inptr)
{
	const struct lge_notify *lgenot;
	char cmd[3];
	int ok;

	while (inptr && (inptr->len < 2 || inptr->data[1] != line[0]))
		bdev.remove_output(&inptr);
	if (!inptr) {
		warnx("No output match for %s", line.c_str());
		return;
	}

	cmd[0] = inptr->data[0];
	bdev.remove_output(&inptr);

	//warnx("Read packet %c%s", cmd[0], line.c_str());

	cmd[1] = line[0];
	cmd[2] = '\0';

	if (line.length() < sizeof ("x 01 ")) {
		warnx("Invalid packet");
		return;
	}

	std::string ack = line.substr(sizeof("x 01 ") - 1, 2);
	if (ack == "OK")
		ok = 1;
	else if (ack == "NG")
		ok = 0;
	else {
		warnx("Invalid OK/NG");
		return;
	}
	std::string arg = line.substr(sizeof("x 01 ") + 1);

	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (strncmp(cmd, lgenot->cmd, 2) == 0) {
			(this->*lgenot->func)(lgenot, ok, arg);
		}
	}
}

int
lge_status::send_status_request(const std::string &code)
{
	const struct lge_notify *lgenot;

	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (code == lgenot->code) {
			bdev.send("%s 00 FF\r", lgenot->cmd);
			return 0;
		}
	}
	warnx("send_status_request(%s)", code.c_str());
	return -1;
}

int
lge_status::query(const std::string &code, std::string &out_buf)
{
	/* Not really supported (but could use a cache) */
	return STATUS_UNKNOWN;
}

const struct lge_command {
	const char *cmd;
	const char *fmt;
	size_t narg;
	int split;
	struct timeval throttle;
} lge_commands[] = {
#define THROTTLED_COMMAND(name, cmd, code, arg, s, ms) { cmd, code " 00 " arg "\r", 0, 0, { .tv_sec = s, .tv_usec = ms * 1000 } },
#define SIMPLE_COMMAND(name, cmd, code, arg) { cmd, code " 00 " arg "\r", 0, 0 },
#define UINT_COMMAND(name, cmd, code) { cmd, code " 00 %02X\r", 1, 0 },
#define UINT2_SUFF_COMMAND(name, cmd, code, suff) { cmd, code " 00 %02X %02X " suff "\r", 1, 1 },
#include "lge_command.h"
	{ NULL }
};

int
lge_status::query_command(const std::string &code)
const
{
	const struct lge_command *lgecmd;

	for (lgecmd = lge_commands ; lgecmd->cmd ; lgecmd++) {
		if (code == lgecmd->cmd) {
			return 0;
		}
	}
	return -1;
}

int
lge_status::query_status(const std::string &code)
const
{
	const struct lge_notify *lgenot;

	for (lgenot = lge_notifies ; lgenot->code ; lgenot++) {
		if (code == lgenot->code) {
			return 0;
		}
	}
	return -1;
}

void
lge_status::send_command(const std::string &cmd, const std::vector<int32_t> &args)
{
	const struct lge_command *lgecmd;

	for (lgecmd = lge_commands ; lgecmd->cmd ; lgecmd++) {
		if (cmd == lgecmd->cmd) {
			break;
		}
	}
	if (!lgecmd->cmd) {
		warnx("No such command: %s", cmd.c_str());
		return;
	}
	if (args.size() != lgecmd->narg) {
		warnx("Mismatch number of arguments %zd <> %zd", args.size(), lgecmd->narg);
		return;
	}
	if (args.size() == 0)
		bdev.send_throttle(&lgecmd->throttle, lgecmd->fmt, "" /* Suppress warning */);
	else if (lgecmd->split)
		bdev.send_throttle(&lgecmd->throttle, lgecmd->fmt, (int)args[0] / 256, (int)args[0] & 255);
	else
		bdev.send_throttle(&lgecmd->throttle, lgecmd->fmt, (int)args[0]);
}

struct status *lge_creator(backend_ptr &bdev)
{
	return new lge_status(bdev);
}

