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

struct backend_output {
	TAILQ_ENTRY(backend_output) link;
	char *data;
	ssize_t len;
};

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
			bdev->status.dispatch->update_status(bdev, &bdev->status, (char*)data);
		}
		evbuffer_drain(bdev->input, i + 1);
	}
}

static void
backend_writecb(int fd, short what, void *cbarg) {
	struct backend_device *bdev = cbarg;
	struct backend_output *out = TAILQ_FIRST(&bdev->output);

	if (!out)
		return;

	TAILQ_REMOVE(&bdev->output, out, link);

	if (write (bdev->line_fd, out->data, out->len) != out->len)
		err (1, "write");
	free(out->data);
	free(out);

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

#if 0
static int
code_note_cmp (const void *a, const void *b) {
	const struct backend_notify_code *code_note_a = a;
	const struct backend_notify_code *code_note_b = b;

	return strcmp (code_note_a->code, code_note_b->code);
}

static void
backend_notify (struct backend *backend, const char *code, status_notify_cb_t cb, int replace) {
	struct backend_notify_code code_search = {(char*)code};
	struct backend_notify_code *code_note = lfind (&code_search, backend->codes, &backend->num_codes, sizeof (*code_note), code_note_cmp);
	status_notify_token_t token;

	if (send_status_request (backend->dev->line_fd, code) < 0) {
		warn ("backend_notify: send_status_request");
		return;
	}

	if (code_note && !replace)
		return;

	token = status_notify (&backend->dev->status, code, cb, backend);
	if (!token) {
		warn ("backend_notify: status_notify");
		return;
	}

	if (code_note) {
		status_stop_notify (code_note->token);
		code_note->token = token;
		return;
	}

	if (backend->num_codes == backend->alloced_codes) {
		if (!backend->alloced_codes) {
			backend->codes = malloc (8 * sizeof (*backend->codes));
			if (!backend->codes) {
				warn ("backend_notify: malloc");
				status_stop_notify (token);
				return;
			}
			backend->alloced_codes = 8;
		} else {
			struct backend_notify_code *new_codes = realloc (backend->codes,
					2 * backend->alloced_codes * sizeof (*backend->codes));
			if (!backend) {
				warn ("backend_notify: realloc");
				status_stop_notify (token);
				return;
			}
			backend->codes = new_codes;
			backend->alloced_codes *= 2;
		}
	}
	code_note = backend->codes + backend->num_codes++;
	code_note->code = strdup (code);
	if (!code_note->code) {
		warn ("backend_notify: strdup");
		status_stop_notify (token);
		backend->num_codes--;
		return;
	}
	code_note->token = token;
}

static void
backend_stop_notify (struct backend *backend, const char *code) {
	struct backend_notify_code code_search = {(char*)code};
	struct backend_notify_code *code_note = lfind (&code_search, backend->codes, &backend->num_codes, sizeof (*code_note), code_note_cmp);
	size_t cindx = code_note - backend->codes;

	if (!code_note)
		return;

	status_stop_notify (code_note->token);
	if (cindx < --backend->num_codes)
		memmove(backend->codes + cindx, backend->codes + cindx + 1, backend->num_codes - cindx);
}

static void
handle_query_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, void *data, size_t len) {
	struct backend *backend = cbarg;
	int res = bufferevent_write (backend->be, data, len);
	if (res < 0)
		warn ("handle_query_notify_cb");
	
	bufferevent_enable (backend->be, EV_WRITE);
	backend_stop_notify (backend, code);
}

static void
backend_notify_cb (struct status *st, status_notify_token_t token, const char *code, void *cbarg, void *data, size_t len) {
	struct backend *backend = cbarg;
	int res = bufferevent_write (backend->be, data, len);
	if (res < 0)
		warn ("handle_query_notify_cb");
	
	bufferevent_enable (backend->be, EV_WRITE);
}

