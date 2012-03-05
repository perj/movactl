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

#include "backend.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <event.h>
#include <err.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <search.h>
#include <stdio.h>
#include <sys/queue.h>
#include <fcntl.h>
#include <stdarg.h>
#include <netinet/in.h>
#include <sys/stat.h>

#include "status.h"
#include "line.h"
#include "api_serverside.h"

#include "backend_type.h"

struct backend_device {
	SLIST_ENTRY(backend_device) link;
	char *name;

	char *line;
	int line_fd;
	struct event read_ev;
	struct event write_ev;

	struct evbuffer *input;
	TAILQ_HEAD(, backend_output) output;
	struct timeval out_throttle;
	struct backend_output **outptr;

	char *client;
	int client_fd;
	struct event client_event;
	int client_is_file;

	struct status status;
};

SLIST_HEAD(, backend_device) backends = SLIST_HEAD_INITIALIZER(backends);

struct backend_notify_code {
	char *code;
	status_notify_token_t token;
};

void
add_backend_device(const char *str) {
	int ms = 0;
	char *name = strdup(str);

	if (!name)
		err(1, "strdup(backend)");

	char *type = strchr(name, ':');

	if (!type)
		errx (1, "No type for backend: %s", str);
	*type++ = '\0';

	char *path = strchr(type, ':');
	if (!path)
		errx (1, "No path for backend: %s:%s", str, type);
	*path++ = '\0';

	char *client = strchr(path, ':');
	if (client) {
		char *ts;

		*client++ = '\0';

		ts = strchr(client, ':');
		if (ts) {
			*ts++ = '\0';
			ms = atoi(ts);
		}

		if (!*client)
			client = NULL;
	}

	struct backend_device *bdev = calloc (1, sizeof (*bdev));
	if (!bdev)
		err (1, "malloc(backend)");

	const struct backend_type *bt = backend_type(type, strlen(type));
	if (!bt)
		errx (1, "Unknown device type: %s", type);

	bdev->status.dispatch = bt->dispatch;
	bdev->name = name;
	bdev->line = path;
	bdev->line_fd = -1;
	bdev->client = client;
	bdev->client_fd = -1;
	bdev->out_throttle.tv_sec = ms / 1000;
	bdev->out_throttle.tv_usec = (ms % 1000) * 1000;
	TAILQ_INIT(&bdev->output);
	bdev->outptr = &TAILQ_FIRST(&bdev->output);

	SLIST_INSERT_HEAD(&backends, bdev, link);
}

static void
backend_readcb(int fd, short what, void *cbarg) {
	struct backend_device *bdev = cbarg;
	size_t len;

	int res = evbuffer_read (bdev->input, fd, 1024);
	if (res < 0)
		err (1, "evbuffer_read");
	if (res == 0)
		event_loopexit (NULL);

	while ((len = EVBUFFER_LENGTH(bdev->input))) {
		unsigned char *data = EVBUFFER_DATA(bdev->input);
		size_t i;

		for (i = 0 ; i < len ; i++) {
			if (strchr(bdev->status.dispatch->packet_separators, data[i]))
				break;
		}
		if (i == len)
			break;

		if (i > 0) {
			data[i] = '\0';
			bdev->status.dispatch->update_status(bdev, &bdev->status, (char*)data,
					&TAILQ_FIRST(&bdev->output), bdev->outptr);
		}
		evbuffer_drain(bdev->input, i + 1);
	}
}

static void
backend_writecb(int fd, short what, void *cbarg) {
	struct backend_device *bdev = cbarg;
	struct backend_output *out = *bdev->outptr;

	if (!out)
		return;

	bdev->outptr = &TAILQ_NEXT(out, link);

	if (write (bdev->line_fd, out->data, out->len) != out->len)
		err (1, "write");

	event_add(&bdev->write_ev, &bdev->out_throttle);
}

