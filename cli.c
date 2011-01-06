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

#include "line.h"
#include "base64.h"

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

struct command_candidate
{
	struct command *cmd;
	int name_off;
	int is_exact;
	struct command_candidate *next;
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

extern char *optarg;
extern int optind;
extern int optopt;

int
main (int argc, char *argv[]) {
	struct command *cmd;
	struct command_candidate *cand, *candidates = NULL, *pcand;
	int argi = 1;
	int argo = 0;
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

#if 0
	if (argc > 1) {
		if (strcmp (argv[1], "listen") == 0)
			return cli_notify(fd, argc - 2, argv + 2, 0);
		if (strcmp (argv[1], "status") == 0)
			return cli_notify(fd, argc - 2, argv + 2, 1);
	}
#endif

	for (cmd = commands; cmd->name; cmd++) {
		cand = malloc (sizeof (*cand));
		if (!cand)
			err (1, "malloc");

		cand->cmd = cmd;
		cand->name_off = 0;
		cand->next = candidates;
		candidates = cand;
	}

	while (argi < argc) {
		const char *arg = argv[argi] + argo;
		const char *ea;
		int al;
		int have_exact = 0;

		ea = strpbrk (arg, "_- ");
		if (ea) {
			argo += ea - arg + 1;
			al = ea - arg;
		} else {
			argi++;
			argo = 0;
			al = strlen (arg);
		}

		pcand = NULL;
		cand = candidates;
		while (cand) {
			if (strncmp (arg, cand->cmd->name + cand->name_off, al) != 0) {
				if (pcand)
					pcand->next = cand->next;
				else
					candidates = cand->next;
				free (cand);
				if (pcand)
					cand = pcand->next;
				else
					cand = candidates;
			} else {
				ea = strpbrk (cand->cmd->name + cand->name_off, "_- ");
				if (ea) {
					cand->is_exact = ea - cand->cmd->name - cand->name_off == al;
					cand->name_off = ea - cand->cmd->name + 1;
				} else {
					cand->is_exact = (int)strlen (cand->cmd->name + cand->name_off) == al;
					cand->name_off += strlen (cand->cmd->name + cand->name_off);
				}
				if (cand->is_exact)
					have_exact = 1;
				pcand = cand;
				cand = cand->next;
			}
		}

		if (!candidates) 
			errx (1, "No matching command");
		if (have_exact) {
			/* Eliminate non-exacts */
			pcand = NULL;
			cand = candidates;
			while (cand) {
				if (!cand->is_exact) {
					if (pcand)
						pcand->next = cand->next;
					else
						candidates = cand->next;
					free (cand);
					if (pcand)
						cand = pcand->next;
					else
						cand = candidates;
				} else {
					pcand = cand;
					cand = cand->next;
				}
			}
		}
		if (!candidates->next && argi == argc - candidates->cmd->nargs) {
			res = send_int_command(fd, candidates->cmd, argv + argi, argc - argi);
			if (res < 0)
				err (1, "send_int_command");
			return 0;
		}
	}

	if (!candidates)
		errx (1, "No matching command");
	if (candidates->next) {
		/* See if any command is complete */
		for (cand = candidates; cand; cand = cand->next) {
			//fprintf (stderr, "Checking %s for completness, name_off = %d\n", cand->cmd->name, cand->name_off);
			if (cand->name_off == (int)strlen (cand->cmd->name)) {
				if (cand->cmd->nargs > 0)
					errx (1, "Command requires an argument.");

				res = send_int_command(fd, cand->cmd, NULL, 0);
				if (res < 0)
					err (1, "send_command");
				return 0;
			}
		}
		fprintf (stderr, "Multiple matches:\n");
		for (cand = candidates; cand; cand = cand->next) {
			fprintf (stderr, "%s\n", cand->cmd->name);
		}
		return 1;
	}

	if (candidates->cmd->nargs > 0)
		errx (1, "Command requires an argument.");

	res = send_int_command(fd, cand->cmd, NULL, 0);
	if (res < 0)
		err (1, "send_command");

	return 0;
}
