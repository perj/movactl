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

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>
#include <event.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <string.h>

#include "line.h"
#include "status.h"
#include "backend.h"
#include "launchd.h"
#include "api_serverside.h"

int running;
int launchd_flag;

void
quit_event (int fd, short what, void *cbarg) {
	running = 0;
	event_loopexit (NULL);
}

extern char *optarg;
extern int optind;
extern int optopt;

int
main (int argc, char *argv[]) {
	struct event term_ev;
	char opt;

	while ((opt = getopt(argc, argv, ":l")) != -1) {
		switch (opt) {
		case 'l':
			launchd_flag = 1;
			break;
		case ':':
			err (1, "-%c requires an argument.", optopt);
		case '?':
			err (1, "unknown option -%c", optopt);
		}
	}
	argc -= optind;
	argv += optind;

	if (!argc)
		errx (1, "No devices");
	while (argc) {
		add_backend_device(*argv++);
		argc--;
	}

	/*
	 * Believe it or not, but it seems both kqueue and poll engines are broken on OS X right now.
	 * Might just be the Prolific driver, but keeping to select for now.
	 */
	setenv ("EVENT_NOKQUEUE", "1", 0);
	setenv ("EVENT_NOPOLL", "1", 0);
	event_init();

	signal_set (&term_ev, SIGTERM, quit_event, NULL);
	signal_add (&term_ev, NULL);

	if (launchd_flag)
		launchd_init();
	backend_listen_all();

	running = 1;
	while (running) {
		backend_reopen_devices();

		if (event_dispatch ())
			err (1, "event_dispatch");
		
		if (running) {
			warnx ("EOF, reopening after sleep");
			sleep (1);
		}
	}

	serverside_close_all();
	warnx ("Exiting normally");
	return 0;
}
