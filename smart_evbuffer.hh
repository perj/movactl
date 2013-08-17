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
#ifndef SMART_EVBUFFER_HH
#define SMART_EVBUFFER_HH

#include <exception>

#include <event.h>

class smart_evbuffer
{
	struct evbuffer *buf;

public:
	smart_evbuffer()
	{
		buf = evbuffer_new();
		if (!buf)
			throw std::bad_alloc();
	}

	~smart_evbuffer()
	{
		evbuffer_free(buf);
	}

	smart_evbuffer(const smart_evbuffer &) = delete;
	smart_evbuffer &operator =(const smart_evbuffer &) = delete;

	int read(evutil_socket_t fd, int howmuch)
	{
		return evbuffer_read(buf, fd, howmuch);
	}

	size_t length()
	{
		return EVBUFFER_LENGTH(buf);
	}

	unsigned char *data()
	{
		return EVBUFFER_DATA(buf);
	}

	int drain(size_t len)
	{
		return evbuffer_drain(buf, len);
	}

	void reset(void)
	{
		drain(length());
	}
};

#endif /*SMART_EVBUFFER_HH*/
