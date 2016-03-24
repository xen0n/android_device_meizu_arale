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
#define LOG_TAG "af_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

//#include "stereo_hal.h"
#include <utils/threads.h>  // For Mutex::Autolock.
#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <kd_camera_feature.h>
#include <aaa_log.h>
#include <mtkcam/common/faces.h>
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/aaa_hal_if.h>
#include <aaa_hal.h>
#include <camera_custom_nvram.h>
#include <af_param.h>
#include <pd_param.h>
#include <awb_param.h>
#include <ae_param.h>
#include <af_tuning_custom.h>
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <nvram_drv.h>
#include <nvram_drv_mgr.h>
#include <mtkcam/acdk/cct_feature.h>
#include <flash_param.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <af_feature.h>
#include <mtkcam/algorithm/lib3a/af_algo_if.h>
#include <mtkcam/algorithm/lib3a/pd_algo_if.h>
#include "af_mgr_if.h"
#include "af_mgr.h"
#include "camera_custom_cam_cal.h"  //seanlin 121022 for test
#include "cam_cal_drv.h" //seanlin 121022 for test
#include "nvbuf_util.h"
#include <isp_mgr.h>
#include "aaa_common_custom.h"
// AF v1.2
#include <math.h>
#include <android/sensor.h>             // for g/gyro sensor listener
#include <utils/SensorListener.h>    // for g/gyro sensor listener
#define SENSOR_ACCE_POLLING_MS  33
#define SENSOR_GYRO_POLLING_MS  33
#define SENSOR_ACCE_SCALE       100
#define SENSOR_GYRO_SCALE       100

#define WIN_SIZE_MIN 8
#define WIN_SIZE_MAX 1020
#define WIN_POS_MIN  16
#define FL_WIN_SIZE_MIN 8
#define FL_WIN_SIZE_MAX 4094
#define FL_WIN_POS_MIN  16

NVRAM_LENS_PARA_STRUCT* g_pNVRAM_LENS;
using namespace NS3A;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

// AF v2.1 HybridAF MUL
MINT32 PDAF_SCAN_MODE = 0;
MINT32 PL_WdataFile = 0;


#define ISPREADREG
AfMgr* AfMgr::s_pAfMgr = MNULL;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AfMgrDev : public AfMgr
{
public:
    static    AfMgr&    getInstance() {
        static AfMgrDev<eSensorDev> singleton;
        AfMgr::s_pAfMgr = &singleton;
        return singleton;
    }

    AfMgrDev() : AfMgr(eSensorDev) {}
    virtual ~AfMgrDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// AF v1.2
// g/gyro sensor listener handler and data
static MINT32 gAcceInfo[3];
static MUINT64 gAcceTS;
static MINT32 gGyroInfo[3];
static MUINT64 gGyroTS;
static SensorListener* gpSensorListener=NULL;
static MBOOL gListenLogOn = MFALSE;

DAF_TBL_STRUCT m_DAF_TBL;
void afListenHandler(ASensorEvent event)
{
    switch(event.type)
    {
        case ASENSOR_TYPE_ACCELEROMETER:
        {
            gAcceInfo[0] = event.acceleration.x * SENSOR_ACCE_SCALE;
            gAcceInfo[1] = event.acceleration.y * SENSOR_ACCE_SCALE;
            gAcceInfo[2] = event.acceleration.z * SENSOR_ACCE_SCALE;
            gAcceTS = event.timestamp;
            if(gListenLogOn)
            {
                MY_LOG("SensorEventAF Acc(%f,%f,%f,%lld)",
                event.acceleration.x,
                event.acceleration.y,
                event.acceleration.z,
                    event.timestamp);
            }
            break;
        }
        case ASENSOR_TYPE_GYROSCOPE:
        {
            gGyroInfo[0] = event.vector.x * SENSOR_GYRO_SCALE;
            gGyroInfo[1] = event.vector.y * SENSOR_GYRO_SCALE;
            gGyroInfo[2] = event.vector.z * SENSOR_GYRO_SCALE;
            gGyroTS = event.timestamp;
            if(gListenLogOn)
            {
                MY_LOG("SensorEventAF Gyro(%f,%f,%f,%lld)",
                event.vector.x,
                event.vector.y,
                event.vector.z,
                    event.timestamp);
            }
            break;
        }
        case ASENSOR_TYPE_MAGNETIC_FIELD:
        case ASENSOR_TYPE_LIGHT:
        case ASENSOR_TYPE_PROXIMITY:
        default:
        {
            MY_LOG("unknown type(%d)",event.type);
            break;
        }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr &AfMgr::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev) {
        case ESensorDev_Main: //  Main Sensor
            return  AfMgrDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond: //  Main Second Sensor
            return  AfMgrDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_Sub: //  Sub Sensor
            return  AfMgrDev<ESensorDev_Sub>::getInstance();
        default:
            MY_LOG("i4SensorDev = %d", i4SensorDev);
            if (AfMgr::s_pAfMgr) {
                return  *AfMgr::s_pAfMgr;
            } else {
                return  AfMgrDev<ESensorDev_Main>::getInstance();
            }
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::AfMgr(ESensorDev_T eSensorDev)
{
    mEngFileName = new char[200];
    mEngFileName[0]=0;

    m_Users   = 0;
    m_CCTUsers   = 0;
    m_i4CurrSensorDev=(MINT32)eSensorDev;
    m_pMcuDrv = NULL;
    m_pIAfAlgo = NULL;
    m_pIPdAlgo = NULL;
    // AF v1.2
    gAcceValid = MFALSE;
    gGyroValid = MFALSE;
    memset(&m_sAFInput,   0, sizeof(m_sAFInput));
    memset(&m_sAFOutput,   0, sizeof(m_sAFOutput));
    memset(&m_sAFParam,   0, sizeof(m_sAFParam));
    memset(&m_sAFConfig,   0, sizeof(m_sAFConfig));
    memset(&m_NVRAM_LENS,   0, sizeof(m_NVRAM_LENS));
    memset(&m_CameraFocusArea,   0, sizeof(m_CameraFocusArea));
    memset(&m_FDArea,   0, sizeof(m_FDArea));
    memset(&m_sAFFullStat,   0, sizeof(m_sAFFullStat));
    memset(&m_sEZoom,   0, sizeof(m_sEZoom));

    m_i4AF_in_Hsize = 0;
    m_i4AF_in_Vsize = 0;
    m_i4CurrSensorId = 0x1;
    m_i4CurrLensId = 0;
    m_eCurrAFMode=0;

    m_i4EnableAF = -1;
    m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
    m_sAFInput.i4FullScanStep = 1;
    m_pAFCallBack = NULL;
    m_i4AFPreStatus = AF_MARK_NONE;
    m_bDebugEnable = FALSE;
    m_i4AutoFocuscb = FALSE;
    m_i4AF_TH[0] = -1;
    m_i4AF_TH[1] = -1;
    m_tcaf_mode=0;
    m_i4AutoFocus = FALSE;
    m_eSensorTG=ESensorTG_1;
    m_i4SensorIdx=0;
    for (MINT32 i=0; i<36; i++)   {
        m_i8PreVStat[i] = 0;
    }
    m_flkwin_syncflag  = 0;
    m_isPDAF_ON=0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AfMgr::~AfMgr()
{
    delete []mEngFileName;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::sendAFNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorIdx, "afmgr");
    MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
    pPipe->destroyInstance("afmgr");
    return fgRet;
}

MRESULT AfMgr::init(MINT32 i4SensorIdx, MINT32 isInitMCU)
{
    MRESULT ret = S_3A_OK;
    char t_buffer[0x800];
    int t_size = 0x800;
    int t_offset = 0x100;
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.af_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    property_get("debug.afwin.w", value, "0");
    m_bDebugAFwinW = atoi(value);
    property_get("debug.afwin.h", value, "0");
    m_bDebugAFwinH = atoi(value);

    Mutex::Autolock lock(m_Lock);
    m_tcaf_mode=0;
    m_i4SensorIdx=i4SensorIdx;
    gSetRectFirst = true;
    if (m_Users > 0)
    {
        MY_LOG("[init] no init, %d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return S_3A_OK;
    }
    else
    {
        m_i4EnableAF = -1;
        android_atomic_inc(&m_Users);
        MY_LOG("[init]EnableAF %d", m_i4EnableAF);
    }

    MY_LOG("[init] start, m_Users: %d", m_Users);
    // --- init MCU ---
    SensorStaticInfo rSensorStaticInfo;
    if (m_i4EnableAF == -1)
    {
       IHalSensorList* const pIHalSensorList = IHalSensorList::get();
       IHalSensor* pIHalSensor = pIHalSensorList->createSensor("af_mgr", m_i4SensorIdx);
       SensorDynamicInfo rSensorDynamicInfo;

       switch(m_i4CurrSensorDev)
       {
           case ESensorDev_Main:
               pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
               pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
               break;
           case ESensorDev_Sub:
               pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
               pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
               break;
           case ESensorDev_MainSecond:
               pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
               pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
               break;
           default:
               MY_ERR("Invalid sensor device: %d", m_i4CurrSensorDev);
       }
       if(pIHalSensor) pIHalSensor->destroyInstance("af_mgr");

       switch (rSensorDynamicInfo.TgInfo)
       {
           case CAM_TG_1:  m_eSensorTG = ESensorTG_1;  break;
           case CAM_TG_2:  m_eSensorTG = ESensorTG_2;   break;
           default:
               MY_LOG("rSensorDynamicInfo.TgInfo = %d", rSensorDynamicInfo.TgInfo);
               //return MFALSE;
       }
       m_Pix_Id = (MINT32)rSensorStaticInfo.sensorFormatOrder;  // 0:BGGR, 1:GBRG, 2GRBG, 3RGGB
       MY_LOG("TG = %d, m_Pix_Id = %d (0:B,1:GB,2:GR,3:R)\n", m_eSensorTG, m_Pix_Id);

       m_i4CurrSensorId=rSensorStaticInfo.sensorDevID;
       MCUDrv::lensSearch(m_i4CurrSensorDev, m_i4CurrSensorId);
       m_i4CurrLensId = MCUDrv::getCurrLensID(m_i4CurrSensorDev);
       MY_LOG("[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

        if (m_i4CurrLensId == 0xFFFF)   m_i4EnableAF = 0;
        else                            m_i4EnableAF = 1;
    }

    if(isInitMCU)
    {
    m_pMcuDrv = MCUDrv::createInstance(m_i4CurrLensId);
    if (!m_pMcuDrv)
    {
        MY_ERR("McuDrv::createInstance fail");
        m_i4EnableAF = 0;
    }

    if (m_pMcuDrv->init(m_i4CurrSensorDev) < 0)
    {
        MY_ERR("m_pMcuDrv->init() fail");
        m_i4EnableAF = 0;
    }
    }
    // --- init ISP Drv/Reg ---
    #ifdef ISPREADREG
    m_pIspReg = IspDrv::createInstance();
    if (!m_pIspReg) {
        MY_ERR("IspDrv::createInstance() fail \n");
    }

    if (m_pIspReg->init("af_mgr") == MFALSE) {
        MY_ERR("pIspDrv->init() fail \n");
        m_pIspReg = NULL;
    }
    #endif

    m_isPDAF_ON=rSensorStaticInfo.PDAF_Support;

    //m_pPipe = INormalPipe::createInstance(m_i4SensorIdx,"af_mgr");
    setAF_IN_HSIZE();
    MY_LOG("[setAF_IN_HSIZE][SensorDev]%d [H]%d [V]%d\n", m_i4CurrSensorDev, m_i4AF_in_Hsize,  m_i4AF_in_Vsize);
    if ((m_sAFInput.sEZoom.i4W == 0) || (m_sAFInput.sEZoom.i4H == 0))
    {
        m_sAFInput.sEZoom.i4W = m_i4AF_in_Hsize;
        m_sAFInput.sEZoom.i4H = m_i4AF_in_Vsize;
    }
    if (m_i4AF_in_Hsize==(MINT32)rSensorStaticInfo.captureWidth)
    {
         m_sAFInput.i4IsZSD = TRUE;
  }
    else
    {
        m_sAFInput.i4IsZSD = FALSE;
        m_isPDAF_ON=0;
    }
    // --- init af algo ---
    #if USE_OPEN_SOURCE_AF
    m_pIAfAlgo=IAfAlgo::createInstance<EAAAOpt_OpenSource>(m_i4CurrSensorDev);
    #else
    m_pIAfAlgo=IAfAlgo::createInstance<EAAAOpt_MTK>(m_i4CurrSensorDev);
    #endif
    if (!m_pIAfAlgo)
    {
        MY_ERR("AfAlgo pointer NULL \n");
        m_i4EnableAF = 0;
        return S_AF_OK;
    }
    if(m_isPDAF_ON)
        m_pIPdAlgo = IPdAlgo::createInstance(m_i4CurrSensorDev);

    m_i4AFPreStatus = AF_MARK_NONE;
    m_sAFOutput.sAFArea.sRect[0].i4Info=AF_MARK_NONE;
    m_i4AutoFocuscb = FALSE;
    m_i4LastFocusModeTAF = FALSE;
    m_sAFOutput.i4FDDetect=0;
    // --- NVRAM ---
    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)g_pNVRAM_LENS);
    if(err!=0)
        MY_ERR("AfAlgo NvBufUtil get buf fail! \n");
    m_NVRAM_LENS.rFocusRange = g_pNVRAM_LENS->rFocusRange;
    m_NVRAM_LENS.rAFNVRAM= g_pNVRAM_LENS->rAFNVRAM;
    m_NVRAM_LENS.rPDNVRAM    = g_pNVRAM_LENS->rPDNVRAM;
    #ifdef AFEXTENDCOEF
    CustAF_THRES_T sAFExtCoef=getAFExtendCoef(m_i4AF_in_Hsize, m_i4AF_in_Vsize, m_i4CurrSensorId);
    if(sAFExtCoef.i4ISONum)
    {
        memcpy(&m_NVRAM_LENS.rAFNVRAM.sREV2_AF_TH, &sAFExtCoef, sizeof(sAFExtCoef));
        m_sAFInput.i4IsRevMode2=TRUE;
    }
    else m_sAFInput.i4IsRevMode2=FALSE;
    MY_LOG("[AFExtendCoef] %d",m_sAFInput.i4IsRevMode2);
    #endif

    #if 1
    //MY_LOG("[nvram][THRES_MAIN]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    //MY_LOG("[nvram][SUB_MAIN]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_SUB);
    MY_LOG("[nvram][Normal Num]%d [Macro Num]%d", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum, m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4MacroNum);
    //MY_LOG("[nvram][VAFC Fail Cnt]%d", m_NVRAM_LENS.rAFNVRAM.i4VAFC_FAIL_CNT);
    //MY_LOG("[nvram][LV thres]%d", m_NVRAM_LENS.rAFNVRAM.i4LV_THRES);
    //MY_LOG("[nvram][PercentW]%d [PercentH]%d", m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W, m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H);
    //MY_LOG("[nvram][AFC step]%d", m_NVRAM_LENS.rAFNVRAM.i4AFC_STEP_SIZE);
    //MY_LOG("[nvram][InfPos]%d", m_NVRAM_LENS.rFocusRange.i4InfPos);
    //MY_LOG("[nvram][MacroPos]%d", m_NVRAM_LENS.rFocusRange.i4MacroPos);
    #endif

    // AF v1.2
    // --- init g/gyro sensor listener ---
    // i4Coef[5] == 1:enable SensorListener, else: disable
    if((m_NVRAM_LENS.rAFNVRAM.i4SensorEnable > 0) && (m_AndroidServiceState==TRUE) && (gpSensorListener==NULL))
    {
        gpSensorListener = SensorListener::createInstance();
        gpSensorListener->setListener(afListenHandler);

        gAcceInfo[0] = gAcceInfo[1] = gAcceInfo[2] = 0;
        gAcceValid = gpSensorListener->enableSensor(SensorListener::SensorType_Acc, SENSOR_ACCE_POLLING_MS);
        if(!gAcceValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Acc);
        }

        gGyroInfo[0] = gGyroInfo[1] = gGyroInfo[2] = 0;
        gGyroValid = gpSensorListener->enableSensor(SensorListener::SensorType_Gyro, SENSOR_GYRO_POLLING_MS);
        if(!gGyroValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
        }
    }

    // --- Param ---
    m_sAFParam = getAFParam();

    // AF v2.0 : use NVRAM parameters instead of AFParam
    MY_LOG("[Param][Normal min step]%d [Macro min step]%d", m_NVRAM_LENS.rAFNVRAM.i4AFS_STEP_MIN_NORMAL, m_NVRAM_LENS.rAFNVRAM.i4AFS_STEP_MIN_MACRO);

    // --- Config ---
    m_sAFConfig = getAFConfig();
    m_CameraFocusArea.u4Count=0;
    setAFArea(m_CameraFocusArea);
    m_LastAFArea.i4X = m_sAFInput.sAFArea.sRect[0].i4X;
    m_LastAFArea.i4Y = m_sAFInput.sAFArea.sRect[0].i4Y;
    m_LastAFArea.i4W = m_sAFInput.sAFArea.sRect[0].i4W;
    m_LastAFArea.i4H = m_sAFInput.sAFArea.sRect[0].i4H;
    m_LastAFArea.i4Info = 0;    // monitor

    // AF v1.2
    // i4Coef[7] > 0: enable new scenechange , else: disable
    //if((m_NVRAM_LENS.rAFNVRAM.i4SceneMonitorLevel > 0) && (m_AndroidServiceState==TRUE) )
    //{   // update m_NVRAM_LENS.rAFNVRAM before setAFParam to AF alg
    //    updateSceneChangeParams();
    //}

    m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    m_pIAfAlgo->initAF(m_sAFInput, m_sAFOutput);
    m_next_query_FrmNum=0;

    if (m_NVRAM_LENS.rAFNVRAM.i4ReadOTP == TRUE)   // AF v2.0 : use NVRAM parameters instead of AFParam
    {
        readOTP();
    }

    m_i4AF_TH[0] = -1;
    m_i4AF_TH[1] = -1;

    setAFConfig(m_sAFOutput.sAFStatConfig);
    setAFWinTH(m_sAFOutput.sAFStatConfig);
    m_AEsetPGN=m_sAFOutput.sAFStatConfig.i4SGG_GAIN;
    setGMR(m_sAFOutput.sAFStatConfig);
    //setAFWinConfig(m_sAFOutput.sAFArea);
    m_flkwin_syncflag = 0;
    m_flkwin_synccnt  = 2;
    setFlkWinConfig();
    //printAFConfigLog0();
    //printAFConfigLog1();
    if (m_pIAfAlgo)
        m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);
    m_NonInitState=MFALSE;

    MINT32 AFtbl_num = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
    m_DAF_TBL.af_dac_min = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
    m_DAF_TBL.af_dac_max = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_num-1];
    m_DAF_TBL.is_daf_run=0;
    if( (m_sAFInput.i4IsVDO==TRUE) && (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO))
    {
        AFtbl_num = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
        m_DAF_TBL.af_dac_min = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
        m_DAF_TBL.af_dac_max = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos[AFtbl_num-1];
    }

    if ((m_pIPdAlgo!=NULL) && (m_isPDAF_ON==1))
    {
        MINT32 i4_ret=0;
        PD_INIT_T initPdData;

        IHalSensorList* const pIHalSensorList = IHalSensorList::get();
        IHalSensor* pIHalSensor = pIHalSensorList->createSensor("PDAF-af_mgr", m_i4SensorIdx);

        MINT32 scen=SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        pIHalSensor->sendCommand(m_i4CurrSensorDev, SENSOR_CMD_GET_SENSOR_PDAF_INFO,(MINTPTR)&scen, (MINTPTR)&m_stPDsensorIF, 0);
        MY_LOG("m_stPDsensorIF    %d %d %d %d %d",
        m_stPDsensorIF.i4OffsetX, m_stPDsensorIF.i4OffsetY,m_stPDsensorIF.i4PitchX,  m_stPDsensorIF.i4PitchY,m_stPDsensorIF.i4PairNum);

        if(m_NVRAM_LENS.rPDNVRAM.rCaliData.i4Size==0)
        {
            MY_LOG("NVRAM NO PDAF calib data, read from EEPROM !!");
            memset(t_buffer,0xFF,t_size);
            char*ptr=t_buffer;
            pIHalSensor->sendCommand(m_i4CurrSensorDev,SENSOR_CMD_GET_PDAF_DATA, (MUINTPTR)&t_offset, (MUINTPTR)&ptr, (MUINTPTR)&t_size);
            memcpy(initPdData.rPDNVRAM.rCaliData.uData, t_buffer,t_size);
            initPdData.rPDNVRAM.rCaliData.i4Size = 0x57C;

            err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)g_pNVRAM_LENS);
            memcpy(g_pNVRAM_LENS->rPDNVRAM.rCaliData.uData, t_buffer,t_size);
            g_pNVRAM_LENS->rPDNVRAM.rCaliData.i4Size = 0x57C;
            NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev);
        }
        else
        {
            memcpy(initPdData.rPDNVRAM.rCaliData.uData, m_NVRAM_LENS.rPDNVRAM.rCaliData.uData,m_NVRAM_LENS.rPDNVRAM.rCaliData.i4Size);
            initPdData.rPDNVRAM.rCaliData.i4Size = m_NVRAM_LENS.rPDNVRAM.rCaliData.i4Size;
            MY_LOG("NVRAM Have PDAF calib data %d", initPdData.rPDNVRAM.rCaliData.i4Size);
        }
        if(pIHalSensor) pIHalSensor->destroyInstance("PDAF-af_mgr");

        initPdData.rPDNVRAM.rTuningData=m_NVRAM_LENS.rPDNVRAM.rTuningData;

        MY_LOG("[PDAF],%d %d %d %d %d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[0],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[1] ,
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[2],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[3],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx1[4],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[0],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[1],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[2],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[3],
        initPdData.rPDNVRAM.rTuningData.i4ConfIdx2[4]);

        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[0][5]);

        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[1][5]);
        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[2][5]);

        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[3][5]);

        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[4][5]);

        MY_LOG("[PDAF],%d %d %d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][0],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][1],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][2],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][3],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][4],
        initPdData.rPDNVRAM.rTuningData.i4ConfTbl[5][5]);

        MY_LOG("[PDAF],%d %d %d %d",
        initPdData.rPDNVRAM.rTuningData.i4ConfThr,
        initPdData.rPDNVRAM.rTuningData.i4SaturateLevel,
        initPdData.rPDNVRAM.rTuningData.i4SaturateThr, sizeof(NVRAM_LENS_PARA_STRUCT));

        MY_LOG("[PDAF][i4FocusPDSizeX] %d [i4FocusPDSizeY] %d  ",initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeX,initPdData.rPDNVRAM.rTuningData.i4FocusPDSizeY);
        i4_ret = m_pIPdAlgo->initPD(initPdData);
        if(i4_ret)
        {
            MY_LOG("PDAF calib data wrong!!");
            MINT32 kidx;
            for (kidx=0; kidx< 0x600; kidx=kidx+16)
                MY_LOG("===0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x,\n",
                initPdData.rPDNVRAM.rCaliData.uData[kidx  ],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 1],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 2],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 3],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 4],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 5],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 6],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 7],
                initPdData.rPDNVRAM.rCaliData.uData[kidx+8],initPdData.rPDNVRAM.rCaliData.uData[kidx+ 9],initPdData.rPDNVRAM.rCaliData.uData[kidx+10],initPdData.rPDNVRAM.rCaliData.uData[kidx+11],initPdData.rPDNVRAM.rCaliData.uData[kidx+12],initPdData.rPDNVRAM.rCaliData.uData[kidx+13],initPdData.rPDNVRAM.rCaliData.uData[kidx+14],initPdData.rPDNVRAM.rCaliData.uData[kidx+15]);
            m_isPDAF_ON=0;
        }

        PD_CONFIG_T a_sPDConfig;
        a_sPDConfig.i4Bits=10;
        a_sPDConfig.i4IsPacked=1;
        a_sPDConfig.i4RawHeight=m_i4AF_in_Vsize;
        a_sPDConfig.i4RawStride=m_i4AF_in_Hsize*10/8;;
        a_sPDConfig.i4RawWidth=m_i4AF_in_Hsize;
        a_sPDConfig.sPdBlockInfo.i4BlockNumX= m_i4AF_in_Hsize/m_stPDsensorIF.i4PitchX;
        a_sPDConfig.sPdBlockInfo.i4BlockNumY= m_i4AF_in_Vsize/m_stPDsensorIF.i4PitchY;
        a_sPDConfig.sPdBlockInfo.i4OffsetX = m_stPDsensorIF.i4OffsetX;
        a_sPDConfig.sPdBlockInfo.i4OffsetY = m_stPDsensorIF.i4OffsetY;
        if(m_stPDsensorIF.i4PairNum >16) m_stPDsensorIF.i4PairNum=16;
        a_sPDConfig.sPdBlockInfo.i4PairNum = m_stPDsensorIF.i4PairNum;
        a_sPDConfig.sPdBlockInfo.i4PitchX  = m_stPDsensorIF.i4PitchX;
        a_sPDConfig.sPdBlockInfo.i4PitchY  = m_stPDsensorIF.i4PitchY;
        a_sPDConfig.sPdBlockInfo.i4SubBlkH = m_stPDsensorIF.i4SubBlkH;
        a_sPDConfig.sPdBlockInfo.i4SubBlkW = m_stPDsensorIF.i4SubBlkW;
        for(MUINT32 Pidx=0; Pidx<m_stPDsensorIF.i4PairNum; Pidx++)
        {
            a_sPDConfig.sPdBlockInfo.i4PosL[Pidx][0]=m_stPDsensorIF.i4PosL[Pidx][0];
            a_sPDConfig.sPdBlockInfo.i4PosL[Pidx][1]=m_stPDsensorIF.i4PosL[Pidx][1];
            a_sPDConfig.sPdBlockInfo.i4PosR[Pidx][0]=m_stPDsensorIF.i4PosR[Pidx][0];
            a_sPDConfig.sPdBlockInfo.i4PosR[Pidx][1]=m_stPDsensorIF.i4PosR[Pidx][1];
        }
        MY_LOG_IF(0, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d\n",
            a_sPDConfig.i4Bits,
            a_sPDConfig.i4IsPacked,
            a_sPDConfig.i4RawHeight,
            a_sPDConfig.i4RawStride,
            a_sPDConfig.i4RawWidth,
            a_sPDConfig.sPdBlockInfo.i4BlockNumX,
            a_sPDConfig.sPdBlockInfo.i4BlockNumY,
            a_sPDConfig.sPdBlockInfo.i4OffsetX,
            a_sPDConfig.sPdBlockInfo.i4OffsetY,
            a_sPDConfig.sPdBlockInfo.i4PairNum,
            a_sPDConfig.sPdBlockInfo.i4PitchX,
            a_sPDConfig.sPdBlockInfo.i4PitchY,
            a_sPDConfig.sPdBlockInfo.i4SubBlkH,
            a_sPDConfig.sPdBlockInfo.i4SubBlkW
        );
        /*MINT32 kkxx;
             for(kkxx=0;kkxx<a_sPDConfig.sPdBlockInfo.i4PairNum;kkxx++)
             MY_LOG("%d %d %d %d",
             a_sPDConfig.sPdBlockInfo.i4PosL[kkxx][0],    a_sPDConfig.sPdBlockInfo.i4PosL[kkxx][1],
             a_sPDConfig.sPdBlockInfo.i4PosR[kkxx][0],a_sPDConfig.sPdBlockInfo.i4PosR[kkxx][1]);*/

        if(i4_ret==0)
        {
            i4_ret=m_pIPdAlgo->setPDBlockInfo(a_sPDConfig);
            m_pIPdAlgo->getInfoForHybridAF(m_sAFInput.i4PDInfo);
            MY_LOG("[init] PDinit %d%d\n", m_sAFInput.i4PDInfo[0], m_sAFInput.i4PDInfo[1]);
        }

        if(i4_ret)
        {
            m_isPDAF_ON=0;
            MY_LOG("[init] PD init data error, close PDAF");
        }
        for(MUINT32 Pidx=0; Pidx<16; Pidx++)    m_PDAFPureRawQu[Pidx]=-1;
        m_pdaf_pure_lens_pos = 0;

    }
    else  MY_LOG("[init] no PD algo");
    MY_LOG("PDAF_Support = %d ", m_isPDAF_ON);
    MY_LOG("[init] finish");
    m_FirstTimeAFON=0;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::uninit(MINT32 isInitMCU)
{
    MY_LOG("[uninit] +");
    MRESULT ret = S_3A_OK;

    Mutex::Autolock lock(m_Lock);
    if (m_Users <= 0)
    {
        return S_3A_OK;
    }
    android_atomic_dec(&m_Users);
    if (m_Users != 0)
    {
        MY_LOG("[uninit] Still %d users \n", m_Users);
        return S_AF_OK;
    }
    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->destroyInstance();
        m_pIAfAlgo = NULL;
    }
    if (m_pIPdAlgo)
    {
         m_pIPdAlgo->destroyInstance();
         m_pIPdAlgo = NULL;
    }

    // AF v1.2
    // --- uninit g/gyro sensor listener ---
    // i4Coef[5] == 1:enable SensorListener, else: disable
    if( (m_NVRAM_LENS.rAFNVRAM.i4SensorEnable > 0)  && (m_AndroidServiceState==TRUE) &&(gpSensorListener!=NULL))
    {
        if(gAcceValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Acc);
        }

        if(gGyroValid)
        {
            gpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
        }

        gpSensorListener->destroyInstance();
        gpSensorListener = NULL;
    }
    if ((m_pMcuDrv) && (isInitMCU))
    {
        MY_LOG("[uninitMcuDrv] - SensorDev: %d", m_i4CurrSensorDev);
        m_pMcuDrv->uninit(m_i4CurrSensorDev);
        m_pMcuDrv->destroyInstance();
        m_pMcuDrv = NULL;
    }
    #ifdef ISPREADREG
    if (m_pIspReg)   {
        m_pIspReg->uninit("af_mgr");
        m_pIspReg = NULL;
    }
    #endif
    m_i4EnableAF = -1;
    MY_LOG("[uninit] - m_Users: %d", m_Users);
    return S_AF_OK;
}

