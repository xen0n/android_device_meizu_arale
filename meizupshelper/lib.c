#define LOG_TAG "MeizuPSHelper"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <cutils/log.h>

#include "meizupshelper.h"

#define MEIZU_PS_CALIBRATION_TRIGGER_PATH "/sys/class/meizu/ps/ps_calibration"
#define MEIZU_PS_ENABLER_PATH "/sys/class/meizu/ps/ps_enable"


int32_t meizu_psh_run_calibration(void)
{
	int fd;
	int ret;

	fd = open(MEIZU_PS_CALIBRATION_TRIGGER_PATH, O_WRONLY);
	if (fd < 0) {
		ALOGE("%s: failed to open trigger: %s", __func__, strerror(errno));
		return 1;
	}

	ret = write(fd, "1", 1);
	if (ret < 0) {
		ALOGE("%s: write failed: %s", __func__, strerror(errno));
		ret = 1;
	} else {
		ret = 0;
	}

	ALOGI("%s: done", __func__);
	close(fd);
	return ret;
}


void meizu_psh_trigger_reset(int32_t wait_msecs)
{
	int fd;
	int ret;

	fd = open(MEIZU_PS_ENABLER_PATH, O_WRONLY);
	if (fd < 0) {
		ALOGE("%s: failed to open enabler: %s", __func__, strerror(errno));
		return;
	}

	write(fd, "1", 1);
	usleep(wait_msecs * 1000);
	write(fd, "0", 1);
	close(fd);
}
