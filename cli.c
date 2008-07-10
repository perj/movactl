
#include "cli.h"

#include <stdio.h>
#include <err.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "command.h"
#include "line.h"
#include "api.h"

#undef _MORANTZ_COMMAND_H
#undef SIMPLE_COMMAND
#undef SIGNINT_COMMAND
#undef UINT_COMMAND

#define SIMPLE_COMMAND(name, code, arg) \
	{ #name, ma_command_ ## name, NULL, NULL },
#define SIGNINT_COMMAND(name, code, prefix) \
	{ #name, NULL, ma_command_ ## name, NULL },
#define UINT_COMMAND(name, code, prefix, width) \
	{ #name, NULL, NULL, ma_command_ ## name },

struct command
{
	const char *name;
	int (*simple_cmd_func) (int fd);
	int (*signint_cmd_func) (int fd, int value);
	int (*uint_cmd_func) (int fd, unsigned int value);
} commands[] = {
#include "command.h"
	{NULL}
};

const char *line = "/dev/tty.usbserial";
const char *sock = "/tmp/morantz.sock";

struct command_candidate
{
	struct command *cmd;
	int name_off;
	int is_exact;
	struct command_candidate *next;
};

int
main (int argc, char *argv[]) {
	struct command *cmd;
	struct command_candidate *cand, *candidates = NULL, *pcand;
	int argi = 1;
	int argo = 0;
	int res;
	int fd;

	fd = ma_open_local (sock);
	if (fd < 0) {
		fd = open_line (line, O_WRONLY);
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
		if (argi == argc - 1 && argo == 0 && !candidates->next) {
			if (candidates->cmd->signint_cmd_func) {
				res = candidates->cmd->signint_cmd_func(fd, atoi(argv[argi]));
				if (res < 0)
					err (1, "send_command");
				return 0;
			}
			if (candidates->cmd->uint_cmd_func) {
				res = candidates->cmd->uint_cmd_func(fd, atoi(argv[argi]));
				if (res < 0)
					err (1, "send_command");
				return 0;
			}
		}
	}

	if (!candidates)
		errx (1, "No matching command");
	if (candidates->next) {
		/* See if any command is completed */
		for (cand = candidates; cand; cand = cand->next) {
			//fprintf (stderr, "Checking %s for completness, name_off = %d\n", cand->cmd->name, cand->name_off);
			if (cand->name_off == (int)strlen (cand->cmd->name)) {
				if (!cand->cmd->simple_cmd_func)
					errx (1, "Command requires an argument.");

				res = cand->cmd->simple_cmd_func (fd);
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

	if (!candidates->cmd->simple_cmd_func)
		errx (1, "Command requires an argument.");

	res = candidates->cmd->simple_cmd_func (fd);
	if (res < 0)
		err (1, "send_command");

	return 0;
}