MRESULT AfMgr::CCTMCUNameinit(MINT32 i4SensorIdx)
{
    MY_LOG("[init] CCT MCU Name +");
    Mutex::Autolock lock(m_Lock);
    m_i4SensorIdx=i4SensorIdx;
    if (m_CCTUsers > 0)
    {
        MY_LOG("[CCTMCUNameinit] no init, %d has created \n", m_CCTUsers);
        android_atomic_inc(&m_CCTUsers);
        return S_3A_OK;
    }
    android_atomic_inc(&m_CCTUsers);
    SensorStaticInfo rSensorStaticInfo;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    switch(m_i4CurrSensorDev)
    {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            break;
        default:
            MY_ERR("Invalid sensor device: %d", m_i4CurrSensorDev);
    }
    m_i4CurrSensorId=rSensorStaticInfo.sensorDevID;
    MCUDrv::lensSearch(m_i4CurrSensorDev, m_i4CurrSensorId);
    m_i4CurrLensId = MCUDrv::getCurrLensID(m_i4CurrSensorDev);
    MY_LOG("[lens][SensorDev]0x%04x, [SensorId]0x%04x, [CurrLensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

    if (m_i4CurrLensId == 0xFFFF)   m_i4EnableAF = 0;
    else                            m_i4EnableAF = 1;

    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)g_pNVRAM_LENS);
    if(err!=0)    MY_ERR("AfAlgo NvBufUtil get buf fail! \n");

    MY_LOG("[init] CCT MCU Name -");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::CCTMCUNameuninit()
{
    MY_LOG("[uninit] CCT MCU Name +");
    Mutex::Autolock lock(m_Lock);

    if (m_CCTUsers <= 0)    return S_3A_OK;

    android_atomic_dec(&m_CCTUsers);
    if (m_CCTUsers != 0)
    {
        MY_LOG("[CCTMCUNameuninit] Still %d users \n", m_CCTUsers);
        return S_AF_OK;
    }

    m_i4EnableAF = -1;
    MY_LOG("[uninit] CCT MCU Name -");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFOpeartion()
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_OPERATION]\n");
    setAFMode(AF_MODE_AFS);
    triggerAF();
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPMFOpeartion(MINT32 a_i4MFpos)
{
    MINT32 i4TimeOutCnt = 0;

    MY_LOG("[ACDK_CCT_V2_OP_MF_OPERATION]\n");

    setAFMode(AF_MODE_MF);
    triggerAF();
    setMFPos(a_i4MFpos);

    while (!isFocusFinish())
    {
        usleep(5000); // 5ms
        i4TimeOutCnt++;
        if (i4TimeOutCnt > 100)
            break;
    }

    MY_LOG("[MF]pos:%d, value:%lld\n", a_i4MFpos, m_sAFInput.sAFStat.i8Stat24);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetAFInfo(MVOID *a_pAFInfo, MUINT32 *a_pOutLen)
{
    ACDK_AF_INFO_T *pAFInfo = (ACDK_AF_INFO_T *)a_pAFInfo;

    MY_LOG("[ACDK_CCT_V2_OP_GET_AF_INFO]\n");

    pAFInfo->i4AFMode = m_eLIB3A_AFMode;
    pAFInfo->i4AFMeter = LIB3A_AF_METER_SPOT;
    pAFInfo->i4CurrPos = m_sAFOutput.i4AFPos;

    *a_pOutLen = sizeof(ACDK_AF_INFO_T);

    MY_LOG("[AF Mode] = %d\n", pAFInfo->i4AFMode);
    MY_LOG("[AF Meter] = %d\n", pAFInfo->i4AFMeter);
    MY_LOG("[AF Current Pos] = %d\n", pAFInfo->i4CurrPos);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetBestPos(MINT32 *a_pAFBestPos, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_BEST_POS]%d\n", m_sAFOutput.i4AFBestPos);
    *a_pAFBestPos = m_sAFOutput.i4AFBestPos;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFCaliOperation(MVOID *a_pAFCaliData, MUINT32 *a_pOutLen)
{
    ACDK_AF_CALI_DATA_T *pAFCaliData = (ACDK_AF_CALI_DATA_T *)a_pAFCaliData;
    AF_DEBUG_INFO_T rAFDebugInfo;
    MUINT32 aaaDebugSize;
    MINT32 i4TimeOutCnt = 0;

    MY_LOG("[ACDK_CCT_V2_OP_AF_CALI_OPERATION]\n");

    setAFMode(AF_MODE_AFS);
    usleep(500000);    // 500ms
    m_eLIB3A_AFMode = LIB3A_AF_MODE_CALIBRATION;
    m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);

    usleep(500000);    // 500ms
    while(!isFocusFinish())
    {
        usleep(30000); // 30ms
        i4TimeOutCnt++;
        if (i4TimeOutCnt > 2000) break;
    }

    getDebugInfo(rAFDebugInfo);
    pAFCaliData->i4Gap = (MINT32)rAFDebugInfo.Tag[3].u4FieldValue;

    for (MINT32 i = 0; i < 512; i++)
    {
        if (rAFDebugInfo.Tag[i+4].u4FieldValue != 0)
        {
            pAFCaliData->i8Vlu[i] = (MINT64)rAFDebugInfo.Tag[i+4].u4FieldValue;  // need fix it
            pAFCaliData->i4Num = i+1;
        }
        else  break;
    }

    pAFCaliData->i4BestPos = m_sAFOutput.i4AFBestPos;

    MY_LOG("[AFCaliData] Num = %d\n", pAFCaliData->i4Num);
    MY_LOG("[AFCaliData] Gap = %d\n", pAFCaliData->i4Gap);
    for (MINT32 i=0; i<pAFCaliData->i4Num; i++)
        MY_LOG("[AFCaliData] Vlu %d = %lld\n", i, pAFCaliData->i8Vlu[i]);
    MY_LOG("[AFCaliData] Pos = %d\n", pAFCaliData->i4BestPos);

    setAFMode(AF_MODE_AFS);
    *a_pOutLen = sizeof(MINT32);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSetFocusRange(MVOID *a_pFocusRange)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_SET_RANGE]\n");
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    m_NVRAM_LENS.rFocusRange = *pFocusRange;

    if (m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos(m_NVRAM_LENS.rFocusRange.i4InfPos,m_i4CurrSensorDev);
        m_pMcuDrv->setMCUMacroPos(m_NVRAM_LENS.rFocusRange.i4MacroPos,m_i4CurrSensorDev);
    }

    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    }

    MY_LOG("[Inf Pos] = %d\n", m_NVRAM_LENS.rFocusRange.i4InfPos);
    MY_LOG("[Marco Pos] = %d\n", m_NVRAM_LENS.rFocusRange.i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFocusRange(MVOID *a_pFocusRange, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_RANGE]\n");
    FOCUS_RANGE_T *pFocusRange = (FOCUS_RANGE_T *)a_pFocusRange;

    *pFocusRange = m_NVRAM_LENS.rFocusRange;
    *a_pOutLen = sizeof(FOCUS_RANGE_T);

    MY_LOG("[Inf Pos] = %d\n", pFocusRange->i4InfPos);
    MY_LOG("[Marco Pos] = %d\n", pFocusRange->i4MacroPos);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetNVRAMParam(MVOID *a_pAFNVRAM, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_READ]\n");

    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = reinterpret_cast<NVRAM_LENS_PARA_STRUCT*>(a_pAFNVRAM);
    int err;
    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)g_pNVRAM_LENS);
    if(err!=0)
        MY_ERR("CCTOPAFGetNVRAMParam NvBufUtil get buf fail! \n");
    m_NVRAM_LENS.rFocusRange = g_pNVRAM_LENS->rFocusRange;
    m_NVRAM_LENS.rAFNVRAM= g_pNVRAM_LENS->rAFNVRAM;

    /*
    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = reinterpret_cast<NVRAM_LENS_PARA_STRUCT*>(a_pAFNVRAM);
    if (FAILED(NvramDrvMgr::getInstance().init(m_i4CurrSensorDev)))
    {
         MY_ERR("NvramDrvMgr init fail\n");
         return E_AWB_SENSOR_ERROR;
    }

    NvramDrvMgr::getInstance().getRefBuf(g_pNVRAM_LENS);
    NvramDrvMgr::getInstance().uninit();

    m_NVRAM_LENS.Version = g_pNVRAM_LENS->Version;
    m_NVRAM_LENS.rFocusRange = g_pNVRAM_LENS->rFocusRange;
    m_NVRAM_LENS.rAFNVRAM= g_pNVRAM_LENS->rAFNVRAM;
    */

    *pAFNVRAM = m_NVRAM_LENS;
    *a_pOutLen = sizeof(NVRAM_LENS_PARA_STRUCT);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFApplyNVRAMParam(MVOID *a_pAFNVRAM)
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_APPLY]\n");
    NVRAM_LENS_PARA_STRUCT *pAFNVRAM = (NVRAM_LENS_PARA_STRUCT *)a_pAFNVRAM;
    m_NVRAM_LENS = *pAFNVRAM;

    MY_LOG("Apply to Phone[Thres Main]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("Apply to Phone[Thres Sub]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("Apply to Phone[HW_TH]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_TH.i4HW_TH[0]);   // AF v2.0
    MY_LOG("Apply to Phone[Statgain]%d\n", m_NVRAM_LENS.rAFNVRAM.i4StatGain);

    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFParam(m_sAFParam, m_sAFConfig, m_NVRAM_LENS.rAFNVRAM);
    }
    if (m_pMcuDrv)
    {
        m_pMcuDrv->setMCUInfPos(m_NVRAM_LENS.rFocusRange.i4InfPos,m_i4CurrSensorDev);
        m_pMcuDrv->setMCUMacroPos(m_NVRAM_LENS.rFocusRange.i4MacroPos,m_i4CurrSensorDev);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFSaveNVRAMParam()
{
    MY_LOG("[ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM]\n");

    MY_LOG("WriteNVRAM from Phone[Thres Main]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("WriteNVRAM from Phone[Thres Sub]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_Coef.i4THRES_MAIN);
    MY_LOG("WriteNVRAM from Phone[HW_TH]%d\n", m_NVRAM_LENS.rAFNVRAM.sAF_TH.i4HW_TH[0]);    // AF v2.0
    MY_LOG("WriteNVRAM from Phone[Statgain]%d\n", m_NVRAM_LENS.rAFNVRAM.i4StatGain);
    NVRAM_LENS_PARA_STRUCT* g_pNVRAM_LENS;
    int err1;
    err1 = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev, (void*&)g_pNVRAM_LENS);
    //g_pNVRAM_LENS->Version = m_NVRAM_LENS.Version;
    g_pNVRAM_LENS->rFocusRange = m_NVRAM_LENS.rFocusRange;
    g_pNVRAM_LENS->rAFNVRAM = m_NVRAM_LENS.rAFNVRAM;
    int err2;
    err2 = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_LENS, m_i4CurrSensorDev);
    if(err1!=0)
        return err1;
    else
        return err2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetFV(MVOID *a_pAFPosIn, MVOID *a_pAFValueOut, MUINT32 *a_pOutLen)
{
    ACDK_AF_POS_T *pAFPos = (ACDK_AF_POS_T *) a_pAFPosIn;
    ACDK_AF_VLU_T *pAFValue = (ACDK_AF_VLU_T *) a_pAFValueOut;

    MY_LOG("[ACDK_CCT_V2_OP_AF_GET_FV]\n");
    pAFValue->i4Num = pAFPos->i4Num;
    setAFMode(AF_MODE_AFS);
    usleep(500000); // 500ms
    setAFMode(AF_MODE_MF);

    for (MINT32 i = 0; i < pAFValue->i4Num; i++)
    {
        setMFPos(pAFPos->i4Pos[i]);
        usleep(500000); // 500ms
        pAFValue->i8Vlu[i] = m_sAFInput.sAFStat.i8Stat24;
        MY_LOG("[FV]pos = %d, value = %lld\n", pAFPos->i4Pos[i], pAFValue->i8Vlu[i]);
    }

    setAFMode(AF_MODE_AFS);
    *a_pOutLen = sizeof(ACDK_AF_VLU_T);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFEnable()
{
    MY_LOG("[ACDK_CCT_OP_AF_ENABLE]\n");
    m_i4EnableAF = 1;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFDisable()
{
    MY_LOG("[ACDK_CCT_OP_AF_DISABLE]\n");
    m_i4EnableAF = 0;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::CCTOPAFGetEnableInfo(MVOID *a_pEnableAF, MUINT32 *a_pOutLen)
{
    MY_LOG("[ACDK_CCT_OP_AF_GET_ENABLE_INFO]%d\n", m_i4EnableAF);
    MINT32 *pEnableAF = (MINT32 *)a_pEnableAF;
    *pEnableAF = m_i4EnableAF;
    *a_pOutLen = sizeof(MINT32);
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::triggerAF()
{
    if (m_pIAfAlgo)
        m_pIAfAlgo->triggerAF();

    //m_sAFOutput.i4IsAFDone = MFALSE;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFMode(MINT32 eAFMode)
{
    //MINT32 eAFMode = a_eAFMode;

    if (m_eCurrAFMode == eAFMode)
        return S_AF_OK;

    MY_LOG("[setAFMode] %d to %d  TG(%d)\n", m_eCurrAFMode,eAFMode, m_eSensorTG);

    if(( m_eCurrAFMode==AF_MODE_AFS && eAFMode==AF_MODE_AFC) ||
       ( m_eCurrAFMode==AF_MODE_AFC && eAFMode==AF_MODE_AFS))
        m_flkwin_synccnt=2;
    m_eCurrAFMode = eAFMode;

    switch (m_eCurrAFMode) {
        case AF_MODE_AFS: // AF-Single Shot Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
            break;
        case AF_MODE_AFC: // AF-Continuous Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC;
            break;
        case AF_MODE_AFC_VIDEO: // AF-Continuous Mode (Video)
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFC_VIDEO;
            break;
        case AF_MODE_MACRO: // AF Macro Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_MACRO;
            break;
        case AF_MODE_INFINITY: // Focus is set at infinity
            m_eLIB3A_AFMode = LIB3A_AF_MODE_INFINITY;
            break;
        case AF_MODE_MF: // Manual Focus Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_MF;
            break;
        case AF_MODE_FULLSCAN: // AF Full Scan Mode
            m_eLIB3A_AFMode = LIB3A_AF_MODE_FULLSCAN;
            break;
        default:
            m_eLIB3A_AFMode = LIB3A_AF_MODE_AFS;
            break;
    }

    if (m_pIAfAlgo)
        m_pIAfAlgo->setAFMode(m_eLIB3A_AFMode);
    else
        MY_LOG("Null m_pIAfAlgo\n");

    return S_AF_OK;
}
EAfState_T AfMgr::getAFState()
{
    EAfState_T af_state=E_AF_INACTIVE;
    if (m_pIAfAlgo)
        af_state=m_pIAfAlgo->getAFState();
    else
        MY_LOG("Null m_pIAfAlgo\n");
    return af_state;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setAFArea(CameraFocusArea_T a_sAFArea)
{
    MY_LOG_IF((m_bDebugEnable&1), "[setAFArea][Cnt]%d [L]%d [R]%d [U]%d [B]%d\n", a_sAFArea.u4Count, a_sAFArea.rAreas[0].i4Left, a_sAFArea.rAreas[0].i4Right, a_sAFArea.rAreas[0].i4Top, a_sAFArea.rAreas[0].i4Bottom);

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC)
        a_sAFArea.u4Count = 0;

    if (m_tcaf_mode && a_sAFArea.u4Count==0)
        return S_AF_OK;

    if ((a_sAFArea.u4Count == 0) || (a_sAFArea.rAreas[0].i4Left == a_sAFArea.rAreas[0].i4Right) || (a_sAFArea.rAreas[0].i4Top == a_sAFArea.rAreas[0].i4Bottom))
    {
        a_sAFArea.u4Count = 1;
        a_sAFArea.rAreas[0].i4Left   = -1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W / 100;
        a_sAFArea.rAreas[0].i4Right  =  1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_W / 100;
        a_sAFArea.rAreas[0].i4Top    = -1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H / 100;
        a_sAFArea.rAreas[0].i4Bottom =  1000 * m_NVRAM_LENS.rAFNVRAM.i4SPOT_PERCENT_H / 100;
    }

    if ((m_Users>0) && (memcmp(&m_CameraFocusArea, &a_sAFArea, sizeof(m_CameraFocusArea)) == 0))
        return S_AF_OK;

    m_CameraFocusArea = a_sAFArea;
    if (a_sAFArea.u4Count >= AF_WIN_NUM_SPOT)
        m_sAFInput.sAFArea.i4Count = AF_WIN_NUM_SPOT;
    else
        m_sAFInput.sAFArea.i4Count = a_sAFArea.u4Count;

    if (m_sAFInput.sAFArea.i4Count != 0)
    {
        for (MINT32 i=0; i<m_sAFInput.sAFArea.i4Count; i++)
        {
            a_sAFArea.rAreas[i].i4Left   = (a_sAFArea.rAreas[i].i4Left   +1000) * m_sAFInput.sEZoom.i4W  / 2000 + m_sAFInput.sEZoom.i4X;
            a_sAFArea.rAreas[i].i4Right  = (a_sAFArea.rAreas[i].i4Right  +1000) * m_sAFInput.sEZoom.i4W  / 2000 + m_sAFInput.sEZoom.i4X;
            a_sAFArea.rAreas[i].i4Top    = (a_sAFArea.rAreas[i].i4Top    +1000) * m_sAFInput.sEZoom.i4H  / 2000 + m_sAFInput.sEZoom.i4Y;
            a_sAFArea.rAreas[i].i4Bottom = (a_sAFArea.rAreas[i].i4Bottom +1000) * m_sAFInput.sEZoom.i4H  / 2000 + m_sAFInput.sEZoom.i4Y;

            m_sAFInput.sAFArea.sRect[i].i4W = a_sAFArea.rAreas[i].i4Right - a_sAFArea.rAreas[i].i4Left;
            m_sAFInput.sAFArea.sRect[i].i4H = a_sAFArea.rAreas[i].i4Bottom - a_sAFArea.rAreas[i].i4Top;
            m_sAFInput.sAFArea.sRect[i].i4X = a_sAFArea.rAreas[i].i4Left;
            m_sAFInput.sAFArea.sRect[i].i4Y = a_sAFArea.rAreas[i].i4Top;
            m_sAFInput.sAFArea.sRect[i].i4Info   = a_sAFArea.rAreas[i].i4Weight;
        }
    }

    if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_AFS) || (m_eLIB3A_AFMode == LIB3A_AF_MODE_MACRO))
    {
        m_LastAFArea.i4X = m_sAFInput.sAFArea.sRect[0].i4X;
        m_LastAFArea.i4Y = m_sAFInput.sAFArea.sRect[0].i4Y;
        m_LastAFArea.i4W = m_sAFInput.sAFArea.sRect[0].i4W;
        m_LastAFArea.i4H = m_sAFInput.sAFArea.sRect[0].i4H;
        m_LastAFArea.i4Info = 1;    // taf
    }

    MY_LOG("[setAFArea][Cnt]%d [L]%d [R]%d [U]%d [B]%d to [Cnt]%d [W]%d [H]%d [X]%d [Y]%d\n", a_sAFArea.u4Count, a_sAFArea.rAreas[0].i4Left, a_sAFArea.rAreas[0].i4Right, a_sAFArea.rAreas[0].i4Top, a_sAFArea.rAreas[0].i4Bottom, m_sAFInput.sAFArea.i4Count, m_sAFInput.sAFArea.sRect[0].i4W, m_sAFInput.sAFArea.sRect[0].i4H, m_sAFInput.sAFArea.sRect[0].i4X, m_sAFInput.sAFArea.sRect[0].i4Y);

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setCamMode(MINT32 a_eCamMode)
{
    MY_LOG("[setCamMode]%d\n", a_eCamMode);
    m_tcaf_mode=0;
    m_sAFInput.i4IsVDO= FALSE;

    if(a_eCamMode == eAppMode_VideoMode)
    {
        m_sAFInput.i4IsVDO= TRUE;
        m_sAFInput.i4IsZSD = FALSE;
    }
    else
        m_sAFInput.i4IsVDO= FALSE;

    if(m_sAFParam.i4AFS_MODE==2)  //AF auto mode always don't AFC
        m_sAFInput.i4IsVDO= FALSE;

    MINT32 AFtbl_num = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
    m_DAF_TBL.af_dac_min = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[0];
    m_DAF_TBL.af_dac_max = m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos[AFtbl_num-1];
    if( (m_sAFInput.i4IsVDO==TRUE) && (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO))
    {
        AFtbl_num = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
        m_DAF_TBL.af_dac_min = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos[0];
        m_DAF_TBL.af_dac_max = m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset + m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos[AFtbl_num-1];
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setZoomWinInfo(MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height)
{
    MY_LOG("[setZoomWinInfo][w]%d[h]%d[x]%d[y]%d\n", u4Width, u4Height, u4XOffset, u4YOffset);
    m_RawZoomInfo.i4X = (MINT32)u4XOffset;
    m_RawZoomInfo.i4Y = (MINT32)u4YOffset;
    m_RawZoomInfo.i4W = (MINT32)u4Width;
    m_RawZoomInfo.i4H = (MINT32)u4Height;
    return S_AF_OK;
}
MVOID AfMgr::applyZoomInfo()
{
    if (  ((m_sEZoom.i4X == m_RawZoomInfo.i4X) && (m_sEZoom.i4Y == m_RawZoomInfo.i4Y)
        &&(m_sEZoom.i4W == m_RawZoomInfo.i4W) && (m_sEZoom.i4H == m_RawZoomInfo.i4H)) || (m_i4EnableAF == 0))
    {
        return;
    }
    m_sEZoom.i4X = m_RawZoomInfo.i4X;
    m_sEZoom.i4Y = m_RawZoomInfo.i4Y;
    m_sEZoom.i4W = m_RawZoomInfo.i4W;
    m_sEZoom.i4H = m_RawZoomInfo.i4H;
    m_tcaf_mode=0;

    if ((m_sEZoom.i4X == 0) || (m_sEZoom.i4Y == 0))
    {
        if (m_i4AF_in_Hsize != m_sEZoom.i4W)
            m_i4Factor = m_i4AF_in_Hsize * 100 / m_sEZoom.i4W;
        else
            m_i4Factor = 100;
    }
    /* real digital factor, m_i4Factor is always 1.00 */
    MINT32 i4DzFactor = 100*(2*m_sEZoom.i4X + m_sEZoom.i4W)/m_sEZoom.i4W; /* 1X-> 100, 2X->200, ... */
    /* init window config from customized parameter */
    MINT32 i4WinCfg=(m_NVRAM_LENS.rAFNVRAM.i4ZoomInWinChg); // AF v2.0
    /* error check */
    if(i4WinCfg<1) i4WinCfg = 1; /*i4WinCfg=0 --> 1 & error check*/
    if(i4WinCfg>4) i4WinCfg = 4; /*error check*/
    if(i4WinCfg==4)
    {
        MY_LOG("[applyZoomInfo] always DZ, DZ=%d, Bound=%d \n",i4DzFactor,i4WinCfg*100);
        /* i4WinCfg=4-> DigZoomFac>4, AF win no change */
        m_sAFInput.sEZoom.i4X = m_sEZoom.i4X * m_i4Factor / 100;
        m_sAFInput.sEZoom.i4Y = m_sEZoom.i4Y * m_i4Factor / 100;
        m_sAFInput.sEZoom.i4W = m_sEZoom.i4W * m_i4Factor / 100;
        m_sAFInput.sEZoom.i4H = m_sEZoom.i4H * m_i4Factor / 100;
    }
    else /* i4WinCfg = 1~3 */
    {
        /* i4WinCfg=1-> DigZoomFac>1, AF win no change */
        /* i4WinCfg=2-> DigZoomFac>2, AF win no change */
        /* i4WinCfg=3-> DigZoomFac>3, AF win no change */
        if(i4DzFactor >= i4WinCfg*100) /* fix to upper bound */
        {
            MY_LOG("[applyZoomInfo] >bound2fix, DZ=%d, Bound=%d\n",i4DzFactor,i4WinCfg*100);
            m_sAFInput.sEZoom.i4W = (2*m_sEZoom.i4X + m_sEZoom.i4W)/i4WinCfg ;
            m_sAFInput.sEZoom.i4H = (2*m_sEZoom.i4Y + m_sEZoom.i4H)/i4WinCfg ;
            m_sAFInput.sEZoom.i4X = ((2*m_sEZoom.i4X + m_sEZoom.i4W)-m_sAFInput.sEZoom.i4W)/2;
            m_sAFInput.sEZoom.i4Y = ((2*m_sEZoom.i4Y + m_sEZoom.i4H)-m_sAFInput.sEZoom.i4H)/2;
        }
        else /* (i4DzFactor < i4WinCfg*100), AF win change aligning to digital zoom factor */
        {
            MY_LOG("[applyZoomInfo] <bound2DZ, DZ=%d, Bound=%d \n",i4DzFactor,i4WinCfg*100);
            m_sAFInput.sEZoom.i4X = m_sEZoom.i4X ;
            m_sAFInput.sEZoom.i4Y = m_sEZoom.i4Y ;
            m_sAFInput.sEZoom.i4W = m_sEZoom.i4W ;
            m_sAFInput.sEZoom.i4H = m_sEZoom.i4H ;
        }
    }
    m_CameraFocusArea.u4Count = 0;
    setAFArea(m_CameraFocusArea);

    MY_LOG("[applyZoomInfo][w]%d[h]%d[x]%d[y]%d to [w]%d[h]%d[x]%d[y]%d\n", m_RawZoomInfo.i4W, m_RawZoomInfo.i4H, m_RawZoomInfo.i4X, m_RawZoomInfo.i4Y, m_sAFInput.sEZoom.i4W, m_sAFInput.sEZoom.i4H, m_sAFInput.sEZoom.i4X, m_sAFInput.sEZoom.i4Y);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFDInfo(MVOID* a_sFaces)
{
    MtkCameraFaceMetadata *pFaces = (MtkCameraFaceMetadata *)a_sFaces;

    if(pFaces == NULL)
    {
    MY_LOG("[%s()] FD window is NULL pointer 1\n", __FUNCTION__);
    return S_AF_OK;
    }

    if((pFaces->faces) == NULL)
    {
    MY_LOG("[%s()] FD window is NULL pointer 2\n", __FUNCTION__);
    return S_AF_OK;
    }

    m_tcaf_mode=0;
    CameraFocusArea_T sFDArea;

    sFDArea.rAreas[0].i4Left   = (pFaces->faces->rect[0]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sFDArea.rAreas[0].i4Right  = (pFaces->faces->rect[2]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sFDArea.rAreas[0].i4Top    = (pFaces->faces->rect[1]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;
    sFDArea.rAreas[0].i4Bottom = (pFaces->faces->rect[3]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;

    m_FDArea.i4Count = (MINT32)pFaces->number_of_faces;
    m_FDArea.sRect[0].i4X = sFDArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4Y = sFDArea.rAreas[0].i4Top;
    m_FDArea.sRect[0].i4W = sFDArea.rAreas[0].i4Right - sFDArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4H = sFDArea.rAreas[0].i4Bottom - sFDArea.rAreas[0].i4Top;
    MINT32 i4WinPercent = (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent&0xFFFF);
    if (i4WinPercent > 150)   {i4WinPercent = 150;}

    m_FDArea.sRect[0].i4X = m_FDArea.sRect[0].i4X + m_FDArea.sRect[0].i4W * ((100 - i4WinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4Y = m_FDArea.sRect[0].i4Y + m_FDArea.sRect[0].i4H * ((100 - i4WinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4W = m_FDArea.sRect[0].i4W * (i4WinPercent) / 100;
    m_FDArea.sRect[0].i4H = m_FDArea.sRect[0].i4H * (i4WinPercent) / 100;

    if ((m_FDArea.sRect[0].i4W == 0) || (m_FDArea.sRect[0].i4H == 0))   {
        m_FDArea.i4Count = 0;
    }

    if (m_FDArea.i4Count != 0) {
        MY_LOG("[setFDInfo]cnt:%d, X:%d Y:%d W:%d H:%d", m_FDArea.i4Count, m_FDArea.sRect[0].i4X, m_FDArea.sRect[0].i4Y, m_FDArea.sRect[0].i4W, m_FDArea.sRect[0].i4H);
    }

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setFDWin(m_FDArea);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }

    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setOTInfo(MVOID* a_sOT)
{
    MtkCameraFaceMetadata *pObtinfo = (MtkCameraFaceMetadata *)a_sOT;
    m_tcaf_mode=0;
    m_FDArea.i4Score=pObtinfo->faces->score;

    CameraFocusArea_T sOBTArea;

    sOBTArea.rAreas[0].i4Left   = (pObtinfo->faces->rect[0]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sOBTArea.rAreas[0].i4Right  = (pObtinfo->faces->rect[2]+1000) * m_sEZoom.i4W / 2000 + m_sEZoom.i4X;
    sOBTArea.rAreas[0].i4Top    = (pObtinfo->faces->rect[1]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;
    sOBTArea.rAreas[0].i4Bottom = (pObtinfo->faces->rect[3]+1000) * m_sEZoom.i4H / 2000 + m_sEZoom.i4Y;

    m_FDArea.i4Count = (MINT32)pObtinfo->number_of_faces;
    m_FDArea.sRect[0].i4X = sOBTArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4Y = sOBTArea.rAreas[0].i4Top;
    m_FDArea.sRect[0].i4W = sOBTArea.rAreas[0].i4Right - sOBTArea.rAreas[0].i4Left;
    m_FDArea.sRect[0].i4H = sOBTArea.rAreas[0].i4Bottom - sOBTArea.rAreas[0].i4Top;

    MINT32 i4WinPercent = (m_NVRAM_LENS.rAFNVRAM.i4FDWinPercent&0xFFFF);
    if (i4WinPercent > 100)   {i4WinPercent = 100;}

    m_FDArea.sRect[0].i4X = m_FDArea.sRect[0].i4X + m_FDArea.sRect[0].i4W * ((100 - i4WinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4Y = m_FDArea.sRect[0].i4Y + m_FDArea.sRect[0].i4H * ((100 - i4WinPercent)>>1) / 100;
    m_FDArea.sRect[0].i4W = m_FDArea.sRect[0].i4W * (i4WinPercent) / 100;
    m_FDArea.sRect[0].i4H = m_FDArea.sRect[0].i4H * (i4WinPercent) / 100;

    if ((m_FDArea.sRect[0].i4W == 0) || (m_FDArea.sRect[0].i4H == 0))   {
        m_FDArea.i4Count = 0;
    }

    MY_LOG("[setOTInfo]cnt:%d, X:%d Y:%d W:%d H:%d", m_FDArea.i4Count, m_FDArea.sRect[0].i4X, m_FDArea.sRect[0].i4Y, m_FDArea.sRect[0].i4W, m_FDArea.sRect[0].i4H);

    if (m_pIAfAlgo)  {
        m_pIAfAlgo->setFDWin(m_FDArea);
    }
    else   {
        MY_LOG("Null m_pIAfAlgo\n");
    }

    return S_AF_OK;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFMaxAreaNum()
{
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    SensorStaticInfo rSensorStaticInfo;
    switch(m_i4CurrSensorDev)
    {
        case ESensorDev_Main:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            break;
        case ESensorDev_Sub:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            break;
        case ESensorDev_MainSecond:
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            break;
        default:
            MY_ERR("Invalid sensor device: %d", m_i4CurrSensorDev);
    }
    m_i4CurrSensorId=rSensorStaticInfo.sensorDevID;
    MCUDrv::lensSearch(m_i4CurrSensorDev, m_i4CurrSensorId);
    m_i4CurrLensId = MCUDrv::getCurrLensID(m_i4CurrSensorDev);
    MY_LOG("[Dev]0x%04x, [SensorId]0x%04x, [LensId]0x%04x", m_i4CurrSensorDev, m_i4CurrSensorId, m_i4CurrLensId);

    if (m_i4CurrLensId == 0xFFFF)
    {
        MY_LOG("[getAFMaxAreaNum][AF disable]%d\n", 0);
        return 0;
    }
    else
    {
        if (m_i4EnableAF == 0)
        {
            MY_LOG("Open AF driver fail!\n");
            return 0;
        }
        else
        {
            MY_LOG("[getAFMaxAreaNum]%d\n", AF_WIN_NUM_SPOT);
            return AF_WIN_NUM_SPOT;
        }
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMaxLensPos()
{
#if 1
    return 1023;
#else
    if (m_pMcuDrv)
    {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo(&MotorInfo,m_i4CurrSensorDev);
        MY_LOG("[getMaxLensPos]%d\n", (MINT32)MotorInfo.u4MacroPosition);
        return (MINT32)MotorInfo.u4MacroPosition;
    }
    else
    {
        MY_LOG("[getMaxLensPos]m_pMcuDrv NULL\n");
        return 0;
    }
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getMinLensPos()
{
#if 1
    return 0;
#else
    if (m_pMcuDrv)
    {
        mcuMotorInfo MotorInfo;
        m_pMcuDrv->getMCUInfo(&MotorInfo,m_i4CurrSensorDev);
        MY_LOG("[getMinLensPos]%d\n", (MINT32)MotorInfo.u4InfPosition);
        return (MINT32)MotorInfo.u4InfPosition;
    }
    else
    {
        MY_LOG("[getMinLensPos]m_pMcuDrv NULL\n");
        return 0;
    }
#endif
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFBestPos()
{
    return m_sAFOutput.i4AFBestPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFPos()
{
    return m_sAFOutput.i4AFPos;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFStable()
{
    return m_sAFOutput.i4IsAFDone;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableOffset()
{
// AF v2.0
//    if (m_sAFInput.i4IsZSD)
//    {
//        return m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4Offset;
//    }
//    else
//    {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
            return m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Offset;
        else
            return m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Offset;
//    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableMacroIdx()
{
// AF v2.0
//    if (m_sAFInput.i4IsZSD)
//    {
//        return m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4NormalNum;
//    }
//    else
//    {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
            return m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4NormalNum;
        else
            return m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4NormalNum;
//    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::getAFTableIdxNum()
{
    return AF_TABLE_NUM;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID* AfMgr::getAFTable()
{
// AF v2.0
//    if (m_sAFInput.i4IsZSD)
//    {
//        return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sZSD_AF_Coef.sTABLE.i4Pos;
//    }
//    else
//    {
        if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
            return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sVAFC_Coef.sTABLE.i4Pos;

        else
            return (MVOID*)m_NVRAM_LENS.rAFNVRAM.sAF_Coef.sTABLE.i4Pos;
//    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setMFPos(MINT32 a_i4Pos)
{
    MY_LOG("[setMF]%d\n", a_i4Pos);
    if ((m_eLIB3A_AFMode == LIB3A_AF_MODE_MF) && ((m_sAFOutput.i4AFPos != a_i4Pos)||(m_FirstTimeAFON==0)))
    {
        m_FirstTimeAFON=1;
        MY_LOG("[setMFPos]%d\n", a_i4Pos);
        m_i4MFPos = a_i4Pos;

        if (a_i4Pos==(MINT32)0xFFFFFFFF)
        {
            MY_LOG("[setMFPos]skip set mf pos\n");
            return S_AF_OK;
        }

        if (m_pIAfAlgo)
            m_pIAfAlgo->setMFPos(m_i4MFPos);
        else
            MY_LOG("Null m_pIAfAlgo\n");
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setFullScanstep(MINT32 a_i4Step)
{
    if (m_sAFInput.i4FullScanStep != a_i4Step)
    {
        MY_LOG("[setFullScanstep]%d\n", a_i4Step);
        m_sAFInput.i4FullScanStep = a_i4Step;
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T AfMgr::getFLKStat()
{
     if(m_i4FLKValid==0)    m_sAFFullStat.bValid=MTRUE;
     else                   m_sAFFullStat.bValid=MFALSE;
     MY_LOG_IF((m_bDebugEnable&1), "[getFLKStat]bValid %d\n",m_sAFFullStat.bValid);

    return m_sAFFullStat;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MVOID AfMgr::getAFRefWin(CameraArea_T &rWinSize)
{
    if (m_i4EnableAF == 0)
    {
        MY_LOG("[getAFRefWin] disableAF");
        rWinSize.i4Left   = 0;
        rWinSize.i4Right  = 0;
        rWinSize.i4Top    = 0;
        rWinSize.i4Bottom = 0;
        return;
    }
    rWinSize.i4Left   = m_CameraFocusArea.rAreas[0].i4Left;
    rWinSize.i4Right  = m_CameraFocusArea.rAreas[0].i4Right;
    rWinSize.i4Top    = m_CameraFocusArea.rAreas[0].i4Top;
    rWinSize.i4Bottom = m_CameraFocusArea.rAreas[0].i4Bottom;
}

MVOID AfMgr::setAE2AFInfo(AE2AFInfo_T rAEInfo)
{

    MY_LOG_IF((m_bDebugEnable&1),"setAE2AFInfo");
    if (m_i4EnableAF == 0)
    {
        m_sAFInput.i8GSum = 50;
        m_sAFInput.i4ISO = 100;
        m_sAFInput.i4IsAEStable = 1;
        m_sAFInput.i4SceneLV = 80;
        return;
    }

    m_sAFInput.i8GSum =rAEInfo.iYvalue;
    m_sAFInput.i4ISO = rAEInfo.i4ISO;
    m_sAFInput.i4IsAEStable = rAEInfo.i4IsAEStable;
    m_sAFInput.i4SceneLV = rAEInfo.i4SceneLV;
    m_sAFInput.i4ShutterValue = rAEInfo.ishutterValue;
    memcpy(m_aeBlockV, rAEInfo.aeBlockV,25);

    m_sAFInput.i4IsFlashFrm      = rAEInfo.i4IsFlashFrm;
    m_sAFInput.i4AEBlockAreaYCnt = rAEInfo.i4AEBlockAreaYCnt;
    m_sAFInput.pAEBlockAreaYvalue= rAEInfo.pAEBlockAreaYvalue;
}
MRESULT AfMgr::doAF(MVOID *pAFStatBuf)
{
    MY_LOG_IF((m_bDebugEnable&1), "[Signal] doAF");
    if (m_i4EnableAF == 0)
    {
        m_sAFOutput.i4IsAFDone = 1;
        m_sAFOutput.i4IsFocused = 0;
        m_i4LastFocusModeTAF= FALSE;
        m_sAFOutput.i4AFPos = 0;
        MY_LOG("disableAF");
        return S_AF_OK;
    }

    m_pdaf_pure_lens_pos = m_sAFOutput.i4AfDacIndex; //m_sAFOutput.i4AFPos;
    if(PDAF_SCAN_MODE > 1)
        m_pdaf_pure_lens_pos = m_sAFOutput.i4AFPos;

    MUINT32 i4curFrmNum = m_cur_frm_num;
    if(m_cur_frm_num>0)
    {
        if((m_bpdctrl_flg==1) && (m_prv_frm_num != m_cur_frm_num))
            m_PDAFPureRawQu[(m_cur_frm_num-1)%16]=-1;
        if(m_PDAFPureRawQu[(m_cur_frm_num-1)%16] != -1 )
        {
            m_sAFInput.i4PDPureRawfrm = m_PDAFPureRawQu[(m_cur_frm_num-1)%16];
            m_PDAFPureRawQu[(m_cur_frm_num-1)%16]=-1;
            MY_LOG("[frm_] %d [get_frm_ststus] %d\n",i4curFrmNum-1,  m_sAFInput.i4PDPureRawfrm);
            m_bpdctrl_flg=1;
        }
        else
        {
            m_sAFInput.i4PDPureRawfrm = m_PDAFPureRawQu[m_cur_frm_num%16];
            m_PDAFPureRawQu[m_cur_frm_num%16]=-1;
            MY_LOG("[frm_] %d [get_frm_ststus] %d\n",i4curFrmNum,  m_sAFInput.i4PDPureRawfrm);
            m_bpdctrl_flg=0;
        }
    }
    else
    {
        m_sAFInput.i4PDPureRawfrm = -1;
        MY_LOG("[frm_] %d [get_frm_ststus] %d\n",i4curFrmNum,  m_sAFInput.i4PDPureRawfrm);
    }

    //if( (m_sAFInput.i4IsVDO==TRUE) && (m_sAFOutput.i4IsAFDone==TRUE ) return S_AF_OK;
    if( (m_sAFInput.i4IsVDO!=TRUE) || (m_sAFOutput.i4IsAFDone==FALSE ))
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.af_motor.disable", value, "0");
        m_bMotorDisable = atoi(value);
        property_get("debug.af_motor.position", value, "0");
        m_MotorManual = atoi(value);
        property_get("debug.af_ois.disable", value, "0");
        m_OisDisable = atoi(value);
        //MY_LOG("i4IsVDO=%d, i4IsZSD=%d\n", m_sAFInput.i4IsVDO, m_sAFInput.i4IsZSD);

        if ((m_pMcuDrv) &&  (m_OisDisable&4))
        {
            m_pMcuDrv->setMCUParam(m_OisDisable,m_i4CurrSensorDev);
        }
        printAFConfigLog0();
        printAFConfigLog1();
    }
    // AF v1.2
    // i4Coef[7] > 0: enable new scenechange , else: disable
    if( (m_NVRAM_LENS.rAFNVRAM.i4SceneMonitorLevel > 0)
      ||(m_NVRAM_LENS.rAFNVRAM.i4VdoSceneMonitorLevel > 0) )
    {
        m_pIAfAlgo->setAEBlockInfo(m_aeBlockV, 25);
    }

    // i4Coef[5] == 1:enable SensorListener, 0: disable
    if(m_NVRAM_LENS.rAFNVRAM.i4SensorEnable > 0)
    {
        MY_LOG_IF((m_bDebugEnable&1),"SensorEventAF gAcceValid=%d, gGyroValid=%d\n", gAcceValid, gGyroValid);
        gListenLogOn = m_bDebugEnable&1;
        if(gAcceValid)
        {
            m_pIAfAlgo->setAcceSensorInfo(gAcceInfo, SENSOR_ACCE_SCALE);
        }
        if(gGyroValid)
        {
            m_pIAfAlgo->setGyroSensorInfo(gGyroInfo, SENSOR_GYRO_SCALE);
        }
    }
    getLensInfo(m_sAFInput.sLensInfo);

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
    {
        if (m_sAFOutput.i4IsMonitorFV == TRUE)
            m_sAFInput.sAFStat = Trans4WintoOneStat(pAFStatBuf);
        else
            m_sAFInput.sAFStat = TransAFtoOneStat(pAFStatBuf);

            m_LastAFArea.i4X = m_sAFInput.sAFArea.sRect[0].i4X;
            m_LastAFArea.i4Y = m_sAFInput.sAFArea.sRect[0].i4Y;
            m_LastAFArea.i4W = m_sAFInput.sAFArea.sRect[0].i4W;
            m_LastAFArea.i4H = m_sAFInput.sAFArea.sRect[0].i4H;
            m_LastAFArea.i4Info = 0;    // monitor
        MY_LOG("[LastAFArea] [x] %d [Y] %d [W] %d [H] %d [Info]\n", m_LastAFArea.i4X, m_LastAFArea.i4Y, m_LastAFArea.i4W, m_LastAFArea.i4H, m_LastAFArea.i4Info);


    }
    else    //AFS mode or else
    {
        m_FDArea.i4Count = 0;
        m_sAFOutput.i4FDDetect=0;
        if((m_sAFOutput.i4IsAFDone==0) && (m_flkwin_synccnt==0)) //doing AF
            m_sAFInput.sAFStat = TransAFtoOneStat(pAFStatBuf);
        else
            m_sAFInput.sAFStat = Trans4WintoOneStat(pAFStatBuf);
    }
    //MY_LOG("LastAFWin type[%d] size %d %d %d %d\n", m_LastAFArea.i4Info, m_LastAFArea.i4W, m_LastAFArea.i4H, m_LastAFArea.i4X, m_LastAFArea.i4Y);

    m_sAFFullStat = TransToFullStat(pAFStatBuf);

    m_AFStatusTbl[i4curFrmNum%16][0]=i4curFrmNum;
    m_AFStatusTbl[i4curFrmNum%16][1]=(MUINT32)m_sAFOutput.i4IsAFDone;

    //depth AF for algo data
    m_DAF_TBL.curr_p1_frm_num=i4curFrmNum;
    if(m_DAF_TBL.is_daf_run==1) m_sAFInput.i4HybridAFMode = 1;
    else                        m_sAFInput.i4HybridAFMode = 0;

    if(m_isPDAF_ON==1)          m_sAFInput.i4HybridAFMode = 2;

    if(m_DAF_TBL.is_daf_run==1)
    {
        m_sAFInput.i4CurrP1FrmNum = i4curFrmNum;
        m_sAFInput.i4DafDacIndex  = m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_dac_index;
        m_sAFInput.i4DafConfidence= m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_confidence;
        MY_LOG("DAF--[Mode]%d [cp1#]%d [cp2#]%d [nextF#]%d [DafDac]%d [DafConf]%d [daf_dist]%d\n",
        (MINT32)m_sAFInput.i4HybridAFMode,
        (MINT32)m_sAFInput.i4CurrP1FrmNum,
        (MINT32)m_DAF_TBL.curr_p2_frm_num,
        (MINT32)m_next_query_FrmNum,
        (MINT32)m_sAFInput.i4DafDacIndex,
        (MINT32)m_sAFInput.i4DafConfidence,
        m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_distance);

        if(m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_confidence)
        {
            MY_LOG("DAFAA-%d %d\n", m_daf_distance, m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_confidence);
            m_daf_distance = (MINT32)m_DAF_TBL.daf_vec[m_next_query_FrmNum % DAF_TBL_QLEN].daf_distance;
        }
    }
    //PDAF
    if ((m_pIPdAlgo!=NULL) && (m_isPDAF_ON==1))
    {
        // AF v2.1 HybridAF MUL
        for(MINT32 i = 0 ; i < AF_PSUBWIN_NUM ; i++)
        {
            m_sAFInput.i4PDafDacIndex[i]=m_sPDOutput[i].i4FocusLensPos;
            m_sAFInput.i4PDafConfidence[i]=m_sPDOutput[i].i4ConfidenceLevel;
            m_sAFInput.i4PDafConverge[i]= (MINT32)(m_sPDOutput[i].fPdValue*1000);
        }
    }
    //MY_LOG("[PDAF]1 [Confidence] %d [LensPos] %d [value] %d\n", m_sPDOutput.i4ConfidenceLevel,m_sPDOutput.i4FocusLensPos, m_sAFInput.i4PDafConverge);
    if( m_flkwin_synccnt>0)
    {
        MY_LOG("AF win Preparing\n");
    }
  else
    {
        if (m_pIAfAlgo)
            m_pIAfAlgo->handleAF(m_sAFInput, m_sAFOutput);
        else
            MY_LOG("Null m_pIAfAlgo\n");

    if(m_DAF_TBL.is_daf_run==1)
    {
        //depth AF after algo data
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].frm_mun        = i4curFrmNum;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].is_learning    = m_sAFOutput.i4IsLearning;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].is_querying    = m_sAFOutput.i4IsQuerying;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_valid       = m_sAFOutput.i4AfValid;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_dac_index   = m_sAFOutput.i4AfDacIndex;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_confidence  = m_sAFOutput.i4AfConfidence;

        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_start_x = (MUINT16)m_LastAFArea.i4X;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_start_y = (MUINT16)m_LastAFArea.i4Y;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_end_x   = (MUINT16)m_LastAFArea.i4X + m_LastAFArea.i4W;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_end_y   = (MUINT16)m_LastAFArea.i4Y + m_LastAFArea.i4H;

        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].daf_dac_index  = 0;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].daf_confidence = 0;
        m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].daf_distance   = 0;

        if(1 == m_sAFOutput.i4IsQuerying) {
            m_DAF_TBL.is_query_happen = m_sAFOutput.i4QueryFrmNum;
//            StereoHal::getInstance()->STEREODepthAFQuery();
        }

        m_next_query_FrmNum = m_sAFOutput.i4QueryFrmNum;

        MY_LOG("DAF--[islrn]%d [isqry]%d [afVld]%d [af_dac]%d [afConf]%d [af_win]%d %d %d %d [nextF#]%d\n",
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].is_learning,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].is_querying,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_valid,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_dac_index,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_confidence,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_start_x,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_start_y,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_end_x,
        (MINT32)m_DAF_TBL.daf_vec[i4curFrmNum % DAF_TBL_QLEN].af_win_end_y,
        (MINT32)m_next_query_FrmNum);
    }

    if (m_pMcuDrv)
    {
        MY_LOG("[AFStatH]%lld [AFStatV]%lld [moveMCUTG%d] %d", m_sAFInput.sAFStat.i8Stat24, m_sAFInput.sAFStat.i8StatV, m_eSensorTG,m_sAFOutput.i4AFPos);
        if(m_bMotorDisable==0)
            m_pMcuDrv->moveMCU(m_sAFOutput.i4AFPos,m_i4CurrSensorDev);
        else
            MY_LOG("[ADB Disable Motor]\n");

        if((m_MotorManual!=0) && (m_bMotorDisable==1))
            m_pMcuDrv->moveMCU(m_MotorManual,m_i4CurrSensorDev);
    }
    else    MY_LOG("Null m_pMcuDrv\n");

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
    {
        if (m_sAFOutput.sAFArea.sRect[0].i4Info != m_i4AFPreStatus)
        {
            if (m_pAFCallBack)
            {
                MY_LOG("Callback notify [pre]%d [now]%d", m_i4AFPreStatus, m_sAFOutput.sAFArea.sRect[0].i4Info);

                // pl v2.0 ; debug and simulation
                char value[PROPERTY_VALUE_MAX] = {'\0'};
                property_get("debug.PLWFile.enable", value, "0");
                PL_WdataFile = atoi(value);
        if(PL_WdataFile==2)
                {
                if(2 == m_sAFOutput.sAFArea.sRect[0].i4Info)
                {
                    m_pMcuDrv->moveMCU(1023,m_i4CurrSensorDev);
                }
            }

                m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, !m_sAFOutput.i4IsAFDone, 0, 0);
                m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
                m_pAFCallBack->doDataCb(I3ACallBack::eID_DATA_AF_FOCUSED, &m_sAFOutput.sAFArea, sizeof(m_sAFOutput.sAFArea));
                m_i4LastFocusModeTAF= FALSE;
            }
            m_i4AFPreStatus = m_sAFOutput.sAFArea.sRect[0].i4Info;
        }
    }
    else
        m_i4AFPreStatus = AF_MARK_NONE;
  }
    setAF_IN_HSIZE();

    setAFWinTH(m_sAFOutput.sAFStatConfig);
    setGMR(m_sAFOutput.sAFStatConfig);
    applyZoomInfo();

    if (m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC || m_eLIB3A_AFMode == LIB3A_AF_MODE_AFC_VIDEO)
    {
        MY_LOG_IF((m_bDebugEnable&1), "CAF Preparing monitor %d, afdone %d\n", m_sAFOutput.i4IsMonitorFV, m_sAFOutput.i4IsAFDone);
        if (m_sAFOutput.i4IsMonitorFV == TRUE)
            setFlkWinConfig();
        else
            setAFWinConfig(m_sAFOutput.sAFArea);
    }
    else    //AFS mode or else
    {
        // AF v2.1 HybridAF MUL
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("scan.pd.enable", value, "0");
        PDAF_SCAN_MODE = atoi(value);

        m_sAFOutput.i4FDDetect=0;
        MY_LOG_IF((m_bDebugEnable&1), "TAF Preparing monitor %d, afdone %d\n", m_sAFOutput.i4IsMonitorFV, m_sAFOutput.i4IsAFDone);
        if(m_sAFOutput.i4IsAFDone==0)  //Doing AF
                setAFWinConfig(m_sAFOutput.sAFArea);
        else    setFlkWinConfig();
        if(CUST_CAF_WINDOW_FOLLOW_TAF_WINDOW())    m_tcaf_mode=1;
    }

    // set AF info
    AF_INFO_T sAFInfo;
    sAFInfo.i4AFPos = m_sAFOutput.i4AFPos;
    IspTuningMgr::getInstance().setAFInfo(m_i4CurrSensorDev, sAFInfo);

    if((m_sAFOutput.i4IsMonitorFV == TRUE) && (m_i4FLKValid <0))
        m_i4FLKValid++;
    else if(m_sAFOutput.i4IsMonitorFV != TRUE)
        m_i4FLKValid=-2;

    if(m_sAFOutput.i4IsAFDone)
        m_NonInitState=MTRUE;

    m_prv_frm_num = m_cur_frm_num;
    return S_AF_OK;
}


MVOID AfMgr::doPDBuffer(MVOID * buffer, MINT32 w,MINT32 h, MINT32 stride)
{
    m_pdaf_rawbuf=buffer;
    m_pdaf_w=w;
    m_pdaf_h=h;
    m_pdaf_stride=stride;
    MY_LOG("[doPDBuffer]%d %d\n",m_pdaf_w,m_pdaf_h);
}
MINT32 AfMgr::PDPureRawInterval ()
{
    #ifdef PD_PURE_RAW_INTERVAL
        return PD_PURE_RAW_INTERVAL;
    #else
        return 1;
    #endif
}

MRESULT AfMgr::doPDTask()
{
    Mutex::Autolock lock(m_Lock);

    if((m_i4EnableAF == 0) || (m_pIPdAlgo==NULL) || (m_isPDAF_ON==0))
    {
        MY_LOG("no PD task: %d, %p, %d %d", m_i4EnableAF, m_pIPdAlgo, m_isPDAF_ON, m_i4CurrSensorDev);
        //MY_LOG("no PD task");
        return S_AF_OK;
    }
    if ((m_pIPdAlgo!=NULL) && (m_isPDAF_ON==1) && m_pdaf_rawbuf!=NULL)
    {
        PD_EXTRACT_INPUT_T a_sPDInput;
        //a_sPDInput.pPDBuf   = (MVOID *)pImg; for virtual channel
        PD_EXTRACT_DATA_T  a_sPDOutput[AF_PSUBWIN_NUM];
        a_sPDInput.pRawBuf  = (MVOID *)m_pdaf_rawbuf;

        AREA_T pdWins[AF_PSUBWIN_NUM];
        MINT32 pdWinNum;
        MINT32 ret = S_3A_OK;
        if (m_pIAfAlgo)
        {
            AREA_T afRoi;
            afRoi.i4W = m_LastAFArea.i4W;
            afRoi.i4H = m_LastAFArea.i4H;
            afRoi.i4X = m_LastAFArea.i4X;
            afRoi.i4Y = m_LastAFArea.i4Y;
            afRoi.i4Info = 0;
            if(PDAF_SCAN_MODE > 1)
            {
                afRoi.i4Info = PDAF_SCAN_MODE;    // tell af algo to start scanning
            }

            ret = m_pIAfAlgo->getFocusWindows(afRoi, &pdWins[0], &pdWinNum);

            if(S_3A_OK != ret)
            {
                pdWins[0].i4W = m_LastAFArea.i4W;
                pdWins[0].i4H = m_LastAFArea.i4H;
                pdWins[0].i4X = m_LastAFArea.i4X;
                pdWins[0].i4Y = m_LastAFArea.i4Y;
                pdWinNum = 1;
            }
        }

        for (MINT32 i = 0; i < pdWinNum; i++)
        {
            a_sPDInput.sFocusWin.i4W = pdWins[i].i4W;
            a_sPDInput.sFocusWin.i4H = pdWins[i].i4H;
            a_sPDInput.sFocusWin.i4X = pdWins[i].i4X;
            a_sPDInput.sFocusWin.i4Y = pdWins[i].i4Y;

            MY_LOG("[PDAF]win %d [WinPos] %d, %d, %d, %d\n", i, a_sPDInput.sFocusWin.i4X, a_sPDInput.sFocusWin.i4Y, a_sPDInput.sFocusWin.i4W, a_sPDInput.sFocusWin.i4H);

            m_pIPdAlgo->extractPD(a_sPDInput, a_sPDOutput[i]);
        }


        for (MINT32 i = 0; i < pdWinNum; i++)
        {
            m_sPDInput[i].bIsFace      = 0;
            m_sPDInput[i].i4CurLensPos = m_pdaf_pure_lens_pos;
            m_sPDInput[i].sPDExtractData.pPDLData   = a_sPDOutput[i].pPDLData;
            m_sPDInput[i].sPDExtractData.pPDLPos    = a_sPDOutput[i].pPDLPos;
            m_sPDInput[i].sPDExtractData.pPDRData   = a_sPDOutput[i].pPDRData;
            m_sPDInput[i].sPDExtractData.pPDRPos    = a_sPDOutput[i].pPDRPos;
            m_sPDInput[i].sPDExtractData.sPdWin.i4H = a_sPDOutput[i].sPdWin.i4H;
            m_sPDInput[i].sPDExtractData.sPdWin.i4W = a_sPDOutput[i].sPdWin.i4W;
            m_sPDInput[i].sPDExtractData.sPdWin.i4X = a_sPDOutput[i].sPdWin.i4X;
            m_sPDInput[i].sPDExtractData.sPdWin.i4Y = a_sPDOutput[i].sPdWin.i4Y;

            ret = m_pIPdAlgo->handlePD(m_sPDInput[i],m_sPDOutput[i]);
            MY_LOG("[PDAF]win %d [Confidence] %d [LensPos] %d [value] %d\n", i, m_sPDOutput[i].i4ConfidenceLevel,m_sPDOutput[i].i4FocusLensPos, (MINT32)(m_sPDOutput[i].fPdValue*1000));

            if(PDAF_SCAN_MODE > 1)
            {
                if (ret == E_3A_ERR)    // means scantable mode finished
                {
                    if (m_pIAfAlgo)
                    {
                        AREA_T afRoi;
                        afRoi.i4W = m_LastAFArea.i4W;
                        afRoi.i4H = m_LastAFArea.i4H;
                        afRoi.i4X = m_LastAFArea.i4X;
                        afRoi.i4Y = m_LastAFArea.i4Y;
                        afRoi.i4Info = -1;        // tell af algo to stop scanning
                        m_pIAfAlgo->getFocusWindows(afRoi, &pdWins[0], &pdWinNum);
                    }
                    PDAF_SCAN_MODE = 0;
                    break;
                }
            }
        }
        m_pdaf_rawbuf=NULL;
        //MY_LOG("[PDAF]1 [Confidence] %d [LensPos] %d [value] %d\n", m_sPDOutput.i4ConfidenceLevel,m_sPDOutput.i4FocusLensPos, (MINT32)(m_sPDOutput.fPdValue*1000));
    }
    return S_AF_OK;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setFlkWinConfig()
{
    if( m_flkwin_syncflag!=1)
  {
        m_flkwin_syncflag = 1;
        m_flkwin_synccnt = 2;
    }
    if(m_flkwin_synccnt) m_flkwin_synccnt--;

    MY_LOG_IF((m_bDebugEnable&1), "[setFlkWinConfig]\n");
    MINTPTR handle;

    if(m_bDebugAFwinW)   {    m_i4AF_in_Hsize=m_bDebugAFwinW;}
    if(m_bDebugAFwinH)   {    m_i4AF_in_Vsize=m_bDebugAFwinH;}
    MINT32 i4WOri = Boundary((WIN_SIZE_MIN*6 +WIN_POS_MIN*2),m_i4AF_in_Hsize, (WIN_SIZE_MAX*6+WIN_POS_MIN*2));
    MINT32 i4HOri = Boundary((WIN_SIZE_MIN*6 +WIN_POS_MIN*2),m_i4AF_in_Vsize, (WIN_SIZE_MAX*6+WIN_POS_MIN*2));
    MINT32  i4W = (((i4WOri-(WIN_POS_MIN*2))/6)>>1)<<1;
    MINT32  i4H = (((i4HOri-(WIN_POS_MIN*2))/6)>>1)<<1;
    MINT32 i4X = WIN_POS_MIN;
    MINT32 i4Y = WIN_POS_MIN;
    MINT32 wintmp;

    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE == sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setFlkWinConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setFlkWinConfig fail");
        }
        else
        {
            // 13 bits (8192x8192) - double buffer, "must even position"
            wintmp=(Boundary(WIN_POS_MIN, i4X,         i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W  , i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINX_1, wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4X + i4W*2, i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W*3, i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINX_2, wintmp );
            wintmp=(Boundary(WIN_POS_MIN, i4X + i4W*4, i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W*5, i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINX_3, wintmp );

            wintmp=(Boundary(WIN_POS_MIN, i4Y,         i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H  , i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINY_1,wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4Y + i4H*2, i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H*3, i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINY_2,wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4Y + i4H*4, i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H*5 - m_i4AF_in_Vsize, i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINY_3,wintmp);

            // 10 bits (1022x1022) - double buffer
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_SIZE, (i4W & 0x3FF) + ((i4H&0x3FF)<<16) );

            if (m_sAFOutput.i4FDDetect==0)
            {
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_1, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_2, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_3, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
            }
            else if (m_FDArea.i4Count)
            {
                MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, i4WOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, i4HOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_1, ( i4XE & 0x1FFE)    + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_2, ( i4XE & 0x1FFE)    + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_3, ( i4XE & 0x1FFE)    + ((i4YE&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
            }
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setFlkWinConfig fail");
        }
    }
    else //TG2
    {

        if (MFALSE == sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF_D,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setFlkWinConfig()"))))

        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setFlkWinConfig fail");
        }
        else
        {
            // 13 bits (8192x8192) - double buffer, "must even position"
            wintmp=(Boundary(WIN_POS_MIN, i4X,         i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W  , i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINX_1, wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4X + i4W*2, i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W*3, i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINX_2, wintmp );
            wintmp=(Boundary(WIN_POS_MIN, i4X + i4W*4, i4WOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4X + i4W*5, i4WOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINX_3, wintmp );

            wintmp=(Boundary(WIN_POS_MIN, i4Y,         i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H ,  i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINY_1,wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4Y + i4H*2, i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H*3, i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINY_2,wintmp);
            wintmp=(Boundary(WIN_POS_MIN, i4Y + i4H*4, i4HOri-WIN_POS_MIN)&0x1FFE)+ ((Boundary(WIN_POS_MIN, i4Y + i4H*5 - m_i4AF_in_Vsize, i4HOri-WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_WINY_3,wintmp);

            // 10 bits (1022x1022) - double buffer
            IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_SIZE, (i4W & 0x3FF) + ((i4H&0x3FF)<<16) );


            if (m_sAFOutput.i4FDDetect==0)
            {
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_1, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_2, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_3, (Boundary(FL_WIN_POS_MIN, i4X + i4W*2, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y + i4H*2, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, i4W*2, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H*2, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
            }
            else
            {
                MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, i4WOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, i4HOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_1, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_2, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_3, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
            }

            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setFlkWinConfig fail");
        }

    }

    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setFlkWinConfig()")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setFlkWinConfig fail");

    // AF v2.0 : set 6x6 AF win to FdArea
    //gSetRectFirst = true;
    if (m_sAFOutput.i4FDDetect && (m_FDArea.i4Count>0))  //keep FD win before FD AF
    {
        gFDRect.i4W = m_FDArea.sRect[0].i4W;
        gFDRect.i4H = m_FDArea.sRect[0].i4H;
        gFDRect.i4X = m_FDArea.sRect[0].i4X;
        gFDRect.i4Y = m_FDArea.sRect[0].i4Y;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFWinConfig(AF_AREA_T a_sAFArea)
{
    if( m_flkwin_syncflag!=0)
    {
      m_flkwin_syncflag = 0;
      m_flkwin_synccnt = 2;
    }
    if(m_flkwin_synccnt)  m_flkwin_synccnt--;
    // AF v2.0 : set 6x6 AF win to FdArea
    if (m_sAFOutput.i4FDDetect)  // sAFStat.i8Stat24 is set as sAFStat.i8StatFL
    {
        // set saved FDArea to 6x6
        a_sAFArea.i4Count = 1;
        a_sAFArea.sRect[0].i4W = m_FDArea.sRect[0].i4W = gFDRect.i4W;
        a_sAFArea.sRect[0].i4H = m_FDArea.sRect[0].i4H = gFDRect.i4H;
        a_sAFArea.sRect[0].i4X = m_FDArea.sRect[0].i4X = gFDRect.i4X;
        a_sAFArea.sRect[0].i4Y = m_FDArea.sRect[0].i4Y = gFDRect.i4Y;

        MY_LOG("[AFv2][BlackFaceAF] focusing Face(%d %d %d %d)\n",
            gFDRect.i4X, gFDRect.i4Y, gFDRect.i4W, gFDRect.i4H);
    }

    MY_LOG("[setAFWinConfig][cnt]%d[w]%d[h]%d[x]%d[y]%d\n", a_sAFArea.i4Count, a_sAFArea.sRect[0].i4W, a_sAFArea.sRect[0].i4H, a_sAFArea.sRect[0].i4X, a_sAFArea.sRect[0].i4Y);
    if ((a_sAFArea.i4Count != 1) || (a_sAFArea.sRect[0].i4W == 0) || (a_sAFArea.sRect[0].i4H == 0))  return;

    if(m_bDebugAFwinW)   {    m_i4AF_in_Hsize=m_bDebugAFwinW;}
    if(m_bDebugAFwinH)   {    m_i4AF_in_Vsize=m_bDebugAFwinH;}

    MINT32 i4WOri = Boundary((WIN_SIZE_MIN*6 +WIN_POS_MIN*2),m_i4AF_in_Hsize, (WIN_SIZE_MAX*6+WIN_POS_MIN*2));
    MINT32 i4HOri = Boundary((WIN_SIZE_MIN*6 +WIN_POS_MIN*2),m_i4AF_in_Vsize, (WIN_SIZE_MAX*6+WIN_POS_MIN*2));
    MINTPTR handle;
    MINT32 i4W = Boundary(WIN_SIZE_MIN, (((a_sAFArea.sRect[0].i4W/6)>>1)<<1), WIN_SIZE_MAX);
    MINT32 i4H = Boundary(WIN_SIZE_MIN, (((a_sAFArea.sRect[0].i4H/6)>>1)<<1), WIN_SIZE_MAX);
    MINT32 i4X = Boundary(WIN_POS_MIN, (a_sAFArea.sRect[0].i4X >>1)<<1, i4WOri-WIN_POS_MIN-i4W*6);
    MINT32 i4Y = Boundary(WIN_POS_MIN, (a_sAFArea.sRect[0].i4Y >>1)<<1, i4HOri-WIN_POS_MIN-i4H*6);
    MINT32 wintmp;

    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFWinConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFWinConfig fail");
        }
        else
        {
            if(i4X + i4W*6 + WIN_POS_MIN >= i4WOri)
            {
                if((i4WOri - WIN_POS_MIN*2- i4X) > (WIN_SIZE_MIN*6))
                    i4W = ((((i4WOri - WIN_POS_MIN*2- i4X)/6)>>1)<<1) ;
                else
                {
                    i4W = ((((i4WOri - WIN_POS_MIN*2)/6)>>1)<<1);
                    i4X =WIN_POS_MIN;
                }

                MY_LOG("[setAFWinConfig] over Hsiz %d\n", i4X + i4W*6);
            }
            if(i4Y + i4H*6 + WIN_POS_MIN >= i4HOri)
            {
                if((i4HOri - WIN_POS_MIN*2- i4Y) >(WIN_SIZE_MIN*6))
                    i4H = ((((i4HOri - WIN_POS_MIN*2- i4Y)/6)>>1)<<1) ;
                else
                {
                    i4H = ((((i4HOri - WIN_POS_MIN*2)/6)>>1)<<1);
                    i4Y =WIN_POS_MIN;
                }
                MY_LOG("[setAFWinConfig]over Vsiz %d\n", i4Y + i4H*6);
            }
            // 13 bits (8192x8192) - double buffer, "must even position"
            wintmp= (i4X    &     0x1FFE) + (((i4X + i4W  )&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_1,  wintmp);
            wintmp=((i4X + i4W*2)&0x1FFE) + (((i4X + i4W*3)&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_2, wintmp);
            wintmp=((i4X + i4W*4)&0x1FFE) + (((i4X + i4W*5)&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_3, wintmp );
            wintmp=  (i4Y    &     0x1FFE) + (((i4Y + i4H  )&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_1,  wintmp );
            wintmp= ((i4Y + i4H*2)&0x1FFE) + (((i4Y + i4H*3)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_2, wintmp );
            wintmp= ((i4Y + i4H*4)&0x1FFE) + (((i4Y + i4H*5)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_3, wintmp );

            // 10 bits (1022x1022) - double buffer
            wintmp=(i4W & 0x3FE) + ((i4H&0x3FE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_SIZE, wintmp );

            wintmp=(Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
            + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_1,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_2,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_3,  wintmp);

            // 12 bits (4096x4096) - double buffer
            wintmp=(Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
            + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_1, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_2, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_3, wintmp );

            if (m_sAFOutput.i4FDDetect==0)
            {
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_1, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_2, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_3, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
            }
            else if (m_sAFOutput.i4IsAFDone==1)
            {
                MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, i4WOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, i4HOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_1, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_2, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_WIN_3, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
            }
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                    MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinConfig fail");
        }
    }
    else //TG2
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF_D,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFWinConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFWinConfig fail");
        }
        else
        {
            if(i4X + i4W*6 + WIN_POS_MIN >= i4WOri)
            {
                if((i4WOri - WIN_POS_MIN*2- i4X) >(WIN_SIZE_MIN*6))
                    i4W = ((((i4WOri - WIN_POS_MIN*2- i4X)/6)>>1)<<1) ;
                else
                {
                    i4W = ((((i4WOri - WIN_POS_MIN*2)/6)>>1)<<1);
                    i4X =WIN_POS_MIN;
                }

                MY_LOG("[setAFWinConfig] over Hsiz %d\n", i4X + i4W*6);
            }
            if(i4Y + i4H*6 + WIN_POS_MIN >= i4HOri)
            {
                if((i4HOri - WIN_POS_MIN*2- i4Y) >(WIN_SIZE_MIN*6))
                    i4H = ((((i4HOri - WIN_POS_MIN*2- i4Y)/6)>>1)<<1) ;
                else
                {
                    i4H = ((((i4HOri - WIN_POS_MIN*2)/6)>>1)<<1);
                    i4Y =WIN_POS_MIN;
                }
                MY_LOG("[setAFWinConfig]over Vsiz %d\n", i4Y + i4H*6);
            }
            // 13 bits (8192x8192) - double buffer, "must even position"
            wintmp= (i4X    &     0x1FFE) + (((i4X + i4W  )&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_1,  wintmp);
            wintmp=((i4X + i4W*2)&0x1FFE) + (((i4X + i4W*3)&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_2, wintmp);
            wintmp=((i4X + i4W*4)&0x1FFE) + (((i4X + i4W*5)&0x1FFE)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_3, wintmp );
            wintmp=  (i4Y    &     0x1FFE) + (((i4Y + i4H  )&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_1,  wintmp );
            wintmp= ((i4Y + i4H*2)&0x1FFE) + (((i4Y + i4H*3)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_2, wintmp );
            wintmp= ((i4Y + i4H*4)&0x1FFE) + (((i4Y + i4H*5)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_3, wintmp );

            // 10 bits (1022x1022) - double buffer
            wintmp=(i4W & 0x3FE) + ((i4H&0x3FE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_SIZE, wintmp );

            wintmp=(Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
            + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_1,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_2,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_3,  wintmp);

            // 12 bits (4096x4096) - double buffer
            wintmp=(Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
            + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_1, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_2, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_3, wintmp );

            if (m_sAFOutput.i4FDDetect==0)
            {
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_1, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_2, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_3, (Boundary(FL_WIN_POS_MIN, i4X, i4WOri - FL_WIN_POS_MIN) & 0x1FFE)
                + ((Boundary(FL_WIN_POS_MIN, i4Y, i4HOri - FL_WIN_POS_MIN)&0x1FFE)<<16));

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, i4W, FL_WIN_SIZE_MAX) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, i4H, FL_WIN_SIZE_MAX)&0xFFE)<<16) );
            }
            else if (m_sAFOutput.i4IsAFDone==1)
            {
                MINT32 i4XE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4X>>1)<<1, i4WOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                MINT32 i4YE = Boundary(FL_WIN_POS_MIN, (m_FDArea.sRect[0].i4Y>>1)<<1, i4HOri - FL_WIN_POS_MIN - FL_WIN_SIZE_MIN);
                // 13 bits (8192x8192) - double buffer, "must even position"
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_1, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_2, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_WIN_3, ( i4XE & 0x1FFE) + ((i4YE&0x1FFE)<<16) );

                // 12 bits (4096x4096) - double buffer
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_1, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_2, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
                IOPIPE_SET_MODUL_REG(handle, CAM_AF_D_FLO_SIZE_3, (Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4W>>1)<<1, i4WOri - i4XE - FL_WIN_POS_MIN) & 0xFFE)
                + ((Boundary(FL_WIN_SIZE_MIN, (m_FDArea.sRect[0].i4H>>1)<<1, i4HOri - i4YE - FL_WIN_POS_MIN)&0xFFE)<<16) );
            }
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                    MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinConfig fail");
        }
    }
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setAFWinConfig")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinConfig fail");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAF_IN_HSIZE()
{
    MINTPTR handle;
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_TG_OUT_SIZE, m_i4SensorIdx, (MINTPTR)(&m_i4AF_in_Hsize), (MINTPTR)(&m_i4AF_in_Vsize)))
        MY_ERR("EPIPECmd_GET_TG_OUT_SIZE fail");

    MY_LOG_IF((m_bDebugEnable&1),"[setAF_IN_HSIZE][SensorDev]%d [H]%d [V]%d\n", m_i4CurrSensorDev, m_i4AF_in_Hsize,  m_i4AF_in_Vsize);

    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_EN, 1,-1))
            MY_ERR("EPipe_P1Sel_SGG_EN fail");

        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG, 1,-1))
            MY_ERR("EPipe_P1Sel_SGG  fail");

        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF,
                                       (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAF_IN_HSIZE()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAF_IN_HSIZE fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_IMAGE_SIZE, m_i4AF_in_Hsize);
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                    MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAF_IN_HSIZE fail");
        }
    }
    else
    {
        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_EN_D, 1,-1))
            MY_ERR("EPipe_P1Sel_SGG_EN fail");

        if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_SGG_D, 1,-1))
            MY_ERR("EPipe_P1Sel_SGG  fail");

        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF_D,
                                       (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAF_IN_HSIZE()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAF_D_IN_HSIZE fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_IMAGE_SIZE, m_i4AF_in_Hsize);
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                    MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAF_IN_HSIZE fail");
        }

    }
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setAF_IN_HSIZE")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAF_D_IN_HSIZE fail");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFWinTH(AF_CONFIG_T a_sAFConfig)
{
    MINTPTR handle;
    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFWinTH()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFWinTH fail");
        }
        else
        {
            //if ((m_i4AF_TH[0] != a_sAFConfig.AF_TH[0]) || (m_i4AF_TH[1] != a_sAFConfig.AF_TH[1]))
            {
                MY_LOG_IF((m_bDebugEnable&1),"[TH0/1]%d %d\n", a_sAFConfig.AF_TH[0], a_sAFConfig.AF_TH[1]);
                IOPIPE_SET_MODUL_REG(handle,  CAM_AF_TH, 0x01000000+(Boundary(0, a_sAFConfig.AF_TH[1], 255)<<16)+ Boundary(0, a_sAFConfig.AF_TH[0], 255));
                IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_TH,        (Boundary(0, a_sAFConfig.AF_TH[1], 255)<<16)+ Boundary(0, a_sAFConfig.AF_TH[0], 255));
                m_i4AF_TH[0] = a_sAFConfig.AF_TH[0];
                m_i4AF_TH[1] = a_sAFConfig.AF_TH[1];
            }
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinTH fail");
        }
    }
    else
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF_D,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFWinTH()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFWinTH fail");
        }
        else
        {
            //if ((m_i4AF_TH[0] != a_sAFConfig.AF_TH[0]) || (m_i4AF_TH[1] != a_sAFConfig.AF_TH[1]))
            {
                MY_LOG_IF((m_bDebugEnable&1),"[TH0/1]%d %d\n", a_sAFConfig.AF_TH[0], a_sAFConfig.AF_TH[1]);
                IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_TH, 0x01000000+(Boundary(0, a_sAFConfig.AF_TH[1], 255)<<16)+ Boundary(0, a_sAFConfig.AF_TH[0], 255));
                IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_TH,        (Boundary(0, a_sAFConfig.AF_TH[1], 255)<<16)+ Boundary(0, a_sAFConfig.AF_TH[0], 255));
                m_i4AF_TH[0] = a_sAFConfig.AF_TH[0];
                m_i4AF_TH[1] = a_sAFConfig.AF_TH[1];
            }
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinTH fail");
        }
    }
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setAFWinTH")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFWinTH fail");
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setGMR(AF_CONFIG_T a_sAFConfig)
{
    MINTPTR handle;
    MINT32 i4_SGG[7], i4_SGG_setting1, i4_SGG_setting2;

    i4_SGG[0]=    Boundary(0, a_sAFConfig.i4SGG_GMR1, 255);
    i4_SGG[1]=    Boundary(0, a_sAFConfig.i4SGG_GMR2, 255);
    i4_SGG[2]=    Boundary(0, a_sAFConfig.i4SGG_GMR3, 255);
    i4_SGG[3]=    Boundary(0, a_sAFConfig.i4SGG_GMR4, 255);
    i4_SGG[4]=    Boundary(0, a_sAFConfig.i4SGG_GMR5, 255);
    i4_SGG[5]=    Boundary(0, a_sAFConfig.i4SGG_GMR6, 255);
    i4_SGG[6]=    Boundary(0, a_sAFConfig.i4SGG_GMR7, 255);

    i4_SGG_setting1 = i4_SGG[0] + (i4_SGG[1]<<8) + (i4_SGG[2]<<16) + (i4_SGG[3]<<24);
    i4_SGG_setting2 = i4_SGG[4] + (i4_SGG[5]<<8) + (i4_SGG[6]<<16);
    MY_LOG_IF((m_bDebugEnable&1),"[SGG1 1~7]%d %d %d %d %d %d %d\n",i4_SGG[0],i4_SGG[1],i4_SGG[2],i4_SGG[3],i4_SGG[4],i4_SGG[5],i4_SGG[6]);

    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_SGG1,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setGMR()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setGMR fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_PGN, Boundary(0, m_AEsetPGN, 2047));
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_GMRC_1, i4_SGG_setting1);
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_GMRC_2, i4_SGG_setting2);
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setGMR fail");
        }
    }
    else
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_SGG1_D,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setGMR()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setGMR fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_D_PGN, Boundary(0, m_AEsetPGN, 2047));
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_D_GMRC_1, i4_SGG_setting1);
            IOPIPE_SET_MODUL_REG(handle,  CAM_SGG1_D_GMRC_2, i4_SGG_setting2);
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setGMR fail");
        }
    }
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setGMR")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setGMR fail");

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::setAFConfig(AF_CONFIG_T a_sAFConfig)
{
    MY_LOG("[setAFConfig]\n");
    MINTPTR handle;
    MINT32 AFContmp, pixelodd=0, istwinmode=0;

    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_TWIN_INFO, (MINTPTR)&istwinmode, -1,-1))
        MY_ERR("GET_TWIN_INFO fail");

    MY_LOG("GET_TWIN_INFO get %d\n", istwinmode);

    if ((m_Pix_Id == 1) || (m_Pix_Id == 2))   // 0:BGGR, 1:GBRG, 2GRBG, 3RGGB
        pixelodd=1;  // 1: odd
    else pixelodd=0;  // 0: even

    if (istwinmode)    pixelodd=1;

    AFContmp=(Boundary(0, /*AF_TARY*/ 0          , 5)<<8) //AF_TARY   value= 0~5, bit 8~10
            +(Boundary(0, /*AF_TARX*/ 0          , 5)<<4) //AF_TARX   value= 0~5, bit 4~6
            +(Boundary(0, /*AF_ODD*/pixelodd     , 1)<<3) //AF ODD      0 or 1,      bit 3
            +(Boundary(0, 1, 1)<<2) //AF ZZ      0 or 1,     bit 2
            + Boundary(0, istwinmode, 2)  ;    //AF DECI,     value= 0~2, bit 0~1

    if(m_eSensorTG ==ESensorTG_1)
    {
        if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF,
                                           (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFConfig fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_CON, AFContmp);
            //AF Filter
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLT_1, (a_sAFConfig.AF_FILT1[3]<<24) + (a_sAFConfig.AF_FILT1[2]<<16) + (a_sAFConfig.AF_FILT1[1]<<8) + a_sAFConfig.AF_FILT1[0]);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLT_2, (a_sAFConfig.AF_FILT1[7]<<24) + (a_sAFConfig.AF_FILT1[6]<<16) + (a_sAFConfig.AF_FILT1[5]<<8) + a_sAFConfig.AF_FILT1[4]);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLT_3, (a_sAFConfig.AF_FILT1[11]<<24) + (a_sAFConfig.AF_FILT1[10]<<16) + (a_sAFConfig.AF_FILT1[9]<<8) + a_sAFConfig.AF_FILT1[8]);

            //AF Vertical Filter
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLT_4,(((a_sAFConfig.AF_FILT2[1]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[0]&0x7FF)));
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLT_5,(((a_sAFConfig.AF_FILT2[3]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[2]&0x7FF)));
            if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFConfig fail");
        }
    }
    else
    {
         if (MFALSE ==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,NSImageio::NSIspio::EModule_AF_D,
                                            (MINTPTR)&handle, (MINTPTR)(&("AFMgr::setAFConfig()"))))
         {
             MY_ERR("EPIPECmd_GET_MODULE_HANDLE setAFConfig fail");
         }
         else
         {
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_CON, AFContmp);
             //AF Filter
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLT_1, (a_sAFConfig.AF_FILT1[3]<<24) + (a_sAFConfig.AF_FILT1[2]<<16) + (a_sAFConfig.AF_FILT1[1]<<8) + a_sAFConfig.AF_FILT1[0]);
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLT_2, (a_sAFConfig.AF_FILT1[7]<<24) + (a_sAFConfig.AF_FILT1[6]<<16) + (a_sAFConfig.AF_FILT1[5]<<8) + a_sAFConfig.AF_FILT1[4]);
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLT_3, (a_sAFConfig.AF_FILT1[11]<<24) + (a_sAFConfig.AF_FILT1[10]<<16) + (a_sAFConfig.AF_FILT1[9]<<8) + a_sAFConfig.AF_FILT1[8]);

             //AF Vertical Filter
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLT_4,(((a_sAFConfig.AF_FILT2[1]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[0]&0x7FF)));
             IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLT_5,(((a_sAFConfig.AF_FILT2[3]&0x7FF)<<16) + (a_sAFConfig.AF_FILT2[2]&0x7FF)));
             if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
                 MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFConfig fail");
         }
    }
    if (MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFMgr::setAFConfig")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE setAFConfig fail");
}

