#define LOG_TAG "MeizuPSHelper"

#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <cutils/log.h>

#include <xen0n/SensorAdjustment.h>

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


int32_t meizu_psh_run_calibration_stock(void)
{
	return calibrate_proximity_sensor();
}


int32_t meizu_psh_read_calibration_value_stock(void)
{
	return read_proximity_sensor_calibration_value();
}


int32_t meizu_psh_write_calibration_value_stock(int32_t value)
{
	return write_proximity_sensor_calibration_value(value);
}
