#define LOG_TAG "MeizuPSHelper"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <cutils/log.h>

#include "meizupshelper.h"

#define MEIZU_PS_CALIBRATION_TRIGGER_PATH "/sys/class/meizu/ps/ps_calibration"


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
