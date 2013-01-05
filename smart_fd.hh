/*
 * Copyright (c) 2013 Per Johansson
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

#include <system_error>

#include <errno.h>
#include <unistd.h>

class smart_fd
{
public:
	int fd;

	smart_fd(int fd = -1)
		: fd(fd)
	{
	}

	smart_fd(const smart_fd &) = delete;
	smart_fd &operator = (const smart_fd &) = delete;

	smart_fd(smart_fd &&a)
		: fd(a.fd)
	{
		a.fd = -1;
	}

	~smart_fd()
	{
		close();
	}

	operator int ()
	{
		return fd;
	}

	smart_fd &operator = (int a)
	{
		close();
		fd = a;
		return *this;
	}

	operator bool ()
	{
		return fd != -1;
	}

	void close()
	{
		if (fd != -1)
			::close(fd);
		fd = -1;
	}

	int release()
	{
		int r = fd;

		if (fd == -1)
			throw std::runtime_error("no fd");
		fd = -1;
		return r;
	}

};

class smart_pipe
{
public:
	smart_fd read;
	smart_fd write;

	smart_pipe()
	{
		int fds[2];

		if (pipe(fds))
			throw std::system_error(errno, std::system_category(), "pipe");

		read = fds[0];
		write = fds[1];
	}
};

