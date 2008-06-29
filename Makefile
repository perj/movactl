
CFLAGS = -Wall

PROG = marantz
OBJS = line.o status.o command.o cli.o api_frontend.o

$(PROG): $(OBJS)
	   $(CC) $(LDFLAGS) -o $(PROG) $(OBJS)

-include .depend