MVOID AfMgr::setSGGPGN(MINT32 i4SGG_PGAIN)
{
    m_AEsetPGN=i4SGG_PGAIN;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::printAFConfigLog0()
{
    if (!m_pIspReg)
    {
        MY_LOG("[printAFConfigLog0] m_pIspReg NULL");
        return;
    }

    #ifdef ISPREADREG

    MY_LOG_IF((m_bDebugEnable&1), "DoAFSW TG%d [P1/P1D]0x%x/0x%x, [P1DMA/P1DDMA]0x%x/0x%x [AFDdone]%x  [AFDdoneST]%x", m_eSensorTG,
        ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1),
        ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_D),
        ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_DMA),
        ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_DMA_D),
        ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_INT_P1_EN_D));

    MY_LOG_IF((m_bDebugEnable&1),"DoAFSW [EIS/SGG2/ESFKO/SGG1/AF/FLK] %x, [AF_D/SGG1_D/AFO_D]%x,",
            (MINT32)((ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1,     EIS_EN)<<5)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1,      SGG2_EN)<<4)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_DMA,   ESFKO_EN)<<3)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1,       SGG1_EN)<<2)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1,       AF_EN)<<1)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1,       FLK_EN))),
    (MINT32)((ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_D,     AF_D_EN)<<2)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_D,     SGG1_D_EN)<<1)
            +(ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_CTL_EN_P1_DMA_D, AFO_D_EN))));

