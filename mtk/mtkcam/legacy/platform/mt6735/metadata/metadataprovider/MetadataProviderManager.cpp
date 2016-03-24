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

#define LOG_TAG "MtkCam/Metadata"
//
#include <utils/RWLock.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/Log.h>
#include <mtkcam/metadata/IMetadataProvider.h>


/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


namespace NSCam {
namespace NSMetadataProviderManager {
/******************************************************************************
 *
 ******************************************************************************/
namespace
{
    typedef DefaultKeyedVector<int32_t, sp<IMetadataProvider> > Map_t;
    Map_t               gMap;
    RWLock              gRWLock;
};


/******************************************************************************
 *
 ******************************************************************************/
void
clear()
{
    RWLock::AutoWLock _l(gRWLock);
    gMap.clear();
    MY_LOGD("pid:%d tid:%d", ::getpid(), ::gettid());
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
add(int32_t deviceId, IMetadataProvider* pProvider)
{
    RWLock::AutoWLock _l(gRWLock);
    ssize_t index = gMap.add(deviceId, pProvider);
    MY_LOGD("[%d] deviceId:%d -> %p", index, deviceId, pProvider);
    return index;
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadataProvider*
valueFor(int32_t deviceId)
{
    RWLock::AutoRLock _l(gRWLock);
    sp<IMetadataProvider> p = gMap.valueFor(deviceId);
    return  p.get();
}


/******************************************************************************
 *
 ******************************************************************************/
IMetadataProvider*
valueAt(size_t index)
{
    RWLock::AutoRLock _l(gRWLock);
    sp<IMetadataProvider> p = gMap.valueAt(index);
    return  p.get();
}


/******************************************************************************
 *
 ******************************************************************************/
int32_t
keyAt(size_t index)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gMap.keyAt(index);
}


/******************************************************************************
 *
 ******************************************************************************/
ssize_t
indexOfKey(int32_t deviceId)
{
    RWLock::AutoRLock _l(gRWLock);
    return  gMap.indexOfKey(deviceId);
}


/******************************************************************************
 *
 ******************************************************************************/
};  // namespace NSMetadataProviderManager
};  // namespace NSCam