void
backend_reopen_devices(void) {
	struct backend_device *bdev;

	SLIST_FOREACH(bdev, &backends, link) {
		if (bdev->line_fd >= 0) {
			event_del(&bdev->read_ev);
			event_del(&bdev->write_ev);
			evbuffer_free(bdev->input);
			close (bdev->line_fd);
		}

		while (!TAILQ_EMPTY(&bdev->output)) {
			struct backend_output *out = TAILQ_FIRST(&bdev->output);

			TAILQ_REMOVE(&bdev->output, out, link);
			free(out->data);
			free(out);
		}
		bdev->outptr = &TAILQ_FIRST(&bdev->output);

		bdev->line_fd = open_line (bdev->line, O_RDWR);
		if (bdev->line_fd < 0)
			err (1, "open_line");

		event_set (&bdev->read_ev, bdev->line_fd, EV_READ | EV_PERSIST, backend_readcb, bdev);
		event_set (&bdev->write_ev, -1, EV_TIMEOUT, backend_writecb, bdev);

		if (event_add(&bdev->read_ev, NULL))
			err (1, "event_add");

		bdev->input = evbuffer_new();
		if (!bdev->input)
			err (1, "evbuffer_new");

		bdev->status.dispatch->status_setup(bdev, &bdev->status);
	}
}

void
backend_listen_fd (const char *name, int fd) {
	struct backend_device *bdev;

	SLIST_FOREACH(bdev, &backends, link) {
		if (strcmp(name, bdev->name) == 0)
			break;
	}
	if (!bdev)
		errx (1, "backend_listen: No matching device %s", name);

	serverside_listen_fd(bdev, fd);
}

void
backend_listen_all (void) {
	struct backend_device *bdev;

	SLIST_FOREACH(bdev, &backends, link) {
		char *e = NULL;
		int p;
		char *c, *client, *cl;

		if (!bdev->client)
			continue;

		cl = client = strdup(bdev->client);
		if (!client)
			err(1, "backend_listen_all");

		while ((c = strsep(&client, ","))) {
			if ((p = strtol(c, &e, 0)) > 0 && p < 65536 && e && *e == '\0')
				serverside_listen_tcp(bdev, c);
			else
				serverside_listen_local(bdev, c);
		}
		free(cl);
	}
}

void
backend_close_all (void) {
	struct backend_device *bdev;

	SLIST_FOREACH(bdev, &backends, link) {
		if (bdev->line_fd >= 0) {
			event_del(&bdev->read_ev);
			event_del(&bdev->write_ev);
			evbuffer_free(bdev->input);
			close (bdev->line_fd);
			bdev->line_fd = -1;
		}
	}
}

void
backend_send(struct backend_device *bdev, const char *fmt, ...) {
	struct backend_output *out = malloc(sizeof (*out));
	va_list ap;

	if (!out)
		err (1, "malloc");
	va_start(ap, fmt);
	out->len = vasprintf(&out->data, fmt, ap);
	va_end(ap);

	if (out->len < 0)
		err (1, "vasprintf");

	TAILQ_INSERT_TAIL(&bdev->output, out, link);
	if (!event_pending(&bdev->write_ev, EV_TIMEOUT, NULL))
		backend_writecb(-1, 0, bdev);
}

void
backend_remove_output(struct backend_device *bdev, struct backend_output **inptr) {
	struct backend_output *out = *inptr;

	if (!out)
		return;

	if (bdev->outptr == &TAILQ_NEXT(out, link))
		bdev->outptr = inptr;

	TAILQ_REMOVE(&bdev->output, out, link);
	free(out->data);
	free(out);
}

void
backend_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args) {
	bdev->status.dispatch->send_command(bdev, cmd, narg, args);
}

struct status *
backend_get_status(struct backend_device *bdev) {
	return &bdev->status;
}

void
backend_send_status_request(struct backend_device *bdev, const char *code) {
	 bdev->status.dispatch->send_status_request(bdev, code);
}
