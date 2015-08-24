#!/system/bin/sh

# mknod is not present in stock Android, let's use busybox
PATH=/system/xbin:/system/bin

#on init
#    mkdir /mnt/media_rw/usbotg 0700 media_rw media_rw
#    mkdir /storage/usbotg 0700 root root

#on post-fs-data

# Change permissions for accdet calibration
chown system:system /sys/class/meizu/accdet_calibration/cmd
chown system:system /sys/class/meizu/accdet_calibration/data
chown system:system /sys/class/meizu/accdet_calibration/state

#
# Connectivity related device nodes & configuration (begin)
#

#/dev/ttyMT2 for Connectivity BT/FM/GPS usage
chmod 0660 /dev/ttyMT2
chown system:system /dev/ttyMT2


# STP, WMT, GPS, FM and BT Driver
#   insmod /system/lib/modules/mtk_hif_sdio.ko
#   insmod /system/lib/modules/mtk_stp_wmt.ko
#   insmod /system/lib/modules/mtk_stp_uart.ko
#   insmod /system/lib/modules/mtk_stp_gps.ko
#   insmod /system/lib/modules/mtk_stp_bt.ko
#   insmod /system/lib/modules/mtk_fm_drv.ko
#   insmod /system/lib/modules/mtk_wmt_wifi.ko

#SMB
chown system:system /proc/smb/ScreenComm
chmod 0660 /proc/smb/ScreenComm

# Create char device file for WMT, GPS, BT, FM, WIFI
#    mknod /dev/stpwmt c 190 0;
#    mknod /dev/stpgps c 191 0;
#    mknod /dev/stpbt  c 192 0;

#    chmod 0660 /dev/stpwmt
#    chown system:system /dev/stpwmt

#    chmod 0660 /dev/wmtdetect
#    chown system:system /dev/wmtdetect

mknod /dev/wmtWifi c 153 0
chmod 0660 /dev/wmtWifi
chown system:system /dev/wmtWifi


#Camera
chmod 0660 /dev/GAF001AF
chown system:camera /dev/GAF001AF

chmod 0660 /dev/DW9714AF
chown system:camera /dev/DW9714AF

chmod 0660 /dev/AD5820AF
chown system:camera /dev/AD5820AF

chmod 0660 /dev/BU64745GWZAF
chown system:camera /dev/BU64745GWZAF

chmod 0660 /dev/LC898212AF
chown system:camera /dev/LC898212AF

chmod 0660 /dev/LC898212XD
chown system:camera /dev/LC898212XD

#FlashLight
chmod 0660 /sys/class/flashlightdrv/kd_camera_flashlight/flash1
chown media:system /sys/class/flashlightdrv/kd_camera_flashlight/flash1
chmod 0660 /sys/class/flashlightdrv/kd_camera_flashlight/flash2
chown media:system /sys/class/flashlightdrv/kd_camera_flashlight/flash2
# BT
#    chmod 0660 /dev/stpbt
#    chown bluetooth:radio /dev/stpbt

# GPS
#    chown gps:gps /dev/stpgps
chown gps:gps /sys/class/gpsdrv/gps/pwrctl
chown gps:gps /sys/class/gpsdrv/gps/suspend
chown gps:gps /sys/class/gpsdrv/gps/state
chown gps:gps /sys/class/gpsdrv/gps/pwrsave
chown gps:gps /sys/class/gpsdrv/gps/status
#    chmod 0660 /dev/stpgps

# WiFi
mkdir /data/misc/wifi 0770 wifi wifi
mkdir /data/misc/wifi/sockets 0770 wifi wifi
mkdir /data/misc/wpa_supplicant 0770 wifi wifi
chown wifi:wifi /data/misc/wifi

# ANT
chmod 0660 /dev/stpant
chown ant:radio /dev/stpant

#Disable for one Single loader
# Load WiFi Driver
#   insmod /system/lib/modules/wlan_mt.ko

# Char device for BT 3.0 HS
mknod /dev/ampc0 c 151 0
chown bluetooth:bluetooth /dev/ampc0
chmod 0660 /dev/ampc0


