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

#define LOG_TAG "MtkCam/inFlightRequest"
//
#include "MyUtils.h"
#include <mtkcam/v3/hwpipeline/InFlightRequest.h>
#include <mtkcam/Trace.h>
#include <cutils/properties.h>
//
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;


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

#define MY_LOGD1(...)               MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)

/******************************************************************************
 *
 ******************************************************************************/
InFlightRequest::
InFlightRequest()
    : mLock()
    , mRequestCond()
    , mRequest()
    , mRequestMap_meta()
    , mRequestMap_image()
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.inflightrequest", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
registerRequest(
    sp<IPipelineFrame> const&pFrame
)
{
    MY_LOGD1("+");
    Mutex::Autolock _l(mLock);
    //add request
    mRequest.push_back(pFrame->getFrameNo());

    //add node
    Vector<IPipelineDAG::NodeObj_T>const& vNode = pFrame->getPipelineDAG().getToposort();
    for(size_t s = 0; s < vNode.size(); s++) {
        NodeId_T node = vNode[s].id;

        sp<IStreamInfoSet const> in, out;
        if( OK != pFrame->queryIOStreamInfoSet(node, in, out)) {
            MY_LOGE("queryIOStreamInfoSet failed");
            break;
        }
        //
        if( out->getImageInfoNum() > 0 || out->getMetaInfoNum() > 0) {
            if (mRequestMap_image.indexOfKey(node) < 0) {
                RequestList frameL;
                frameL.push_back(pFrame->getFrameNo());
                mRequestMap_image.add(node, frameL);
            } else {
                mRequestMap_image.editValueFor(node).push_back(pFrame->getFrameNo());
            }

            if (mRequestMap_meta.indexOfKey(node) < 0) {
                RequestList frameL;
                frameL.push_back(pFrame->getFrameNo());
                mRequestMap_meta.add(node, frameL);
            } else {
                mRequestMap_meta.editValueFor(node).push_back(pFrame->getFrameNo());
            }
        }

    }
    // register
    pFrame->attachListener(this, NULL);
    MY_LOGD1("-");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilDrained()
{
    CAM_TRACE_CALL();

    MY_LOGD_IF(1, "+");
    Mutex::Autolock _l(mLock);
    while(!mRequest.empty())
    {
        MY_LOGD_IF(1, "frameNo:%u in the front", *(mRequest.begin()));
        mRequestCond.wait(mLock);
    }
    MY_LOGD_IF(1, "-");
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeDrained(NodeId_T id)
{
    CAM_TRACE_CALL();

    waitUntilNodeMetaDrained(id);
    waitUntilNodeImageDrained(id);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeMetaDrained(NodeId_T id)
{

Mutex::Autolock _l(mLock);
    while(mRequestMap_meta.indexOfKey(id) >= 0 &&
          !mRequestMap_meta.valueFor(id).empty())
    {
         MY_LOGD_IF(1, "Node: %d has frameNo: %d in the front",
            id, *(mRequestMap_meta.valueFor(id).begin()));
         mRequestCond.wait(mLock);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
waitUntilNodeImageDrained(NodeId_T id)
{

Mutex::Autolock _l(mLock);
    while(mRequestMap_image.indexOfKey(id) >= 0 &&
          !mRequestMap_image.valueFor(id).empty())
    {
        MY_LOGD_IF(1, "Node: %d has frameNo: %d in the front",
           id, *(mRequestMap_image.valueFor(id).begin()));
         mRequestCond.wait(mLock);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
onPipelineFrame(
    MUINT32 const frameNo,
    MUINT32 const message,
    MVOID*const /*pCookie*/
)
{
    MY_LOGD1("frame: %d message: %#x", frameNo, message);
    Mutex::Autolock _l(mLock);
    if(message == eMSG_FRAME_RELEASED){
        for(RequestList::iterator it = mRequest.begin(); it != mRequest.end(); it++) {
            if(*it == frameNo) {
                mRequest.erase(it);
                mRequestCond.broadcast();
                break;
            }
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
onPipelineFrame(
    MUINT32 const frameNo,
    NodeId_T const nodeId,
    MUINT32 const message,
    MVOID*const /*pCookie*/
)
{
    MY_LOGD1("frame: %d, nodeId: %d, msg: %d", frameNo, nodeId, message);

    Mutex::Autolock _l(mLock);
    if(mRequestMap_meta.indexOfKey(nodeId) < 0 && mRequestMap_image.indexOfKey(nodeId) < 0) {
        MY_LOGE("no node in meta/image mapper: %d", nodeId);
        return;
    }

    if (message == eMSG_ALL_OUT_META_BUFFERS_RELEASED) {
        RequestList& list = mRequestMap_meta.editValueFor(nodeId);
        for(RequestList::iterator it = list.begin(); it != list.end(); it++) {
            if(*it == frameNo) {
                list.erase(it);
                break;
            }
        }
        mRequestCond.broadcast();
    }
    else if (message == eMSG_ALL_OUT_IMAGE_BUFFERS_RELEASED) {
        RequestList& list = mRequestMap_image.editValueFor(nodeId);
        for(RequestList::iterator it = list.begin(); it != list.end(); it++) {
            if(*it == frameNo) {
                list.erase(it);
                break;
            }
        }
        mRequestCond.broadcast();
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
InFlightRequest::
clear()
{
    Mutex::Autolock _l(mLock);
    mRequestMap_meta.clear();
    mRequestMap_image.clear();
    mRequest.clear();
}