static void
handle_query (struct backend *backend, char *arg) {
	char buf[256];
	size_t len;
	int res = backend->dev->status.dispatch->status_serialize (&backend->dev->status, arg, buf, &len);

	if (res == SERIALIZE_OK) {
		res = bufferevent_write (backend->be, buf, len);
		if (res < 0)
			warn ("handle_query");
		bufferevent_enable (backend->be, EV_WRITE);
		return;
	}
	
	if (res != SERIALIZE_UNKNOWN) {
		warn ("handle_query");
		return;
	}

	backend_notify (backend, arg, handle_query_notify_cb, 0);
}

static void
handle_send (struct backend *backend, char *arg) {
	char *cp = strchr (arg, ':');

	if (cp) {
		*cp++ = '\0';
		send_command (backend->dev->line_fd, arg, cp);
		if (strcmp (arg, "PWR") == 0)
			sleep (1); /* Unit need some time after power change, even if not changed. */
	}
}

static void
handle_start (struct backend *backend, char *arg) {
	backend_notify (backend, arg, backend_notify_cb, 1);
}

static void
handle_stop (struct backend *backend, char *arg) {
	backend_stop_notify (backend, arg);
}
#endif

#if 0
static void
backend_handle_line (struct backend *backend, char *line) {
	char *sp = strchr (line, ' ');
	const struct backend_command *cmd;

	if (sp)
		*sp++ = '\0';

	cmd = backend_command (line, sp - line - 1);
	if (cmd)
		cmd->handler (backend, sp);
	else
		warnx ("Unknown command: %s", line);
}

static void
backend_read (struct bufferevent *be, void *arg) {
	struct backend *backend = arg;
	char *line;

	while ((line = evbuffer_readline(be->input))) {
		backend_handle_line (backend, line);
		free (line);
	}
}

static void
backend_write_done (struct bufferevent *be, void *arg) {
	bufferevent_disable (be, EV_WRITE);
}

static void
backend_error (struct bufferevent *be, short what, void *arg) {
	struct backend *backend = arg;
	struct backend_notify_code *code_note;

	if (what != (EVBUFFER_READ | EVBUFFER_EOF)) {
		/* Presume errno to still be up to date. */
		warnx ("backend: read error: %d", what);
	}

	backend_read (be, arg);

	bufferevent_free (be);
	close (backend->fd);

	for (code_note = backend->codes; code_note < backend->codes + backend->num_codes; code_note++) {
		free (code_note->code);
		status_stop_notify (code_note->token);
	}
	free (backend);
}

static void
accept_connection (int fd, short what, void *cbarg) {
	struct sockaddr_storage addr;
	socklen_t al = sizeof (addr);
	int cfd = accept (fd, (struct sockaddr*)&addr, &al);
	struct backend *backend;

	if (cfd < 0) {
		warn ("accept_connection");
		return;
	}

	backend = calloc (1, sizeof (*backend));
	if (!backend) {
		warn ("accept_connection");
		close (cfd);
		return;
	}

	backend->dev = cbarg;

	backend->fd = cfd;
	backend->be = bufferevent_new (cfd, backend_read, backend_write_done, backend_error, backend);
	if (!backend->be) {
		warn ("accept_connection");
		free (backend);
		close (cfd);
		return;
	}
	bufferevent_enable (backend->be, EV_READ);
}
#endif

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

		if (!bdev->client)
			continue;

		if ((p = strtol(bdev->client, &e, 0)) > 0 && p < 65536 && e && *e == '\0')
			serverside_listen_tcp(bdev, bdev->client);
		else
			serverside_listen_local(bdev, bdev->client);
	}
}

#if 0
void
backend_close_all (void) {
	struct backend_device *bdev;

	SLIST_FOREACH(bdev, &backends, link) {
		if (bdev->client_fd == -1)
			continue;

		event_del (&bdev->client_event);
		close (bdev->client_fd);
		if (bdev->client && bdev->client_is_file)
			unlink (bdev->client);
	}
}
#endif

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
backend_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args) {
	bdev->status.dispatch->send_command(bdev, cmd, narg, args);
}

