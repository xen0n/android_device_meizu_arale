#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include "immvibeclient.h"


/* opaque handle to immvibe driver */
struct immvibe_handle {
	int fd;
};


struct immvibe_handle *immvibe_open(int *out_status)
{
	int fd;
	struct immvibe_handle *result;
	long magic = TSPDRV_MAGIC_NUMBER;
	int ret = 0;

	fd = open(TSPDRV, O_WRONLY);
	if (fd < 0) {
		// perror("open_driver: open");
		if (out_status) {
			*out_status = fd;
		}
		return NULL;
	}

	ret = ioctl(fd, TSPDRV_SET_MAGIC_NUMBER, &magic);
	if (ret) {
		// perror("open_driver: ioctl");
		close(fd);
		if (out_status) {
			*out_status = ret;
		}
		return NULL;
	}

	result = (struct immvibe_handle *)malloc(sizeof(struct immvibe_handle));
	if (!result) {
		close(fd);
		if (out_status) {
			*out_status = -ENOMEM;
		}
		return NULL;
	}

	result->fd = fd;

	if (out_status) {
		*out_status = 0;
	}
	return result;
}


void immvibe_close(struct immvibe_handle *fd)
{
	if (fd) {
		close(fd->fd);
		free(fd);
	}
}


int immvibe_play(struct immvibe_handle *fd, uint8_t force)
{
	int ret = 0;

	unsigned char buf[4];
	buf[0] = buf[1] = buf[2] = 0;
	buf[3] = (unsigned char) force;

	if (!write(fd->fd, &buf, sizeof(buf))) {
		// fprintf(stderr, "do_play: write failed\n");

		ret = -EAGAIN;
		goto bail;
	}

bail:
	return ret;
}


int immvibe_stop_kernel_timer(struct immvibe_handle *fd)
{
	return ioctl(fd->fd, TSPDRV_STOP_KERNEL_TIMER, NULL);
}


int immvibe_get_num_actuators(struct immvibe_handle *fd)
{
	return ioctl(fd->fd, TSPDRV_GET_NUM_ACTUATORS, NULL);
}


int immvibe_get_debug_level(struct immvibe_handle *fd)
{
	return ioctl(fd->fd, TSPDRV_GET_DBG_LEVEL, NULL);
}


int immvibe_set_amp_enabled(struct immvibe_handle *fd, int enable)
{
	return ioctl(fd->fd, enable ? TSPDRV_ENABLE_AMP : TSPDRV_DISABLE_AMP, NULL);
}
