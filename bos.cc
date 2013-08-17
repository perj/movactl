
#include <iostream>
#include <system_error>

#include <signal.h>
#include <stdlib.h>
#include <string.h>

#include <sys/wait.h>

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

