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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

//! \file  eis_drv.cpp

#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>
#include <algorithm>
#include <utils/Condition.h>

using namespace std;
using namespace android;

#include "mtkcam/imageio/ispio_stddef.h"   // for EIS register struct

using namespace NSImageio_FrmB;
using namespace NSIspio_FrmB;

#include "mtkcam/drv_common/imem_drv.h"
#include "mtkcam/drv_common/isp_reg.h"
#include "mtkcam/drv_FrmB/isp_drv_FrmB.h"
#include "mtkcam/iopipe/CamIO/INormalPipe.h"    // for pass1 register setting API
#include "mtkcam/hal/IHalSensor.h"
#include "mtkcam/v1/config/PriorityDefs.h"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;
using namespace NSIspDrv_FrmB;
#include <mtkcam/Log.h>
#include <mtkcam/utils/common.h>
using namespace NSCam::Utils;

#include "m4u_lib.h"

#include "eis_drv_r.h"

/****************************************************************************************
* Define Value
****************************************************************************************/
#undef LOG_TAG
#define LOG_TAG "EisDrv"

#undef __func__
#define __func__ __FUNCTION__

#define EIS_DRV_DEBUG
#ifdef EIS_DRV_DEBUG

#define EIS_LOG_R(fmt, arg...)    CAM_LOGD("[%s][R]" fmt, __func__, ##arg)
#define EIS_INF_R(fmt, arg...)    CAM_LOGI("[%s][R]" fmt, __func__, ##arg)
#define EIS_WRN_R(fmt, arg...)    CAM_LOGW("[%s][R] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define EIS_ERR_R(fmt, arg...)    CAM_LOGE("[%s][R] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define EIS_LOG_R(a,...)
#define EIS_INF_R(a,...)
#define EIS_WRN_R(a,...)
#define EIS_ERR_R(a,...)
#endif

#define EISO_MEMORY_SIZE 256    // 32 * 64 (bits) = 256 bytes
#define EISO_BUFFER_NUM 10
#define EISO_CONDITION_WAIT_TIME 17
#define FEO_BUFFER_NUM 3
#define FEO_STRIDE (EIS_FE_MAX_INPUT_W/16)
#define EIS_DRV_NAME "EisDrv"

/*******************************************************************************
* Global variable
********************************************************************************/
static MINT32 g_debugDump_R = 0;

static MINT32 EisIrq_key = 0;

