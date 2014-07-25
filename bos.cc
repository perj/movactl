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

#include <iostream>
#include <system_error>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>
#include <unistd.h>

static pid_t
spawn()
{
	pid_t child = fork();

	if (child == -1)
		throw std::system_error(errno, std::system_category(), "fork");
	return child;
}

static bool
run(pid_t child)
{
	sigset_t sigset;
	sigfillset(&sigset);

	for (;;) {
		int sig;

		if (sigwait(&sigset, &sig)) {
			kill(child, SIGTERM);
			throw std::system_error(errno, std::system_category(), "sigwait");
		}

		std::cerr << "BOS(" << getpid() << ") received signal " << strsignal(sig) << "\n";

		switch (sig) {
		case SIGCHLD:
			return false;
		case SIGINT:
		case SIGTERM:
			if (kill(child, sig))
				throw std::system_error(errno, std::system_category(), "sigwait");
			return true;
		default:
			kill(child, sig);
			break;
		}
	}
}

static std::pair<bool, int>
reap(pid_t child)
{
	for (int state = 0 ; ; state++) {
		int status;
		switch (waitpid(child, &status, WNOHANG)) {
		case -1:
			throw std::system_error(errno, std::system_category(), "waitpid");
		case 0:
			break;
		default:
			return {true, status};
		}

		switch (state) {
		default:
			sleep(2);
			break;
		case 2:
			std::cerr << "BOS(" << getpid() << ") sending TERM to exiting child " << child << "\n";
			kill(child, SIGTERM);
			sleep(2);
			break;
		case 5:
			std::cerr << "BOS(" << getpid() << ") sending INT to exiting child " << child << "\n";
			kill(child, SIGINT);
			sleep(2);
			break;
		case 8:
			std::cerr << "BOS(" << getpid() << ") sending KILL to exiting child " << child << "\n";
			kill(child, SIGKILL);
			sleep(2);
			break;
		case 12:
			std::cerr << "BOS(" << getpid() << ") abandonning child " << child << "\n";
			return {false, 0};
		}
	}
}

void
bos()
{
	int exit_code = 0;
	bool exiting = false;
	do {
		pid_t child = spawn();
		if (child == 0)
			return;

		std::cerr << "BOS(" << getpid() << ") starting for pid " << child << "\n";

		exiting = run(child);

		std::cerr << "BOS(" << getpid() << ") child " << child << " exiting\n";

		auto status = reap(child);

		if (status.first) {
			if (WIFEXITED(status.second)) {
				std::cerr << "BOS(" << getpid() << ") child " << child << " exited with code " << WEXITSTATUS(status.second) << "\n";
				exit_code = WEXITSTATUS(status.second);
				if (exit_code == 0)
					exiting = true;
			} else if (WIFSIGNALED(status)) {
				std::cerr << "BOS(" << getpid() << ") child " << child << " exited with signal " << strsignal(WTERMSIG(status.second));
				if (WCOREDUMP(status.second))
					std::cerr << " (core dumped)\n";
				else
					std::cerr << "\n";
				exit_code = 128 + WTERMSIG(status.second);
			}
		} else {
			exit_code = 255;
		}
	} while (!exiting);

	std::cerr << "BOS(" << getpid() << ") exiting with code " << exit_code << "\n";
	exit(exit_code);
}

