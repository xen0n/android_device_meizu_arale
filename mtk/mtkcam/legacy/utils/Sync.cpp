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

#define LOG_TAG "MtkCam/Sync"

//
#define __STDC_LIMIT_MACROS
#include <stdint.h>
#include "MyUtils.h"
#include <utils/Sync.h>
//
#include <sync/sync.h>
#include <sw_sync.h>
//



using namespace android;
using namespace NSCam;
using namespace NSCam::Utils::Sync;
//


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


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class FenceImp : public IFence
{
    friend  class IFence;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Data Members.
    MINT                                mFenceFd;
    android::String8                    mFenceName;

protected:  ////
    virtual                             ~FenceImp();
                                        FenceImp(MINT fenceFd = -1);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IFence Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.

    virtual MINT                        dup() const;

    virtual MERROR                      wait(MINT timeoutMs);

    virtual MERROR                      waitForever(char const* logname);

public:     ////                        Attributes.

    virtual char const*                 name() const    { return mFenceName.string(); }

    virtual MBOOL                       isValid() const { return (mFenceFd != -1); }

    virtual MINT                        getFd() const   { return mFenceFd; }

    virtual MINT64                      getSignalTime() const;

};
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IFence> const IFence::NO_FENCE(new FenceImp);


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IFence>
IFence::
create(MINT fenceFd)
{
    return new FenceImp(fenceFd);
}


/******************************************************************************
 *
 ******************************************************************************/
android::sp<IFence>
IFence::
merge(
    char const* szFenceName,
    android::sp<IFence> const& f1,
    android::sp<IFence> const& f2
)
{
    CAM_TRACE_CALL();
    //
    int result;
    if (f1->isValid() && f2->isValid()) {
        result = ::sync_merge(szFenceName, f1->getFd(), f2->getFd());
    } else if (f1->isValid()) {
        result = ::sync_merge(szFenceName, f1->getFd(), f1->getFd());
    } else if (f2->isValid()) {
        result = ::sync_merge(szFenceName, f2->getFd(), f2->getFd());
    } else {
        return NO_FENCE;
    }

    if (result == -1) {
        MY_LOGE("Error merge: sync_merge(\"%s\", %d, %d)",
                szFenceName, f1->getFd(), f2->getFd());
        return NO_FENCE;
    }

    return new FenceImp(result);
}


/******************************************************************************
 *
 ******************************************************************************/
FenceImp::
~FenceImp()
{
    if  ( mFenceFd != -1 ) {
        ::close(mFenceFd);
        mFenceFd = -1;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
FenceImp::
FenceImp(MINT fenceFd)
    : mFenceFd(fenceFd)
{
    if  ( 0 <= fenceFd  ) {
        struct sync_fence_info_data *info = ::sync_fence_info(mFenceFd);
        if  ( info != NULL ) {
            mFenceName = info->name;
        }
        ::sync_fence_info_free(info);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
FenceImp::
dup() const
{
    return ::dup(mFenceFd);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FenceImp::
wait(MINT timeoutMs)
{
    CAM_TRACE_CALL();
    //
    if  ( mFenceFd == -1 ) {
        return  OK;
    }
    int err = ::sync_wait(mFenceFd, timeoutMs);
    return  err < 0 ? -errno : MERROR(OK);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FenceImp::
waitForever(char const* logname)
{
    CAM_TRACE_CALL();

    if (mFenceFd == -1) {
        return  OK;
    }
    unsigned int warningTimeout = 3000;
    int err = ::sync_wait(mFenceFd, warningTimeout);
    if (err < 0 && errno == ETIME) {
        MY_LOGW("%s: fence %d didn't signal in %u ms", logname, mFenceFd,
                warningTimeout);

        err = ::sync_wait(mFenceFd, TIMEOUT_NEVER);
    }
    return  err < 0 ? -errno : MERROR(OK);
}

/******************************************************************************
 *
 ******************************************************************************/
MINT64
FenceImp::
getSignalTime() const
{
    if (mFenceFd == -1) {
        return -1;
    }

    struct sync_fence_info_data* finfo = ::sync_fence_info(mFenceFd);
    if (finfo == NULL) {
        MY_LOGE("sync_fence_info returned NULL for fd %d", mFenceFd);
        return -1;
    }
    if (finfo->status != 1) {
        ::sync_fence_info_free(finfo);
        return INT64_MAX;
        //return -1;
    }

    struct sync_pt_info* pinfo = NULL;
    uint64_t timestamp = 0;
    while ((pinfo = ::sync_pt_info(finfo, pinfo)) != NULL) {
        if (pinfo->timestamp_ns > timestamp) {
            timestamp = pinfo->timestamp_ns;
        }
    }
    ::sync_fence_info_free(finfo);

    return nsecs_t(timestamp);
}


/******************************************************************************
 *
 ******************************************************************************/
namespace {
class TimelineImp : public ITimeline
{
    friend  class ITimeline;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////                        Data Members.
    MINT                                mTimelineFd;
    android::String8                    mName;

protected:  ////
    virtual                             ~TimelineImp();
                                        TimelineImp(
                                            MINT timelineFd,
                                            char const* szTimelineName
                                        );

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  ISyncTimeline Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                        Operations.

    virtual MERROR                      inc(size_t count);

    virtual MINT                        createFence(
                                            char const* szFenceName,
                                            size_t value
                                        );

public:     ////                        Attributes.

    virtual char const*                 name() const    { return mName.string(); }

};
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<ITimeline>
ITimeline::
create(char const* szTimelineName)
{
    MINT timelineFd = ::sw_sync_timeline_create();
    if  ( timelineFd < 0 ) {
        MY_LOGE("sw_sync_timeline");
        return  0;
    }
    //
    return new TimelineImp(timelineFd, szTimelineName);
}


/******************************************************************************
 *
 ******************************************************************************/
TimelineImp::
~TimelineImp()
{
    if  ( mTimelineFd != -1 ) {
        ::close(mTimelineFd);
        mTimelineFd = -1;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
TimelineImp::
TimelineImp(
    MINT timelineFd,
    char const* szTimelineName
)
    : mTimelineFd(timelineFd)
    , mName(szTimelineName)
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
TimelineImp::
inc(size_t count)
{
    CAM_TRACE_CALL();

    int err;
    err = sw_sync_timeline_inc(mTimelineFd, count);
    if (err < 0) {
        MY_LOGE("can't increment sync obj:");
        return -1;
    }

    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT
TimelineImp::
createFence(
    char const* szFenceName,
    size_t value
)
{
    if  ( ! szFenceName ) {
        MY_LOGE("Null fence name passed !!");
        return  -1;
    }
    //
    MINT fd = ::sw_sync_fence_create(mTimelineFd, szFenceName, value);
    if  ( fd < 0 ) {
        MY_LOGE(
            "sw_sync_fence_create, timelineFd:%d, fenceName:%s value:%zu",
            mTimelineFd, szFenceName, value
        );
        return  -1;
    }
    return  fd;
}

