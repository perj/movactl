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
#include <glob.h>
#include <limits.h>
#include <syslog.h>
#include <errno.h>

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

int
open_remote (const char *host, const char *port) {
	const struct addrinfo hints = { .ai_socktype = SOCK_STREAM };
	struct addrinfo *res, *curr;
	int r;
	int fd = -1;

	if ((r = getaddrinfo(host, port, &hints, &res)))
		errx(1, "getaddrinfo(%s, %s): %s", host, port, gai_strerror(r));

	for (curr = res ; curr ; curr = curr->ai_next) {
		fd = socket(curr->ai_family, curr->ai_socktype, curr->ai_protocol);

		if (fd < 0)
			continue;

		if (connect(fd, curr->ai_addr, curr->ai_addrlen)) {
			close(fd);
			fd = -1;
			continue;
		}

		break;
	}
	freeaddrinfo(res);

	return fd;
}

int
open_local_or_remote (const char *path) {
	char linkdst[PATH_MAX + 1];
	char *cp;

	int r = readlink(path, linkdst, PATH_MAX);
	if (r == -1) {
		if (errno == EINVAL)
			return open_local(path);
		return -1;
	}

	linkdst[r] = '\0';

	if (linkdst[0] == '.' || strchr(linkdst, '/'))
		return open_local(path);

	cp = strchr(linkdst, ':');
	if (!cp)
		return open_local(path);

	*cp++ = '\0';
	return open_remote(linkdst, cp);
}

static int
send_int_command (fd_set *line_set, int maxfd, struct command *cmd, char **args, int nargs) {
	char pargs[nargs][4];
	struct iovec vecs[nargs + 3];
	int i;
	int fd;

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

	for (fd = 0 ; fd <= maxfd ; fd++) {
		if (FD_ISSET(fd, line_set))
			writev(fd, vecs, i + 2);
	}
	return 0;
}

static int
set_server_enabled (fd_set *line_set, int maxfd, int enable, const char *server) {
	struct iovec vecs[3];
	int fd;

	vecs[0].iov_base = (void*)(enable ? "SENA" : "SDIS");
	vecs[0].iov_len = 4;
	vecs[1].iov_base = (void*)server;
	vecs[1].iov_len = strlen(server);
	vecs[2].iov_base = (void*)"\n";
	vecs[2].iov_len = 1;

	for (fd = 0 ; fd <= maxfd ; fd++) {
		if (FD_ISSET(fd, line_set))
			writev(fd, vecs, 3);
	}
	return 0;
}

