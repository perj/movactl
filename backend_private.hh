
#include <string>

#include "backend.h"
#include "event_unhandled_exception.hh"
#include "smart_fd.hh"
#include "smart_event.hh"
#include "smart_evbuffer.hh"

class output_list
{
	std::forward_list<backend_output> list;
	decltype(list)::iterator send_iter = list.end();
	decltype(list)::iterator insert_iter = list.before_begin();

public:
	decltype(send_iter) to_send()
	{
		if (send_iter == list.end())
			return send_iter;
		return send_iter++;
	}

	const backend_output *inptr()
	{
		if (list.empty() || send_iter == list.begin())
			return NULL;
		return &list.front();
	}

	void push(backend_output o)
	{
		insert_iter = list.insert_after(insert_iter, std::move(o));
		if (send_iter == list.end())
			send_iter = insert_iter;
	}

	void pop()
	{
		if (insert_iter == list.begin())
			insert_iter = list.before_begin();
		list.pop_front();
	}

	void clear()
	{
		list.clear();
		send_iter = list.end();
		insert_iter = list.before_begin();
	}

	decltype(list.end()) end()
	{
		return list.end();
	}
};

class backend_device {
public:
	backend_ptr &ptr;

	std::string name;

	std::string line;
	smart_fd line_fd;
	smart_event<event_unhandled_exception::handle> read_ev;
	smart_event<event_unhandled_exception::handle> write_ev;

	smart_evbuffer input;
	output_list output;
	struct timeval out_throttle;

	std::string client;

	backend_device(backend_ptr &ptr, std::string name, std::string line, std::string client, int throttle);

	virtual void open();
	virtual void close();

	void listen(int fd);
	void listen_to_client();

	void send(const struct timeval *throttle, const char *fmt, va_list ap);
	void send(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
	void send_throttle(const struct timeval *throttle, const char *fmt, ...) __attribute__((format(printf, 3, 4)));
	void remove_output(const struct backend_output **inptr);

	static backend_device &impl(backend_ptr &ptr);
	static void create(std::string name, const backend_ptr::creator &creator,
			std::string line, std::string client, int throttle);

	virtual std::unique_ptr<status_notify_token> start_notify(const std::string &code, backend_ptr::notify_cb cb) = 0;
	virtual void stop_notify(struct status_notify_info &ptr) = 0;

	virtual const char *packet_separators() const = 0;
	virtual void update_status(const std::string &packet, const struct backend_output *inptr) = 0;
	virtual int send_status_request(const std::string &code) = 0;
	virtual int query_command(const std::string &code) const = 0;
	virtual int query_status(const std::string &code) const = 0;
	virtual int query(const std::string &code, std::string &out_buf) = 0;
	virtual void send_command(const std::string &cmd, const std::vector<int32_t> &args) = 0;
private:
	void readcb(short what);
	void writecb();
};

