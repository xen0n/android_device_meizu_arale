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
#define LOG_TAG "aaa_hal_yuv"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>

#include <mcu_drv.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>

#include <kd_camera_feature.h>
#include <mtkcam/common/faces.h>
#include <math.h>
#include <camera_custom_flashlight.h>
//#include <aaa_yuv_tuning_custom.h>

#include "aaa_hal_yuv.h"

#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>

#include <aaa_common_custom.h>
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <sys/prctl.h>


using namespace NS3A;
using namespace NSCam;
using namespace NSIspTuning;
//using namespace NSYuvTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

#define MIN_ISO (50)

#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}\

#define GET_PROP(prop, dft, val)\
{\
   char value[PROPERTY_VALUE_MAX] = {'\0'};\
   property_get(prop, value, (dft));\
   (val) = atoi(value);\
}

#define YUV_IMG_WD                  (320)
#define YUV_IMG_HT                  (240)
#define ANDR_IMG_WD                 (2000)
#define ANDR_IMG_HT                 (2000)

typedef enum
{
    E_YUV_STATE_IDLE          = 0,
    E_YUV_STATE_PREVIEW       = 1,
    E_YUV_STATE_CAPTURE       = 2,
    E_YUV_STATE_RECORDING     = 3,
    E_YUV_STATE_PRECAPTURE    = 4
} E_YUV_STATE;

typedef enum
{
    E_YUV_SAF_DONE = 0,
    E_YUV_SAF_FOCUSING = 1,
    E_YUV_SAF_INCAF = 2
} E_YUV_SAF;

typedef enum
{
    E_YUV_WIN_STATE_NO_ACTIVITY  = 0,
    E_YUV_WIN_STATE_RESET_IN_FD  = 1,
    E_YUV_WIN_STATE_RESET_IN_CAF = 2
} E_YUV_WIN_STATE;

template<MINT32 const iSensorDev>
class Hal3AYuvDev : public Hal3AYuv
{
public:
    static Hal3AYuv* getInstance()
    {
        static Hal3AYuvDev<iSensorDev> singleton;
        return &singleton;
    }