if(m_eSensorTG ==ESensorTG_1)
{
     MY_LOG_IF((m_bDebugEnable&1), "DoAFTG1 [XY]%d %d [Sz]%d %d  [G]%d %d %d %d %d %d %d [Hz]%d ",
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_WINX_1, AF_WINX_0),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_WINY_1, AF_WINY_0),

                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_SIZE, AF_WIN_WD),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_SIZE, AF_WIN_HT),

                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_1, SGG1_GMR_1),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_1, SGG1_GMR_2),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_1, SGG1_GMR_3),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_1, SGG1_GMR_4),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_2, SGG1_GMR_5),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_2, SGG1_GMR_6),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_GMRC_2, SGG1_GMR_7),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_IMAGE_SIZE, AF_IMAGE_WD));

    /* MY_LOG("DoAFTG1 [FltW1]%d %d %d %d [AFMode]%d ",

                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_FLO_WIN_1, AF_FLO_WINX_1),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_FLO_WIN_1, AF_FLO_WINY_1),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_FLO_SIZE_1, AF_FLO_WD_1),
                                ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_FLO_SIZE_1, AF_FLO_HT_1),
                                m_eLIB3A_AFMode                        );*/
}
else
{
    MY_LOG("DoAFTG2 [XY]%d %d [Sz]%d %d  [G]%d %d %d %d %d %d %d [Hz]%d ",
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_WINX_1, AF_D_WINX_0),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_WINY_1, AF_D_WINY_0),

                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_SIZE, AF_D_WIN_WD),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_SIZE, AF_D_WIN_HT),

                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_1, SGG1_D_GMR_1),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_1, SGG1_D_GMR_2),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_1, SGG1_D_GMR_3),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_1, SGG1_D_GMR_4),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_2, SGG1_D_GMR_5),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_2, SGG1_D_GMR_6),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_SGG1_D_GMRC_2, SGG1_D_GMR_7),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_IMAGE_SIZE, AF_D_IMAGE_WD));

    /*MY_LOG("DoAFTG2 [FltW1]%d %d %d %d [AFMode]%d ",

                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_WIN_1, AF_D_FLO_WINX_1),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_WIN_1, AF_D_FLO_WINY_1),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_SIZE_1, AF_D_FLO_WD_1),
                               ISP_READ_BITS_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_SIZE_1, AF_D_FLO_HT_1),
                               m_eLIB3A_AFMode                          );*/

}
#endif

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::printAFConfigLog1()
{
    if (!m_pIspReg)    {
         MY_LOG("[printAFConfigLog1] m_pIspReg NULL");
         return;
     }

    #ifdef ISPREADREG
    MY_LOG_IF((m_bDebugEnable&2),"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_BASE_ADDR),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_XSIZE),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_OFST_ADDR),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_YSIZE),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_STRIDE),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_CON),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_CON2),

   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_ERR_STAT),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AFO_D_ERR_STAT),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_CTL_SEL_P1_D));

    MY_LOG_IF((m_bDebugEnable&2),"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ",
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_CON),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINX_1),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINX_2),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINX_3),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINY_1),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINY_2),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_WINY_3),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_SIZE),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLT_1),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLT_2),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLT_3),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_TH));

    MY_LOG_IF((m_bDebugEnable&2),"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x ",
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_WIN_1),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_SIZE_1),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_WIN_2),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_SIZE_2),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_WIN_3),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_SIZE_3),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_TH),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_IMAGE_SIZE),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLT_4),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLT_5),
   ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_STAT_L),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_STAT_M));

    MY_LOG_IF((m_bDebugEnable&2),"0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_1L),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_1M),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_1V),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_2L),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_2M),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_2V),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_3L),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_3M),
    ISP_READ_REG_NOPROTECT(m_pIspReg , CAM_AF_D_FLO_STAT_3V));

