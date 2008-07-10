
CFLAGS = -g -Wall -Werror -Wwrite-strings -Wshadow -Wpointer-arith -Wcast-align -Wsign-compare
CPPFLAGS += -I/usr/pkg/include
LDFLAGS += -g

CLI_PROG = morantz
CLI_OBJS = line.o command.o cli.o api_frontend.o serialize.o cli_notify.o

D_PROG = morantzd
D_OBJS = line.o status.o daemon.o api_backend.o serialize.o launchd.o
D_LIBS = -L/usr/pkg/lib -levent

LIB = libmorantz.dylib
LIB_OBJS = api_frontend.o serialize.o
LIB_LDFLAGS = -dynamiclib

all: $(CLI_PROG) $(D_PROG) $(LIB)

$(CLI_PROG): $(CLI_OBJS)
	$(CC) $(LDFLAGS) -o $(CLI_PROG) $(CLI_OBJS)

$(D_PROG): $(D_OBJS)
	$(CC) $(LDFLAGS) -o $(D_PROG) $(D_OBJS) ${D_LIBS}

$(LIB): $(LIB_OBJS)
	$(CC) $(LDFLAGS) $(LIB_LDFLAGS) -o $(LIB) $(LIB_OBJS)

api_backend.o: backend_command.h
cli_notify.o: notify_command.h

%.h: %.gperf
	gperf --output-file=$@ --hash-function-name=$(basename $@)_hash --lookup-function-name=$(basename $@) --enum --switch=1 $<

depend:
	mkdep $(CPPFLAGS) *.c

clean:
	rm *.o

-include .depend
