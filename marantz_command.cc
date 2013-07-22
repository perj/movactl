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

#include <stdio.h>
#include <string.h>
#include <err.h>

#include "marantz_status.h"
#include "backend.h"

#define THROTTLED_COMMAND(name, code, arg, s, ms) \
{ code arg, "@" code ":" arg "\r", 0, { .tv_sec = s, .tv_usec = ms * 1000 } },

#define SIMPLE_COMMAND(name, code, arg) \
{ code arg, "@" code ":" arg "\r", 0 },

#define SIGNINT_COMMAND(name, code, prefix) \
{ code prefix, "@" code ":" prefix "%+d\r",  1},

#define UINT_COMMAND(name, code, prefix, width) \
{ code prefix, "@" code ":" prefix "%0" #width "u\r", 1 },

const struct ma_command {
	const char *cmd;
	const char *fmt;
	size_t narg;
	struct timeval throttle;
} ma_commands[] = {
#include "marantz_command.h"
	{ NULL }
};

int
ma_status::query_command (const std::string &code)
const
{
	const struct ma_command *macmd;

	for (macmd = ma_commands ; macmd->cmd ; macmd++) {
		if (code == macmd->cmd) {
			return 0;
		}
	}
	return -1;
}

void
ma_status::send_command(const std::string &cmd, const std::vector<int32_t> &args)
{
	const struct ma_command *macmd;

	for (macmd = ma_commands ; macmd->cmd ; macmd++) {
		if (cmd == macmd->cmd) {
			break;
		}
	}
	if (!macmd->cmd) {
		warnx("No such command: %s", cmd.c_str());
		return;
	}
	if (args.size() != macmd->narg) {
		warnx("Mismatch number of arguments %zd <> %zd", args.size(), macmd->narg);
		return;
	}
	if (args.size() == 1)
		send_throttle(&macmd->throttle, macmd->fmt, (int)args[0]);
	else
		send_throttle(&macmd->throttle, macmd->fmt, "" /* Suppress warning */);
}

