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
#define LOG_TAG "afo_buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <ispdrv_mgr.h>
#include <mtkcam/featureio/tuning_mgr.h>
#include <linux/cache.h>
#include <utils/threads.h>
#include <list>
#include <isp_tuning.h>

#include <mtkcam/drv/isp_reg.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include "afo_buf_mgr.h"

using namespace std;
using namespace android;
using namespace NS3A;
using namespace NSIspTuning;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;


typedef list<BufInfo_T> BufInfoList_T;
/*******************************************************************************
*  AFO buffer
*******************************************************************************/
#define AF_HW_WIN 36
#define AF_HW_FLOWIN 3
#define AF_WIN_DATA 8
#define AF_FLOWIN_DATA 12
#define AFO_BUFFER_SIZE (AF_WIN_DATA*AF_HW_WIN+AF_FLOWIN_DATA*AF_HW_FLOWIN)
#define AFO_XSIZE (AFO_BUFFER_SIZE-1)
#define MAX_AFO_BUFFER_CNT (1)

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AFOBufMgr
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor/Dtor.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    //  Copy constructor is disallowed.
    AFOBufMgr(AFOBufMgr const&);
    //  Copy-assignment operator is disallowed.
    AFOBufMgr& operator=(AFOBufMgr const&);

public:
    AFOBufMgr(ESensorDev_T const eSensorDev);
    ~AFOBufMgr();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static AFOBufMgr& getInstance(MINT32 const i4SensorDev);
    MBOOL init(MINT32 const i4SensorIdx);
    MBOOL uninit();
    MBOOL debugPrint();
    MBOOL enqueueHwBuf(BufInfo_T& rBufInfo);
    MBOOL dequeueHwBuf(BufInfo_T& rBufInfo);
    MUINT32 getCurrHwBuf();
    MUINT32 getNextHwBuf();
    MBOOL allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize);
    MBOOL freeBuf(BufInfo_T &rBufInfo);
    MBOOL updateDMABaseAddr(MUINT32 u4BaseAddr);
    MBOOL DMAInit();
    MBOOL DMAUninit();
    MBOOL AFStatEnable(MBOOL En);

    inline MBOOL sendCommandNormalPipe(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
    {
        INormalPipe* pPipe = INormalPipe::createInstance(m_i4SensorIdx, "afo_buf_mgr");
        MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
        pPipe->destroyInstance("afo_buf_mgr");
        return fgRet;
    }

    inline MBOOL setTGInfo(MINT32 const i4TGInfo)
    {
        MY_LOG("[%s()]i4TGInfo: %d\n", __FUNCTION__, i4TGInfo);
        switch (i4TGInfo)
        {
            case CAM_TG_1: m_eSensorTG = ESensorTG_1; break;
            case CAM_TG_2: m_eSensorTG = ESensorTG_2; break;
            default:
                MY_ERR("i4TGInfo= %d", i4TGInfo);
                return MFALSE;
        }
        return MTRUE;
    }


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    ESensorDev_T const     m_eSensorDev;
    ESensorTG_T            m_eSensorTG;
    IMemDrv*               m_pIMemDrv;
    volatile MINT32        m_Users;
    mutable android::Mutex m_Lock;
    MBOOL                  m_bDebugEnable;
    MINT32                 m_i4SensorIdx;
    INormalPipe*           m_pPipe;
    BufInfoList_T          m_rHwBufList;
    BufInfo_T              m_rAFOBufInfo[2];
    MINT32                 m_i4AF_in_Hsize;
    MINT32                 m_i4AF_in_Vsize;
    MINT32                 m_DMAInitDone;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AFOBufMgrDev : public AFOBufMgr
{
public:
    static AFOBufMgr& getInstance()
    {
        static AFOBufMgrDev<eSensorDev> singleton;
        return singleton;
    }

    AFOBufMgrDev(): AFOBufMgr(eSensorDev) {}
    virtual ~AFOBufMgrDev() {}
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::init(MINT32 const i4SensorDev, MINT32 const i4SensorIdx)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgr::getInstance(ESensorDev_Main).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgr::getInstance(ESensorDev_Sub).init(i4SensorIdx);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgr::getInstance(ESensorDev_MainSecond).init(i4SensorIdx);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::uninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
       ret_main = AFOBufMgr::getInstance(ESensorDev_Main).uninit();
    if (i4SensorDev & ESensorDev_Sub)
       ret_sub = AFOBufMgr::getInstance(ESensorDev_Sub).uninit();
    if (i4SensorDev & ESensorDev_MainSecond)
       ret_main2 = AFOBufMgr::getInstance(ESensorDev_MainSecond).uninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::DMAInit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgr::getInstance(ESensorDev_Main).DMAInit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgr::getInstance(ESensorDev_Sub).DMAInit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgr::getInstance(ESensorDev_MainSecond).DMAInit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::DMAUninit(MINT32 const i4SensorDev)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgr::getInstance(ESensorDev_Main).DMAUninit();
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgr::getInstance(ESensorDev_Sub).DMAUninit();
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgr::getInstance(ESensorDev_MainSecond).DMAUninit();

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::AFStatEnable(MINT32 const i4SensorDev, MBOOL En)
{
    MBOOL ret_main, ret_sub, ret_main2;
    ret_main = ret_sub = ret_main2 = MTRUE;

    if (i4SensorDev & ESensorDev_Main)
        ret_main = AFOBufMgr::getInstance(ESensorDev_Main).AFStatEnable(En);
    if (i4SensorDev & ESensorDev_Sub)
        ret_sub = AFOBufMgr::getInstance(ESensorDev_Sub).AFStatEnable(En);
    if (i4SensorDev & ESensorDev_MainSecond)
        ret_main2 = AFOBufMgr::getInstance(ESensorDev_MainSecond).AFStatEnable(En);

    return ret_main && ret_sub && ret_main2;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::enqueueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgr::getInstance(ESensorDev_Main).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgr::getInstance(ESensorDev_Sub).enqueueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgr::getInstance(ESensorDev_MainSecond).enqueueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::dequeueHwBuf(MINT32 const i4SensorDev, BufInfo_T& rBufInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgr::getInstance(ESensorDev_Main).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgr::getInstance(ESensorDev_Sub).dequeueHwBuf(rBufInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgr::getInstance(ESensorDev_MainSecond).dequeueHwBuf(rBufInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::updateDMABaseAddr(MINT32 const i4SensorDev)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgr::getInstance(ESensorDev_Main).updateDMABaseAddr(AFOBufMgr::getInstance(ESensorDev_Main).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgr::getInstance(ESensorDev_Sub).updateDMABaseAddr(AFOBufMgr::getInstance(ESensorDev_Sub).getNextHwBuf());
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgr::getInstance(ESensorDev_MainSecond).updateDMABaseAddr(AFOBufMgr::getInstance(ESensorDev_MainSecond).getNextHwBuf());

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IAFOBufMgr::setTGInfo(MINT32 const i4SensorDev, MINT32 const i4TGInfo)
{
    if (i4SensorDev & ESensorDev_Main)
        return AFOBufMgr::getInstance(ESensorDev_Main).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_Sub)
        return AFOBufMgr::getInstance(ESensorDev_Sub).setTGInfo(i4TGInfo);
    else if (i4SensorDev & ESensorDev_MainSecond)
        return AFOBufMgr::getInstance(ESensorDev_MainSecond).setTGInfo(i4TGInfo);

    MY_ERR("Incorrect sensor device: i4SensorDev = %d", i4SensorDev);
    return MFALSE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgr& AFOBufMgr::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
        case ESensorDev_Main: //  Main Sensor
            return  AFOBufMgrDev<ESensorDev_Main>::getInstance();
        case ESensorDev_MainSecond: //  Main Second Sensor
            return  AFOBufMgrDev<ESensorDev_MainSecond>::getInstance();
        case ESensorDev_Sub: //  Sub Sensor
            return  AFOBufMgrDev<ESensorDev_Sub>::getInstance();
        default:
            MY_ERR("i4SensorDev = %d", i4SensorDev);
            return  AFOBufMgrDev<ESensorDev_Main>::getInstance();
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgr::AFOBufMgr(ESensorDev_T const eSensorDev)
    : m_eSensorDev(eSensorDev)
    , m_eSensorTG(ESensorTG_None)
    , m_pIMemDrv(IMemDrv::createInstance())
    , m_Users(0)
    , m_Lock()
{}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AFOBufMgr::~AFOBufMgr(){}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::init(MINT32 const i4SensorIdx)
{
    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.afo_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_i4SensorIdx, m_Users);

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    //IHalSensor* pIHalSensor = pIHalSensorList->createSensor("afo_buf_mgr", m_i4SensorIdx);
    //SensorDynamicInfo rSensorDynamicInfo;
    SensorStaticInfo rSensorStaticInfo;
    switch  ( m_eSensorDev )
    {
        case ESensorDev_Main:
            //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN, &rSensorDynamicInfo);
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN, &rSensorStaticInfo);
            break;
        case ESensorDev_Sub:
            //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_SUB, &rSensorDynamicInfo);
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_SUB, &rSensorStaticInfo);
            break;
        case ESensorDev_MainSecond:
            //pIHalSensor->querySensorDynamicInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorDynamicInfo);
            pIHalSensorList->querySensorStaticInfo(NSCam::SENSOR_DEV_MAIN_2, &rSensorStaticInfo);
            break;
        default:    //  Shouldn't happen.
            MY_ERR("Invalid sensor device: %d", m_eSensorDev);
        return MFALSE;
    }
    m_i4AF_in_Hsize=rSensorStaticInfo.previewWidth;
    m_i4AF_in_Vsize=rSensorStaticInfo.previewHeight;

    //if(pIHalSensor)   pIHalSensor->destroyInstance("afo_buf_mgr");
    MY_LOG("AFO TG = %d, W/H =%d, %d, SensorIdx %d, \n", m_eSensorTG, m_i4AF_in_Hsize, m_i4AF_in_Vsize, m_i4SensorIdx);

    //m_pPipe = INormalPipe::createInstance(m_i4SensorIdx,"afo_buf_mgr");
    ret = m_pIMemDrv->init();
    MY_LOG("m_pIMemDrv->init() %d\n", ret);
    m_rHwBufList.clear();
    m_DMAInitDone=0;

    MY_LOG("[AFOBufMgr] allocateBuf \n");
    for(MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++)
    {
        m_rAFOBufInfo[i].useNoncache = 0;
        allocateBuf(m_rAFOBufInfo[i], AFO_BUFFER_SIZE);
        enqueueHwBuf(m_rAFOBufInfo[i]);
    }
    android_atomic_inc(&m_Users);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::uninit()
{
    MBOOL ret = MTRUE;
    IMEM_BUF_INFO buf_info;

    MY_LOG("[%s()] - E. m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0)    return MTRUE;

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) // There is no more User after decrease one User
    {
        MY_LOG("[AFOBufMgr] freeBuf\n");
        for (MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++) {
            freeBuf(m_rAFOBufInfo[i]);
        }
        MY_LOG("[AFOBufMgr]m_pIMemDrv uninit\n");
        m_pIMemDrv->uninit();
        m_DMAInitDone=0;
        //m_pPipe->destroyInstance("afo_buf_mgr");
    }
    else    // There are still some users.
    {
        MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::debugPrint()
{
    BufInfoList_T::iterator it;
    for (it = m_rHwBufList.begin(); it != m_rHwBufList.end(); it++ )
        MY_LOG("m_rHwBufList.virtAddr:[0x%x]/phyAddr:[0x%x] \n",it->virtAddr,it->phyAddr);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::enqueueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"AFO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    MY_LOG_IF(m_bDebugEnable,"AFO rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
    m_rHwBufList.push_back(rBufInfo);
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::dequeueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"%s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    if (m_rHwBufList.size())
    {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }
    MY_LOG_IF(m_bDebugEnable,"rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AFOBufMgr::getCurrHwBuf()
{
    if (m_rHwBufList.size() > 0)
    {
        return m_rHwBufList.front().phyAddr;
    }
    else
    {
        MY_ERR("AFO No free buffer\n");
        return 0;
    }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32 AFOBufMgr::getNextHwBuf()
{
    BufInfoList_T::iterator it;
    if (m_rHwBufList.size() > 1)
    {
        it = m_rHwBufList.begin();
        it++;
        return it->phyAddr;
    }
    else
    { // No free buffer
       MY_ERR("AFO No free buffer\n");
       return 0;
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    MY_LOG("AFO allocVirtBuf size %d",u4BufSize);
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return MFALSE;
    }
    if (m_pIMemDrv->mapPhyAddr(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return MFALSE;
    }
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::freeBuf(BufInfo_T &rBufInfo)
{
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }
    if (m_pIMemDrv->freeVirtBuf(&rBufInfo))
    {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::updateDMABaseAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"AFO %s() m_eSensorDev(%d) u4BaseAddr=0x%x\n", __FUNCTION__, m_eSensorDev, u4BaseAddr);
    MINT32 i4W = 64;        MINT32 i4H = 64;
    MINT32 i4X = 128;        MINT32 i4Y = 128;        MINT32 wintmp;

    if(!u4BaseAddr)
    {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }
    MUINTPTR handle;
    MINT32 istwinmode=0;

    if (MFALSE ==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_TWIN_INFO, (MINTPTR)&istwinmode, -1,-1))
        MY_ERR("GET_TWIN_INFO  fail");
    MY_LOG("GET_TWIN_INFO get %d\n", istwinmode);

    if (m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE ==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                                            NSImageio::NSIspio::EModule_AFO,
                                            (MINTPTR)&handle, (MINTPTR)(&("AFOBufMgr::DMAConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_BASE_ADDR, u4BaseAddr);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_XSIZE, AFO_XSIZE);
            MY_LOG("SET_AFO_CFG_DONE ");
            if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
            {
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
            }
        }
    }
    else  //ESensorTG_2
    {
        if (MFALSE ==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                                               NSImageio::NSIspio::EModule_AFO_D,
                                               (MINTPTR)&handle, (MINTPTR)(&("AFOBufMgr::DMAConfig()"))))
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_BASE_ADDR, u4BaseAddr);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_XSIZE, AFO_XSIZE);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_OFST_ADDR, 0);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_YSIZE, 0);
            IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_STRIDE, AFO_XSIZE+1);
            //IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_CON, 0x800A0820);
            //IOPIPE_SET_MODUL_REG(handle, CAM_AFO_D_CON2, 0x00201100);
            MY_LOG("SET_AFO_D_CFG_DONE ");

            if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
            {
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
            }
        }
    }
    if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AFOBufMgr::DMAConfig()")), MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }
    if (m_DMAInitDone == 1) return MTRUE;
//=================================================================
    if (m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE ==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                                            NSImageio::NSIspio::EModule_AF,
                                            (MINTPTR)&handle, (MINTPTR)(&("AF::DMAConfig()")))
            )
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE AF fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_CON, istwinmode);
            wintmp= i4X  + ((i4X + i4W)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_1,  wintmp);
            wintmp=i4X + i4W*2 + ((i4X + i4W*3)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_2, wintmp);
            wintmp=(i4X + i4W*4) + ((i4X + i4W*5)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINX_3, wintmp );
            wintmp=  i4Y     + ((i4Y + i4H)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_1,  wintmp );
            wintmp= i4Y + i4H*2 + ((i4Y + i4H*3)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_2, wintmp );
            wintmp= i4Y + i4H*4 + ((i4Y + i4H*5)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_WINY_3, wintmp );
            wintmp=i4W + (i4H<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_SIZE, wintmp );
            wintmp=i4X    + (i4Y<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_1,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_2,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_WIN_3,  wintmp);
            wintmp=i4W + (i4H<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_1, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_2, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_FLO_SIZE_3, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_IMAGE_SIZE, m_i4AF_in_Hsize);
            MY_LOG("AFO SET_AF_CFG_DONE ");
            if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
            {
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE AF fail");
            }
        }
    }
    else  //ESensorTG_2
    {
        if (MFALSE ==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_GET_MODULE_HANDLE,
                                            NSImageio::NSIspio::EModule_AF_D,
                                            (MINTPTR)&handle, (MINTPTR)(&("AF::DMAConfig()")))
        )
        {
            MY_ERR("EPIPECmd_GET_MODULE_HANDLE AF fail");
        }
        else
        {
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_CON, istwinmode);
            wintmp= i4X  + ((i4X + i4W)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_1,  wintmp);
            wintmp=i4X + i4W*2 + ((i4X + i4W*3)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_2, wintmp);
            wintmp=(i4X + i4W*4) + ((i4X + i4W*5)<<16) ;
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINX_3, wintmp );
            wintmp=  i4Y     + ((i4Y + i4H)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_1,  wintmp );
            wintmp= i4Y + i4H*2 + ((i4Y + i4H*3)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_2, wintmp );
            wintmp= i4Y + i4H*4 + ((i4Y + i4H*5)<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_WINY_3, wintmp );
            wintmp=i4W + (i4H<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_SIZE, wintmp );
            wintmp=i4X    + (i4Y<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_1,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_2,  wintmp);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_WIN_3,  wintmp);
            wintmp=i4W + (i4H<<16);
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_1, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_2, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_FLO_SIZE_3, wintmp );
            IOPIPE_SET_MODUL_REG(handle,  CAM_AF_D_IMAGE_SIZE, m_i4AF_in_Hsize);
            MY_LOG("AFO SET_AF_D_CFG_DONE ");
            if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
            {
                MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE AF fail");
            }
        }


    }
    if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MINTPTR)(&("AF::DMAConfig()")), MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE AF fail");
    }
    m_DMAInitDone=1;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::DMAInit()
{
    MY_LOG_IF(m_bDebugEnable,"AFO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    updateDMABaseAddr(getCurrHwBuf());
    if (m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_ESFKO, MTRUE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_ESFKO");
            return MFALSE;
        }

        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO, MTRUE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AFO");
            return MFALSE;
        }

    }
    else
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO_D, MTRUE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AFO");
            return MFALSE;
        }
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::DMAUninit()
{
    MY_LOG_IF(m_bDebugEnable,"AFO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    if (m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_ESFKO, MFALSE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_ESFKO");
            return MFALSE;
        }

        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO, MFALSE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AF0");
            return MFALSE;
        }
    }
    else
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AFO_D, MFALSE, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AFO_D");
            return MFALSE;
        }
    }
    m_DMAInitDone=0;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::AFStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"AFO m_eSensorDev(%d) AFStatEnable(%d)\n",m_eSensorDev, En);
    if (m_eSensorTG == ESensorTG_1)
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AF, En, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AF");
            return MFALSE;
        }
          if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_SGG1, En, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_SGG1");
            return MFALSE;
        }
    }
    else
    {
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_AF_D, En, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AF_D");
            return MFALSE;
        }
        if (MFALSE==sendCommandNormalPipe(NSImageio::NSIspio::EPIPECmd_SET_MODULE_EN, NSImageio::NSIspio::EModule_SGG1_D, En, MNULL))
        {
            MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_SGG1_D");
            return MFALSE;
        }
    }
    return MTRUE;
}

