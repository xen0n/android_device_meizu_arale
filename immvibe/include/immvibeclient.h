#ifndef _IMMVIBECLIENT_H_
#define _IMMVIBECLIENT_H_

#include <stdint.h>
#include <sys/ioctl.h>


/* taken from open source kernel bits found on GitHub */
#define TSPDRV_MODULE_NAME                  "tspdrv"
#define TSPDRV                              "/dev/" TSPDRV_MODULE_NAME
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


/* library interface */
struct immvibe_handle;

struct immvibe_handle *immvibe_open(int *out_status);
void immvibe_close(struct immvibe_handle *fd);

int immvibe_play(struct immvibe_handle *fd, uint8_t force);
int immvibe_stop_kernel_timer(struct immvibe_handle *fd);
int immvibe_get_num_actuators(struct immvibe_handle *fd);
int immvibe_get_debug_level(struct immvibe_handle *fd);
int immvibe_set_amp_enabled(struct immvibe_handle *fd, int enable);

/* high level interface */
uint8_t immvibe_api_get_force_userspace(void);
int immvibe_api_set_force_userspace(uint8_t force);

#endif
