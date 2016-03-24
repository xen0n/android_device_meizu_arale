/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/DCNode.Eng"
//
#include <cutils/properties.h>      // [debug] should be remove
//
#include <CamUtils.h>
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <mtkcam/hwutils/CamManager.h>
using namespace NSCam::Utils;
//
#include <vector>
//
#include <stdio.h>
#include <stdlib.h>
#include <mtkcam/featureio/aaa_hal_common.h>
//
using namespace std;
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <DefaultCtrlNodeImpl.h>
using namespace NSCamNode;
//

#define MAX_VIDEO_DUMPCNT  200
static uint32_t g_u4RawDumpCnt = 0;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeEngImpl::
DefaultCtrlNodeEngImpl(const char* userName)
    : DefaultCtrlNodeImpl(userName),mPreCapFlashInfo_flag(0),mu4RawDumpMgcNum(0xffff),mu43aInfoFileHead(0)
{
    FUNC_NAME;
}


/*******************************************************************************
*
********************************************************************************/
DefaultCtrlNodeEngImpl::
~DefaultCtrlNodeEngImpl()
{
    FUNC_NAME;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onInit()
{
    FUNC_START;
    //
    mpHal3a = IHal3A::createInstance(
                        IHal3A::E_Camera_1,
                        getSensorIdx(),
                        LOG_TAG);
    if(mpHal3a == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        return MFALSE;
    }
    //
    mpHal3a->addCallbacks(this);
    //
    updateDelay(EQueryType_Init);
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if(mpIspSyncCtrl)
    {
        MINT32 i;
        vector<MUINT32> zoomRatios;
        for(i=0; i<=mspParamsMgr->getInt(CameraParameters::KEY_MAX_ZOOM); i++)
        {
            zoomRatios.push_back(mspParamsMgr->getZoomRatioByIndex(i));
        }
        mpIspSyncCtrl->setZoomRatioList(zoomRatios);
        //
        if(CamManager::getInstance()->isMultiDevice())
        {
            mpIspSyncCtrl->setHdrState(SENSOR_VHDR_MODE_NONE);
        }
        else
        {
            mpIspSyncCtrl->setHdrState(mspParamsMgr->getVHdr());
        }
        mpIspSyncCtrl->addPass1ReplaceBuffer(mReplaceBufNumFullsize, mReplaceBufNumResize);
        mpIspSyncCtrl->setPreviewSize(mPreviewSize.w, mPreviewSize.h);
        if (mspParamsMgr != NULL)
        {
            mpIspSyncCtrl->setCurZoomRatio(mspParamsMgr->getZoomRatioByIndex(mspParamsMgr->getInt(CameraParameters::KEY_ZOOM)));
        }
        //
        MUINT32 sensorW, sensorH;
        mpIspSyncCtrl->getSensorInfo(
                            mSensorScenario,
                            sensorW,
                            sensorH,
                            mSensorType);
        mSensorSize.w = sensorW;
        mSensorSize.h = sensorH;
        //
        MSize videoSize, rrzoMinSize, initRrzoSize;
        //
        if( mSensorSize.w*mSensorSize.h >= IMG_2160P_SIZE &&
            mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE)
        {
            videoSize.w = IMG_2160P_W;
            videoSize.h = IMG_2160P_H;
        }
        else
        {
            videoSize.w = IMG_1080P_W;
            videoSize.h = IMG_1080P_H;
        }
        //
        rrzoMinSize.w = EIS_MIN_WIDTH;
        rrzoMinSize.h = (MUINT32)((EIS_MIN_WIDTH*mPreviewSize.h)/mPreviewSize.w);
        //
        MSize maxSize, sizeList[3] = {mPreviewSize, videoSize, rrzoMinSize};
        //4K2K video record does not support full size output.
        MY_LOGD("RH(%d)",mRecordingHint);
        if( mRecordingHint == MTRUE &&
            mMode == MODE_VIDEO_PREVIEW)
        {
            maxSize.w = 0;
            maxSize.h = 0;
            for(i=0; i<3; i++)
            {
                MY_LOGD("sizeList[%d]=%dx%d",
                        i,
                        sizeList[i].w,
                        sizeList[i].h);
                if(maxSize.w < sizeList[i].w)
                {
                   maxSize.w = sizeList[i].w;
                }
                if(maxSize.h < sizeList[i].h)
                {
                   maxSize.h = sizeList[i].h;
                }
            }
            //
            MY_LOGD("maxSize(%dx%d)",
                    maxSize.w,
                    maxSize.h);
            initRrzoSize.w = (maxSize.w*EIS_FACTOR)/100;
            initRrzoSize.h = (maxSize.h*EIS_FACTOR)/100;
            MY_LOGD("initRrzoSize(%dx%d)",
                    initRrzoSize.w,
                    initRrzoSize.h);
            //
            MUINT32 aaaUpdatePeriod;
            if(mPreviewMaxFps < AAA_UPDATE_BASE)
            {
                aaaUpdatePeriod = 1;
            }
            else
            {
                aaaUpdatePeriod = mPreviewMaxFps/AAA_UPDATE_BASE;
            }
            if(mspParamsMgr->getVideoStabilization())
            {
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))
                {
                    mpIspSyncCtrl->setVideoSize(3840, 2160);
                }
                else
                {
                    mpIspSyncCtrl->setVideoSize(
                                        (MUINT32)((mPreviewSize.w*EIS_FACTOR)/100),
                                        (MUINT32)((mPreviewSize.h*EIS_FACTOR)/100));
                }
            }
            else
            {
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))
                {
                    mpIspSyncCtrl->setVideoSize(3840, 2160);
                }
                else
                {
                    mpIspSyncCtrl->setVideoSize(0, 0);
                }
            }
            //
            mpIspSyncCtrl->set3AUpdatePeriod(aaaUpdatePeriod);
            mpIspSyncCtrl->setRrzoMinSize(
                                rrzoMinSize.w,
                                rrzoMinSize.h);
        }
        else
        {
            initRrzoSize.w = mPreviewSize.w;
            initRrzoSize.h = mPreviewSize.h;
            //
            mpIspSyncCtrl->setVideoSize(0, 0);
            mpIspSyncCtrl->set3AUpdatePeriod(1);
            mpIspSyncCtrl->setRrzoMinSize(0, 0);
        }
        //
        if( initRrzoSize.w > mSensorSize.w)
        {
            initRrzoSize.w = mSensorSize.w;
        }
        if( initRrzoSize.h > mSensorSize.h)
        {
            initRrzoSize.h = mSensorSize.h;
        }
        MY_LOGD("initRrzoSize(%dx%d)",
                initRrzoSize.w,
                initRrzoSize.h);
        //
        mpIspSyncCtrl->setPass1InitRrzoSize(
                            initRrzoSize.w,
                            initRrzoSize.h);
        mpIspSyncCtrl->calRrzoMaxZoomRatio();
    }
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpIspSyncCtrl->getHdrState() != SENSOR_VHDR_MODE_NONE)
    {
        mpVHdrHal = VHdrHal::CreateInstance(LOG_TAG,getSensorIdx());
        if(mpVHdrHal)
        {
            mpVHdrHal->Init(mspParamsMgr->getVHdr());
        }
        else
        {
            MY_LOGE("mpVHdrHal is NULL");
        }
    }
    #endif
    //
