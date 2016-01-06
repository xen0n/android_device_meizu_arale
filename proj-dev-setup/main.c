#define LOG_TAG "proj_dev_setup"

#include <fcntl.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <cutils/properties.h>
#include <cutils/log.h>
#include <private/android_filesystem_config.h>


static void s_mknod(const char *pathname, mode_t type, unsigned int maj, unsigned int min)
{
	mknod(pathname, type, makedev(maj, min));
}


static void s_mkdirp(const char *pathname)
{
	char buf[PATH_MAX];
	char currpath[PATH_MAX], *pathpiece;
	struct stat st;
	int ret;

	// buffer for strtok
	// hopefully the strings are not too long
	strncpy(buf, pathname, PATH_MAX);

	// reset path
	strcpy(currpath, "");
	// create the pieces of the path along the way
	pathpiece = strtok(buf, "/");
	if(buf[0] == '/') {
		// prepend / if needed
		strcat(currpath, "/");
	}
	while(pathpiece != NULL) {
		if(strlen(currpath) + strlen(pathpiece) + 2/*NUL and slash*/ > PATH_MAX) {
			ALOGE("Invalid path specified: too long");
			return;  // return 1;
		}
		strcat(currpath, pathpiece);
		strcat(currpath, "/");
		if(stat(currpath, &st) != 0) {
			ret = mkdir(currpath, 0777);
			if(ret < 0) {
				ALOGE("mkdir failed for %s, %s\n", currpath, strerror(errno));
				return;  // return ret;
			}
		}
		pathpiece = strtok(NULL, "/");
	}
}


#if 0
static void s_echo(const char *pathname, const void *buf, size_t len)
{
	int fd;
	int remaining;
	int ret;

	fd = open(pathname, O_WRONLY);
	if (fd < 0) {
		ALOGE("echo: failed to open file %s: %s", pathname, strerror(errno));
		return;
	}

	remaining = len;
	while (remaining) {
		ret = write(fd, buf, remaining);
		if (ret < 0) {
			ALOGE("echo: write failed: %s", strerror(errno));
			goto bail;
		}
		remaining -= ret;
	}

bail:
	close(fd);
}
#endif


