
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

#include "status.h"
#include "line.h"

extern struct ma_status status;
extern int line_fd;

struct backend_notify_code {
	char *code;
	status_notify_token_t token;
};

struct backend {
	int fd;
	struct bufferevent *be;

	struct backend_notify_code *codes;
	size_t num_codes;
	size_t alloced_codes;
};

static int
code_note_cmp (const void *a, const void *b) {
	const struct backend_notify_code *code_note_a = a;
	const struct backend_notify_code *code_note_b = b;

	return strcmp (code_note_a->code, code_note_b->code);
}

static void
backend_notify (struct backend *backend, const char *code, void (*cb)(struct ma_status *status, status_notify_token_t token, 
		const char *code, void *cbarg, void *data, size_t len), int replace) {
	struct backend_notify_code code_search = {(char*)code};
	struct backend_notify_code *code_note = lfind (&code_search, backend->codes, &backend->num_codes, sizeof (*code_note), code_note_cmp);
	status_notify_token_t token;

	if (send_status_request (line_fd, code) < 0) {
		warn ("backend_notify: send_status_request");
		return;
	}

	if (code_note && !replace)
		return;

	token = status_notify (&status, code, cb, backend);
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
handle_query_notify_cb (struct ma_status *st, status_notify_token_t token, const char *code, void *cbarg, void *data, size_t len) {
	struct backend *backend = cbarg;
	int res = bufferevent_write (backend->be, data, len);
	if (res < 0)
		warn ("handle_query_notify_cb");
	
	bufferevent_enable (backend->be, EV_WRITE);
	backend_stop_notify (backend, code);
}

static void
backend_notify_cb (struct ma_status *st, status_notify_token_t token, const char *code, void *cbarg, void *data, size_t len) {
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
	int res = status_serialize (&status, arg, buf, &len);

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
		send_command (line_fd, arg, cp);
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

#include "backend_command.h"

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
	struct sockaddr_un unaddr;
	socklen_t al = sizeof (unaddr);
	int cfd = accept (fd, (struct sockaddr*)&unaddr, &al);
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

struct event *
backend_listen_fd (int fd, const char *path) {
	struct event *event = malloc (sizeof (*event));
	if (!event) {
		close (fd);
		return NULL;
	}
	event_set (event, fd, EV_READ | EV_PERSIST, accept_connection, path ? strdup (path) : NULL);
	if (event_add (event, NULL)) {
		close (fd);
		free (event);
		return NULL;
	}

	return event;
}

struct event *
backend_listen_local (const char *path) {
	struct sockaddr_un unaddr = {0};
	int fd;

	if (unlink (path) && errno != ENOENT)
		return NULL;

	fd = socket (PF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0)
		return NULL;

	unaddr.sun_family = AF_LOCAL;
	strncpy (unaddr.sun_path, path, sizeof (unaddr.sun_path) - 1);
	unaddr.sun_path[sizeof (unaddr.sun_path) - 1] = '\0';

	if (bind (fd, (struct sockaddr*)&unaddr, sizeof (unaddr))) {
		close (fd);
		return NULL;
	}

	if (listen (fd, 128)) {
		close (fd);
		return NULL;
	}

	return backend_listen_fd (fd, path);
}

void
backend_close_listen (struct event *ev) {
	event_del (ev);
	close (ev->ev_fd);
	if (ev->ev_arg) {
		unlink (ev->ev_arg);
		free (ev->ev_arg);
	}
	free (ev);
}

