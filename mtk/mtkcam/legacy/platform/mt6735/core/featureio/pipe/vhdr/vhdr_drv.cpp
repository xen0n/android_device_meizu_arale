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
*      TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

/**
* @file vhdr_drv.cpp
*
* VHDR Driver Source File
*
*/
 #define MTK_LOG_ENABLE 1
#include <utils/Errors.h>
#include <cutils/log.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <linux/cache.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>

using namespace std;
using namespace android;

#include "mtkcam/imageio/ispio_stddef.h"   // for  register struct

using namespace NSImageio;
using namespace NSIspio;

#include "mtkcam/drv/isp_reg.h" // for register name
#include "mtkcam/drv/isp_drv.h" // for isp driver object

#include "mtkcam/v1/config/PriorityDefs.h"
#include "mtkcam/iopipe/CamIO/INormalPipe.h"    // for pass1 register setting API
#include "mtkcam/hal/IHalSensor.h"

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSCamIOPipe;

#include "mtkcam/drv/imem_drv.h"
//#include "camera_custom_vhdr.h"    // TODO-not suer and file is not exist

#include "isp_mgr/isp_mgr.h"

using namespace NSIspTuning;

#include <mtkcam/utils/common.h>
#include <mtkcam/hwutils/HwMisc.h>
using namespace NSCam::Utils;

#include "vhdr_drv_imp.h"

/*******************************************************************************
*
********************************************************************************/
#define VHDR_DRV_DEBUG

#ifdef VHDR_DRV_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef LOG_TAG
#define LOG_TAG "VHdrDrv"
#define VHDR_LOG(fmt, arg...)    ALOGD("[%s]" fmt, __func__, ##arg)
#define VHDR_INF(fmt, arg...)    ALOGI("[%s]" fmt, __func__, ##arg)
#define VHDR_WRN(fmt, arg...)    ALOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define VHDR_ERR(fmt, arg...)    ALOGE("[%s] %s ERROR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define VHDR_LOG(a,...)
#define VHDR_INF(a,...)
#define VHDR_WRN(a,...)
#define VHDR_ERR(a,...)
#endif

#define VHDR_DRV_NAME "VHdrDrv"
#define LCSO_BUFFER_NUM 10
#define ROUND_TO_2X(x) ((x) & (~0x1))
#define ALIGN_SIZE(in,align) (in & ~(align-1))

