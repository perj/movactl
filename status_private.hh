#ifndef STATUS_PRIVATE_HH
#define STATUS_PRIVATE_HH

#include <forward_list>
#include <string>

#include "status.h"

struct status_notify_info
{
	status &status;
	std::string code;
	status_ptr::notify_cb cb;

	status_notify_info(struct status &status, std::string code, status_ptr::notify_cb cb)
		: status(status), code(std::move(code)), cb(std::move(cb))
	{
	}

	bool
	operator == (const status_notify_info &r)
	{
		return this == &r;
	}
};

struct status
{
protected:
	backend_device &bdev;
private:
	std::forward_list<status_notify_info> notify_chain;

public:
	status(backend_device &bdev);
	virtual ~status();

	status_notify_token_t start_notify(const std::string &code, status_ptr::notify_cb cb);
	void stop_notify(status_notify_token_t token);

	virtual void status_setup() = 0;
	virtual const char *packet_separators() const = 0;
	virtual void update_status(const std::string &packet, const struct backend_output *inptr) = 0;
	virtual int send_status_request(const std::string &code) = 0;
	virtual int query_command(const std::string &code) const = 0;
	virtual int query_status(const std::string &code) const = 0;
	virtual int query(const std::string &code, std::string &out_buf) = 0;
	virtual void send_command(const std::string &cmd, const std::vector<int32_t> &args) = 0;

protected:
	void notify(const std::string &code, int val);
	void notify(const std::string &code, const std::string &val);
};

#endif /*STATUS_PRIVATE_HH*/
