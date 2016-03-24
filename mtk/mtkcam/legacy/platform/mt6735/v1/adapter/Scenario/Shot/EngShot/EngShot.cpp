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

#define LOG_TAG "MtkCam/Shot"
// get_boot_mode
#include <sys/stat.h>
#include <fcntl.h>

//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <mtkcam/exif/IDbgInfoContainer.h>
#include <mtkcam/hwutils/CameraProfile.h>
using namespace CPTool;
//
#include <mtkcam/hal/IHalSensor.h>
//#include <mtkcam/hal/sensor_hal.h>
//
#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/ISingleShot.h>
#include <mtkcam/camshot/ISmartShot.h>
#include <mtkcam/camshot/IMfllShot.h>
//
#include <mtkcam/exif/IBaseCamExif.h>
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "EngParam.h"
#include "EngShot.h"
//
#include <mtkcam/featureio/IHal3A.h>
//
#include <ae_param.h>
//

using namespace NS3A;
//
using namespace android;
using namespace NSShot;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] " fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
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
extern "C"
sp<IShot>
createInstance_EngShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    CAM_LOGD("createInstance_EngShot");
    sp<IShot>       pShot = NULL;
    sp<EngShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new EngShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new EngShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
EngShot::
onCreate()
{
#warning "[TODO] EngShot::onCreate()"
    bool ret = true;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
EngShot::
onDestroy()
{
#warning "[TODO] EngShot::onDestroy()"
}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
EngShot(
    char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId
)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
}


/******************************************************************************
 *
 ******************************************************************************/
