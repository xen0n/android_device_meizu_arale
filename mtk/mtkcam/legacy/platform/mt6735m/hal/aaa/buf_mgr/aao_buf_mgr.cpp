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
#define LOG_TAG "aao_buf_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <utils/threads.h>
#include <sys/stat.h>
#include <cutils/properties.h>
#include <string.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include "aao_buf_mgr.h"
#include <mtkcam/common.h>

using namespace NS3Av3;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAAOBufMgr::
IAAOBufMgr()
    : m_pIMemDrv(IMemDrv::createInstance())
    , m_Users(0)
    , m_Lock()
    , m_bDebugEnable(MFALSE)
    , m_i4SensorIdx(0)
    , m_rHwBufList()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAAOBufMgr::
~IAAOBufMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
init(MINT32 const i4SensorIdx)
{
    MRESULT ret = MTRUE;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.aao_buf_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    MY_LOG("[%s()] m_i4SensorIdx: %d, m_Users: %d \n", __FUNCTION__, i4SensorIdx, m_Users);

    // sensor index
    m_i4SensorIdx = i4SensorIdx;

    Mutex::Autolock lock(m_Lock);

    if (m_Users > 0)
    {
        MY_LOG("%d has created \n", m_Users);
        android_atomic_inc(&m_Users);
        return MTRUE;
    }

    // AAO DMA buffer init

    // imem driver init
    m_pIMemDrv->init();

    // removes all elements from the list container
    m_rHwBufList.clear();

    // Enqueue HW buffer
    for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
        m_rAAOBufInfo[i].useNoncache = 0;
        allocateBuf(m_rAAOBufInfo[i], AAO_BUFFER_SIZE);
        enqueueHwBuf(m_rAAOBufInfo[i]);
    }

    //DMAInit();

    // Enable AA stat
    //AAStatEnable(MTRUE);

    debugPrint();

    android_atomic_inc(&m_Users);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
uninit()
{
    IMEM_BUF_INFO buf_info;

    MY_LOG("[%s()] m_Users: %d \n", __FUNCTION__, m_Users);

    Mutex::Autolock lock(m_Lock);

    // If no more users, return directly and do nothing.
    if (m_Users <= 0) {
        return MTRUE;
    }

    // More than one user, so decrease one User.
    android_atomic_dec(&m_Users);

    if (m_Users == 0) { // There is no more User after decrease one User
        for (MINT32 i = 0; i < MAX_AAO_BUFFER_CNT; i++) {
            freeBuf(m_rAAOBufInfo[i]);
        }

        // imem driver ininit
        m_pIMemDrv->uninit();
    } else {    // There are still some users.
        MY_LOG_IF(m_bDebugEnable,"Still %d users \n", m_Users);
    }

    return MTRUE;
}