#backlight
chmod 0664 /sys/class/meizu/lm3697_bled/mode
chown system:system /sys/class/meizu/lm3697_bled/mode
chmod 0664 /sys/class/meizu/lm3697_bled/brightness
chown system:radio  /sys/class/meizu/lm3697_bled/brightness

#leds
chown system:system /sys/class/leds/mx-led/brightness
chown system:system /sys/class/leds/mx-led/blink
#TP
chown root:system /sys/devices/mx_tsp/gesture_control

# Permissions for power saving mode
chown system:system /sys/power/power_mode

#
# Connectivity related device nodes & configuration (end)
#
# sensors
chown system:system /dev/iio:device0
chown system:system /sys/bus/iio/devices/iio:device0/buffer
chown system:system /sys/bus/iio/devices/iio:device0/debug_accel_accuracy_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_accel_cal_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_accel_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_anglvel_accuracy_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_compass_cal_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_compass_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_determine_engine_on
chown system:system /sys/bus/iio/devices/iio:device0/debug_dmp_event_int_on
chown system:system /sys/bus/iio/devices/iio:device0/debug_dmp_on
chown system:system /sys/bus/iio/devices/iio:device0/debug_gyro_cal_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_gyro_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_low_power_gyro_on
chown system:system /sys/bus/iio/devices/iio:device0/debug_magn_accuracy_enable
chown system:system /sys/bus/iio/devices/iio:device0/debug_reg_dump
chown system:system /sys/bus/iio/devices/iio:device0/dev
chown system:system /sys/bus/iio/devices/iio:device0/event_pedometer_enable
chown system:system /sys/bus/iio/devices/iio:device0/event_smd_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_6quat_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_6quat_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_9quat_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_9quat_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_scale
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_x_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_x_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_y_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_y_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_z_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_accel_z_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_scale
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_x_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_x_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_y_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_y_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_z_calibbias
chown system:system /sys/bus/iio/devices/iio:device0/in_anglvel_z_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_calib_anglvel_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_calib_anglvel_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_calib_magn_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_calib_magn_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_geomag_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_geomag_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_scale
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_x_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_y_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_magn_z_dmp_bias
chown system:system /sys/bus/iio/devices/iio:device0/in_p6quat_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_p6quat_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_p9quat_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_p9quat_rate
chown system:system /sys/bus/iio/devices/iio:device0/in_step_detector_enable
chown system:system /sys/bus/iio/devices/iio:device0/in_step_indicator_enable
chown system:system /sys/bus/iio/devices/iio:device0/info_accel_matrix
chown system:system /sys/bus/iio/devices/iio:device0/info_anglvel_matrix
chown system:system /sys/bus/iio/devices/iio:device0/info_firmware_loaded
chown system:system /sys/bus/iio/devices/iio:device0/info_gyro_sf
chown system:system /sys/bus/iio/devices/iio:device0/info_magn_matrix
chown system:system /sys/bus/iio/devices/iio:device0/info_secondary_name
chown system:system /sys/bus/iio/devices/iio:device0/misc_accel_recalibration
chown system:system /sys/bus/iio/devices/iio:device0/misc_batchmode_timeout
chown system:system /sys/bus/iio/devices/iio:device0/misc_bin_accel_covariance
chown system:system /sys/bus/iio/devices/iio:device0/misc_bin_cur_magn_covariance
chown system:system /sys/bus/iio/devices/iio:device0/misc_bin_dmp_firmware
chown system:system /sys/bus/iio/devices/iio:device0/misc_bin_magn_covariance
chown system:system /sys/bus/iio/devices/iio:device0/misc_bin_soft_iron_matrix
chown system:system /sys/bus/iio/devices/iio:device0/misc_flush_batch
chown system:system /sys/bus/iio/devices/iio:device0/misc_gyro_recalibration
chown system:system /sys/bus/iio/devices/iio:device0/misc_magn_recalibration
chown system:system /sys/bus/iio/devices/iio:device0/misc_ref_mag_3d
chown system:system /sys/bus/iio/devices/iio:device0/misc_self_test
chown system:system /sys/bus/iio/devices/iio:device0/name
chown system:system /sys/bus/iio/devices/iio:device0/out_pedometer_counter
chown system:system /sys/bus/iio/devices/iio:device0/out_pedometer_steps
chown system:system /sys/bus/iio/devices/iio:device0/out_pedometer_time
chown system:system /sys/bus/iio/devices/iio:device0/out_temperature
chown system:system /sys/bus/iio/devices/iio:device0/params_accel_calibration_rate
chown system:system /sys/bus/iio/devices/iio:device0/params_accel_calibration_threshold
chown system:system /sys/bus/iio/devices/iio:device0/params_pedometer_int_on
chown system:system /sys/bus/iio/devices/iio:device0/params_pedometer_int_thresh
chown system:system /sys/bus/iio/devices/iio:device0/params_pedometer_step_thresh
chown system:system /sys/bus/iio/devices/iio:device0/params_smd_delay_threshold
chown system:system /sys/bus/iio/devices/iio:device0/params_smd_delay_threshold2
chown system:system /sys/bus/iio/devices/iio:device0/params_smd_threshold
chown system:system /sys/bus/iio/devices/iio:device0/poll_pedometer
chown system:system /sys/bus/iio/devices/iio:device0/poll_smd
chown system:system /sys/bus/iio/devices/iio:device0/power
chown system:system /sys/bus/iio/devices/iio:device0/scan_elements
chown system:system /sys/bus/iio/devices/iio:device0/subsystem
chown system:system /sys/bus/iio/devices/iio:device0/trigger
chown system:system /sys/bus/iio/devices/iio:device0/uevent
chown system:system /sys/bus/iio/devices/iio:device0/buffer/enable
chown system:system /sys/bus/iio/devices/iio:device0/buffer/length
chown system:system /sys/bus/iio/devices/iio:device0/power/async
chown system:system /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_en
chown system:system /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_index
chown system:system /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_type
chown system:system /sys/bus/iio/devices/iio:device0/trigger/current_trigger
chown system:system /system/etc/firmware/inv_dmpfirmware.bin
chown system:system /sys/bus/iio/devices/iio:device0/debug_cfg_write
chown system:system /sys/bus/iio/devices/iio:device0/debug_clock_sel
chown system:system /sys/bus/iio/devices/iio:device0/debug_cycle_mode_off
chown system:system /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_accel_rate
chown system:system /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_gyro_rate
chown system:system /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_on
chown system:system /sys/bus/iio/devices/iio:device0/debug_lp_en_off
chown system:system /sys/bus/iio/devices/iio:device0/debug_mem_read
chown system:system /sys/bus/iio/devices/iio:device0/debug_mem_write
chown system:system /sys/bus/iio/devices/iio:device0/debug_reg_write
chown system:system /sys/bus/iio/devices/iio:device0/debug_reg_write_addr
chown system:system /sys/bus/iio/devices/iio:device0/sensor_pwr_state
chown system:system /sys/bus/iio/devices/iio:device0/sensor_phone_calling