EngShot::
~EngShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    AutoCPTLog cptlog(Event_Shot_sendCmd, cmd, arg1);
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    case eCmd_setShotParam:
        ret = setEngParam(reinterpret_cast<void const*>(arg1), arg2);
        ret &= ImpShot::sendCommand(cmd, arg1, sizeof(ShotParam));
        break;
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }
    //
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_reset()
{
#warning "[TODO] EngShot::onCmd_reset()"
    bool ret = true;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
 extern "C"
{
int getBootMode(void)
{
  int fd;
  size_t s;
  char boot_mode[4] = {'0'};

  fd = open("/sys/class/BOOT/BOOT/boot/boot_mode", O_RDWR);
  if (fd < 0)
  {
    CAM_LOGW("fail to open: %s\n", "/sys/class/BOOT/BOOT/boot/boot_mode");
    return 0;
  }

  s = read(fd, (void *)&boot_mode, sizeof(boot_mode) - 1);
  close(fd);

  if(s <= 0)
  {
        CAM_LOGW("could not read boot mode sys file\n");
       return 0;
  }

  boot_mode[s] = '\0';
  return atoi(boot_mode);
}
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
onCmd_capture()
{
    AutoCPTLog cptlog(Event_Shot_capture);
    MBOOL ret = MTRUE;
    MBOOL isMfll = MFALSE;

    NSCamShot::ICamShot *pSingleShot; // NSCamShot::ISingleShot *pSingleShot;
    switch (mpEngParam->mi4CamShotType)
    {
        case EngParam::ENG_CAMSHOT_TYPE_MFLL:
            isMfll = MTRUE;
            pSingleShot =
                NSCamShot::IMfllShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "IMFLL", 3); // createInstance(EShotMode const eShotMode, char const* const pszCamShotName, MUINT32 mfb);

            CAM_LOGD("IMfllShot::createInstance");
            break;
        case EngParam::ENG_CAMSHOT_TYPE_SINGLE:
        default:
            isMfll = MFALSE;
            pSingleShot =
                NSCamShot::ISingleShot::createInstance(static_cast<EShotMode>(mu4ShotMode), "EngShot");

            CAM_LOGD("ISingleShot::createInstance");
            break;
    }
    //
    MUINT32 nrtype = queryCapNRType( getCaptureIso(), isMfll );
    if (1 == mpEngParam->mi4ManualMultiNREn)
    {
        nrtype = mpEngParam->mi4ManualMultiNRType; // Overwrite auto Multi-NR values if mi4ManualMultiNREn is 1
        CAM_LOGD("mi4ManualMultiNREn, type = %d", mpEngParam->mi4ManualMultiNRType);
    }
    //
    pSingleShot->init();
    //
    pSingleShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
    //
    MINT32 isEnablePostViewFromPrvCB = 0;
    if (0 == mShotParam.miPostviewDisplayFormat)
    {
        mShotParam.miPostviewDisplayFormat = mShotParam.miPostviewClientFormat;
        isEnablePostViewFromPrvCB = 1;
        CAM_LOGD("mShotParam.miPostviewDisplayFormat to %d from %d", mShotParam.miPostviewClientFormat, mShotParam.miPostviewDisplayFormat);
    }

    EImageFormat ePostViewFmt =
        static_cast<EImageFormat>(mShotParam.miPostviewDisplayFormat);

    MINT32 i4msg = 0;
    if (mpEngParam->mi4EngRawSaveEn)
    {
        i4msg |= NSCamShot::ECamShot_DATA_MSG_RAW;
    }
    if (1 == isEnablePostViewFromPrvCB)
    {
        i4msg |= NSCamShot::ECamShot_DATA_MSG_POSTVIEW;
    }
    pSingleShot->enableDataMsg(
            NSCamShot::ECamShot_DATA_MSG_JPEG | i4msg
#if 0
            | ((ePostViewFmt != eImgFmt_UNKNOWN) ?
             NSCamShot::ECamShot_DATA_MSG_POSTVIEW : NSCamShot::ECamShot_DATA_MSG_NONE)
#endif
            );
    //
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
        if(pIHalSensorList)
        {
            MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());

            NSCam::SensorStaticInfo sensorStaticInfo;
            memset(&sensorStaticInfo, 0, sizeof(NSCam::SensorStaticInfo));
            pIHalSensorList->querySensorStaticInfo(sensorDev, &sensorStaticInfo);

            mpEngParam->u4RawPixelID = sensorStaticInfo.sensorFormatOrder; /* SENSOR_OUTPUT_FORMAT_RAW_B=0, SENSOR_OUTPUT_FORMAT_RAW_Gb, SENSOR_OUTPUT_FORMAT_RAW_Gr, SENSOR_OUTPUT_FORMAT_RAW_R */

#define scenario_case(scenario, KEY, mpEngParam)       \
            case scenario:                            \
                (mpEngParam)->u4SensorWidth = sensorStaticInfo.KEY##Width;  \
                (mpEngParam)->u4SensorHeight = sensorStaticInfo.KEY##Height; \
                break;

            switch (mpEngParam->mi4EngSensorMode)
            {
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_PREVIEW, preview, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE, capture, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO, video, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1, video1, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2, video2, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM1, SensorCustom1, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM2, SensorCustom2, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM3, SensorCustom3, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM4, SensorCustom4, mpEngParam );
            scenario_case( EngParam::ENG_SENSOR_MODE_CUSTOM5, SensorCustom5, mpEngParam );
            default:
                MY_LOGE("not support sensor scenario(0x%x)", mpEngParam->mi4EngSensorMode);
                ret = MFALSE;
                    break;
            }
        }
