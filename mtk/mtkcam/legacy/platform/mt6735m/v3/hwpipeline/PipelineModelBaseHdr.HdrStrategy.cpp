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

#define LOG_TAG "MtkCam/HwPipeline"
//
#include "MyUtils.h"
#include <mtkcam/v3/hwpipeline/PipelineModelBaseHdr.h>
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


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineModelBaseHdr::
evaluateAppRequest(
    evaluate_app_request_params& rParams
)
{
    Pipeline::HdrStrategy::evaluate_request::Params params;
    params.subFrameCnt    = mSubRequestCnt;
    params.requestNo      = rParams.pAppRequest->requestNo;
    params.frameNo        = params.requestNo*params.subFrameCnt;
    params.pAppCallback   = rParams.pAppRequest->pAppCallback;
    //params.pHdrCallback   = this;//rParams.pAppRequest->pPipelineCallback;
    params.vIImageBuffers = &rParams.pAppRequest->vIImageBuffers;
    params.vOImageBuffers = &rParams.pAppRequest->vOImageBuffers;
    params.vIMetaBuffers  = &rParams.pAppRequest->vIMetaBuffers;
    params.vOMetaBuffers  = &rParams.vOMetaBuffers;
    params.ppFrame        = &rParams.pFrame;
    params.vFrame         = &rParams.vFrame;
//FIX ME:
    for ( size_t i=0; i<params.subFrameCnt; i++) {
        if(mpPipelineFrameNumberGenerator == 0) {
            MY_LOGW("PipelineFrameNumberGenerator is NULL, set frameNo to requestNo:%d", rParams.pAppRequest->requestNo);
            params.frameNo    = rParams.pAppRequest->requestNo;
        } else {
            if ( i==0 )
                params.frameNo    = mpPipelineFrameNumberGenerator->generateFrameNo();
            else
                mpPipelineFrameNumberGenerator->generateFrameNo();
        }
    }
    return Pipeline::HdrStrategy::evaluate_request()(params, this);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineModelBaseHdr::
refineRequestAppParams(
    evaluate_request_pipeline::Params& rParams
)   const
{
    MERROR err = OK;
    RWLock::AutoRLock _l(mRWLock);
    //
    {
        refine_request_app_params_based_on_face_detect_params params;
        params.pIAppMetaData= rParams.pIAppMetaData;
        params.pAppStreams  = rParams.pAppStreams;
        if  (OK != (err = refineRequestAppParamsLocked(params))) {
            return err;
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
PipelineModelBaseHdr::
refineRequestAppParamsLocked(
    refine_request_app_params_based_on_face_detect_params& rParams
)   const
{
    //  If Face detection is not OFF or scene mode is face priority,
    //  add App:Meta:FD_result stream to Output App Meta Streams.
    IMetadata::IEntry const& entryFdMode = rParams.pIAppMetaData->entryFor(MTK_STATISTICS_FACE_DETECT_MODE);
    IMetadata::IEntry const& entryfaceScene = rParams.pIAppMetaData->entryFor(MTK_CONTROL_SCENE_MODE);

    bool openFD = (0 != mDebugFdMode) ||
             ( !entryFdMode.isEmpty() && MTK_STATISTICS_FACE_DETECT_MODE_OFF != entryFdMode.itemAt(0, Type2Type<MUINT8>())) ||
             ( !entryfaceScene.isEmpty() && MTK_CONTROL_SCENE_MODE_FACE_PRIORITY == entryfaceScene.itemAt(0, Type2Type<MUINT8>()));

    if (openFD)
    {
        sp<IMetaStreamInfo> pStreamInfo = getMetaStreamInfoLocked_FaceDetectResult();
        if ( pStreamInfo.get() )
            rParams.pAppStreams->vOAppMetaStreams.add(pStreamInfo->getStreamId(), pStreamInfo);
    }

    //
    return  OK;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<IMetaStreamInfo>
PipelineModelBaseHdr::
getMetaStreamInfoLocked_FaceDetectResult() const
{
    MY_LOGV("Not supported");
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MUINT
PipelineModelBaseHdr::
queryMinimalUsage(
    NodeId_T const /*nodeId*/,
    IImageStreamInfo const*const /*pStreamInfo*/
)   const
{
    return 0;
}

