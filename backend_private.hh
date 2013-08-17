/*
 * Copyright (c) 2013 Pelle Johansson
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>

#include "backend.h"
#include "event_unhandled_exception.hh"
#include "smart_fd.hh"
#include "smart_event.hh"
#include "smart_evbuffer.hh"

class output_list
{
	typedef std::forward_list<backend_output> list_type;
	list_type list;
	list_type::iterator send_iter;
	list_type::iterator insert_iter;

public:
	output_list()
		: send_iter(list.end()), insert_iter(list.before_begin())
	{
	}

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

