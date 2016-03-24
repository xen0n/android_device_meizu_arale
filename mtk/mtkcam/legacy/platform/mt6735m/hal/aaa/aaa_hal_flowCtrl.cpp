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
#define LOG_TAG "Hal3AFlowCtrl"

#include <hal/aaa/aaa_hal_if.h>
#include <hal/aaa/aaa_hal_flowCtrl.h>
#include <hal/aaa/aaa_hal_raw.h>
#include <hal/aaa/IEventIrq.h>
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <kd_camera_feature.h>
//#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/hal/IHalSensor.h>
//#include <isp_tuning_mgr.h>
//#include <isp_tuning.h>
#include <mtkcam/exif/IBaseCamExif.h>
//#include <mtkcam/drv_common/isp_reg.h>
//#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/imageio/ispio_pipe_ports.h>
//#include <aaa_common_custom.h>
#include <mtkcam/Trace.h>
#include <sys/errno.h>

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/utils/common.h>
#include <sys/prctl.h>

//MINT32        g_bAFThreadLoop = 0;
//MINT32        g_semAFIRQWait = 0;
//pthread_t     g_AFThread;
//sem_t         g_semAFThreadstart;
//IspDrv*       g_pIspDrv;


using namespace NS3Av3;
//using namespace NSIspTuning;
using namespace android;
using namespace NSCam::Utils;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#define ERROR_CHECK(API)\
   {\
   MRESULT err = API;\
   if (FAILED(err))\
   {\
       setErrorCode(err);\
       return MFALSE;\
   }}\

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AFlowCtrl*
Hal3AFlowCtrl::
createInstance(MINT32 i4SensorOpenIndex)
{

#if 0
    static Hal3AFlowCtrl singleton;
    singleton.init(i4SensorDevId, i4SensorOpenIndex);
    return &singleton;
#else // if Hal3ARaw and Hal3AYuv are complete
    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList) return NULL;

    MINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);
    MINT32 eSensorType = pHalSensorList->queryType(i4SensorOpenIndex); // sensor hal defined

    if (eSensorType == NSSensorType::eRAW)
        return Hal3ARaw::createInstance(i4SensorDevId, i4SensorOpenIndex);
