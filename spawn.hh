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

#include <string>
#include <vector>

#include <spawn.h>

namespace spawn
{

class file_actions
{
public:
	posix_spawn_file_actions_t actions;

	file_actions()
	{
		posix_spawn_file_actions_init(&actions);
		if (!actions)
			throw std::bad_alloc();
	}

	file_actions(const file_actions &) = delete;
	file_actions &operator =(const file_actions &) = delete;

	~file_actions()
	{
		posix_spawn_file_actions_destroy(&actions);
	}

	operator posix_spawn_file_actions_t* ()
	{
		return &actions;
	}

	void adddup2(int from, int to)
	{
		posix_spawn_file_actions_adddup2(&actions, from, to);
	}

	void add_stdout(int fd)
	{
		adddup2(fd, STDOUT_FILENO);
	}

	void addclose(int fd)
	{
		posix_spawn_file_actions_addclose(&actions, fd);
	}
};

};