    Hal3AYuvDev() : Hal3AYuv()
    {
        m_i4SensorDev = iSensorDev;
        MY_LOG("[%s] Hal3AYuv is created. m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    }

    virtual ~Hal3AYuvDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv*
Hal3AYuv::
createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex)
{
    Hal3AYuv *pHal3AYuv = NULL;

    switch (i4SensorDevId)
    {
    case SENSOR_DEV_MAIN:
        pHal3AYuv = Hal3AYuvDev<SENSOR_DEV_MAIN>::getInstance();
        pHal3AYuv->init(i4SensorOpenIndex);
        break;
    case SENSOR_DEV_SUB:
        pHal3AYuv = Hal3AYuvDev<SENSOR_DEV_SUB>::getInstance();
        pHal3AYuv->init(i4SensorOpenIndex);
        break;
    case SENSOR_DEV_MAIN_2:
        pHal3AYuv = Hal3AYuvDev<SENSOR_DEV_MAIN_2>::getInstance();
        pHal3AYuv->init(i4SensorOpenIndex);
        break;
    default:
        MY_ERR("Unsupport i4SensorDevId(0x%08x)\n", i4SensorDevId);
        break;
    }

    return pHal3AYuv;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AYuv::
destroyInstance()
{
    uninit();
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::Hal3AYuv()
    : Hal3AIf()
    , m_Users(0)
    , m_Lock()
    , m_LockAF()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    , m_i4SensorType(ESensorType_YUV)
    , m_pIHalSensor(NULL)
    , m_fgAfTrig(MFALSE)
    , m_fgAfTrigMode(MFALSE)
    , m_fgAeLockSupp(0)
    , m_fgAwbLockSupp(0)
    , m_i4LEDPartId(1)
    , m_pStrobeDrvObj(NULL)
    , m_bAFThreadLoop(0)
    , m_ValidateLock()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AYuv::~Hal3AYuv()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
init(MINT32 i4SensorOpenIndex)
{
    MINT32 i4CurrLensId = 0;
    MINT32 i4CurrSensorId = 0;
    MRESULT ret = S_3A_OK;

    Mutex::Autolock lock(m_Lock);

    GET_PROP("debug.aaa_hal_yuv.log", "0", m_u4LogEn);
    GET_PROP("debug.aaa_hal_yuv.aftrigmode", "0", m_fgAfTrigMode);

    MBOOL fgLogEn = (m_u4LogEn & HAL3AYUV_LOG_INIT) ? MTRUE : MFALSE;

    if (m_Users > 0)
    {
        android_atomic_inc(&m_Users);
        MY_LOG_IF(fgLogEn, "[%s] m_Users(%d)\n", __FUNCTION__, m_Users);
        return S_3A_OK;
    }

    MY_LOG("[%s] First init, m_i4SensorDev(%d), m_i4SensorOpenIdx(%d)\n", __FUNCTION__, m_i4SensorDev, i4SensorOpenIndex);
    m_i4SensorOpenIdx = i4SensorOpenIndex;

    // SensorHal init
    if (!m_pIHalSensor)
    {
        IHalSensorList* const pIHalSensorList = IHalSensorList::get();
        m_pIHalSensor = pIHalSensorList->createSensor("aaa_hal_yuv", m_i4SensorOpenIdx);

        if (!m_pIHalSensor)
        {
            MY_ERR("m_pIHalSensor create fail");
            return E_3A_ERR;
        }
        else
        {
            SensorStaticInfo rSensorStaticInfo;
            pIHalSensorList->querySensorStaticInfo(m_i4SensorDev, &rSensorStaticInfo);
            i4CurrSensorId = rSensorStaticInfo.sensorDevID;
            MY_LOG("[%s] m_pIHalSensor(0x%08x) create OK. ID(0x%08x)", __FUNCTION__, m_pIHalSensor, i4CurrSensorId);
        }
    }

    // lens init
    MCUDrv::lensSearch(m_i4SensorDev, i4CurrSensorId);
    i4CurrLensId = MCUDrv::getCurrLensID(m_i4SensorDev);
    m_fgIsDummyLens = (i4CurrLensId == SENSOR_DRIVE_LENS_ID) ? MFALSE : MTRUE;
    MY_LOG("[%s] i4CurrLensId(0x%08x), m_fgIsDummyLens(%d)\n", __FUNCTION__, i4CurrLensId, m_fgIsDummyLens);

    // strobe init
    m_pStrobeDrvObj = StrobeDrv::getInstance(m_i4SensorDev);
    if (m_pStrobeDrvObj)
    {
        m_pStrobeDrvObj->getPartId(&m_i4LEDPartId);
        m_pStrobeDrvObj->init();
        m_aeFlashlightType = m_pStrobeDrvObj->getFlashlightType();
        if (m_aeFlashlightType == StrobeDrv::FLASHLIGHT_NONE)
        {
            m_pStrobeDrvObj->uninit();
            m_pStrobeDrvObj = NULL;
        }
        MY_LOG("strobe type(%d)", m_aeFlashlightType);
    }
    NSCamCustom::custom_GetYuvFLParam(m_i4LEDPartId, m_rYuvFlParam);
    MY_LOG("[%s] m_pStrobeDrvObj(0x%08x), m_aeFlashlightType(%d)\n", __FUNCTION__, m_pStrobeDrvObj, m_aeFlashlightType);

    if (m_pStrobeDrvObj)
    {
        MY_LOG("[%s] ParId(%d) Th(%3.3f) Duty(%d) Step(%d) Frm(%d) PreAF(%d) Gain(%d) HighDuty(%d) TO(%d) AfLamp(%d)", __FUNCTION__,
            m_i4LEDPartId,
            m_rYuvFlParam.dFlashlightThreshold,
            m_rYuvFlParam.i4FlashlightDuty,
            m_rYuvFlParam.i4FlashlightStep,
            m_rYuvFlParam.i4FlashlightFrameCnt,
            m_rYuvFlParam.i4FlashlightPreflashAF,
            m_rYuvFlParam.i4FlashlightGain10X,
            m_rYuvFlParam.i4FlashlightHighCurrentDuty,
            m_rYuvFlParam.i4FlashlightHighCurrentTimeout,
            m_rYuvFlParam.i4FlashlightAfLampSupport
        );
        if (m_pStrobeDrvObj->setStep(m_rYuvFlParam.i4FlashlightStep) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setStep(%d)", __FUNCTION__, m_rYuvFlParam.i4FlashlightStep);
        }
    }

    m_fBVThreshold      = m_rYuvFlParam.dFlashlightThreshold;
    m_u4PreFlashFrmCnt  = m_rYuvFlParam.i4FlashlightFrameCnt;
    m_u4StrobeDuty      = m_rYuvFlParam.i4FlashlightDuty;
    m_i4AutoFocus = MFALSE;
    m_i4AutoFocusTimeout = 0;
    m_bAeLimiter = 0;
    m_i4FDFrmCnt = 0;
    m_i4FDApplyCnt = 0;
    m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
    m_fgAfTrig = MFALSE;
    m_bForceUpdatParam = MFALSE;
    m_i4State = E_YUV_STATE_IDLE;

    // init ASD
    SENSOR_AE_AWB_REF_STRUCT ref;
    memset(&ref, 0, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_EV_INFO_AWB_REF_GAIN, (MUINTPTR)&ref, 0, 0);
    memcpy(&m_AsdRef, &ref, sizeof(SENSOR_AE_AWB_REF_STRUCT));
    m_AsdRef.SensorLV05LV13EVRef =
        ASDLog2Func(ref.SensorAERef.AeRefLV05Shutter * ref.SensorAERef.AeRefLV05Gain,
                    ref.SensorAERef.AeRefLV13Shutter * ref.SensorAERef.AeRefLV13Gain);
    MY_LOG("[%s] ASD AE Ref: Lv05S(%d) Lv05G(%d) Lv13S(%d) Lv13G(%d) EVRef(%d)\n", __FUNCTION__,
        ref.SensorAERef.AeRefLV05Shutter, ref.SensorAERef.AeRefLV05Gain,
        ref.SensorAERef.AeRefLV13Shutter, ref.SensorAERef.AeRefLV13Gain,
        m_AsdRef.SensorLV05LV13EVRef);

    // init
    TuningMgr::getInstance().init();
#ifdef USE_3A_THREAD
    m_TgInfo = CAM_TG_ERR; //need to call Hal3AYuv::queryTGInfoFromSensorHal() to get correct TG info
    postCommand(ECmd_Init);
    mpIspDrv = IspDrv::createInstance();
    mpIspDrv->init("Hal3AYuv");
    createThread();
    EnableAFThread(1);
#else
    sendCommand(ECmd_Init, 0);
    EnableAFThread(1);
#endif
    android_atomic_inc(&m_Users);

    return S_3A_OK;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AYuv::
uninit()
{
    MRESULT ret = S_3A_OK;

    Mutex::Autolock lock(m_Lock);

    MBOOL fgLogEn = (m_u4LogEn & HAL3AYUV_LOG_UNINIT) ? MTRUE : MFALSE;

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    // There is no more User after decrease one User
    if (m_Users == 0)
    {
        MY_LOG("[%s] Uninit flow, m_i4SensorDev(%d)\n", __FUNCTION__, m_i4SensorDev);

        //Reset Parameter
        Param_T npara;
        m_rParam = npara;
#ifdef USE_3A_THREAD
        destroyThread();
        EnableAFThread(0);
        if (mpIspDrv)
        {
            mpIspDrv->uninit("Hal3AYuv");
            mpIspDrv = NULL;
        }
        postCommand(ECmd_Uninit, 0);
#else
        EnableAFThread(0);
        sendCommand(ECmd_Uninit, 0);
#endif

        TuningMgr::getInstance().uninit();

        // strobe uninit
        if (m_pStrobeDrvObj)
        {
            m_bFlashActive = MFALSE;
            m_pStrobeDrvObj->setOnOff(0);
            m_pStrobeDrvObj->uninit();
            m_pStrobeDrvObj = NULL;
        }

        // SensorHal uninit
        if (m_pIHalSensor)
        {
            m_pIHalSensor->destroyInstance("aaa_hal_yuv");
            m_pIHalSensor = NULL;
        }
    }
    // There are still some users
    else
    {
        MY_LOG_IF(fgLogEn, "[%s] m_Users(%d)\n", __FUNCTION__, m_Users);
    }

    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MRESULT
Hal3AYuv::
onPreCaptureStart()
{
    const MINT32 i4FlashFrmCnt = m_rYuvFlParam.i4FlashlightFrameCnt;

    MY_LOG("[%s +] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    if (m_pStrobeDrvObj)
    {
        MBOOL fgFlashOn = (eShotMode_ZsdShot == m_rParam.u4ShotMode) ? m_isFlashOnCapture : isAEFlashOn();
        if (fgFlashOn)
        {
            queryAEFlashlightInfoFromSensor(m_rAeInfo);
            m_fNoFlashBV = m_fPreFlashBV = calcBV(m_rAeInfo);

            //ON flashlight
            if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setTimeOutTime(0)", __FUNCTION__);
            }
            if (m_pStrobeDrvObj->setDuty(m_u4StrobeDuty) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setLevel(%d)", __FUNCTION__, m_u4StrobeDuty);
            }
            if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
            {
                MY_LOG("[%s] setFire(1)", __FUNCTION__);
                m_u4PreFlashFrmCnt = i4FlashFrmCnt - 1;
                m_u4PreFlashFrmCnt = (MINT32) m_u4PreFlashFrmCnt < i4FlashFrmCnt ? m_u4PreFlashFrmCnt : 0;
                m_bFlashActive = MTRUE;
                m_bExifFlashOn = 1;
            }

            if (m_rYuvFlParam.i4FlashlightPreflashAF)
            {
                // trigger single AF on pre-flash
                MINT32 i4AfState = isFocused();
                MY_LOG("[%s] i4AfState(%d)\n", __FUNCTION__, i4AfState);

                setAFMode(AF_MODE_AFS);
                m_i4AutoFocus = MTRUE;
                m_i4AutoFocusTimeout = 30;
                resetAFAEWindow();
                m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_CANCEL_AF, 0, 0, 0);
                m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE, 0, 0, 0);
            }
        }
        else
        {
            MY_LOG("[%s] No need to flash, ready to capture", __FUNCTION__);
            m_bReadyToCapture = 1;
            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_READY2CAP, 0, 0, 0);
        }
    }
    else
    {
        MY_LOG("[%s] No Strobe, ready to capture", __FUNCTION__);
        m_bReadyToCapture = 1;
        m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_READY2CAP, 0, 0, 0);
    }

    // transit state to precapture
    m_i4State = E_YUV_STATE_PRECAPTURE;

    MY_LOG("[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onPreCaptureEnd()
{
    MY_LOG("[%s] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    resetReadyToCapture();
    m_isFlashOnCapture = 0;

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
updatePreCapture()
{
    const MINT32 i4FlashFrmCnt = m_rYuvFlParam.i4FlashlightFrameCnt;
    MBOOL fgLogEn = (m_u4LogEn & HAL3AYUV_LOG_PRECAPUPDATE) ? MTRUE : MFALSE;

    if (m_pStrobeDrvObj)
    {
        MY_LOG_IF(fgLogEn, "[%s +] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
        if (m_bFlashActive == MTRUE)
        {
            if (m_rYuvFlParam.i4FlashlightPreflashAF)
            {
                if (m_i4AutoFocus)
                {
                    MINT32 i4AfState = isFocused();
                    MY_LOG("[%s] SAF(%d)\n", i4AfState);
                    if (i4AfState == SENSOR_AF_FOCUSED)
                    {
                        m_i4AutoFocus = MFALSE;
                        m_u4PreFlashFrmCnt += 2;
                    }
                    else if (m_i4AutoFocusTimeout == 0)
                    {
                        m_i4AutoFocus = MFALSE;
                    }
                    m_i4AutoFocusTimeout = m_i4AutoFocusTimeout > 0 ? m_i4AutoFocusTimeout - 1 : 0;
                }
            }

            queryAEFlashlightInfoFromSensor(m_rAeInfo);
            m_fPreFlashBV = calcBV(m_rAeInfo);
            m_u4PreFlashFrmCnt = m_u4PreFlashFrmCnt > 0 ? m_u4PreFlashFrmCnt - 1 : 0;
            MY_LOG_IF(fgLogEn, "[%s] Cnt(%d/%d)", __FUNCTION__, m_u4PreFlashFrmCnt, i4FlashFrmCnt);

            if (0 == m_u4PreFlashFrmCnt && MFALSE == m_i4AutoFocus)
            {
                m_i4PreFlashShutter = m_rAeInfo.Exposuretime;
                m_i4PreFlashGain = m_rAeInfo.Gain;
                //OFF flashlight after preflash done.
                if (m_bFlashActive == MTRUE)
                {
                    setAeLock(MTRUE);
                    setAwbLock(MTRUE);
                    if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                    {
                        MY_LOG("[%s] setFire OFF", __FUNCTION__);
                    }
                }
                m_u4PreFlashFrmCnt = i4FlashFrmCnt;
                m_bReadyToCapture = 1;
                m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_READY2CAP, 0, 0, 0);
            }
        }
        MY_LOG_IF(fgLogEn, "[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);
    }

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onCaptureStart()
{
    MY_LOG("[%s +] m_i4SensorDev(%d), shotMode(%d) = %d", __FUNCTION__, m_i4SensorDev, m_rParam.u4ShotMode);
    updateAeFlashCaptureParams();

    // transit state to capture
    m_i4State = E_YUV_STATE_CAPTURE;

    MY_LOG("[%s -] m_i4SensorDev(%d)", __FUNCTION__, m_i4SensorDev);

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onCaptureEnd()
{
    MY_LOG("[%s] m_i4SensorDev(%d), shotMode(%d) = %d", __FUNCTION__, m_i4SensorDev, m_rParam.u4ShotMode);

    if (m_pStrobeDrvObj && m_bFlashActive == MTRUE && m_rParam.u4ShotMode != CAPTURE_MODE_BURST_SHOT)
    {
        m_pStrobeDrvObj->setOnOff(0);
        MY_LOG("[%s] setFire OFF", __FUNCTION__);

        m_bFlashActive = MFALSE;
    }
    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onRecordingStart()
{
    MY_LOG("ECmd_RecordingStart:\n");

    if (m_pStrobeDrvObj && m_rParam.u4StrobeMode == FLASHLIGHT_AUTO)
    {
        MBOOL fgFlashOn = isAEFlashOn();
        if (fgFlashOn)
        {
            //ON flashlight
            if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
            {
                MY_LOG("setTimeOutTime: 0\n");
            }
            if (m_pStrobeDrvObj->setDuty(m_u4StrobeDuty) == MHAL_NO_ERROR)
            {
                MY_LOG("setLevel:%d\n", m_u4StrobeDuty);
            }
            if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
            {
                MY_LOG("setFire ON\n");
                m_bFlashActive = MTRUE;
                m_bExifFlashOn = 1;
            }
        }
    }

    //resetAFAEWindow();

    // transit state to recording
    m_i4State = E_YUV_STATE_RECORDING;

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onRecordingEnd()
{
    MY_LOG("ECmd_RecordingEnd:\n");
    if (m_pStrobeDrvObj && m_bFlashActive == MTRUE)
    {
        m_pStrobeDrvObj->setOnOff(0);
        MY_LOG("setFire OFF\n");

        m_bFlashActive = MFALSE;
    }

    // transit state to preview
    m_i4State = E_YUV_STATE_PREVIEW;

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onPreviewStart()
{
    MY_LOG("[%s]", __FUNCTION__);
    // Force reset Parameter
    Param_T old_para,rst_para;
    memset(&rst_para, 0, sizeof(Param_T));
    old_para = m_rParam;
    m_rParam = rst_para;
    m_bForceUpdatParam = MTRUE;
    setParams(old_para);
    m_bForceUpdatParam = MFALSE;

    m_bExifFlashOn = 0;

    // transit state to preview
    if (m_i4State == E_YUV_STATE_IDLE)
    {
        m_i4InitReadyCnt = getDelayFrame(EQueryType_Init);
        MY_LOG("[%s] Start to count down (%d)", __FUNCTION__, m_i4InitReadyCnt);
    }

    m_i4State = E_YUV_STATE_PREVIEW;

    return S_3A_OK;
}

MRESULT
Hal3AYuv::
onPreviewEnd()
{
    return S_3A_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#ifdef USE_3A_THREAD
MRESULT
Hal3AYuv::
queryTGInfoFromSensorHal()
{
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = m_pIHalSensor->querySensorDynamicInfo(m_i4SensorDev, &senInfo);

    MY_LOG("m_i4SensorDev(%d), TgInfo(%d)\n", m_i4SensorDev, senInfo.TgInfo);

    if ((senInfo.TgInfo != CAM_TG_1) && (senInfo.TgInfo != CAM_TG_2))
    {
        MY_ERR("YUV sensor is connected with TgInfo: %d\n", senInfo.TgInfo);
        return E_3A_ERR;
    }

    m_TgInfo = senInfo.TgInfo; //now, TG info is obtained! TG1 or TG2

    return S_3A_OK;
}


MVOID
Hal3AYuv::
waitVSirq()
{
    if ((m_TgInfo != CAM_TG_1) && (m_TgInfo != CAM_TG_2))
    {
        MY_ERR("m_TgInfo = %d, Suggestion: Need to call queryTGInfoFromSensorHal() before waitVSirq().", m_TgInfo);
        return;
    }

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;
    waitIrq.Clear=ISP_DRV_IRQ_CLEAR_WAIT;
    waitIrq.Timeout=5000; // 5000 msec
    waitIrq.Type = (m_TgInfo == CAM_TG_1) ?
        ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status = (m_TgInfo == CAM_TG_1) ?
        CAM_CTL_INT_P1_STATUS_VS1_INT_ST : CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    waitIrq.UserNumber=ISP_DRV_IRQ_USER_3A;
    waitIrq.UserName=const_cast<char *>("Hal3AYuvVsync");

    if (mpIspDrv==NULL)
    {
        MY_ERR("isp drv = NULL");
    }
    if (mpIspDrv->waitIrq(&waitIrq) <= 0)
    {
        MY_ERR("wait vsync timeout");
    }
}

MBOOL
Hal3AYuv::postCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
#else
MBOOL
Hal3AYuv::sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
#endif
{
    MBOOL bRet = MTRUE;

    if (eCmd != ECmd_Update)
    {
        m_i4AFSwitchCtrl = -1;
        MY_LOG("[%s] eCmd(%d)\n", __FUNCTION__, eCmd);
    }

    switch (eCmd)
    {
    case ECmd_Init:
    case ECmd_Uninit:
    case ECmd_CameraPreviewEnd:
    case ECmd_CamcorderPreviewEnd:
        m_i4AFSwitchCtrl = -1;
        // transit state to idle
        m_i4State = E_YUV_STATE_IDLE;
        break;

    case ECmd_CameraPreviewStart:
    case ECmd_CamcorderPreviewStart:
        onPreviewStart();
        break;

    case ECmd_PrecaptureStart:
        onPreCaptureStart();
        break;
    case ECmd_PrecaptureEnd:
        onPreCaptureEnd();
        break;
    case ECmd_CaptureStart:
        onCaptureStart();
        break;
    case ECmd_CaptureEnd:
        onCaptureEnd();
        break;
    case ECmd_RecordingStart:
        onRecordingStart();
        break;
    case ECmd_RecordingEnd:
        onRecordingEnd();
        break;
    case ECmd_Update:
        {
            MBOOL fgLogEn = (m_u4LogEn & HAL3AYUV_LOG_3AUPDATE) ? MTRUE : MFALSE;
            const ParamIspProfile_T* pParamIspProf = reinterpret_cast<const ParamIspProfile_T*>(i4Arg);
            MY_LOG_IF(fgLogEn, "[%s] ECmd_Update (%d)", __FUNCTION__, pParamIspProf->i4MagicNum);
            get3AStatusFromSensor();

            switch (m_i4State)
            {
            case E_YUV_STATE_PRECAPTURE:
                updatePreCapture();
                break;
            case E_YUV_STATE_IDLE:
            case E_YUV_STATE_CAPTURE:
                // do nothing
                break;
            default:
                //MY_LOG("ECmd_Update:\n");
                ::sem_post(&m_semAFThreadStart);
                break;
            }

            m_i4InitReadyCnt = m_i4InitReadyCnt > 0 ? m_i4InitReadyCnt - 1 : 0;

            validate(*pParamIspProf, MTRUE);

            on3AProcFinish(pParamIspProf->i4MagicNum);
        }
        break;
    default:
        MY_ERR("[%s] Undefined command", __FUNCTION__);
        break;
    }

    return bRet;
}

MBOOL Hal3AYuv::isInVideo()
{
    MBOOL fgVdo = (m_i4State == E_YUV_STATE_RECORDING);
    return fgVdo;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setAeLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AE_LOCK : SENSOR_3A_AE_UNLOCK;

    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (m_fgAeLockSupp == 1)
    {
        MY_LOG("AE Lock supports, send CMD\n");
        m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_3A_CMD, (MUINTPTR)&iYuv3ACmd,0,0);
    }

    return MTRUE;

}

MBOOL Hal3AYuv::setAwbLock(MBOOL bLock)
{
    int iYuv3ACmd = bLock ? SENSOR_3A_AWB_LOCK : SENSOR_3A_AWB_UNLOCK;

    MY_LOG("[%s] bLock = %d\n", __FUNCTION__, bLock);

    if (m_fgAwbLockSupp == 1)
    {
        MY_LOG("AWB Lock supports, send CMD\n");
        m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_3A_CMD, (MUINTPTR)&iYuv3ACmd,0,0);
    }

    return MTRUE;
}

MBOOL Hal3AYuv::setVideoFrmRate(MINT32 i4FrmRate)
{
    MY_LOG("[%s] i4FrmRate(%d)\n", __FUNCTION__, i4FrmRate);
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_VIDEO_FRAME_RATE, (MUINTPTR)&i4FrmRate, 0, 0);

    return MTRUE;
}

MBOOL Hal3AYuv::setFrmRate(MINT32 i4Min, MINT32 i4Max)
{
    MY_LOG("[%s] i4FrmRate(%d,%d)\n", __FUNCTION__, i4Min, i4Max);
    m_pIHalSensor->sendCommand(m_i4SensorDev,  SENSOR_CMD_SET_MIN_MAX_FPS, (MUINTPTR)&i4Min, (MUINTPTR)&i4Max, 0);

    return MTRUE;
}

#define setYuvFeature(Cmd, Param)\
{\
    int cmd = Cmd;\
    int param = (Param);\
    MY_LOG("[%s] "#Cmd" i4Param(%d)", __FUNCTION__, param);\
    m_pIHalSensor->sendCommand(\
        m_i4SensorDev, SENSOR_CMD_SET_YUV_FEATURE_CMD, (MUINTPTR)&cmd, (MUINTPTR)&param, 0);\
}\

MINT32 Hal3AYuv::enableAELimiterControl(MBOOL  bIsAELimiter)
{
    MY_LOG("[%s] bIsAELimiter = %d\n", __FUNCTION__, bIsAELimiter);

    m_bAeLimiter = bIsAELimiter;

    setAeLock(bIsAELimiter);
    setAwbLock(bIsAELimiter);

    return MTRUE;
}

MBOOL Hal3AYuv::setParams(Param_T const &rNewParam)
{
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0;
    MINT32 i4SceneModeUpdate;
    MINT32 i4SceneModeChg;

    MY_LOG("[%s] + \n", __FUNCTION__);

    i4SceneModeUpdate = 1; //rNewParam.u4SceneMode != SCENE_MODE_HDR;

    i4SceneModeChg = m_rParam.u4SceneMode != rNewParam.u4SceneMode || m_bForceUpdatParam;

    if (m_rParam.u4EffectMode != rNewParam.u4EffectMode || m_bForceUpdatParam)
    {
        MY_LOG("[FID_COLOR_EFFECT], (%d)->(%d) \n", m_rParam.u4EffectMode, rNewParam.u4EffectMode);
        setYuvFeature(FID_COLOR_EFFECT, rNewParam.u4EffectMode);
    }

    // scene mode
    if (i4SceneModeUpdate)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(%d) \n", m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            setYuvFeature(FID_SCENE_MODE, rNewParam.u4SceneMode);
        }

        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV], Idx:(%d)->(%d), Step:(%f)->(%f) \n", m_rParam.i4ExpIndex, rNewParam.i4ExpIndex, m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            setYuvFeature(FID_AE_EV, mapAEToEnum(rNewParam.i4ExpIndex, rNewParam.fExpCompStep));
        }

        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE], (%d)->(%d) \n", m_rParam.u4AwbMode, rNewParam.u4AwbMode);
            setYuvFeature(FID_AWB_MODE, rNewParam.u4AwbMode);
        }

        if (m_rParam.u4BrightnessMode != rNewParam.u4BrightnessMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_BRIGHT], (%d)->(%d) \n", m_rParam.u4BrightnessMode, rNewParam.u4BrightnessMode);
            setYuvFeature(FID_ISP_BRIGHT, rNewParam.u4BrightnessMode);
        }
        if (m_rParam.u4HueMode != rNewParam.u4HueMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_HUE], (%d)->(%d) \n", m_rParam.u4HueMode, rNewParam.u4HueMode);
            setYuvFeature(FID_ISP_HUE, rNewParam.u4HueMode);
        }
        if (m_rParam.u4SaturationMode != rNewParam.u4SaturationMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_SAT], (%d)->(%d) \n", m_rParam.u4SaturationMode, rNewParam.u4SaturationMode);
            setYuvFeature(FID_ISP_SAT, rNewParam.u4SaturationMode);
        }
        if (m_rParam.u4ContrastMode != rNewParam.u4ContrastMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_CONTRAST], (%d)->(%d) \n", m_rParam.u4ContrastMode, rNewParam.u4ContrastMode);
            setYuvFeature(FID_ISP_CONTRAST, rNewParam.u4ContrastMode);
        }
        if (m_rParam.u4EdgeMode != rNewParam.u4EdgeMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_ISP_EDGE], (%d)->(%d) \n", m_rParam.u4EdgeMode, rNewParam.u4EdgeMode);
            setYuvFeature(FID_ISP_EDGE, rNewParam.u4EdgeMode);
        }
        if (m_rParam.u4IsoSpeedMode != rNewParam.u4IsoSpeedMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_ISO], (%d)->(%d) \n", m_rParam.u4IsoSpeedMode, rNewParam.u4IsoSpeedMode);
            setYuvFeature(FID_AE_ISO, mapISOToEnum(rNewParam.u4IsoSpeedMode));
        }
    }
    else if (rNewParam.u4SceneMode == SCENE_MODE_HDR)
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(SCENE_MODE_HDR) \n", m_rParam.u4SceneMode);
            // set scene mode off (backward compatible) first, then set scene mode hdr (for JB4.2)
            setYuvFeature(FID_SCENE_MODE, SCENE_MODE_OFF);
            setYuvFeature(FID_SCENE_MODE, SCENE_MODE_HDR);

        }

        if (m_rParam.i4ExpIndex != rNewParam.i4ExpIndex || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AE_EV], Idx:(%d)->(%d), Step:(%f)->(%f) \n", m_rParam.i4ExpIndex, rNewParam.i4ExpIndex, m_rParam.fExpCompStep, rNewParam.fExpCompStep);
            setYuvFeature(FID_AE_EV, mapAEToEnum(rNewParam.i4ExpIndex,rNewParam.fExpCompStep));
        }

        if (m_rParam.u4AwbMode != rNewParam.u4AwbMode || m_bForceUpdatParam)
        {
            MY_LOG("[FID_AWB_MODE], (%d)->(%d) \n", m_rParam.u4AwbMode, rNewParam.u4AwbMode);
            setYuvFeature(FID_AWB_MODE, rNewParam.u4AwbMode);
        }
    }
    else
    {
        if (i4SceneModeChg)
        {
            MY_LOG("[FID_SCENE_MODE], (%d)->(%d) \n", m_rParam.u4SceneMode, rNewParam.u4SceneMode);
            setYuvFeature(FID_SCENE_MODE, rNewParam.u4SceneMode);
        }
    }

    if (m_rParam.u4AntiBandingMode != rNewParam.u4AntiBandingMode || m_bForceUpdatParam)
    {
        MY_LOG("[FID_AE_FLICKER],(%d)->(%d) \n",m_rParam.u4AntiBandingMode, rNewParam.u4AntiBandingMode);
        setYuvFeature(FID_AE_FLICKER, rNewParam.u4AntiBandingMode);
    }

    if ((m_bAeLimiter == 0 && m_rParam.bIsAELock != rNewParam.bIsAELock) || m_bForceUpdatParam)
    {
        setAeLock(rNewParam.bIsAELock);
    }

    if ((m_bAeLimiter == 0 && m_rParam.bIsAWBLock != rNewParam.bIsAWBLock) || m_bForceUpdatParam)
    {
        setAwbLock(rNewParam.bIsAWBLock);
    }

    //for frame rate
    if (m_rParam.bRecordingHint!=rNewParam.bRecordingHint||m_rParam.i4MaxFps!=rNewParam.i4MaxFps||m_rParam.i4MinFps!=rNewParam.i4MinFps || m_bForceUpdatParam)
    {
        MINT32 i4MinFps = rNewParam.i4MinFps;
        MINT32 i4MaxFps = rNewParam.i4MaxFps;
        if (rNewParam.bRecordingHint)
        {
            i4MinFps = MIN(MAX(15000, i4MinFps), i4MaxFps);
            MY_LOG("[%s] Recording, MW Min(%d), Clipped Min(%d)", __FUNCTION__, rNewParam.i4MinFps, i4MinFps);
        }
        MY_LOG("[FID_FIX_FRAMERATE]: Max(%d)->(%d), Min(%d)->(%d) \n", m_rParam.i4MaxFps, i4MaxFps, m_rParam.i4MinFps, i4MinFps);
        setFrmRate(i4MinFps/1000, i4MaxFps/1000);
    }

    if (m_rParam.u4StrobeMode != rNewParam.u4StrobeMode)
    {
        MY_LOG("StrobeMode=%d\n", rNewParam.u4StrobeMode);
        setFlashMode(rNewParam.u4StrobeMode);
    }

    Mutex::Autolock lock(m_LockAF);

    if (((m_rParam.u4AfMode != rNewParam.u4AfMode) || m_bForceUpdatParam) && !m_fgIsDummyLens)
    {
        MY_LOG("m_rParam.u4ShotMode=%d,rNewParam.u4ShotMode=%d", m_rParam.u4ShotMode, rNewParam.u4ShotMode);
        //if (rNewParam.u4ShotMode == eShotMode_Autorama &&
        //    rNewParam.u4AfMode == AF_MODE_AFC)
        //{
        //    MY_LOG("[FID_AF_MODE]eShotMode_Autorama(%d)->(%d),dummy(%d) \n",m_rParam.u4AfMode,rNewParam.u4AfMode,m_fgIsDummyLens);
        //}
        //else
        {
            MY_LOG("[FID_AF_MODE](%d)->(%d),dummy(%d) \n",m_rParam.u4AfMode,rNewParam.u4AfMode,m_fgIsDummyLens);
            setAFMode(rNewParam.u4AfMode);
            m_rParam.u4AfMode = rNewParam.u4AfMode;
        }
    }

    //update AF area
    if (m_max_af_areas > 0)
    {
        UINT32 u4Diff = 0;
        AREA_T focusArea[MAX_FOCUS_AREAS];
        for (MUINT32 i = 0; i < rNewParam.rFocusAreas.u4Count; i++)
        {
            u4Diff += (UINT32)
                (m_rParam.rFocusAreas.rAreas[i].i4Left   != rNewParam.rFocusAreas.rAreas[i].i4Left) +
                (m_rParam.rFocusAreas.rAreas[i].i4Right  != rNewParam.rFocusAreas.rAreas[i].i4Right) +
                (m_rParam.rFocusAreas.rAreas[i].i4Top    != rNewParam.rFocusAreas.rAreas[i].i4Top) +
                (m_rParam.rFocusAreas.rAreas[i].i4Bottom != rNewParam.rFocusAreas.rAreas[i].i4Bottom);
            focusArea[i].i4Left   = rNewParam.rFocusAreas.rAreas[i].i4Left;
            focusArea[i].i4Top    = rNewParam.rFocusAreas.rAreas[i].i4Top;
            focusArea[i].i4Right  = rNewParam.rFocusAreas.rAreas[i].i4Right;
            focusArea[i].i4Bottom = rNewParam.rFocusAreas.rAreas[i].i4Bottom;
        }
        if (u4Diff != 0)
        {
            setFocusAreas(rNewParam.rFocusAreas.u4Count, focusArea);
        }
    }
    //update AE area
    if (m_max_metering_areas > 0)
    {
        UINT32 u4Diff = 0;
        AREA_T meteringArea[MAX_METERING_AREAS];
        for (MUINT32 i = 0; i < rNewParam.rMeteringAreas.u4Count; i++)
        {
            u4Diff += (UINT32)
                (m_rParam.rMeteringAreas.rAreas[i].i4Left   != rNewParam.rMeteringAreas.rAreas[i].i4Left) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Right  != rNewParam.rMeteringAreas.rAreas[i].i4Right) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Top    != rNewParam.rMeteringAreas.rAreas[i].i4Top) +
                (m_rParam.rMeteringAreas.rAreas[i].i4Bottom != rNewParam.rMeteringAreas.rAreas[i].i4Bottom);
            meteringArea[i].i4Left   = rNewParam.rMeteringAreas.rAreas[i].i4Left;
            meteringArea[i].i4Top    = rNewParam.rMeteringAreas.rAreas[i].i4Top;
            meteringArea[i].i4Right  = rNewParam.rMeteringAreas.rAreas[i].i4Right;
            meteringArea[i].i4Bottom = rNewParam.rMeteringAreas.rAreas[i].i4Bottom;
        }
        if (u4Diff != 0)
        {
            setMeteringAreas(rNewParam.rMeteringAreas.u4Count, meteringArea);
        }
    }

    m_rParam = rNewParam;

    m_bForceUpdatParam = MFALSE;

    MY_LOG("[%s()] - \n", __FUNCTION__);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::getSupportedParams(FeatureParam_T &rFeatureParam)
{
    MINT32 ae_lock=0,awb_lock=0;
    MINT32 max_focus=0,max_meter=0;

    MY_LOG("[%s()] \n", __FUNCTION__);

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AE_AWB_LOCK,(MUINTPTR)&ae_lock,(MUINTPTR)&awb_lock,0);
    m_fgAeLockSupp = ae_lock==1?1:0;
    m_fgAwbLockSupp = awb_lock==1?1:0;
    rFeatureParam.bExposureLockSupported = m_fgAeLockSupp;
    rFeatureParam.bAutoWhiteBalanceLockSupported = m_fgAwbLockSupp;
    MY_LOG("AE_sup(%d),AWB_sub(%d) \n",m_fgAeLockSupp,m_fgAwbLockSupp);

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AF_MAX_NUM_FOCUS_AREAS,(MUINTPTR)&max_focus,0,0);
    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_AE_MAX_NUM_METERING_AREAS,(MUINTPTR)&max_meter,0,0);
    rFeatureParam.u4MaxMeterAreaNum = max_meter>=1?1:0;
    rFeatureParam.u4MaxFocusAreaNum = max_focus>=1?1:0;
    m_max_metering_areas = max_meter;
    m_max_af_areas = max_focus;
    MY_LOG("FOCUS_max(%d),METER_max(%d) \n",max_focus,max_meter);

    rFeatureParam.bEnableDynamicFrameRate = CUST_ENABLE_VIDEO_DYNAMIC_FRAME_RATE();

    rFeatureParam.u4FocusLength_100x = 350;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::autoFocus()
{
    MY_LOG("[%s] dummylens(%d)", __FUNCTION__, m_fgIsDummyLens);

    Mutex::Autolock lock(m_LockAF);

    if (m_fgIsDummyLens == 1 || m_max_af_areas == 0)
    {
        //add for cts
        m_i4AutoFocus = E_YUV_SAF_FOCUSING;
        MY_LOG("[%s] AF Not Support\n", __FUNCTION__);
        return MTRUE;
    }

    if ((m_rParam.u4AfMode != AF_MODE_AFC) && (m_rParam.u4AfMode != AF_MODE_AFC_VIDEO))
    {
        if (m_fgAfTrigMode)
        {
            m_fgAfTrig = MTRUE;
        }
        else
        {
            m_fgAfTrig = MFALSE;
            setAFLampOnOff(MTRUE);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)m_AFzone,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,0,0,0);
            m_i4AutoFocus = E_YUV_SAF_FOCUSING;
            m_i4AutoFocusTimeout = 30;
        }
        MY_LOG("[%s] Do SAF CMD\n", __FUNCTION__);
    }
    else
    {
        m_i4AutoFocus = E_YUV_SAF_INCAF;
        MY_LOG("[%s] called in AF mode(%d)", __FUNCTION__, m_rParam.u4AfMode);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::cancelAutoFocus()
{
    MY_LOG("[%s()] \n", __FUNCTION__);
    if (m_fgIsDummyLens == 1){return MTRUE;}

#if 0
    switch (m_rParam.u4AfMode)
    {
    case AF_MODE_INFINITY:
        //m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        //break;
    case AF_MODE_AFC:
    case AF_MODE_AFC_VIDEO:
        m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        break;
    default:
    case AF_MODE_AFS:
        MY_LOG("Do nothing, u4AfMode = %d\n", m_rParam.u4AfMode);
        break;
    }
#else
    if (m_rParam.u4AfMode != AF_MODE_AFS && m_rParam.u4ShotMode != eShotMode_Autorama)
    {
        m_rParam.u4AfMode = AF_MODE_INFINITY;
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
    }
    m_i4AutoFocus = E_YUV_SAF_DONE;
#endif

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
//    ERROR_CHECK(AeMgr::getInstance().setZoomWinInfo(u4XOffset, u4YOffset, u4Width, u4Height))

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
{
    MY_LOG("[%s]", __FUNCTION__);

    SENSOR_EXIF_INFO_STRUCT mSensorInfo;
    EXIF_INFO_T rEXIFInfo;
    memset(&rEXIFInfo, 0, sizeof(EXIF_INFO_T));
    memset(&mSensorInfo, 0, sizeof(SENSOR_EXIF_INFO_STRUCT));

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_EXIF_INFO,(MUINTPTR)&mSensorInfo,0,0);

    MY_LOG("[%s] FNumber(%d), AEISOSpeed(%d), AWBMode(%d), CapExposureTime(%d), FlashLightTimeus(%d), RealISOValue(%d)",
            __FUNCTION__,
            mSensorInfo.FNumber, m_rParam.u4IsoSpeedMode, m_rParam.u4AwbMode,
            mSensorInfo.CapExposureTime, m_bExifFlashOn, mSensorInfo.RealISOValue);

    rEXIFInfo.u4FNumber = mSensorInfo.FNumber>0 ? mSensorInfo.FNumber : 28;
    rEXIFInfo.u4FocalLength = 350;
    rEXIFInfo.u4SceneMode = m_rParam.u4SceneMode;
    rEXIFInfo.u4AWBMode = m_rParam.u4AwbMode;
    rEXIFInfo.u4CapExposureTime = mSensorInfo.CapExposureTime>0? mSensorInfo.CapExposureTime : 0;
    rEXIFInfo.u4FlashLightTimeus = m_bExifFlashOn;
    rEXIFInfo.u4AEISOSpeed = m_rParam.u4IsoSpeedMode;    // in ISO value (not enum)
    rEXIFInfo.u4RealISOValue = mSensorInfo.RealISOValue; // in ISO value
    rEXIFInfo.i4AEExpBias = m_rParam.i4ExpIndex;

    pIBaseCamExif->set3AEXIFInfo(&rEXIFInfo);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getDelayFrame(EQueryType_T const eQueryType) const
{
    MUINT32 ret = 0;
    SENSOR_DELAY_INFO_STRUCT pDelay;

    MY_LOG("[%s()] \n", __FUNCTION__);

    memset(&pDelay,0x0,sizeof(SENSOR_DELAY_INFO_STRUCT));
    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_GET_YUV_DELAY_INFO,(MUINTPTR)&pDelay,0,0);
    MY_LOG("Init:%d,effect:%d,awb:%d,af:%d,ev:%d,sat:%d,bright:%d,contrast:%d \n",
           pDelay.InitDelay,pDelay.EffectDelay,pDelay.AwbDelay,pDelay.AFSwitchDelayFrame,
           pDelay.EvDelay,pDelay.SatDelay,pDelay.BrightDelay,pDelay.ContrastDelay);

    switch (eQueryType)
    {
        case EQueryType_Init:
        {
            ret = (pDelay.InitDelay>0 && pDelay.InitDelay<5)?pDelay.InitDelay:0;
            return ret;
        }
        case EQueryType_Effect:
        {
             ret = (pDelay.EffectDelay>0 && pDelay.EffectDelay<5)?pDelay.EffectDelay:0;
             return ret;
        }
        case EQueryType_AWB:
        {
            ret = (pDelay.AwbDelay>0 && pDelay.AwbDelay<5)?pDelay.AwbDelay:0;
            return ret;
        }
        case EQueryType_AF:
        {
            ret = pDelay.AFSwitchDelayFrame;
            ret = ret < 1200 ? ret : 0;
            return ret;
        }
        case EQueryType_Ev:
        {
            ret = (pDelay.EvDelay>0 && pDelay.EvDelay<5)?pDelay.EvDelay:0;
            return ret;
        }
        case EQueryType_Sat:
        {
            ret = (pDelay.SatDelay>0 && pDelay.SatDelay<5)?pDelay.SatDelay:0;
            return ret;
        }
        case EQueryType_Bright:
        {
            ret = (pDelay.BrightDelay>0 && pDelay.BrightDelay<5)?pDelay.BrightDelay:0;
            return ret;
        }
        case EQueryType_Contrast:
        {
            ret = (pDelay.ContrastDelay>0 && pDelay.ContrastDelay<5)?pDelay.ContrastDelay:0;
            return ret;
        }
        default:
            return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AYuv::setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr)
{
    //ERROR_CHECK(IspTuningMgr::getInstance().setIspProfile(eIspProfile))
    //ERROR_CHECK(IspTuningMgr::getInstance().validate())
    MY_LOG("[%s] IspProfile(%d), Magic#(%d)", __FUNCTION__, rParamIspProfile.eIspProfile, rParamIspProfile.i4MagicNum);
    return validate(rParamIspProfile, MFALSE);
}

ESoftwareScenario
Hal3AYuv::
scenarioMapping(EIspProfile_T eIspProfile)
{
    // check software scenario
    ESoftwareScenario eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;

    switch(eIspProfile)
    {
    case EIspProfile_Preview:
    case EIspProfile_Video:
    // iHDR
    case EIspProfile_IHDR_Preview:
    case EIspProfile_IHDR_Video:
    case EIspProfile_MHDR_Preview:
    case EIspProfile_MHDR_Video:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Normal_Stream;
        break;
    case EIspProfile_Capture:
    case EIspProfile_Capture_SWNR:
    case EIspProfile_Capture_MultiPass_ANR_1:
    case EIspProfile_Capture_MultiPass_ANR_2:
    case EIspProfile_MHDR_Capture:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Normal_Capture;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Normal_Capture;
        break;
    case EIspProfile_ZSD_Capture:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_ZSD_Capture;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_ZSD_Capture;
        break;
    case EIspProfile_VSS_Capture:
    case EIspProfile_VSS_Capture_SWNR:
    case EIspProfile_VSS_Capture_MultiPass_ANR_1:
    case EIspProfile_VSS_Capture_MultiPass_ANR_2:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_VSS_Capture;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_VSS_Capture;
        break;
    case EIspProfile_PureRAW_Capture:
    case EIspProfile_PureRAW_Capture_SWNR:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Pure_Raw_Stream;
        else if (m_i4SensorDev == ESensorDev_Sub)
            eSoftwareScenario = eSoftwareScenario_Sub_Pure_Raw_Stream;
        else // FIXME: main2
            eSoftwareScenario = eSoftwareScenario_Main_Pure_Raw_Stream;
        break;
    // N3D
    case EIspProfile_N3D_Preview:
    case EIspProfile_N3D_Video:
    case EIspProfile_N3D_Capture:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Normal_Stream;
        else
            eSoftwareScenario = eSoftwareScenario_Main2_N3D_Stream;
        break;
    // MFB
    case EIspProfile_MFB_Capture_EE_Off:
    case EIspProfile_MFB_Capture_EE_Off_SWNR:
    case EIspProfile_MFB_PostProc_EE_Off:
    case EIspProfile_MFB_PostProc_ANR_EE:
    case EIspProfile_MFB_PostProc_ANR_EE_SWNR:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Mfb_Capture;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Capture;
        break;
    case EIspProfile_MFB_Blending_All_Off:
    case EIspProfile_MFB_Blending_All_Off_SWNR:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Mfb_Blending;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Blending;
        break;
    case EIspProfile_MFB_PostProc_Mixing:
    case EIspProfile_MFB_PostProc_Mixing_SWNR:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Mfb_Mixing;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Mfb_Mixing;
        break;
    case EIspProfile_VSS_MFB_PostProc_Mixing:
    case EIspProfile_VSS_MFB_PostProc_Mixing_SWNR:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_VSS_Mfb_Mixing;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_VSS_Mfb_Mixing;
        break;
    // vFB
    case EIspProfile_VFB_PostProc:
        if (m_i4SensorDev == ESensorDev_Main)
            eSoftwareScenario = eSoftwareScenario_Main_Vfb_Stream_2;
        else
            eSoftwareScenario = eSoftwareScenario_Sub_Vfb_Stream_2;
        break;

    default:
        MY_ERR("[%s]Incorect ISP profile (%d)", __FUNCTION__, eIspProfile);
        break;
    }

    return eSoftwareScenario;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL
Hal3AYuv::
validate(const ParamIspProfile_T& rParamIspProfile_, MBOOL fgPerframe)
{
    Mutex::Autolock lock(m_ValidateLock);

    ParamIspProfile_T rParamIspProfile = rParamIspProfile_;
    //queryTGInfoFromSensorHal();//get TG info before setIspProfile

    ESoftwareScenario eSoftwareScenario = scenarioMapping(rParamIspProfile.eIspProfile);
    TuningMgr::getInstance().byPassSetting(eSoftwareScenario, rParamIspProfile.i4MagicNum);

    if (rParamIspProfile.iValidateOpt != ParamIspProfile_T::EParamValidate_None)
    {
        if (rParamIspProfile.iValidateOpt == ParamIspProfile_T::EParamValidate_All)
        {
            configUpdate(m_TgInfo, rParamIspProfile.i4MagicNum);
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AYuv::
on3AProcFinish(MINT32 i4MagicNum)
{
/*
    // result
    m_rResult.i4FrmId = i4MagicNum;
    //m_rResult.i4PrecaptureId

    //m_rResult.eAfState = (EAfState_T) IAfMgr::getInstance().getAFState();
    //m_rResult.eAeState = (EAeState_T) IAeMgr::getInstance()->getAeState();
    //m_rResult.eAwbState = (EAwbState_T) IAwbMgr::getInstance().getAWBState(m_i4SensorDev);

    m_rResultBuf.updateResult(m_rResult.i4FrmId, m_rResult);
*/
    // TODO: send 3A finish
    MUINT32 fgOK = MTRUE;
    MUINT32 fgAeAwbReady = m_i4InitReadyCnt == 0; //MTRUE;
    MUINT32 u4Notify =
        (fgOK << I3ACallBack::e3AProcOK) |
        (fgAeAwbReady << I3ACallBack::e3APvInitReady);

//#if NOTIFY_3A_DONE
    m_cbSet.doNotifyCb(
        I3ACallBack::eID_NOTIFY_3APROC_FINISH,
        /*m_rResult.i4FrmId*/i4MagicNum,
        u4Notify,
        0);
//#endif
}

/******************************************************************************
*
*******************************************************************************/
MRESULT
Hal3AYuv::
configUpdate(MUINT32 u4TgInfo, MINT32 i4MagicNum)
{
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

using namespace NSImageio;
using namespace NSIspio;

    switch (u4TgInfo)
    {
        case CAM_TG_1: u4TgInfo = EPortIndex_TG1I; break;
        case CAM_TG_2: u4TgInfo = EPortIndex_TG2I; break;
        case CAM_SV_1: u4TgInfo = EPortIndex_CAMSV_TG1I; break;
        case CAM_SV_2: u4TgInfo = EPortIndex_CAMSV_TG2I; break;
        default:
            MY_ERR("[%s] Err tg idx(%d) err, force using EPortIndex_TG1I", __FUNCTION__, u4TgInfo);
            u4TgInfo = EPortIndex_TG1I;
            break;
    }

    MRESULT result = S_3A_OK;
#if 1 //ISP_P1_UPDATE
    INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorOpenIdx, "aaa_hal_yuv");
    if ( MFALSE == pPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_P1_UPDATE, u4TgInfo, i4MagicNum, 0) )
    {
        MY_ERR("[%s] sensorIdx(%d) tg(%d) magic(%d)", __FUNCTION__, m_i4SensorOpenIdx, u4TgInfo, i4MagicNum);
        result = E_3A_ERR;
    }
    pPipe->destroyInstance("aaa_hal_yuv");
#endif
    return result;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::doAFUpdate(void)
{
    MINT32 af_status;

    Mutex::Autolock lock(m_LockAF);

    MBOOL fgLogEn = (m_u4LogEn & HAL3AYUV_LOG_AFUPDATE) ? MTRUE : MFALSE;

    MY_LOG_IF(fgLogEn, "[%s +]", __FUNCTION__);

    if (0 == m_fgIsDummyLens && m_max_af_areas > 0)
    {
        if (m_fgAfTrig && m_fgAfTrigMode == 1)
        {
            m_fgAfTrig = 0;
            MY_LOG("[%s] Trigger AF Start.", __FUNCTION__);
            setAFLampOnOff(MTRUE);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)m_AFzone,0,0);
            m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_SINGLE_FOCUS_MODE,0,0,0);
            m_i4AutoFocus = E_YUV_SAF_FOCUSING;
            m_i4AutoFocusTimeout = 30;
        }

        if (m_i4AutoFocus == E_YUV_SAF_FOCUSING)
        {
            af_status = isFocused();

            if (af_status == SENSOR_AF_FOCUSED)
            {
                MY_LOG("[%s] SAF(SENSOR_AF_FOCUSED)\n", __FUNCTION__);
                m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
                m_i4AutoFocus = E_YUV_SAF_DONE;
                setAFLampOnOff(MFALSE);
            }
            else if (m_i4AutoFocusTimeout == 0)
            {
                MY_LOG("[%s] SAF(TimeOut)\n", __FUNCTION__);
                m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
                m_i4AutoFocus = E_YUV_SAF_DONE;
                setAFLampOnOff(MFALSE);
            }

            m_i4AutoFocusTimeout = m_i4AutoFocusTimeout > 0 ? m_i4AutoFocusTimeout - 1 : 0;
        }
        else if (m_i4AutoFocus == E_YUV_SAF_INCAF)
        {
            MY_LOG("[%s] autofocus callback in conti mode", __FUNCTION__);
            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, 0, 0, 0);
            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
            m_i4AutoFocus = E_YUV_SAF_DONE;
        }
        else
        {
            if (m_rParam.u4AfMode == AF_MODE_AFC || m_rParam.u4AfMode == AF_MODE_AFC_VIDEO)
            {
                if (m_i4AFSwitchCtrl > 0)
                {
                    m_i4AFSwitchCtrl--;
                }
                else if (m_i4AFSwitchCtrl == 0)
                {
                    MY_LOG("[%s] Send CAF CMD\n", __FUNCTION__);
                    m_i4FDFrmCnt = 0;
                    resetAFAEWindow();
                    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_SET_YUV_CONSTANT_AF,0,0,0);
                    m_i4AFSwitchCtrl = -1;
                    m_i4PreAfStatus = SENSOR_AF_STATUS_MAX;
                }
                else //(m_i4AFSwitchCtrl == -1)
                {
                    af_status = isFocused();

                    if (m_i4PreAfStatus != af_status)
                    {
                        if (af_status == SENSOR_AF_FOCUSED)
                        {
                            MY_LOG("[%s] CAF(SENSOR_AF_FOCUSED)\n", __FUNCTION__);
                            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, 0, 0, 0);
                            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
                        }
                        else
                        {
                            MY_LOG("[%s] CAF(%d)\n", __FUNCTION__, af_status);
                            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, (af_status == SENSOR_AF_FOCUSING), 0, 0);
                            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 0, 0, 0);
                        }
                    }

                    m_i4PreAfStatus = af_status;

                    if (m_i4WinState == E_YUV_WIN_STATE_RESET_IN_CAF)
                    {
                        // reset window when FD off.
                        MY_LOG("[%s] Leave FD and reset window\n", __FUNCTION__);
                        m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
                        resetAFAEWindow();
                    }
                }
            }
        }
    }
    else
    {
        if (m_i4AutoFocus)
        {
            MY_LOG("[%s] AF not support\n", __FUNCTION__);
            m_i4AutoFocus = E_YUV_SAF_DONE;
            m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, 1, 0, 0);
        }
    }

    MY_LOG_IF(fgLogEn, "[%s -]\n", __FUNCTION__);

    return 0;
}

