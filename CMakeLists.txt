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

if(STAGING_DIR)
	set(CXX_STD "-std=gnu++0x")
	set(EXTRAWARN "-Wno-cast-align")
	set(OPT "-Os -fvisibility=hidden")
	SET(LIBEVENT "${target}/usr/lib/libevent_core.so")
else()
	set(CXX_STD "-std=c++11 -stdlib=libc++")
	SET(EXTRAWARN "-Wno-static-in-inline")
	set(OPT "-Os -fvisibility=hidden -flto")
	SET(LIBEVENT "event")
endif()

set(WARNFLAGS "-Wall -Werror -Wwrite-strings -Wpointer-arith -Wcast-align -Wsign-compare ${EXTRAWARN}")
set(CMAKE_C_FLAGS "-g ${OPT} ${WARNFLAGS} -Wshadow")
set(CMAKE_CXX_FLAGS "-g ${OPT} ${CXX_STD} ${WARNFLAGS}")

set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(movactl cli.c base64.c cli_notify.c complete.c all_commands.h all_notify.h)
target_link_libraries(movactl ${LIBEVENT})

add_executable(movactld line.c status.cc daemon.cc backend.cc launchd.c api_serverside.cc base64.c
		marantz_status.cc marantz_command.cc lge_status.cc backend_type.h api_serverside_command.h)
target_link_libraries(movactld ${LIBEVENT})

if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	set(idle_source "osx_system_idle.c")
	add_definitions(-DIDLE=osx_system_idle -DGETPROGNAME="getprogname()")
else()
	add_definitions(-DGETPROGNAME="movactl")
endif()
add_executable(listener listener.cc ${idle_source})
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
	target_link_libraries(listener boost_system-mt "-framework IOKit" "-framework CoreFoundation")
else()
	target_link_libraries(listener ${target}/usr/lib/libboost_system.a pthread)
endif()

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
