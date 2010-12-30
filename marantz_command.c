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

#include "line.h"

#define SIMPLE_COMMAND(name, code, arg) \
	int \
	ma_command_ ## name (int fd) { \
		int res = send_command (fd, code, arg); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#define SIGNINT_COMMAND(name, code, prefix) \
	int \
	ma_command_ ## name (int fd, int value) { \
		char valbuf[13]; \
		int res; \
 \
		snprintf(valbuf, sizeof(valbuf), prefix "%+d", value); \
		res = send_command (fd, code, valbuf); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#define UINT_COMMAND(name, code, prefix, width) \
	int \
	ma_command_ ## name (int fd, unsigned int value) { \
		char valbuf[13]; \
		int res; \
 \
		snprintf(valbuf, sizeof(valbuf), prefix "%0*u", width, value); \
		res = send_command (fd, code, valbuf); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#include "marantz_command.h"