MRESULT Hal3AYuv::EnableAFThread(MINT32 a_bEnable)
{
    MRESULT ret = S_3A_OK;

    if (a_bEnable)
    {
        if (m_bAFThreadLoop == 0)
        {
#if 0
            m_pIspDrv = IspDrv::createInstance();

            if (!m_pIspDrv)
            {
                MY_ERR("IspDrv::createInstance() fail \n");
                return E_3A_NULL_OBJECT;
            }

            if (m_pIspDrv->init() < 0)
            {
                MY_ERR("pIspDrv->init() fail \n");
                return E_3A_ERR;
            }
#endif
            // create AF thread
            MY_LOG("[AFThread] Create");
            m_bAFThreadLoop = 1;
            ::sem_init(&m_semAFThreadStart, 0, 0);
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_AF};
            pthread_create(&m_AFThread, &attr, AFThreadFunc, this);
        }
    }
    else
    {
        if (m_bAFThreadLoop == 1)
        {
        #if 0
            if (m_pIspDrv)
            {
                m_pIspDrv->uninit();
                m_pIspDrv = NULL;
            }
        #endif
            m_bAFThreadLoop = 0;
            ::sem_post(&m_semAFThreadStart);

            pthread_join(m_AFThread, NULL);

            MY_LOG("[AFThread] Delete");
        }
    }

    return ret;

}

