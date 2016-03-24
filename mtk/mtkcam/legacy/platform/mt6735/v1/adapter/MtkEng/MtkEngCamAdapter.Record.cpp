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

#define LOG_TAG "MtkCam/CamAdapter"
//
#include <camera/MtkCamera.h>
//
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include "inc/MtkEngCamAdapter.h"
using namespace NSMtkEngCamAdapter;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//


/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
*   Function Prototype.
*******************************************************************************/
status_t
CamAdapter::
startRecording()
{
    status_t status = OK;
    //
    if(recordingEnabled())
    {
        MY_LOGW("Recording has been started");
    }
    else
    {
        int32_t vdoWidth = 0, vdoHeight = 0;
        mpParamsMgr->getVideoSize(&vdoWidth, &vdoHeight);
        MY_LOGD("VDO:%dx%d, %d",
                vdoWidth,
                vdoHeight,
                IMG_1080P_SIZE);
        //
        if( mLastVdoWidth != vdoWidth ||
            mLastVdoHeight != vdoHeight)
        {
            if(vdoWidth*vdoHeight > IMG_1080P_SIZE)
            {
                mpStateManager->getCurrentState()->onStopPreview(this);
                mpStateManager->getCurrentState()->onStartPreview(this);
            }
            mLastVdoWidth = vdoWidth;
            mLastVdoHeight = vdoHeight;
        }
        //
        status = mpStateManager->getCurrentState()->onStartRecording(this);
        if  ( OK != status ) {
            goto lbExit;
        }
    }
    //
lbExit:
    if  ( OK == status ) {
        CamManager* pCamMgr = CamManager::getInstance();
        pCamMgr->setRecordingHint(true);
    }
    return status;
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
stopRecording()
{
    if(recordingEnabled())
    {
        mpStateManager->getCurrentState()->onStopRecording(this);
    }
    else
    {
        MY_LOGW("Recording has been stopped");
    }
    CamManager* pCamMgr = CamManager::getInstance();
    pCamMgr->setRecordingHint(false);
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleStartRecording()
{
    MY_LOGD("+");
    //
    status_t ret = DEAD_OBJECT;
    MINT32 vdoWidth, vdoHeight;
    //
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        //
        if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= 30)
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC;
        }
        else
        {
            mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_REC_HS;
        }
        //
        mpResMgrDrv->setMode(&mode);
    }
    //
    MINT32 recW, recH;
    mpParamsMgr->getVideoSize(&recW, &recH);
    if(mbFixFps)
    {
        if( (   mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30 &&
                recW >= IMG_720P_W) ||
            (recW > IMG_1080P_W))
        {
            mspThermalMonitor->setFrameRate(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE));
            mspThermalMonitor->start();
        }
    }
    //
    mpEngBufHdl->unmapPort(EngBufHandler::eBuf_AP);
    mpEngBufHdl->unmapPort(EngBufHandler::eBuf_Generic);
    //
    mpParamsMgr->getVideoSize(&vdoWidth, &vdoHeight);
    //
    if(::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT), MtkCameraParameters::PIXEL_FORMAT_BITSTREAM) == 0)
    {
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Rec, PASS2_PRV_DST_3, MTRUE);
        //
        mpPass2Node->startRecording(
                        vdoWidth,
                        vdoHeight,
                        mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
                        MFALSE);
    }
    else
    {
        if(mbTwoRunP2)
        {
            mpEngBufHdl->mapPort(EngBufHandler::eBuf_Rec, TRANSFORM_DST_1, MTRUE);
        }
        else
        {
            mpEngBufHdl->mapPort(EngBufHandler::eBuf_Rec, PASS2_PRV_DST_1, MTRUE);
        }
        //
        mpPass2Node->startRecording(
                        vdoWidth,
                        vdoHeight,
                        mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
                        MFALSE);
    }
    //
    if(mpEngCtrlNode)
    {
        mpEngCtrlNode->setMode(DefaultCtrlNode::MODE_VIDEO_RECORD);
        //
        if(mpEngCtrlNode->isRecBufSnapshot())
        {
            int vdoWidth = 0, vdoHeight = 0;
            mpParamsMgr->getVideoSize(&vdoWidth, &vdoHeight);
            mpRecBufSnapshotlNode->startRecording(
                                        vdoWidth,
                                        vdoHeight,
                                        Format::queryImageFormat(mpParamsMgr->getStr(MtkCameraParameters::KEY_VIDEO_FRAME_FORMAT)));
        }
    }
    //
    mpStateManager->transitState(IState::eState_Recording);
    //
    MY_LOGD("Start recording");
    //
    ret = OK;
lbExit:
    //
    MY_LOGD("-");
    //
    return  ret;
}


/******************************************************************************
*
*
*******************************************************************************/
status_t
CamAdapter::
onHandleStopRecording()
{
    MY_LOGD("+");
    //
    status_t ret = OK;
    //
    if(mpStateManager->isState(IState::eState_VideoSnapshot))
    {
        // wait for previous callback is done
        waitForShotDone();
    }
    //
    if(mspThermalMonitor->isRunning())
    {
        mspThermalMonitor->stop();
    }
    //
    if(mpEngCtrlNode)
    {
        mpEngCtrlNode->setMode(DefaultCtrlNode::MODE_VIDEO_PREVIEW);
        if(mpEngCtrlNode->isRecBufSnapshot())
        {
            mpRecBufSnapshotlNode->stopRecording();
        }
    }
    //
    if(mpPass2Node)
    {
        mpPass2Node->stopRecording();
    }
    //
    mpEngBufHdl->unmapPort(EngBufHandler::eBuf_Rec);
    if(mbTwoRunP2)
    {
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_AP,        TRANSFORM_DST_1);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Generic,   TRANSFORM_DST_1);
    }
    else
    {
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_AP,        PASS2_PRV_DST_1);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Generic,   PASS2_PRV_DST_1);
    }
    //
    mpStateManager->transitState(IState::eState_Preview);
    //
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_PRV;
        mpResMgrDrv->setMode(&mode);
    }
    //
    MY_LOGD("Stop recording");
    //
lbExit:
    //
    MY_LOGD("-");
    //
    return  ret;
}


/******************************************************************************
*
*
*******************************************************************************/
bool
CamAdapter::
recordingEnabled() const
{

    return (    mpStateManager->isState(IState::eState_Recording)||
                mpStateManager->isState(IState::eState_VideoSnapshot));
}