#endif




}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_FULL_STAT_T AfMgr::TransToFullStat(MVOID *pAFStatBuf)
{
    m_i4FLKValid--;
    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);
    AF_FULL_STAT_T sAFStat;

    memset(&sAFStat, 0, sizeof(sAFStat));
    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)
    {
            sAFStat.i8StatH[i] = ((((MINT64)pAFStat->sStat[i].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;
            sAFStat.i8StatV[i] = pAFStat->sStat[i].u4StatV&0x3FFFFFF;
    }
    m_i4FLKValid++;
    return sAFStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_STAT_T AfMgr::Trans4WintoOneStat(MVOID *pAFStatBuf)
{
    AF_STAT_T sAFStat;
    MINT64 i8Stat24 = 0;
    MINT64 i8StatV = 0;
    MINT32 i4posx = 2;
    MINT32 i4posy = 2;

    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);

    memset(&sAFStat, 0, sizeof(sAFStat));

    sAFStat.i8Stat24 = 0;
    sAFStat.i8StatV  = 0;

    if(m_tcaf_mode && !m_sAFOutput.i4FDDetect)    {
        i4posx = m_sAFOutput.sAFArea.sRect[0].i4X -m_sAFInput.sEZoom.i4X+ (m_sAFOutput.sAFArea.sRect[0].i4W>>1);
        i4posy = m_sAFOutput.sAFArea.sRect[0].i4Y -m_sAFInput.sEZoom.i4Y+ (m_sAFOutput.sAFArea.sRect[0].i4H>>1);
        if (i4posx > m_sAFOutput.sAFArea.sRect[0].i4W/12+FL_WIN_POS_MIN) i4posx-= (m_sAFOutput.sAFArea.sRect[0].i4W/12+FL_WIN_POS_MIN);
        else i4posx=0;
        if (i4posy > m_sAFOutput.sAFArea.sRect[0].i4H/12+FL_WIN_POS_MIN) i4posy-= (m_sAFOutput.sAFArea.sRect[0].i4H/12+FL_WIN_POS_MIN);
        else i4posy=0;
        i4posx = i4posx/ (m_sAFInput.sEZoom.i4W/6);
        i4posy = i4posy/ (m_sAFInput.sEZoom.i4H/6);
        i4posx = Boundary(0,i4posx,4);
        i4posy = Boundary(0,i4posy,4);
    }

    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)
    {
        // --- H24 ---
        if (i==(i4posx + i4posy*6) || i==(i4posx+1+ i4posy*6) || i==(i4posx+6+ i4posy*6) || i==(i4posx+7+ i4posy*6))
        {
            i8Stat24 = ((((MINT64)pAFStat->sStat[i].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;
            sAFStat.i8Stat24 += i8Stat24;

            // --- V ---
            i8StatV = pAFStat->sStat[i].u4StatV&0x3FFFFFF;
            sAFStat.i8StatV += i8StatV;
        }
    }
    // --- Floating ---
    MINT64 statsH[MAX_AF_HW_WIN] = {0};
    MINT64 statsV[MAX_AF_HW_WIN] = {0};
    if ((m_sAFOutput.i4FDDetect) && (m_FDArea.i4Count))
    {
        statsH[MAX_AF_HW_WIN-1] = (((MINT64)(pAFStat->sStat[36].u4StatV&0xFF)<<32) + (MINT64)pAFStat->sStat[36].u4Stat24);
        statsV[MAX_AF_HW_WIN-1] = statsH[MAX_AF_HW_WIN-1];
        MY_LOG("FL FV %lld", statsH[MAX_AF_HW_WIN-1], statsV[MAX_AF_HW_WIN-1]);
    }
    else
    {
        statsH[MAX_AF_HW_WIN-1] = -1;
        statsV[MAX_AF_HW_WIN-1] = -1;
    }
    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFStats(&statsH[0], &statsV[0], MAX_AF_HW_WIN);
    }
   /* sAFStat.i8StatFL = 0;
    if ((m_sAFOutput.i4FDDetect) && (m_FDArea.i4Count))    {
        sAFStat.i8Stat24 = sAFStat.i8StatFL;
        sAFStat.i8StatV = sAFStat.i8StatFL;
    }*/
#if 1  // print 6x6 Horizontal filter result .
        MINT32 k=0;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %d    %d    %d    %d    %d    %d",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);   k=k+6;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %lld    %lld    %lld    %lld    %lld    %lld",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);   k=k+6;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %lld    %lld    %lld    %lld    %lld    %lld",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);   k=k+6;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %lld    %lld    %lld    %lld    %lld    %lld",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);   k=k+6;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %lld    %lld    %lld    %lld    %lld    %lld",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);   k=k+6;
    MY_LOG_IF((m_bDebugEnable&4),"AvF:    %lld    %lld    %lld    %lld    %lld    %lld",
              ((((MINT64)pAFStat->sStat[k+0].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+0].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+1].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+1].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+2].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+2].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+3].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+3].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+4].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+4].u4Stat24,
              ((((MINT64)pAFStat->sStat[k+5].u4StatV>>28)&0xF)<<32) + (MINT64)pAFStat->sStat[k+5].u4Stat24);
    MY_LOG_IF((m_bDebugEnable&4),"AvF:========================");