MVOID * Hal3AYuv::AFThreadFunc(void *arg)
{
    MY_LOG("[%s] tid: %d \n", __FUNCTION__, gettid());
    ::prctl(PR_SET_NAME,"AFthread_YUV", 0, 0, 0);

    Hal3AYuv* p3Ayuv = reinterpret_cast<Hal3AYuv*>(arg);
#if 0
    if (!p3Ayuv->m_pIspDrv)
    {
        MY_LOG("[%s] m_pIspDrv null\n", __FUNCTION__);
        return NULL;
    }
#endif

    while (p3Ayuv->m_bAFThreadLoop)
    {
        ::sem_wait(&p3Ayuv->m_semAFThreadStart);
        //if (p3Ayuv->m_pIspDrv->waitIrq(WaitIrq) >= 0) // success
        {
            p3Ayuv->doAFUpdate();
        }
        //else
        //{
        //    MY_LOG("[%s] AF irq timeout\n", __FUNCTION__);
        //}
    }

    MY_LOG("[%s] End \n", __FUNCTION__);

    return NULL;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32
Hal3AYuv::
addCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s] cb(%p)", __FUNCTION__, cb);
    return m_cbSet.addCallback(cb);
}

MINT32
Hal3AYuv::
removeCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s] cb(%p)", __FUNCTION__, cb);
    return m_cbSet.removeCallback(cb);
}