/*******************************************************************************
*
********************************************************************************/
EisDrv_R *EisDrv_R::CreateInstance(const MUINT32 &aSensorIdx)
{
    return EisDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
EisDrv_R *EisDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    EIS_LOG_R("aSensorIdx(%u)",aSensorIdx);

    return new EisDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrvImp::DestroyInstance()
{
    EIS_LOG_R("+");
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
EisDrvImp::EisDrvImp(const MUINT32 &aSensorIdx) : EisDrv_R()
{
    // reference count
    mUsers = 0;

    // INormalPipe_FrmB object
    m_pNormalPipe = NULL;

    // ISP driver object
    m_pISPDrvObj = NULL;
    m_pISPVirtDrv = NULL;

    // IMEM
    m_pIMemDrv = NULL;

    mEisoDmaBuf.memID = -5;
    mEisoDmaBuf.useNoncache = 1;
    mEisoDmaBuf.virtAddr = mEisoDmaBuf.phyAddr = mEisoDmaBuf.size = 0;

    // variable for EIS algo and EIS Hal
    mIsConfig   = 0;
    mIsFirst    = 1;
    mIs2Pixel   = 0;
    mTotalMBNum = 0;
    mImgWidth   = 0;
    mImgHeight  = 0;
    mEisDivH    = 0;
    mEisDivV    = 0;
    mSensorIdx  = aSensorIdx;
    mSensorType = EIS_NULL_SENSOR;
    mEisHwCfgSce = EIS_SCE_NONE;
    mEiso_addrIdx = 0;
    mEiso_virAddr = 0;
    mEiso_phyAddr = 0;
    mEisHwSupport = MTRUE;

    // Pass1 HW setting callback
    m_pEisP1Cb = NULL;

    // state
    mState = EIS_SW_STATE_NONE;

    // EISO data
    while(!mEisoData.empty())
    {
        mEisoData.pop();
    }
}

/*******************************************************************************
*
********************************************************************************/
EisDrvImp::~EisDrvImp()
{
}

/*******************************************************************************
*
********************************************************************************/
MVOID *EisDrvImp::EisoThreadLoop(MVOID *arg)
{
    EisDrvImp *_this = reinterpret_cast<EisDrvImp *>(arg);
    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Change Thread Setting ======

    _this->ChangeThreadSetting("EisoThreadLoop");

    //====== Main Loop ======

    //EIS_LOG_R("Sidx(%u),wait mEisoSem",_this->mSensorIdx);
    ::sem_wait(&_this->mEisoSem); // wait here until someone use sem_post() to wake this semaphore up
    //EIS_LOG_R("Sidx(%u),got mEisoSem",_this->mSensorIdx);

    EIS_SW_STATE_ENUM eState = _this->GetEisState();

    while(eState != EIS_SW_STATE_UNINIT)
    {
        eState = _this->GetEisState();

        switch(eState)
        {
            case EIS_SW_STATE_ALIVE:
                    // 1. wait Vsync
                    err = _this->UpdateEiso();
                    if(EIS_RETURN_NO_ERROR != err)
                    {
                        EIS_ERR_R("Sidx(%u),UpdateEiso fail(%d)",_this->mSensorIdx,err);
                    }

                    // 2. wait pass1 done
                    err = _this->UpdateEisoIdx();
                    if(EIS_RETURN_NO_ERROR != err)
                    {
                        EIS_ERR_R("Sidx(%u),UpdateEisoIdx fail(%d)",_this->mSensorIdx,err);
                    }
                break;
            case EIS_SW_STATE_UNINIT:
                EIS_LOG_R("Sidx(%u),UNINIT",_this->mSensorIdx);
                break;
            default:
                EIS_ERR_R("Sidx(%u),State Error(%d)",_this->mSensorIdx,eState);
        }

        eState = _this->GetEisState();
    }

    ::sem_post(&_this->mEisoSemEnd);
    EIS_LOG_R("Sidx(%u),post mEisoSemEnd",_this->mSensorIdx);

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::Init()
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        EIS_LOG_R("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Dynamic Debug ======

#if (EIS_DEBUG_FLAG)

    EIS_INF_R("EIS_DEBUG_FLAG on");
    g_debugDump_R = 1;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump_R = atoi(value);

#endif

    //====== Set State ======

    SetEisState(EIS_SW_STATE_ALIVE);

    //====== Member variable ======

    EIS_LOG_R("mSensorIdx(%u) init", mSensorIdx);

    //====== Prepare Memory for EISO DMA ======

    //IMEM
    m_pIMemDrv = IMemDrv::createInstance();
    if(m_pIMemDrv == NULL)
    {
        EIS_ERR_R("Null IMemDrv Obj");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    if(MTRUE != m_pIMemDrv->init())
    {
        EIS_ERR_R("m_pIMemDrv->init fail");
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    if(mEisoDmaBuf.memID == -5)
    {
        MUINT32 eisMemSize = EISO_MEMORY_SIZE * EISO_BUFFER_NUM;

        CreateMemBuf(eisMemSize,1,&mEisoDmaBuf);
        if(mEisoDmaBuf.virtAddr == 0 && mEisoDmaBuf.phyAddr == 0)
        {
            EIS_ERR_R("create IMem fail");
            err = EIS_RETURN_MEMORY_ERROR;
            return err;
        }

        EIS_LOG_R("EisIMem : ID(%d),SZ(%u),VA(0x%x),PA(0x%x),cache(%d)",mEisoDmaBuf.memID,
                                                                      mEisoDmaBuf.size,
                                                                      mEisoDmaBuf.virtAddr,
                                                                      mEisoDmaBuf.phyAddr,
                                                                      mEisoDmaBuf.useNoncache);
    }

    //====== Create ISP Driver Object  ======

    m_pISPDrvObj = NSIspDrv_FrmB::IspDrv::createInstance();
    if(m_pISPDrvObj == NULL)
    {
        EIS_ERR_R("m_pISPDrvObj create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }

    if(MTRUE != m_pISPDrvObj->init(LOG_TAG))
    {
        EIS_ERR_R("m_pISPDrvObj->init() fail");
        err = EIS_RETURN_API_FAIL;
        return err;
    }

    EisIrq_key = m_pISPDrvObj->registerIrq(LOG_TAG);


    //====== Create INormalPipe_FrmB Object ======

    m_pNormalPipe = INormalPipe_FrmB::createInstance(mSensorIdx, LOG_TAG);
    if(m_pNormalPipe == NULL)
    {
        EIS_ERR_R("create INormalPipe_FrmB fail");
        return EIS_RETURN_NULL_OBJ;
    }

    //====== Create Pass1 Callback Class ======

    m_pEisP1Cb = new EisP1Cb(this);
    m_pNormalPipe->sendCommand(EPIPECmd_SET_EIS_CBFP,(MINTPTR)m_pEisP1Cb,-1,-1);

    #if 0   // opend it when needed
    // Command Queue
    m_pISPVirtDrv = m_pISPDrvObj->getCQInstance(ISP_DRV_CQ0);
    if(m_pISPVirtDrv == NULL)
    {
        EIS_ERR_R("m_pISPVirtDrv create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }
    #endif

    //====== Create Thread ======

    //> init semphore

    ::sem_init(&mEisoSem, 0, 0);
    ::sem_init(&mEisoSemEnd, 0, 0);

    //> create thread

    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_DETACHED);

    pthread_create(&mEisoThread,&attr,EisoThreadLoop,this);

    pthread_attr_destroy(&attr);

    //====== Increase Reference Count ======

    android_atomic_inc(&mUsers);

    EIS_LOG_R("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::Uninit()
{
    //====== Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers <= 0) // No more users
    {
        EIS_LOG_R("mSensorIdx(%u) has 0 user",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    // >= one user
    android_atomic_dec(&mUsers);

    if(mUsers == 0)
    {
        //====== Set State ======

        SetEisState(EIS_SW_STATE_UNINIT);

        EIS_LOG_R("mSensorIdx(%u) uninit",mSensorIdx);

        //====== Disable EIS ======

        //EnableEis(MFALSE);    // driver will reset HW

        //====== Destory ISP Driver Object ======

        if(m_pISPVirtDrv != NULL)
        {
            EIS_LOG_R("m_pISPVirtDrv uninit");
            m_pISPVirtDrv = NULL;
        }

        if(m_pISPDrvObj != NULL)
        {
            EIS_LOG_R("m_pISPDrvObj uninit");
            if(MTRUE != m_pISPDrvObj->uninit(LOG_TAG))
            {
                EIS_ERR_R("m_pISPDrvObj->uninit fail");
                err = EIS_RETURN_API_FAIL;
            }

            m_pISPDrvObj->destroyInstance();
            m_pISPDrvObj = NULL;
        }

        //====== Free Memory ======

        DestroyMemBuf(1,&mEisoDmaBuf);

        mEisoDmaBuf.memID = -5;
        mEisoDmaBuf.useNoncache = 1;
        mEisoDmaBuf.virtAddr = mEisoDmaBuf.phyAddr = mEisoDmaBuf.size = 0;

        if(m_pIMemDrv != NULL)
        {
            EIS_LOG_R("m_pIMemDrv uninit");
            if(MTRUE != m_pIMemDrv->uninit())
            {
                EIS_ERR_R("m_pIMemDrv->uninit fail");
                err = EIS_RETURN_API_FAIL;
            }

            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //====== Destory INormalPipe_FrmB ======

        if(m_pNormalPipe != NULL)
        {
            EIS_LOG_R("m_pNormalPipe uninit");
            m_pNormalPipe->destroyInstance(LOG_TAG);
            m_pNormalPipe = NULL;
        }

        //====== Clean Member Variable ======

        // EIS and related register setting
        memset(&mEisRegSetting,0,sizeof(EIS_REG_INFO));
        memset(&mEisoDmaSetting,0,sizeof(EISO_DMA_INFO));

        mIsConfig   = 0;
        mIsFirst    = 1;
        mIs2Pixel   = 0;
        mTotalMBNum = 0;
        mImgWidth   = 0;
        mImgHeight  = 0;
        mEisDivH    = 0;
        mEisDivV    = 0;
        mSensorType = EIS_NULL_SENSOR;
        mEisHwCfgSce = EIS_SCE_NONE;
        mEiso_addrIdx = 0;
        mEiso_virAddr = 0;
        mEiso_phyAddr = 0;

        // delete pass1 callback class

        delete m_pEisP1Cb;
        m_pEisP1Cb = NULL;

        // EISO data
        while(!mEisoData.empty())
        {
            mEisoData.pop();
        }

        //====== Set State ======

        SetEisState(EIS_SW_STATE_NONE);
    }
    else
    {
        EIS_LOG_R("mSensorIdx(%u) has %d users ",mSensorIdx,mUsers);
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    EIS_LOG_R("Cnt(%u),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR_R("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR_R("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            EIS_ERR_R("m_pIMemDrv->allocVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR_R("m_pIMemDrv->mapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG_R("-");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisDrvImp::DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    EIS_LOG_R("Cnt(%u)", bufCnt);

    MINT32 err = EIS_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                EIS_LOG_R("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR_R("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR_R("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            EIS_LOG_R("Buffer doesn't exist");
            return EIS_RETURN_NO_ERROR;
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR_R("m_pIMemDrv->unmapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            EIS_ERR_R("m_pIMemDrv->freeVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG_R("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::ConfigEisReg(const EIS_SCENARIO_ENUM &aSce,const MUINT32 &aSensorTg)
{
    if(mIsConfig == 1)
    {
        if(g_debugDump_R >= 1)
        {
            EIS_LOG_R("configured");
        }
        return EIS_RETURN_NO_ERROR;
    }
    else
    {
        //====== Sensor Info ======

        mSensorTg = aSensorTg;
        EIS_LOG_R("(idx,tg)=(%u,%u)",mSensorIdx,mSensorTg);

        //====== Record Configure Scenario ======

        mEisHwCfgSce = aSce;
        EIS_LOG_R("sce(%u)",mEisHwCfgSce);

        //====== EISO VA.PA and Index Init ======

        mEiso_addrIdx = 0;
        mEiso_virAddr = mEisoDmaBuf.virtAddr;
        mEiso_phyAddr = mEisoDmaBuf.phyAddr;

        //====== Sensor Independent Register ======

        // EISO setting
        mEisoDmaSetting.reg_eiso_base_addr = mEisoDmaBuf.phyAddr;
        mEisoDmaSetting.reg_eiso_xsize     = EISO_MEMORY_SIZE - 1; // fix number

        // ====== Write Register Value ======

        WriteReg(REG_EISO);
        WriteReg(REG_SGG);

        //====== Flag Setting ======

        mIsConfig = 1;

        EIS_LOG_R("-");
        return EIS_RETURN_NO_ERROR;
    }
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 EisDrvImp::EnableEis(const MBOOL &aEn)
{
    EIS_LOG_R("aEn(%d)",aEn);

    MINT32 err = EIS_RETURN_NO_ERROR;

    // [mark] ISP driver will take care EIS_EN
    // EIS_EN
    //if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_EIS, aEn, -1))
    //{
    //    EIS_ERR_R("EIS_EN fail");
    //    err = EIS_RETURN_API_FAIL;
    //}

    // [mark] This is only temp usage for ISP driver design. HW doesn't have EISO_EN bit.
    // EISO_EN
    if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_EISO, aEn, -1))
    {
        EIS_ERR_R("EISO_EN fail");
        err = EIS_RETURN_API_FAIL;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::UpdateEiso()
{
    if(EIS_SW_STATE_UNINIT == GetEisState())
    {
        return EIS_RETURN_NO_ERROR;
    }

    MINTPTR handle = -1;

    //====== Wait Vsync ======

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    waitIrq.Clear               = ISP_DRV_IRQ_CLEAR_NONE;
    waitIrq.UserInfo.Type       = ISP_DRV_IRQ_TYPE_INT;
    waitIrq.UserInfo.Status     = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
    waitIrq.UserInfo.UserKey    = EisIrq_key;
    waitIrq.Timeout             = 503;

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("wait Vsync");
    }

    if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
    {
        if(EIS_SW_STATE_UNINIT != GetEisState())
        {
            EIS_ERR_R("wait Vsync fail,state(%d)",mState);
            EIS_ERR_R("TG(%d),Clear(%d),Type(%d),Status(%d),UserKey(%u),Timeout(%d)",mSensorTg,
                                                                                     waitIrq.Clear,
                                                                                     waitIrq.UserInfo.Type,
                                                                                     waitIrq.UserInfo.Status,
                                                                                     waitIrq.UserInfo.UserKey,
                                                                                     waitIrq.Timeout);

            return EIS_RETURN_API_FAIL;
        }
        else
        {
            EIS_LOG_R("state(%d)",GetEisState());
            return EIS_RETURN_NO_ERROR;
        }
    }

    Mutex::Autolock lock(mIdxLock);

    //EIS_LOG_R("Eiso:idx(%u),PA(0x%08x)",mEiso_addrIdx,mEiso_phyAddr);

    //====== Get Handle ======

    if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_EISO,(MINTPTR)(&handle),(MINTPTR)(&(LOG_TAG))))
    {
        EIS_ERR_R("get EISO handle fail");
        return EIS_RETURN_API_FAIL;
    }

    //====== Write to NormalPipe =======

    IOPIPE_SET_MODUL_REG(handle,CAM_EISO_BASE_ADDR,mEiso_phyAddr);

    //====== Configure Done ======

    if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
    {
        EIS_ERR_R("EISO cofig done fail");
        m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1);
        return EIS_RETURN_API_FAIL;
    }

    //====== Release Handle ======

    if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1))
    {
        EIS_ERR_R("release EISO handle fail");
        return EIS_RETURN_API_FAIL;
    }

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("-");
    }
    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::UpdateEisoIdx()
{
    static MINT32 isFirst = 1;

    if(EIS_SW_STATE_UNINIT == GetEisState())
    {
        EIS_LOG_R("Eis state is EIS_SW_STATE_UNINIT");
        return EIS_RETURN_NO_ERROR;
    }

    //====== Wait Pass1 Done ======

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    waitIrq.Clear               = ISP_DRV_IRQ_CLEAR_NONE;
    waitIrq.UserInfo.Type       = ISP_DRV_IRQ_TYPE_INT;
    waitIrq.UserInfo.Status     = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
    waitIrq.UserInfo.UserKey    = EisIrq_key;
    waitIrq.Timeout             = 501;
    waitIrq.SpecUser = ISP_DRV_WAITIRQ_SPEUSER_EIS;

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("wait P1 done");
    }

    if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
    {
        if(EIS_SW_STATE_UNINIT != GetEisState())
        {
            EIS_ERR_R("wait P1 done fail,state(%d)",GetEisState());
            EIS_ERR_R("TG(%d),Clear(%d),Type(%d),Status(%d),UserKey(%u),Timeout(%d)",mSensorTg,
                                                                                     waitIrq.Clear,
                                                                                     waitIrq.UserInfo.Type,
                                                                                     waitIrq.UserInfo.Status,
                                                                                     waitIrq.UserInfo.UserKey,
                                                                                     waitIrq.Timeout);
        }
        else
        {
            EIS_LOG_R("[miss]state(%d)",GetEisState());
        }

        //> push repeat data, because pass1 done didn't come

        EISO_DATA eisoData;

        {
            Mutex::Autolock lock(mQueLock);

            if(isFirst == 1)
            {
                eisoData.va = mEiso_virAddr;
                eisoData.pa = mEiso_phyAddr;
            }
            else
            {
                MUINT32 tempEisoIdx = (mEiso_addrIdx == 0) ? EISO_BUFFER_NUM-1 : mEiso_addrIdx-1;

                eisoData.va = mEisoDmaBuf.virtAddr + (EISO_MEMORY_SIZE * tempEisoIdx);
                eisoData.pa = mEisoDmaBuf.phyAddr  + (EISO_MEMORY_SIZE * tempEisoIdx);
            }

            eisoData.timeStamp = 0;

            mEisoData.push(eisoData);

            EIS_LOG_R("miss:condition broadcast");
            mEisoCond.broadcast();
        }

        return EIS_RETURN_NO_ERROR;
    }

    Mutex::Autolock lock(mIdxLock);

    EISO_DATA eisoData;

    //====== Save Ready EISO Data ======

    {
        Mutex::Autolock lock(mQueLock);

        if(isFirst == 1)
        {
            eisoData.va = mEiso_virAddr;
            eisoData.pa = mEiso_phyAddr;
            isFirst = 0;
        }
        else
        {
            MUINT32 tempEisoIdx = (mEiso_addrIdx == 0) ? EISO_BUFFER_NUM-1 : mEiso_addrIdx-1;

            eisoData.va = mEisoDmaBuf.virtAddr + (EISO_MEMORY_SIZE * tempEisoIdx);
            eisoData.pa = mEisoDmaBuf.phyAddr  + (EISO_MEMORY_SIZE * tempEisoIdx);
        }

        eisoData.timeStamp = GetTimeStamp(waitIrq.EisMeta.tLastSOF2P1done_sec,waitIrq.EisMeta.tLastSOF2P1done_usec);

        mEisoData.push(eisoData);
        if(g_debugDump_R > 0)
        {
        EIS_LOG_R("condition broadcast,TS(%lld)",eisoData.timeStamp);
        }
        mEisoCond.broadcast();
    }

    //====== Update EISO Phy Addr ======

    mEiso_addrIdx++;

    if(mEiso_addrIdx == EISO_BUFFER_NUM)
    {
        mEiso_addrIdx = 0;
    }

    mEiso_virAddr = mEisoDmaBuf.virtAddr + (EISO_MEMORY_SIZE * mEiso_addrIdx);
    mEiso_phyAddr = mEisoDmaBuf.phyAddr  + (EISO_MEMORY_SIZE * mEiso_addrIdx);

    if(g_debugDump_R > 0)
    {
    EIS_LOG_R("Eiso:idx(%u->%u),NEXT(va,pa)=(0x%08x,0x%08x)",(mEiso_addrIdx == 0)?EISO_BUFFER_NUM-1:mEiso_addrIdx-1,
                                                             mEiso_addrIdx,
                                                             mEiso_virAddr,
                                                             mEiso_phyAddr);
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrvImp::SetEisoThreadState(EIS_SW_STATE_ENUM aState)
{
    EIS_LOG_R("Sidx(%u),aState(%d)",mSensorIdx,aState);

    if(mEisHwSupport == MFALSE && aState == EIS_SW_STATE_ALIVE)
    {
        EIS_LOG_R("Sidx(%u)EIS HW not support",mSensorIdx);
        return;
    }

    if(aState == EIS_SW_STATE_ALIVE)
    {
        //====== Clear Pass1 Done Signal First ======

        ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

        waitIrq.Clear               = ISP_DRV_IRQ_CLEAR_STATUS;
        waitIrq.UserInfo.Type       = ISP_DRV_IRQ_TYPE_INT;
        waitIrq.UserInfo.Status     = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
        waitIrq.UserInfo.UserKey    = m_pISPDrvObj->registerIrq(LOG_TAG);
        waitIrq.Timeout             = 401;

        if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
        {
            EIS_ERR_R("ISP_DRV_IRQ_CLEAR_STATUS fail");
            EIS_ERR_R("TG(%d),Clear(%d),Type(%d),Status(%d),UserKey(%u),Timeout(%d)",mSensorTg,
                                                                                     waitIrq.Clear,
                                                                                     waitIrq.UserInfo.Type,
                                                                                     waitIrq.UserInfo.Status,
                                                                                     waitIrq.UserInfo.UserKey,
                                                                                     waitIrq.Timeout);
        }

        //====== Post Sem ======

        ::sem_post(&mEisoSem);
        EIS_LOG_R("post mEisoSem");
    }
    else if(aState == EIS_SW_STATE_UNINIT)
    {
        SetEisState(EIS_SW_STATE_UNINIT);
    }
    else if(aState == EIS_SW_STATE_UNINIT_CHECK)
    {
        //====== Destroy Thread ======

        if(mEisHwSupport == MFALSE)
        {
            ::sem_post(&mEisoSem);
            EIS_LOG_R("post mEisoSem");
        }

        //> wait sem
        EIS_LOG_R("wait mEisoSemEnd");
        ::sem_wait(&mEisoSemEnd);
        EIS_LOG_R("got mEisoSemEnd");

        //> delete thread : no need, suggested by ss team (because already use mEisoSemEnd to guarantee EisoThread is exit)
        //pthread_join(mEisoThread, NULL);
    }
    EIS_LOG_R("-");
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisDrvImp::SetFirstFrame(const MUINT32 &aFirst)
{
    EIS_LOG_R("First(0x%x)", (aFirst & 0x1));

    MINTPTR handle = -1;

    mEisRegSetting.reg_eis_prep_me_ctrl2 &= 0x07FFF;

    if(aFirst == 1)
    {
        mEisRegSetting.reg_eis_prep_me_ctrl2 = (aFirst << 15) | mEisRegSetting.reg_eis_prep_me_ctrl2;
    }

    EIS_LOG_R("reg_eis_prep_me_ctrl2(0x%08x)",mEisRegSetting.reg_eis_prep_me_ctrl2);

    //> get handle
    if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_EIS,(MINTPTR)(&handle),(MINTPTR)(&(LOG_TAG))))
    {
        EIS_ERR_R("get EIS handle fail");
        return EIS_RETURN_API_FAIL;
    }

    //> setting

    IOPIPE_SET_MODUL_REG(handle,CAM_EIS_PREP_ME_CTRL2,mEisRegSetting.reg_eis_prep_me_ctrl2);

    //> configure done

    if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
    {
        EIS_ERR_R("EISO cofig done fail");
        m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1);
        return EIS_RETURN_API_FAIL;
    }

    //> release handle

    if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1))
    {
        EIS_ERR_R("release EISO handle fail");
        return EIS_RETURN_API_FAIL;
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisDrvImp::GetFirstFrameInfo()
{
    return mIsFirst ? 0 : 1;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisDrvImp::Get2PixelMode()
{
    return mIs2Pixel ? 1 : 0;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrvImp::GetEisInputSize(MUINT32 *aWidth, MUINT32 *aHeight)
{
    *aWidth  = mImgWidth;
    *aHeight = mImgHeight;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisDrvImp::GetEisDivH()
{
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mEisDivH(%u)", mEisDivH);
    }
    return mEisDivH;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisDrvImp::GetEisDivV()
{
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mEisDivV(%u)", mEisDivV);
    }
    return mEisDivV;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32 EisDrvImp::GetEisMbNum()
{
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mTotalMBNum(%u)", mTotalMBNum);
    }
    return mTotalMBNum;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL EisDrvImp::GetEisSupportInfo(const MUINT32 &aSensorIdx)
{
    NormalPipe_EIS_Info eisInfo;

    m_pNormalPipe->sendCommand(EPIPECmd_GET_EIS_INFO,aSensorIdx,(MINTPTR)&eisInfo, -1);

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("sensorIdx(%u),support(%d)",aSensorIdx,eisInfo.mSupported);
    }

    mEisHwSupport = eisInfo.mSupported;
    return eisInfo.mSupported;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::GetEisHwStatistic(EIS_STATISTIC_STRUCT *apEisStat,const MINT64 &aTimeStamp)
{

    if(g_debugDump_R > 0)
    {
    EIS_LOG_R("imgTS(%lld)",aTimeStamp);
    }

    if(aTimeStamp == 0)
    {
        return EIS_RETURN_NO_ERROR;
    }
    else
    {
        //====== Wait Semaphore ======

        MUINT32 *pEisoAddr = NULL;
        MINT32 hasMatch = 0;

        {
            Mutex::Autolock lock(mQueLock);

            while(!mEisoData.empty())
            {
                if(mEisoData.front().timeStamp < aTimeStamp)    // EIS missed pass1 done
                {
                    EIS_LOG_R("miss,(va,pa,ts)=(0x%08x,0x%08x,%lld)",mEisoData.front().va,mEisoData.front().pa,mEisoData.front().timeStamp);
                    mEisoData.pop();
                }
                else if(mEisoData.front().timeStamp == aTimeStamp)  // match
                {
                    pEisoAddr = (MUINT32 *)mEisoData.front().va;
                    mEisoData.pop();
                    hasMatch = 1;
                    break;
                }
                else    // EISO timestamp is bigger than imgTS
                {
                    hasMatch = 0;
                    break;
                }
            }

            if(mEisoData.empty() && hasMatch == 0)
            {
                //EIS_LOG_R("eiso condition wait");
                mEisoCond.waitRelative(mQueLock,(long long int)EISO_CONDITION_WAIT_TIME*1000000LL); //conditaional wait 5ms
                //EIS_LOG_R("eiso condition wait done");

                while(!mEisoData.empty())
                {
                    if(mEisoData.front().timeStamp < aTimeStamp)    // EIS missed pass1 done
                    {
                        EIS_LOG_R("miss,(va,pa,ts)=(0x%08x,0x%08x,%lld)",mEisoData.front().va,mEisoData.front().pa,mEisoData.front().timeStamp);
                        mEisoData.pop();
                    }
                    else if(mEisoData.front().timeStamp == aTimeStamp)  // match
                    {
                        pEisoAddr = (MUINT32 *)mEisoData.front().va;
                        mEisoData.pop();
                        hasMatch = 1;
                        break;
                    }
                    else    // EISO timestamp is bigger than imgTS
                    {
                        hasMatch = 0;
                        break;
                    }
                }
            }
        }

        if(hasMatch == 1)
        {
            //====== Get EISO Data ======

            if(pEisoAddr == NULL)
            {
                EIS_ERR_R("pEisoAddr is NULL");
            }

            if(g_debugDump_R > 0)
            {
            EIS_LOG_R("pEisoAddr(0x%08x)",(MINTPTR)pEisoAddr);
            }

            //====== Get EIS HW Statistic ======

            for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
            {
                if(i != 0)
                {
                    pEisoAddr += 2;  // 64bits(8bytes)

                    if(g_debugDump_R == 3)
                    {
                        EIS_LOG_R("i(%d),pEisoAddr(0x%08x)",i,(MINTPTR)pEisoAddr);
                    }
                }

                apEisStat->i4LMV_X2[i]   = Complement2(*pEisoAddr & 0x1F, 5);               //[0:4]
                apEisStat->i4LMV_Y2[i]   = Complement2(((*pEisoAddr & 0x3E0) >> 5), 5);    //[5:9]
                apEisStat->SAD[i]        = (*pEisoAddr & 0x7FC00) >> 10;                    //[10:18]
                apEisStat->NewTrust_X[i] = (*pEisoAddr & 0x03F80000) >> 19;                 //[19:25]
                apEisStat->NewTrust_Y[i] = ((*pEisoAddr & 0xFC000000) >> 26) + ((*(pEisoAddr+1) & 0x00000001) << 6);    //[26:32]
                apEisStat->i4LMV_X[i]    = Complement2(((*(pEisoAddr + 1) & 0x00003FFE) >> 1), 13);     //[33:45] -> [1:13]
                apEisStat->i4LMV_Y[i]    = Complement2(((*(pEisoAddr + 1) & 0x07FFC000) >> 14), 13);    //[46:58] -> [14:26]
                apEisStat->SAD2[i]       = 0;
                apEisStat->AVG_SAD[i]    = 0;
            }

            if(g_debugDump_R == 3)
            {
                for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
                {
            		EIS_LOG_R("EIS[%d]=lmv(%d,%d),lmv2(%d,%d),trust(%d,%d),sad(%d)",i,
            				                                                      apEisStat->i4LMV_X[i], apEisStat->i4LMV_Y[i],
            				                                                      apEisStat->i4LMV_X2[i], apEisStat->i4LMV_Y2[i],
            				                                                      apEisStat->NewTrust_X[i], apEisStat->NewTrust_Y[i],
            				                                                      apEisStat->SAD[i]);
                }
                EIS_LOG_R("-");
            }
        }
        else
        {
            EIS_WRN_R("no matching EISO data");
            return EIS_RETURN_EISO_MISS;
        }
    }

    return EIS_RETURN_NO_ERROR;
}

/******************************************************************************
*
*******************************************************************************/
MVOID EisDrvImp::DumpReg()
{
    EIS_LOG_R("mSensorTg(%u)",mSensorTg);

    // enable bit
    EIS_LOG_R("EIS_EN(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_CTL_EN2,EIS_EN));
    EIS_LOG_R("ESFKO_EN(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_CTL_DMA_EN, ESFKO_EN));

    // SGG
    EIS_LOG_R("SGG_EN(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_CTL_EN1, SGG_EN));
    EIS_LOG_R("SGG_PGN(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_SGG_PGN));
    EIS_LOG_R("SGG_GMRC_1(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_SGG_GMR, SGG_GMR1));
    EIS_LOG_R("SGG_GMRC_2(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_SGG_GMR, SGG_GMR2));
    EIS_LOG_R("SGG_GMRC_3(0x%08x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_SGG_GMR, SGG_GMR3));

    // EISO
    EIS_LOG_R("EISO_ADDR(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EISO_BASE_ADDR));
    EIS_LOG_R("EISO_XSIZE(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EISO_XSIZE));

    //EIS
    EIS_LOG_R("EIS_SEL(0x%x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_CTL_SEL,EIS_RAW_SEL));
    EIS_LOG_R("EIS_SEL(0x%x)",(MUINT32)ISP_READ_BITS_NOPROTECT(m_pISPDrvObj,CAM_CTL_SPARE3,EIS_DB_LD_SEL));

    MUINT32 regVal = (MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_PREP_ME_CTRL1);

    EIS_LOG_R("EIS_PREP_ME_CTRL1(0x%08x)",regVal);
    EIS_LOG_R("win_numV(0x%08x)",(regVal & 0xF0000000) >> 28);
    EIS_LOG_R("win_numH(0x%08x)",(regVal & 0xE000000) >> 25);
    EIS_LOG_R("rp_numV(0x%08x)",(regVal& 0x1E00000) >> 21);
    EIS_LOG_R("knee(0x%08x)",(regVal & 0x1E0000) >> 17);
    EIS_LOG_R("clip(0x%08x)",(regVal & 0x1E000) >> 13);
    EIS_LOG_R("rp_numH(0x%08x)",(regVal & 0x1F00) >> 8);
    EIS_LOG_R("subG_en(0x%08x)",(regVal & 0x40) >> 6);
    EIS_LOG_R("DS_IIR_V(0x%08x)",(regVal & 0x38) >> 3);
    EIS_LOG_R("DS_IIR_H(0x%08x)",regVal & 0x7);

    regVal = (MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_PREP_ME_CTRL2);

    EIS_LOG_R("EIS_PREP_ME_CTRL2(0x%08x)",regVal);
    EIS_LOG_R("first_frame:0x%08x",(regVal & 0x8000) >> 15);
    EIS_LOG_R("write_en(rp_modi):0x%08x",(regVal & 0x4000) >> 14);
    EIS_LOG_R("FIR_H:(0x%08x,0x%08x)",(regVal & 0x3F00) >> 8);
    EIS_LOG_R("GAIN_IIR_V:(0x%08x,0x%08x)",(regVal & 0xE0) >> 5);
    EIS_LOG_R("GAIN_IIR_H:(0x%08x,0x%08x)",(regVal & 0x1C) >> 2);
    EIS_LOG_R("proc_gain:(0x%08x,0x%08x)",regVal & 0x3);

    EIS_LOG_R("EIS_LMV_TH(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_LMV_TH));
    EIS_LOG_R("EIS_FL_OFFSET(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_FL_OFFSET));
    EIS_LOG_R("EIS_MB_OFFSET(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_MB_OFFSET));
    EIS_LOG_R("EIS_MB_INTERVAL(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_MB_INTERVAL));
    EIS_LOG_R("EIS_IMAGE_CTRL(0x%08x)",(MUINT32)ISP_READ_REG_NOPROTECT(m_pISPDrvObj,CAM_EIS_IMAGE_CTRL));

    EIS_LOG_R("-");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisDrvImp::Complement2(MUINT32 value, MUINT32 digit)
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID EisDrvImp::BoundaryCheck(MUINT32 &aInput,const MUINT32 &upBound,const MUINT32 &lowBound)
{
    if(aInput > upBound)
    {
        aInput = upBound;
    }

    if(aInput < lowBound)
    {
        aInput = lowBound;
    }
}

/*******************************************************************************
*
*******************************************************************************/
MINT64 EisDrvImp::GetTimeStamp(const MUINT32 &aSec,const MUINT32 &aUs)
{
    MINT64 tmp = aSec;
    tmp *= 1000000000LL;
    tmp += ( (MINT64)aUs * 1000LL);
    return  tmp;
}

/*******************************************************************************
*
*******************************************************************************/
MINT32 EisDrvImp::WriteReg(const REG_NAME_ENUM &aRegName)
{
    MINTPTR handle = -1;

    //====== Get Handle ======

    switch(aRegName)
    {
        case REG_EIS:

            //> get handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_EIS,(MINTPTR)(&handle),(MINTPTR)(&(LOG_TAG))))
            {
                EIS_ERR_R("get EIS handle fail");
                return EIS_RETURN_API_FAIL;
            }

            //> setting

            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_PREP_ME_CTRL1,mEisRegSetting.reg_eis_prep_me_ctrl1);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_PREP_ME_CTRL2,mEisRegSetting.reg_eis_prep_me_ctrl2);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_LMV_TH,       mEisRegSetting.reg_eis_lmv_th);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_FL_OFFSET,    mEisRegSetting.reg_eis_fl_offset);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_MB_OFFSET,    mEisRegSetting.reg_eis_mb_offset);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_MB_INTERVAL,  mEisRegSetting.reg_eis_mb_interval);
            IOPIPE_SET_MODUL_REG(handle,CAM_EIS_IMAGE_CTRL,   mEisRegSetting.reg_eis_image_ctrl);

            //> configure done

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
            {
                EIS_ERR_R("EIS cofig done fail");
                goto lbWriteRegExitFail;
            }

            //> release handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1))
            {
                EIS_ERR_R("release EIS handle fail");
                return EIS_RETURN_API_FAIL;
            }

            break;
        case REG_EISO:

            //> get handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_EISO,(MINTPTR)(&handle),(MINTPTR)(&(LOG_TAG))))
            {
                EIS_ERR_R("get EISO handle fail");
                return EIS_RETURN_API_FAIL;
            }

            //> setting

            IOPIPE_SET_MODUL_REG(handle,CAM_EISO_BASE_ADDR,mEisoDmaSetting.reg_eiso_base_addr);
            IOPIPE_SET_MODUL_REG(handle,CAM_EISO_XSIZE,mEisoDmaSetting.reg_eiso_xsize);

            //> configure done

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
            {
                EIS_ERR_R("EISO cofig done fail");
                goto lbWriteRegExitFail;
            }

            //> release handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1))
            {
                EIS_ERR_R("release EISO handle fail");
                return EIS_RETURN_API_FAIL;
            }

            break;
        case REG_SGG:

            //> get handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_SGG1,(MINTPTR)(&handle),(MINTPTR)(&(LOG_TAG))))
            {
                EIS_ERR_R("get SGG handle fail");
                return EIS_RETURN_API_FAIL;
            }

            //> setting

            IOPIPE_SET_MODUL_REG(handle,CAM_SGG_PGN,0x10);
            IOPIPE_SET_MODUL_REG(handle,CAM_SGG_GMR,0x7F3F1F);

            //> configure done

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
            {
                EIS_ERR_R("SGG cofig done fail");
                goto lbWriteRegExitFail;
            }

            //> release handle

            if(MTRUE != m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1))
            {
                EIS_ERR_R("release SGG handle fail");
                return EIS_RETURN_API_FAIL;
            }

            break;
        default:
            EIS_ERR_R("wrong reg(%d)",aRegName);
    }

    return EIS_RETURN_NO_ERROR;

lbWriteRegExitFail:

    m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(LOG_TAG)),-1);

    return EIS_RETURN_API_FAIL;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrvImp::SetEisState(const EIS_SW_STATE_ENUM &aState)
{
    Mutex::Autolock lock(mSateLock);
    mState = aState;
    EIS_LOG_R("aState(%d),mState(%d)",aState,mState);
}

/*******************************************************************************
*
********************************************************************************/
EIS_SW_STATE_ENUM EisDrvImp::GetEisState()
{
    Mutex::Autolock lock(mSateLock);
    return mState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisDrvImp::ChangeThreadSetting(char const *userName)
{
    //> set name

    ::prctl(PR_SET_NAME,userName, 0, 0, 0);

    //> set policy/priority
    {
        const MINT32 expect_policy   = SCHED_OTHER;
        const MINT32 expect_priority = NICE_CAMERA_3A_MAIN - 4;
        MINT32 policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);

        EIS_LOG_R("policy:(expect,result)=(%d,%d),priority:(expect,result)=(%d, %d)",expect_policy,policy,expect_priority,priority);
    }
}

/*******************************************************************************
*
********************************************************************************/
EisP1Cb::EisP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

/*******************************************************************************
*
********************************************************************************/
EisP1Cb::~EisP1Cb()
{
    m_pClassObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void EisP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    EisDrvImp *_this = reinterpret_cast<EisDrvImp *>(m_pClassObj);
	EIS_INPUT_INFO *aEisInInfo = (EIS_INPUT_INFO *)pInput;
	EIS_REG_CFG *a_pEisCfgData = (EIS_REG_CFG *)pOutput;
    MUINT32 subG_en = 0;
    MUINT32 eis_sel = 1;    // RAW domain

    if(_this->mIsConfig == 0)
    {
        EIS_LOG_R("not config done");
        a_pEisCfgData->bEIS_Bypass = MTRUE;
        return;
    }

    // ====== Dynamic Debug ======

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("TG(%u,%u),HBIN(%u,%u),RMX(%u,%u)",aEisInInfo->sTGOut.w,aEisInInfo->sTGOut.h,
                                                     aEisInInfo->sHBINOut.w,aEisInInfo->sHBINOut.h,
                                                     aEisInInfo->sRMXOut.w,aEisInInfo->sRMXOut.h);
        EIS_LOG_R("YUV(%d),TWIN(%d)",aEisInInfo->bYUVFmt,aEisInInfo->bIsTwin);
    }

    //====== EIS Input Size ======

    if(_this->mImgWidth != aEisInInfo->sTGOut.w || _this->mImgHeight != aEisInInfo->sTGOut.h)
    {
        EIS_LOG_R("first:new(%u,%u),old(%u,%u)",aEisInInfo->sTGOut.w,aEisInInfo->sTGOut.h,_this->mImgWidth,_this->mImgHeight);
        _this->mIsFirst = 1;
        a_pEisCfgData->bEIS_Bypass = MFALSE;
    }
    else
    {
        _this->mIsFirst = 0;
        a_pEisCfgData->bEIS_Bypass = MTRUE;
    }

    _this->mImgWidth   = aEisInInfo->sTGOut.w;
    _this->mImgHeight  = aEisInInfo->sTGOut.h;

    // ====== Image Size Dependent Register ======

    if(aEisInInfo->bYUVFmt== MFALSE)    // RAW sensor specific register
    {
        //> SGG setting

        a_pEisCfgData->bSGG_Bypass = MTRUE;
        a_pEisCfgData->bSGG_EN = MFALSE;

        //> sensor type
        _this->mSensorType = EIS_RAW_SENSOR;
    }
    else if(aEisInInfo->bYUVFmt== MTRUE)   // YUV sensor specific register
    {
        //> SGG setting

        a_pEisCfgData->bSGG_Bypass = MFALSE;
        a_pEisCfgData->bSGG_EN = MTRUE;
        a_pEisCfgData->SGG_SEL = 0; // HW default value

        //> sensor type
        _this->mSensorType = EIS_YUV_SENSOR;
    }
    else
    {
        EIS_ERR_R("Not support sensor type(%d)");
        return;
    }

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("mImgWidth(%u),mImgHeight(%u),mSensorType(%d)",_this->mImgWidth,_this->mImgHeight,_this->mSensorType);
        EIS_LOG_R("sce(%u),eis_sel(%u),2Pixel(%u)",_this->mEisHwCfgSce,eis_sel,_this->mIs2Pixel);
    }

    //> EIS enable bit

    a_pEisCfgData->bEIS_EN = MTRUE;

    //> MUX select, no use. ISP driver already fix EIS_RAW_SEL to 1

    a_pEisCfgData->EIS_SEL = eis_sel;

    if(a_pEisCfgData->bEIS_Bypass == MTRUE)
    {
        _this->mEisRegSetting.reg_eis_prep_me_ctrl2 &= 0x07FFF;

        if(g_debugDump_R > 0)
        {
            EIS_LOG_R("me_ctrl2(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl2);
        }

        a_pEisCfgData->_EIS_REG.CTRL_1   = _this->mEisRegSetting.reg_eis_prep_me_ctrl1;
        a_pEisCfgData->_EIS_REG.CTRL_2   = _this->mEisRegSetting.reg_eis_prep_me_ctrl2;
        a_pEisCfgData->_EIS_REG.LMV_TH   = _this->mEisRegSetting.reg_eis_lmv_th;
        a_pEisCfgData->_EIS_REG.FL_ofs   = _this->mEisRegSetting.reg_eis_fl_offset;
        a_pEisCfgData->_EIS_REG.MB_ofs   = _this->mEisRegSetting.reg_eis_mb_offset;
        a_pEisCfgData->_EIS_REG.MB_int   = _this->mEisRegSetting.reg_eis_mb_interval;
        a_pEisCfgData->_EIS_REG.GMV      = 0;  // not use
        a_pEisCfgData->_EIS_REG.ERR_CTRL = 0xF0000;  // HW default value, not use
        a_pEisCfgData->_EIS_REG.IMG_CTRL = _this->mEisRegSetting.reg_eis_image_ctrl;
        a_pEisCfgData->bEIS_Bypass       = MFALSE;
        return;
    }

    //====== Setting Depend on Image Size ======

    MUINT32 win_numV, win_numH;
    MUINT32 rp_numV, rp_numH;
    MUINT32 knee_1, knee_2;
    MUINT32 proc_gain;
    MUINT32 dc_dl;
    MUINT32 vert_shr, hori_shr;
    MUINT32 rp_offsetV, rp_offsetH;
    MUINT32 win_sizeV, win_sizeH;

    switch(_this->mEisHwCfgSce)
    {
        case EIS_SCE_EIS :
        case EIS_SCE_EIS_PLUS :
        case EIS_SCE_MFBLL :
        case EIS_SCE_MT :
            // horizontal
            if(_this->mImgWidth > HD_8M_WIDTH)
            {
                _this->mEisDivH = 4;
            }
            else if(_this->mImgWidth > HD_720P_WIDTH)
            {
                _this->mEisDivH = 2;
            }
            else
            {
                _this->mEisDivH = 1;
            }

            if(_this->mImgWidth > D1_WIDTH)
            {
                win_numH = 4;
            }
            else if(_this->mImgWidth > CIF_WIDTH)
            {
                win_numH = 4;
            }
            else
            {
                win_numH = 2;
            }

            // vertical
            if(_this->mImgHeight > HD_8M_HEIGHT)
            {
                _this->mEisDivV = 4;
            }
            else if(_this->mImgHeight > HD_720P_HEIGHT)
            {
                _this->mEisDivV = 2;
            }
            else
            {
                _this->mEisDivV = 1;
            }

            if(_this->mImgHeight > D1_HEIGHT)
            {
                win_numV = 8;
            }
            else if( _this->mImgHeight > CIF_HEIGHT)
            {
                win_numV = 4;
            }
            else
            {
                win_numV = 3;
            }
        break;
        default :
            EIS_ERR_R("Wrong EIS scenario(%d)",_this->mEisHwCfgSce);
            return;
    }

    _this->mTotalMBNum = win_numH * win_numV;

    //====== Fix Setting ======

    knee_1 = 4;
    knee_2 = 6;
    proc_gain = 0;
    _this->mEisRegSetting.reg_eis_lmv_th = 0;    // not use right now
    _this->mEisRegSetting.reg_eis_fl_offset = 0;

    //====== Setting Calculateing ======

    // decide dc_dl
    if(_this->mImgWidth > CIF_WIDTH && _this->mImgHeight > CIF_HEIGHT)
    {
        dc_dl = 32;
    }
    else
    {
        dc_dl = 16;
    }

    // decide vert_shr and hori_shr
    if(_this->mEisDivH == 1)
    {
        hori_shr = 3;
    }
    else
    {
        hori_shr = 4;
    }

    if(_this->mEisDivV == 1)
    {
        vert_shr = 3;
    }
    else
    {
        vert_shr = 4;
    }

    //Decide MB_OFFSET
    MUINT32 dead_left, dead_upper;

    dead_left  = (1 << hori_shr) * 2 + dc_dl;
    dead_upper = (1 << vert_shr) * 2;

    rp_offsetH = (dead_left  + 16 * _this->mEisDivH + 8) / _this->mEisDivH;
    rp_offsetV = (dead_upper + 16 * _this->mEisDivV + 8) / _this->mEisDivV;

    //Decide MB_INTERVAL
    MUINT32 first_win_left_corner, first_win_top_corner;
    MUINT32 active_sizeH,active_sizeV;

    first_win_left_corner = (rp_offsetH - 16) * _this->mEisDivH;
    first_win_top_corner  = (rp_offsetV - 16) * _this->mEisDivV;

    active_sizeH = _this->mImgWidth  - first_win_left_corner - 16;
    active_sizeV = _this->mImgHeight - first_win_top_corner - 8;

    win_sizeH = active_sizeH / win_numH / _this->mEisDivH;
    win_sizeV = active_sizeV / win_numV / _this->mEisDivV;

    //Decide rp_numH, rp_numV;
    rp_numH = ((win_sizeH - 1) / 16) - 1;
    rp_numV = ((win_sizeV - 1) / 16) - 1;

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("first_win_left_corner(%u),first_win_top_corner(%u)",first_win_left_corner,first_win_top_corner);
        EIS_LOG_R("active_sizeH(%u),active_sizeV(%u)",active_sizeH,active_sizeV);
        EIS_LOG_R("win_sizeH(%u),win_sizeV(%u)",win_sizeH,win_sizeV);
        EIS_LOG_R("rp_numH(%u),rp_numV(%u)",rp_numH,rp_numV);
    }

    if(rp_numH > 16)
    {
        rp_numH = 16;
    }

    MUINT32 tempRpV = 2048 / (win_numH * win_numV * rp_numH);
    tempRpV = std::min((MUINT32)8,tempRpV);
    rp_numV = std::min(tempRpV,rp_numV);

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("tempRpV(%u)",tempRpV);
    }

    // MB_OFFSET value check

    MUINT32 offset_lowBound_H, offset_lowBound_V, offset_upperBound_H, offset_upperBound_V;
    MINT32 temp_fl_offset_H = (_this->mEisRegSetting.reg_eis_fl_offset & 0xFFF0000) >> 16;
    MINT32 temp_fl_offset_V = _this->mEisRegSetting.reg_eis_fl_offset & 0xFFF;

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("temp_fl_offset_H(%d)",temp_fl_offset_H);
        EIS_LOG_R("temp_fl_offset_V(%d)",temp_fl_offset_V);
    }

    // low bound
    if(temp_fl_offset_H < 0)
    {
        offset_lowBound_H = 11 - temp_fl_offset_H;
    }
    else
    {
        offset_lowBound_H = 11 + temp_fl_offset_H;
    }


    if(temp_fl_offset_V < 0)
    {
        offset_lowBound_V = 9 - temp_fl_offset_V;
    }
    else
    {
        offset_lowBound_V = 9 + temp_fl_offset_V;
    }

    // up bound
    if(temp_fl_offset_H > 0)
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mEisDivH) - (rp_numH*16) - temp_fl_offset_H - (win_sizeH*(win_numH-1));
    }
    else
    {
        offset_upperBound_H = (_this->mImgWidth/_this->mEisDivH) - (rp_numH*16) - 1 - (win_sizeH*(win_numH-1));
    }

    if(temp_fl_offset_V > 0)
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mEisDivV) - (rp_numV*16) - temp_fl_offset_V - (win_sizeV*(win_numV-1));
    }
    else
    {
        offset_upperBound_V = (_this->mImgHeight/_this->mEisDivV) - (rp_numV*16) - 1 - (win_sizeV*(win_numV-1));
    }

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("ori rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);
        EIS_LOG_R("bound: H(%d/%d),V(%d/%d)",offset_lowBound_H,offset_upperBound_H,offset_lowBound_V,offset_upperBound_V);
    }

    _this->BoundaryCheck(rp_offsetH,offset_upperBound_H,offset_lowBound_H);
    _this->BoundaryCheck(rp_offsetV,offset_upperBound_V,offset_lowBound_V);

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("final rp_offset (H/V)=(%d/%d)",rp_offsetH,rp_offsetV);
    }

    // MB_INTERVAL value check

    MUINT32 interval_lowBound_H, interval_lowBound_V, interval_upperBound_H, interval_upperBound_V;

    // low bound
    interval_lowBound_H = (rp_numH + 1) * 16;
    interval_lowBound_V = (rp_numV + 1) * 16 + 1;

    // up bound
    if(temp_fl_offset_H > 0)
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mEisDivH) - rp_offsetH - (rp_numH*16) - temp_fl_offset_H) / (win_numH-1);
    }
    else
    {
        interval_upperBound_H = ((_this->mImgWidth/_this->mEisDivH) - rp_offsetH - (rp_numH*16) - 1) / (win_numH-1);
    }

    if(temp_fl_offset_V > 0)
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mEisDivV) - rp_offsetV - (rp_numV*16) - temp_fl_offset_V) / (win_numV-1);
    }
    else
    {
        interval_upperBound_V = ((_this->mImgHeight/_this->mEisDivV) - rp_offsetV - (rp_numV*16) - 1) / (win_numV-1);
    }

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("ori win_size (H/V)=(%d/%d)",win_sizeH,win_sizeV);
        EIS_LOG_R("bound: H(%d/%d),V(%d/%d)",interval_lowBound_H,interval_upperBound_H,interval_lowBound_V,interval_upperBound_V);
    }

    _this->BoundaryCheck(win_sizeH,interval_upperBound_H,interval_lowBound_H);
    _this->BoundaryCheck(win_sizeV,interval_upperBound_V,interval_lowBound_V);

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("final win_size (H/V)=(%d/%d)",win_sizeH,win_sizeV);
    }

    //====== Intrgrate Setting ======

    _this->mEisRegSetting.reg_eis_prep_me_ctrl1 = (win_numV << 28) | (win_numH << 25) | (rp_numV << 21) |
                                           (knee_1 << 17) | (knee_2 << 13) | (rp_numH << 8) |
                                           (subG_en << 6) | (_this->mEisDivV << 3) | _this->mEisDivH;

    _this->mEisRegSetting.reg_eis_prep_me_ctrl2 = (1 << 15) | (1 << 14) | (dc_dl << 8) | (vert_shr << 5) | (hori_shr << 2) | proc_gain;

    _this->mEisRegSetting.reg_eis_mb_offset   = (rp_offsetH << 16) | rp_offsetV;
    _this->mEisRegSetting.reg_eis_mb_interval = (win_sizeH << 16) | win_sizeV;
    _this->mEisRegSetting.reg_eis_image_ctrl  = (_this->mImgWidth << 16) | _this->mImgHeight;

    //======= Set to Output Data ======

    //> EIS register

    a_pEisCfgData->_EIS_REG.CTRL_1   = _this->mEisRegSetting.reg_eis_prep_me_ctrl1;
    a_pEisCfgData->_EIS_REG.CTRL_2   = _this->mEisRegSetting.reg_eis_prep_me_ctrl2;
    a_pEisCfgData->_EIS_REG.LMV_TH   = _this->mEisRegSetting.reg_eis_lmv_th;
    a_pEisCfgData->_EIS_REG.FL_ofs   = _this->mEisRegSetting.reg_eis_fl_offset;
    a_pEisCfgData->_EIS_REG.MB_ofs   = _this->mEisRegSetting.reg_eis_mb_offset;
    a_pEisCfgData->_EIS_REG.MB_int   = _this->mEisRegSetting.reg_eis_mb_interval;
    a_pEisCfgData->_EIS_REG.GMV      = 0;  // not use
    a_pEisCfgData->_EIS_REG.ERR_CTRL = 0xF0000;  // HW default value, not use
    a_pEisCfgData->_EIS_REG.IMG_CTRL = _this->mEisRegSetting.reg_eis_image_ctrl;

    //====== Debug ======

    if(g_debugDump_R > 0)
    {
        EIS_LOG_R("reg_eis_prep_me_ctrl1(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl1);
    	EIS_LOG_R("win_numV:(0x%08x,0x%08x)",win_numV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0xF0000000) >> 28);
        EIS_LOG_R("win_numH:(0x%08x,0x%08x)",win_numH,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0xE000000) >> 25);
        EIS_LOG_R("rp_numV:(0x%08x,0x%08x)",rp_numV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E00000) >> 21);
        EIS_LOG_R("knee:(0x%08x,0x%08x)",knee_1,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E0000) >> 17);
        EIS_LOG_R("clip:(0x%08x,0x%08x)",knee_2,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1E000) >> 13);
        EIS_LOG_R("rp_numH:(0x%08x,0x%08x)",rp_numH,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x1F00) >> 8);
        EIS_LOG_R("subG_en:(0x%08x,0x%08x)",subG_en,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x40) >> 6);
        EIS_LOG_R("DS_IIR_V:(0x%08x,0x%08x)",_this->mEisDivV,(_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x38) >> 3);
        EIS_LOG_R("DS_IIR_H:(0x%08x,0x%08x)",_this->mEisDivH,_this->mEisRegSetting.reg_eis_prep_me_ctrl1 & 0x7);

        EIS_LOG_R("reg_eis_prep_me_ctrl2(0x%08x)",_this->mEisRegSetting.reg_eis_prep_me_ctrl2);
        EIS_LOG_R("first_frame:0x%08x",(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x8000) >> 15);
        EIS_LOG_R("write_en(rp_modi):0x%08x",(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x4000) >> 14);
        EIS_LOG_R("dc_dl:(0x%08x,0x%08x)",dc_dl,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x3F00) >> 8);
        EIS_LOG_R("GAIN_IIR_V:(0x%08x,0x%08x)",vert_shr,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0xE0) >> 5);
        EIS_LOG_R("GAIN_IIR_H:(0x%08x,0x%08x)",hori_shr,(_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x1C) >> 2);
        EIS_LOG_R("proc_gain:(0x%08x,0x%08x)",proc_gain,_this->mEisRegSetting.reg_eis_prep_me_ctrl2 & 0x3);

        EIS_LOG_R("reg_eis_lmv_th(0x%08x)",_this->mEisRegSetting.reg_eis_lmv_th);
        EIS_LOG_R("reg_eis_fl_offset(0x%08x)",_this->mEisRegSetting.reg_eis_fl_offset);

        EIS_LOG_R("reg_eis_mb_offset(0x%08x)",_this->mEisRegSetting.reg_eis_mb_offset);
        EIS_LOG_R("rp_offsetH:(0x%08x,0x%08x)",rp_offsetH,(_this->mEisRegSetting.reg_eis_mb_offset&0xFFF0000)>>16);
        EIS_LOG_R("rp_offsetV:(0x%08x,0x%08x)",rp_offsetV,_this->mEisRegSetting.reg_eis_mb_offset&0xFFF);

        EIS_LOG_R("reg_eis_mb_interval(0x%08x)",_this->mEisRegSetting.reg_eis_mb_interval);
        EIS_LOG_R("win_sizeH:(0x%08x,0x%08x)",win_sizeH,(_this->mEisRegSetting.reg_eis_mb_interval&0xFFF0000)>>16);
        EIS_LOG_R("win_sizeV:(0x%08x,0x%08x)",win_sizeV,_this->mEisRegSetting.reg_eis_mb_interval&0xFFF);

        EIS_LOG_R("reg_eis_image_ctrl(0x%08x)",_this->mEisRegSetting.reg_eis_image_ctrl);
        EIS_LOG_R("Width:(0x%08x,0x%08x)",_this->mImgWidth,(_this->mEisRegSetting.reg_eis_image_ctrl&0x1FFF0000)>>16);
        EIS_LOG_R("Height:(0x%08x,0x%08x)",_this->mImgHeight,_this->mEisRegSetting.reg_eis_image_ctrl&0x1FFF);
    }
}

