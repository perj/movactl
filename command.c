
#include <stdio.h>

#include "line.h"

#define SIMPLE_COMMAND(name, code, arg) \
	int \
	ma_command_ ## name (int fd) { \
		int res = send_command (fd, code, arg); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#define SIGNINT_COMMAND(name, code, prefix) \
	int \
	ma_command_ ## name (int fd, int value) { \
		char valbuf[13]; \
		int res; \
 \
		snprintf(valbuf, sizeof(valbuf), prefix "%+d", value); \
		res = send_command (fd, code, valbuf); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#define UINT_COMMAND(name, code, prefix, width) \
	int \
	ma_command_ ## name (int fd, unsigned int value) { \
		char valbuf[13]; \
		int res; \
 \
		snprintf(valbuf, sizeof(valbuf), prefix "%0*u", width, value); \
		res = send_command (fd, code, valbuf); \
		if (res < 0) \
			return -1; \
		return 0; \
	}

#include "command.h"
