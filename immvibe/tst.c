#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "immvibeclient.h"


static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s p <force>     -- force 0 to 255\n", progname);
	fprintf(stderr, "       %s s             -- stop vibration\n", progname);
	fprintf(stderr, "       %s A             -- enable amp\n", progname);
	fprintf(stderr, "       %s a             -- disable amp\n", progname);
	fprintf(stderr, "       %s n             -- get number of actuators\n", progname);
	fprintf(stderr, "       %s d             -- get current debug level\n", progname);
}


int main(int argc, char *argv[])
{
	struct immvibe_handle *fd = NULL;
	int ret = 0;

	if (argc == 1 || argc > 3) {
		goto bail_usage;
	}

	fd = immvibe_open(&ret);
	if (!fd) {
		fprintf(stderr, "%s: immvibe_open failed: errno %d\n", argv[0], ret);
		ret = 99;
		goto bail;
	}

	if (!strcmp(argv[1], "p")) {
		if (argc != 3) {
			goto bail_usage;
		}

		int force = atoi(argv[2]);
		if (force < 0 || force > 255) {
			goto bail_usage;
		}

		ret = immvibe_play(fd, force);
	} else if (!strcmp(argv[1], "s")) {
		if (argc != 2) {
			goto bail_usage;
		}

		ret = immvibe_stop_kernel_timer(fd);
	} else if (!strcmp(argv[1], "A")) {
		if (argc != 2) {
			goto bail_usage;
		}

		ret = immvibe_set_amp_enabled(fd, 1);
	} else if (!strcmp(argv[1], "a")) {
		if (argc != 2) {
			goto bail_usage;
		}

		ret = immvibe_set_amp_enabled(fd, 0);
	} else if (!strcmp(argv[1], "n")) {
		if (argc != 2) {
			goto bail_usage;
		}

		ret = immvibe_get_num_actuators(fd);
	}

	if (!strcmp(argv[1], "d")) {
		if (argc != 2) {
			goto bail_usage;
		}

		ret = immvibe_get_debug_level(fd);
	}

	if (ret < 0) {
		fprintf(stderr, "immvibe call failed: %d\n", ret);
		ret = 1;
	} else {
		printf("%d\n", ret);
		ret = 0;
	}

	immvibe_close(fd);
	return ret;

bail_usage:
	usage(argv[0]);
	ret = 127;

bail:
	immvibe_close(fd);
	return ret;
}
