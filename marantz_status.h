/*
 * Copyright (c) 2008 Pelle Johansson
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

#ifndef MARANTZ_STATUS_H
#define MARANTZ_STATUS_H

#include <limits.h>
#include <stdint.h>

#include "status.hh"

#define MAVOL_MIN INT_MIN

enum {
#define NOTIFY(name, code, type) ST_KNOW_BIT_ ## name,
#define STATUS(name, code, type) /* can't know */
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS
#define NOTIFY(name, code, type) ST_KNOW_ ## name = 1LL << ST_KNOW_BIT_ ## name,
#define STATUS(name, code, type) /* can't know */
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS
};

#include "status_private.hh"

struct ma_status : public status
{
#define NOTIFY(name, code, type) status_ ## type ## _t name;
#define STATUS(name, code, type) status_ ## type ## _t name;
#include "marantz_notify.h"
#undef NOTIFY
#undef STATUS

	status_bool_t auto_status_feedback_layer[4];

	uint64_t known_fields = 0;

	ma_status(backend_ptr &ptr, std::string name, std::string line, std::string client, int throttle);

	virtual void open();

	virtual const char *packet_separators() const;
	virtual void update_status(const std::string &packet, const struct backend_output *inptr);
	virtual int send_status_request(const std::string &code);
	virtual int query_command(const std::string &code) const;
	virtual int query_status(const std::string &code) const;
	virtual int query(const std::string &code, std::string &out_buf);
	virtual void send_command(const std::string &cmd, const std::vector<int32_t> &args);

	void enable_auto_status_layer(int layer);

#define INFO(name, code, level, id) void update_##id(const struct ma_info *info, const std::string &arg);
#define INFO_KNOW(name, code, level, know, id) void update_##id(const struct ma_info *info, const std::string &arg);
#define INFO_ACK_ONLY(name, code)
#define INFO_NO_AUTO(name, code, id) void update_##id(const struct ma_info *info, const std::string &arg);
#define INFO_KNOW_NO_AUTO(name, code, know, id) void update_##id(const struct ma_info *info, const std::string &arg);
#define NO_INFO(name, code, level)
#define INFO_CMD_ONLY(name, code, id) void update_##id(const struct ma_info *info, const std::string &arg);
#include "marantz_info.h"
#undef INFO
#undef INFO_KNOW
#undef INFO_ACK_ONLY
#undef INFO_NO_AUTO
#undef INFO_KNOW_NO_AUTO
#undef NO_INFO
#undef INFO_CMD_ONLY
};

class status *marantz_creator(backend_ptr &ptr, std::string name, std::string line, std::string client, int throttle);

#endif /*MARANTZ_STATUS_H*/