/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;
static MINT32 gLcsoDebugDump = 0;

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrv::CreateInstance(const MUINT32 &aSensorIdx)
{
    return VHdrDrvImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrv *VHdrDrvImp::GetInstance(const MUINT32 &aSensorIdx)
{
    VHDR_LOG("aSensorIdx(%u)",aSensorIdx);
    return new VHdrDrvImp(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::DestroyInstance()
{
    VHDR_LOG("+");
    delete this;
}

/*******************************************************************************
*
********************************************************************************/
VHdrDrvImp::VHdrDrvImp(const MUINT32 &aSensorIdx) : VHdrDrv()
{
    mUsers = 0;

    m_pNormalPipe = NULL;
    m_pISPDrvObj = NULL;
    m_pISPVirtDrv = NULL;  // for command queue
    m_pIMemDrv = NULL;

    mLcsEn = MFALSE;
    mLcsD_En = MFALSE;
    mLcs_outW = 0;
    mLcs_outH = 0;
    mLcsD_outW = 0;
    mLcsD_outH = 0;
    mSensorIdx = aSensorIdx;
    mFlareGain = 0x100;
    mFlareOffset = 0;
    mIsVhdrConfig = 0;

    mLcso_memInfo.memID = -5;
    mLcso_memInfo.useNoncache = 1;
    mLcso_memInfo.virtAddr = mLcso_memInfo.phyAddr = mLcso_memInfo.size = 0;
    mLcsoD_memInfo.memID = -5;
    mLcsoD_memInfo.useNoncache = 1;
    mLcsoD_memInfo.virtAddr = mLcsoD_memInfo.phyAddr = mLcsoD_memInfo.size = 0;
    mLcso_addrIdx = 0;
    mLcso_virAddr = 0;
    mLcso_phyAddr = 0;
    mLcsoD_addrIdx = 0;
    mLcsoD_phyAddr = 0;
    mLcsoD_virAddr = 0;

    mSensorDev = 0;
    mSensorTg  = CAM_TG_NONE;

    mState = VHDR_STATE_NONE;

    mVrWithEis = MFALSE;
    mLcsAws    = 0;
    mLcsD_Aws  = 0;
    mVideoPrvW = 0;
    mVideoPrvH = 0;
    mPass1OutW = 0;
    mPass1OutH = 0;
    mZoomRatio = 100;
    mpVHdrP1Cb = NULL;

    mLcsoThread = NULL;
    ::sem_init(&mLcsoSem, 0, 0);
    ::sem_init(&mLcsoSemEnd, 0, 0);
    ::sem_init(&mTsSem, 0, 0);
    ::sem_init(&mUninitSem, 0, 0);

    while(!mLcsoData.empty())
    {
        mLcsoData.pop();
    }

    while(!mLcsoData4P2.empty())
    {
        mLcsoData4P2.pop();
    }

    while(!mLcsoD_Data.empty())
    {
        mLcsoD_Data.pop();
    }

    while(!mLcsoD_Data4P2.empty())
    {
        mLcsoD_Data4P2.pop();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID *VHdrDrvImp::LcsoThreadLoop(MVOID *arg)
{
    VHdrDrvImp *_this = reinterpret_cast<VHdrDrvImp *>(arg);
    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Change Thread Setting ======

    _this->ChangeThreadSetting("LcsoThreadLoop");

    //====== Main Loop ======

    VHDR_LOG("wait mLcsoSem");
    ::sem_wait(&_this->mLcsoSem); // wait here until someone use sem_post() to wake this semaphore up
    VHDR_LOG("got mLcsoSem");

    VHDR_STATE_ENUM eState = _this->GetVHdrState();

    while(eState != VHDR_STATE_UNINIT)
    {
        eState = _this->GetVHdrState();

        switch(eState)
        {
            case VHDR_STATE_ALIVE:
                    // 1. wait Vsync
                    err = _this->UpdateLcso();
                    if(VHDR_RETURN_NO_ERROR != err)
                    {
                        VHDR_ERR("UpdateLcso fail(%d)",err);
                    }

                    // 2. wait pass1 done
                    err = _this->UpdateLcsoIdx();
                    if(VHDR_RETURN_NO_ERROR != err)
                    {
                        VHDR_ERR("UpdateLcsoIdx fail(%d)",err);
                    }
                break;
            case VHDR_STATE_UNINIT:
                VHDR_LOG("UNINIT");
                break;
            default:
                VHDR_ERR("State Error(%d)",eState);
        }

        eState = _this->GetVHdrState();
    }

    ::sem_post(&_this->mLcsoSemEnd);
    VHDR_LOG("post mLcsoSemEnd");

    ::sem_post(&_this->mUninitSem);
    VHDR_LOG("post mUninitSem");

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Init()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

#if (VHDR_DEBUG_FLAG)

    g_debugDump = 3;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    g_debugDump = atoi(value);

#endif

    char lcsoValue[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.lcso.dump", lcsoValue, "0");
    gLcsoDebugDump = atoi(lcsoValue);

    //====== Set State ======

    SetVHdrState(VHDR_STATE_ALIVE);

    VHDR_LOG("mSensorIdx(%u) init",mSensorIdx);

    //====== Create ISP Driver Object  ======

    m_pISPDrvObj = IspDrv::createInstance();
    if(m_pISPDrvObj == NULL)
    {
        VHDR_ERR("m_pISPDrvObj create instance fail");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }

    if(MFALSE == m_pISPDrvObj->init(VHDR_DRV_NAME))
    {
        VHDR_ERR("m_pISPDrvObj->init() fail");
        err = VHDR_RETURN_API_FAIL;
        return err;
    }

    #if 0   // opend it when needed
    // Command Queue
    m_pISPVirtDrv = m_pISPDrvObj->getCQInstance(ISP_DRV_CQ0);
    if(m_pISPVirtDrv == NULL)
    {
        VHDR_ERR("m_pISPVirtDrv create instance fail");
        err = EIS_RETURN_NULL_OBJ;
        return err;
    }
    #endif

    //====== Create INormalPipe Object ======

    m_pNormalPipe = INormalPipe::createInstance(mSensorIdx, VHDR_DRV_NAME);
    if(m_pNormalPipe == NULL)
    {
        VHDR_ERR("create INormalPipe fail");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }

    //====== Create IMem Object ======

    m_pIMemDrv = IMemDrv::createInstance();
    if(m_pIMemDrv == NULL)
    {
        VHDR_ERR("Null IMemDrv Obj");
        err = VHDR_RETURN_NULL_OBJ;
        return err;
    }

    if(MFALSE == m_pIMemDrv->init())
    {
        VHDR_ERR("m_pIMemDrv->init fail");
        err = VHDR_RETURN_API_FAIL;
        return err;
    }

    //====== Create Pass1 Callback Class ======

    mpVHdrP1Cb = new VHdrP1Cb(this);

    //====== Create Thread ======

    //> init semphore

    ::sem_init(&mLcsoSem, 0, 0);
    ::sem_init(&mLcsoSemEnd, 0, 0);
    ::sem_init(&mTsSem, 0, 0);
    ::sem_init(&mUninitSem, 0, 0);

    //> create thread

    pthread_create(&mLcsoThread, NULL, LcsoThreadLoop, this);

    android_atomic_inc(&mUsers);    // increase reference count
    VHDR_LOG("-");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Reference Count ======

    if(mUsers <= 0) // No more users
    {
        VHDR_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    // >= one user
    android_atomic_dec(&mUsers);

    MINT32 err = VHDR_RETURN_NO_ERROR;

    if(mUsers == 0)
    {
        //====== Set State ======

        SetVHdrState(VHDR_STATE_UNINIT);

        VHDR_LOG("mSensorIdx(%u) uninit",mSensorIdx);

        //====== Destory ISP Driver Object ======

        if(m_pISPVirtDrv != NULL)
        {
            m_pISPVirtDrv = NULL;
        }

        if(m_pISPDrvObj != NULL)
        {
            if(MFALSE == m_pISPDrvObj->uninit(VHDR_DRV_NAME))
            {
                VHDR_ERR("m_pISPDrvObj->uninit fail");
                err = VHDR_RETURN_API_FAIL;
            }

            m_pISPDrvObj->destroyInstance();
            m_pISPDrvObj = NULL;
        }

        //====== Destory INormalPipe ======

        if(m_pNormalPipe != NULL)
        {
            m_pNormalPipe->destroyInstance(VHDR_DRV_NAME);
            m_pNormalPipe = NULL;
        }

        //====== Destory IMem ======

        if(m_pIMemDrv != NULL)
        {
            DestroyMemBuf(1,&mLcso_memInfo);
            DestroyMemBuf(1,&mLcsoD_memInfo);

            mLcso_memInfo.memID = -5;
            mLcso_memInfo.useNoncache = 1;
            mLcso_memInfo.virtAddr = mLcso_memInfo.phyAddr = mLcso_memInfo.size = 0;
            mLcsoD_memInfo.memID = -5;
            mLcsoD_memInfo.useNoncache = 1;
            mLcsoD_memInfo.virtAddr = mLcsoD_memInfo.phyAddr = mLcsoD_memInfo.size = 0;

            mLcso_addrIdx = 0;
            mLcso_virAddr = 0;
            mLcso_phyAddr = 0;
            mLcsoD_addrIdx = 0;
            mLcsoD_virAddr = 0;
            mLcsoD_phyAddr = 0;

            if(MFALSE == m_pIMemDrv->uninit())
            {
                VHDR_ERR("m_pIMemDrv->uninit fail");
                err = VHDR_RETURN_API_FAIL;
            }

            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //====== Rest Member Variable ======

        mUsers = 0;
        mLcsEn = MFALSE;
        mLcsD_En = MFALSE;
        mLcs_outW = 0;
        mLcs_outH = 0;
        mLcsD_outW = 0;
        mLcsD_outH = 0;
        mSensorIdx = 0;
        mFlareGain = 0x100;
        mFlareOffset = 0;
        mIsVhdrConfig = 0;

        mSensorDev = 0;
        mSensorTg  = CAM_TG_NONE;

        mVrWithEis = MFALSE;
        mLcsAws    = 0;
        mLcsD_Aws  = 0;
        mVideoPrvW = 0;
        mVideoPrvH = 0;
        mPass1OutW = 0;
        mPass1OutH = 0;

        // delete pass1 callback class

        delete mpVHdrP1Cb;
        mpVHdrP1Cb = NULL;

        //====== Destroy Thread ======

        //> wait sem
        VHDR_LOG("wait mLcsoSemEnd");
        ::sem_wait(&mLcsoSemEnd);
        VHDR_LOG("got mLcsoSemEnd");

        //> delete thread
        pthread_join(mLcsoThread, NULL);

        //====== Clean Queue ======

        while(!mLcsoData.empty())
        {
            mLcsoData.pop();
        }

        while(!mLcsoData4P2.empty())
        {
            mLcsoData4P2.pop();
        }

        while(!mLcsoD_Data.empty())
        {
            mLcsoD_Data.pop();
        }

        while(!mLcsoD_Data4P2.empty())
        {
            mLcsoD_Data4P2.pop();
        }

        //====== Set State ======

        SetVHdrState(VHDR_STATE_NONE);
    }
    else
    {
         VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    VHDR_LOG("X");
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetFlareInfo(const MINT16 &aFlareGain,const MINT16 &aFlareOffset)
{
    mFlareGain = aFlareGain >> 1;   // AE is 512 based, LCS is 256 based, confirm with CC
    mFlareOffset = aFlareOffset >> 2;
    VHDR_LOG("(gain,ofst)=I(%u,%u),O(%u,%u)",aFlareGain,aFlareOffset,mFlareGain,mFlareOffset);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg)
{
    mSensorDev = aSensorDev;
    mSensorTg  = aSensorTg;
    VHDR_LOG("(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetLcsoThreadState(VHDR_STATE_ENUM aState)
{
    VHDR_LOG("aState(%d)",aState);

    if(aState == VHDR_STATE_ALIVE)
    {
        //====== Clear Pass1 Done Signal First ======

        ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

        waitIrq.Clear      = ISP_DRV_IRQ_CLEAR_STATUS;
        waitIrq.Type       = (mSensorTg == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
        waitIrq.Status     = (mSensorTg == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_PASS1_DON_ST : CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
        waitIrq.Timeout    = 411;
        waitIrq.UserNumber = ISP_DRV_IRQ_USER_VHDR;
        waitIrq.UserName   = const_cast<char *>("VHDR");

        if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
        {
            VHDR_ERR("ISP_DRV_IRQ_CLEAR_STATUS fail");
            VHDR_ERR("TG(%d),Clear(%d),Type(%d),Status(%d),Timeout(%u),UserNumber(%d)",mSensorTg,
                                                                                       waitIrq.Clear,
                                                                                       waitIrq.Type,
                                                                                       waitIrq.Status,
                                                                                       waitIrq.Timeout,
                                                                                       waitIrq.UserNumber);
        }

        //====== Post Sem ======

        ::sem_post(&mLcsoSem);
        VHDR_LOG("post mLcsoSem");

        //====== ConfigVHdr Done ======

        mIsVhdrConfig = 1;
    }
    else if(aState == VHDR_STATE_UNINIT)
    {
        SetVHdrState(VHDR_STATE_UNINIT);

        ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

            waitIrq.Type       = (mSensorTg == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
            waitIrq.Status       = (mSensorTg == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_VS1_INT_ST : CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
            waitIrq.UserNumber = ISP_DRV_IRQ_USER_VHDR;

            if(MTRUE != m_pISPDrvObj->flushIrq(waitIrq))
        {
            VHDR_ERR("Flush irq fail");
        }

            waitIrq.Type       = (mSensorTg == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
            waitIrq.Status       = (mSensorTg == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_PASS1_DON_ST : CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
            waitIrq.UserNumber = ISP_DRV_IRQ_USER_VHDR;

            if(MTRUE != m_pISPDrvObj->flushIrq(waitIrq))
        {
            VHDR_ERR("Flush irq fail");
        }

        VHDR_LOG("wait mUninitSem");
        ::sem_wait(&mUninitSem);    // block MW thread to prevent no VD or pass1 done signal
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetVideoSizeInfo(const VHDR_CMD_ENUM &aCmd,MINT32 arg1,MINT32 arg2,MINT32 arg3)
{
    switch(aCmd)
    {
        case VHDR_CMD_SET_VR_EIS_ON_OFF:
            mVrWithEis = arg1;
            if(g_debugDump > 0)
            {
                VHDR_LOG("mVrWithEis(%d)",mVrWithEis);
            }
            break;
        case VHDR_CMD_SET_VIDEO_SIZE:
            mVideoPrvW = arg1;
            mVideoPrvH = arg2;
            if(g_debugDump > 0)
            {
                VHDR_LOG("VideoPrv(W,H)=(%d,%d)",mVideoPrvW,mVideoPrvH);
            }
            break;
        case VHDR_CMD_SET_ZOOM_RATIO:
            mZoomRatio = (MUINT32)arg1;
            if(g_debugDump > 0)
            {
                VHDR_LOG("zoomRatio(%u)",mZoomRatio);
            }
            break;
        case VHDR_CMD_SET_PASS1_OUT_SIZE:
            mPass1OutW = arg1;
            mPass1OutH = arg2;
            if(g_debugDump > 0)
            {
                VHDR_LOG("Pass1Out(W,H)=(%d,%d)",mPass1OutW,mPass1OutH);
            }
            break;
        default:
            VHDR_ERR("wrong cmd(%d)",aCmd);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::GetLceiDmaInfo(LCEI_DMA_INFO *apLceiDmaInfo,const MINT64 &aTimeStamp)
{
    //====== Start Update LCEI ======

    Mutex::Autolock lock(mQueLock);

    VHDR_LOG("tg(%u),TS(%lld)",mSensorTg,aTimeStamp);

    MUINT32 hasMatch = 0;

    if(mSensorTg == CAM_TG_1)
    {
        if(mLcsoData4P2.empty())
        {
            VHDR_ERR("mLcsoData4P2 empty)");
            return;
        }

        while(!mLcsoData4P2.empty())
        {
            if(mLcsoData4P2.front().timeStamp < aTimeStamp)
            {
                VHDR_LOG("drop,PA(0x%08x),TS(%lld)",mLcsoData4P2.front().pa,mLcsoData4P2.front().timeStamp);
                mLcsoData4P2.pop();
            }
            else if(mLcsoData4P2.front().timeStamp == aTimeStamp)
            {
                hasMatch = 1;
                break;
            }
            else
            {
                hasMatch = 0;
                break;
            }
        }

        if(hasMatch == 1)
        {
            apLceiDmaInfo->xSize       = mLcs_outW - 1;
            apLceiDmaInfo->ySize       = mLcs_outH - 1;
            apLceiDmaInfo->stride      = mLcs_outW;
            apLceiDmaInfo->memID       = mLcso_memInfo.memID;
            apLceiDmaInfo->va          = mLcsoData4P2.front().va;
            apLceiDmaInfo->pa          = mLcsoData4P2.front().pa;
            apLceiDmaInfo->size        = mLcs_outW * mLcs_outH;
            apLceiDmaInfo->bufSecu     = mLcso_memInfo.bufSecu;
            apLceiDmaInfo->bufCohe     = mLcso_memInfo.bufCohe;
            apLceiDmaInfo->useNoncache = mLcso_memInfo.useNoncache;

            if(g_debugDump > 0)
            {
                VHDR_LOG("match,PA(0x%08x),TS(%lld)",mLcsoData4P2.front().pa,mLcsoData4P2.front().timeStamp);
            }
            mLcsoData4P2.pop();
        }
        else
        {
            apLceiDmaInfo->memID = VHDR_LCSO_SYNC_FAIL;
            VHDR_ERR("no match");
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        if(mLcsoD_Data4P2.empty())
        {
            VHDR_ERR("mLcsoD_Data4P2 empty)");
            return;
        }

        while(!mLcsoD_Data4P2.empty())
        {
            if(mLcsoD_Data4P2.front().timeStamp < aTimeStamp)
            {
                VHDR_LOG("dropD,PA(0x%08x),TS(%lld)",mLcsoD_Data4P2.front().pa,mLcsoD_Data4P2.front().timeStamp);
                mLcsoD_Data4P2.pop();
            }
            else if(mLcsoD_Data4P2.front().timeStamp == aTimeStamp)
            {
                hasMatch = 1;
                break;
            }
            else
            {
                hasMatch = 0;
                break;
            }
        }

        if(hasMatch == 1)
        {
            apLceiDmaInfo->xSize       = mLcsD_outW - 1;
            apLceiDmaInfo->ySize       = mLcsD_outH - 1;
            apLceiDmaInfo->stride      = mLcsD_outW;
            apLceiDmaInfo->memID       = mLcsoD_memInfo.memID;
            apLceiDmaInfo->va          = mLcsoD_Data4P2.front().va;
            apLceiDmaInfo->pa          = mLcsoD_Data4P2.front().pa;
            apLceiDmaInfo->size        = mLcsD_outW * mLcsD_outH;
            apLceiDmaInfo->bufSecu     = mLcsoD_memInfo.bufSecu;
            apLceiDmaInfo->bufCohe     = mLcsoD_memInfo.bufCohe;
            apLceiDmaInfo->useNoncache = mLcsoD_memInfo.useNoncache;

            if(g_debugDump > 0)
            {
                VHDR_LOG("matchD,PA(0x%08x),TS(%lld)",mLcsoD_Data4P2.front().pa,mLcsoD_Data4P2.front().timeStamp);
            }
            mLcsoD_Data4P2.pop();
        }
        else
        {
            apLceiDmaInfo->memID = VHDR_LCSO_SYNC_FAIL;
            VHDR_ERR("no match");
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%u)",mSensorTg);
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::ConfigLcs()
{
    VHDR_LOG("(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);

    MINTPTR handle = -1;

    if(mSensorTg == CAM_TG_1)
    {
        //====== Prepare Register Setting ======

        // CAM_LCS_CON

        MUINT32 lcsCon = 0;

        mLcs_outW = 0x40;   // 64
        mLcs_outH = 0x30;   // 48

        lcsCon = lcsCon | (mLcs_outH << 24) | (mLcs_outW << 16) | 0x01;
        VHDR_LOG("lcsCon(0x%08x)",lcsCon);

        //CAM_LCS_AWS

        MUINT32 lcsInWidth = 0;
        MUINT32 lcsInHeight = 0;
        MUINT32 lcsAws = 0;
        NormalPipe_HBIN_Info hbinInfo;

        //> query 2-pixel mode
        // (1) not 2-pixel mode : TG width, TG height
        // (2) 2-pixel mode : TG width/2, TG height

        VHDR_LOG("mSensorIdx(%u)",mSensorIdx);
        m_pNormalPipe->sendCommand(EPIPECmd_GET_TG_OUT_SIZE, mSensorIdx, (MINTPTR)(&lcsInWidth),(MINTPTR)(&lcsInHeight));
        m_pNormalPipe->sendCommand(EPIPECmd_GET_HBIN_INFO, mSensorIdx, (MINTPTR)(&hbinInfo), -1);

        if(hbinInfo.mEnabled == MTRUE && hbinInfo.size.w == (lcsInWidth/2))
        {
            VHDR_LOG("2-pixel mode");
            lcsInWidth /= 2;
        }

        lcsAws  = lcsAws | (lcsInHeight << 16) | lcsInWidth;
        mLcsAws = lcsAws;

        VHDR_LOG("lcsInWidth(0x%08x),lcsInHeight(0x%08x)",lcsInWidth,lcsInHeight);
        VHDR_LOG("lcsAws(0x%08x)",lcsAws);

        //CAM_LCS_FLR

        MUINT32 lcsFlrGain = mFlareGain;        // default value = 0x100 (not use)
        MUINT32 lcsFlrOffset = mFlareOffset;    // default value = 0 (not use)
        MUINT32 lcsFlr = 0;

        lcsFlr = lcsFlr | (lcsFlrGain << 7) | lcsFlrOffset;
        VHDR_LOG("lcsFlr(0x%08x)",lcsFlr);

        // CAM_LCS_LRZR_1 & CAM_LCS_LRZR_2

        MUINT32 lcsLrzrX = ((2 * mLcs_outW - 2) << 20) / (lcsInWidth - 1);
        MUINT32 lcsLrzrY = ((mLcs_outH - 1) << 20) / (lcsInHeight - 1);

        VHDR_LOG("lcsLrzrX(0x%08x),lcsLrzrY(0x%08x)",lcsLrzrX,lcsLrzrY);

        //====== Set to LCE ======

        ISP_MGR_LCE_T::getInstance(tuningSensorDevType,tuningSensorTgType).setLcsoSize(mLcs_outW,mLcs_outH);

        //====== Get Handle ======

#if 0   // use callback to config

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCS,(MINT32)(&handle),(MINT32)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCS handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe ======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_CON,lcsCon);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_ST,0);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_AWS,lcsAws);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_FLR,lcsFlr);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_LRZR_1,lcsLrzrX);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_LRZR_2,lcsLrzrY);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCS cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCS handle fail");
            return VHDR_RETURN_API_FAIL;
        }
#endif
    }
    else if(mSensorTg == CAM_TG_2)   //RAW_D
    {
        //====== Prepare Register Setting ======

        // CAM_LCS_D_CON

        MUINT32 lcsD_Con = 0;

        mLcsD_outW = 0x40;   // 64
        mLcsD_outH = 0x30;   // 48

        lcsD_Con = lcsD_Con | (mLcsD_outH << 24) | (mLcsD_outW << 16) | 0x01;
        VHDR_LOG("lcsD_Con(0x%08x)",lcsD_Con);

        //CAM_LCS_AWS

        MUINT32 lcsD_InWidth = 0;
        MUINT32 lcsD_InHeight = 0;
        MUINT32 lcsD_Aws = 0;
        NormalPipe_HBIN_Info hbinD_Info;

        //> query 2-pixel mode
        // (1) not 2-pixel mode : TG width, TG height
        // (2) 2-pixel mode : TG width/2, TG height

        VHDR_INF("mSensorIdx(%u)",mSensorIdx);
        m_pNormalPipe->sendCommand(EPIPECmd_GET_TG_OUT_SIZE, mSensorIdx, (MINTPTR)(&lcsD_InWidth),(MINTPTR)(&lcsD_InHeight));
        m_pNormalPipe->sendCommand(EPIPECmd_GET_HBIN_INFO, mSensorIdx, (MINTPTR)(&hbinD_Info), -1);

        if(hbinD_Info.mEnabled == MTRUE && hbinD_Info.size.w == (lcsD_InWidth/2))
        {
            VHDR_LOG("2-pixel mode");
            lcsD_InWidth /= 2;
        }

        lcsD_Aws  = lcsD_Aws | (lcsD_InHeight << 16) | lcsD_InWidth;
        mLcsD_Aws = lcsD_Aws;

        VHDR_LOG("lcsD_InWidth(0x%08x),lcsD_InHeight(0x%08x)",lcsD_InWidth,lcsD_InHeight);
        VHDR_LOG("lcsD_Aws(0x%08x)",lcsD_Aws);

        //CAM_LCS_D_FLR

        MUINT32 lcsD_FlrGain = mFlareGain;      // default value = 0x100 (not use)
        MUINT32 lcsD_FlrOffset = mFlareOffset;  // default value = 0 (not use)
        MUINT32 lcsD_Flr = 0;

        lcsD_Flr = lcsD_Flr | (lcsD_FlrGain << 7) | lcsD_FlrOffset;
        VHDR_LOG("lcsD_Flr(0x%08x)",lcsD_Flr);

        // CAM_LCS_D_LRZR_1 & CAM_LCS_D_LRZR_2

        MUINT32 lcsD_LrzrX = ((2 * mLcsD_outW - 2) << 20) / (lcsD_InWidth - 1);
        MUINT32 lcsD_LrzrY = ((mLcsD_outH - 1) << 20) / (lcsD_InHeight - 1);

        VHDR_LOG("lcsD_LrzrX(0x%08x),lcsD_LrzrY(0x%08x)",lcsD_LrzrX,lcsD_LrzrY);

        //====== Set to LCE ======

        ISP_MGR_LCE_T::getInstance(tuningSensorDevType,tuningSensorTgType).setLcsoSize(mLcsD_outW,mLcsD_outH);

        //====== Get Handle ======

#if 0   // use callback to config

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCS_D,(MINT32)(&handle),(MINT32)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCS_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe ======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_CON,lcsD_Con);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_ST,0);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_AWS,lcsD_Aws);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_FLR,lcsD_Flr);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_LRZR_1,lcsD_LrzrX);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_LRZR_2,lcsD_LrzrY);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCS_D cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCS_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }
#endif
    }
    else
    {
        VHDR_ERR("wrong TG(%u)",mSensorTg);
    }

    VHDR_LOG("-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::ConfigLcso()
{
    VHDR_LOG("(dev,tg)=(%u,%u)",mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);
    MINTPTR handle = -1;

    if(mSensorTg == CAM_TG_1)   //RAW
    {
        //====== Create Memory for LCSO and LCEI ======

        MUINT32 lcs_memSize = mLcs_outW * mLcs_outH * LCSO_BUFFER_NUM;

        VHDR_LOG("mLcs_outW(%u),mLcs_outH(%u),lcs_memSize(%u)",mLcs_outW,mLcs_outH,lcs_memSize);

        CreateMemBuf(lcs_memSize,1,&mLcso_memInfo);
        if(mLcso_memInfo.virtAddr == 0 && mLcso_memInfo.phyAddr == 0)
        {
            VHDR_ERR("lcs create IMem fail");
            return VHDR_RETURN_API_FAIL;
        }

        mLcso_addrIdx = 0;
        mLcso_virAddr = mLcso_memInfo.virtAddr;
        mLcso_phyAddr = mLcso_memInfo.phyAddr;

        VHDR_LOG("LcsoMem:ID(%d),SZ(%u),VA(0x%08x),PA(0x%08x),cache(%d)",mLcso_memInfo.memID,
                                                                         mLcso_memInfo.size,
                                                                         mLcso_memInfo.virtAddr,
                                                                         mLcso_memInfo.phyAddr,
                                                                         mLcso_memInfo.useNoncache);

        //====== Set to LCE ======

        ISP_MGR_LCE_T::getInstance(tuningSensorDevType,tuningSensorTgType).setLceiAddr(mLcso_phyAddr);  // only for initial value

        //====== Prepare Register Setting ======

        MUINT32 lcso_xSize = mLcs_outW - 1;
        MUINT32 lcso_ySize = mLcs_outH - 1;
        MUINT32 lcso_con  = 0x80202020;  //default value = 0x80202020
        MUINT32 lcso_con2 = 0x202000;    //default value = 0x202000

        VHDR_LOG("lcso_xSize(0x%08x),lcso_ySize(0x%08x)",lcso_xSize,lcso_ySize);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCSO,(MINTPTR)(&handle),(MINTPTR)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCSO handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe =======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_BASE_ADDR,mLcso_phyAddr);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_OFST_ADDR,0);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_XSIZE,lcso_xSize);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_YSIZE,lcso_ySize);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_STRIDE,mLcs_outW);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_CON,lcso_con);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_CON2,lcso_con2);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCSO cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCSO handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        //====== Create Memory for LCSO and LCEI ======

        MUINT32 lcsD_memSize = mLcsD_outW * mLcsD_outH * LCSO_BUFFER_NUM;

        VHDR_LOG("mLcsD_outW(%u),mLcsD_outH(%u),lcsD_memSize(%u)",mLcsD_outW,mLcsD_outH,lcsD_memSize);

        CreateMemBuf(lcsD_memSize,1,&mLcsoD_memInfo);
        if(mLcsoD_memInfo.virtAddr == 0 && mLcsoD_memInfo.phyAddr == 0)
        {
            VHDR_ERR("lcsD create IMem fail");
            return VHDR_RETURN_API_FAIL;
        }

        mLcsoD_addrIdx = 0;
        mLcsoD_virAddr = mLcsoD_memInfo.virtAddr;
        mLcsoD_phyAddr = mLcsoD_memInfo.phyAddr;

        VHDR_LOG("LcsoD_Mem:ID(%d),SZ(%u),VA(0x%08x),PA(0x%08x),cache(%d)",mLcsoD_memInfo.memID,
                                                                           mLcsoD_memInfo.size,
                                                                           mLcsoD_memInfo.virtAddr,
                                                                           mLcsoD_memInfo.phyAddr,
                                                                           mLcsoD_memInfo.useNoncache);

        //====== Set to LCE ======

        ISP_MGR_LCE_T::getInstance(tuningSensorDevType,tuningSensorTgType).setLceiAddr(mLcsoD_phyAddr); // only for initial value

        //====== Prepare Register Setting ======

        MUINT32 lcsoD_xSize = mLcsD_outW - 1;
        MUINT32 lcsoD_ySize = mLcsD_outH - 1;
        MUINT32 lcsoD_con  = 0x80202020;  //default value = 0x80202020
        MUINT32 lcsoD_con2 = 0x202000;    //default value = 0x202000

        VHDR_LOG("lcsoD_xSize(0x%08x),lcsoD_ySize(0x%08x)",lcsoD_xSize,lcsoD_ySize);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCSO_D,(MINTPTR)(&handle),(MINTPTR)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCSO_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe =======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_BASE_ADDR,mLcsoD_phyAddr);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_OFST_ADDR,0);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_XSIZE,lcsoD_xSize);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_YSIZE,lcsoD_ySize);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_STRIDE,mLcsD_outW);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_CON,lcsoD_con);
        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_CON2,lcsoD_con2);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCSO_D cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCSO_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%u)",mSensorTg);
    }

    VHDR_LOG("-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ConfigRmg(const MUINT32 &aLeFirst)
{
    // 0 : first two rows are SE,  1 : first two rows are LE
    VHDR_LOG("(leFirst,dev,tg)=(%u,%u,%u)",aLeFirst,mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);

    ISP_MGR_RMG_T::getInstance(tuningSensorDevType,tuningSensorTgType).setRmgLeFirst(aLeFirst);

    // because this command need to be after MW configPipe, and don't want to add on more API
    // so just put it here
    m_pNormalPipe->sendCommand(EPIPECmd_SET_LCS_CBFP,(MINTPTR)mpVHdrP1Cb,-1,-1);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::UpdateLcs()
{

#if 0   // use callback to config

    VHDR_LOG("tg(%u)",mSensorTg);

    MINT32 handle = -1;

    if(mSensorTg == CAM_TG_1)
    {
        //====== Update Flare ======

        // CAM_LCS_FLR

        MUINT32 lcsFlrGain = mFlareGain;
        MUINT32 lcsFlrOffset = mFlareOffset;
        MUINT32 lcsFlr = 0;

        lcsFlr = lcsFlr | (lcsFlrGain << 7) | lcsFlrOffset;
        VHDR_LOG("lcsFlr(0x%08x)",lcsFlr);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCS,(MINT32)(&handle),(MINT32)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCS handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe ======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_FLR,lcsFlr);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCS flare cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCS handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        //====== Update Flare ======

        // CAM_LCS_D_FLR

        MUINT32 lcsD_FlrGain = mFlareGain;      // default value = 0x100 (not use)
        MUINT32 lcsD_FlrOffset = mFlareOffset;  // default value = 0 (not use)
        MUINT32 lcsD_Flr = 0;

        lcsD_Flr = lcsD_Flr | (lcsD_FlrGain << 7) | lcsD_FlrOffset;
        VHDR_LOG("lcsD_Flr(0x%08x)",lcsD_Flr);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCS_D,(MINT32)(&handle),(MINT32)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCS_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe ======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCS_D_FLR,lcsD_Flr);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCS_D flare cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINT32)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCS_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%u)",mSensorTg);
    }
