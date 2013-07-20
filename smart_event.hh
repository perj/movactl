
#include <event.h>

#include <functional>
#include <memory>

#include "smart_fd.hh"

/* XXX bases */

template <void (*unhandled_exception)()>
class smart_event
{
	struct event ev;
	std::shared_ptr<smart_fd> fd;
	std::function<void (evutil_socket_t, short)> callback;

	static void
	callback_wrapper(evutil_socket_t fd, short what, void *cbarg)
	{
		try {
			static_cast<smart_event*>(cbarg)->callback(fd, what);
		} catch (...) {
			unhandled_exception();
		}
	}

public:
	smart_event()
	{
		memset(&ev, 0, sizeof(ev));
	}

	smart_event(const smart_event&) = delete;
	smart_event &operator =(const smart_event &) = delete;

	~smart_event()
	{
		if (event_initialized(&ev))
			event_del(&ev);
	}

	void set_fd(std::shared_ptr<smart_fd> fd)
	{
		this->fd = std::move(fd);
	}

	void set_fd(int fd)
	{
		this->fd = std::make_shared<smart_fd>(fd);
	}

	void set(short what, decltype(callback) cb)
	{
		callback = cb;
		event_set(&ev, *fd, what, callback_wrapper, this);
	}

	void set_signal(int signum, decltype(callback) cb)
	{
		callback = cb;
		signal_set(&ev, signum, callback_wrapper, this);
	}

	int add()
	{
		return event_add(&ev, NULL);
	}

	int add(const struct timeval &timeout)
	{
		return event_add(&ev, &timeout);
	}

	bool pending(short what, struct timeval *out_timeout = NULL)
	{
		return event_pending(&ev, what, out_timeout);
	}

	void del()
	{
		event_del(&ev);
	}

	void reset()
	{
		if (event_initialized(&ev))
			event_del(&ev);
		memset(&ev, 0, sizeof(ev));
		fd.reset();
	}
};
