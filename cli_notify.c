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

#include <stdio.h>
#include <stdlib.h>
#include <err.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/uio.h>
#include <unistd.h>
#include <event.h>
#include <syslog.h>

#include "cli.h"
#include "base64.h"
#include "complete.h"

typedef void (*notify_cb_t)(int fd, const char *name, const char *code, const char *arg, size_t len);

struct notify_data
{
	TAILQ_ENTRY(notify_data) link;
	const char *name;
	const char *code;
	notify_cb_t cb;
	int once;
};

TAILQ_HEAD(, notify_data) notifies = TAILQ_HEAD_INITIALIZER(notifies);

static void
start_notify(int fd, const char *name, const char *code, notify_cb_t cb, int once) {
	struct notify_data *data = malloc(sizeof (*data));
	struct iovec vecs[3];

	if (!data)
		err(1, "malloc");

	data->name = name;
	data->code = code;
	data->cb = cb;
	data->once = once;
	TAILQ_INSERT_TAIL(&notifies, data, link);

	vecs[0].iov_base = (void*)"STRT";
	vecs[0].iov_len = 4;
	vecs[1].iov_base = (void*)code;
	vecs[1].iov_len = 4;
	vecs[2].iov_base = (void*)"\n";
	vecs[2].iov_len = 1;
	
	if (writev(fd, vecs, 3) == -1)
		err(1, "writev");
}

static void
stop_notify(int fd, struct notify_data *data) {
	struct notify_data *d;

	TAILQ_REMOVE(&notifies, data, link);

	TAILQ_FOREACH(d, &notifies, link) {
		if (strcmp(d->code, data->code) == 0)
			break;
	}
	if (!d) {
		struct iovec vecs[3];

		vecs[0].iov_base = (void*)"STOP";
		vecs[0].iov_len = 4;
		vecs[1].iov_base = (void*)data->code;
		vecs[1].iov_len = 4;
		vecs[2].iov_base = (void*)"\n";
		vecs[2].iov_len = 1;
		
		if (writev(fd, vecs, 3) == -1)
			err(1, "writev");
	}
	free(data);
}

#define ESTART(type) \
	void notify_ ## type ## _cb (int fd, const char *n, const char *code, const char *val, size_t len) { \
		if (len != 4) \
			return; \
 \
		int v = debase64_int24(val); \
		switch (v) {
#define EV(type, name, val) \
		case val: \
			printf("%s " #name "\n", n); \
			fflush(stdout); \
			break;
#define EEND(type) \
		default: \
			printf("%s unknown:0x%x\n", n, v); \
			fflush(stdout); \
		} \
	}
#include "status_enums.h"
#undef ESTART
#undef EV
#undef EEND

void
notify_int_cb (int fd, const char *n, const char *code, const char *val, size_t len) {
	if (len != 4)
		return;

	printf ("%s %d\n", n, debase64_int24(val));
	fflush(stdout);
}

void
notify_string_cb (int fd, const char *n, const char *code, const char *val, size_t len) {
	printf ("%s %.*s\n", n, (int)len, val);
	fflush(stdout);
}

struct notify_code {
	const char *name;
	const char *code;
	notify_cb_t cb;
} notify_codes[] = {
#define NOTIFY(name, code, type) { #name, code, notify_ ## type ## _cb },
#define STATUS(name, code, type) { #name, code, notify_ ## type ## _cb },
#include "all_notify.h"
#undef NOTIFY
#undef STATUS
	{ NULL }
};

static int
filter_notifies (int fd, struct complete_candidate **cands) {
	struct complete_candidate *cand, **pcand;
	struct evbuffer *buf = evbuffer_new();
	char *line;

	evbuffer_add(buf, "QSTS", 4);
	for (cand = *cands ; cand ; cand = cand->next)
		evbuffer_add(buf, ((struct notify_code*)cand->aux)->code, 4);
	syslog(LOG_DEBUG, "QSTS fd: %d query: %.*s", fd, (int)EVBUFFER_LENGTH(buf), EVBUFFER_DATA(buf));
	evbuffer_add(buf, "\n", 1);

	write (fd, EVBUFFER_DATA(buf), EVBUFFER_LENGTH(buf));

	evbuffer_drain(buf, EVBUFFER_LENGTH(buf));
	while (!(line = evbuffer_readline(buf)))
		evbuffer_read(buf, fd, 1024);
	if (strncmp(line, "EDIS", 4) == 0)
		return 1;
	if (strncmp(line, "QSTS", 4) != 0)
		errx(1, "Unexpected reply, not QSTS: %s", line);

	line += 4;

	/* Reply should be in same order as query. */
	pcand = cands;
	while ((cand = *pcand)) {
		if (strncmp(((struct notify_code*)cand->aux)->code, line, 4) == 0) {
			pcand = &cand->next;
			line += 4;
		} else {
			*pcand = cand->next;
			free(cand);
		}
	}
	return 0;
}

int
cli_notify (int fd, int argc, char *argv[], int once) {
	int num = 0;
	struct evbuffer *data;
	struct complete_candidate *candidates = NULL, *cand;
	struct notify_code *nc;
	int argi = 0;

	do {
		candidates = NULL;
		for (nc = notify_codes ; nc->name ; nc++) {
			cand = malloc (sizeof (*cand));
			if (!cand)
				err (1, "malloc");

			cand->name = nc->name;
			cand->name_off = 0;
			cand->next = candidates;
			cand->aux = nc;
			candidates = cand;
		}

		syslog(LOG_DEBUG, "complete at %d '%s'", argi, argv[argi]);
		argi += complete(&candidates, argc - argi, (const char**)argv + argi, (void(*)(struct complete_candidate*))free); 
		syslog(LOG_DEBUG, "after complete at %d '%s'", argi, argv[argi]);

		if (candidates) {
			if (filter_notifies(fd, &candidates))
				errx(1, "server disabled");
		}

		if (!candidates) 
			errx (1, "No matching notification");

		if (candidates->next) {
			fprintf (stderr, "Multiple matches:\n");
			for (cand = candidates; cand; cand = cand->next) {
				fprintf (stderr, "%s\n", cand->name);
			}
			return 1;
		}

		nc = candidates->aux;
		start_notify(fd, nc->name, nc->code, nc->cb, once);
		num++;

		free(candidates);
	} while (argi < argc);

	data = evbuffer_new();
	while (!TAILQ_EMPTY(&notifies)) {
		int res = evbuffer_read(data, fd, 1024);
		char *line;

		if (res < 0)
			err (1, "evbuffer_read");
		if (res == 0)
			errx (1, "EOF");

		while ((line = evbuffer_readline(data))) {
			int len = strlen(line);

			if (len >= 8 && strncmp(line, "STAT", 4) == 0) {
				const char *l = line + 4;
				const char *v = line + 8;
				struct notify_data *d;
				
				TAILQ_FOREACH(d, &notifies, link) {
					if (strncmp(l, d->code, 4) == 0) {
						d->cb(fd, d->name, d->code, v, len - 8);
						if (d->once) {
							stop_notify(fd, d);
							break; /* XXX multiple with same code? */
						}
					}
				}
			}
		}
	}
	return 0;
}
