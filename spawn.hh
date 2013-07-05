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

#if defined(__has_include)
#if __has_include(<spawn.h>)
#define HAVE_POSIX_SPAWN 1
#endif
#endif

#ifdef HAVE_POSIX_SPAWN
#include <spawn.h>
#else
#include <set>
#endif

namespace spawn
{

class file_actions
{
public:
#ifdef HAVE_POSIX_SPAWN
	posix_spawn_file_actions_t actions;
#else
	std::set<int> to_close;
	std::set<std::pair<int, int>> to_dup;
#endif

	file_actions()
	{
#ifdef HAVE_POSIX_SPAWN
		posix_spawn_file_actions_init(&actions);
		if (!actions)
			throw std::bad_alloc();
#endif
	}

	file_actions(const file_actions &) = delete;
	file_actions &operator =(const file_actions &) = delete;

	~file_actions()
	{
#ifdef HAVE_POSIX_SPAWN
		posix_spawn_file_actions_destroy(&actions);
#endif
	}

#ifdef HAVE_POSIX_SPAWN
	operator posix_spawn_file_actions_t* ()
	{
		return &actions;
	}
#endif

	void adddup2(int from, int to)
	{
#ifdef HAVE_POSIX_SPAWN
		posix_spawn_file_actions_adddup2(&actions, from, to);
#else
		to_dup.insert(std::make_pair(from, to));
#endif
	}

	void add_stdout(int fd)
	{
		adddup2(fd, STDOUT_FILENO);
	}

	void addclose(int fd)
	{
#ifdef HAVE_POSIX_SPAWN
		posix_spawn_file_actions_addclose(&actions, fd);
#else
		to_close.insert(fd);
#endif
	}
};

};

#ifndef HAVE_POSIX_SPAWN
static int
posix_spawn(pid_t *pid, const char *path, const spawn::file_actions &file_actions, const void *attrp, char *const argv[], char *const envp[])
{
	int child;

	if (attrp)
	{
		errno = ENOSYS;
		return -1;
	}

	switch (child = fork()) {
	case -1:
		return -1;
	case 0:
		for (auto &p : file_actions.to_dup)
			dup2(p.first, p.second);
		for (auto c : file_actions.to_close)
			close(c);
		execve(path, argv, envp);
		_exit(1);
	}

	*pid = child;
	return 0;
}

static int
posix_spawn(pid_t *pid, const char *path, const void *file_actions, const void *attrp, char *const argv[], char *const envp[])
{
	if (file_actions)
	{
		errno = ENOSYS;
		return -1;
	}
	return posix_spawn(pid, path, spawn::file_actions(), attrp, argv, envp);
}
#endif
