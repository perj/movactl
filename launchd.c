
#include <launch.h>
#include <sys/types.h>
#include <event.h>
#include <err.h>
#include <errno.h>

#include "backend.h"

static struct event launchd_ev;

extern int running;

static void
launchd_handle (launch_data_t msg) {
	if (launch_data_get_type (msg) != LAUNCH_DATA_DICTIONARY) {
		warnx ("launchd_handle: msg not dictionary");
		launch_data_free (msg);
		return;
	}

	launch_data_t sockets = launch_data_dict_lookup (msg, LAUNCH_JOBKEY_SOCKETS);

	if (sockets) {
		launch_data_t clients = launch_data_dict_lookup (sockets, "client");

		if (clients) {
			int num = launch_data_array_get_count (clients);
			int i;

			for (i = 0 ; i < num ; i++) {
				launch_data_t client = launch_data_array_get_index (clients, i);
				int fd = launch_data_get_fd (client);

				if (fd < 0) {
					warn ("launchd_handle: get_fd");
					continue;
				}
				
				if (!backend_listen_fd (fd, NULL))
					warn ("backend_listen_fd");
			}
		}
	} else
		warnx ("launchd_handle: no sockets");

	launch_data_free (msg);
}

static void
launchd_event (int fd, short what, void *cbarg) {
	launch_data_t msg = launch_msg (NULL);

	if (!msg) {
		if (errno)
			err (1, "launchd_handle");
		return;
	}

	launchd_handle (msg);
}

int
launchd_init (void) {
	launch_data_t checkin = launch_data_new_string (LAUNCH_KEY_CHECKIN);
	launch_data_t response;
	int fd;

	if (!checkin)
		errx (1, "launchd_init: Failed to create check in request");

	response = launch_msg (checkin);
	launch_data_free (checkin);

	if (!response)
		err (1, "launchd_init: No response");

	if (launch_data_get_type (response) == LAUNCH_DATA_ERRNO)
		errc (1, launch_data_get_errno (response), "launchd_init: Error checking in");

	fd = launch_get_fd ();
	if (fd < 0)
		err (1, "launchd_init: get_fd");

	event_set (&launchd_ev, fd, EV_READ | EV_PERSIST, launchd_event, NULL);
	event_add (&launchd_ev, NULL);

	launchd_handle (response);

	return 0;
}
