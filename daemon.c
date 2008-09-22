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

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>
#include <event.h>
#include <stdlib.h>

#include "line.h"
#include "status.h"
#include "backend.h"
#include "launchd.h"

struct ma_status status;
int line_fd;

int running;

void
quit_event (int fd, short what, void *cbarg) {
	running = 0;
	event_loopexit (NULL);
}

void
line_reader (int fd, short what, void *cbarg) {
	char buf[256];
	int res;

	if ((res = read_line (fd, buf, sizeof (buf))) > 0) {
		fprintf (stderr, "Read line %s\n", buf);
		update_status (fd, &status, buf);
	} else {
		if (res)
			err (1, "read_line");
		event_loopexit (NULL);
	}
}

int
main (int argc, char *argv[]) {
	struct event line_ev;
#ifndef USE_LAUNCHD
	struct event *backend_event;
#endif
	struct event term_ev;
	const char *line = "/dev/tty.usbserial";
	const char *sock = "/tmp/morantz.sock";
	extern char *optarg;
	extern int optind;
	extern int optopt;
	char opt;

	while ((opt = getopt(argc, argv, ":s:d:")) != -1) {
		switch (opt) {
		case 's':
			sock = optarg;
			break;
		case 'd':
			line = optarg;
			break;
		case ':':
			err (1, "-%c requires an argument.", optopt);
		case '?':
			err (1, "unknown option -%c", optopt);
		}
	}
	argc -= optind;
	argv += optind;

	/*
	 * Believe it or not, but it seems both kqueue and poll engines are broken on OS X right now.
	 * Might just be the Prolific driver, but keeping to select for now.
	 */
	setenv ("EVENT_NOKQUEUE", "1", 0);
	setenv ("EVENT_NOPOLL", "1", 0);
	event_init();

	signal_set (&term_ev, SIGTERM, quit_event, NULL);
	signal_add (&term_ev, NULL);

#ifdef USE_LAUNCHD
	launchd_init();
#else
	backend_event = backend_listen_local (sock);
	if (!backend_event)
		err (1, "backend_listen_local");
#endif

	line_fd = -1;
	running = 1;
	while (running) {
		if (line_fd >= 0) {
			close (line_fd);
			fprintf (stderr, "EOF, reopening after sleep\n");
			sleep (1);
		}

		line_fd = open_line (line, O_RDWR);
		if (line_fd < 0)
			err (1, "open_line");

		event_set (&line_ev, line_fd, EV_READ | EV_PERSIST, line_reader, NULL);
		if (event_add (&line_ev, NULL))
			err (1, "event_add");

		enable_auto_status_layer (line_fd, &status, 1);
		status.known_fields = 0;

		if (event_dispatch ())
			err (1, "event_dispatch");
	}

#ifndef USE_LAUNCHD
	backend_close_listen (backend_event);
#endif
	warnx ("Exiting normally");
	return 0;
}
