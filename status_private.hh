#ifndef STATUS_PRIVATE_HH
#define STATUS_PRIVATE_HH

#include <forward_list>
#include <string>

#include "status.hh"
#include "backend_private.hh"

struct status_notify_info
{
	class status &status;
	std::string code;
	backend_ptr::notify_cb cb;

	status_notify_info(class status &status, std::string code, backend_ptr::notify_cb cb)
		: status(status), code(std::move(code)), cb(std::move(cb))
	{
	}

	bool
	operator == (const status_notify_info &r)
	{
		return this == &r;
	}
};

class status : public backend_device
{
private:
	std::forward_list<status_notify_info> notify_chain;

public:
	status(backend_ptr &ptr, std::string name, std::string line, std::string client, int throttle);
	virtual ~status();

	std::unique_ptr<status_notify_token> start_notify(const std::string &code, backend_ptr::notify_cb cb);
	void stop_notify(struct status_notify_info &ptr);

protected:
	void notify(const std::string &code, int val);
	void notify(const std::string &code, const std::string &val);
};

#endif /*STATUS_PRIVATE_HH*/