static void
filter_candidates (fd_set *line_set, int maxfd, struct complete_candidate **cands) {
	struct complete_candidate *cand, **pcand;
	struct evbuffer *buf = evbuffer_new();
	char *lines[maxfd + 1], *l[maxfd + 1];
	int i, nlines = 0;

	evbuffer_add(buf, "QCMD", 4);
	for (cand = *cands ; cand ; cand = cand->next) {
		if (cand->aux)
			evbuffer_add(buf, ((struct command*)cand->aux)->code, 4);
	}
	if (EVBUFFER_LENGTH(buf) <= 4) {
		evbuffer_free(buf);
		return;
	}
	evbuffer_add(buf, "\n", 1);

	for (i = 0 ; i <= maxfd ; i++) {
		if (FD_ISSET(i, line_set))
			write (i, EVBUFFER_DATA(buf), EVBUFFER_LENGTH(buf));
	}

	for (i = 0 ; i <= maxfd ; i++) {
		if (FD_ISSET(i, line_set)) {
			evbuffer_drain(buf, EVBUFFER_LENGTH(buf));
			while (!(lines[nlines] = evbuffer_readline(buf)))
				evbuffer_read(buf, i, 1024);
			if (strncmp(lines[nlines], "QCMD", 4) != 0 && strncmp(lines[nlines], "EDIS", 4) != 0)
				errx(1, "Unexpected reply, not QCMD: %s", lines[nlines]);
			l[nlines] = lines[nlines] + 4;
			nlines++;
		}
	}

	/* Reply should be in same order as query. */
	pcand = cands;
	while ((cand = *pcand)) {
		int match = 0;

		if (!cand->aux) {
			match = 1; /* cli command */
		} else {
			for (i = 0 ; i < nlines ; i++) {
				if (strncmp(((struct command*)cand->aux)->code, l[i], 4) == 0) {
					match = 1;
					l[i] += 4;
				}
			}
		}
		if (match) {
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
	fd_set line_set;
	glob_t g;
	int i, j;
	unsigned gidx;
	char pattern[PATH_MAX];
	int logmask = ~LOG_DEBUG;
	int logopt = 0;

	FD_ZERO(&line_set);

	while ((opt = getopt(argc, argv, ":Ds:")) != -1) {
		switch (opt) {
		case 's':
			fd = open_local (optarg);
			if (fd < 0)
				err (1, "open_local");
			FD_SET(fd, &line_set);
			break;
		case 'D':
			logmask |= LOG_DEBUG;
			logopt |= LOG_PERROR;
			break;
		case ':':
			errx (1, "-%c requires an argument.", optopt);
		case '?':
			errx (1, "unknown option -%c", optopt);
		}
	}
	argc -= optind - 1;
	argv += optind - 1;

	openlog(GETPROGNAME, logopt, LOG_USER | LOG_DEBUG);
	setlogmask(logmask);

	for (i = 1 ; i < argc ; i++) {
		if (argv[i][0] == ':') {
			char *cp;

			if ((cp = strchr(argv[i] + 1, ':'))) {
				*cp = '\0';
				fd = open_remote(argv[i] + 1, cp + 1);
				if (fd < 0)
					err(1, "open_remote");
				FD_SET(fd, &line_set);
				*cp = ':';
			} else {
				snprintf(pattern, sizeof(pattern), "/var/run/movactl.%s*.sock", argv[i] + 1);
				glob(pattern, GLOB_NOSORT, NULL, &g);
				for (gidx = 0 ; gidx < g.gl_pathc ; gidx++) {
					fd = open_local_or_remote(g.gl_pathv[gidx]);
					if (fd < 0)
						err(1, "open_local_or_remote");
					FD_SET(fd, &line_set);
				}
				globfree(&g);
				if (gidx == 0)
					errx(1, "No matching lines for %s", argv[i] + 1);
			}
			for (j = i ; j < argc ; j++)
				argv[j] = argv[j + 1];
			argc--;
			i--;
		}
	}
	if (fd < 0) {
		snprintf(pattern, sizeof(pattern), "/var/run/movactl.*.sock");
		glob(pattern, GLOB_NOSORT, NULL, &g);
		for (gidx = 0 ; gidx < g.gl_pathc ; gidx++) {
			fd = open_local_or_remote(g.gl_pathv[gidx]);
			if (fd < 0)
				err(1, "open_local_or_remote");
			FD_SET(fd, &line_set);
		}
		globfree(&g);
		if (gidx == 0)
			errx(1, "No lines found");
	}

	const char **builtin;
	for (builtin = (const char *[]){"status", "listen", "disable", "enable", NULL} ; *builtin ; builtin++) {
		cand = malloc (sizeof (*cand));
		if (!cand)
			err (1, "malloc");
		cand->name = *builtin;
		cand->name_off = 0;
		cand->next = candidates;
		cand->aux = NULL;
		candidates = cand;
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
		filter_candidates(&line_set, fd, &candidates);

	if (!candidates) 
		errx (1, "No matching command");

	if (!candidates->next) {
		if (strcmp(candidates->name, "disable") == 0 || strcmp(candidates->name, "enable") == 0) {
			if (argc != argi + 1)
				errx (1, "%s: Needs an arguments", candidates->name);
			res = set_server_enabled(&line_set, fd, strcmp(candidates->name, "enable") == 0, argv[argi]);
			if (res < 0)
				err (1, "set_server_enabled");
			return 0;
		} else if (!candidates->aux) {
			j = -1;
			for (i = 0 ; i <= fd ; i++) {
				if (FD_ISSET(i, &line_set)) {
					if (j != -1)
						errx(1, "Can only listen/status on a single line currently");
					j = i;
				}
			}
			return cli_notify(j, argc - argi, argv + argi, strcmp(candidates->name, "status") == 0);
		}

 		if (argi == argc - ((struct command*)candidates->aux)->nargs) {
			res = send_int_command(&line_set, fd, candidates->aux, argv + argi, argc - argi);
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
