/*
 * Copyright (C) 2015 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "MTKPower-xen0n"

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cutils/log.h>
#include <cutils/properties.h>

#include <hardware/power.h>
#include "power-feature.h"

#define GESTURE_CONTROL_PATH "/sys/devices/mx_tsp/gesture_hex"
#define DOUBLE_TAP_TO_WAKE_ENABLE_VALUE "00020001"
#define DOUBLE_TAP_TO_WAKE_DISABLE_VALUE "00010000"

#define DOUBLE_TAP_TO_WAKE_STORE_PROP "persist.cmhw.mz_taptowake"
#define DOUBLE_TAP_TO_WAKE_STORE_DEFAULT 0


static int _taptowake_write_control_file(int enabled)
{
    int fd;
    int ret;
    char *buf;

    fd = open(GESTURE_CONTROL_PATH, O_WRONLY);
    if (fd < 0) {
        int errsv = errno;
        ALOGE("%s: open control file failed: %s", __func__, strerror(errsv));
        return -1;
    }

    /* let's be lazy and hard-code the length as it won't ever change */
    buf = enabled ? DOUBLE_TAP_TO_WAKE_ENABLE_VALUE : DOUBLE_TAP_TO_WAKE_DISABLE_VALUE;
    ret = write(fd, buf, 8);
    if (ret < 0) {
        int errsv = errno;
        ALOGE("%s: write control file failed: %s", __func__, strerror(errsv));
        goto bail;
    }

    ret = 0;

bail:
    close(fd);
    return ret;
}


static void _taptowake_write_preference(int enabled)
{
    int ret;

    ret = property_set(DOUBLE_TAP_TO_WAKE_STORE_PROP, enabled ? "1" : "0");
    if (ret < 0) {
        int errsv = errno;
        ALOGE("%s: preference write failed (%d): %s", __func__, ret, strerror(errsv));
    }
}


static int _taptowake_read_preference(void)
{
    return property_get_int32(DOUBLE_TAP_TO_WAKE_STORE_PROP, DOUBLE_TAP_TO_WAKE_STORE_DEFAULT);
}


static void _taptowake_set_state(int enabled)
{
    int ret;

    ALOGD("%s: enabled=%d", __func__, enabled);
    ret = _taptowake_write_control_file(enabled);
    if (!ret) {
        _taptowake_write_preference(enabled);
    } else {
        ALOGE("%s: control file write failed (%d), skipping writing pref", __func__, ret);
    }
}


void set_device_specific_feature(struct power_module *module,
    feature_t feature, int state)
{
    switch (feature) {
        case POWER_FEATURE_DOUBLE_TAP_TO_WAKE:
            ALOGD("set_device_specific_feature(%p, POWER_FEATURE_DOUBLE_TAP_TO_WAKE, %d)", module, state);
            _taptowake_set_state(state);
            return;

        default:
            ALOGD("set_device_specific_feature(%p, %d, %d): stub!", module, feature, state);
    }
}


/* read the preference at bootup and set kernel parameter accordingly */
static void __attribute__((constructor)) _taptowake_onload(void)
{
    int state = _taptowake_read_preference();
    ALOGI("%s: persisted tap-to-wake status is %d", __func__, state);
    _taptowake_write_control_file(state);
}
