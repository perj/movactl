
#include <event.h>

#include <memory>

#include "smart_fd.hh"

/* XXX bases */

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
		static_cast<smart_bufferevent*>(cbarg)->read_callback();
	}

	static void
	write_callback_wrapper(struct bufferevent *be, void *cbarg)
	{
		static_cast<smart_bufferevent*>(cbarg)->write_callback();
	}

	static void
	error_callback_wrapper(struct bufferevent *be, short what, void *cbarg)
	{
		static_cast<smart_bufferevent*>(cbarg)->error_callback(what);
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