#undef  scenario_case


    // Align Jpeg Size to RAW Size
    //Only when raw save and in normal mode
    if ((mpEngParam->mi4EngRawSaveEn == 1 && getBootMode() == 0) &&
            (mpEngParam->mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_NORMAL_CAPTURE)&&
            (mpEngParam->mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_NORMAL_VIDEO)&&
            (mpEngParam->mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_SLIM_VIDEO1)&&
            (mpEngParam->mi4EngSensorMode != EngParam::ENG_SENSOR_MODE_SLIM_VIDEO2))
    {
        // 1. keep to the same ratio
        // 2. size is closed to sensor raw.

        // srcW/srcH < dstW/dstH
        if (mpEngParam->u4SensorWidth * mShotParam.mi4PostviewHeight <= mShotParam.mi4PostviewWidth * mpEngParam->u4SensorHeight) {
            mShotParam.mi4PictureWidth = ( mpEngParam->u4SensorWidth ) & (~0xF);
            mShotParam.mi4PictureHeight = ( mpEngParam->u4SensorWidth * mShotParam.mi4PostviewHeight / mShotParam.mi4PostviewWidth ) & (~0xF);
        }
        //srcW/srcH > dstW/dstH
        else
        {
            mShotParam.mi4PictureWidth = ( mpEngParam->u4SensorHeight* mShotParam.mi4PostviewWidth / mShotParam.mi4PostviewHeight ) & (~0xF);
            mShotParam.mi4PictureHeight = ( mpEngParam->u4SensorHeight ) & (~0xF);
        }

    }

    // shot param
    NSCamShot::ShotParam rShotParam(
            eImgFmt_YUY2,                    //yuv format
            mShotParam.mi4PictureWidth,      //picutre width
            mShotParam.mi4PictureHeight,     //picture height
            mShotParam.mu4Transform,         //picture transform
            ePostViewFmt,                    //postview format
            mShotParam.mi4PostviewWidth,     //postview width
            mShotParam.mi4PostviewHeight,    //postview height
            0,                               //postview transform
            mShotParam.mu4ZoomRatio          //zoom
            );

    // jpeg param
    NSCamShot::JpegParam rJpegParam(
            NSCamShot::ThumbnailParam(
                mJpegParam.mi4JpegThumbWidth,
                mJpegParam.mi4JpegThumbHeight,
                mJpegParam.mu4JpegThumbQuality,
                MTRUE),
            mJpegParam.mu4JpegQuality,         //Quality
            MFALSE                             //isSOI
            );


    // sensor param
    MUINT32 u4Scenario;
    if (1 == mpEngParam->mi4EngRawSaveEn)
    {
// mpEngParam->mi4EngSensorMode = SENSOR_SCENARIO_ID_NORMAL_CAPTURE; // force sensor mode to SENSOR_SCENARIO_ID_NORMAL_CAPTURE until stable version
// mpEngParam->mi4EngIspMode = EngParam::ENG_RAW_TYPE_PROCESSED_RAW; // force raw type to processed raw until stable version // ENG_RAW_TYPE_PROCESSED_RAW = 1

            u4Scenario = mpEngParam->mi4EngSensorMode;
    }
    else
    {
#warning [FIXME] workaround for Alta phone capture mode cant work
        u4Scenario = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
    }

    NSCamShot::SensorParam rSensorParam(
            getOpenId(),                            //sensor idx
            u4Scenario,                             //Scenaio
            10,                                     //bit depth
            MFALSE,                                 //bypass delay
            MFALSE                                  //bypass scenario
            , mpEngParam->mi4EngIspMode);            // u4RawType

    //
    mpEngParam->u4Bitdepth = rSensorParam.u4Bitdepth;

    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    ret = pSingleShot->setShotParam(rShotParam);
    //
    ret = pSingleShot->setJpegParam(rJpegParam);
    //
    if (0 == isEnablePostViewFromPrvCB)
    {
        ret = pSingleShot->setPrvBufHdl((MVOID*)mpPrvBufHandler);
    }
    //
    ret = pSingleShot->sendCommand( NSCamShot::ECamShot_CMD_SET_NRTYPE, nrtype, 0, 0 );
    //
    if (mpEngParam->u4VHDState != SENSOR_VHDR_MODE_NONE)
    {
        NS3A::EIspProfile_T ispProfile = NS3A::EIspProfile_Video;
        IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        if(mpEngParam->u4VHDState == SENSOR_VHDR_MODE_IVHDR)
        {
            ispProfile = NS3A::EIspProfile_IHDR_Video;
            pHal3A->SetAETargetMode(1); //AE_MODE_AOE_TARGET
        }
        else
        if(mpEngParam->u4VHDState == SENSOR_VHDR_MODE_MVHDR)
        {
            ispProfile = NS3A::EIspProfile_MHDR_Video;
            pHal3A->SetAETargetMode(2); //AE_MODE_MVHDR_TARGET
        }
        pHal3A->destroyInstance(LOG_TAG);
        pHal3A = NULL;

        ret = pSingleShot->sendCommand( NSCamShot::ECamShot_CMD_SET_VHDR_PROFILE, ispProfile, 0, 0 );
    }
    //
    mpEngParam->mi4MFLLnum = 0;

    // manual exp time / sensor gain control
    if (mpEngParam->u4Exposuretime != 0 && mpEngParam->u4SensorGain != 0)
    {
        ret = setAEEnable(0);
        ret = setManualAEControl(mpEngParam->u4Exposuretime, mpEngParam->u4SensorGain, mpEngParam->u4EVvalue);
    }

    //EV breacket
    if (1 == mpEngParam->mi4EngRawEVBEn)//
    {
        ret = setAEEnable(0);
        strAEOutput aeOut;
        IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        pHal3A->send3ACtrl(E3ACtrl_GetEvCapture, (MINT8)((mpEngParam->u4EVvalue)/10), (MINTPTR)&aeOut); //D0Nenable, 0Ndisable
        MY_LOGD("send3ACtrl ev-value: %d ", ((mpEngParam->u4EVvalue)/10));
        ret = setManualAEControl(aeOut.EvSetting.u4Eposuretime, aeOut.EvSetting.u4AfeGain, aeOut.EvSetting.u4IspGain);
        pHal3A->destroyInstance(LOG_TAG);
    }
    //
    ret = pSingleShot->startOne(rSensorParam);

    ret = setAEEnable(1);
    //
    ret = pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();


    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
