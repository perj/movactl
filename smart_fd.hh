
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

