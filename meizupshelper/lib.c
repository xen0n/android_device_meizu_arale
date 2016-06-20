#define LOG_TAG "MeizuPSHelper"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <cutils/log.h>
#include <selinux/android.h>

#include "meizupshelper.h"

#define MEIZU_PS_CALIBRATION_TRIGGER_PATH "/sys/class/meizu/ps/ps_calibration"


static void fix_trigger_file_context(void)
{
	selinux_android_restorecon(MEIZU_PS_CALIBRATION_TRIGGER_PATH, 0);
}


int32_t meizu_psh_run_calibration(void)
{
	int fd;
	int ret;

	fix_trigger_file_context();
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