//******************************************************************************
// Map AE exposure to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapAEToEnum(MINT32 mval,MFLOAT mstep)
{
    MINT32 pEv,ret;

    pEv = 100 * mval * mstep;

    if     (pEv <-250) { ret = AE_EV_COMP_n30;}  // EV compensate -3.0
    else if(pEv <-200) { ret = AE_EV_COMP_n25;}  // EV compensate -2.5
    else if(pEv <-170) { ret = AE_EV_COMP_n20;}  // EV compensate -2.0
    else if(pEv <-160) { ret = AE_EV_COMP_n17;}  // EV compensate -1.7
    else if(pEv <-140) { ret = AE_EV_COMP_n15;}  // EV compensate -1.5
    else if(pEv <-130) { ret = AE_EV_COMP_n13;}  // EV compensate -1.3
    else if(pEv < -90) { ret = AE_EV_COMP_n10;}  // EV compensate -1.0
    else if(pEv < -60) { ret = AE_EV_COMP_n07;}  // EV compensate -0.7
    else if(pEv < -40) { ret = AE_EV_COMP_n05;}  // EV compensate -0.5
    else if(pEv < -10) { ret = AE_EV_COMP_n03;}  // EV compensate -0.3
    else if(pEv ==  0) { ret = AE_EV_COMP_00; }  // EV compensate -2.5
    else if(pEv <  40) { ret = AE_EV_COMP_03; }  // EV compensate  0.3
    else if(pEv <  60) { ret = AE_EV_COMP_05; }  // EV compensate  0.5
    else if(pEv <  90) { ret = AE_EV_COMP_07; }  // EV compensate  0.7
    else if(pEv < 110) { ret = AE_EV_COMP_10; }  // EV compensate  1.0
    else if(pEv < 140) { ret = AE_EV_COMP_13; }  // EV compensate  1.3
    else if(pEv < 160) { ret = AE_EV_COMP_15; }  // EV compensate  1.5
    else if(pEv < 180) { ret = AE_EV_COMP_17; }  // EV compensate  1.7
    else if(pEv < 210) { ret = AE_EV_COMP_20; }  // EV compensate  2.0
    else if(pEv < 260) { ret = AE_EV_COMP_25; }  // EV compensate  2.5
    else if(pEv < 310) { ret = AE_EV_COMP_30; }  // EV compensate  3.0
    else               { ret = AE_EV_COMP_00;}

    MY_LOG("[%s()]EV:(%d),Ret:(%d)\n", __FUNCTION__, pEv,ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapISOToEnum(MUINT32 u4NewAEISOSpeed)
{
    MINT32 ret;

    switch(u4NewAEISOSpeed){
        case 0:
            ret = AE_ISO_AUTO;
            break;
        case 100:
            ret = AE_ISO_100;
            break;
        case 200:
            ret = AE_ISO_200;
            break;
        case 400:
            ret = AE_ISO_400;
            break;
        case 800:
             ret = AE_ISO_800;
           break;
        case 1600:
            ret = AE_ISO_1600;
           break;
        default:
            MY_LOG("The iso enum value is incorrectly:%d\n", u4NewAEISOSpeed);
            ret = AE_ISO_AUTO;
            break;
    }
    MY_LOG("[%s()]ISOVal:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEISOSpeed, ret);

    return ret;
}

//******************************************************************************
// Map AE ISO to Enum
//******************************************************************************
MINT32 Hal3AYuv::mapEnumToISO(MUINT32 u4NewAEIsoEnum) const
{
    MINT32 ret;

    switch(u4NewAEIsoEnum){
        case AE_ISO_AUTO:
            ret = 100;
            break;
        case AE_ISO_100:
            ret = 100;
            break;
        case AE_ISO_200:
            ret = 200;
            break;
        case AE_ISO_400:
            ret = 400;
            break;
        case AE_ISO_800:
             ret = 800;
           break;
        case AE_ISO_1600:
            ret = 1600;
           break;
        default:
            ret = 100;
            break;
    }
    MY_LOG("[%s()]ISOEnum:(%d),Ret:(%d)\n", __FUNCTION__, u4NewAEIsoEnum, ret);

    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getExposureInfo(ExpSettingParam_T &strHDRInputSetting)
{
#if 0
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    return IAeMgr::getInstance().getExposureInfo(strHDRInputSetting);
}
#endif
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::getCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    CaptureParam_T rCaptureInfo;
    MINT32 yuvCmd = 0;
    MINT32 yuvParam = 0;
#if 0
    if (m_rParam.u4CamMode == eAppMode_FactoryMode){
         yuvCmd = YUV_AUTOTEST_GET_SHUTTER_RANGE;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvShutterRange = yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_SHADDING;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvShading = yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_GAMMA;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvGamma= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_SHUTTER;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvShutter= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_GAIN;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvGain= yuvParam;

         yuvCmd = YUV_AUTOTEST_GET_AE;
         m_pSensorHal->sendCommand(static_cast<halSensorDev_e>(m_i4SensorDev),SENSOR_CMD_SET_YUV_AUTOTEST,(int)&yuvCmd,(int)&yuvParam,0);
         a_rCaptureInfo.u4YuvAE= yuvParam;

         MY_LOG("[Factory]getCaptureParams shading(%d),Gamma(%d),AE(%d),Shutter(%d),Gain(%d) \n",
               a_rCaptureInfo.u4YuvShading,a_rCaptureInfo.u4YuvGamma,a_rCaptureInfo.u4YuvAE,
               a_rCaptureInfo.u4YuvGain,a_rCaptureInfo.u4YuvShutter);
    }
    else

    {
        if (i4EVidx == 0 && index == 0)
        {
            a_rCaptureInfo.u4YuvAE = 0;
        }
        else if (i4EVidx == 0)
        {
            MUINT32 u4EVIndex[3] = {-2, 0, 2};
            index = ((index > 2) ? 2 : (index < 0 ? 0 : index));
            a_rCaptureInfo.u4YuvAE = u4EVIndex[index];
        }
        else
        {
            a_rCaptureInfo.u4YuvAE = i4EVidx;
        }
        MY_LOG("[%s] index(%d) i4EVidx(%d) u4YuvAE(%d)", __FUNCTION__, index, i4EVidx, a_rCaptureInfo.u4YuvAE);
    }
#endif
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AYuv::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    MINT32 yuvParam = 0;

    MY_LOG("[%s] EvIdx(%d), OriginalIdx(%d), CompStep(%3.6f)", __FUNCTION__, a_rCaptureInfo.i4YuvEvIdx, m_rParam.i4ExpIndex, m_rParam.fExpCompStep);

    yuvParam = a_rCaptureInfo.i4YuvEvIdx; //mapAEToEnum(a_rCaptureInfo.i4YuvEvIdx, m_rParam.fExpCompStep);
    setYuvFeature(FID_AE_EV, yuvParam);

    return S_3A_OK;
}

MINT32 Hal3AYuv::setFlashMode(MINT32 mode)
{
    MY_LOG("[%s] mode=%d\n", __FUNCTION__, mode);

    if (m_pStrobeDrvObj)
    {
        if (mode<LIB3A_FLASH_MODE_MIN || mode>LIB3A_FLASH_MODE_MAX)
        {
            //return FL_ERR_FlashModeNotSupport;
            MY_LOG("FL_ERR_FlashModeNotSupport\n");
        }
        else
        {
            if (m_rParam.u4StrobeMode == FLASHLIGHT_TORCH && mode != (MINT32) m_rParam.u4StrobeMode) //prviouw mode is torch. and change to another mode.
            {
                m_pStrobeDrvObj->setOnOff(0);
                MY_LOG("FLASHLIGHT_TORCH OFF\n");
            }

            if (mode == FLASHLIGHT_TORCH)
            {
                m_pStrobeDrvObj->setDuty(m_u4StrobeDuty);
                m_pStrobeDrvObj->setStep(m_rYuvFlParam.i4FlashlightStep);
                m_pStrobeDrvObj->setTimeOutTime(0);
                m_pStrobeDrvObj->setOnOff(0);
                m_pStrobeDrvObj->setOnOff(1);
                MY_LOG("FLASHLIGHT_TORCH ON\n");
            }
            else if(mode==FLASHLIGHT_FORCE_OFF)
            {
                m_pStrobeDrvObj->setTimeOutTime(1000);
                m_pStrobeDrvObj->setOnOff(0);
                MY_LOG("FLASHLIGHT_FORCE_OFF\n");
            }
        }
    }
    else
    {
        MY_LOG("No Strobe!\n");
    }

    return S_3A_OK;
}

MINT32 Hal3AYuv::updateAeFlashCaptureParams()
{
    MINT32 i4StrobeGain = 0;
    MINT32 i4Shutter = 0, i4CfgGain = 0, i4Gain = 0;
    MINT32 i4StrobeWidth = 0;
    MINT32 i4HighcurrentTimeout = 0;

    MINT32 i4TestShutter = 0;
    MINT32 i4TestGain = 0;
    GET_PROP("debug.aaa_hal_yuv.shutter", "-1", i4TestShutter);
    GET_PROP("debug.aaa_hal_yuv.gain", "-1", i4TestGain);

    if (m_pStrobeDrvObj && m_u4StrobeDuty >0 && m_bFlashActive == MTRUE)
    {
        i4StrobeGain = m_rYuvFlParam.i4FlashlightGain10X;
        MY_LOG("[%s] i4StrobeGain(%d), m_fNoFlashBV(%f), m_fPreFlashBV(%f), m_i4PreFlashShutter(%d), m_i4PreFlashGain(%d)",
            __FUNCTION__, i4StrobeGain, m_fNoFlashBV, m_fPreFlashBV, m_i4PreFlashShutter, m_i4PreFlashGain);
        convertFlashExpPara(
            i4StrobeGain, m_rAeInfo.GAIN_BASE,
            m_fNoFlashBV*1024, m_fPreFlashBV*1024,
            m_i4PreFlashShutter, m_i4PreFlashGain, 1024,
            i4Shutter, i4Gain, i4CfgGain);

        m_fNoFlashBV = 0.0;
        i4StrobeWidth = m_u4StrobeDuty;
        if (i4StrobeGain > 10 && m_rParam.u4ShotMode != CAPTURE_MODE_BURST_SHOT)
        {
            MY_LOG("[%s] High current mode, i4Shutter(%d), i4Gain(%d)", __FUNCTION__, i4Shutter, i4Gain);
            // strobe led driver should implement 0xff as 2x.
            i4StrobeWidth = m_rYuvFlParam.i4FlashlightHighCurrentDuty;
            i4HighcurrentTimeout = m_rYuvFlParam.i4FlashlightHighCurrentTimeout;
            // set to sensor
            if (i4TestShutter != -1)
                i4Shutter = i4TestShutter;
            if (i4TestGain != -1)
                i4Gain = i4TestGain;
            setEShutterParam(i4Shutter, i4Gain);
        }

        if (m_pStrobeDrvObj->setTimeOutTime(i4HighcurrentTimeout) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setTimeOutTime(%d) ms", __FUNCTION__, i4HighcurrentTimeout);
        }
        if (m_pStrobeDrvObj->setDuty(i4StrobeWidth) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setLevel(%d)", __FUNCTION__, i4StrobeWidth);
        }
        if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
        {
            MY_LOG("[%s] setFire ON", __FUNCTION__);
        }
    }

    return S_3A_OK;
}

MINT32 Hal3AYuv::setAFMode(MINT32 AFMode)
{
    switch (AFMode)
    {
    case AF_MODE_AFS:
        MY_LOG("[%s] AF_MODE_AFS", __FUNCTION__);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        break;
    case AF_MODE_INFINITY:
    {
        MY_LOG("[%s] AF_MODE_INFINITY", __FUNCTION__);
        int iYuv3ACmd = SENSOR_3A_AF_INFINITY;
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_CANCEL_AF,0,0,0);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_3A_CMD,(MUINTPTR)&iYuv3ACmd,0,0);
    }
        break;
    case AF_MODE_AFC:
    case AF_MODE_AFC_VIDEO:
        MY_LOG("[%s] AF_MODE_AFC(%d)", __FUNCTION__, AFMode);
        m_i4AFSwitchCtrl = getDelayFrame(EQueryType_AF);
        MY_LOG("[%s] m_i4AFSwitchCtrl(%d)", __FUNCTION__, m_i4AFSwitchCtrl);
        break;
    default:
        break;
    }
    return S_3A_OK;
}

MINT32 Hal3AYuv::isFocused()
{
    MINT32 err = MHAL_NO_ERROR;
    MINT32 i4AfStatus = m_i4AfStatus;

    if (m_fgIsDummyLens)
    {
        return SENSOR_AF_FOCUSED;
    }

    if (SENSOR_AF_SCENE_DETECTING == i4AfStatus)
        i4AfStatus = SENSOR_AF_FOCUSING;

    return i4AfStatus;
}

MINT32
Hal3AYuv::
get3AStatusFromSensor()
{
    MBOOL fgLog = (m_u4LogEn & HAL3AYUV_LOG_3ASTA) ? MTRUE : MFALSE;

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AF_STATUS, (MUINTPTR)&m_i4AfStatus, 0, 0);
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AE_STATUS, (MUINTPTR)&m_i4AeStatus, 0, 0);
    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_AWB_STATUS, (MUINTPTR)&m_i4AwbStatus, 0, 0);

    MY_LOG_IF(fgLog, "[%s] AF(%d), AE(%d), AWB(%d)", __FUNCTION__, m_i4AfStatus, m_i4AeStatus, m_i4AwbStatus);

    return 0;
}


