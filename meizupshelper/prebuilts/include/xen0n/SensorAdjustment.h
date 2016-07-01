/**
 * Interface of libSensorAdjustment.so, reverse-engineered by @xen0n.
 */

#ifndef _XEN0N_LIBSENSORADJUSTMENT_H_
#define _XEN0N_LIBSENSORADJUSTMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

int calibrate_proximity_sensor(void);
int read_proximity_sensor_calibration_value(void);

// value should be < 1000 or fail
int write_proximity_sensor_calibration_value(int value);
int write_proximity_sensor_calib_value(int value);

/* TODO: other functions */

#ifdef __cplusplus
}
#endif

#endif  /* #ifndef _XEN0N_LIBSENSORADJUSTMENT_H_ */