int main(int argc __attribute__((unused)), char **argv __attribute__((unused)))
{
	ALOGI("starting");

	// !/system/bin/sh

	// on init
	//    mkdir /mnt/media_rw/usbotg 0700 media_rw media_rw
	//    mkdir /storage/usbotg 0700 root root

	// on post-fs-data

	// Change permissions for accdet calibration
	chown("/sys/class/meizu/accdet_calibration/cmd", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/class/meizu/accdet_calibration/data", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/class/meizu/accdet_calibration/state", AID_SYSTEM, AID_SYSTEM);

	//
	// Connectivity related device nodes & configuration (begin)
	//

	// /dev/ttyMT2 for Connectivity BT/FM/GPS usage
	chmod("/dev/ttyMT2", 0660);
	chown("/dev/ttyMT2", AID_SYSTEM, AID_SYSTEM);


	// STP, WMT, GPS, FM and BT Driver
	//   insmod /system/lib/modules/mtk_hif_sdio.ko
	//   insmod /system/lib/modules/mtk_stp_wmt.ko
	//   insmod /system/lib/modules/mtk_stp_uart.ko
	//   insmod /system/lib/modules/mtk_stp_gps.ko
	//   insmod /system/lib/modules/mtk_stp_bt.ko
	//   insmod /system/lib/modules/mtk_fm_drv.ko
	//   insmod /system/lib/modules/mtk_wmt_wifi.ko

	// SMB
	chown("/proc/smb/ScreenComm", AID_SYSTEM, AID_SYSTEM);
	chmod("/proc/smb/ScreenComm", 0660);

	// Create char device file for WMT, GPS, BT, FM, WIFI
	// xen0n: stp stands for setup... oh what English is this
	// don't know why they're commented out in Flyme, at least the bluetooth one
	// seems to be required in proper bluetooth initialization...
	s_mknod("/dev/stpwmt", S_IFCHR, 190, 0);
	s_mknod("/dev/stpgps", S_IFCHR, 191, 0);
	s_mknod("/dev/stpbt", S_IFCHR, 192, 0);

	chmod("/dev/stpwmt", 0660);
	chown("/dev/stpwmt", AID_SYSTEM, AID_SYSTEM);

	chmod("/dev/wmtdetect", 0660);
	chown("/dev/wmtdetect", AID_SYSTEM, AID_SYSTEM);

	s_mknod("/dev/wmtWifi", S_IFCHR, 153, 0);
	chmod("/dev/wmtWifi", 0660);
	chown("/dev/wmtWifi", AID_SYSTEM, AID_SYSTEM);


	// Camera
	chmod("/dev/GAF001AF", 0660);
	chown("/dev/GAF001AF", AID_SYSTEM, AID_CAMERA);

	chmod("/dev/DW9714AF", 0660);
	chown("/dev/DW9714AF", AID_SYSTEM, AID_CAMERA);

	chmod("/dev/AD5820AF", 0660);
	chown("/dev/AD5820AF", AID_SYSTEM, AID_CAMERA);

	chmod("/dev/BU64745GWZAF", 0660);
	chown("/dev/BU64745GWZAF", AID_SYSTEM, AID_CAMERA);

	chmod("/dev/LC898212AF", 0660);
	chown("/dev/LC898212AF", AID_SYSTEM, AID_CAMERA);

	chmod("/dev/LC898212XD", 0660);
	chown("/dev/LC898212XD", AID_SYSTEM, AID_CAMERA);

	// FlashLight
	chmod("/sys/class/flashlightdrv/kd_camera_flashlight/flash1", 0660);
	chown("/sys/class/flashlightdrv/kd_camera_flashlight/flash1", AID_MEDIA, AID_SYSTEM);
	chmod("/sys/class/flashlightdrv/kd_camera_flashlight/flash2", 0660);
	chown("/sys/class/flashlightdrv/kd_camera_flashlight/flash2", AID_MEDIA, AID_SYSTEM);

	// BT
	chmod("/dev/stpbt", 0660);
	chown("/dev/stpbt", AID_BLUETOOTH, AID_RADIO);

	// GPS
	chown("/dev/stpgps", AID_GPS, AID_GPS);
	chown("/sys/class/gpsdrv/gps/pwrctl", AID_GPS, AID_GPS);
	chown("/sys/class/gpsdrv/gps/suspend", AID_GPS, AID_GPS);
	chown("/sys/class/gpsdrv/gps/state", AID_GPS, AID_GPS);
	chown("/sys/class/gpsdrv/gps/pwrsave", AID_GPS, AID_GPS);
	chown("/sys/class/gpsdrv/gps/status", AID_GPS, AID_GPS);
	chmod("/dev/stpgps", 0660);

	// WiFi
	s_mkdirp("/data/misc/wifi");
	chmod("/data/misc/wifi", 0770);
	chown("/data/misc/wifi", AID_WIFI, AID_WIFI);
	s_mkdirp("/data/misc/wifi/sockets");
	chmod("/data/misc/wifi/sockets", 0770);
	chown("/data/misc/wifi/sockets", AID_WIFI, AID_WIFI);
	s_mkdirp("/data/misc/wpa_supplicant");
	chmod("/data/misc/wpa_supplicant", 0770);
	chown("/data/misc/wpa_supplicant", AID_WIFI, AID_WIFI);
	chown("/data/misc/wifi", AID_WIFI, AID_WIFI);

	// ANT
	chmod("/dev/stpant", 0660);
	chown("/dev/stpant", AID_SYSTEM, AID_SYSTEM);

	// Disable for one Single loader
	// Load WiFi Driver
	//    insmod /system/lib/modules/wlan_mt.ko

	// Char device for BT 3.0 HS
	s_mknod("/dev/ampc0", S_IFCHR, 151, 0);
	chown("/dev/ampc0", AID_BLUETOOTH, AID_BLUETOOTH);
	chmod("/dev/ampc0", 0660);


	// backlight
	chmod("/sys/class/meizu/lm3697_bled/mode", 0664);
	chown("/sys/class/meizu/lm3697_bled/mode", AID_SYSTEM, AID_SYSTEM);
	chmod("/sys/class/meizu/lm3697_bled/brightness", 0664);
	chown("/sys/class/meizu/lm3697_bled/brightness", AID_SYSTEM, AID_RADIO);

	// leds
	// moved to ueventd.mt6595.rc; COMPATIBILITY WITH FLYME BOOT.IMG IS LOST!
	// chown("/sys/class/leds/mx-led/brightness", AID_SYSTEM, AID_SYSTEM);
	// chown("/sys/class/leds/mx-led/blink", AID_SYSTEM, AID_SYSTEM);
	// chown("/sys/class/leds/mx-led/trigger", AID_SYSTEM, AID_SYSTEM);

	// Permissions for power saving mode
	chown("/sys/power/power_mode", AID_SYSTEM, AID_SYSTEM);

	// 
	// Connectivity related device nodes & configuration (end)
	// 
	// sensors
	chown("/dev/iio:device0", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/buffer", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_accel_accuracy_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_accel_cal_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_accel_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_anglvel_accuracy_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_compass_cal_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_compass_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_determine_engine_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_dmp_event_int_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_dmp_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_gyro_cal_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_gyro_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_low_power_gyro_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_magn_accuracy_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_reg_dump", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/dev", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/event_pedometer_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/event_smd_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_6quat_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_6quat_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_9quat_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_9quat_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_scale", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_x_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_x_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_y_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_y_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_z_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_accel_z_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_scale", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_x_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_x_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_y_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_y_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_z_calibbias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_anglvel_z_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_calib_anglvel_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_calib_anglvel_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_calib_magn_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_calib_magn_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_geomag_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_geomag_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_scale", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_x_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_y_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_magn_z_dmp_bias", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_p6quat_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_p6quat_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_p9quat_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_p9quat_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_step_detector_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/in_step_indicator_enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_accel_matrix", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_anglvel_matrix", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_firmware_loaded", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_gyro_sf", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_magn_matrix", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/info_secondary_name", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_accel_recalibration", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_batchmode_timeout", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_bin_accel_covariance", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_bin_cur_magn_covariance", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_bin_dmp_firmware", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_bin_magn_covariance", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_bin_soft_iron_matrix", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_flush_batch", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_gyro_recalibration", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_magn_recalibration", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_ref_mag_3d", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/misc_self_test", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/name", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/out_pedometer_counter", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/out_pedometer_steps", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/out_pedometer_time", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/out_temperature", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_accel_calibration_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_accel_calibration_threshold", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_pedometer_int_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_pedometer_int_thresh", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_pedometer_step_thresh", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_smd_delay_threshold", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_smd_delay_threshold2", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/params_smd_threshold", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/poll_pedometer", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/poll_smd", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/power", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/scan_elements", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/subsystem", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/trigger", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/uevent", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/buffer/enable", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/buffer/length", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/power/async", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_en", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_index", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_type", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/trigger/current_trigger", AID_SYSTEM, AID_SYSTEM);
	chown("/system/etc/firmware/inv_dmpfirmware.bin", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_cfg_write", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_clock_sel", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_cycle_mode_off", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_accel_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_gyro_rate", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_on", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_lp_en_off", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_mem_read", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_mem_write", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_reg_write", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/debug_reg_write_addr", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/sensor_pwr_state", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/bus/iio/devices/iio:device0/sensor_phone_calling", AID_SYSTEM, AID_SYSTEM);

	chmod("/dev/iio:device0", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/buffer", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/debug_accel_accuracy_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_accel_cal_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_accel_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_anglvel_accuracy_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_compass_cal_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_compass_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_determine_engine_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_dmp_event_int_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_dmp_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_gyro_cal_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_gyro_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_low_power_gyro_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_magn_accuracy_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_reg_dump", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/dev", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/event_pedometer_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/event_smd_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_6quat_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_6quat_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_9quat_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_9quat_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_scale", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_x_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_x_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_y_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_y_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_z_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_accel_z_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_scale", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_x_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_x_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_y_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_y_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_z_calibbias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_anglvel_z_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_calib_anglvel_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_calib_anglvel_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_calib_magn_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_calib_magn_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_geomag_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_geomag_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_scale", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_x_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_y_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_magn_z_dmp_bias", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_p6quat_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_p6quat_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_p9quat_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_p9quat_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_step_detector_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/in_step_indicator_enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_accel_matrix", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_anglvel_matrix", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_firmware_loaded", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_gyro_sf", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_magn_matrix", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/info_secondary_name", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_accel_recalibration", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_batchmode_timeout", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_bin_accel_covariance", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_bin_cur_magn_covariance", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_bin_dmp_firmware", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_bin_magn_covariance", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_bin_soft_iron_matrix", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_flush_batch", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_gyro_recalibration", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_magn_recalibration", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_ref_mag_3d", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/misc_self_test", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/name", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/out_pedometer_counter", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/out_pedometer_steps", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/out_pedometer_time", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/out_temperature", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_accel_calibration_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_accel_calibration_threshold", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_pedometer_int_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_pedometer_int_thresh", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_pedometer_step_thresh", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_smd_delay_threshold", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_smd_delay_threshold2", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/params_smd_threshold", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/poll_pedometer", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/poll_smd", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/power", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/scan_elements", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/subsystem", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/trigger", 0770);
	chmod("/sys/bus/iio/devices/iio:device0/uevent", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/buffer/enable", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/buffer/length", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/power/async", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_en", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_index", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_type", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/trigger/current_trigger", 0660);
	chmod("/system/etc/firmware/inv_dmpfirmware.bin", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_cfg_write", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_clock_sel", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_cycle_mode_off", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_accel_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_gyro_rate", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_data_collection_mode_on", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_lp_en_off", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_mem_read", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_mem_write", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_reg_write", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/debug_reg_write_addr", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/sensor_pwr_state", 0660);
	chmod("/sys/bus/iio/devices/iio:device0/sensor_phone_calling", 0664);

	chown("/sys/devices/platform/gpio-keys.0/key_hall_state", AID_SYSTEM, AID_SYSTEM);
	chmod("/sys/devices/platform/gpio-keys.0/key_hall_state", 0664);

	// for holster and gesture
	chown("/sys/devices/mx_tsp/hall_mode", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/devices/mx_tsp/gesture_control", AID_SYSTEM, AID_SYSTEM);
	chown("/sys/devices/mx_tsp/gesture_data", AID_SYSTEM, AID_SYSTEM);

	// gesture too, for easy manipulation from Java
	chown("/sys/devices/mx_tsp/gesture_hex", AID_SYSTEM, AID_SYSTEM);

	// /dev/ttyMT1 for ext_mdinit
	chmod("/dev/ttyMT1", 0660);
	chown("/dev/ttyMT1", AID_SYSTEM, AID_SYSTEM);

	// FLYME_EDIT:zhouyueguo@SHELL.PowerSavingMode.Feature Permissions add for power saving mode {@
	// Permissions for power saving mode
	chown("/sys/power/power_mode", AID_SYSTEM, AID_SYSTEM);
	chmod("/sys/power/power_mode", 0660);
	// @}

	// workaround: calibrate proximity sensor on boot
	// now unneeded as the function is now exposed as an init service
	// s_echo("/sys/class/meizu/ps/ps_calibration", "1", 1);

	// Encrypt phone function
	property_set("vold.post_fs_data_done", "1");

	ALOGI("exiting");

	return 0;
}