MINT32 Hal3AYuv::clamp(MINT32 x, MINT32 min, MINT32 max)
{
    if (x > max) return max;
    if (x < min) return min;
    return x;
}

MVOID Hal3AYuv::mapAeraToZone(
    AREA_T *p_area, MINT32 areaW,
    MINT32 areaH, MINT32* p_zone,
    MINT32 zoneW, MINT32 zoneH)
{

    MINT32 left, top, right, bottom;

    p_area->i4Left   = clamp(p_area->i4Left,   0, areaW-1);
    p_area->i4Right  = clamp(p_area->i4Right,  0, areaW-1);
    p_area->i4Top    = clamp(p_area->i4Top,    0, areaH-1);
    p_area->i4Bottom = clamp(p_area->i4Bottom, 0, areaH-1);

    left   = p_area->i4Left   * zoneW / areaW;
    right  = p_area->i4Right  * zoneW / areaW;
    top    = p_area->i4Top    * zoneH / areaH;
    bottom = p_area->i4Bottom * zoneH / areaH;

    *p_zone     = clamp(left,   0, zoneW-1);
    *(p_zone+1) = clamp(top,    0, zoneH-1);
    *(p_zone+2) = clamp(right,  0, zoneW-1);
    *(p_zone+3) = clamp(bottom, 0, zoneH-1);
    *(p_zone+4) = zoneW;
    *(p_zone+5) = zoneH;

    MY_LOG("[%s] LTRBWH(%d,%d,%d,%d,%d,%d) -> LTRBWH(%d,%d,%d,%d,%d,%d)", __FUNCTION__,
        p_area->i4Left, p_area->i4Top, p_area->i4Right, p_area->i4Bottom, areaW, areaH,
        *p_zone, *(p_zone+1), *(p_zone+2), *(p_zone+3), *(p_zone+4), *(p_zone+5));
}