#endif

    return sAFStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AF_STAT_T AfMgr::TransAFtoOneStat(MVOID *pAFStatBuf)
{
    AF_STAT_T sAFStat;
    MINT64 i8Stat24 = 0;
    MINT64 i8StatV = 0;

    // AF v2.0 : set 6x6 AF win to FdArea
    MINT64 statsH[MAX_AF_HW_WIN] = {0};
    MINT64 statsV[MAX_AF_HW_WIN] = {0};

    AF_HW_STAT_T *pAFStat = reinterpret_cast<AF_HW_STAT_T *>(pAFStatBuf);

    sAFStat.i8Stat24 = 0;
    sAFStat.i8StatV  = 0;

    for (MINT32 i=0; i<(MAX_AF_HW_WIN-1); i++)
    {
        // --- H24 ---
        i8Stat24 = ((((MINT64)(pAFStat->sStat[i].u4StatV>>28))&0xF)<<32) + (MINT64)pAFStat->sStat[i].u4Stat24;
        sAFStat.i8Stat24 += i8Stat24;

        // --- V ---
        i8StatV = pAFStat->sStat[i].u4StatV&0x3FFFFFF;
        sAFStat.i8StatV += i8StatV;

        // AF v2.0 : set 6x6 AF win to FdArea
        statsH[i] = i8Stat24;
        statsV[i] = i8StatV;
    }

    // --- Floating ---
    //sAFStat.i8StatFL = 0;

    // AF v2.0 : set 6x6 AF win to FdArea
    statsH[MAX_AF_HW_WIN-1] = -1;
    statsV[MAX_AF_HW_WIN-1] = -1;
    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->setAFStats(&statsH[0], &statsV[0], MAX_AF_HW_WIN);
    }
    MY_LOG_IF((m_bDebugEnable&1),"[AFv2] [6x6] %lld %lld ", sAFStat.i8Stat24, sAFStat.i8StatV);
    /*MY_LOG_IF((m_bDebugEnable&1),"[AFv2] [FL] %lld\n", sAFStat.i8StatFL);
    if ((m_sAFOutput.i4FDDetect) && (m_FDArea.i4Count))   {
        sAFStat.i8Stat24 = sAFStat.i8StatFL;
        sAFStat.i8StatV = sAFStat.i8StatFL;
    }*/  // Floating window is not used.

    return sAFStat;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::Boundary(MINT32 a_i4Min, MINT32 a_i4Vlu, MINT32 a_i4Max)
{
    if (a_i4Max < a_i4Min)  {a_i4Max = a_i4Min;}
    if (a_i4Vlu < a_i4Min)  {a_i4Vlu = a_i4Min;}
    if (a_i4Vlu > a_i4Max)  {a_i4Vlu = a_i4Max;}
    return a_i4Vlu;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::enableAF(MINT32 a_i4En)
{
    MY_LOG("[enableAF]%d\n", a_i4En);
    m_i4EnableAF = a_i4En;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocusFinish()
{
    return m_sAFOutput.i4IsAFDone;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 AfMgr::isFocused()
{
    return m_sAFOutput.i4IsFocused;
}
MINT32 AfMgr::isLastFocusModeTAF()
{
    MY_LOG("isLastFocusModeTAF = %d\n", m_i4LastFocusModeTAF);
    return m_i4LastFocusModeTAF;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getDebugInfo(AF_DEBUG_INFO_T &rAFDebugInfo)
{
    MINT32 err = S_AF_OK;
    if (m_pIAfAlgo)
    {
        m_pIAfAlgo->getDebugInfo(rAFDebugInfo);
        m_pIAfAlgo->SaveAfCoreInfo(mEngFileName);
        return MTRUE;
    }
    else
    {
        MY_LOG("Null m_pIAfAlgo\n");
        return E_AF_NULL_POINTER;
    }
    if (m_isPDAF_ON)
    {
        if (m_pIPdAlgo)
            err = m_pIPdAlgo->getDebugInfo(rAFDebugInfo);
        else
        {
        MY_LOG("Null m_pIPdAlgo\n");
        return E_AF_NULL_POINTER;
        }
    }
    return err;


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::getLensInfo(LENS_INFO_T &a_rLensInfo)
{
    MRESULT ret = E_3A_ERR;
    mcuMotorInfo rMotorInfo;

    if (m_pMcuDrv)
    {
        ret = m_pMcuDrv->getMCUInfo(&rMotorInfo,m_i4CurrSensorDev);
        a_rLensInfo.bIsMotorMoving = rMotorInfo.bIsMotorMoving;
        a_rLensInfo.bIsMotorOpen   = rMotorInfo.bIsMotorOpen;
        a_rLensInfo.i4CurrentPos   = (MINT32)rMotorInfo.u4CurrentPosition;
        a_rLensInfo.i4MacroPos     = (MINT32)rMotorInfo.u4MacroPosition;
        a_rLensInfo.i4InfPos       = (MINT32)rMotorInfo.u4InfPosition;
        a_rLensInfo.bIsSupportSR   = rMotorInfo.bIsSupportSR;
    ret = S_AF_OK;
    }
    return ret;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setCallbacks(I3ACallBack* cb)
{
    m_pAFCallBack = cb;
    return TRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::SingleAF_CallbackNotify()
{

    MUINT32 i4Dist=0;
    i4Dist=m_daf_distance;
    if(i4Dist==0) i4Dist=99999;
    if(m_DAF_TBL.is_daf_run==1)
        MY_LOG("[DAF--]i4Dist %d", i4Dist);

    MY_LOG("SingleAF_CallbackNotify");

    m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_STEREO_DISTANCE, i4Dist,0,0);
    m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
    //m_pAFCallBack->doDataCb(I3ACallBack::eID_DATA_AF_FOCUSED, &m_sAFOutput.sAFArea, sizeof(m_sAFOutput.sAFArea));
    m_i4AutoFocus = FALSE;
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::setBestShotConfig()
{
    MY_LOG("[setBestShotConfig] Not use");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::calBestShotValue(MVOID *pAFStatBuf)
{
    pAFStatBuf=NULL;

    MY_LOG("[calBestShotValue] Not use");
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AfMgr::getBestShotValue()
{
    MY_LOG("[getBestShotValue] Not use");
    return 0;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AfMgr::readOTP()
{
    MUINT32 result=0;
    CAM_CAL_DATA_STRUCT GetCamCalData;
    CamCalDrvBase *pCamCalDrvObj = CamCalDrvBase::createInstance();
    MINT32 i4SensorDevID;

    CAMERA_CAM_CAL_TYPE_ENUM enCamCalEnum = CAMERA_CAM_CAL_DATA_3A_GAIN;

    switch (m_i4CurrSensorDev)
    {
    case ESensorDev_Main:
        i4SensorDevID = SENSOR_DEV_MAIN;
        break;
    case ESensorDev_Sub:
        i4SensorDevID = SENSOR_DEV_SUB;
        break;
    case ESensorDev_MainSecond:
        i4SensorDevID = SENSOR_DEV_MAIN_2;
        return S_AWB_OK;
    case ESensorDev_Main3D:
        i4SensorDevID = SENSOR_DEV_MAIN_3D;
        return S_AWB_OK;
    default:
        i4SensorDevID = SENSOR_DEV_NONE;
        return S_AWB_OK;
    }

    result= pCamCalDrvObj->GetCamCalCalData(i4SensorDevID, enCamCalEnum, (void *)&GetCamCalData);
    MY_LOG("(0x%8x)=pCamCalDrvObj->GetCamCalCalData", result);

    if (result&CamCalReturnErr[enCamCalEnum])
    {
        MY_LOG("err (%s)", CamCalErrString[enCamCalEnum]);
        return E_AF_NOSUPPORT;
    }

    MY_LOG("OTP data [S2aBitEn]%d [S2aAfBitflagEn]%d [S2aAf0]%d [S2aAf1]%d", GetCamCalData.Single2A.S2aBitEn
    , GetCamCalData.Single2A.S2aAfBitflagEn, GetCamCalData.Single2A.S2aAf[0],GetCamCalData.Single2A.S2aAf[1]);

    MINT32 i4InfPos, i4MacroPos;
    if (GetCamCalData.Single2A.S2aBitEn & 0x1)
    {
        i4InfPos = GetCamCalData.Single2A.S2aAf[0];
        if (GetCamCalData.Single2A.S2aBitEn & 0x2)
        {
            i4MacroPos = GetCamCalData.Single2A.S2aAf[1];
            if (i4MacroPos < i4InfPos)
            {
                MY_LOG("OTP abnormal return [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);
                return S_AF_OK;
            }
        }
        else    i4MacroPos = 0;

        MY_LOG("OTP [Inf]%d [Macro]%d", i4InfPos, i4MacroPos);
        if (m_pIAfAlgo)
            m_pIAfAlgo->updateAFtableBoundary(i4InfPos, i4MacroPos);
    }
    MINT32 i;
    for(i = 0; i < 96; i++)
  {
    // TODO: set correct calibration data
        m_DccData[i] = i%2 ? 0x1c : 0xf3; //GetCamCalData.Single2A.S2aDcc[i];
        MY_LOG("OTP [DCC][0x%x] = 0x%x", (0x448+i), m_DccData[i]);
    }
    return S_AF_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::autoFocus()
{
    MY_LOG("autofocus %d", m_eLIB3A_AFMode);
    if(m_i4EnableAF == 0) // dummy lens
    {
        MY_LOG("autofocus : dummy lens");
        m_i4AutoFocuscb = TRUE;
        return;
    }

    if ((m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC) && (m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC_VIDEO))
    {
        m_i4AutoFocus = TRUE;
        m_i4LastFocusModeTAF = TRUE;
    }
    else
        m_i4AutoFocuscb = TRUE;

    m_sAFOutput.i4IsAFDone = MFALSE;

}
MVOID AfMgr::DoCallback()
{
    if(m_pAFCallBack!=NULL && m_i4AutoFocuscb==TRUE)
    {
        m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_MOVING, !m_sAFOutput.i4IsAFDone, 0, 0);
        m_pAFCallBack->doNotifyCb(I3ACallBack::eID_NOTIFY_AF_FOCUSED, m_sAFOutput.i4IsFocused, 0, 0);
        MY_LOG("autofocus callback in conti mode %d", m_eLIB3A_AFMode);
        m_i4AutoFocuscb = FALSE;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::cancelAutoFocus()
{
    if ((m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC) && (m_eLIB3A_AFMode != LIB3A_AF_MODE_AFC_VIDEO))
    m_i4AutoFocus = FALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MVOID AfMgr::TimeOutHandle()
{    MY_LOG("AF thread timeout");

    printAFConfigLog0();
    printAFConfigLog1();
    if (m_i4AutoFocus == TRUE)
    {
        MY_LOG("timeout callback");
        SingleAF_CallbackNotify();
        m_i4AutoFocus = FALSE;
    }
}
MVOID AfMgr::setAndroidServiceState(MBOOL a_state)
{
    m_AndroidServiceState=a_state;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AfMgr::setSensorMode(MINT32 i4NewSensorMode)
{
  m_SensorMode = i4NewSensorMode;
    MY_LOG("[%s()] m_eSensorDev: %d i4NewSensorMode: %d\n", __FUNCTION__, m_i4CurrSensorDev, m_SensorMode);
    return MTRUE;
}
// Depth AF API
MINT32 AfMgr::getDAFtbl(MVOID ** ptbl)
{
    *ptbl=&m_DAF_TBL;
    return DAF_TBL_QLEN;
}
MVOID AfMgr::setCurFrmNum(MUINT32 frm_num)
{
    m_cur_frm_num=frm_num;

    MINT32 get_frm_ststus;
    //0 represe precess raw, 1 represent pure raw
   if(MFALSE==sendAFNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_CUR_FRM_RAWFMT, (MINTPTR)(&get_frm_ststus), 0 ,0))
        MY_ERR("[m_cur_frm_num] EPIPECmd_GET_CUR_FRM_RAWFMT fail");

    m_PDAFPureRawQu[m_cur_frm_num%16]=get_frm_ststus;
  if( m_vFrmInfo.size()>30) m_vFrmInfo.erase( m_vFrmInfo.begin(), m_vFrmInfo.end()-30);
  //queue current frame's information
  AF_FRM_INFO_T frminfo( m_cur_frm_num, get_frm_ststus, getAFPos());
  m_vFrmInfo.push_back(frminfo);

  //depth AF
    m_AFStatusTbl[m_cur_frm_num%16][0]=m_cur_frm_num;
    m_AFStatusTbl[m_cur_frm_num%16][1]=(MUINT32)m_sAFOutput.i4IsAFDone;

  MY_LOG_IF( m_bDebugEnable, "[m_cur_frm_num] %d [get_frm_ststus] %d", m_cur_frm_num, get_frm_ststus);
}

MINT32 AfMgr::queryAFStatusByMagicNum(MUINT32 i4MagicNum)
{
    if(m_AFStatusTbl[i4MagicNum%16][0]==i4MagicNum)
        return (MINT32)m_AFStatusTbl[i4MagicNum%16][1];
    else
    {
        MY_LOG("[query_frm_num] %d [table_frm_num] %d, not match !!", i4MagicNum, m_AFStatusTbl[i4MagicNum%16][0] );
        return -1;
    }
    return -1;
}

//Camera 3.0
MVOID AfMgr::setAperture(MINT32 lens_aperture)
{
    lens_aperture=0;
//check available table, set lens_aperture
}
MINT32 AfMgr::getAperture()
{
    MINT32 lens_aperture=28;
    return  lens_aperture;
}


MVOID AfMgr::setFocalLength(MINT32 lens_focalLength)
{
    lens_focalLength=0;
    //check available table, set lens_focalLength
}
MINT32 AfMgr::getFocalLength ()
{
    MINT32 lens_focalLength =34;
    return  lens_focalLength;
}


MVOID AfMgr::setFocusDistance(MINT32 lens_focusDistance)
{
    lens_focusDistance=0;
    //check available table, set lens_focalLength
}
MINT32 AfMgr::getFocusDistance   ()
{
    MINT32 lens_focusDistance  =200;
    return  lens_focusDistance;
}

MVOID AfMgr::setOpticalStabilizationMode (MINT32 ois_OnOff)
{
    ois_OnOff=0;
    //check available table, set lens_focalLength
}
MINT32 AfMgr::getOpticalStabilizationMode()
{
/*OFF
ON */
    return  0;
}

MINT32 AfMgr::getFocusRange ()
{
    MINT32 focusRange   =200;
    return  focusRange ;
}

MINT32 AfMgr::getLensState  ()
{
/* STATIONARY
   MOVING */
    return  0 ;
}

MVOID AfMgr::setSharpnessMapMode(MINT32 SharpMapOnOff)
{
    SharpMapOnOff=0;
    //check available table, set lens_focalLength
}
MINT32 AfMgr::getSharpnessMapMode()
{
    return    1;
}
MINT32 AfMgr::getMaxSharpnessMapValue ()
{
        return    0xFFFFFFFF;
}
MINT32 AfMgr::getSharpnessMapSize()
{
     return  32;
}
MVOID AfMgr::getSharpnessMap(MVOID* a_sAFFullStat)
{
     a_sAFFullStat=NULL;

     //a_sAFFullStat = (MVOID*)&m_sAFFullStat;
}

MVOID AfMgr::SaveAfMgrInfo(const char * fname)
{
    strcpy(mEngFileName, fname);
}