setManualAEControl(MUINT32 u4ExpTime, MUINT32 u4ExpGain, MUINT32 u4IspGain)
{
        NS3A::CaptureParam_T rCap3AParam;
        NS3A::Param_T rCam3aParam;

        IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
        // disable AE first
        pHal3A->getParams(rCam3aParam);
        rCam3aParam.bIsAELock = 1;
        pHal3A->setParams(rCam3aParam);
        // set manual exp time / sensor gain
        pHal3A->getCaptureParams(rCap3AParam);
        rCap3AParam.u4ExposureMode = 0;
        if (1 == mpEngParam->mi4EngRawEVBEn)
        rCap3AParam.u4Eposuretime = u4ExpTime;
        else
            rCap3AParam.u4Eposuretime = u4ExpTime * 1000;
        rCap3AParam.u4AfeGain = u4ExpGain;
        rCap3AParam.u4IspGain = u4IspGain;
        pHal3A->updateCaptureParams(rCap3AParam);
        pHal3A->destroyInstance(LOG_TAG);
         MY_LOGD("manual AE: exp time, afegain, ispgain = (%d, %d, %d)",  rCap3AParam.u4Eposuretime, rCap3AParam.u4AfeGain, rCap3AParam.u4IspGain);
        return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
EngShot::
setAEEnable(MUINT32 u4Enable)
{
    bool ret = MTRUE;
    IHal3A* pHal3A = IHal3A::createInstance( IHal3A::E_Camera_1, getOpenId(), LOG_TAG);
    ret = pHal3A->send3ACtrl(E3ACtrl_EnableDisableAE, u4Enable, 0); //D0Nenable, 0Ndisable
    pHal3A->destroyInstance(LOG_TAG);
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
EngShot::
onCmd_cancel()
{
    AutoCPTLog cptlog(Event_Shot_cancel);
#warning "[TODO] EngShot::onCmd_cancel()"
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EngShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    AutoCPTLog cptlog(Event_Shot_handleNotifyCb);
    EngShot *pEngShot = reinterpret_cast <EngShot *>(user);
    if (NULL != pEngShot)
    {
        if ( NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType)
        {
            pEngShot->mpShotCallback->onCB_Shutter(true,
                                                      0
                                                     );
        }
    }

    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
EngShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo const msg)
{
    EngShot *pEngShot = reinterpret_cast<EngShot *>(user);
    if (NULL != pEngShot)
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType)
        {
            MUINT32 u4Size = 0;
            for (size_t i = 0; i < msg.pBuffer->getPlaneCount(); i++)
            {
                u4Size += msg.pBuffer->getBufSizeInBytes(i);
            }

            MUINT8* puBuf = (MUINT8 *)malloc(u4Size);
            if (puBuf != NULL)
            {
                MUINT8* puTmp = puBuf;
                memcpy(puTmp, (MUINT8 *)msg.pBuffer->getBufVA(0), msg.pBuffer->getBufSizeInBytes(0));
                for (size_t i = 1; i < msg.pBuffer->getPlaneCount(); i++)
                {
                    puTmp = puTmp + msg.pBuffer->getBufSizeInBytes(i - 1);
                    memcpy(puTmp, (MUINT8 *)msg.pBuffer->getBufVA(i), msg.pBuffer->getBufSizeInBytes(i));
                }

                pEngShot->handlePostViewData(puBuf, u4Size);
                free(puBuf);
            }
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pEngShot->handleJpegData(
                    (IImageBuffer*)msg.pBuffer,
                    (IImageBuffer*)msg.ext1,
                    (IDbgInfoContainer*)msg.ext2
                    );
        }
        else if (NSCamShot::ECamShot_DATA_MSG_RAW == msg.msgType)
        {
            pEngShot->mpEngParam->u4rawStride=msg.pBuffer->getBufStridesInBytes(0);
            pEngShot->mpShotCallback->onCB_RawImage(0, msg.pBuffer->getBufSizeInBytes(0), (MUINT8*)msg.pBuffer->getBufVA(0));
        }
    }
    return MTRUE;
}

