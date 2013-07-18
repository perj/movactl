# Copyright (c) 2013 Per Johansson
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

cmake_minimum_required(VERSION 2.8)
project(movactl)

set(WARNFLAGS "-Wall -Werror -Wwrite-strings -Wpointer-arith -Wcast-align -Wsign-compare -Wno-static-in-inline")
set(CMAKE_C_FLAGS "-g ${WARNFLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "-g -std=c++11 -stdlib=libc++ ${WARNFLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(movactl cli.c base64.c cli_notify.c complete.c all_commands.h all_notify.h)
target_link_libraries(movactl event)

add_executable(movactld line.c status.c daemon.c backend.cc launchd.c api_serverside.cc base64.c
		marantz_status.c marantz_command.c lge_status.c backend_type.h api_serverside_command.h)
target_link_libraries(movactld event)

add_executable(listener listener.cc osx_system_idle.c)
target_link_libraries(listener boost_system-mt "-framework IOKit" "-framework CoreFoundation")

add_custom_command(OUTPUT all_commands.h
		COMMAND $(CC) -E "${CMAKE_CURRENT_SOURCE_DIR}/marantz_precommand.c" "${CMAKE_CURRENT_SOURCE_DIR}/lge_precommand.c" | sed -n -e 's/ \#\# //g' -e 's/" *"//g' -e 's/  */ /g' -e '/COMMAND/p' | sort -u > all_commands.h
		DEPENDS marantz_command.h lge_command.h marantz_precommand.c lge_precommand.c
		)

add_custom_command(OUTPUT all_notify.h
		COMMAND $(CC) -E "${CMAKE_CURRENT_SOURCE_DIR}/marantz_prenotify.c" "${CMAKE_CURRENT_SOURCE_DIR}/lge_prenotify.c" | sed -n -e 's/ \#\# //g' -e 's/" *"//g' -e 's/  */ /g' -e '/NOTIFY/p' | sort -u > all_notify.h
		DEPENDS marantz_notify.h lge_notify.h marantz_prenotify.c lge_prenotify.c
		)

file(GLOB_RECURSE gperf_files "*.gperf")
foreach(_file ${gperf_files})
	get_filename_component(_base ${_file} NAME_WE)
	add_custom_command(OUTPUT ${_base}.h
			COMMAND gperf -L ANSI-C --output-file=${_base}.h --hash-function-name=${_base}_hash --lookup-function-name=${_base} --enum --switch=1 ${_file}
			DEPENDS ${_file}
			)
endforeach()
