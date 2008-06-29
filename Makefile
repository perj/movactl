
CFLAGS = -Wall

CLI_PROG = marantz
CLI_OBJS = line.o command.o cli.o

D_PROG = marantzd
D_OBJS = line.o status.o daemon.o

LIB = libmarantz.dylib
LIB_OBJS = api_frontend.o
LIB_LDFLAGS = -dynamiclib

all: $(CLI_PROG) $(D_PROG) $(LIB)

$(CLI_PROG): $(CLI_OBJS)
	$(CC) $(LDFLAGS) -o $(CLI_PROG) $(CLI_OBJS)

$(D_PROG): $(D_OBJS)
	$(CC) $(LDFLAGS) -o $(D_PROG) $(D_OBJS)

$(LIB): $(LIB_OBJS)
	$(CC) $(LDFLAGS) $(LIB_LDFLAGS) -o $(LIB) $(LIB_OBJS)

depend:
	mkdep *.c

-include .depend