chmod 0770 /dev/iio:device0
chmod 0770 /sys/bus/iio/devices/iio:device0/buffer
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_accel_accuracy_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_accel_cal_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_accel_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_anglvel_accuracy_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_compass_cal_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_compass_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_determine_engine_on
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_dmp_event_int_on
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_dmp_on
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_gyro_cal_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_gyro_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_low_power_gyro_on
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_magn_accuracy_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_reg_dump
chmod 0660 /sys/bus/iio/devices/iio:device0/dev
chmod 0660 /sys/bus/iio/devices/iio:device0/event_pedometer_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/event_smd_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_6quat_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_6quat_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_9quat_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_9quat_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_scale
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_x_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_x_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_y_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_y_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_z_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_accel_z_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_scale
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_x_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_x_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_y_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_y_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_z_calibbias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_anglvel_z_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_calib_anglvel_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_calib_anglvel_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_calib_magn_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_calib_magn_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_geomag_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_geomag_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_scale
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_x_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_y_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_magn_z_dmp_bias
chmod 0660 /sys/bus/iio/devices/iio:device0/in_p6quat_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_p6quat_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_p9quat_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_p9quat_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/in_step_detector_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/in_step_indicator_enable
chmod 0660 /sys/bus/iio/devices/iio:device0/info_accel_matrix
chmod 0660 /sys/bus/iio/devices/iio:device0/info_anglvel_matrix
chmod 0660 /sys/bus/iio/devices/iio:device0/info_firmware_loaded
chmod 0660 /sys/bus/iio/devices/iio:device0/info_gyro_sf
chmod 0660 /sys/bus/iio/devices/iio:device0/info_magn_matrix
chmod 0660 /sys/bus/iio/devices/iio:device0/info_secondary_name
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_accel_recalibration
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_batchmode_timeout
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_bin_accel_covariance
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_bin_cur_magn_covariance
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_bin_dmp_firmware
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_bin_magn_covariance
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_bin_soft_iron_matrix
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_flush_batch
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_gyro_recalibration
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_magn_recalibration
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_ref_mag_3d
chmod 0660 /sys/bus/iio/devices/iio:device0/misc_self_test
chmod 0660 /sys/bus/iio/devices/iio:device0/name
chmod 0660 /sys/bus/iio/devices/iio:device0/out_pedometer_counter
chmod 0660 /sys/bus/iio/devices/iio:device0/out_pedometer_steps
chmod 0660 /sys/bus/iio/devices/iio:device0/out_pedometer_time
chmod 0660 /sys/bus/iio/devices/iio:device0/out_temperature
chmod 0660 /sys/bus/iio/devices/iio:device0/params_accel_calibration_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/params_accel_calibration_threshold
chmod 0660 /sys/bus/iio/devices/iio:device0/params_pedometer_int_on
chmod 0660 /sys/bus/iio/devices/iio:device0/params_pedometer_int_thresh
chmod 0660 /sys/bus/iio/devices/iio:device0/params_pedometer_step_thresh
chmod 0660 /sys/bus/iio/devices/iio:device0/params_smd_delay_threshold
chmod 0660 /sys/bus/iio/devices/iio:device0/params_smd_delay_threshold2
chmod 0660 /sys/bus/iio/devices/iio:device0/params_smd_threshold
chmod 0660 /sys/bus/iio/devices/iio:device0/poll_pedometer
chmod 0660 /sys/bus/iio/devices/iio:device0/poll_smd
chmod 0770 /sys/bus/iio/devices/iio:device0/power
chmod 0770 /sys/bus/iio/devices/iio:device0/scan_elements
chmod 0770 /sys/bus/iio/devices/iio:device0/subsystem
chmod 0770 /sys/bus/iio/devices/iio:device0/trigger
chmod 0660 /sys/bus/iio/devices/iio:device0/uevent
chmod 0660 /sys/bus/iio/devices/iio:device0/buffer/enable
chmod 0660 /sys/bus/iio/devices/iio:device0/buffer/length
chmod 0660 /sys/bus/iio/devices/iio:device0/power/async
chmod 0660 /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_en
chmod 0660 /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_index
chmod 0660 /sys/bus/iio/devices/iio:device0/scan_elements/in_timestamp_type
chmod 0660 /sys/bus/iio/devices/iio:device0/trigger/current_trigger
chmod 0660 /system/etc/firmware/inv_dmpfirmware.bin
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_cfg_write
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_clock_sel
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_cycle_mode_off
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_accel_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_gyro_rate
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_data_collection_mode_on
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_lp_en_off
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_mem_read
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_mem_write
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_reg_write
chmod 0660 /sys/bus/iio/devices/iio:device0/debug_reg_write_addr
chmod 0660 /sys/bus/iio/devices/iio:device0/sensor_pwr_state  
chmod 0664 /sys/bus/iio/devices/iio:device0/sensor_phone_calling

chown system:system /sys/devices/platform/gpio-keys.0/key_hall_state
chmod 0664 /sys/devices/platform/gpio-keys.0/key_hall_state

#for holster and gesture
chown system:system /sys/devices/mx_tsp/hall_mode
chown system:system /sys/devices/mx_tsp/gesture_control
chown system:system /sys/devices/mx_tsp/gesture_data

#/dev/ttyMT1 for ext_mdinit
chmod 0660 /dev/ttyMT1
chown system:system /dev/ttyMT1

#FLYME_EDIT:zhouyueguo@SHELL.PowerSavingMode.Feature Permissions add for power saving mode {@
#Permissions for power saving mode
chown system:system /sys/power/power_mode
chmod 0660 /sys/power/power_mode
#@}

# Encrypt phone function
setprop vold.post_fs_data_done 1
