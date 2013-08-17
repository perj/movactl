/*
 * Copyright (c) 2013 Pelle Johansson
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
#ifndef SMART_EVENT_HH
#define SMART_EVENT_HH

#include <event.h>

#include <string.h>

#include <exception>
#include <functional>
#include <memory>

#include "smart_fd.hh"

/* XXX bases */

template <void (*unhandled_exception)() = std::terminate>
class smart_event
{
	struct event ev;
	std::shared_ptr<smart_fd> fd;
	std::function<void (evutil_socket_t, short)> callback;

	static void
	callback_wrapper(evutil_socket_t fd, short what, void *cbarg)
	{
		try {
			static_cast<smart_event*>(cbarg)->callback(fd, what);
		} catch (...) {
			unhandled_exception();
		}
	}

public:
	smart_event()
	{
		memset(&ev, 0, sizeof(ev));
	}

	smart_event(const smart_event&) = delete;
	smart_event &operator =(const smart_event &) = delete;

	~smart_event()
	{
		if (event_initialized(&ev))
			event_del(&ev);
	}

	void set_fd(std::shared_ptr<smart_fd> fd)
	{
		this->fd = std::move(fd);
	}

	void set_fd(int fd)
	{
		this->fd = std::make_shared<smart_fd>(fd);
	}

	void set(short what, decltype(callback) cb)
	{
		callback = cb;
		event_set(&ev, *fd, what, callback_wrapper, this);
	}

	void set_signal(int signum, decltype(callback) cb)
	{
		callback = cb;
		signal_set(&ev, signum, callback_wrapper, this);
	}

	int add()
	{
		return event_add(&ev, NULL);
	}

	int add(const struct timeval &timeout)
	{
		return event_add(&ev, &timeout);
	}

	bool pending(short what, struct timeval *out_timeout = NULL)
	{
		return event_pending(&ev, what, out_timeout);
	}

	void del()
	{
		event_del(&ev);
	}

	void reset()
	{
		if (event_initialized(&ev))
			event_del(&ev);
		memset(&ev, 0, sizeof(ev));
		fd.reset();
	}
};

#endif /*SMART_EVENT_HH*/
