# Copyright (c) 2008 Pelle Johansson <pelle@morth.org>
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

CFLAGS = -g -Wall -Werror -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-align -Wsign-compare
CPPFLAGS += -I/usr/pkg/include -I/opt/local/include
LDFLAGS += -g

CLI_PROG = movactl
CLI_OBJS = cli.o base64.o cli_notify.o complete.o
CLI_LIBS = -L/usr/pkg/lib -L/opt/local/lib -levent

D_PROG = movactld
D_OBJS = line.o status.o daemon.o backend.o launchd.o api_serverside.o base64.o
D_OBJS+= marantz_status.o marantz_command.o
D_OBJS+= lge_status.o
D_LIBS = -L/usr/pkg/lib -L/opt/local/lib -levent

D_LAUNCHD_PLIST=/Library/LaunchDaemons/org.morth.per.${D_PROG}.plist
VN_LAUNCHD_PLIST=/Library/LaunchAgents/org.morth.per.volumenotifier.plist

#LIB = libmovactl.dylib
#LIB_OBJS = marantz_notify.o serialize.o
#LIB_LDFLAGS = -dynamiclib

all: $(CLI_PROG) $(D_PROG) $(LIB)

$(CLI_PROG): $(CLI_OBJS)
	$(CC) $(LDFLAGS) -o $(CLI_PROG) $(CLI_OBJS) ${CLI_LIBS}

$(D_PROG): $(D_OBJS)
	$(CC) $(LDFLAGS) -o $(D_PROG) $(D_OBJS) ${D_LIBS}

#$(LIB): $(LIB_OBJS)
#	$(CC) $(LDFLAGS) $(LIB_LDFLAGS) -o $(LIB) $(LIB_OBJS)

api_serverside.o: api_serverside_command.h
cli.o: all_commands.h
cli_notify.o: all_notify.h

all_commands.h: marantz_command.h lge_command.h marantz_precommand.c lge_precommand.c
	$(CC) -E marantz_precommand.c lge_precommand.c | sed -n -e 's/ ## //g' -e 's/" *"//g' -e 's/  */ /g' -e '/COMMAND/p' | sort -u > $@

all_notify.h: marantz_notify.h lge_notify.h marantz_prenotify.c lge_prenotify.c
	$(CC) -E marantz_prenotify.c lge_prenotify.c | sed -n -e 's/ ## //g' -e 's/" *"//g' -e 's/  */ /g' -e '/NOTIFY/p' | sort -u > $@

%.h: %.gperf
	gperf --output-file=$@ --hash-function-name=$(basename $@)_hash --lookup-function-name=$(basename $@) --enum --switch=1 $<

depend:
	mkdep $(CPPFLAGS) *.c

clean:
	rm *.o

install: all
	mkdir -p /usr/local/bin
	mkdir -p /usr/local/sbin
	install "${D_PROG}" /usr/local/sbin/
	install "${CLI_PROG}" /usr/local/bin/
	install volumenotifier.sh /usr/local/sbin/volumenotifier
	test -f "${D_LAUNCHD_PLIST}" || install -m 644 "${D_PROG}.plist" "${D_LAUNCHD_PLIST}"
	test -f "${VN_LAUNCHD_PLIST}" || install -m 644 volumenotifier.plist "${VN_LAUNCHD_PLIST}"
	launchctl load "${D_LAUNCHD_PLIST}"
	launchctl load -S Aqua "${VN_LAUNCHD_PLIST}"

-include .depend
