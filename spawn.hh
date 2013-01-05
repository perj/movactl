
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
