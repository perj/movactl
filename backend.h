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

#ifndef BACKEND_H
#define BACKEND_H

#include <sys/types.h>
#include <sys/queue.h>
#include <event.h>

#ifdef __cplusplus
extern "C" {
#endif

struct backend_device;
struct status;

struct backend_output {
	char *data;
	ssize_t len;
	struct timeval throttle;
};

void add_backend_device(const char *str);

void backend_reopen_devices(void);

void backend_listen_fd (const char *dev, int fd);

void backend_listen_all (void);
void backend_close_all (void);

void backend_send_command(struct backend_device *bdev, const char *cmd, int narg, int32_t *args);
void backend_send(struct backend_device *bdev, const char *fmt, ...) __attribute__((format(printf, 2, 3)));
void backend_send_throttle(struct backend_device *bdev, const struct timeval *throttle, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
void backend_remove_output(struct backend_device *bdev, const struct backend_output **inptr);

struct status *backend_get_status(struct backend_device *bdev);
void backend_send_status_request(struct backend_device *bdev, const char *code);

#ifdef __cplusplus
}
#endif

#endif /*BACKEND_H*/
