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

#include <utils/Log.h>
#include <cutils/memory.h>
//
#include <binder/IPCThreadState.h>
#include <binder/ProcessState.h>
#include <binder/IServiceManager.h>
//
/*
#include <surfaceflinger/Surface.h>
#include <surfaceflinger/ISurface.h>
#include <surfaceflinger/SurfaceComposerClient.h>
*/
//
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
//
#include "inc/CamLog.h"
#include "inc/Utils.h"
#include "inc/MyCamera.h"
//
using namespace android;
//
//
//
//
//
MyCamera::
MyCamera()
    : CameraListener()
    , mpCamera(0)
    , mi4PreviewWidth(640)
    , mi4PreviewHeight(480)
{
}


//
//
//
bool
MyCamera::
open(sp<MyCamera> rpMyCamera, int id)
{
    sp<Camera> pCamera = Camera::connect(id, String16("CamTest"), 0);
    if  ( pCamera == 0 )
    {
        MY_LOGE("Camera::connect, id(%d)", id);
        rpMyCamera = NULL;
        return  false;
    }
    //
    //
    rpMyCamera->mpCamera = pCamera;
    pCamera->setListener(rpMyCamera);
    //
    MY_LOGD("Camera::connect, id(%d), camera(%p)", id, pCamera.get());
    return  true;
}


//
//
//
void
MyCamera::
close()
{
    if  ( mpCamera != 0 )
    {
        MY_LOGD("Camera::disconnect, camera(%p)", mpCamera.get());
        mpCamera->disconnect();
        mpCamera = NULL;
    }
}


//
//
//
bool
MyCamera::
setPreviewFormat(char const* format)
{
    CameraParameters params(get()->getParameters());
    params.setPreviewFormat(format);

    String8 s = params.flatten();
    if  (OK != get()->setParameters(s))
    {
        MY_LOGE("%p", format);
        MY_LOGE("%s", format);
        return  false;
    }
    return  true;
}


//
//
//
bool
MyCamera::
setPreviewSize(int const w, int const h)
{
    MY_LOGD("w/h=%d/%d", w, h);
    //
    CameraParameters params(get()->getParameters());
    params.setPreviewSize(w, h);

    String8 s = params.flatten();
    if  (OK != get()->setParameters(s))
    {
        MY_LOGE("");
        return  false;
    }
    //
    mi4PreviewWidth = w;
    mi4PreviewHeight = h;
    return  true;
}