#if 0    // this chip not support EIS2.0
    if(mVideoSize.w*mVideoSize.h > IMG_1080P_SIZE)
    {
        mEisScenario = EIS_SCE_EIS;
    }
    else
    {
        mEisScenario = EIS_SCE_EIS_PLUS;
    }
#else
    mEisScenario = EIS_SCE_EIS;
#endif
    //
    mRollbackBufCnt = 0;
    //
    { // video raw dump
        int camera_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
        int tg_out_mode = 0; // NORNAL
        MY_LOGD("camera mode = %d", camera_mode);
        if (camera_mode == 1)
        {
            tg_out_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_PREVIEW_DUMP_RESOLUTION);
            MY_LOGD("tg_out_mode = %d", tg_out_mode);
        }

        //3ADB_collection for flash precapture
        int en3AFlashRawDump=mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_3ADB_FLASH_ENABLE);

        if (camera_mode != 0 || en3AFlashRawDump == 1 )//3ADB_collection for flash precapture: en3AFlashRawDump == 1
        {

            MY_LOGD("create RawDumpThread instance");

            status_t status = NO_ERROR;

            NSCam::IHalSensorList *pSensorHalList = NULL;
            NSCam::SensorStaticInfo sensorStaticInfo;

            pSensorHalList = NSCam::IHalSensorList::get();
            MUINT32 sensorDev = pSensorHalList->querySensorDevIdx(getSensorIdx());
            pSensorHalList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            MUINT32 rawSensorBit = 0;
            switch (sensorStaticInfo.rawSensorBit)
            {
                case RAW_SENSOR_8BIT:
                    rawSensorBit = 8;
                    break;
                case RAW_SENSOR_10BIT:
                    rawSensorBit = 10;
                    break;
                case RAW_SENSOR_12BIT:
                    rawSensorBit = 12;
                    break;
                case RAW_SENSOR_14BIT:
                    rawSensorBit = 14;
                    break;
            }
            //3Ainfofileheaderdata
            mu4Order=sensorStaticInfo.sensorFormatOrder;
            mu4Bit=rawSensorBit;
            FeatureParam_T r3ASupportedParam;
            mpHal3a->getSupportedParams(r3ASupportedParam);
            mu4IsoGain=r3ASupportedParam.u4MiniISOGain;
            //
            mpRawDumpCmdQueThread = IRawDumpCmdQueThread::createInstance(sensorStaticInfo.sensorFormatOrder, rawSensorBit, mspParamsMgr);
            if  ( mpRawDumpCmdQueThread  == 0 || OK != (status = mpRawDumpCmdQueThread->run()) )
            {
                MY_LOGE(
                    "Fail to run mpRawDumpCmdQueThread  - mpRawDumpCmdQueThread .get(%p), status[%s(%d)]",
                    mpRawDumpCmdQueThread.get(), ::strerror(-status), -status
                );
                g_u4RawDumpCnt = 0;
                // goto lbExit;
            }

            MY_LOGD("mpRawDumpCmdQueThread::setCallbacks is called");
            mpRawDumpCmdQueThread->setCallbacks(mspCamMsgCbInfo);
        }
    }
    char rRawCntValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("eng.video.dumpcnt", rRawCntValue, "0");
    mu4MaxRawDumpCnt = (atoi(rRawCntValue) == 0) ? MAX_VIDEO_DUMPCNT : atoi(rRawCntValue);
    MY_LOGD("mu4MaxRawDumpCnt:%d", mu4MaxRawDumpCnt);

    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onUninit()
{
    FUNC_START;
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    #if EIS_TEST
    if(mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(LOG_TAG);
        mpEisHal = NULL;
    }
    #endif
    //
    #ifdef MTK_CAM_VHDR_SUPPORT
    if(mpVHdrHal)
    {
        mpVHdrHal->Uninit();
        mpVHdrHal->DestroyInstance(LOG_TAG);
        mpVHdrHal = NULL;
    }
    #endif
    //
    if(mpHal3a)
    {
        mpHal3a->removeCallbacks(this);
        mpHal3a->destroyInstance(LOG_TAG);
        mpHal3a = NULL;
    }
    //
    if(mpCallbackZoom)
    {
        mpCallbackZoom->destroyUser();
        mpCallbackZoom->destroyInstance();
        mpCallbackZoom = NULL;
    }
    // start of Video Raw Dump
    int camera_mode = mspParamsMgr->getInt(MtkCameraParameters::KEY_CAMERA_MODE);
    //
    int en3AFlashRawDump=mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_3ADB_FLASH_ENABLE);//

    if (camera_mode != 0 || en3AFlashRawDump == 1)//
    {
        sp<IRawDumpCmdQueThread> pRawDumpCmdQueThread = mpRawDumpCmdQueThread;
        if ( pRawDumpCmdQueThread  != 0 ) {
            MY_LOGD(
                "RawDumpCmdQ Thread: (tid, getStrongCount)=(%d, %d)",
                pRawDumpCmdQueThread->getTid(), pRawDumpCmdQueThread->getStrongCount()
            );
            pRawDumpCmdQueThread->requestExit();
        }
    }
    // end of Video Raw Dump
    FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void EngUtil(MINT32, IHal3A*, sp<IParamsManager>);

