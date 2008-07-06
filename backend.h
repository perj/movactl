#ifndef BACKEND_H
#define BACKEND_H

#include <event.h>

struct event *backend_listen_fd (int fd, const char *path);
struct event *backend_listen_local (const char *path);
void backend_close_listen (struct event *ev);

#endif /*BACKEND_H*/
