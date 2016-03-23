/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
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

#define LOG_TAG "test-surface"

#include <cutils/memory.h>

#include <utils/Log.h>

#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>

#include <ui/GraphicBuffer.h>
#include <gui/Surface.h>
#include <gui/ISurfaceComposer.h>
#include <gui/SurfaceComposerClient.h>
#include <cutils/properties.h>

#include <ui/DisplayInfo.h>
#include <ui/gralloc_extra.h>

#include "GuiExtClientProducer.h"
#include "GuiExtClientConsumer.h"

#include "graphics_mtk_defs.h"

using namespace android;

struct FRAME {
    char name[128];     // file name
    uint32_t w;         // width
    uint32_t s;         // stride
    uint32_t h;         // height
    uint32_t fmt;       // format
    uint32_t api;       // api connection type
    uint32_t usageEx;
    uint32_t id;
};

FRAME test_frames[] = {
    {"/data/ibmbw_720x480_mtk.yuv",     720,  720,  480, HAL_PIXEL_FORMAT_NV12_BLK,   NATIVE_WINDOW_API_MEDIA,   GRALLOC_EXTRA_BIT_TYPE_VIDEO, 0}
};

const static int TEST_FRAMES = sizeof(test_frames) / sizeof(struct FRAME);

GuiExtClientProducer &producer(GuiExtClientProducer::getInstance());

//
// use FRAME data to dispay with an ANativeWindow
// as we postBuffer before
//
status_t showTestFrame(ANativeWindow *w, const FRAME &f) {
    char value[PROPERTY_VALUE_MAX];

    // set api connection type as register
    native_window_api_connect(w, 0);

    // set buffer size
    native_window_set_buffers_dimensions(w, f.s, f.h);

    // set format
    native_window_set_buffers_format(w, f.fmt);

    // set usage software write-able and hardware texture bind-able
    native_window_set_usage(w, GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE);

    // set scaling to match window display size
    native_window_set_scaling_mode(w, NATIVE_WINDOW_SCALING_MODE_SCALE_TO_WINDOW);

    // set buffer rotation
    property_get("debug.sftest.orientation", value, "0");
    int orientation = atoi(value);
    switch (orientation)
    {
        case 1:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90);
            ALOGD("rot 90");
            break;

        case 2:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_180);
            ALOGD("rot 180");
            break;

        case 3:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_270);
            ALOGD("rot 270");
            break;

        case 4:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_FLIP_H);
            ALOGD("flip H");
            break;

        case 5:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_FLIP_V);
            ALOGD("flip V");
            break;

        case 6:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_H);
            ALOGD("rot 90 + flip H");
            break;

        case 7:
            native_window_set_buffers_transform(w, HAL_TRANSFORM_ROT_90 | HAL_TRANSFORM_FLIP_V);
            ALOGD("rot 90 + flip V");
            break;

        default:
            ALOGD("rot 0 and no flip");
    }

    // set buffer count
    native_window_set_buffer_count(w, 10);

    ANativeWindowBuffer *buf;
    sp<GraphicBuffer>   gb;
    void                *ptr;
    const Rect          rect(f.w, f.h);

    int err;
    int fenceFd = -1;
    err = w->dequeueBuffer(w, &buf, &fenceFd);                     // dequeue to get buffer handle
    sp<Fence> fence1(new Fence(fenceFd));
    fence1->wait(Fence::TIMEOUT_NEVER);
    if(err) {
        ALOGE("%s", strerror(-err));
    }

    gb = new GraphicBuffer(buf, false);

    // set extra buff value
    {
        gralloc_extra_ion_sf_info_t sf_info;

        gralloc_extra_query(gb->handle, GRALLOC_EXTRA_GET_IOCTL_ION_SF_INFO, &sf_info);

        sf_info.pool_id = f.id;

        gralloc_extra_sf_set_status(&sf_info,
            GRALLOC_EXTRA_MASK_TYPE | GRALLOC_EXTRA_MASK_DIRTY,
            f.usageEx | GRALLOC_EXTRA_BIT_DIRTY);

        gralloc_extra_perform(gb->handle, GRALLOC_EXTRA_SET_IOCTL_ION_SF_INFO, &sf_info);
    }

    gb->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_HW_TEXTURE, rect, &ptr);
    {
        FILE *file = fopen(f.name, "rb");                // read file into buffer

        if (file != NULL)
        {
            fread(ptr, f.s * f.h * 3 / 2, 1, file);      // bpp is 1.5
            fclose(file);
        }
        else
        {
            ALOGE("open file %s failed", f.name);
        }
    }
    gb->unlock();

    err = w->queueBuffer(w, buf, -1);                                    // queue to display
    sp<Fence> fence2(new Fence(fenceFd));
    fence2->wait(Fence::TIMEOUT_NEVER);
    if(err) {
        ALOGE("%s", strerror(-err));
    }

    property_get("debug.sftest.sleep", value, "1000");
    int delay = atoi(value);

    usleep(delay * 1000);

    // disconnect as unregister
    native_window_api_disconnect(w, 0);

    return NO_ERROR;
}


status_t main(int /*argc*/, char** /*argv*/) {

    // set up the thread-pool
    sp<ProcessState> proc(ProcessState::self());
    ProcessState::self()->startThreadPool();

    // create a client to surfaceflinger
    sp<SurfaceComposerClient> client = new SurfaceComposerClient();
    DisplayInfo dinfo;
    sp<IBinder> display = SurfaceComposerClient::getBuiltInDisplay(
            ISurfaceComposer::eDisplayIdMain);
    SurfaceComposerClient::getDisplayInfo(display, &dinfo);
    uint32_t dispw = dinfo.w;
    uint32_t disph = dinfo.h;

    ALOGD("display (w,h):(%d,%d)", dispw, disph);

    sp<SurfaceControl> surfaceControl = client->createSurface(
        String8("test-surface"), dispw - 100, disph - 100, PIXEL_FORMAT_RGB_565);

    sp<Surface> surface = surfaceControl->getSurface();
    ANativeWindow* window = surface.get();

    SurfaceComposerClient::openGlobalTransaction();
    surfaceControl->setPosition(50, 50);
    surfaceControl->setLayer(100000);
    SurfaceComposerClient::closeGlobalTransaction();

    for (int i = 0; i < TEST_FRAMES; i++) {
        status_t ret = producer.alloc(GRALLOC_USAGE_HW_COMPOSER, test_frames[i].w, test_frames[i].h, &test_frames[i].id);
        printf("[test] alloc id=%d, ret=%d\n", test_frames[i].id, ret);
    }

    int i = 0;
    int cc = 0;
    while(true) {
        printf("[test] %d ... ", i);
        showTestFrame(window, test_frames[i]);
        printf("done, f=%d\n", ++cc);

        i = i + 1;
        if (i >= TEST_FRAMES) {
            printf("\n... loop again ...\n");
            i = 0;
        }

        char value[PROPERTY_VALUE_MAX];
        property_get("debug.sftest.frame", value, "100");
        int num = atoi(value);
        if (cc > num)
            break;
    }

    for (int i = 0; i < TEST_FRAMES; i++) {
        status_t ret = producer.free(test_frames[i].id);
        printf("[test] free id=%d, ret=%d\n", test_frames[i].id, ret);
    }

    printf("test complete. CTRL+C to finish.\n");
    IPCThreadState::self()->joinThreadPool();
    return NO_ERROR;
}
