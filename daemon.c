
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/signal.h>
#include <unistd.h>
#include <string.h>

#include "line.h"
#include "status.h"

const char *line = "/dev/tty.usbserial";
const char *sock = "/tmp/marantz.sock";

struct ma_status status;
int line_fd;

int
main (int argc, char *argv[]) {
	char buf[256];
	int res;

	while (1) {
		line_fd = open_line (line, O_RDWR);
		if (line_fd < 0)
			err (1, "open_line");

		enable_auto_status_layer (line_fd, &status, 1);
		memset (&status, 0, sizeof (status));
		while ((res = read_line (line_fd, buf, sizeof (buf))) > 0) {
			fprintf (stderr, "Read line %s\n", buf);
			update_status (line_fd, &status, buf);
		}
		close (line_fd);
		if (res)
			err (1, "read_line");
		fprintf (stderr, "EOF, reopening after sleep\n");
		sleep (1);
	}
}
