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
#include <cutils/properties.h>      // [debug] should be remove
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
#include <inc/MtkEngCamAdapter.h>
using namespace NSMtkEngCamAdapter;
//
#include <mtkcam/hal/IHalSensor.h>
using namespace NSCam;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <camera_custom_vt.h>
//
#include <cutils/properties.h>
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
    MINT32 sensorFps = 0;
    //
    MBOOL bIsForceRatation = MFALSE, bConnectFullRaw = MFALSE, bConnectFeatrue = MFALSE, bSlowMotion = MFALSE, bVHdr = MFALSE;
    MINT32 rawType = 1, vdoWidth = 0, vdoHeight = 0, prvWidth = 0, prvHeight = 0;
    MUINT32 pass1RingBufNum = 3, sensorScenario = 0, rotationAnagle = 0;
    DefaultCtrlNode::MODE_TYPE modeType = DefaultCtrlNode::MODE_IDLE;
    DefaultCtrlNode::CONFIG_INFO configInfo;
    ResMgrDrv::MODE_STRUCT mode;
    //
    mpParamsMgr->getPreviewSize(&prvWidth, &prvHeight);
    mPreviewMaxFps = mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE); // default value
    //
    if(!(CamManager::getInstance()->isMultiDevice()))
    {
        bVHdr = mpParamsMgr->getVHdr();
    }
    //
    mLastVdoWidth = 0;
    mLastVdoHeight = 0;
    //
    configInfo.replaceBufNumResize = 0;
    configInfo.replaceBufNumFullsize = 0;
    configInfo.rollbackBufNum = 0;
    configInfo.rollbackBufPeriod = 1;
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
    if(mpParamsMgr->getRecordingHint())
    {
        if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30)
        {
            bSlowMotion = MTRUE;
            mbTwoRunP2 = MFALSE;
            configInfo.rollbackBufNum = 0;
            sensorScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
        }
        else
        {
            if(vdoWidth*vdoHeight > IMG_1080P_SIZE)
            {
                mbTwoRunP2 = MFALSE;
                configInfo.replaceBufNumResize += 2;
                configInfo.rollbackBufNum = 0;
            }
            else
            if(mpParamsMgr->getVideoSnapshotSupport())
            {
                configInfo.rollbackBufNum = 1;
            }
            sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        }
        modeType = DefaultCtrlNode::MODE_VIDEO_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_VIDEO_PRV;
        mUserName = "VDO";
    }
    else
    if(mName == MtkCameraParameters::APP_MODE_NAME_MTK_ZSD)
    {
        sensorScenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        configInfo.rollbackBufNum = get_zsd_cap_stored_frame_cnt();
        configInfo.rollbackBufPeriod = get_zsd_cap_stored_update_period();
        modeType = DefaultCtrlNode::MODE_ZSD_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_ZSD;
        mUserName = "ZSD";
    }
    else
    if(mName == MtkCameraParameters::APP_MODE_NAME_MTK_VT)
    {
        sensorScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
        configInfo.rollbackBufNum = 0;
        modeType = DefaultCtrlNode::MODE_VIDEO_PREVIEW;
        mode.dev = ResMgrDrv::DEV_VT;
        mUserName = "VT";
        int32_t dummyAngle = get_dummy_angle();
        SensorStaticInfo const*const pSensorStaticInfo = pHalSensorList->querySensorStaticInfo(getOpenId());
        MUINT const orientation = pSensorStaticInfo->orientationAngle;
        MUINT const facing = pSensorStaticInfo->facingDirection;
        if((orientation + dummyAngle) >= 360)
        {
             rotationAnagle = orientation - 360;
        }
        else
        {
            rotationAnagle = orientation + dummyAngle;
        }
        MY_LOGD("VT:Id(%d),Ori(%d),Facing(%d),DA(%d)",
                getOpenId(),
                rotationAnagle,
                facing,
                dummyAngle);
        bIsForceRatation = MTRUE;
    }
    else
    {
        if(bVHdr)
        {
            sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
        }
        else
        {
            sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        }
        //
        configInfo.rollbackBufNum = 0;
        modeType = DefaultCtrlNode::MODE_NORMAL_PREVIEW;
        mode.scenSw = ResMgrDrv::SCEN_SW_CAM_PRV;
        //
        if( mName == MtkCameraParameters::APP_MODE_NAME_DEFAULT ||
            mName == MtkCameraParameters::APP_MODE_NAME_MTK_VIDEO)
        {
            mUserName = "VDO";
        }
        else
        {
            mUserName = "PHO";
        }
    }

    { // start of Engineer Mode
        String8 ms8SaveMode = mpParamsMgr->getStr(MtkCameraParameters::KEY_RAW_SAVE_MODE);

        int camera_mode = mpParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        if (camera_mode != 0)
        {
            ms8SaveMode = String8("4"); // force sensor scenario to "normal video" // should remove when em camera app modifed
        }

        const char *strSaveMode = ms8SaveMode.string();

#if 0
/******************************************************************************
 *  Sensor Scenario ID
 ******************************************************************************/
enum
{
    SENSOR_SCENARIO_ID_NORMAL_PREVIEW,
    SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
    SENSOR_SCENARIO_ID_NORMAL_VIDEO,
    SENSOR_SCENARIO_ID_SLIM_VIDEO1,
    SENSOR_SCENARIO_ID_SLIM_VIDEO2,
    /**************************************************************************
     * All unnamed scenario id for a specific sensor must be started with
     * values >= SENSOR_SCENARIO_ID_UNNAMED_START.
     **************************************************************************/
    SENSOR_SCENARIO_ID_UNNAMED_START = 0x100,
};
#endif

        sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;

        switch (strSaveMode[0])
        {
            case '1': // 1: "Preview Mode",
                // sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                break;
            case '2': // 2: "Capture Mode",
                // sensorScenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
                MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                break;
            case '4': // 4: "Video Preview Mode"
                sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_VIDEO");
                break;
            case '5': // 5: "Slim Video 1"
                sensorScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                MY_LOGD("SENSOR_SCENARIO_ID_SLIM_VIDEO1");
                break;
            case '6': // 6: "Slim Video 2"
                sensorScenario = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                MY_LOGD("SENSOR_SCENARIO_ID_SLIM_VIDEO2");
                break;
            case '0': // 0: do not save
            case '3': // 3: "JPEG Only"
            default:
                sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                MY_LOGD("SENSOR_SCENARIO_ID_NORMAL_PREVIEW");
                break;
        }

    } // end of Engineer Mode

    { // for sensor porting debug
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.force_sensormode", value, "-1");
        MINT32 val = atoi(value);
        if( val > 0 ) {
            sensorScenario = val;
            MY_LOGD("force use sensor mode %d", sensorScenario);
        }
    }
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.pass1.rawtype", value, "-1");
        MINT32 val = atoi(value);
        if( val > -1 ) {
            rawType = val;
            bConnectFullRaw = MTRUE;
            MY_LOGD("debug.pass1.rawtype %d", rawType);
        }
    }
    //
    if( pSensorHalObj != NULL )
    {
        pSensorHalObj->sendCommand( pHalSensorList->querySensorDevIdx(getOpenId()),
                            SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                            (MINTPTR)&sensorScenario, (MINTPTR)&sensorFps, 0);
        MY_LOGE_IF(0 == sensorFps, "sensorFps in scenario(%d) is 0", sensorScenario);
        sensorFps /= 10; //NOTE: sensor default output frame rate is 10 base
        mPreviewMaxFps = (mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= sensorFps )
                    ? mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE)
                    : sensorFps;
    }
    else
    {
        MY_LOGE("pSensorHalObj is NULL");
    }
    //
    mpPass2Node = Pass2Node::createInstance(PASS2_FEATURE);
    if(mpPass2Node)
    {
        if( bSlowMotion ||
            sensorScenario == SENSOR_SCENARIO_ID_SLIM_VIDEO1)
        {
            mpPass2Node->enableFeature(MFALSE);
        }
        else
        {
            mpPass2Node->enableFeature(MTRUE);
            configInfo.replaceBufNumResize += 3;
        }
        //
        mpPass2Node->setCamMsgCb(mpCamMsgCbInfo);
        mpPass2Node->setParameters(
                        mpParamsMgr,
                        getOpenId());
        mpPass2Node->startPreview(
                        prvWidth,
                        prvHeight,
                        mPreviewMaxFps);
    }
    //
    if(mPreviewMaxFps > 30)
    {
        pass1RingBufNum += ((mPreviewMaxFps/30)*2+1);
        if(pass1RingBufNum > 15)
        {
            pass1RingBufNum = 15;
            configInfo.replaceBufNumResize += (15-pass1RingBufNum);
        }
    }
    //
    MY_LOGD("isMultiDevice(%d),VHdr(%d),Name(%s),userName(%s),sensorScenario(%d),pass1RingBufNum(%d),sensorFps(%d),APFps(%d),frameRate(%d)",
            CamManager::getInstance()->isMultiDevice(),
            bVHdr,
            mName.string(),
            mUserName.string(),
            sensorScenario,
            pass1RingBufNum,
            sensorFps,
            mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE),
            mPreviewMaxFps);
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
                        mPreviewMaxFps,
                        rawType);
    //
    if  ( ! BaseCamAdapter::init() )
    {
        goto lbExit;
    }
    //
    mpCamGraph          = ICamGraph::createInstance(
                                        getOpenId(),
                                        mUserName.string());
    //
    mpPass1Node         = Pass1Node::createInstance(p1NodeInitCfg);
    //
    mpRecBufSnapshotlNode = RecBufSnapshotlNode::createInstance(mpParamsMgr);
    //
    if(mpCapBufMgr)
    {
       mpCapBufMgr->setMaxDequeNum(configInfo.rollbackBufNum);
    }
    //
    if(mpEngCtrlNode)
    {
        configInfo.spParamsMgr = mpParamsMgr;
        configInfo.spCamMsgCbInfo = mpCamMsgCbInfo;
        configInfo.pCapBufMgr = mpCapBufMgr;
        configInfo.replaceBufNumFullsize = configInfo.rollbackBufNum;
        configInfo.previewMaxFps = mPreviewMaxFps;
        mpEngCtrlNode->setConfig(configInfo);
        mpEngCtrlNode->setParameters();
        mpEngCtrlNode->setMode(modeType);
    }
    //
    if(bIsForceRatation)
    {
        mpEngBufHdl->setForceRotation(
                            MTRUE,
                            rotationAnagle);
        mpEngCtrlNode->setForceRotation(
                            MTRUE,
                            rotationAnagle);
    }
    //
    if( (   (vdoWidth*vdoHeight) <= IMG_1080P_SIZE &&
            bVHdr == false &&
            mpParamsMgr->getVideoSnapshotSupport() == true &&
            modeType != DefaultCtrlNode::MODE_NORMAL_PREVIEW &&
            mName != MtkCameraParameters::APP_MODE_NAME_MTK_VT &&
            bSlowMotion == MFALSE) ||
        modeType == DefaultCtrlNode::MODE_ZSD_PREVIEW ||
        rawType != 1)
    {
        bConnectFullRaw = MTRUE;
    }
    MY_LOGD("rawType(%d),bConnectFullRaw(%d)",rawType,bConnectFullRaw);
    //
    MY_LOGD("mbTwoRunP2(%d)",mbTwoRunP2);
    if(mbTwoRunP2)
    {
        MUINT32 bufStridesInBytes[] = {0,0,0};
        mpTwoRunP1AllocBufHdl = AllocBufHandler::createInstance();
        mpTwoRunP1AllocBufHdl->init();
        //alloc ringbuffer
        AllocInfo allocinfo(
                    prvWidth,
                    prvHeight,
                    eImgFmt_UYVY,
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK,
                    bufStridesInBytes);
        for(MUINT32 i = 0; i < 3 ; i++ )
        {
            if( !mpTwoRunP1AllocBufHdl->requestBuffer(PASS2_PRV_DST_0, allocinfo) )
            {
                MY_LOGE("request buffer failed: data PASS2_PRV_DST_0");
                ret = UNKNOWN_ERROR;
                goto lbExit;
            }
        }
        //
        mpExtImgProcNode = ExtImgProcNode::createInstance();
        mpImgTransformNode = ImgTransformNode::createInstance();
    }
    //
    MY_LOGD("AllocBufHandler:init");
    mpAllocBufHdl->init();
    //
    MY_LOGD("EngBufHandler:map port");
    if(mbTwoRunP2)
    {
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Disp,      TRANSFORM_DST_0, MTRUE);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_AP,        TRANSFORM_DST_1);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Generic,   TRANSFORM_DST_1);
    }
    else
    {
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Disp,      PASS2_PRV_DST_0, MTRUE);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_AP,        PASS2_PRV_DST_1);
        mpEngBufHdl->mapPort(EngBufHandler::eBuf_Generic,   PASS2_PRV_DST_1);
    }
    mpEngBufHdl->mapPort(EngBufHandler::eBuf_FD,        PASS2_PRV_DST_2);
    mpEngBufHdl->mapPort(EngBufHandler::eBuf_OT,        PASS2_PRV_DST_2);
    //
    MY_LOGD("CamGraph:setBufferHandler");
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl);
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl);
    if(mbTwoRunP2)
    {
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_0,    mpTwoRunP1AllocBufHdl);
        mpCamGraph->setBufferHandler(   TRANSFORM_DST_0,    mpEngBufHdl);
        mpCamGraph->setBufferHandler(   TRANSFORM_DST_1,    mpEngBufHdl);
    }
    else
    {
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_0,    mpEngBufHdl);
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_1,    mpEngBufHdl);
    }
    mpCamGraph->setBufferHandler(   PASS2_PRV_DST_2,    mpEngBufHdl);
    mpCamGraph->setBufferHandler(   PASS2_PRV_DST_3,    mpEngBufHdl);
    //
    MY_LOGD("CamGraph:connectData");
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node,        mpEngCtrlNode);
    if(bConnectFullRaw)
    {
        mpCamGraph->connectData(    PASS1_FULLRAW,      CONTROL_FULLRAW,    mpPass1Node,        mpEngCtrlNode);
    }
    //
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    PASS2_PRV_SRC,      mpEngCtrlNode,  mpPass2Node);
    if(mbTwoRunP2)
    {
        mpCamGraph->connectData(PASS2_PRV_DST_0,    EXTIMGPROC_SRC,     mpPass2Node,        mpExtImgProcNode);
        mpCamGraph->connectData(EXTIMGPROC_DST_0,   TRANSFORM_SRC,      mpExtImgProcNode,   mpImgTransformNode);
        mpCamGraph->connectData(TRANSFORM_DST_1,    RECBUFSS_SRC,       mpImgTransformNode, mpRecBufSnapshotlNode);
    }
    else
    {
        mpCamGraph->connectData(PASS2_PRV_DST_1,    RECBUFSS_SRC,       mpPass2Node,        mpRecBufSnapshotlNode);
    }
    //
    MY_LOGD("CamGraph:connectNotify");
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node,        mpEngCtrlNode);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node,        mpEngCtrlNode);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node,        mpEngCtrlNode);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node,        mpEngCtrlNode);
    mpCamGraph->connectNotify(  CONTROL_STOP_PASS1, mpEngCtrlNode,  mpPass1Node);
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
    if( !mpEngCtrlNode->waitPreviewReady() )
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

    // flash enable
    MUINT32 flashCaliEn = mpParamsMgr->getInt(MtkCameraParameters::KEY_ENG_FLASH_CALIBRATION);
    enableFlashCalibration(flashCaliEn);

    if( !mpEngCtrlNode->precap() )
    {
        MY_LOGE("precap failed");
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
*******************************************************************************/
status_t
CamAdapter::
forceStopAndCleanPreview()
{
    MY_LOGD("+");
    //
    mPreviewMaxFps = 0;
    //
    if ( mpEngCtrlNode != NULL)
    {
        mpEngCtrlNode->setMode(DefaultCtrlNode::MODE_IDLE);
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
    if ( mpPass2Node != NULL )
    {
        mpPass2Node->enableGDPreview(MFALSE);
        mpPass2Node->stopPreview();
        mpPass2Node->destroyInstance();
        mpPass2Node = NULL;
    }
    //
    if ( mpExtImgProcNode != NULL )
    {
        mpExtImgProcNode->destroyInstance();
        mpExtImgProcNode = NULL;
    }
    //
    if ( mpImgTransformNode != NULL )
    {
        mpImgTransformNode->destroyInstance();
        mpImgTransformNode = NULL;
    }
    //
    if( mpRecBufSnapshotlNode != NULL )
    {
        mpRecBufSnapshotlNode->destroyInstance();
        mpRecBufSnapshotlNode = NULL;
    }
    //
    if ( mpAllocBufHdl != NULL )
    {
        mpAllocBufHdl->uninit();
    }
    //
    if ( mpTwoRunP1AllocBufHdl != NULL )
    {
        mpTwoRunP1AllocBufHdl->uninit();
        mpTwoRunP1AllocBufHdl = NULL;
    }
    //
    if ( mpEngBufHdl != NULL )
    {
        mpEngBufHdl->unmapPort(EngBufHandler::eBuf_Disp);
        mpEngBufHdl->unmapPort(EngBufHandler::eBuf_AP);
        mpEngBufHdl->unmapPort(EngBufHandler::eBuf_Generic);
        mpEngBufHdl->unmapPort(EngBufHandler::eBuf_FD);
        mpEngBufHdl->unmapPort(EngBufHandler::eBuf_OT);
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



status_t
/******************************************************************************
*
*******************************************************************************/
CamAdapter::
enableFlashCalibration(int enable)
{
    MY_LOGD("+");
    //
    status_t ret = OK;
    //
    #if '1'==MTKCAM_HAVE_3A_HAL
    //
    IHal3A* pHal3a = IHal3A::createInstance(IHal3A::E_Camera_1, getOpenId(), getName());

    if ( ! pHal3a )
    {
        MY_LOGE("pHal3a == NULL");
        return INVALID_OPERATION;
    }

    if ( ! pHal3a->enableFlashQuickCalibration(enable) )
    {
        MY_LOGE("enableFlashQuickCalibration fail");
        ret = INVALID_OPERATION;
        goto lbExit;
    }
    #endif

lbExit:
    #if '1'==MTKCAM_HAVE_3A_HAL
    pHal3a->destroyInstance(getName());
    #endif
    //
    MY_LOGD("-");
    return ret;
}