MVOID Hal3AYuv::setFocusAreas(MINT32 a_i4Cnt, AREA_T *a_psFocusArea)
{
    MY_LOG("[%s] +", __FUNCTION__);

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_af_areas))
    {
        return ;
    }
    else  // spot or matrix meter
    {
        m_sAFAREA[0] = *a_psFocusArea;

        m_sAFAREA[0].i4Left   = clamp(m_sAFAREA[0].i4Left   + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAFAREA[0].i4Right  = clamp(m_sAFAREA[0].i4Right  + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAFAREA[0].i4Top    = clamp(m_sAFAREA[0].i4Top    + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        m_sAFAREA[0].i4Bottom = clamp(m_sAFAREA[0].i4Bottom + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        MY_LOG("[%s] LTRB(%d,%d,%d,%d)", __FUNCTION__,
            m_sAFAREA[0].i4Left, m_sAFAREA[0].i4Top, m_sAFAREA[0].i4Right, m_sAFAREA[0].i4Bottom);
        mapAeraToZone(&m_sAFAREA[0], ANDR_IMG_WD, ANDR_IMG_HT, &m_AFzone[0], YUV_IMG_WD, YUV_IMG_HT);
    }

    MY_LOG("[%s] -", __FUNCTION__);
}

MVOID Hal3AYuv::getFocusAreas(MINT32 &a_i4Cnt, AREA_T **a_psFocusArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psFocusArea = &m_sAFAREA[0];
}

MVOID Hal3AYuv::getMeteringAreas(MINT32 &a_i4Cnt, AREA_T **a_psAEArea)
{
    MY_LOG("[AF][%s()] \n", __FUNCTION__);

    a_i4Cnt = 1;
    *a_psAEArea = &m_sAEAREA[0];
}

MVOID Hal3AYuv::setMeteringAreas(MINT32 a_i4Cnt, AREA_T const *a_psAEArea)
{
    MINT32 err = MHAL_NO_ERROR;
    MUINT32* zone_addr = (MUINT32*)&m_AEzone[0];

    MY_LOG("[%s] +", __FUNCTION__);

    if ((a_i4Cnt == 0) || (a_i4Cnt > m_max_metering_areas))
    {
        return;
    }
    else  // spot or matrix meter
    {
        m_sAEAREA[0] = *a_psAEArea;

        m_sAEAREA[0].i4Left   = clamp(m_sAEAREA[0].i4Left   + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAEAREA[0].i4Right  = clamp(m_sAEAREA[0].i4Right  + ANDR_IMG_WD/2, 0, ANDR_IMG_WD-1);
        m_sAEAREA[0].i4Top    = clamp(m_sAEAREA[0].i4Top    + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);
        m_sAEAREA[0].i4Bottom = clamp(m_sAEAREA[0].i4Bottom + ANDR_IMG_HT/2, 0, ANDR_IMG_HT-1);

        MY_LOG("[%s] LTRB(%d,%d,%d,%d)", __FUNCTION__,
            m_sAEAREA[0].i4Left, m_sAEAREA[0].i4Top, m_sAEAREA[0].i4Right, m_sAEAREA[0].i4Bottom);

        mapAeraToZone(&m_sAEAREA[0], ANDR_IMG_WD, ANDR_IMG_HT, &m_AEzone[0], YUV_IMG_WD, YUV_IMG_HT);
        MY_LOG("[%s] zone_addr(0x%08x)", __FUNCTION__, zone_addr);
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AE_WINDOW,(MUINTPTR)zone_addr,0,0);
    }

    MY_LOG("[%s] -", __FUNCTION__);
}

MVOID
Hal3AYuv::
setFDEnable(MBOOL bEnable)
{
    Mutex::Autolock lock(m_LockAF);

    if (bEnable)
    {
        MY_LOG("[%s] Start FD", __FUNCTION__);
        m_fgFDEnable = MTRUE;
        m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
    }
    else
    {
        MY_LOG("[%s] Stop FD", __FUNCTION__);
        m_fgFDEnable = MFALSE;
        //m_i4WinState = 2;
    }
}

MBOOL Hal3AYuv::setFDInfo(MVOID* a_sFaces)
{
    MINT32 i4Cnt;
    AREA_T rFDArea;
    MtkCameraFaceMetadata *pFaces;

    MBOOL fgLog = (m_u4LogEn & HAL3AYUV_LOG_FDUPDATE) ? MTRUE : MFALSE;

    Mutex::Autolock lock(m_LockAF);

    pFaces = (MtkCameraFaceMetadata *)a_sFaces;

    if (!m_fgFDEnable && m_i4WinState!=E_YUV_WIN_STATE_RESET_IN_CAF)
    {
        MY_LOG("[%s] FD is disabled, reset window", __FUNCTION__);
        m_i4WinState = E_YUV_WIN_STATE_RESET_IN_CAF;
        return MFALSE;
    }

    if (pFaces == NULL)
    {
        MY_LOG("[%s] Leave", __FUNCTION__);
        m_i4WinState = E_YUV_WIN_STATE_RESET_IN_CAF;
        return MTRUE;
    }

    i4Cnt = pFaces->number_of_faces;
    MY_LOG_IF(fgLog, "[%s] Faces(%d)", __FUNCTION__, pFaces->number_of_faces);
#if 0
    if (i4Cnt)
    {
        rFDArea.i4Left   = pFaces->faces->rect[0];
        rFDArea.i4Top    = pFaces->faces->rect[1];
        rFDArea.i4Right  = pFaces->faces->rect[2];
        rFDArea.i4Bottom = pFaces->faces->rect[3];

        if (rFDArea.i4Right == rFDArea.i4Left || rFDArea.i4Bottom == rFDArea.i4Top)
        {
            i4Cnt = 0;
        }
        MY_LOG_IF(fgLog, "[%s] Faces(%d), LTRB(%d,%d,%d,%d)", __FUNCTION__,
            pFaces->number_of_faces, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);

        if (i4Cnt != 0)
        {
            if (m_max_af_areas > 0)
            {
                setFocusAreas(i4Cnt, &rFDArea);
                m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(int)m_AFzone,0,0);
            }
            if (m_max_metering_areas > 0)
            {
                setMeteringAreas(i4Cnt, &rFDArea);
            }
        }
    }
#else
    if (i4Cnt)
    {
        m_i4FDFrmCnt -= (m_i4FDFrmCnt > 0 ? 1 : 0);

        rFDArea.i4Left   = pFaces->faces->rect[0];
        rFDArea.i4Top    = pFaces->faces->rect[1];
        rFDArea.i4Right  = pFaces->faces->rect[2];
        rFDArea.i4Bottom = pFaces->faces->rect[3];

        if (rFDArea.i4Right == rFDArea.i4Left || rFDArea.i4Bottom == rFDArea.i4Top)
        {
            i4Cnt = 0;
        }
        MY_LOG_IF(fgLog, "[%s] Faces(%d), LTRB(%d,%d,%d,%d)", __FUNCTION__,
            pFaces->number_of_faces, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);
    }
    else
    {
        m_i4FDFrmCnt += (m_i4FDFrmCnt < 8 ? 1 : 0);
    }

    MY_LOG_IF(fgLog, "[%s] m_i4FDFrmCnt(%d)", __FUNCTION__, m_i4FDFrmCnt);

    if (m_i4FDFrmCnt < 3)
    {
        m_i4FDApplyCnt -= (m_i4FDApplyCnt > 0 ? 1 : 0);
        if (m_i4FDApplyCnt == 0)
        {
            m_i4WinState = E_YUV_WIN_STATE_RESET_IN_FD;
            m_i4FDApplyCnt = 3;
            if (i4Cnt != 0)
            {
                MY_LOG_IF(fgLog, "[%s] number_of_faces(%d), LTRB(%d,%d,%d,%d)\n",
                __FUNCTION__, i4Cnt, rFDArea.i4Left, rFDArea.i4Top, rFDArea.i4Right, rFDArea.i4Bottom);

                if (m_max_af_areas > 0)
                {
                    setFocusAreas(i4Cnt, &rFDArea);
                    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)m_AFzone,0,0);
                }
                if (m_max_metering_areas > 0)
                {
                    setMeteringAreas(i4Cnt, &rFDArea);
                }
            }
        }
    }
    else
    {
        m_i4FDApplyCnt = 3;
        if (m_i4WinState == E_YUV_WIN_STATE_RESET_IN_FD)
        {
            m_i4WinState = E_YUV_WIN_STATE_NO_ACTIVITY;
            resetAFAEWindow();
        }
    }
#endif

    return MTRUE;
}

MBOOL Hal3AYuv::resetAFAEWindow()
{
    MINT32 ai4Zone[6];

    MY_LOG("[%s]", __FUNCTION__);

    // reset to center point
    ai4Zone[0] = ai4Zone[2] = YUV_IMG_WD/2;
    ai4Zone[1] = ai4Zone[3] = YUV_IMG_HT/2;
    ai4Zone[4] = YUV_IMG_WD;
    ai4Zone[5] = YUV_IMG_HT;

    if (m_max_af_areas > 0)
    {
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AF_WINDOW,(MUINTPTR)ai4Zone,0,0);
    }

    if (m_max_metering_areas > 0)
    {
        m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_YUV_AE_WINDOW,(MUINTPTR)ai4Zone,0,0);
    }

    return MTRUE;
}

MINT32 Hal3AYuv::isNeedFiringFlash(MBOOL bEnCal)
{
    MY_LOG("[%s]", __FUNCTION__);

    MBOOL bFlashOn;

    if (m_rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_OFF)
    {
        MY_LOG("[%s] FLASHLIGHT_FORCE_OFF\n", __FUNCTION__);
        bFlashOn = 0;
    }
    else if (m_rParam.u4StrobeMode == LIB3A_FLASH_MODE_FORCE_ON)
    {
        MY_LOG("[%s] FLASHLIGHT_FORCE_ON\n", __FUNCTION__);
        bFlashOn = 1;
    }
    else //auto
    {
        if (isAEFlashOn())
        {
            MY_LOG("[%s] isAEFlashOn ON\n", __FUNCTION__);
            bFlashOn = 1;
        }
        else
        {
            MY_LOG("[%s] isAEFlashOn OFF\n", __FUNCTION__);
            bFlashOn = 0;
        }
    }

#if defined(DUMMY_FLASHLIGHT)
    MY_LOG("[%s] DUMMY_FLASHLIGHT\n", __FUNCTION__);
    bFlashOn = 0;
#endif

    m_isFlashOnCapture = bFlashOn;

    return bFlashOn;

}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::isAEFlashOn()
{
    MINT32 rtn = 0;
    MUINT32 u4TrigFlashOn;

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_TRIGGER_FLASHLIGHT_INFO, (MUINTPTR)&u4TrigFlashOn,0,0);

    MY_LOG("[%s] m_aeFlashlightType(%d), StrobeMode(%d), u4TrigFlashOn(%d)", __FUNCTION__,
        m_aeFlashlightType,
        m_rParam.u4StrobeMode,
        u4TrigFlashOn);

    if (FLASHLIGHT_LED_PEAK == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType ||
        FLASHLIGHT_LED_CONSTANT == (FLASHLIGHT_TYPE_ENUM)m_aeFlashlightType)
    {
        if ( (LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_FORCE_ON == m_rParam.u4StrobeMode )
        {
            rtn = 1;
        }
        else if ((LIB3A_FLASH_MODE_T)LIB3A_FLASH_MODE_AUTO == m_rParam.u4StrobeMode && u4TrigFlashOn
                 /*(m_fBVThreshold > m_strobeTrigerBV ) */)
        {
            rtn = 1;
        }
    }
    return rtn;
}

MINT32 Hal3AYuv::setAFLampOnOff(MBOOL bOnOff)
{
    MINT32 i4Ret = S_3A_OK;
    MINT32 i4AfLampSupport = m_rYuvFlParam.i4FlashlightAfLampSupport;

    if (m_pStrobeDrvObj && i4AfLampSupport)
    {
        if (m_rParam.u4StrobeMode != FLASHLIGHT_TORCH && !isInVideo() /*m_rParam.u4CamMode != eAppMode_VideoMode*/)
        {
            MY_LOG("[%s] bOnOff(%d), StrobeMode(%d)\n", __FUNCTION__, bOnOff, m_rParam.u4StrobeMode);
            if (bOnOff)
            {
                MBOOL fgFlashOn = isAEFlashOn();
                if (fgFlashOn)
                {
                    //ON flashlight
                    if (m_pStrobeDrvObj->setTimeOutTime(0) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setTimeOutTime: 0\n");
                    }
                    if (m_pStrobeDrvObj->setDuty(m_u4StrobeDuty) == MHAL_NO_ERROR)
                    {
                        MY_LOG("setLevel:%d\n", m_u4StrobeDuty);
                    }
                    if (m_pStrobeDrvObj->setOnOff(1) == MHAL_NO_ERROR)
                    {
                        MY_LOG("[%s] setFire ON\n", __FUNCTION__);
                    }
                }
                else
                {
                    MY_LOG("[%s] No need to turn on AF lamp.\n", __FUNCTION__);
                }
            }
            else
            {
                if (m_pStrobeDrvObj->setOnOff(0) == MHAL_NO_ERROR)
                {
                    MY_LOG("[%s] setFire OFF\n", __FUNCTION__);
                }
            }
        }
        else
        {
            MY_LOG("[%s] StrobeMode(%d), CamMode(%d), skip\n", __FUNCTION__,
                m_rParam.u4StrobeMode, m_rParam.u4CamMode);
        }
    }
    else
    {
        MY_LOG("[%s] strobe object(0x%08x), AfLampSupport(%d)\n", __FUNCTION__, m_pStrobeDrvObj, i4AfLampSupport);
        i4Ret = E_3A_NULL_OBJECT;
    }

    return i4Ret;
}

/*******************************************************************************
*
********************************************************************************/
MDOUBLE Hal3AYuv::calcBV(const SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo)
{
    MDOUBLE  AV=0, TV=0, SV=0, BV=0;
    MINT32 ISO = 0;

    AV  = AEFlashlightLog2((MDOUBLE)rAeFlashlightInfo.u4Fno/10)*2;
    TV  = AEFlashlightLog2(1000000/((MDOUBLE)rAeFlashlightInfo.Exposuretime));
    ISO = rAeFlashlightInfo.Gain * MIN_ISO / rAeFlashlightInfo.GAIN_BASE;
    SV  = AEFlashlightLog2(((MDOUBLE)ISO)/3.125);

    BV = AV + TV - SV ;

    MY_LOG("[%s] AV(%3.6f), TV(%3.6f), ISO(%d), SV(%3.6f), BV(%3.6f)", __FUNCTION__, AV, TV, ISO, SV, BV);

    return (BV);
}