bool
EngShot::
setEngParam(void const* pParam, size_t const size)
{
    if  ( ! pParam )
    {
        MY_LOGE("Null pointer to EngParam");
        return  false;
    }
    //
    if  ( size != sizeof(EngParam) )
    {
        MY_LOGE("size[%d] != sizeof(EngParam)[%d]; please fully build source codes", size, sizeof(EngParam));
        return  false;
    }
    //
    mpEngParam = const_cast<EngParam *>(reinterpret_cast<EngParam const*>(pParam));

    return  true;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
#if 1
    AutoCPTLog cptlog(Event_Shot_handlePVData);
    MY_LOGD("+ (puBuf, size) = (%p, %d)", puBuf, u4Size);
    mpShotCallback->onCB_PostviewClient(0,
                                         u4Size,
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        );

    MY_LOGD("-");
#endif
    return  MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
EngShot::
handleJpegData(IImageBuffer* pJpeg, IImageBuffer* pThumb, IDbgInfoContainer* pDbg)
{
    AutoCPTLog cptlog(Event_Shot_handleJpegData);
    MUINT8* puJpegBuf = (MUINT8*)pJpeg->getBufVA(0);
    MUINT32 u4JpegSize = pJpeg->getBitstreamSize();
    MUINT8* puThumbBuf = NULL;
    MUINT32 u4ThumbSize = 0;
    if( pThumb != NULL )
    {
        puThumbBuf = (MUINT8*)pThumb->getBufVA(0);
        u4ThumbSize = pThumb->getBitstreamSize();
    }

    MY_LOGD("+ (puJpgBuf, jpgSize, puThumbBuf, thumbSize, dbg) = (%p, %d, %p, %d, %p)",
            puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize, pDbg);

    MUINT8 *puExifHeaderBuf = new MUINT8[ DBG_EXIF_SIZE ];
    MUINT32 u4ExifHeaderSize = 0;

    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "makeExifHeader");
    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize, pDbg);
    MY_LOGD("(thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)",
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize);

    // dummy raw callback
    mpShotCallback->onCB_RawImage(0, 0, NULL);

    // Jpeg callback
    CPTLogStr(Event_Shot_handleJpegData, CPTFlagSeparator, "onCB_CompressedImage");
    mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize,
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index
                                         true                     //final image
                                         );
    MY_LOGD("-");

    delete [] puExifHeaderBuf;

    return MTRUE;

}


