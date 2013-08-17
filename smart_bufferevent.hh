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

#include <event.h>

#include <memory>

#include "smart_fd.hh"

/* XXX bases */

template <void (*unhandled_exception)()>
class smart_bufferevent
{
	std::shared_ptr<smart_fd> fd;
	struct bufferevent *be;
	std::function<void ()> read_callback;
	std::function<void ()> write_callback;
	std::function<void (short)> error_callback;

	static void
	read_callback_wrapper(struct bufferevent *be, void *cbarg)
	{
		try {
			static_cast<smart_bufferevent*>(cbarg)->read_callback();
		} catch (...) {
			unhandled_exception();
		}
	}

	static void
	write_callback_wrapper(struct bufferevent *be, void *cbarg)
	{
		try {
			static_cast<smart_bufferevent*>(cbarg)->write_callback();
		} catch (...) {
			unhandled_exception();
		}
	}

	static void
	error_callback_wrapper(struct bufferevent *be, short what, void *cbarg)
	{
		try {
			static_cast<smart_bufferevent*>(cbarg)->error_callback(what);
		} catch (...) {
			unhandled_exception();
		}
	}

public:
	template <class FD>
	smart_bufferevent(FD fd, decltype(read_callback) rcb, decltype(write_callback) wcb,
			decltype(error_callback) ecb)
		: fd(std::make_shared<smart_fd>(std::move(fd))), read_callback(std::move(rcb)), write_callback(std::move(wcb)),
		error_callback(std::move(ecb))
	{
		be = bufferevent_new(*this->fd, read_callback_wrapper, write_callback_wrapper,
				error_callback_wrapper, this);
		if (!be)
			throw std::bad_alloc();
	}

	~smart_bufferevent()
	{
		bufferevent_free(be);
	}

	operator struct bufferevent * ()
	{
		return be;
	}

	struct bufferevent *
	operator -> ()
	{
		return be;
	}

	int write(const std::string &str)
	{
		return bufferevent_write(be, str.c_str(), str.length());
	}
};
