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

#include <utils/threads.h>
#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <isp_tuning.h>
#include <isp_tuning_mgr.h>
#include <af_tuning_custom.h>
#include <mtkcam/drv_common/isp_reg.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <mtkcam/hal/IHalSensor.h>
#include "afo_buf_mgr.h"
#include <mtkcam/common.h>
using namespace NS3Av3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

/*******************************************************************************
*  AFO buffer
*******************************************************************************/
#define AF_HW_WIN 37
#define AF_WIN_DATA 8
#define AFO_BUFFER_SIZE (AF_WIN_DATA*AF_HW_WIN)
#define AFO_XSIZE (AFO_BUFFER_SIZE - 1)
#define MAX_AFO_BUFFER_CNT (1)


AFOBufMgr::AFOBufMgr()
{
}
AFOBufMgr::~AFOBufMgr()
{
}

MBOOL AFOBufMgr::sendCommandNormalPipe(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
#if (CAM3_3ATESTLVL <= CAM3_3AUT)
#warning "FIXME"
    return MTRUE;
#endif
    INormalPipe_FrmB* pPipe = (INormalPipe_FrmB*)INormalPipe_FrmB::createInstance(m_i4SensorIdx, "afo_buf_mgr", 1);
    MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
    pPipe->destroyInstance("afo_buf_mgr");
    return fgRet;
}

MUINT64 blockmodel1[]={
1120000, 2230000, 3333111, 1111225, 3332323, 3232999,
1234567, 7654321, 5554444, 3336666, 5224477, 9977554,
1234501, 7890123, 4567890, 1234568, 2345678, 1235689,
1597532, 2687123, 7531595, 5795132, 4561237, 8527412,
9565656, 9999999, 7777777, 5555555, 2222222, 1000001,
6565656, 8585858, 4444444, 6666666, 3333333, 1111222,
1000000 };



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::init(MINT32 eSensorDev, MINT32 i4SensorIdx)
{
    MBOOL ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.afo_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);
    m_i4SensorIdx = i4SensorIdx;
    m_eSensorDev = eSensorDev;

    MY_LOG("[%s()] m_eSensorDev: %d, m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, m_eSensorDev, m_Users);

    Mutex::Autolock lock(m_Lock);
    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }

    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    SensorStaticInfo rSensorStaticInfo;
    switch  ( m_eSensorDev )
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
        default:    //  Shouldn't happen.
            MY_ERR("Invalid sensor device: %d", m_eSensorDev);
        return MFALSE;
    }
    m_i4AF_in_Hsize=rSensorStaticInfo.previewWidth;
    m_i4AF_in_Vsize=rSensorStaticInfo.previewHeight;
    m_pIMemDrv=IMemDrv::createInstance();
    MY_LOG("AFO W/H =%d, %d, SensorIdx %d, \n",m_i4AF_in_Hsize, m_i4AF_in_Vsize);
    MY_LOG("m_pIMemDrv->init() %d\n", m_pIMemDrv->init());
    m_rHwBufList.clear();
    m_DMAInitDone=0;
    MY_LOG("[AFOBufMgr] allocateBuf \n");
    for(MINT32 i = 0; i < MAX_AFO_BUFFER_CNT; i++)
    {
        m_rAFOBufInfo[i].useNoncache = 0;
        allocateBuf(m_rAFOBufInfo[i], AFO_BUFFER_SIZE);
        enqueueHwBuf(m_rAFOBufInfo[i]);


    }
    memcpy((MUINT8*)m_rAFOBufInfo[0].virtAddr, (MUINT8*)&blockmodel1,(AFO_BUFFER_SIZE));
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
MBOOL AFOBufMgr::enqueueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG("AFO %s() m_eSensorDev(%d)\n", __FUNCTION__, m_eSensorDev);
    MY_LOG("AFO rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
    m_rHwBufList.push_back(rBufInfo);
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::dequeueHwBuf(BufInfo_T& rBufInfo)
{
    if (m_rHwBufList.size())
    {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }
    MY_LOG("rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);
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
#if (CAM3_3ATESTLVL <= CAM3_3AUT)
#warning "FIXME"
        return MTRUE;
#endif
    if(!u4BaseAddr)
    {
        MY_ERR("u4BaseAddr is NULL\n");
        return E_ISPMGR_NULL_ADDRESS;
    }
    MUINTPTR handle;
    if (MFALSE ==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_GET_MODULE_HANDLE,
                                        NSImageio_FrmB::NSIspio_FrmB::EModule_AFO,
                                        (MUINTPTR)&handle, (MUINTPTR)(&("AFOBufMgr::DMAConfig()"))))
    {
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
    }
    else
    {
        IOPIPE_SET_MODUL_REG(handle, CAM_AFO_XSIZE, AFO_XSIZE);
        IOPIPE_SET_MODUL_REG(handle, CAM_AFO_BASE_ADDR, u4BaseAddr);
        MY_LOG("updateDMABaseAddr :[0x%x]\n",u4BaseAddr);
        if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL))
            MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MUINTPTR)(&("AFOBufMgr::DMAConfig()")), MNULL))
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");

    if (m_DMAInitDone == 1) return MTRUE;
    m_DMAInitDone=1;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::DMAInit()
{
    updateDMABaseAddr(getCurrHwBuf());
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_ESFKO, MTRUE, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_ESFKO");
        return MFALSE;
    }

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AFO, MTRUE, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AFO");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::DMAUninit()
{
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_ESFKO, MFALSE, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_ESFKO");
        return MFALSE;
    }

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AFO, MFALSE, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AF0");
        return MFALSE;
    }
    m_DMAInitDone=0;
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL AFOBufMgr::AFStatEnable(MBOOL En)
{
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AF, En, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AF");
        return MFALSE;
    }
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_SGG1, En, MNULL))
    {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_SGG1");
        return MFALSE;
    }

    return MTRUE;
}

