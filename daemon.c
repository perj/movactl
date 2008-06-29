
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>

#include "line.h"
#include "status.h"

const char *line = "/dev/tty.usbserial";

struct ma_status status;

int
main (int argc, char *argv[]) {
	int fd;
	char buf[256];
	int res;

	while (1) {
		fd = open_line (line, O_RDWR);
		if (fd < 0)
			err (1, "open_line");

		enable_auto_status_layer (fd, &status, 1);
		memset (&status, 0, sizeof (status));
		while ((res = read_line (fd, buf, sizeof (buf))) > 0) {
			fprintf (stderr, "Read line %s\n", buf);
			update_status (fd, &status, buf);
		}
		close (fd);
		if (res)
			err (1, "read_line");
		fprintf (stderr, "EOF, reopening after sleep\n");
		sleep (1);
	}
}
