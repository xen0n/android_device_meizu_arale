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
#include <inc/CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
//
#include <camera_custom_zsd.h>
#include <camera/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkStereoCamAdapter.h>
using namespace NSMtkStereoCamAdapter;
//
#include <mtkcam/hal/IHalSensor.h>
using namespace NSCam;
//
#include <camera_custom_vt.h>
//
#include <cutils/properties.h>
//
#include <sys/prctl.h>
//
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
/******************************************************************************
*
*******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%d)(%s)[%s] " fmt, ::gettid(), getOpenId(), getName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, arg...)    if (cond) { MY_LOGV(arg); }
#define MY_LOGD_IF(cond, arg...)    if (cond) { MY_LOGD(arg); }
#define MY_LOGI_IF(cond, arg...)    if (cond) { MY_LOGI(arg); }
#define MY_LOGW_IF(cond, arg...)    if (cond) { MY_LOGW(arg); }
#define MY_LOGE_IF(cond, arg...)    if (cond) { MY_LOGE(arg); }

/******************************************************************************
*
*******************************************************************************/
bool
CamAdapter::
previewEnabled() const
{
    return (    mpStateManager->isState(IState::eState_Preview)||
                recordingEnabled());
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
startPreview()
{
    return  mpStateManager->getCurrentState()->onStartPreview(this);
}


/******************************************************************************
*
*******************************************************************************/
void
CamAdapter::
stopPreview()
{
    mpStateManager->getCurrentState()->onStopPreview(this);
}


/******************************************************************************
*   CamAdapter::startPreview() -> IState::onStartPreview() ->
*   IStateHandler::onHandleStartPreview() -> CamAdapter::onHandleStartPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStartPreview()
{
    MY_LOGD("+");
    //
    status_t ret = INVALID_OPERATION;
    //
    MINT32 frameRate = 24; // TODO: FIX hardcode
    MINT32 sensorFps = 0;
    MINT32 sensorMain2Fps = 0;
    //
    MBOOL bIsForceRatation = MFALSE, bConnectFeatrue = MFALSE;
    MINT32 rawType = 1, vdoWidth = 0, vdoHeight = 0, prvWidth = 0, prvHeight = 0;
    MUINT32 pass1RingBufNum = 3, sensorScenario = 0, sensorMain2Scenario = 0, rotationAnagle = 0;
    DefaultCtrlNode::MODE_TYPE modeType = DefaultCtrlNode::MODE_IDLE;
    DefaultCtrlNode::CONFIG_INFO configInfo, cfgInfo_Main2;
    ResMgrDrv::MODE_STRUCT mode;
    mpParamsMgr->getPreviewSize(&prvWidth, &prvHeight);
    mbZsdMode = ( ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_ZSD_MODE), MtkCameraParameters::ON) == 0 ) ? MTRUE : MFALSE;
    //
    mLastVdoWidth = 0;
    mLastVdoHeight = 0;
    //
    configInfo.replaceBufNumResize      = 0;
    configInfo.replaceBufNumFullsize    = 0;
    configInfo.rollbackBufNum           = 0;
    configInfo.rollbackBufPeriod        = 1;
    cfgInfo_Main2.replaceBufNumResize   = 0;
    cfgInfo_Main2.replaceBufNumFullsize = 0;
    cfgInfo_Main2.rollbackBufNum        = 0;
    cfgInfo_Main2.rollbackBufPeriod     = 1;
    //
    mpParamsMgr->getVideoSize(&vdoWidth, &vdoHeight);
    MY_LOGD("VDO:%dx%d = %d, 1080P(%d)",
            vdoWidth,
            vdoHeight,
            vdoWidth*vdoHeight,
            IMG_1080P_SIZE);
    //
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    IHalSensor* pSensorHalObj = NULL;
    if( pHalSensorList != NULL )
    {
        pSensorHalObj = pHalSensorList->createSensor(
                                            LOG_TAG,
                                            getOpenId());
    }
    else
    {
        MY_LOGE("pHalSensorList == NULL");
    }
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->getMode(&mode);
    }
    //
    if( mbZsdMode )
    {
        sensorScenario      = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorMain2Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        configInfo.rollbackBufNum       = get_zsd_cap_stored_frame_cnt();
        cfgInfo_Main2.rollbackBufNum    = get_zsd_cap_stored_frame_cnt();
        configInfo.rollbackBufPeriod    = get_zsd_cap_stored_update_period();
        cfgInfo_Main2.rollbackBufPeriod = get_zsd_cap_stored_update_period();
        modeType = DefaultCtrlNode::MODE_STEREO_ZSD_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        mUserName = "SZSD";
    }
    else
    {
        sensorScenario      = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        sensorMain2Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        //
        configInfo.rollbackBufNum   = 0;
        cfgInfo_Main2.rollbackBufNum= 0;
        modeType = DefaultCtrlNode::MODE_STEREO_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        //
        mUserName = "STEREO";
    }
    //
    if( pSensorHalObj != NULL )
    {
        pSensorHalObj->sendCommand( pHalSensorList->querySensorDevIdx(getOpenId_Main()),
                            SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                            (MINTPTR)&sensorScenario, (MINTPTR)&sensorFps, 0);
        pSensorHalObj->sendCommand( pHalSensorList->querySensorDevIdx(getOpenId_Main2()),
                            SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                            (MINTPTR)&sensorMain2Scenario, (MINTPTR)&sensorMain2Fps, 0);
        MY_LOGE_IF( 0==sensorFps || 0==sensorMain2Fps,
            "sensorFps(%d/%d) in scenario(%d/%d)",
            sensorFps, sensorMain2Fps, sensorScenario, sensorMain2Scenario);
        //NOTE: sensor default output frame rate is 10 base
        sensorFps /= 10;
        sensorMain2Fps /= 10;
        frameRate = (sensorFps < sensorMain2Fps )
                    ? ((sensorFps < frameRate) ? sensorFps : frameRate)
                    : ((sensorMain2Fps < frameRate) ? sensorMain2Fps : frameRate);
    }
    else
    {
        MY_LOGE("pSensorHalObj is NULL");
    }
    //
    if(frameRate > 30)
    {
        configInfo.replaceBufNumResize      += ((frameRate/30)*2+1);
        cfgInfo_Main2.replaceBufNumResize   += ((frameRate/30)*2+1);
    }
    //
    MY_LOGD("VHdr(%d),Name(%s),userName(%s),sensorScenario(%d),pass1RingBufNum(%d),sensorFps(%d),sensorMain2Fps(%d),frameRate(%d)",
            mpParamsMgr->getVHdr(),
            mName.string(),
            mUserName.string(),
            sensorScenario,
            pass1RingBufNum,
            sensorFps,
            sensorMain2Fps,
            frameRate);
    //
    if(mpResMgrDrv)
    {
        mpResMgrDrv->setMode(&mode);
    }
    //
    Pass1NodeInitCfg p1NodeInitCfg(
                        sensorScenario,
                        pass1RingBufNum,
                        10,
                        frameRate,
                        rawType);
    Pass1NodeInitCfg p1NodeInitCfg_Main2(
                        sensorMain2Scenario,
                        pass1RingBufNum,
                        10,
                        frameRate,
                        rawType);
    SCNodeInitCfg SCNodeInitCfg(
                        STEREO_CTRL_PREVIEW,
                        prvWidth, prvHeight,
                        prvWidth, prvHeight);
    //
    if  ( ! BaseCamAdapter::init() )
    {
        goto lbExit;
    }
    //
    mpCamGraph          = ICamGraph::createInstance(
                                        3,    // TODO: Main+Main2
                                        mUserName.string());
    //
    mpPass1Node         = Pass1Node::createInstance(p1NodeInitCfg);
    mpPass1Node_Main2   = Pass1Node::createInstance(p1NodeInitCfg_Main2);
    mpPass1Node->setSensorIdx( getOpenId_Main() );
    mpPass1Node_Main2->setSensorIdx( getOpenId_Main2() );
    mpStereoCtrlNode    = StereoCtrlNode::createInstance(SCNodeInitCfg);
    //
    //
    if(mpCapBufMgr)
    {
       mpCapBufMgr->setMaxDequeNum(configInfo.rollbackBufNum);
    }
    if(mpCapBufMgr_Main2)
    {
       mpCapBufMgr_Main2->setMaxDequeNum(cfgInfo_Main2.rollbackBufNum);
    }
    //
    configInfo.spParamsMgr              = mpParamsMgr;
    configInfo.spCamMsgCbInfo           = mpCamMsgCbInfo;
    configInfo.pCapBufMgr               = mpCapBufMgr;
    configInfo.replaceBufNumResize      = configInfo.rollbackBufNum;
    configInfo.replaceBufNumFullsize    = configInfo.rollbackBufNum;
    configInfo.previewMaxFps            = frameRate;
    cfgInfo_Main2.spParamsMgr           = mpParamsMgr;
    cfgInfo_Main2.spCamMsgCbInfo        = mpCamMsgCbInfo;
    cfgInfo_Main2.pCapBufMgr            = mpCapBufMgr_Main2;
    cfgInfo_Main2.replaceBufNumResize   = cfgInfo_Main2.rollbackBufNum;
    cfgInfo_Main2.replaceBufNumFullsize = configInfo.rollbackBufNum;
    cfgInfo_Main2.previewMaxFps         = frameRate;
    if(mpDefaultCtrlNode)
    {
        mpDefaultCtrlNode->setConfig(configInfo);
        mpDefaultCtrlNode->setParameters();
        mpDefaultCtrlNode->setMode(modeType);
    }
    if(mpDefaultCtrlNode_Main2)
    {
        mpDefaultCtrlNode_Main2->setConfig(cfgInfo_Main2);
        mpDefaultCtrlNode_Main2->setParameters();
        mpDefaultCtrlNode_Main2->setMode(modeType);
    }
    //
    if(bIsForceRatation)
    {
        mpDefaultBufHdl->setForceRotation(
                            MTRUE,
                            rotationAnagle);
        mpDefaultCtrlNode->setForceRotation(
                            MTRUE,
                            rotationAnagle);
        mpDefaultCtrlNode_Main2->setForceRotation(
                            MTRUE,
                            rotationAnagle);
    }
    //
    MY_LOGD("rawType(%d)",rawType);
    //
    MY_LOGD("AllocBufHandler:init");
    mpAllocBufHdl->init();
    mpAllocBufHdl_Main2->init();
    //
    MY_LOGD("DefaultBufHandler:map port");
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp,      TRANSFORM_DST_0, 0, MTRUE);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_AP,        TRANSFORM_DST_1);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Generic,   TRANSFORM_DST_1);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_FD,        STEREO_DST);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_OT,        STEREO_DST);
    //
    MY_LOGD("CamGraph:setBufferHandler");
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl,          getOpenId_Main());
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl,          getOpenId_Main());
    mpCamGraph->setBufferHandler(   STEREO_IMG,         mpAllocBufHdl,          getOpenId_Main());
    mpCamGraph->setBufferHandler(   TRANSFORM_DST_0,    mpDefaultBufHdl,        getOpenId_Main());
    mpCamGraph->setBufferHandler(   TRANSFORM_DST_1,    mpDefaultBufHdl,        getOpenId_Main());
    mpCamGraph->setBufferHandler(   STEREO_DST,         mpDefaultBufHdl,        getOpenId_Main());
    //
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl_Main2,    getOpenId_Main2());
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl_Main2,    getOpenId_Main2());
    mpCamGraph->setBufferHandler(   STEREO_IMG,         mpAllocBufHdl_Main2,    getOpenId_Main2());
    //
    MY_LOGD("CamGraph:connectData");
    // [ P1Node -- DCNode ]
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node_Main2,          mpDefaultCtrlNode_Main2);
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node,                mpDefaultCtrlNode);
    mpCamGraph->connectData(    PASS1_FULLRAW,      CONTROL_FULLRAW,    mpPass1Node_Main2,          mpDefaultCtrlNode_Main2);
    mpCamGraph->connectData(    PASS1_FULLRAW,      CONTROL_FULLRAW,    mpPass1Node,                mpDefaultCtrlNode);
    // [ DCNode -- SyncNode ]
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    SYNC_SRC_1,         mpDefaultCtrlNode_Main2,    mpSyncNode);
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    SYNC_SRC_0,         mpDefaultCtrlNode,          mpSyncNode);
    // [ SyncNode -- SNode (prepare stereo algo input data) ]
    mpCamGraph->connectData(    SYNC_DST_1,         STEREO_SRC,         mpSyncNode,                 mpStereoNode_Main2);
    mpCamGraph->connectData(    SYNC_DST_0,         STEREO_SRC,         mpSyncNode,                 mpStereoNode);
    // [ SNode -- SCNode (do stereo feature algo) ]
    mpCamGraph->connectData(    STEREO_IMG,         STEREO_CTRL_IMG_1,  mpStereoNode_Main2,         mpStereoCtrlNode);
    mpCamGraph->connectData(    STEREO_FEO,         STEREO_CTRL_FEO_1,  mpStereoNode_Main2,         mpStereoCtrlNode);
    mpCamGraph->connectData(    STEREO_IMG,         STEREO_CTRL_IMG_0,  mpStereoNode,               mpStereoCtrlNode);
    mpCamGraph->connectData(    STEREO_FEO,         STEREO_CTRL_FEO_0,  mpStereoNode,               mpStereoCtrlNode);
    // [ SCNode -- ITNode ]
    mpCamGraph->connectData(    STEREO_CTRL_DST_M,  TRANSFORM_SRC,      mpStereoCtrlNode,           mpImgTransformNode);
    //
    MY_LOGD("CamGraph:connectNotify");
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node_Main2,  mpDefaultCtrlNode_Main2);
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node,        mpDefaultCtrlNode);
    //
    if ( !mpCamGraph->init() ) {
        MY_LOGE("CamGraph:init");
        goto lbExit;
    }
    //
    if ( !mpCamGraph->start() ) {
        // use CAMERA_ERROR_SERVER_DIED callback to handle startPreview error
        mpCamMsgCbInfo->mNotifyCb(
                CAMERA_MSG_ERROR,
                CAMERA_ERROR_SERVER_DIED, 0,
                mpCamMsgCbInfo->mCbCookie
                );
        // do not return fail to avoid java exception
        mpStateManager->transitState(IState::eState_Preview);
        ret = OK;
        //
        MY_LOGE("CamGraph:start");
        goto lbExit;
    }
    //
    if( !mpDefaultCtrlNode->waitPreviewReady()
     || !mpDefaultCtrlNode_Main2->waitPreviewReady() )
    {
        MY_LOGE("wait preview ready failed");
        goto lbExit;
    }
    //
    MY_LOGD("transitState->eState_Preview");
    mpStateManager->transitState(IState::eState_Preview);
    MY_LOGD("-");
    ret = OK;
   //
lbExit:
    if(pSensorHalObj)
    {
        pSensorHalObj->destroyInstance(LOG_TAG);
        pSensorHalObj = NULL;
    }
    //
    if(ret != OK)
    {
        forceStopAndCleanPreview();
    }
    //
    return ret;
}


/******************************************************************************
*   CamAdapter::stopPreview() -> IState::onStopPreview() ->
*   IStateHandler::onHandleStopPreview() -> CamAdapter::onHandleStopPreview()
*******************************************************************************/
status_t
CamAdapter::
onHandleStopPreview()
{
    return forceStopAndCleanPreview();
}


/******************************************************************************
*   CamAdapter::takePicture() -> IState::onPreCapture() ->
*   IStateHandler::onHandlePreCapture() -> CamAdapter::onHandlePreCapture()
*******************************************************************************/
status_t
CamAdapter::
onHandlePreCapture()
{
    MY_LOGD("+");
    void* threadRet = NULL;
    if( pthread_create(&mThreadHandle, NULL, doThreadPrecap, this) != 0 )
    {
        MY_LOGE("pthread create failed");
        goto lbExit;
    }
    if( !mpDefaultCtrlNode->precap() )
    {
        MY_LOGE("precap failed");
        goto lbExit;
    }
    if( pthread_join(mThreadHandle, &threadRet) != 0 )
    {
        MY_LOGE("pthread join fail");
        goto lbExit;
    }
    if( !(bool)threadRet )
    {
        MY_LOGE("main2 precap failed");
        goto lbExit;
    }

    mpStateManager->transitState(IState::eState_PreCapture);
    MY_LOGD("-");
    return OK;

lbExit:
    return INVALID_OPERATION;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
CamAdapter::
main2precapture()
{
    MY_LOGD("+");
    return mpDefaultCtrlNode_Main2->precap();
}


/******************************************************************************
 *
 ******************************************************************************/
void*
CamAdapter::
doThreadPrecap(void* arg)
{
    ::prctl(PR_SET_NAME,"precapThread", 0, 0, 0);
    CamAdapter* pSelf = reinterpret_cast<CamAdapter*>(arg);
    return (void*)pSelf->main2precapture();
}


/******************************************************************************
*
*******************************************************************************/
status_t
CamAdapter::
forceStopAndCleanPreview()
{
    MY_LOGD("+");
    //
    if ( mpDefaultCtrlNode != NULL)
    {
        mpDefaultCtrlNode->setMode(DefaultCtrlNode::MODE_IDLE);
    }
    if ( mpDefaultCtrlNode_Main2 != NULL)
    {
        mpDefaultCtrlNode_Main2->setMode(DefaultCtrlNode::MODE_IDLE);
    }
    //
    if ( mpCamGraph != NULL )
    {
        mpCamGraph->stop();
        mpCamGraph->uninit();
        mpCamGraph->disconnect();
        mpCamGraph->destroyInstance();
        mpCamGraph = NULL;
    }
    //
    if ( mpPass1Node != NULL )
    {
        mpPass1Node->destroyInstance();
        mpPass1Node = NULL;
    }
    //
    if ( mpPass1Node_Main2 != NULL )
    {
        mpPass1Node_Main2->destroyInstance();
        mpPass1Node_Main2 = NULL;
    }
    //
    if ( mpStereoCtrlNode != NULL )
    {
        mpStereoCtrlNode->destroyInstance();
        mpStereoCtrlNode = NULL;
    }
    //
    if ( mpAllocBufHdl != NULL )
    {
        mpAllocBufHdl->uninit();
    }
    //
    if ( mpAllocBufHdl_Main2 != NULL )
    {
        mpAllocBufHdl_Main2->uninit();
    }
    //
    if ( mpDefaultBufHdl != NULL )
    {
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_Disp);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_AP);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_Generic);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_FD);
        mpDefaultBufHdl->unmapPort(DefaultBufHandler::eBuf_OT);
    }
    //
    MY_LOGD("transitState->eState_Idle");
    mpStateManager->transitState(IState::eState_Idle);
    //
    if(mpResMgrDrv)
    {
        ResMgrDrv::MODE_STRUCT mode;
        mpResMgrDrv->getMode(&mode);
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_IDLE;
        mpResMgrDrv->setMode(&mode);
    }
    //
    MY_LOGD("-");
    //
    return OK;
}