#endif
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::UpdateLcso()
{
    if(VHDR_STATE_UNINIT == GetVHdrState())
    {
        ::sem_post(&mUninitSem);
        VHDR_LOG("post mUninitSem");
        return VHDR_RETURN_NO_ERROR;
    }

    MINTPTR handle = -1;

    //====== Wait Vsync ======

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    waitIrq.Clear      = ISP_DRV_IRQ_CLEAR_NONE;
    waitIrq.Type       = (mSensorTg == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status     = (mSensorTg == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_VS1_INT_ST : CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
    waitIrq.Timeout    = 513;
    waitIrq.UserNumber = ISP_DRV_IRQ_USER_VHDR;
    waitIrq.UserName   = const_cast<char *>("VHDR");

    if(g_debugDump > 0)
    {
        VHDR_LOG("wait Vsync");
    }

    if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
    {
        if(VHDR_STATE_UNINIT != GetVHdrState())
        {
            VHDR_ERR("wait Vsync fail,state(%d)",mState);
            VHDR_ERR("TG(%d),Clear(%d),Type(%d),Status(%d),Timeout(%u),UserNumber(%d)",mSensorTg,
                                                                                       waitIrq.Clear,
                                                                                       waitIrq.Type,
                                                                                       waitIrq.Status,
                                                                                       waitIrq.Timeout,
                                                                                       waitIrq.UserNumber);

            return VHDR_RETURN_API_FAIL;
        }
        else
        {
            VHDR_LOG("state(%d)",GetVHdrState());
            return VHDR_RETURN_NO_ERROR;
        }
    }

    Mutex::Autolock lock(mIdxLock);

    if(mSensorTg == CAM_TG_1)
    {
        VHDR_LOG("Lcso:idx(%u),PA(0x%08x)",mLcso_addrIdx,mLcso_phyAddr);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCSO,(MINTPTR)(&handle),(MINTPTR)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCSO handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe =======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_BASE_ADDR,mLcso_phyAddr);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCSO cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCSO handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        VHDR_LOG("LcsoD:idx(%u),PA(0x%08x)",mLcsoD_addrIdx,mLcsoD_phyAddr);

        //====== Get Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_GET_MODULE_HANDLE, EModule_LCSO_D,(MINTPTR)(&handle),(MINTPTR)(&(VHDR_DRV_NAME))))
        {
            VHDR_ERR("get LCSO_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }

        //====== Write to NormalPipe =======

        IOPIPE_SET_MODUL_REG(handle,CAM_LCSO_D_BASE_ADDR,mLcsoD_phyAddr);

        //====== Configure Done ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_CFG_DONE, handle,-1,-1))
        {
            VHDR_ERR("LCSO_D cofig done fail");
            m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Release Handle ======

        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_RELEASE_MODULE_HANDLE,handle,(MINTPTR)(&(VHDR_DRV_NAME)),-1))
        {
            VHDR_ERR("release LCSO_D handle fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%d)",mSensorTg);
        return VHDR_RETURN_INVALID_PARA;
    }

    if(VHDR_STATE_UNINIT == GetVHdrState())
    {
        ::sem_post(&mUninitSem);
        VHDR_LOG("post mUninitSem");
    }

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::UpdateLcsoIdx()
{
    static MINT32 isFirst = 1;

    if(VHDR_STATE_UNINIT == GetVHdrState())
    {
        ::sem_post(&mUninitSem);
        VHDR_LOG("post mUninitSem");
        return VHDR_RETURN_NO_ERROR;
    }

    //====== Wait Pass1 Done ======

    ISP_DRV_WAIT_IRQ_STRUCT waitIrq;

    waitIrq.Clear      = ISP_DRV_IRQ_CLEAR_NONE;
    waitIrq.Type       = (mSensorTg == CAM_TG_1) ? ISP_DRV_IRQ_TYPE_INT_P1_ST : ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
    waitIrq.Status     = (mSensorTg == CAM_TG_1) ? CAM_CTL_INT_P1_STATUS_PASS1_DON_ST : CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
    waitIrq.Timeout    = 511;
    waitIrq.UserNumber = ISP_DRV_IRQ_USER_VHDR;
    waitIrq.UserName   = const_cast<char *>("VHDR");

    if(g_debugDump > 0)
    {
        VHDR_LOG("wait P1 done");
    }

    if(MTRUE != m_pISPDrvObj->waitIrq(&waitIrq))
    {
        if(VHDR_STATE_UNINIT != GetVHdrState())
        {
            VHDR_ERR("wait P1 done fail,state(%d)",GetVHdrState());
            VHDR_ERR("TG(%d),Clear(%d),Type(%d),Status(%d),Timeout(%u),UserNumber(%d)",mSensorTg,
                                                                                       waitIrq.Clear,
                                                                                       waitIrq.Type,
                                                                                       waitIrq.Status,
                                                                                       waitIrq.Timeout,
                                                                                       waitIrq.UserNumber);

            VHDR_LOG("[miss]post mTsSem");
        }
        else
        {
            VHDR_LOG("[miss]post mTsSem,state(%d)",GetVHdrState());
        }


        if(mSensorTg == CAM_TG_1)
        {
            LCSO_DATA lcsoData;

            {
                Mutex::Autolock lock(mQueLock);

                if(isFirst == 1)
                {
                    lcsoData.va = mLcso_virAddr;
                    lcsoData.pa = mLcso_phyAddr;
                }
                else
                {
                    MUINT32 tempLcsoIdx = (mLcso_addrIdx == 0) ? LCSO_BUFFER_NUM-1 : mLcso_addrIdx-1;

                    lcsoData.va = mLcso_memInfo.virtAddr + (mLcs_outW * mLcs_outH * tempLcsoIdx);
                    lcsoData.pa = mLcso_memInfo.phyAddr  + (mLcs_outW * mLcs_outH * tempLcsoIdx);
                }

                lcsoData.timeStamp = -1;

                mLcsoData.push(lcsoData);
            }
        }
        else if(mSensorTg == CAM_TG_2)
        {
            LCSO_DATA lcsoD_Data;

            {
                Mutex::Autolock lock(mQueLock);

                if(isFirst == 1)
                {
                    lcsoD_Data.va = mLcsoD_virAddr;
                    lcsoD_Data.pa = mLcsoD_phyAddr;
                }
                else
                {
                    MUINT32 tempLcsoDIdx = (mLcsoD_addrIdx == 0) ? LCSO_BUFFER_NUM-1 : mLcsoD_addrIdx-1;

                    lcsoD_Data.va = mLcsoD_memInfo.virtAddr + (mLcsD_outW * mLcsD_outH * tempLcsoDIdx);
                    lcsoD_Data.pa = mLcsoD_memInfo.phyAddr  + (mLcsD_outW * mLcsD_outH * tempLcsoDIdx);
                }

                lcsoD_Data.timeStamp = -1;

                mLcsoD_Data.push(lcsoD_Data);
            }
        }

        ::sem_post(&mTsSem);
        return VHDR_RETURN_NO_ERROR;
    }

    Mutex::Autolock lock(mIdxLock);

    if(mSensorTg == CAM_TG_1)
    {
        //====== DUMP ======

        if(gLcsoDebugDump == 1 || gLcsoDebugDump == 3)
        {
            DumpLcso("Lcso",mLcso_virAddr);
        }

        LCSO_DATA lcsoData;

        //====== Save Ready LCSO PA ======

        {
            Mutex::Autolock lock(mQueLock);

            if(isFirst == 1)
            {
                lcsoData.va = mLcso_virAddr;
                lcsoData.pa = mLcso_phyAddr;
                isFirst = 0;
            }
            else
            {
                MUINT32 tempLcsoIdx = (mLcso_addrIdx == 0) ? LCSO_BUFFER_NUM-1 : mLcso_addrIdx-1;

                lcsoData.va = mLcso_memInfo.virtAddr + (mLcs_outW * mLcs_outH * tempLcsoIdx);
                lcsoData.pa = mLcso_memInfo.phyAddr  + (mLcs_outW * mLcs_outH * tempLcsoIdx);
            }

            lcsoData.timeStamp = -1;

            mLcsoData.push(lcsoData);

            //====== Update LCSO Phy Addr ======

            mLcso_addrIdx++;

            if(mLcso_addrIdx == LCSO_BUFFER_NUM)
            {
                mLcso_addrIdx = 0;
            }

            mLcso_virAddr = mLcso_memInfo.virtAddr + (mLcs_outW * mLcs_outH * mLcso_addrIdx);
            mLcso_phyAddr = mLcso_memInfo.phyAddr  + (mLcs_outW * mLcs_outH * mLcso_addrIdx);

            VHDR_LOG("Lcso:idx(%u->%u),savePA(0x%08x),NEXT(va,pa)=(0x%08x,0x%08x)",(mLcso_addrIdx == 0)?LCSO_BUFFER_NUM-1:mLcso_addrIdx-1,
                                                                                    mLcso_addrIdx,
                                                                                    mLcsoData.back().pa,
                                                                                    mLcso_virAddr,
                                                                                    mLcso_phyAddr);
        }

    }
    else if(mSensorTg == CAM_TG_2)
    {
        //====== DUMP ======

        if(gLcsoDebugDump == 1 || gLcsoDebugDump == 3)
        {
            DumpLcso("LcsoD",mLcsoD_virAddr);
        }

        LCSO_DATA lcsoD_Data;

        //====== Save Ready LCSOD PA ======

        {
            Mutex::Autolock lock(mQueLock);

            if(isFirst == 1)
            {
                lcsoD_Data.va = mLcsoD_virAddr;
                lcsoD_Data.pa = mLcsoD_phyAddr;
                isFirst = 0;
            }
            else
            {
                MUINT32 tempLcsoDIdx = (mLcsoD_addrIdx == 0) ? LCSO_BUFFER_NUM-1 : mLcsoD_addrIdx-1;

                lcsoD_Data.va = mLcsoD_memInfo.virtAddr + (mLcsD_outW * mLcsD_outH * tempLcsoDIdx);
                lcsoD_Data.pa = mLcsoD_memInfo.phyAddr  + (mLcsD_outW * mLcsD_outH * tempLcsoDIdx);
    }

            lcsoD_Data.timeStamp = -1;

            mLcsoD_Data.push(lcsoD_Data);

            //====== Update LCSOD Phy Addr ======

            mLcsoD_addrIdx++;

            if(mLcsoD_addrIdx == LCSO_BUFFER_NUM)
    {
                mLcsoD_addrIdx = 0;
    }

            mLcsoD_virAddr = mLcsoD_memInfo.virtAddr + (mLcsD_outW * mLcsD_outH * mLcsoD_addrIdx);
            mLcsoD_phyAddr = mLcsoD_memInfo.phyAddr  + (mLcsD_outW * mLcsD_outH * mLcsoD_addrIdx);

            VHDR_LOG("LcsoD:idx(%u->%u),savePA(0x%08x),NEXT(va,pa)=(0x%08x,0x%08x)",(mLcsoD_addrIdx == 0)?LCSO_BUFFER_NUM-1:mLcsoD_addrIdx-1,
                                                                                    mLcsoD_addrIdx,
                                                                                    mLcsoD_Data.back().pa,
                                                                                    mLcsoD_virAddr,
                                                                                    mLcsoD_phyAddr);
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%d)",mSensorTg);
        return VHDR_RETURN_INVALID_PARA;
    }

    VHDR_LOG("post mTsSem");
    ::sem_post(&mTsSem);

    if(VHDR_STATE_UNINIT == GetVHdrState())
    {
        ::sem_post(&mUninitSem);
        VHDR_LOG("post mUninitSem");
    }

    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::UpdateLcsoTimeStamp(const MINT64 &aTimeStamp)
{
    //====== Wait Semaphore ======

    VHDR_LOG("wait mTsSem");
    ::sem_wait(&mTsSem);
    if(g_debugDump > 0)
    {
        VHDR_LOG("got mTsSem");
    }

    //====== Update Time Stamp ======

    Mutex::Autolock lock(mQueLock);

    VHDR_LOG("TG(%u),TS(%lld)",mSensorTg,aTimeStamp);

    if(mSensorTg == CAM_TG_1)
    {
        if(mLcsoData.empty())
    {
            VHDR_ERR("mLcsoData is empty");
        }

        if(aTimeStamp == 0)
        {
            if(g_debugDump > 0)
            {
                VHDR_LOG("dropPA(0x%08x)",mLcsoData.front().pa);
            }
            mLcsoData.pop();
        }
        else
        {
            LCSO_DATA lcsoData4P2;

            lcsoData4P2.va = mLcsoData.front().va;
            lcsoData4P2.pa = mLcsoData.front().pa;
            lcsoData4P2.timeStamp = aTimeStamp;

            mLcsoData4P2.push(lcsoData4P2);
            mLcsoData.pop();

            VHDR_LOG("enqueP2,PA(0x%08x),TS(%lld)",mLcsoData4P2.back().pa,mLcsoData4P2.back().timeStamp);
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        if(mLcsoD_Data.empty())
        {
            VHDR_ERR("mLcsoD_Data is empty");
        }

        if(aTimeStamp == 0)
        {
            if(g_debugDump > 0)
            {
                VHDR_LOG("dropD_PA(0x%08x)",mLcsoD_Data.front().pa);
            }
            mLcsoD_Data.pop();
            }
        else
        {
            LCSO_DATA lcsoD_Data4P2;

            lcsoD_Data4P2.va = mLcsoD_Data.front().va;
            lcsoD_Data4P2.pa = mLcsoD_Data.front().pa;
            lcsoD_Data4P2.timeStamp = aTimeStamp;

            mLcsoD_Data4P2.push(lcsoD_Data4P2);
            mLcsoD_Data.pop();

            VHDR_LOG("enqueD_P2,PA(0x%08x),TS(%lld)",mLcsoD_Data4P2.back().pa,mLcsoD_Data4P2.back().timeStamp);
        }
    }
    else
        {
        VHDR_ERR("wrong TG(%d)",mSensorTg);
    }
        }

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::EnableLcs(const MBOOL &aEn)
{
    VHDR_LOG("(en,TG)=(%d,%u)",aEn,mSensorTg);

    if(mSensorTg == CAM_TG_1)
        {
        // LCS_EN
    #if 0   // use callback to enable
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_LCS,aEn, -1))
            {
            VHDR_ERR("LCS_EN fail");
            return VHDR_RETURN_API_FAIL;
            }
    #else
        mLcsEn = aEn;
    #endif

        // LCSO_EN
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_LCSO,aEn, -1))
        {
            VHDR_ERR("LCSO_EN fail");
            return VHDR_RETURN_API_FAIL;
        }

        // LCS_SEL_EN
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_LCS_EN,aEn,-1))
        {
            VHDR_ERR("LCS_SEL_EN fail");
            return VHDR_RETURN_API_FAIL;
        }

        // LCS_SEL
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_LCS,aEn,-1))
        {
            VHDR_ERR("LCS_SEL fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else if(mSensorTg == CAM_TG_2)
    {
        // LCS_EN_D
    #if 0   // use callback to enable
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_LCS_D,aEn, -1))
        {
            VHDR_ERR("LCS_EN_D fail");
            return VHDR_RETURN_API_FAIL;
        }
    #else
        mLcsD_En = aEn;
    #endif

        // LCSO_EN_D
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_EN, EModule_LCSO_D,aEn, -1))
        {
            VHDR_ERR("LCSO_EN_D fail");
            return VHDR_RETURN_API_FAIL;
        }

        // LCS_SEL_EN_D
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_LCS_EN_D,aEn,-1))
        {
            VHDR_ERR("LCS_SEL_EN fail");
            return VHDR_RETURN_API_FAIL;
        }

        // LCS_SEL_D
        if(MFALSE == m_pNormalPipe->sendCommand(EPIPECmd_SET_MODULE_SEL, EPipe_P1Sel_LCS_D,aEn,-1))
        {
            VHDR_ERR("LCS_SEL fail");
            return VHDR_RETURN_API_FAIL;
        }
    }
    else
    {
        VHDR_ERR("wrong TG(%u)",mSensorTg);
        return VHDR_RETURN_INVALID_PARA;
    }

    return VHDR_RETURN_NO_ERROR;
    }


