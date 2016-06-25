/*
 * Copyright (c) 2012-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2014, The CyanogenMod Project
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 * *    * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#define LOG_NIDEBUG 0

#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdlib.h>

#define LOG_TAG "MTKPower-xen0n"
#include <cutils/log.h>
#include <hardware/hardware.h>
#include <hardware/power.h>
#include <pthread.h>

#include "metadata-defs.h"
#include "hint-data.h"
#include "power-common.h"
#include "power-feature.h"

static struct hw_module_methods_t power_module_methods = {
    .open = NULL,
};

static pthread_mutex_t hint_mutex = PTHREAD_MUTEX_INITIALIZER;

static void power_init(struct power_module *module)
{
    ALOGD("power_init(%p): stub!", module);
}

static void process_video_decode_hint(void *metadata)
{
    ALOGD("process_video_decode_hint(%p): stub!", metadata);
}

static void process_video_encode_hint(void *metadata)
{
    ALOGD("process_video_encode_hint(%p): stub!", metadata);
}

int __attribute__ ((weak)) power_hint_override(
        __attribute__((unused)) struct power_module *module,
        __attribute__((unused)) power_hint_t hint,
        __attribute__((unused)) void *data)
{
    return HINT_NONE;
}

extern void interaction(int duration, int num_args, int opt_list[]);

static void power_hint(struct power_module *module, power_hint_t hint,
        void *data)
{
    pthread_mutex_lock(&hint_mutex);

    /* Check if this hint has been overridden. */
    if (power_hint_override(module, hint, data) == HINT_HANDLED) {
        /* The power_hint has been handled. We can skip the rest. */
        goto out;
    }

    switch(hint) {
        case POWER_HINT_VSYNC:
        case POWER_HINT_INTERACTION:
        case POWER_HINT_CPU_BOOST:
        case POWER_HINT_LAUNCH_BOOST:
        case POWER_HINT_AUDIO:
        case POWER_HINT_SET_PROFILE:
        case POWER_HINT_LOW_POWER:
            ALOGD("power_hint(%p, %d, %p): stub!", module, hint, data);
        break;
        case POWER_HINT_VIDEO_ENCODE:
            process_video_encode_hint(data);
        break;
        case POWER_HINT_VIDEO_DECODE:
            process_video_decode_hint(data);
        break;
        default:
        break;
    }

out:
    pthread_mutex_unlock(&hint_mutex);
}

int __attribute__ ((weak)) set_interactive_override(
        __attribute__((unused)) struct power_module *module,
        __attribute__((unused)) int on)
{
    return HINT_NONE;
}

int __attribute__ ((weak)) get_number_of_profiles()
{
    return 0;
}

#ifdef SET_INTERACTIVE_EXT
extern void cm_power_set_interactive_ext(int on);
#endif

void set_interactive(struct power_module *module, int on)
{
    char governor[80];
    char tmp_str[NODE_MAX];
    struct video_encode_metadata_t video_encode_metadata;
    int rc = 0;

    pthread_mutex_lock(&hint_mutex);

#ifdef SET_INTERACTIVE_EXT
    cm_power_set_interactive_ext(on);
#endif

    if (set_interactive_override(module, on) == HINT_HANDLED) {
        goto out;
    }

    /* ALOGI("Got set_interactive hint"); */
    ALOGD("set_interactive(%p, %d): stub!", module, on);

out:
    pthread_mutex_unlock(&hint_mutex);
}

void set_feature(struct power_module *module, feature_t feature, int state)
{
#ifdef TAP_TO_WAKE_NODE
    char tmp_str[NODE_MAX];
    if (feature == POWER_FEATURE_DOUBLE_TAP_TO_WAKE) {
        snprintf(tmp_str, NODE_MAX, "%d", state);
        sysfs_write(TAP_TO_WAKE_NODE, tmp_str);
        return;
    }
#endif
    set_device_specific_feature(module, feature, state);
}

int get_feature(struct power_module *module __unused, feature_t feature)
{
    if (feature == POWER_FEATURE_SUPPORTED_PROFILES) {
        return get_number_of_profiles();
    }
    return -1;
}

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_3,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "MTK Power HAL by xen0n",
        .author = "xen0n",
        .methods = &power_module_methods,
    },

    .init = power_init,
    .powerHint = power_hint,
    .setInteractive = set_interactive,
    .setFeature = set_feature,
    .getFeature = get_feature
};