/*
    else if (eSensorType == NSSensorType::eYUV)
        return Hal3AYuv::createInstance(i4SensorDevId, i4SensorOpenIndex);
*/
    //else
    //{
        static Hal3AFlowCtrl singleton;
        singleton.init(i4SensorDevId, i4SensorOpenIndex);
        return &singleton;
    //}
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID
Hal3AFlowCtrl::
destroyInstance()
{
    uninit();
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AFlowCtrl::Hal3AFlowCtrl()
    : Hal3AIf()
    //, m_Lock()
    //, m_ValidateLock()
    , m_errorCode(S_3A_OK)
    , m_rParam()
    , m_bReadyToCapture(MFALSE)
    , m_i4SensorDev(0)
    //, m_bDebugEnable(MFALSE)
    , m_i4FrmId(0)
    , m_3ACapMode(ECapMode_P2_Cal|ECapMode_P2_Set)
    , m_PvMode(EPv_Normal)
    , m_b3APvInitOK(MFALSE)
    , mpVSTimer(NULL)
    , mbEnable3ASetParams(MTRUE)
    , mLastFlashOnFrmIdx(-1)
    //, mbInCaptureProcess(MFALSE)
    //, mpIspDrv_forAF(NULL)
    //, mbAFThreadLoop(0)
    //, mpStateMgr(NULL)
    , mpIEventIrq(NULL)
    , mbMainFlashOnThisFrame(MFALSE)
    //, mpScheduler(NULL)
    , m_rResultBuf(6)
    , m_rResultBufCur(6)
    , mbListenUpdate(MFALSE)
    , m_3ALogEnable(0)
    , m_3ACtrlEnable(0)

{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Hal3AFlowCtrl::~Hal3AFlowCtrl()
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AFlowCtrl::
init(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex) //can be called only once by RAW or YUV, no user count
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    char value[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    m_3ALogEnable = atoi(cLogLevel);
    if ( m_3ALogEnable == 0 ) {
        ::property_get("debug.camera.log.hal3a", cLogLevel, "0");
        m_3ALogEnable = atoi(cLogLevel);
    }
    property_get("debug.aaa.ctrl.enable", value, "1023");
    m_3ACtrlEnable = atoi(value);
    MY_LOG("m_3ALogEnable = %d, m_3ACtrlEnable = %d\n", m_3ALogEnable, m_3ACtrlEnable);

    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;
    //Mutex::Autolock lock(m_Lock);

    m_i4SensorDev = i4SensorDevId;
    m_i4SensorOpenIdx = i4SensorOpenIndex;

    MY_LOG_IF(1,"m_i4SensorDev = %d, i4SensorOpenIndex = %d \n", m_i4SensorDev, i4SensorOpenIndex);

    mpVSTimer = new AaaTimer;
    mpCbSet = NULL;
    mbListenUpdate = MFALSE;

    IEventIrq::ConfigParam IrqConfig(1, 0, IEventIrq::E_Event_Vsync);
    mpIEventIrq = IEventIrq::createInstance(IrqConfig, "VSIrq");

    createThread();

    m_TgInfo = CAM_TG_1; //need to get correct TG info

    MY_LOG_IF(1,"[Hal3AFlowCtrl::init] done\n");
    return S_3A_OK;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
Hal3AFlowCtrl::
uninit() //can be called only once by RAW or YUV, no user count
{
    MRESULT ret = S_3A_OK;
    MBOOL bRet = MTRUE;

    //Mutex::Autolock lock(m_Lock);

    destroyThread();

    mpIEventIrq->destroyInstance("VSIrq");

    mpCbSet = NULL;

    delete mpVSTimer;
    mpVSTimer = NULL;

    m_rResultBuf.clearAll();
    m_rResultBufCur.clearAll();

    MY_LOG_IF(1,"[Hal3AFlowCtrl::uninit] done\n");
    return S_3A_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::validatePass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf)
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::setSensorAndPass1(const ParamIspProfile_T& rParamIspProfile_, MBOOL fgPerframe)
{
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID Hal3AFlowCtrl::on3AProcFinish(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur)
{
if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    updateResult(u4MagicNum);

    // TODO: send 3A finish
    MUINT32 fgOK = MTRUE;
    MUINT32 fgAeAwbReady = m_b3APvInitOK;
    MUINT32 u4Notify =
        (fgOK << I3ACallBack::e3AProcOK) |
        (fgAeAwbReady << I3ACallBack::e3APvInitReady);

    if (mpCbSet)
    {
        MINT32 i4CurId = getCurrentHwId();
        MY_LOG_IF(m_3ALogEnable, "[%s] SOF(0x%x)", __FUNCTION__, i4CurId);

        CAM_TRACE_FMT_BEGIN("3A_CB #(%d), SOF(%d)", u4MagicNum, i4CurId);

        mpCbSet->doNotifyCb(
            I3ACallBack::eID_NOTIFY_3APROC_FINISH,
            u4MagicNum,     // magic number
            i4CurId,        // SOF idx
            m_u4FrmIdStat);

        // for flash state
        if (!(m_rParam.u4HalFlag & HAL_FLG_DUMMY))
        {
            mpCbSet->doNotifyCb(
                I3ACallBack::eID_NOTIFY_CURR_RESULT,
                u4MagicNumCur,
                MTK_FLASH_STATE,
                m_rResult.u1FlashState);
        }

        CAM_TRACE_FMT_END();
    }

}
}

MVOID
Hal3AFlowCtrl::
updateResult(MUINT32 u4MagicNum)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    // pass1 result
    getCurrResult(u4MagicNum, m_rResult);
    m_rResultBuf.updateResult(u4MagicNum, m_rResult);
    // result for high quality
    m_rResultBufCur.updateResult(m_u4FrmIdStat, m_rResult);
}

MINT32
Hal3AFlowCtrl::
getResult(MUINT32 i4FrmId, Result_T& rResult)
{
    MINT32 i4Ret = m_rResultBuf.getResult(i4FrmId, rResult);
    if (-1 == i4Ret)
    {
        MY_ERR("Fail get Result(%d)", i4FrmId);
        getCurrResult(i4FrmId, rResult);
    }
    return i4Ret;
}

MINT32
Hal3AFlowCtrl::
getResultCur(MUINT32 i4FrmId, Result_T& rResult)
{
    MINT32 i4Ret = m_rResultBufCur.getResult(i4FrmId, rResult);
    if (-1 == i4Ret)
    {
        MY_ERR("Fail get Result(%d)", i4FrmId);
        getCurrResult(i4FrmId, rResult);
    }
    return i4Ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::postCommand(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] enter(%d)", __FUNCTION__, eCmd);
    MUINT32 u4MagicNum = 0;
    MUINT32 u4MagicNumCur = 0;
    const ParamIspProfile_T* pParamIspProf = reinterpret_cast<const ParamIspProfile_T*>(i4Arg);
    MBOOL fgUpdate = (eCmd == ECmd_Update) && (pParamIspProf->iValidateOpt != ParamIspProfile_T::EParamValidate_None);
    if (eCmd == ECmd_CameraPreviewStart)
    {
        m_rResultBuf.clearAll();
        m_rResultBufCur.clearAll();
    }
    if (fgUpdate)
    {
        MBOOL bPre;
        u4MagicNum = pParamIspProf->i4MagicNum;
        u4MagicNumCur = pParamIspProf->i4MagicNumCur;
        mbMainFlashOnThisFrame = isMeetMainFlashOnCondition();
        bPre = isMeetPreFlashOnCondition();
        if (mbMainFlashOnThisFrame||bPre) mLastFlashOnFrmIdx = u4MagicNum;
        if (mbMainFlashOnThisFrame) setFlashLightOnOff(1, 1);
        if (bPre) setFlashLightOnOff(1, 0);
    }

    postCommandDerived(eCmd, i4Arg);

    if (fgUpdate)
    {
        on3AProcFinish(u4MagicNum, u4MagicNumCur);
    }

    return MTRUE;
}

MBOOL Hal3AFlowCtrl::postCommandDerived(ECmd_T const r3ACmd, MINTPTR const i4Arg)
{
    MY_ERR("[%s] it should be overwritten", __FUNCTION__);
    return MTRUE;
}

MBOOL Hal3AFlowCtrl::sendCommandDerived(ECmd_T const eCmd, MINTPTR const i4Arg)
{
    MY_ERR("[%s] it should be overwritten", __FUNCTION__);
    return MTRUE;
}


MVOID
Hal3AFlowCtrl::notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] u4MagicNum(%d)", __FUNCTION__, u4MagicNum);
    m_u4FrmIdStat = u4MagicNum;
    updateImmediateResult(u4MagicNum);
    if (u4MagicNum == mLastFlashOnFrmIdx)
    {
        setFlashLightOnOff(0, 1); // don't care main or pre
        mLastFlashOnFrmIdx = -1;
    }
}

