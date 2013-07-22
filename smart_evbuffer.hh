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
