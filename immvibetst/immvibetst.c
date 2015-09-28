#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>

#define MODULE_NAME                         "tspdrv"
#define TSPDRV                              "/dev/"MODULE_NAME
#define TSPDRV_MAGIC_NUMBER                 0x494D4D52
#define TSPDRV_IOCTL_GROUP                  0x52
#define TSPDRV_STOP_KERNEL_TIMER            _IO(TSPDRV_IOCTL_GROUP, 1) /* obsolete, may be removed in future */
#define TSPDRV_SET_MAGIC_NUMBER             _IO(TSPDRV_IOCTL_GROUP, 2)
#define TSPDRV_ENABLE_AMP                   _IO(TSPDRV_IOCTL_GROUP, 3)
#define TSPDRV_DISABLE_AMP                  _IO(TSPDRV_IOCTL_GROUP, 4)
#define TSPDRV_GET_NUM_ACTUATORS            _IO(TSPDRV_IOCTL_GROUP, 5)
#define TSPDRV_SET_DEVICE_PARAMETER         _IO(TSPDRV_IOCTL_GROUP, 6)
#define TSPDRV_SET_DBG_LEVEL                _IO(TSPDRV_IOCTL_GROUP, 7)
#define TSPDRV_GET_DBG_LEVEL                _IO(TSPDRV_IOCTL_GROUP, 8)
#define TSPDRV_SET_RUNTIME_RECORD_FLAG      _IO(TSPDRV_IOCTL_GROUP, 9)
#define TSPDRV_GET_RUNTIME_RECORD_FLAG      _IO(TSPDRV_IOCTL_GROUP, 10)
#define TSPDRV_SET_RUNTIME_RECORD_BUF_SIZE  _IO(TSPDRV_IOCTL_GROUP, 11)
#define TSPDRV_GET_RUNTIME_RECORD_BUF_SIZE  _IO(TSPDRV_IOCTL_GROUP, 12)
#define TSPDRV_GET_PARAM_FILE_ID            _IO(TSPDRV_IOCTL_GROUP, 13)

static void usage(const char *progname)
{
	fprintf(stderr, "usage: %s p <strength>  -- strength 0 to 255\n", progname);
	fprintf(stderr, "       %s s             -- stop vibration\n", progname);
	fprintf(stderr, "       %s A             -- enable amp\n", progname);
	fprintf(stderr, "       %s a             -- disable amp\n", progname);
	fprintf(stderr, "       %s n             -- get number of actuators\n", progname);
	fprintf(stderr, "       %s d             -- get current debug level\n", progname);
}


static int open_driver(void)
{
	int fd;
	long magic = TSPDRV_MAGIC_NUMBER;

	fd = open(TSPDRV, O_WRONLY);
	if (fd < 0) {
		perror("open_driver: open");
		return -1;
	}

	if (ioctl(fd, TSPDRV_SET_MAGIC_NUMBER, &magic)) {
		perror("open_driver: ioctl");
		close(fd);
		return -1;
	}

	return fd;
}


static int do_play(int strength)
{
	int fd = open_driver();
	int ret = 0;

	if (fd < 0) {
		ret = 10;
		goto bail;
	}

	unsigned char buf[4];
	buf[0] = buf[1] = buf[2] = 0;
	buf[3] = (unsigned char) strength;

	// while(1) {
		if (!write(fd, &buf, sizeof(buf))) {
			fprintf(stderr, "do_play: write failed\n");

			ret = 11;
			goto bail;
		}
	// }

bail:
	if (fd >= 0) {
		close(fd);
	}

	return ret;
}


static int do_stop(void)
{
	int fd = open_driver();
	int ret = 0;

	if (fd < 0) {
		ret = 20;
		goto bail;
	}

	if (ioctl(fd, TSPDRV_STOP_KERNEL_TIMER, NULL)) {
		perror("do_stop: ioctl");
		ret = 21;
		goto bail;
	}

bail:
	if (fd >= 0) {
		close(fd);
	}

	return ret;
}


static int do_num_actuators(void)
{
	int fd = open_driver();
	int ret = 0;
	int result;

	if (fd < 0) {
		ret = 30;
		goto bail;
	}

	result = ioctl(fd, TSPDRV_GET_NUM_ACTUATORS, NULL);
	if (result < 0) {
		perror("do_stop: ioctl");
		ret = 31;
		goto bail;
	}

	printf("%d\n", result);

bail:
	if (fd >= 0) {
		close(fd);
	}

	return ret;
}


static int do_debug_level(void)
{
	int fd = open_driver();
	int ret = 0;
	int result;

	if (fd < 0) {
		ret = 40;
		goto bail;
	}

	result = ioctl(fd, TSPDRV_GET_DBG_LEVEL, NULL);
	if (result < 0) {
		perror("do_stop: ioctl");
		ret = 41;
		goto bail;
	}

	printf("%d\n", result);

bail:
	if (fd >= 0) {
		close(fd);
	}

	return ret;
}


static int do_set_amp(int enable)
{
	int fd = open_driver();
	int ret = 0;
	int result;

	if (fd < 0) {
		ret = 50;
		goto bail;
	}

	result = ioctl(fd, enable ? TSPDRV_ENABLE_AMP : TSPDRV_DISABLE_AMP, NULL);
	if (result < 0) {
		perror("do_stop: ioctl");
		ret = 41;
		goto bail;
	}

	printf("%d\n", result);

bail:
	if (fd >= 0) {
		close(fd);
	}

	return ret;
}


int main(int argc, char *argv[])
{
	if (argc == 1 || argc > 3) {
		usage(argv[0]);
		return 1;
	}

	if (!strcmp(argv[1], "p")) {
		if (argc != 3) {
			usage(argv[0]);
			return 2;
		}

		int strength = atoi(argv[2]);
		if (strength < 0 || strength > 255) {
			usage(argv[0]);
			return 3;
		}

		return do_play(strength);
	}

	if (!strcmp(argv[1], "s")) {
		if (argc != 2) {
			usage(argv[0]);
			return 4;
		}

		return do_stop();
	}

	if (!strcmp(argv[1], "A")) {
		if (argc != 2) {
			usage(argv[0]);
			return 4;
		}

		return do_set_amp(1);
	}

	if (!strcmp(argv[1], "a")) {
		if (argc != 2) {
			usage(argv[0]);
			return 4;
		}

		return do_set_amp(0);
	}

	if (!strcmp(argv[1], "n")) {
		if (argc != 2) {
			usage(argv[0]);
			return 4;
		}

		return do_num_actuators();
	}

	if (!strcmp(argv[1], "d")) {
		if (argc != 2) {
			usage(argv[0]);
			return 4;
		}

		return do_debug_level();
	}

	usage(argv[0]);
	return 5;
}