MBOOL
IAAOBufMgr::sendCommandNormalPipe(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
#if (CAM3_3ATESTLVL < CAM3_3AIT)
#warning "FIXME"
    return MTRUE;
#endif
    IHalCamIO* pPipe = (IHalCamIO*)INormalPipe_FrmB::createInstance(m_i4SensorIdx, "aao_buf_mgr", 1);
    MBOOL fgRet = pPipe->sendCommand(cmd, arg1, arg2, arg3);
    pPipe->destroyInstance("aao_buf_mgr");
    return fgRet;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
debugPrint()
{
    BufInfoList_T::iterator it;

    for (it = m_rHwBufList.begin(); it != m_rHwBufList.end(); it++ ) {
        MY_LOG("m_rHwBufList.virtAddr:[0x%x]/phyAddr:[0x%x] \n",it->virtAddr,it->phyAddr);
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
enqueueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"%s() rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n", __FUNCTION__, rBufInfo.virtAddr, rBufInfo.phyAddr);

    // add element at the end
    m_rHwBufList.push_back(rBufInfo);
    m_pIMemDrv->cacheSyncbyRange(IMEM_CACHECTRL_ENUM_INVALID, &rBufInfo);
    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
dequeueHwBuf(BufInfo_T& rBufInfo)
{
    MY_LOG_IF(m_bDebugEnable,"%s() \n", __FUNCTION__);
    static MINT32 frameCnt = 0;

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("aao.dump.enable", value, "0");   // if the value is 11, it mean input buffer for test
    MBOOL bEnable = atoi(value);

    if (m_rHwBufList.size()) {
        rBufInfo = m_rHwBufList.front();
        m_rHwBufList.pop_front();
    }

    if (bEnable == 21)
    {
        ::memset(reinterpret_cast<MUINT8*>(rBufInfo.virtAddr), 128, rBufInfo.size);
    }
    else if (bEnable)
    {
        char fileName[64];
        FILE *fp = NULL;

        if(bEnable == 11) {
            sprintf(fileName, "/sdcard/aao/aao.raw");
            fp = fopen(fileName, "r");
        } else {
            sprintf(fileName, "/sdcard/aao/aao_%d.raw", frameCnt++);
            fp = fopen(fileName, "w");
        }

        if (NULL == fp)
        {
            MY_ERR("fail to open file to save img: %s", fileName);
            MINT32 err = mkdir("/sdcard/aao", S_IRWXU | S_IRWXG | S_IRWXO);
            MY_LOG("err = %d", err);
            return MFALSE;
        }
        MY_LOG_IF(m_bDebugEnable,"%s\n", fileName);
        if(bEnable == 11) {
            fread(reinterpret_cast<void *>(rBufInfo.virtAddr), 1, rBufInfo.size, fp);
        } else {
            fwrite(reinterpret_cast<void *>(rBufInfo.virtAddr), 1, rBufInfo.size, fp);
        }
        fclose(fp);
    }
    else {
        frameCnt = 0;
    }

    MY_LOG_IF(m_bDebugEnable,"rBufInfo.virtAddr:[0x%x]/phyAddr:[0x%x] \n",rBufInfo.virtAddr,rBufInfo.phyAddr);

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IAAOBufMgr::
getCurrHwBuf()
{
    if (m_rHwBufList.size() > 0) {
        return m_rHwBufList.front().phyAddr;
    }
    else { // No free buffer
        MY_ERR("No free buffer\n");
        return MFALSE;
    }
    return MFALSE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MUINT32
IAAOBufMgr::
getNextHwBuf()
{
    BufInfoList_T::iterator it;

    if (m_rHwBufList.size() > 1) {
        it = m_rHwBufList.begin();
        it++;
        return it->phyAddr;
    }
    else { // No free buffer
       MY_ERR("No free buffer\n");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
allocateBuf(BufInfo_T &rBufInfo, MUINT32 u4BufSize)
{
    rBufInfo.size = u4BufSize;
    if (m_pIMemDrv->allocVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->allocVirtBuf() error");
        return MFALSE;
    }

    if (m_pIMemDrv->mapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->mapPhyAddr() error");
        return MFALSE;
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
freeBuf(BufInfo_T &rBufInfo)
{
    if (m_pIMemDrv->unmapPhyAddr(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (m_pIMemDrv->freeVirtBuf(&rBufInfo)) {
        MY_ERR("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
updateDMABaseAddr(MUINT32 u4BaseAddr)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);
#if (CAM3_3ATESTLVL < CAM3_3AIT)
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
                                        NSImageio_FrmB::NSIspio_FrmB::EModule_AAO,
                                        (MUINTPTR)&handle, (MUINTPTR)(&("AAOBufMgr::DMAConfig()")))) {
        MY_ERR("EPIPECmd_GET_MODULE_HANDLE fail");
    } else {
           // set module register
        IOPIPE_SET_MODUL_REG(handle, CAM_AAO_BASE_ADDR, u4BaseAddr);
        IOPIPE_SET_MODUL_REG(handle, CAM_AAO_OFST_ADDR, AAO_OFFSET_ADDR);
        IOPIPE_SET_MODUL_REG(handle, CAM_AAO_XSIZE, AAO_XSIZE);
        IOPIPE_SET_MODUL_REG(handle, CAM_AAO_STRIDE, (AAO_STRIDE_BUS_SIZE << 16));

        if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_CFG_DONE, handle, MNULL, MNULL)) {
            MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
        }
    }

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_RELEASE_MODULE_HANDLE, handle, (MUINTPTR)(&("AFOBufMgr::DMAConfig()")), MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_CFG_DONE fail");
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
DMAInit()
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    updateDMABaseAddr(getCurrHwBuf());
    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AWB, MTRUE, MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AWB");
        return MFALSE;
    }

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AAO, MTRUE, MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
DMAUninit()
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AWB, MFALSE, MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AWB");
        return MFALSE;
    }

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AAO, MFALSE, MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AAO");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT
IAAOBufMgr::
AAStatEnable(MBOOL En)
{
    MY_LOG_IF(m_bDebugEnable,"AAStatEnable(%d)\n",En);

    if (MFALSE==sendCommandNormalPipe(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_SET_MODULE_EN, NSImageio_FrmB::NSIspio_FrmB::EModule_AWB, En, MNULL)) {
        MY_ERR("EPIPECmd_SET_MODULE_En fail: EModule_AWB");
        return MFALSE;
    }

    return MTRUE;
}

