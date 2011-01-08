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

#include "cli.h"

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/uio.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <event.h>

#include "line.h"
#include "base64.h"
#include "complete.h"

#define COMMAND(x, y, z) {#x, y, z},
struct command
{
	const char *name;
	const char *code;
	int nargs;
} commands[] = {
#include "all_commands.h"
	{NULL}
};

int
open_local (const char *path) {
	struct sockaddr_un unaddr = {0};
	int fd = socket (PF_LOCAL, SOCK_STREAM, 0);

	if (fd < 0)
		return -1;

	unaddr.sun_family = AF_LOCAL;
	strncpy (unaddr.sun_path, path, sizeof (unaddr.sun_path) - 1);
	unaddr.sun_path[sizeof (unaddr.sun_path) - 1] = '\0';

	if (connect (fd, (struct sockaddr*)&unaddr, sizeof (unaddr))) {
		close (fd);
		return -1;
	}

	return fd;
}

static int
send_int_command (int fd, struct command *cmd, char **args, int nargs) {
	char pargs[nargs][4];
	struct iovec vecs[nargs + 3];
	int i;

	vecs[0].iov_base = (void*)"SEND";
	vecs[0].iov_len = 4;
	vecs[1].iov_base = (void*)cmd->code;
	vecs[1].iov_len = 4;
	for (i = 0 ; i < nargs ; i++) {
		int ia = atoi(args[i]);

		base64_int24(pargs[i], ia);

		vecs[i + 2].iov_base = pargs[i];
		vecs[i + 2].iov_len = 4;
	}
	vecs[i + 2].iov_base = (void*)"\n";
	vecs[i + 2].iov_len = 1;

	return writev(fd, vecs, nargs + 2);
}

static void
filter_candidates (int fd, struct complete_candidate **cands) {
	struct complete_candidate *cand, **pcand;
	struct evbuffer *buf = evbuffer_new();
	char *line, *l;

	evbuffer_add(buf, "QCMD", 4);
	for (cand = *cands ; cand ; cand = cand->next)
		evbuffer_add(buf, ((struct command*)cand->aux)->code, 4);
	evbuffer_add(buf, "\n", 1);
	write (fd, EVBUFFER_DATA(buf), EVBUFFER_LENGTH(buf));
	evbuffer_drain(buf, EVBUFFER_LENGTH(buf));

	while (!(line = evbuffer_readline(buf)))
		evbuffer_read(buf, fd, 1024);

	if (strncmp(line, "QCMD", 4) != 0)
		errx(1, "Unexpected reply: %s", line);

	/* Reply should be in same order as query. */
	pcand = cands;
	l = line + 4;
	while ((cand = *pcand)) {
		if (strncmp(((struct command*)cand->aux)->code, l, 4) == 0) {
			l += 4;
			pcand = &cand->next;
		} else {
			*pcand = cand->next;
			free(cand);
		}
	}
}

extern char *optarg;
extern int optind;
extern int optopt;

int
main (int argc, char *argv[]) {
	struct command *cmd;
	struct complete_candidate *cand, *candidates = NULL;
	int argi = 1;
	int res;
	int fd = -1;
	char opt;
	const char default_sock[] = "/tmp/morantz.sock";

	while ((opt = getopt(argc, argv, ":s:")) != -1) {
		switch (opt) {
		case 's':
			if (fd >= 0)
				err (1, "Only one -s or -d can be given.");
			fd = open_local (optarg);
			if (fd < 0)
				err (1, "open_local");
			break;
		case ':':
			err (1, "-%c requires an argument.", optopt);
		case '?':
			err (1, "unknown option -%c", optopt);
		}
	}
	argc -= optind - 1;
	argv += optind - 1;

	if (fd < 0) {
		fd = open_local (default_sock);
		if (fd < 0)
			err (1, "open_line");
	}

	if (argc > 1) {
		if (strcmp (argv[1], "listen") == 0)
			return cli_notify(fd, argc - 2, argv + 2, 0);
		if (strcmp (argv[1], "status") == 0)
			return cli_notify(fd, argc - 2, argv + 2, 1);
	}

	for (cmd = commands; cmd->name; cmd++) {
		cand = malloc (sizeof (*cand));
		if (!cand)
			err (1, "malloc");

		cand->name = cmd->name;
		cand->name_off = 0;
		cand->next = candidates;
		cand->aux = cmd;
		candidates = cand;
	}

	argi = 1 + complete(&candidates, argc - 1, (const char**)argv + 1, (void(*)(struct complete_candidate*))free);

	if (candidates)
		filter_candidates(fd, &candidates);

	if (!candidates) 
		errx (1, "No matching command");

	if (!candidates->next) {
 		if (argi == argc - ((struct command*)candidates->aux)->nargs) {
			res = send_int_command(fd, candidates->aux, argv + argi, argc - argi);
			if (res < 0)
				err (1, "send_int_command");
			return 0;
		}
		errx (1, "Command requires an argument.");
	}

	fprintf (stderr, "Multiple matches:\n");
	for (cand = candidates; cand; cand = cand->next) {
		fprintf (stderr, "%s\n", cand->name);
	}
	return 1;
}