MBOOL
Hal3AFlowCtrl::isMeetMainFlashOnCondition()
{
    return
        (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_SINGLE))
        ||
        ( ((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_ALWAYS_FLASH)||((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON_AUTO_FLASH) && isStrobeBVTrigger()))
           && (m_rParam.u1CaptureIntent == MTK_CONTROL_CAPTURE_INTENT_STILL_CAPTURE) );
}

MBOOL
Hal3AFlowCtrl::isMeetPreFlashOnCondition()
{
    return
        (((m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_OFF)||(m_rParam.u4AeMode == MTK_CONTROL_AE_MODE_ON)) && (m_rParam.u4StrobeMode == MTK_FLASH_MODE_TORCH));
}


MBOOL
Hal3AFlowCtrl::isStrobeBVTrigger()
{
    MY_ERR("[%s] it should be overwritten", __FUNCTION__);
    return MFALSE;
}

MBOOL Hal3AFlowCtrl::setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/)
{
    MY_ERR("[%s] it should be overwritten", __FUNCTION__);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::setParams(Param_T const &rNewParam)
{
    MY_ERR("[%s] it should be overwritten", __FUNCTION__);
    return MFALSE;
#if 0
    if (!mbEnable3ASetParams) return MTRUE;

    m_rParam = rNewParam;

    return MTRUE;
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::autoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

if (ENABLE_3A_GENERAL & m_3ACtrlEnable) {
    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        //ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFStart));
    }
    //IAfMgr::getInstance().autoFocus(m_i4SensorDev);
}
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::cancelAutoFocus()
{
    MY_LOG("[%s()]\n", __FUNCTION__);

    if ((m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_PICTURE) && (m_rParam.u4AfMode != MTK_CONTROL_AF_MODE_CONTINUOUS_VIDEO))   {
        //ERROR_CHECK(mpStateMgr->sendCmd(ECmd_AFEnd));
    }
    //IAfMgr::getInstance().cancelAutoFocus(m_i4SensorDev);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MBOOL Hal3AFlowCtrl::set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const
//{
//   return MTRUE;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//MBOOL Hal3AFlowCtrl::setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const
//{
//    return MTRUE;
//}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AFlowCtrl::getDelayFrame(EQueryType_T const eQueryType) const
{
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL Hal3AFlowCtrl::setIspPass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf)
{
    MBOOL res = validatePass2(flowType, rCamInfo, pRegBuf);
//    if (pDbgInfoCtnr)
//        setDebugInfo(pDbgInfoCtnr, MFALSE); //read from working buffer

    return res;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 Hal3AFlowCtrl::updateCaptureParams(CaptureParam_T &a_rCaptureInfo)
{
    return S_3A_OK;
}

MVOID Hal3AFlowCtrl::setSensorMode(MINT32 i4SensorMode)
{

}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// setCallbacks
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#if 0
MBOOL Hal3AFlowCtrl::setCallbacks(I3ACallBack* cb)
{
    MY_LOG("[%s()][p]%d\n", __FUNCTION__, cb);

    return MTRUE;/*NeedUpdate*///IAfMgr::getInstance().setCallbacks(m_i4SensorDev, cb);

}
#endif
MINT32
Hal3AFlowCtrl::
attachCb(I3ACallBack* cb)
{
    MY_LOG("[%s] mpCbSet(0x%08x), cb(0x%08x)", __FUNCTION__, mpCbSet, cb);
    mpCbSet = cb;
    return 0;
}

MINT32
Hal3AFlowCtrl::
detachCb(I3ACallBack* cb)
{
    MY_LOG("[%s] mpCbSet(0x%08x), cb(0x%08x)", __FUNCTION__, mpCbSet, cb);
    mpCbSet = NULL;
    return 0;

}


/******************************************************************************
*
*******************************************************************************/
MINT32
Hal3AFlowCtrl::
send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2)
{
/*
    switch (e3ACtrl)
    {
    case E3ACtrl_SetShadingMode:
        LscMgr2::getInstance(static_cast<ESensorDev_T>(m_i4SensorDev))->setOnOff(i4Arg1 ? MTRUE : MFALSE);
        break;
    case E3ACtrl_Enable3ASetParams:
        mbEnable3ASetParams = i4Arg1;
        break;

    default:
        return -1;
    }
*/
    return 0;
}

MBOOL Hal3AFlowCtrl::sem_wait_relativeTime(sem_t *pSem, nsecs_t reltime, const char* info)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] pSem(%p), reltime(%lld), info(%s)\n", __FUNCTION__, pSem, reltime, info);
    struct timespec ts;
    if (clock_gettime(CLOCK_REALTIME, &ts) == -1)
        MY_ERR("error in clock_gettime! Please check\n");

    ts.tv_sec  += reltime/1000000000;
    ts.tv_nsec += reltime%1000000000;
    if (ts.tv_nsec >= 1000000000)
    {
        ts.tv_nsec -= 1000000000;
        ts.tv_sec += 1;
    }
    int s = sem_timedwait(pSem, &ts);
    if (s == -1)
    {
        if (errno == ETIMEDOUT)
        {
            MY_ERR("[%s][%s]sem_timedwait() timed out\n", __FUNCTION__, info);
            return MFALSE;
        }
        else
            MY_ERR("[%s][%s]sem_timedwait() errno = %d\n", __FUNCTION__, info, errno);
    }
    else
        MY_LOG_IF(m_3ALogEnable, "[%s][%s]sem_timedwait() succeeded\n", __FUNCTION__, info);

    return MTRUE;
}
