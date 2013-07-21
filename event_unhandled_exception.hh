
#include <event.h>

#include <exception>

namespace event_unhandled_exception
{
	extern std::exception_ptr exception;

	static inline void
	handle()
	{
		exception = std::current_exception();
		event_loopbreak();
	}

	static inline void
	rethrow_if_set()
	{
		if (!exception)
			return;

		std::exception_ptr tmp = exception;
		exception = nullptr;
		std::rethrow_exception(tmp);
	}
}
