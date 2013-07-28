
#ifndef EVENT_UNHANDLED_EXCEPTION_HH
#define EVENT_UNHANDLED_EXCEPTION_HH

#include <event.h>

#include <exception>

namespace event_unhandled_exception
{
	extern std::exception_ptr exception;

	inline void
	handle()
	{
		exception = std::current_exception();
		event_loopbreak();
	}

	inline void
	rethrow_if_set()
	{
		if (!exception)
			return;

		std::exception_ptr tmp = exception;
		exception = nullptr;
		std::rethrow_exception(tmp);
	}
}

#endif /*EVENT_UNHANDLED_EXCEPTION_HH*/
