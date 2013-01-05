/*
 * Copyright (c) 2013 Per Johansson
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

#include <boost/asio.hpp>

#include <signal.h>
#include <sys/wait.h>

#include "spawn.hh"
#include "smart_fd.hh"

#include "osx_system_idle.h"

#define STEREO_LINE ":stereo"
#define TV_LINE ":tv"

using boost::asio::io_service;
using boost::asio::posix::stream_descriptor;
using boost::asio::ip::tcp;

void reap(pid_t pid)
{
	pid_t r;
	int status;

	do
		r = waitpid(pid, &status, 0);
	while (r == -1 && errno == EINTR);

	if (r == -1)
		throw std::system_error(errno, std::system_category(), "waitpid");
}

smart_fd spawn_movactl(pid_t *pid = NULL)
{
	smart_pipe iopipe;
	extern char **environ;
        static const char *const args[] = { "movactl", STEREO_LINE, "listen", "volume", "power", "audio_source", NULL};

	spawn::file_actions actions;

	actions.add_stdout(iopipe.write);

	int err = posix_spawn(pid, "/usr/local/bin/movactl", actions, NULL, (char**)args, environ);

	if (err)
		throw std::system_error(err, std::system_category(), "posix_spawn");

	return std::move(iopipe.read);
}

void movactl_send(const std::vector<std::string> &args)
{
	std::vector<const char *> argv;
	pid_t pid;

	std::cout << "--movactl";
	argv.push_back("movactl");
	for (auto &a : args) {
		std::cout << " " << a;
		argv.push_back(a.c_str());
	}
	argv.push_back(NULL);
	std::cout << "\n";

	int err = posix_spawn(&pid, "/usr/local/bin/movactl", NULL, NULL, (char**)&argv[0], NULL);

	if (err)
		throw std::system_error(err, std::system_category(), "posix_spawn");

	reap(pid);
}

constexpr unsigned int fnv1a_hash(const char *str)
{
	return *str ? (fnv1a_hash(str + 1) ^ *str) * 16777619 : 2166136261;
}

std::string itos(int v)
{
	std::ostringstream ss;

	ss << v;
	return ss.str();
}

void power_on(const std::string &stereo, const std::string &tv)
{
	time_t now = time(NULL);
	struct tm tm = {0};

	localtime_r(&now, &tm);

	if (tm.tm_hour < 7)
		return;
	if (tm.tm_hour == 7 && tm.tm_min < 30)
		return;

	movactl_send({STEREO_LINE, "power", "on"});
	movactl_send({TV_LINE, "power", "on"});
	movactl_send({STEREO_LINE, "source", "select", stereo});
	movactl_send({TV_LINE, "source", "select", tv});
}

void switch_from(const std::string &from, const std::map<std::string, std::string> &values)
{
	auto it = values.find("audio_source");
	if (it == values.end())
	{
		std::cerr << "switch_from(" << from << "): No audio source\n";
		return;
	}
	if (it->second != from)
	{
		std::cerr << "switch_from(" << from << "): audio_source == " << it->second << "\n";
		return;
	}

	it = values.find("playstation");
	if (it != values.end() && it->second == "up")
	{
		movactl_send({STEREO_LINE, "source", "select", "vcr"});
		movactl_send({TV_LINE, "source", "select", "hdmi1"});
		return;
	}

	it = values.find("wii");
	if (it != values.end() && it->second == "on")
	{
		movactl_send({STEREO_LINE, "source", "select", "dss"});
		movactl_send({TV_LINE, "source", "select", "component"});
		return;
	}

	it = values.find("tv");
	if (it != values.end() && it->second == "on")
	{
		movactl_send({STEREO_LINE, "source", "select", "tv"});
		movactl_send({TV_LINE, "source", "select", "hdmi1"});
		return;
	}

	struct timespec idle;
	if (osx_system_idle(&idle) == 0 && idle.tv_sec <= 600)
	{
		movactl_send({STEREO_LINE, "source", "select", "dvd"});
		movactl_send({TV_LINE, "source", "select", "hdmi1"});
		return;
	}

	movactl_send({STEREO_LINE, "power", "off"});
}

void update(const std::string &line)
{
	std::string stat, value;
	static std::map<std::string, std::string> values;

	std::cout << "update: " << line << "\n";

	std::istringstream(line) >> stat >> value;

	if (value == "negotiating")
		return;

	auto elem = values.insert(make_pair(stat, value));
	if (!elem.second && elem.first->second == value)
	{
		std::cerr << "Not updating " << elem.first->first << ", " << value << " == " << elem.first->second << "\n";
		return;
	}
	elem.first->second = value;

	switch(fnv1a_hash(stat.c_str()))
	{
	case fnv1a_hash("volume"):
		movactl_send({TV_LINE, "volume", "value", itos(atoi(value.c_str()) + 72)});
		break;
	case fnv1a_hash("power"):
		movactl_send({TV_LINE, "power", value});
		break;
	case fnv1a_hash("audio_source"):
		switch(fnv1a_hash(value.c_str()))
		{
		case fnv1a_hash("tv"):
			movactl_send({STEREO_LINE, "volume", "value", "-32"});
			break;
		case fnv1a_hash("dvd"):
		case fnv1a_hash("vcr1"):
			movactl_send({STEREO_LINE, "volume", "value", "-37"});
			break;
		}
		break;
	case fnv1a_hash("playstation"):
		if (value == "up")
			power_on("vcr", "hdmi1");
		else
			switch_from("vcr1", values);
		break;
	case fnv1a_hash("tv"):
		if (value == "on")
			power_on("tv", "hdmi1");
		else
			switch_from("tv", values);
		break;
	case fnv1a_hash("wii"):
		if (value == "on")
			power_on("dss", "component");
		else
			switch_from("dss", values);
		break;
	}
}

std::unique_ptr<boost::asio::deadline_timer> psxtimer;

void update_playstation_timeout(const std::string &line, const boost::system::error_code &error)
{
	if (error)
	{
		std::cerr << "update_playstation_timeout: skipping " << line << " (" << error << ")\n";
		return;
	}
	update("playstation " + line);
}

void update_playstation(const std::string &line)
{
	psxtimer->expires_from_now(boost::posix_time::seconds(3));
	psxtimer->async_wait(std::bind(update_playstation_timeout, std::string(line), std::placeholders::_1));
}

template <typename T>
class input
{
public:
	T object;
	boost::asio::streambuf buffer;
	std::istream stream;
	bool active = false;
	const std::string ewhat;
	std::function<void(const std::string&)> update_fn;

	template <typename ...Args>
	input(std::string ewhat, std::function<void(const std::string&)> update_fn, Args &&...args)
		: object(args...), stream(&buffer), ewhat(ewhat), update_fn(update_fn)
	{
	}

	void activate()
	{
		if (!active)
		{
			boost::asio::async_read_until(object, buffer, '\n',
					std::bind(&input<T>::handle, this,
						std::placeholders::_1,
						std::placeholders::_2));
			active = true;
		}
	}

	void handle(const boost::system::error_code &error, std::size_t bytes)
	{
		active = false;
		if (error)
			throw boost::system::system_error(error, ewhat);

		std::string line;
		while (std::getline(stream, line).good())
			update_fn(line);
		stream.clear();

		activate();
	}
};

void connect_tcp(io_service &io, tcp::socket &socket, const tcp::resolver::query &query)
{
	tcp::resolver resolver(io);
	tcp::resolver::iterator iterator = resolver.resolve(query);
	tcp::resolver::iterator itend;
	boost::system::error_code ec;

	for ( ; iterator != itend ; iterator++)
	{
		if (!socket.connect(*iterator, ec))
			break;
	}
	if (iterator == itend)
		throw boost::system::system_error(ec);
}

int main()
{
	tcp::resolver::query psxquery("192.168.2.2", "7911");
	tcp::resolver::query wiitvquery("192.168.2.2", "7912");

	try
	{
		while (1)
		{
			pid_t movapid;
			smart_fd movafd = spawn_movactl(&movapid);

			io_service io;
			input<stream_descriptor> movainput("movactl", update, io, movafd);
			movafd.release();

			input<tcp::socket> psxinput("playstation", update_playstation, io);
			connect_tcp(io, psxinput.object, psxquery);
			//input<stream_descriptor> psxinput("playstation", update_playstation, io, STDIN_FILENO);

			input<tcp::socket> wiitvinput("wii/tv", update, io);
			connect_tcp(io, wiitvinput.object, wiitvquery);

			psxtimer.reset(new boost::asio::deadline_timer(io));

			try
			{

				while (1)
				{
					std::string line;

					movainput.activate();
					psxinput.activate();
					wiitvinput.activate();

					io.run();
					io.reset();
				}
			}
			catch (boost::system::system_error &e)
			{
				std::cerr << "err: " << e.what() << "\n";
			}

			kill(movapid, SIGTERM);
			reap(movapid);
		}
	}
	catch (std::exception &e)
	{
		std::cerr << "exception: " << e.what() << "\n";
	}
	return 1;
}

