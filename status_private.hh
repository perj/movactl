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
