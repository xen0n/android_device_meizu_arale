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

//#include <cutils/xlog.h>
#include <dlfcn.h>
#include <cutils/log.h>
#include <utils/Errors.h>
#include <gui/IGraphicBufferProducer.h>
#include <mmsdk/IEffectHal.h>


using namespace NSCam;
using namespace android;

#if '1' == MTKCAM_HAVE_SDK_CLIENT
#include <mtkcam/v3/utils/FeatureMgr/FeatureMgr.h>
using namespace NSCam::v3;
using namespace NSCam::v3::Utils;
#endif

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)




FeatureManager::
FeatureManager()
{
    FUNCTION_LOG_START;
    //@todo implement this - use dlopen
    //mpEffectFactory = new EffectFactory();
    FUNCTION_LOG_END;
}


FeatureManager::
~FeatureManager()
{
    FUNCTION_LOG_START;
    mpEffectFactory = NULL;
    FUNCTION_LOG_END;
}


status_t
FeatureManager::
setParameter(android::String8 key, android::String8 value)
{
    FUNCTION_LOG_START;
#if '1' == MTKCAM_HAVE_SDK_CLIENT
    if (key == "3dnr")
    {
        NSCam::v3::Utils::FeatureMgr* pFeatureMgr = NSCam::v3::Utils::FeatureMgr::getFeatureMgrSingleInst();
        const char *v = value.string();
        pFeatureMgr->updateCameraMode(strtol(v, 0, 0), eFeatureMgr_FeatureMap_3DNR);
    }
#endif
    FUNCTION_LOG_END;
    return OK;
}


android::String8
FeatureManager::
getParameter(android::String8 key)
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
    return String8("@todo implement this");
}


status_t
FeatureManager::
setUp(EffectHalVersion const &nameVersion)
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}


status_t
FeatureManager::
tearDown(EffectHalVersion const &nameVersion)
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}


status_t
FeatureManager::
getEffectFactory(sp<IEffectFactory> & effectFactory)
{
    FUNCTION_LOG_START;
    status_t result = OK;

    //@todo implement this
    result = createEffectFactory();
    effectFactory = mpEffectFactory;

    FUNCTION_LOG_END;
    return result;
}

status_t
FeatureManager::
createEffectFactory()
{
    if(mpEffectFactory != NULL)
    {
        return  OK;
    }

    //
    char const szModulePath[] = "libfeatureio.featurefactory.so";
    char const szEntrySymbol[] = "createEffectFactory";
    void* pfnEntry = NULL;
    //IPlatform* pIPlatform = NULL;
    void *gLib = NULL;
    IEffectFactory *pIEffectFactory = NULL;

    //
    gLib = ::dlopen(szModulePath, RTLD_NOW);
    if  ( ! gLib )
    {
        char const *err_str = ::dlerror();
        CAM_LOGE("dlopen: %s error=%s", szModulePath, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pfnEntry = ::dlsym(gLib, szEntrySymbol);
    if  ( ! pfnEntry )
    {
        char const *err_str = ::dlerror();
        CAM_LOGE("dlsym: %s error=%s", szEntrySymbol, (err_str ? err_str : "unknown"));
        goto lbExit;
    }
    //
    pIEffectFactory = reinterpret_cast<IEffectFactory*(*)()>(pfnEntry)();
    if  ( ! pIEffectFactory )
    {
        CAM_LOGE("No hardware instance");
        goto lbExit;
    }
    //
    mpEffectFactory = pIEffectFactory;
    //
lbExit:
    /* Because FeatureManage always exist in mediaserver process, we don't need to close this gLib.
    * If we use @mpEffectFactory after dlclose(gLib), its behavior will become "undefined." (maybe crash)*/
    //if( gLib != NULL)
    //{
    //    ::dlclose(gLib);
    //    gLib = NULL;
    //}
    //M_LOGD("%s():%p return %p in %s", szEntrySymbol, pfnEntry, gIPlatform, szModulePath);
    //return gIPlatform;
    //M_LOGD("%s():%p return %p in %s", szEntrySymbol, pfnEntry, mpEffectFactory, szModulePath);
    return OK;
}