MBOOL
DefaultCtrlNodeEngImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    //FUNC_START;
    //
    MBOOL ret = MTRUE;
    //
    if(msg != PASS1_CONFIG_FRAME)
    {
        MY_LOGD("(0x%08X,0x%08X,0x%08X)",
                msg,
                ext1,
                ext2);
    }
    //
    switch(msg)
    {
        case PASS1_START_ISP:
        {
            if(mpHal3a)
            {
                ECmd_T cmd;
                E3APreviewMode_T pvMode;
                //
                mCurMagicNum = ext1;
                if( mMode == MODE_NORMAL_PREVIEW ||
                    mMode == MODE_ZSD_PREVIEW)
                {
                    pvMode = EPv_Normal;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Preview;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Preview;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Preview;
                    }
                }
                else
                if( mMode == MODE_VIDEO_PREVIEW ||
                    mMode == MODE_VIDEO_RECORD)
                {
                    pvMode = EPv_Video;
                    cmd = ECmd_CameraPreviewStart;
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_IVHDR)
                    {
                        mIspProfile = EIspProfile_IHDR_Video;
                    }
                    else
                    if(mpIspSyncCtrl->getHdrState() == SENSOR_VHDR_MODE_MVHDR)
                    {
                        mIspProfile = EIspProfile_MHDR_Video;
                    }
                    else
                    {
                        mIspProfile = EIspProfile_Video;
                    }
                }
                else
                {
                    MY_LOGE("un-support mMode(%d)",mMode);
                }
                mpHal3a->setSensorMode(mSensorScenario);
                //mpHal3a->set3APreviewMode(pvMode);
                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    VHDR_HAL_CONFIG_DATA vhdrConfigData;
                    vhdrConfigData.cameraVer = VHDR_CAMERA_VER_1;
                    mpVHdrHal->ConfigVHdr(vhdrConfigData);
                }
                #endif
                //
                mpHal3a->sendCommand(cmd);
                ParamIspProfile_T _3A_profile(
                        mIspProfile,
                        mCurMagicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3a->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            //
            #if EIS_TEST
            MBOOL bCreateEis = MFALSE;
            if( mPreviewMaxFps <= EIS_MAX_FPS &&
                (   mMode == MODE_VIDEO_PREVIEW ||
                    mMode == MODE_VIDEO_RECORD ||
                    mMode == MODE_NORMAL_PREVIEW ||
                    mMode == MODE_ZSD_PREVIEW))
            {
                bCreateEis = MTRUE;
            }
            //
            if(bCreateEis)
            {
                mpEisHal = EisHal::CreateInstance(LOG_TAG,getSensorIdx());
                if(mpEisHal)
                {
                    mpEisHal->Init();
                    if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                    {
                        EIS_PASS_ENUM eisPass = EIS_PASS_1;
                        EIS_HAL_CONFIG_DATA  eisConfig;
                        MINT32 eisErr;
                        eisConfig.sensorType    = mSensorType;
                        eisConfig.configSce     = mEisScenario;
                        eisConfig.memAlignment  = 2;
                        eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                        if(eisErr != EIS_RETURN_NO_ERROR)
                        {
                            MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                        }
                    }
                }
                else
                {
                    MY_LOGE("mpEisHal is NULL");
                }
            }
            #endif
            break;
        }
        case PASS1_STOP_ISP:
        {
            #ifdef MTK_CAM_VHDR_SUPPORT
            if(mpVHdrHal)
            {
                mpVHdrHal->SendCommand(VHDR_CMD_SET_STATE,VHDR_STATE_UNINIT);
            }
            #endif
            //
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_SET_STATE,EIS_SW_STATE_UNINIT);
            }
            //
            if(mpHal3a)
            {
                mpHal3a->sendCommand(ECmd_CameraPreviewEnd);
            }
            else
            {
                MY_LOGW("mpHal3a is NULL");
            }
            //
            if(mpEisHal)
            {
                mpEisHal->SendCommand(EIS_CMD_CHECK_STATE,EIS_SW_STATE_UNINIT_CHECK);
            }
            break;
        }
        case PASS1_EOF:
        {
            {
                Mutex::Autolock _l(mLock);
                //
                if( mpHal3a &&
                    getFlag(muStateFlag, FLAG_DO_3A_UPDATE) && // to avoid send update after precaptrue-end
                    ext1 != MAGIC_NUM_INVALID )
                {
                    MUINT32 zoomRatio = 0, cropX = 0, cropY = 0, cropW = 0, cropH = 0;
                    if(mpIspSyncCtrl->getCurPass2ZoomInfo(zoomRatio,cropX,cropY,cropW,cropH))
                    {
                        MY_LOGD("setZoom:ZR(%d),Crop(%d,%d,%dx%d)",zoomRatio,cropX,cropY,cropW,cropH);
                        mpHal3a->setZoom(
                                    zoomRatio,
                                    cropX,
                                    cropY,
                                    cropW,
                                    cropH);
                    }
                    //do 3A update
                    mCurMagicNum = ext1;
                    ParamIspProfile_T _3A_profile(
                            mIspProfile,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3a->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
                }
                else
                {
                    MY_LOGD("skip update");
                }
            }
            //
            { // for engineer mode only // less than 1 ms in normal mode
                String8 const s8AppMode = PARAMSMANAGER_MAP_INST(eMapAppMode)->stringFor(mspParamsMgr->getHalAppMode());
                if (s8AppMode == MtkCameraParameters::APP_MODE_NAME_MTK_ENG)
                {
                    EngUtil(getSensorIdx(), mpHal3a, mspParamsMgr);
                }
            }
            //preCapture save raw and flash info //EM mode
            {
                MY_LOGD("preCapture save raw and flash info: mPreCapFlashInfo_flag=%d, mu4RawDumpMgcNum: %d, mCurMagicNum : %d",mPreCapFlashInfo_flag, mu4RawDumpMgcNum, mCurMagicNum);
                if(mu4RawDumpMgcNum <= mCurMagicNum)//magic# in flash info structure
                {
                   mPreCapFlashInfo_flag = 1; // save this buffer of the magic#
                   MY_LOGD("save flag ( mPreCapFlashInfo_flag ) =%d",mPreCapFlashInfo_flag);
                }
                else
                {
                   //mPreCapFlashInfo_flag = 0;
                   MY_LOGD("dont save flag ( mPreCapFlashInfo_flag ) =%d",mPreCapFlashInfo_flag);
                }
            }
            //
            if(updateReadyBuffer(ext2))
            {
                mbUpdateEis = MTRUE;
                mbUpdateVHdr = MTRUE;
            }
            else
            {
                mbUpdateVHdr = MFALSE;

                #ifdef MTK_CAM_VHDR_SUPPORT
                if(mpVHdrHal)
                {
                    //set timestamp = 0 for VHDR driver.
                    mpVHdrHal->DoVHdr(0);
                }
                #endif

                mbUpdateEis = MFALSE;
                if(mpEisHal)
                {
                    //set timestamp = 0 for EIS driver.
                    mpEisHal->DoEis(EIS_PASS_1,NULL,0);
                }
            }
            break;
        }
        case PASS1_CONFIG_FRAME:
        {
            #if EIS_TEST
            if(mpEisHal)
            {
                if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                {
                    EIS_PASS_ENUM eisPass = EIS_PASS_1;
                    EIS_HAL_CONFIG_DATA  eisConfig;
                    MINT32 eisErr;
                    eisConfig.sensorType    = mSensorType;
                    eisConfig.configSce     = mEisScenario;
                    eisConfig.memAlignment  = 2;
                    eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                    if(eisErr != EIS_RETURN_NO_ERROR)
                    {
                        MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                    }
                }
            }
            #endif
            break;
        }
        default:
        {
            ret = MTRUE;
        }
    }
    //
    //FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultCtrlNodeEngImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNC_START;
    MBOOL         doPost  = MFALSE;
    MUINT32       dstdata = 0;
    MY_LOGV("(%d,0x%08X,0x%08X)", data, buf, ext);

    {
        //drop frame or not
        Mutex::Autolock _l(mLock);
        if( getFlag(muStateFlag, FLAG_BUF_PREVIEW_OK) )
            doPost = MTRUE;
    }

    if( !doPost )
    {
        // drop frame
        handleReturnBuffer(data, buf);
        return MTRUE;
    }

    // update fps
    static MINT64 i8TimeStamp_Old = 0xFFFFFFF;
    static MINT64 i8TimeStamp_New = 0;
    IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
    i8TimeStamp_New = pImageBuffer->getTimestamp();
    if (i8TimeStamp_New > i8TimeStamp_Old)
    {
        mspParamsMgr->updatePreviewFrameInterval((i8TimeStamp_New - i8TimeStamp_Old)/1000);
    }
    i8TimeStamp_Old = i8TimeStamp_New;

    //post buffer to Pass2
    switch(data)
    {
        case CONTROL_FULLRAW:
        {
            dstdata = CONTROL_CAP_SRC;
            break;
        }
        case CONTROL_RESIZEDRAW:
        {
            dstdata = CONTROL_PRV_SRC;
            { // start of Video Raw Dump

                static bool mEnableRawDump = false;
                //(4) enable RawDump
                {
                    String8 const s = mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_DUMP_FLAG);
#undef TRUE
#undef FALSE
                    bool RawDumpFlag = ( ! s.isEmpty() && s == CameraParameters::TRUE ) ? 1 : 0;

                    if ( mEnableRawDump != RawDumpFlag )
                    {
                        MY_LOGD("RawDump flag changed to %d ", RawDumpFlag);
                        mEnableRawDump = RawDumpFlag;
                         g_u4RawDumpCnt = 0;
                    }
                }


                /*
                if(mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE)==1)
                {
                    RAWDUMPCNT = 60;
                }
                else if((mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_VIDEO_RAW_DUMP_RESIZE))==2)
                {
                    RAWDUMPCNT = 120;
                }
                */
                if (mpRawDumpCmdQueThread != 0 && (g_u4RawDumpCnt < mu4MaxRawDumpCnt) && mEnableRawDump == true )
                {
                    MY_LOGD("u4RawDumpCnt, Max = (%d, %d)", g_u4RawDumpCnt, mu4MaxRawDumpCnt); // debug

                    IImageBuffer* pImgbuffer = (IImageBuffer*)buf;
                    mpRawDumpCmdQueThread->postCommand(pImgbuffer);
                    g_u4RawDumpCnt++;
                }
                else if (g_u4RawDumpCnt == mu4MaxRawDumpCnt)
                {
                    MY_LOGD("Stop");
                    mspParamsMgr->set(MtkCameraParameters::KEY_RAW_DUMP_FLAG, CameraParameters::FALSE);
                    mpRawDumpCmdQueThread->postCommand(NULL);
                    mEnableRawDump=false;
                    g_u4RawDumpCnt=0;
                }
            } // end of Video Raw Dump

            //start dump 3aDB collection flash raw
            {
                int const en3AFlashRawDump=mspParamsMgr->getInt(MtkCameraParameters::KEY_ENG_3ADB_FLASH_ENABLE);
                //int const en3AFlashRawDump=1;
                MY_LOGD("en3AFlashRawDump=%d",en3AFlashRawDump);
                if(en3AFlashRawDump == 1)
                {
                    if((mu43aInfoFileHead==0)&&(getFlag(muStateFlag, FLAG_PRECAPTURE)))//start dump 3aDB collection flash raw-save txt file
                    {
                        MY_LOGD("write mu43aInfoFileHead=%d",mu43aInfoFileHead);
                        //
                        String8 s8RawFilePath(mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH));
                        s8RawFilePath = s8RawFilePath.getPathDir();
                        char mpszSuffix[10] = {0};
                        sprintf(mpszSuffix, "/info.txt");
                        s8RawFilePath.append(mpszSuffix);
                        //
                        char str[1024]={0};

                        sprintf(str, "version 0.1\niso100_gain %d\n#image_info w, h, bit, isPacked, size, order, isCompress\nimage_info %d %d %d 0 %d %d 0\n#image_file_name iso, exp, sensor_gain, raw_gain, awb_r, awb_g, awb_b, duty, r_step, flash",
                                         mu4IsoGain, pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h, mu4Bit, pImageBuffer->getBufSizeInBytes(0), mu4Order);
                        //
                        FILE* fptr;
                        fptr = fopen(s8RawFilePath.string(),"wt");
                        if(fptr == NULL)
                        {
                            MY_LOGE("writeheader failed to create file %s", s8RawFilePath.string());
                        }

                        fwrite(str, 1, strlen(str), fptr);
                        putc('\n',fptr);

                        fclose(fptr);

                        mu43aInfoFileHead = 1;
                        MY_LOGD("write mu43aInfoFileHead=%d",mu43aInfoFileHead);

                    }

                    //
                    MY_LOGD("1==en3AFlashRawDump=%d , mPreCapFlashInfo_flag, mCurMagicNum, mu4RawDumpMgcNum:%d , %d, %d",en3AFlashRawDump,mPreCapFlashInfo_flag, mCurMagicNum, mu4RawDumpMgcNum);
                    if (mpRawDumpCmdQueThread != 0 && (mPreCapFlashInfo_flag == 1) && (mu4RawDumpMgcNum!=0xffff))
                    {
                        MY_LOGD("start to dump raw for 3a flash DBC:(mCurMagicNum %d, mu4RawDumpMgcNum %d)", mCurMagicNum, mu4RawDumpMgcNum); // debug

                        IImageBuffer* pImgbuffer = (IImageBuffer*)buf;
                        mpRawDumpCmdQueThread->postCommand(pImgbuffer);
                        mu4RawDumpMgcNum = 0xffff;
                        mPreCapFlashInfo_flag = 0;
                        MY_LOGD("after post command mPreCapFlashInfo_flag=%d",mPreCapFlashInfo_flag);
                    }
                }

            }//end dump 3aDB collection flash raw

            break;
        }
        default:
        {
            MY_LOGE("not support yet: %d", data);
            break;
        }
    }
    //
    #if EIS_TEST
    if(mbUpdateEis)
    {
        if(mpEisHal)
        {
            IspSyncControl::EIS_CROP_INFO eisCrop;
            eisCrop.enable = MFALSE;
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
            EIS_HAL_CONFIG_DATA eisHalCfgData;
            EIS_STATISTIC_STRUCT* pEisStat = new EIS_STATISTIC_STRUCT;

            eisHalCfgData.p1ImgW = (MUINT32)pImageBuffer->getImgSize().w;
            eisHalCfgData.p1ImgH = (MUINT32)pImageBuffer->getImgSize().h;

            mpEisHal->DoEis(
                        EIS_PASS_1,
                        &eisHalCfgData,
                        pImageBuffer->getTimestamp());
            //
            if( mpEisHal->GetEisSupportInfo(getSensorIdx()) &&
                (mMode == MODE_VIDEO_PREVIEW || mMode == MODE_VIDEO_RECORD))
            {
                if( mspParamsMgr->getVideoStabilization() &&
                    mEisScenario == EIS_SCE_EIS)
                {
                    MUINT32 tarWidth = 0, tarHeight = 0;
                    mpEisHal->GetEisResult(
                                eisCrop.xInt,
                                eisCrop.xFlt,
                                eisCrop.yInt,
                                eisCrop.yFlt,
                                tarWidth,
                                tarHeight);
                    //
                    eisCrop.enable = MTRUE;
                    eisCrop.scaleFactor = 100.0/EIS_FACTOR;
                }
            }
            // get EI statistic informaiton
            mpEisHal->GetEisStatistic(pEisStat);

            mpIspSyncCtrl->setEisResult(
                            pImageBuffer,
                            eisCrop,
                            *pEisStat);
        }
        //
        mbUpdateEis = MFALSE;
    }
    #endif
    //
    if(mbUpdateVHdr)
    {
        #ifdef MTK_CAM_VHDR_SUPPORT
        if(mpVHdrHal)
        {
            IImageBuffer* pImageBuffer = (IImageBuffer*)buf;
            mpVHdrHal->DoVHdr(pImageBuffer->getTimestamp());
        }
        #endif
        //
        mbUpdateVHdr = MFALSE;
    }
    //
    handlePostBuffer(dstdata, buf);
    //
    if( data == CONTROL_RESIZEDRAW )
    {
    //  wait here to let pass1node possible to receive CONTROL_STOP_PASS1 notify
    //  before deque next buffer
        MBOOL wait = MFALSE;
        {
            Mutex::Autolock _l(mLock);
            wait = getFlag(muStateFlag, FLAG_PRECAPTURE);
        }

        if( wait )
        {
#define TIMEOUT_WAIT_READYTOCAP     (10000) //us
            CAM_TRACE_NAME("waitReadyToCap");
            MY_LOGD("wait for %d", TIMEOUT_WAIT_READYTOCAP);
            usleep(TIMEOUT_WAIT_READYTOCAP);
#undef TIMEOUT_WAIT_READYTOCAP
        }
    }
    //FUNC_END;
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
void
DefaultCtrlNodeEngImpl::
doNotifyCb(
    MINT32 _msgType,
    MINT32 _ext1,
    MINT32 _ext2,
    MINT32 _ext3,
    MINTPTR _ext4)
{
#define is3aStatus(ext, bit) (ext & (1<<bit))
    //
    switch(_msgType)
    {
        case I3ACallBack::eID_NOTIFY_3APROC_FINISH:
            //NG_TRACE_BEGIN("3Aproc");
            MY_LOGD("3APROC_FINISH:0x%08X,0x%08X",
                    _ext1,
                    _ext2);
            if(mCurMagicNum != (MUINT32)_ext1) {
                MY_LOGE("MagicNum:Cur(0x%08X) != Notify(0x%08X)",
                        mCurMagicNum,
                        _ext1);
                //NG_TRACE_END();
                break;
            }

            if( is3aStatus(_ext2,I3ACallBack::e3AProcOK) ) {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            } else {
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_FAIL);
            }

            if( is3aStatus(_ext2,I3ACallBack::e3APvInitReady) ) {
                updateReadyMagic(mCurMagicNum);
            }
            //NG_TRACE_END();
            break;

        case I3ACallBack::eID_NOTIFY_READY2CAP:
            //NG_TRACE_BEGIN("3AReadyToCap");
            MY_LOGD("ReadyToCap");

            if( !updatePrecap() ) {
                MY_LOGE("updatePrecap failed");
}
            //NG_TRACE_END();
            break;

        //Eng for flash dump precapture raw and setting info of 3A //EM mode
        case I3ACallBack::eID_NOTIFY_COLLECT_FILE_DONE:
            //NG_TRACE_BEGIN("3AReadyToCap");
            MY_LOGD("Ready To Append 3A setting Info and dump precapture raw");

            //NG_TRACE_END();

            expInfo = reinterpret_cast<char*>(_ext4);
            char expInfo1[512];

            strcpy(expInfo1,expInfo);
            mu4RawDumpMgcNum=(MUINT32)_ext2;
            MY_LOGD("mu4RawDumpMgcNum:%d",mu4RawDumpMgcNum);

            //save related flash info.txt
           {

                String8 s8RawFilePath(mspParamsMgr->getStr(MtkCameraParameters::KEY_RAW_PATH));
                String8 ms8RawFileExt(s8RawFilePath.getPathExtension()); // => .raw
                s8RawFilePath = s8RawFilePath.getBasePath();
                char mpszSuffix[10] = {"/info.txt"};
                s8RawFilePath = s8RawFilePath.getPathDir();
                s8RawFilePath.append(mpszSuffix);
                //
                FILE* fptr;
                fptr = fopen(s8RawFilePath.string(),"at");
                if(fptr == NULL)
                {
                    MY_LOGE("failed to create file %s", s8RawFilePath.string());
                }

                fwrite(expInfo1, 1, strlen(expInfo1), fptr);

                fclose(fptr);
            }

            break;

        default:
            break;
    }
#undef is3aStatus

}
/*******************************************************************************
*
********************************************************************************/

}
