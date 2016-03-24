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

#define LOG_TAG "MtkCam/Cam3Device"
//
#include "../MyUtils.h"
#include <mtkcam/v3/device/default/Cam3DeviceCommon.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[Cam3DeviceCommon::%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
Cam3DeviceCommon::
Cam3DeviceCommon(
    String8 const&          rDevName,
    int32_t const           i4OpenId
)
    : Cam3DeviceBase(rDevName, i4OpenId)
    , mpPipelineFrameNumberGenerator(
        IPipelineFrameNumberGenerator::create()
    )
{
    //mpPipelineFrameNumberGenerator = IPipelineFrameNumberGenerator::create();
}

/******************************************************************************
 *
 ******************************************************************************/
Cam3DeviceCommon::
~Cam3DeviceCommon()
{}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
Cam3DeviceCommon::
updateFrame(
    MUINT32 const frameNo,
    MINTPTR const userId,
    ssize_t const nOutMetaLeft,
    android::Vector<android::sp<IMetaStreamBuffer> > vOutMeta
)
{
    MY_LOGD_IF(getLogLevel() >= 2, "frameNo:%u + userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", frameNo, userId, nOutMetaLeft, vOutMeta.size());
    NSCam::Utils::CamProfile profile(__FUNCTION__, "Cam3DeviceCommon");
    //
    sp<IAppStreamManager> pAppStreamManager = getAppStreamManager();
    if  ( pAppStreamManager == 0 ) {
        MY_LOGE("NULL AppStreamManager");
        return;
    }
    profile.print_overtime(1, "getAppStreamManager: frameNo:%u userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", frameNo, userId, nOutMetaLeft, vOutMeta.size());
    //
    pAppStreamManager->updateResult(frameNo, userId, vOutMeta, nOutMetaLeft <= 0);
    profile.print_overtime(1, "updateResult: frameNo:%u userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", frameNo, userId, nOutMetaLeft, vOutMeta.size());
    MY_LOGD_IF(getLogLevel() >= 2, "frameNo:%u - userId:%#" PRIxPTR " OAppMeta#(left:%zd this:%zu)", frameNo, userId, nOutMetaLeft, vOutMeta.size());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
Cam3DeviceCommon::
finishPipelineConfiguration(
    sp<IPipelineModel> aPipeline
)
{
    return aPipeline->setPipelineFrameNumberGenerator(getPipelineFrameNumberGenerator());
}