/*******************************************************************************
*
********************************************************************************/
MDOUBLE Hal3AYuv::AEFlashlightLog2(MDOUBLE x)
{
     return log(x)/log((double)2);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::queryAEFlashlightInfoFromSensor(SENSOR_FLASHLIGHT_AE_INFO_STRUCT& rAeFlashlightInfo)
{
    MINT32 err = MHAL_NO_ERROR;

    memset(&rAeFlashlightInfo, 0, sizeof(SENSOR_FLASHLIGHT_AE_INFO_STRUCT));

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_STROBE_INFO,
        (MUINTPTR)&rAeFlashlightInfo, 0, 0);

    if (rAeFlashlightInfo.u4Fno == 0)
    {
        MY_ERR("Fail to get F#, set to 28");
        rAeFlashlightInfo.u4Fno = 28;
    }
    if (rAeFlashlightInfo.Exposuretime > 1000000 || rAeFlashlightInfo.Exposuretime == 0)
    {
        MY_ERR("Fail to get T(%d), set to 1000", rAeFlashlightInfo.Exposuretime);
        rAeFlashlightInfo.Exposuretime = 1000;
    }
    if (rAeFlashlightInfo.GAIN_BASE < MIN_ISO)
    {
        MY_ERR("Fail to get GAIN_BASE(%d), set to %d", rAeFlashlightInfo.GAIN_BASE, MIN_ISO);
        rAeFlashlightInfo.GAIN_BASE = MIN_ISO;
    }
    if (rAeFlashlightInfo.Gain == 0)
    {
        MY_ERR("Fail to get GAIN, set to %d", rAeFlashlightInfo.GAIN_BASE);
        rAeFlashlightInfo.Gain = rAeFlashlightInfo.GAIN_BASE;
    }

    MY_LOG("[%s] u4Fno(%d), Exposuretime(%d), Gain(%d), GAIN_BASE(%d)", __FUNCTION__,
        rAeFlashlightInfo.u4Fno,
        rAeFlashlightInfo.Exposuretime,
        rAeFlashlightInfo.Gain,
        rAeFlashlightInfo.GAIN_BASE);

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::setEShutterParam(
       MUINT32 a_u4ExpTime, MUINT32 a_u4SensorGain)
{
    MINT32 err;

    if ((a_u4ExpTime == 0) || (a_u4SensorGain == 0)) {
        MY_LOG("setExpParam() error: a_u4ExpTime = %d; a_u4SensorGain = %d; \n", a_u4ExpTime, a_u4SensorGain);
        return MHAL_INVALID_PARA;
    }

    MY_LOG("[%s] ExpTime(%d us), SensorGain(%d)\n", __FUNCTION__, a_u4ExpTime, a_u4SensorGain);

    // exposure time in terms of 32us
    a_u4ExpTime = a_u4ExpTime >> 5;
    a_u4SensorGain = a_u4SensorGain << 4;

    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_SENSOR_EXP_LINE,(MUINTPTR)&a_u4ExpTime,0,0);
    m_pIHalSensor->sendCommand(m_i4SensorDev,SENSOR_CMD_SET_SENSOR_GAIN,(MUINTPTR)&a_u4SensorGain,0,0);

    return MHAL_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID Hal3AYuv::endContinuousShotJobs()
{
    if (m_pStrobeDrvObj && m_bFlashActive == MTRUE)
    {
        // force off for burst shot mode
        m_pStrobeDrvObj->setOnOff(0);
        m_bFlashActive = MFALSE;
        MY_LOG("[%s]: setFire OFF\n", __FUNCTION__);
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo)
{
    Hal3A_HDROutputParam_T HDROutputParam;

    HDROutputParam.u4OutputFrameNum = 3;
    HDROutputParam.u4FinalGainDiff[0] = 4096;
    HDROutputParam.u4FinalGainDiff[1] = 256;
    HDROutputParam.u4TargetTone = 150;

    a_strHDROutputInfo = HDROutputParam;

    return S_3A_OK;
}

void Hal3AYuv::convertFlashExpPara(MINT32 flashEngRatio_x10, MINT32 minAfeGain_x1024,
             MINT32 bv0_x1024, MINT32 bv1_x1024,
             MINT32  exp1, MINT32  afeGain1_x1024, MINT32  ispGain1_x1024,
             MINT32& exp2, MINT32& afeGain2_x1024, MINT32& ispGain2_x1024) const
{
    MY_LOG("convertFlashExpParaa ratio=%d minG=%d bv0=%d bv1=%d\n",flashEngRatio_x10, minAfeGain_x1024, bv0_x1024, bv1_x1024);
    MY_LOG("convertFlashExpParaa exp=%d afe=%d isp=%d\n",exp1, afeGain1_x1024, ispGain1_x1024);
    if(minAfeGain_x1024==0)
        minAfeGain_x1024=2048;
    double bv0;
    double bv1;
    double engRatio;
    double delEv;
    double rat;
    bv0 = bv0_x1024/1024.0;
    bv1 = bv1_x1024/1024.0;
    engRatio = flashEngRatio_x10/10.0;


    /*
    double m0;
    double m1;
    double rat2;
    m0 = pow(2, bv0);
    m1 = pow(2, bv1);
    double rr;
    rat2 = ( (m0+(m1-m0)*engRatio)/m1);
    */

    if (bv1 < bv0)
    {
        exp2 = exp1 / engRatio;
        afeGain2_x1024 = afeGain1_x1024;
        ispGain2_x1024 = 1024;
        MY_LOG("[%s] bv1 < bv0!\n", __FUNCTION__);
            return;
    }

    rat = ((pow(2, bv1-bv0)-1)*engRatio +1)*pow(2, bv0-bv1);

    double maxGainRatio=1;
    if(afeGain1_x1024>minAfeGain_x1024)
        maxGainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
    maxGainRatio *= ispGain1_x1024/1024.0;

    MY_LOG("[%s] rat(%3.6f), maxGainRatio(%3.6f)\n", __FUNCTION__, rat, maxGainRatio);

    double gainRatio;
    double expRatio;
    if(rat>maxGainRatio)
    {
        exp2 = exp1*(maxGainRatio/rat);
        afeGain2_x1024 = minAfeGain_x1024;
        ispGain2_x1024 = 1024;
    }
    else
    {
        gainRatio = afeGain1_x1024/(double)minAfeGain_x1024;
        MY_LOG("[%s] rat(%3.6f), gainRatio(%3.6f)\n", __FUNCTION__, rat, gainRatio);
        if(rat > gainRatio)
        {
            exp2 = exp1;
            afeGain2_x1024 = minAfeGain_x1024;
            ispGain2_x1024 = ispGain1_x1024/(rat/gainRatio);
        }
        else
        {
            exp2 = exp1;
            afeGain2_x1024 = afeGain1_x1024/rat;
            ispGain2_x1024 = ispGain1_x1024;
        }
    }
}

/* LUT for gain & dEv */
#define ASD_LOG2_LUT_RATIO_BASE 256
#define ASD_LOG2_LUT_NO 101
#define ASD_LOG2_LUT_CENTER 0
#define YUV_EVDELTA_THRESHOLD  10


const MINT32 ASD_LOG2_LUT_RATIO[ASD_LOG2_LUT_NO]={
256,/* 0 */
274, 294, 315, 338, 362, 388, 416, 446, 478, 512,/* 0.1~1.0 */
549, 588, 630, 676, 724, 776, 832, 891, 955, 1024,/* 1.1~2.0 */
1097, 1176, 1261, 1351, 1448, 1552, 1663, 1783, 1911, 2048,/* 2.1~3.0 */
2195, 2353, 2521, 2702, 2896, 3104, 3327, 3566, 3822, 4096,/* 3.1~4.0 */
4390, 4705, 5043, 5405, 5793, 6208, 6654, 7132, 7643, 8192,/* 4.1~5.0 */
8780, 9410, 10086, 10809, 11585, 12417, 13308, 14263, 15287, 16384,/* 5.1~6.0 */
17560, 18820, 20171, 21619, 23170, 24834, 26616, 28526, 30574, 32768,/* 6.1~7.0 */
35120, 37640, 40342, 43238, 46341, 49667, 53232, 57052, 61147, 65536,/* 7.1~8.0 */
70240, 75281, 80864, 86475, 92682, 99334, 106464, 114105, 122295, 131072,/* 8.1~9.0 */
140479, 150562, 161369, 172951, 185364, 198668, 212927, 228210, 244589, 262144/* 9.1~10.0 */
};

MINT32 Hal3AYuv::ASDLog2Func(MUINT32 numerator, MUINT32 denominator) const
{
    MUINT32 temp_p;
    MINT32 x;
    MUINT32 *p_LOG2_LUT_RATIO = (MUINT32*)(&ASD_LOG2_LUT_RATIO[0]);

    temp_p = numerator*p_LOG2_LUT_RATIO[ASD_LOG2_LUT_CENTER];

    if (temp_p>denominator*ASD_LOG2_LUT_RATIO_BASE)
    {
        for (x=ASD_LOG2_LUT_CENTER; x<ASD_LOG2_LUT_NO; x++)
        {
            temp_p = denominator*p_LOG2_LUT_RATIO[x];

            if (temp_p>=numerator*ASD_LOG2_LUT_RATIO_BASE)
            {
                if ((temp_p -numerator*ASD_LOG2_LUT_RATIO_BASE)
                    > (numerator*ASD_LOG2_LUT_RATIO_BASE-denominator*p_LOG2_LUT_RATIO[x-1]))
                {
                    return x-1;
                }
                else
                {
                    return x;
                }
            }
            else if (x==ASD_LOG2_LUT_NO-1)
            {
                return (ASD_LOG2_LUT_NO-1);
            }
        }
    }
    return ASD_LOG2_LUT_CENTER;
}


#define ASD_ABS(val) (((val) < 0) ? -(val) : (val))

void Hal3AYuv::calcASDEv(const SENSOR_AE_AWB_CUR_STRUCT& cur)
{
    MINT32 AeEv;
    MY_LOG("[%s] shutter=%d,gain=%d,", __FUNCTION__,
        cur.SensorAECur.AeCurShutter,cur.SensorAECur.AeCurGain);

    //m_i4AELv_x10
    if ((m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain)
        <= (cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain))
    {
        AeEv = 50;//0*80/IspSensorAeAwbRef.SensorLV05LV13EVRef+50;
    }
    else
    {
        AeEv = ASDLog2Func(m_AsdRef.SensorAERef.AeRefLV05Shutter * m_AsdRef.SensorAERef.AeRefLV05Gain,
                           cur.SensorAECur.AeCurShutter * cur.SensorAECur.AeCurGain);
        if (AeEv == 0)
        {
            AeEv = 50;
        }
        else
        {
            if (m_AsdRef.SensorLV05LV13EVRef)
            {
                AeEv = AeEv * 80 / m_AsdRef.SensorLV05LV13EVRef + 50;
            }
            else
            {
                AeEv = 150;
            }
        }
    }

    if (AeEv > 150) // EV range from 50 ~150
    {
        AeEv = 150;
    }

    if (ASD_ABS(m_i4AELv_x10-AeEv) <= YUV_EVDELTA_THRESHOLD)
    {
        m_bAEStable = MTRUE;
    }
    else
    {
        m_bAEStable = MFALSE;
    }

    m_i4AELv_x10 = AeEv;

    MY_LOG("[%s] m_i4AELv_x10=%d", __FUNCTION__, m_i4AELv_x10);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 Hal3AYuv::getASDInfo(ASDInfo_T &a_ASDInfo)
{
    MINT32 err = MHAL_NO_ERROR;

    memset(&a_ASDInfo, 0, sizeof(a_ASDInfo));

    SENSOR_AE_AWB_CUR_STRUCT cur;

    m_pIHalSensor->sendCommand(m_i4SensorDev, SENSOR_CMD_GET_YUV_CURRENT_SHUTTER_GAIN_AWB_GAIN, (MUINTPTR)&cur, 0, 0);

    calcASDEv(cur);

    a_ASDInfo.i4AELv_x10 = m_i4AELv_x10;
    a_ASDInfo.bAEStable = m_bAEStable;
    a_ASDInfo.i4AWBRgain_X128 = cur.SensorAwbGainCur.AwbCurRgain;
    a_ASDInfo.i4AWBBgain_X128 = cur.SensorAwbGainCur.AwbCurBgain;
    a_ASDInfo.i4AWBRgain_D65_X128 = m_AsdRef.SensorAwbGainRef.AwbRefD65Rgain;
    a_ASDInfo.i4AWBBgain_D65_X128 = m_AsdRef.SensorAwbGainRef.AwbRefD65Bgain;
    a_ASDInfo.i4AWBRgain_CWF_X128 = m_AsdRef.SensorAwbGainRef.AwbRefCWFRgain;
    a_ASDInfo.i4AWBBgain_CWF_X128 = m_AsdRef.SensorAwbGainRef.AwbRefCWFBgain;

    MY_LOG("[%s][i4AELv_x10] %d\n", __FUNCTION__, a_ASDInfo.i4AELv_x10);
    MY_LOG("[%s][bAEStable] %d\n", __FUNCTION__, a_ASDInfo.bAEStable);
    MY_LOG("[%s][i4AWBRgain_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_X128);
    MY_LOG("[%s][i4AWBBgain_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_X128);
    MY_LOG("[%s][i4AWBRgain_D65_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_D65_X128);
    MY_LOG("[%s][i4AWBBgain_D65_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_D65_X128);
    MY_LOG("[%s][i4AWBRgain_CWF_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBRgain_CWF_X128);
    MY_LOG("[%s][i4AWBBgain_CWF_X128] %d\n", __FUNCTION__, a_ASDInfo.i4AWBBgain_CWF_X128);

    return S_3A_OK;
}

