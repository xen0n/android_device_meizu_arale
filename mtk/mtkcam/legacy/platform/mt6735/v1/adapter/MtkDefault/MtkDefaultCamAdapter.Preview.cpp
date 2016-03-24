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
#include <camera_custom_vt.h>

#include <camera/MtkCamera.h>
//
#include <inc/ImgBufProvidersManager.h>
//
#include <mtkcam/v1/IParamsManager.h>
#include <mtkcam/v1/ICamAdapter.h>
#include <inc/BaseCamAdapter.h>
#include <inc/MtkDefaultCamAdapter.h>
using namespace NSMtkDefaultCamAdapter;
//
#include <mtkcam/hal/IHalSensor.h>
using namespace NSCam;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
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
    if(mpCpuCtrl)
    {
        CpuCtrl::CPU_CTRL_INFO cpuInfo;
        cpuInfo.bigCore = 4;
        cpuInfo.bigFreq = 1300000;
        cpuInfo.littleCore = 4;
        cpuInfo.littleFreq = 1300000;
        cpuInfo.timeout = 3;
        mpCpuCtrl->enable(cpuInfo);
    }
    //
    MY_LOGD("+");
    //
    status_t ret = INVALID_OPERATION;
    //
    vector<SensorSlimVideoInfo> vSensorSlimVideo;
    MINT32 sensorFps = 0;
    //
    MBOOL bIsForceRatation = MFALSE, bConnectFullRaw = MFALSE, bConnectFeatrue = MFALSE, bSlowMotion = MFALSE, bPdafMode = MFALSE;
    MINT32 rawType = SENSOR_PROCESS_RAW, vdoWidth = 0, vdoHeight = 0, prvWidth = 0, prvHeight = 0;
    MUINT32 pass1RingBufNum = 3, sensorScenario = 0, rotationAnagle = 0;
    DefaultCtrlNode::MODE_TYPE modeType = DefaultCtrlNode::MODE_IDLE;
    DefaultCtrlNode::CONFIG_INFO configInfo;
    ResMgrDrv::MODE_STRUCT mode;
    //
    mpParamsMgr->getPreviewSize(&prvWidth, &prvHeight);
    mPreviewMaxFps = mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE); // default value
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
    MY_LOGD("VDO:%d,%dx%d = %d, 1080P(%d)",
            mb4K2KVideoRecord,
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
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.tworunpass2.enable", value, "0");
        MINT32 val = atoi(value);
        if( val > 0 )
        {
            mbTwoRunP2 = MTRUE;
            MY_LOGD("mbTwoRunP2(%d)", mbTwoRunP2);
        }
    }
    //
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.lowPowerVR.enable", value, "-1");
        MINT32 val = atoi(value);
        if( val > -1 )
        {
            mbLowPowerVideoRecord = val;
            MY_LOGD("mbLowPowerVideoRecord(%d)", mbLowPowerVideoRecord);
        }
    }
    //
    if(mpParamsMgr->getRecordingHint())
    {
        if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) > 30)
        {
            SensorSlimVideoInfo sensorSlimVideo[2];
            bSlowMotion = MTRUE;
            mbTwoRunP2 = MFALSE;
            configInfo.rollbackBufNum = 0;
            //
            if( pSensorHalObj != NULL )
            {
                sensorSlimVideo[0].scenario = SENSOR_SCENARIO_ID_SLIM_VIDEO1;
                pSensorHalObj->sendCommand(
                                    pHalSensorList->querySensorDevIdx(getOpenId()),
                                    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                    (MUINTPTR)&(sensorSlimVideo[0].scenario),
                                    (MUINTPTR)&(sensorSlimVideo[0].fps),
                                    0);
                sensorSlimVideo[0].fps /= 10; //NOTE: sensor default output frame rate is 10 base
                //
                sensorSlimVideo[1].scenario = SENSOR_SCENARIO_ID_SLIM_VIDEO2;
                pSensorHalObj->sendCommand(
                                    pHalSensorList->querySensorDevIdx(getOpenId()),
                                    SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                    (MUINTPTR)&(sensorSlimVideo[1].scenario),
                                    (MUINTPTR)&(sensorSlimVideo[1].fps),
                                    0);
                sensorSlimVideo[1].fps /= 10; //NOTE: sensor default output frame rate is 10 base
                MY_LOGD("Slim video FPS:%d, %d",
                        sensorSlimVideo[0].fps,
                        sensorSlimVideo[1].fps);
            }
            //
            if( sensorSlimVideo[0].fps > 30 &&
                sensorSlimVideo[1].fps > 30)
            {
                if(sensorSlimVideo[0].fps <= sensorSlimVideo[1].fps)
                {
                    vSensorSlimVideo.push_back(sensorSlimVideo[0]);
                    vSensorSlimVideo.push_back(sensorSlimVideo[1]);
                }
                else
                {
                    vSensorSlimVideo.push_back(sensorSlimVideo[1]);
                    vSensorSlimVideo.push_back(sensorSlimVideo[0]);
                }
            }
            else
            if(sensorSlimVideo[0].fps > 30)
            {
                vSensorSlimVideo.push_back(sensorSlimVideo[0]);
            }
            else
            if(sensorSlimVideo[0].fps > 30)
            {
                vSensorSlimVideo.push_back(sensorSlimVideo[1]);
            }
            else
            {
                MY_LOGE("No sensor scenario FPS > 30");
            }
            //
            sensorScenario = 0;
            for(MINT32 i=0; i<vSensorSlimVideo.size(); i++)
            {
                if(mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE) <= vSensorSlimVideo[i].fps)
                {
                    sensorScenario = sensorSlimVideo[i].scenario;
                    MY_LOGD("Use sensor scenario(%d) FPS(%d)",
                            sensorSlimVideo[i].scenario,
                            sensorSlimVideo[i].fps);
                    break;
                }
            }
            //
            if(sensorScenario == 0)
            {
                MY_LOGE("No sensor scenario FPS >= %d",
                        mpParamsMgr->getInt(CameraParameters::KEY_PREVIEW_FRAME_RATE));
                sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            }
        }
        else
        {
            if(mpParamsMgr->getVideoSnapshotSupport())
            {
                if(mb4K2KVideoRecord == MTRUE)
                {
                    mbTwoRunP2 = MFALSE;
                    configInfo.replaceBufNumResize += 2;
                    configInfo.rollbackBufNum = 0;
                }
                else
                {
                    if( ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) == 0 ||
                        ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_3DNR_MODE), MtkCameraParameters::ON) == 0)
                    {
                        configInfo.rollbackBufNum = 0;
                    }
                    else
                    if(mpParamsMgr->getVHdr() != SENSOR_VHDR_MODE_NONE)
                    {
                        configInfo.rollbackBufNum = 0;
                    }
                    else
                    {
                        if(mbLowPowerVideoRecord)
                        {
                            configInfo.rollbackBufNum = 0;
                        }
                        else
                        {
                            configInfo.rollbackBufNum = 1;
                        }
                    }
                }
            }
            //
            if( mbLowPowerVideoRecord &&
                mb4K2KVideoRecord == MFALSE &&
                mpParamsMgr->getVHdr() != SENSOR_VHDR_MODE_IVHDR)
            {
                sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;

                MUINT32 sensorVideMode = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                MBOOL isSupportPDAF = MFALSE;
                // if in low power mode, check if sensor support video mode,
                // if yes, change sensorScenario to video mode
                if( pHalSensorList != NULL )
                {
                    pSensorHalObj->sendCommand(
                                pHalSensorList->querySensorDevIdx(getOpenId()),
                                SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
                                (MUINTPTR)&(sensorVideMode),
                                (MUINTPTR)&isSupportPDAF,
                                0);
                }
                if (isSupportPDAF)
                {
                    sensorScenario = sensorVideMode;
                }
            }
            else
            {
                if(mpParamsMgr->getVHdr() == SENSOR_VHDR_MODE_MVHDR)
                {
                    sensorScenario = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
                }
                else
                {
                    sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
                }
            }
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
        if( pSensorHalObj != NULL )
        {
            pSensorHalObj->sendCommand(
                                pHalSensorList->querySensorDevIdx(getOpenId()),
                                SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                (MINTPTR)&sensorScenario,
                                (MINTPTR)&sensorFps,
                                0);
            sensorFps /= 10; //NOTE: sensor default output frame rate is 10 base
            if(sensorFps > 30)
            {
                MY_LOGW("Use NORMAL_VIDEO for VT because SLIM_VIDEO2 FPS(%d) > 30",sensorFps);
                sensorScenario = SENSOR_SCENARIO_ID_NORMAL_VIDEO;
            }
        }
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
        if(mpParamsMgr->getVHdr() == SENSOR_VHDR_MODE_IVHDR)
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
    //
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
    setParameters();
    //
    mpPass2Node = Pass2Node::createInstance(PASS2_FEATURE);
    if(mpPass2Node)
    {
        if( bSlowMotion ||
            mPreviewMaxFps > 30)
        {
            mpPass2Node->enableFeature(MFALSE);
        }
        else
        {
            mpPass2Node->enableFeature(MTRUE);
            configInfo.replaceBufNumResize += 5;
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
            configInfo.replaceBufNumResize += (pass1RingBufNum -15);
            pass1RingBufNum = 15;
        }
    }
    //
    MY_LOGD("isMultiDevice(%d),hdrState(%d),Name(%s),userName(%s),sensorScenario(%d),pass1RingBufNum(%d),sensorFps(%d),APFps(%d),frameRate(%d)",
            CamManager::getInstance()->isMultiDevice(),
            mpParamsMgr->getVHdr(),
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
    // Get if sensor support PDAF
    if( pHalSensorList != NULL )
    {
        //this should be the correct method to get if sensor support PDAF
        pSensorHalObj->sendCommand(
                    pHalSensorList->querySensorDevIdx(getOpenId()),
                    SENSOR_CMD_GET_SENSOR_PDAF_CAPACITY,
                    (MUINTPTR)&(sensorScenario),
                    (MUINTPTR)&bPdafMode,
                    0);
        if (mb4K2KVideoRecord)
        {
            bPdafMode = MFALSE;
        }
        // force to turn on/off PDAF
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get( "debug.pass1.pdafon", value, "-1");
        MINT32 val = atoi(value);
        if( val > -1 ) {
            bPdafMode = val;
            MY_LOGD("debug.pass1.pdafon %d", bPdafMode);
        }

        MY_LOGD("bPdafMode=%d", bPdafMode);
    }
    else
    {
        MY_LOGE("pHalSensorList is NULL");
    }
    //
    if (bPdafMode)
    {
        rawType = SENSOR_DYNAMIC_PURE_RAW;
        // create PDAF node if support PDAF
        mpPdafNode = PdafNode::createInstance(getOpenId());
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
    if(mpCapBufMgr)
    {
       mpCapBufMgr->setMaxDequeNum(configInfo.rollbackBufNum);
    }
    //
    if(mpDefaultCtrlNode)
    {
        configInfo.spParamsMgr = mpParamsMgr;
        configInfo.spCamMsgCbInfo = mpCamMsgCbInfo;
        configInfo.pCapBufMgr = mpCapBufMgr;
        if(configInfo.rollbackBufNum != 0)
        {
            configInfo.replaceBufNumFullsize = configInfo.rollbackBufNum+1;
        }
        else
        {
            configInfo.replaceBufNumFullsize = 0;
        }
        configInfo.previewMaxFps = mPreviewMaxFps;
        mpDefaultCtrlNode->setConfig(configInfo);
        mpDefaultCtrlNode->setParameters();
        mpDefaultCtrlNode->setMode(modeType);
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
    }
    //
    if( (   mb4K2KVideoRecord == MFALSE &&
            mpParamsMgr->getVHdr() == SENSOR_VHDR_MODE_NONE &&
            mpParamsMgr->getVideoSnapshotSupport() == true &&
            modeType != DefaultCtrlNode::MODE_NORMAL_PREVIEW &&
            mName != MtkCameraParameters::APP_MODE_NAME_MTK_VT &&
            bSlowMotion == MFALSE &&
            mbLowPowerVideoRecord == MFALSE &&
            ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_FACE_BEAUTY), MtkCameraParameters::TRUE) != 0 &&
            ::strcmp(mpParamsMgr->getStr(MtkCameraParameters::KEY_3DNR_MODE), MtkCameraParameters::ON) != 0) ||
            modeType == DefaultCtrlNode::MODE_ZSD_PREVIEW ||
            rawType != SENSOR_PROCESS_RAW||
            bPdafMode == 1) // if PDAF is on, connect full raw (except 4K2K mode)
    {
        bConnectFullRaw = MTRUE;
    }
    if (mb4K2KVideoRecord)
    {
        bConnectFullRaw = MFALSE;
    }
    MY_LOGD("rawType(%d),bConnectFullRaw(%d)",rawType,bConnectFullRaw);
    //
    if( mpParamsMgr->getVideoSnapshotSupport() &&
        configInfo.rollbackBufNum == 0)
    {
        mpRecBufSnapshotlNode = RecBufSnapshotlNode::createInstance(mpParamsMgr);
    }
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
    MY_LOGD("DefaultBufHandler:map port");
    if(mbTwoRunP2)
    {
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp,      TRANSFORM_DST_0, 0, MTRUE);
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_AP,        TRANSFORM_DST_1);
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Generic,   TRANSFORM_DST_1);
    }
    else
    {
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Disp,      PASS2_PRV_DST_0, 0, MTRUE);
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_AP,        PASS2_PRV_DST_1);
        mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_Generic,   PASS2_PRV_DST_1);
    }
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_FD,    PASS2_PRV_DST_2);
    mpDefaultBufHdl->mapPort(DefaultBufHandler::eBuf_OT,    PASS2_PRV_DST_2);
    //
    MY_LOGD("CamGraph:setBufferHandler");
    mpCamGraph->setBufferHandler(   PASS1_RESIZEDRAW,   mpAllocBufHdl);
    mpCamGraph->setBufferHandler(   PASS1_FULLRAW,      mpAllocBufHdl);
    if(mbTwoRunP2)
    {
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_0,    mpTwoRunP1AllocBufHdl);
        mpCamGraph->setBufferHandler(   TRANSFORM_DST_0,    mpDefaultBufHdl);
        mpCamGraph->setBufferHandler(   TRANSFORM_DST_1,    mpDefaultBufHdl);
    }
    else
    {
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_0,    mpDefaultBufHdl);
        mpCamGraph->setBufferHandler(   PASS2_PRV_DST_1,    mpDefaultBufHdl);
    }
    mpCamGraph->setBufferHandler(   PASS2_PRV_DST_2,    mpDefaultBufHdl);
    mpCamGraph->setBufferHandler(   PASS2_PRV_DST_3,    mpDefaultBufHdl);
    //
    MY_LOGD("CamGraph:connectData");
    mpCamGraph->connectData(    PASS1_RESIZEDRAW,   CONTROL_RESIZEDRAW, mpPass1Node,        mpDefaultCtrlNode);
    if(bConnectFullRaw)
    {
        mpCamGraph->connectData(    PASS1_FULLRAW,  CONTROL_FULLRAW,    mpPass1Node,        mpDefaultCtrlNode);
        if (bPdafMode)
        {
            mpCamGraph->connectData (    PASS1_PURERAW,  CONTROL_PURERAW,    mpPass1Node,        mpDefaultCtrlNode);
            mpCamGraph->connectData(    CONTROL_PURERAW_SRC,    PDAF_SRC,      mpDefaultCtrlNode,  mpPdafNode);
        }
    }
    //
    mpCamGraph->connectData(    CONTROL_PRV_SRC,    PASS2_PRV_SRC,      mpDefaultCtrlNode,  mpPass2Node);

    if(mbTwoRunP2)
    {
        mpCamGraph->connectData(PASS2_PRV_DST_0,    EXTIMGPROC_SRC,     mpPass2Node,        mpExtImgProcNode);
        mpCamGraph->connectData(EXTIMGPROC_DST_0,   TRANSFORM_SRC,      mpExtImgProcNode,   mpImgTransformNode);
        //
        if(mpRecBufSnapshotlNode != NULL)
        {
            mpCamGraph->connectData(TRANSFORM_DST_1,    RECBUFSS_SRC,   mpImgTransformNode, mpRecBufSnapshotlNode);
        }
    }
    else
    {
        if(mpRecBufSnapshotlNode != NULL)
        {
            mpCamGraph->connectData(PASS2_PRV_DST_1,    RECBUFSS_SRC,   mpPass2Node,        mpRecBufSnapshotlNode);
        }
    }
    //
    MY_LOGD("CamGraph:connectNotify");
    mpCamGraph->connectNotify(  PASS1_START_ISP,    mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_STOP_ISP,     mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_EOF,          mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  PASS1_CONFIG_FRAME, mpPass1Node,        mpDefaultCtrlNode);
    mpCamGraph->connectNotify(  CONTROL_STOP_PASS1, mpDefaultCtrlNode,  mpPass1Node);
    //
    if ( !mpCamGraph->init() ) {
        MY_LOGE("CamGraph:init");
        goto lbExit;
    }
    //
    mPipStartPreviewFail = MFALSE;
    if ( !mpCamGraph->start() ) {
        // use CAMERA_ERROR_SERVER_DIED callback to handle startPreview error
        mpCamMsgCbInfo->mNotifyCb(
                CAMERA_MSG_ERROR,
                CAMERA_ERROR_SERVER_DIED, 0,
                mpCamMsgCbInfo->mCbCookie
                );
        mPipStartPreviewFail = MTRUE;
        // do not return fail to avoid java exception
        mpStateManager->transitState(IState::eState_Preview);
        ret = OK;
        //
        MY_LOGE("CamGraph:start");
        goto lbExit;
    }
    //
    if( !mpDefaultCtrlNode->waitPreviewReady() )
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

    if( !mpDefaultCtrlNode->precap() )
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
    if ( mpDefaultCtrlNode != NULL)
    {
        mpDefaultCtrlNode->setMode(DefaultCtrlNode::MODE_IDLE);
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
    if ( mpPdafNode != NULL )
    {
        mpPdafNode->destroyInstance();
        mpPdafNode = NULL;
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