/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::EnableLce(const MBOOL &aEn)
{
    VHDR_LOG("(en,dev,tg)=(%d,%u,%u)",aEn,mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);

    ISP_MGR_LCE_T::getInstance(tuningSensorDevType,tuningSensorTgType).setLceEnable(aEn);
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::EnableRmg(const MBOOL &aEn)
{
    VHDR_LOG("(en,dev,tg)=(%d,%u,%u)",aEn,mSensorDev,mSensorTg);

    ESensorDev_T tuningSensorDevType = ConvertSensorDevType(mSensorDev);
    ESensorTG_T tuningSensorTgType   = ConvertSensorTgType(mSensorTg);

    ISP_MGR_RMG_T::getInstance(tuningSensorDevType,tuningSensorTgType).setRmgEnable(aEn);
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrDrvImp::CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = VHDR_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    VHDR_LOG("Cnt(%u),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    bufInfo->size = alingSize;

    if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
    {
        VHDR_ERR("m_pIMemDrv->allocVirtBuf() error");
        err = VHDR_RETURN_API_FAIL;
    }

    if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
    {
        VHDR_ERR("m_pIMemDrv->mapPhyAddr() error");
        err = VHDR_RETURN_API_FAIL;
    }

    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 VHdrDrvImp::DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    VHDR_LOG("Cnt(%u)", bufCnt);

    MINT32 err = VHDR_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                VHDR_LOG("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                VHDR_ERR("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = VHDR_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            VHDR_LOG("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->unmapPhyAddr() error");
            err = VHDR_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            VHDR_ERR("m_pIMemDrv->freeVirtBuf() error");
            err = VHDR_RETURN_API_FAIL;
        }
    }

    return err;
}

/******************************************************************************
*
*******************************************************************************/
ESensorDev_T VHdrDrvImp::ConvertSensorDevType(const MUINT32 &aSensorDev)
{
    switch(aSensorDev)
    {
        case SENSOR_DEV_MAIN: return ESensorDev_Main;
        case SENSOR_DEV_SUB: return ESensorDev_Sub;
        case SENSOR_DEV_MAIN_2: return ESensorDev_MainSecond;
        case SENSOR_DEV_MAIN_3D: return ESensorDev_Main3D;
        default : VHDR_ERR("wrong sensorDev(%d), return ESensorDev_Main",aSensorDev);
                  return ESensorDev_Main;
    }
}

/******************************************************************************
*
*******************************************************************************/
ESensorTG_T VHdrDrvImp::ConvertSensorTgType(const MUINT32 &aSensorTg)
{
    switch(aSensorTg)
    {
        case CAM_TG_1: return ESensorTG_1;
        case CAM_TG_2: return ESensorTG_2;
        default : VHDR_ERR("wrong sensorTg(%d), return ESensorTG_1",aSensorTg);
                  return ESensorTG_1;
    }
}

/******************************************************************************
*
*******************************************************************************/
MVOID VHdrDrvImp::DumpLcso(const char *usr,const MUINT32 &addr)
{
#define LCSO_DUMP_PREFIX "/sdcard/lcsodump/"

    static MUINT32 cnt = 0;

    if(cnt == 4294967295)
    {
        cnt = 1;
    }
    else
    {
        ++cnt;
    }

    char filename[256];
    sprintf(filename, "%s%s_%dx%d_%d.raw",LCSO_DUMP_PREFIX,usr,mLcs_outW,mLcs_outH,cnt);                                              \

    if(!makePath(LCSO_DUMP_PREFIX,0660))
    {
        VHDR_ERR("makePath [%s] fail",LCSO_DUMP_PREFIX);
        return;
    }

    if(MFALSE == SaveToFile(filename,addr))
    {
        VHDR_ERR("dump LCEO fail");
    }
}

/******************************************************************************
*
*******************************************************************************/
MBOOL VHdrDrvImp::SaveToFile(char const *filepath,const MUINTPTR &addr)
{
    MBOOL ret = MFALSE;
    MINT32 fd = -1;
    MUINT8 *pBuf = (MUINT8 *)addr;
    MUINT32 size = mLcs_outW * mLcs_outH;
    MUINT32 written = 0;
    MINT32 nw = 0, cnt = 0;

    VHDR_LOG("save to %s",filepath);

    //====== open file descriptor =======

    fd = ::open(filepath, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if( fd < 0 )
    {
        VHDR_ERR("fail to open %s: %s", filepath, ::strerror(errno));
        goto saveToFileExit;
    }

    //====== Save File ======

    while(written < size )
    {
        nw = ::write(fd, pBuf+written, size-written);
        if(nw < 0)
        {
            VHDR_ERR("fail to write %s, write-count:%d, written-bytes:%d : %s",filepath,cnt,written,::strerror(errno));
            goto saveToFileExit;
        }
        written += nw;
        cnt ++;
    }
    VHDR_LOG("[%d-th plane] write %d bytes to %s",size,filepath);

    ret = MTRUE;
saveToFileExit:

    if  ( fd >= 0 )
    {
        ::close(fd);
    }

    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::SetVHdrState(const VHDR_STATE_ENUM &aState)
{
    mState = aState;
    VHDR_LOG("aState(%d),mState(%d)",aState,mState);
}


/*******************************************************************************
*
********************************************************************************/
VHDR_STATE_ENUM VHdrDrvImp::GetVHdrState()
{
    return mState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrDrvImp::ChangeThreadSetting(char const *userName)
{
    //> set name

    ::prctl(PR_SET_NAME,userName, 0, 0, 0);

    //> set policy/priority
    {
        const MINT32 expect_policy   = SCHED_OTHER;
        const MINT32 expect_priority = NICE_CAMERA_3A_MAIN;
        MINT32 policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);

        VHDR_LOG("policy:(expect,result)=(%d,%d),priority:(expect,result)=(%d, %d)",expect_policy,policy,expect_priority,priority);
    }
}

/*******************************************************************************
*
********************************************************************************/
VHdrP1Cb::VHdrP1Cb(MVOID *arg)
{
    m_pClassObj = arg;
}

/*******************************************************************************
*
********************************************************************************/
VHdrP1Cb::~VHdrP1Cb()
{
    m_pClassObj = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void VHdrP1Cb::p1TuningNotify(MVOID *pInput,MVOID *pOutput)
{
    VHdrDrvImp *_this = reinterpret_cast<VHdrDrvImp *>(m_pClassObj);
    LCS_INPUT_INFO *apLcsInInfo = (LCS_INPUT_INFO *)pInput;
    LCS_REG_CFG *apLcsCfgData   = (LCS_REG_CFG *)pOutput;

    if(_this->mIsVhdrConfig == 0)
    {
        VHDR_LOG("not config done");
        apLcsCfgData->bLCS_Bypass = MTRUE;
        return;
    }

    apLcsCfgData->bLCS_Bypass = MFALSE;
    apLcsCfgData->bLCS_EN = (_this->mSensorTg == CAM_TG_1) ? _this->mLcsEn : _this->mLcsD_En;

    if(_this->mVrWithEis == MFALSE)
    {
        //> LCS_ST

        apLcsCfgData->_LCS_REG.LCS_ST = 0;

        if(_this->mSensorTg == CAM_TG_1)
        {
            //> LCS_CON

            MUINT32 lcsCon = 0 | (_this->mLcs_outH << 24) | (_this->mLcs_outW << 16) | 0x01;

            apLcsCfgData->_LCS_REG.LCS_CON = lcsCon;

            //> LCS_AWS

            apLcsCfgData->_LCS_REG.LCS_AWS = _this->mLcsAws;

            //> LCS_FLR

            MUINT32 lcsFlr = 0 | (_this->mFlareGain << 7) | _this->mFlareOffset;

            apLcsCfgData->_LCS_REG.LCS_FLR = lcsFlr;

            //> LCS_LRZR_1 and LCS_LRZR_2

            MUINT32 lcsInWidth  = _this->mLcsAws & 0x1FFF;
            MUINT32 lcsInHeight = (_this->mLcsAws & 0x1FFF0000) >> 16;

            MUINT32 lcsLrzrX = ((2 * _this->mLcs_outW - 2) << 20) / (lcsInWidth - 1);
            MUINT32 lcsLrzrY = ((_this->mLcs_outH - 1) << 20) / (lcsInHeight - 1);

            apLcsCfgData->_LCS_REG.LCS_LRZR_1 = lcsLrzrX;
            apLcsCfgData->_LCS_REG.LCS_LRZR_2 = lcsLrzrY;

            if(g_debugDump > 0)
            {
                VHDR_LOG("lcsIn(%u,%u),mLcs_out(%u,%u)",lcsInWidth,lcsInHeight,_this->mLcs_outW,_this->mLcs_outH);
            }
        }
        else
        {
            //> LCS_CON

            MUINT32 lcsCon = 0 | (_this->mLcsD_outH << 24) | (_this->mLcsD_outW << 16) | 0x01;

            apLcsCfgData->_LCS_REG.LCS_CON = lcsCon;

            //> LCS_AWS

            apLcsCfgData->_LCS_REG.LCS_AWS = _this->mLcsD_Aws;

            //> LCS_FLR

            MUINT32 lcsFlr = 0 | (_this->mFlareGain << 7) | _this->mFlareOffset;

            apLcsCfgData->_LCS_REG.LCS_FLR = lcsFlr;

            //> LCS_LRZR_1 and LCS_LRZR_2

            MUINT32 lcsInWidth  = _this->mLcsD_Aws & 0x1FFF;
            MUINT32 lcsInHeight = (_this->mLcsD_Aws & 0x1FFF0000) >> 16;

            MUINT32 lcsLrzrX = ((2 * _this->mLcsD_outW - 2) << 20) / (lcsInWidth - 1);
            MUINT32 lcsLrzrY = ((_this->mLcsD_outH - 1) << 20) / (lcsInHeight - 1);

            apLcsCfgData->_LCS_REG.LCS_LRZR_1 = lcsLrzrX;
            apLcsCfgData->_LCS_REG.LCS_LRZR_2 = lcsLrzrY;

            if(g_debugDump > 0)
            {
                VHDR_LOG("lcsIn(%u,%u),mLcsD_out(%u,%u)",lcsInWidth,lcsInHeight,_this->mLcsD_outW,_this->mLcsD_outH);
            }
        }

        if(g_debugDump > 0)
        {
            VHDR_LOG("CON(0x%08x),ST(0x%08x),AWS(0x%08x),FLR(0x%08x),LRZR_1(0x%08x),LRZR_2(0x%08x)",apLcsCfgData->_LCS_REG.LCS_CON,
                                                                                                    apLcsCfgData->_LCS_REG.LCS_ST,
                                                                                                    apLcsCfgData->_LCS_REG.LCS_AWS,
                                                                                                    apLcsCfgData->_LCS_REG.LCS_FLR,
                                                                                                    apLcsCfgData->_LCS_REG.LCS_LRZR_1,
                                                                                                    apLcsCfgData->_LCS_REG.LCS_LRZR_2);
        }
    }
    else
    {
        //====== Calculate Crop ======

        NSCamHW::Rect srcRect(0,0,apLcsInInfo->sRRZOut.w,apLcsInInfo->sRRZOut.h);
        NSCamHW::Rect dstRect(0,0,_this->mVideoPrvW,_this->mVideoPrvH);

        NSCamHW::Rect lcsCropRect = android::MtkCamUtils::calCrop(srcRect,dstRect,_this->mZoomRatio);
        //NSCamHW::Rect lcsCropRect = android::MtkCamUtils::calCrop(srcRect,dstRect,100);

        //====== Calculate EIS for FE Crop ======

        MUINT32 feBlock;

        if(lcsCropRect.w <= 792 && lcsCropRect.h <= 528)
        {
            feBlock = 8;
        }
        else if(lcsCropRect.w <= 2304 && lcsCropRect.h <= 1296)
        {
            feBlock = 16;
        }
        else
        {
            feBlock = 32;
            VHDR_WRN("FE should not use 32");
        }

        lcsCropRect.w = ALIGN_SIZE(lcsCropRect.w,feBlock);
        lcsCropRect.h = ALIGN_SIZE(lcsCropRect.h,feBlock);

        //====== Apply to LCS Input Image ======

        if(!apLcsInInfo->bIsHbin)
        {
            lcsCropRect.x *= 2;
            lcsCropRect.w *= 2;
        }

        lcsCropRect.y *= 2;
        lcsCropRect.h *= 2;

        VHDR_LOG("lcsCropRect(%u,%u,%u,%u)",lcsCropRect.x,lcsCropRect.y,lcsCropRect.w,lcsCropRect.h);

        if(g_debugDump > 0)
        {
            VHDR_LOG("TG(%d),bIsHbin(%d)",_this->mSensorTg,apLcsInInfo->bIsHbin);
            VHDR_LOG("RRZ(%u,%u),HBIN(%u,%u)",apLcsInInfo->sRRZOut.w,apLcsInInfo->sRRZOut.h,apLcsInInfo->sHBINOut.w,apLcsInInfo->sHBINOut.h);
            VHDR_LOG("pass1Out(%d,%d),VideoPrv(%u,%u)",_this->mPass1OutW,_this->mPass1OutH,_this->mVideoPrvW,_this->mVideoPrvH);
            VHDR_LOG("mZoomRatio(%u),feBlock(%u)",_this->mZoomRatio,feBlock);
            VHDR_LOG("LcsOut(%u,%u)",(_this->mSensorTg == CAM_TG_1) ? _this->mLcs_outW : _this->mLcsD_outW,
                                     (_this->mSensorTg == CAM_TG_1) ? _this->mLcs_outH : _this->mLcsD_outH);
        }

        //====== Prepare Reg ======

        MUINT32 lcsCon = 0;

        if(_this->mSensorTg == CAM_TG_1)
        {
            lcsCon = lcsCon | (_this->mLcs_outH << 24) | (_this->mLcs_outW << 16) | 0x01;
        }
        else
        {
            lcsCon = lcsCon | (_this->mLcsD_outH << 24) | (_this->mLcsD_outW << 16) | 0x01;
        }

        MUINT32 lcsSt  = 0 | lcsCropRect.x | (lcsCropRect.y << 16);
        MUINT32 lcsAws = 0 | lcsCropRect.w | (lcsCropRect.h << 16);
        MUINT32 lcsFlr = 0 | (_this->mFlareGain << 7) | _this->mFlareOffset;
        MUINT32 lcsLrzrX = (_this->mSensorTg == CAM_TG_1) ? ((2*_this->mLcs_outW-2)<<20)/(lcsCropRect.w-1) : ((2*_this->mLcsD_outW-2)<<20) / (lcsCropRect.w-1);
        MUINT32 lcsLrzrY = (_this->mSensorTg == CAM_TG_1) ? ((_this->mLcs_outH-1)<<20)/(lcsCropRect.h-1) : ((_this->mLcsD_outH-1)<<20)/(lcsCropRect.h-1);

        apLcsCfgData->_LCS_REG.LCS_CON = lcsCon;
        apLcsCfgData->_LCS_REG.LCS_ST  = lcsSt;
        apLcsCfgData->_LCS_REG.LCS_AWS = lcsAws;
        apLcsCfgData->_LCS_REG.LCS_FLR = lcsFlr;
        apLcsCfgData->_LCS_REG.LCS_LRZR_1 = lcsLrzrX;
        apLcsCfgData->_LCS_REG.LCS_LRZR_2 = lcsLrzrY;

        if(g_debugDump > 0)
        {
            VHDR_LOG("(con,st,aws,flr,lrzr1,lrzr2)=(0x%08x,0x%08x,0x%08x,0x%08x,0x%08x,0x%08x)",apLcsCfgData->_LCS_REG.LCS_CON,
                                                                                                apLcsCfgData->_LCS_REG.LCS_ST,
                                                                                                apLcsCfgData->_LCS_REG.LCS_AWS,
                                                                                                apLcsCfgData->_LCS_REG.LCS_FLR,
                                                                                                apLcsCfgData->_LCS_REG.LCS_LRZR_1,
                                                                                                apLcsCfgData->_LCS_REG.LCS_LRZR_2);
        }
    }
}

