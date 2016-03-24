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
#ifdef MTK_CAM_VHDR_SUPPORT

//! \file vhdr_hal.cpp
#define MTK_LOG_ENABLE 1
#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <linux/cache.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>

using namespace android;

#include "mtkcam/v1/config/PriorityDefs.h"
#include "mtkcam/hal/IHalSensor.h"

using namespace NSCam;

#include "mtkcam/featureio/IHal3A.h"
using namespace NS3A;

#include <mtkcam/utils/common.h>
using namespace NSCam::Utils;

#include "ae_param.h"

#include "vhdr_drv.h"
#include "vhdr_hal_imp.h"

/*******************************************************************************
*
********************************************************************************/
#define VHDR_HAL_DEBUG

#ifdef VHDR_HAL_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#define LOG_TAG "VHdrHal"
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

#define VHDR_HAL_NAME "VHdrHal"

/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;

/*******************************************************************************
*
********************************************************************************/
VHdrHal *VHdrHal::CreateInstance(char const *userName, const MUINT32 &aSensorIdx)
{
    VHDR_LOG("%s",userName);
    return VHdrHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHal *VHdrHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    VHDR_LOG("aSensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return VHdrHalObj<0>::GetInstance();
        case 1 : return VHdrHalObj<1>::GetInstance();
        case 2 : return VHdrHalObj<2>::GetInstance();
        case 3 : return VHdrHalObj<3>::GetInstance();
        default :
            VHDR_WRN("current limit is 4 sensors, use 0");
            return VHdrHalObj<0>::GetInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::DestroyInstance(char const *userName)
{
    VHDR_LOG("%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
VHdrHalImp::VHdrHalImp(const MUINT32 &aSensorIdx) : VHdrHal()
{
    mUsers = 0;

    m_pVHdrDrv = NULL;
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;
    m_p3aHal = NULL;
    m_pVHdrCb3A = NULL;

    mSensorIdx = aSensorIdx;
    mSensorDev = 0;

    mState = VHDR_STATE_NONE;
    mVhdrMode = 0;
    memset((void*)&mSensorStaticInfo, 0, sizeof(mSensorStaticInfo));
    memset((void*)&mSensorDynamicInfo, 0, sizeof(mSensorDynamicInfo));
    mVsyncThread = NULL;
    ::sem_init(&mVsyncSem, 0, 0);
    ::sem_init(&mVsyncSemEnd, 0, 0);
}

/*******************************************************************************
*
********************************************************************************/
MVOID *VHdrHalImp::VHdrThreadLoop(MVOID *arg)
{
    VHdrHalImp *_this = reinterpret_cast<VHdrHalImp *>(arg);
    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Change Thread Setting ======

    _this->ChangeThreadSetting();

    //====== Main Loop ======

    VHDR_STATE_ENUM eState = _this->GetVHdrState();

    while(eState != VHDR_STATE_UNINIT)
    {
        VHDR_LOG("wait mVsyncSem");
        ::sem_wait(&_this->mVsyncSem); // wait here until someone use sem_post() to wake this semaphore up
        VHDR_LOG("got mVsyncSem");

        eState = _this->GetVHdrState();

        switch(eState)
        {
            case VHDR_STATE_ALIVE:
                    err = _this->UpdateVHdr();
                    if(err != VHDR_RETURN_NO_ERROR)
                    {
                        VHDR_ERR("UpdateVHdr fail(%d)",err);
                    }
                break;
            case VHDR_STATE_UNINIT :
                VHDR_LOG("UNINIT");
                break;
            default:
                VHDR_ERR("State Error(%d)",eState);
        }

        eState = _this->GetVHdrState();
    }

    ::sem_post(&_this->mVsyncSemEnd);
    VHDR_LOG("post mVsyncSemEnd");

    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Init(const MUINT32 &aMode)
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
        return VHDR_RETURN_NO_ERROR;
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

#if (VHDR_DEBUG_FLAG)

    VHDR_INF("VHDR_DEBUG_FLAG on");
    g_debugDump = 3;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.vhdr.dump", value, "0");
    g_debugDump = atoi(value);

#endif

    VHDR_LOG("mSensorIdx(%u),mode(%u) init",mSensorIdx,aMode);

    //====== Set State ======

    SetVHdrState(VHDR_STATE_ALIVE);

    //====== Save Mode ======

    mVhdrMode = aMode;

    //====== Create Sensor Object ======

    m_pHalSensorList = IHalSensorList::get();
    if(m_pHalSensorList == NULL)
    {
        VHDR_ERR("IHalSensorList::get fail");
        goto create_fail_exit;
    }

    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        //====== Create VHDR Driver ======

        m_pVHdrDrv = VHdrDrv::CreateInstance(mSensorIdx);

        if(m_pVHdrDrv == NULL)
        {
            VHDR_ERR("VHdrDrv::createInstance fail");
            goto create_fail_exit;
        }

        err = m_pVHdrDrv->Init();
        if(err != VHDR_RETURN_NO_ERROR)
        {
            VHDR_ERR("VHdrDrv::Init fail");
            goto create_fail_exit;
        }

        //====== Create 3A Callback ======

        m_pVHdrCb3A = new VHdrHalCallBack(this);

        //====== Create Thread  ======

        //> init semphore

        ::sem_init(&mVsyncSem, 0, 0);
        ::sem_init(&mVsyncSemEnd, 0, 0);

        //> create thread

        pthread_create(&mVsyncThread, NULL, VHdrThreadLoop, this);

        android_atomic_inc(&mUsers);
        VHDR_LOG("-");
        return VHDR_RETURN_NO_ERROR;
    }
    else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
    {
        android_atomic_inc(&mUsers);
        VHDR_LOG("-");
        return VHDR_RETURN_NO_ERROR;
    }
    else
    {
        VHDR_ERR("wrong mode(%u)",mVhdrMode);
    }

create_fail_exit:

    if(m_pVHdrDrv != NULL)
    {
        m_pVHdrDrv->Uninit();
        m_pVHdrDrv->DestroyInstance();
        m_pVHdrDrv = NULL;
    }

    if(m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

    VHDR_LOG("-");
    return VHDR_RETURN_INVALID_DRIVER;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        VHDR_LOG("mSensorIdx(%u) has 0 user",mSensorIdx);
        return VHDR_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)
    {
        VHDR_LOG("mSensorIdx(%u),mode(%u) uninit",mSensorIdx,mVhdrMode);

        MINT32 err = VHDR_RETURN_NO_ERROR;

        //====== Set State ======

        SetVHdrState(VHDR_STATE_UNINIT);

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
        {
            //====== Delete 3A  ======

            if(m_p3aHal != NULL)
            {

                m_p3aHal->SetAETargetMode(AE_MODE_NORMAL);

                m_p3aHal->removeCallbacks(m_pVHdrCb3A);

                m_p3aHal->destroyInstance(VHDR_HAL_NAME);
                m_p3aHal = NULL;

                if(m_pVHdrCb3A != NULL)
                {
                    delete m_pVHdrCb3A;
                    m_pVHdrCb3A = NULL;
                }
            }

            //====== Destroy VHDR Driver ======

            if(m_pVHdrDrv != NULL)
            {
                //> disable VHDR

                //m_pVHdrDrv->EnableLcs(MFALSE);    //ISP driver will reset
                m_pVHdrDrv->EnableLce(MFALSE);
                m_pVHdrDrv->EnableRmg(MFALSE);

                m_pVHdrDrv->Uninit();
                m_pVHdrDrv->DestroyInstance();
                m_pVHdrDrv = NULL;
            }

            //====== Destroy Thread ======

            VHDR_LOG("post mVsyncSem");
            ::sem_post(&mVsyncSem);

            //wait sem
            VHDR_LOG("wait mVsyncSemEnd");
            ::sem_wait(&mVsyncSemEnd);
            VHDR_LOG("got mVsyncSemEnd");

            pthread_join(mVsyncThread, NULL);
        }
        else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
        {
            //====== Delete 3A  ======

            if(m_p3aHal != NULL)
            {
                VHDR_LOG("set AE to normal");
                m_p3aHal->SetAETargetMode(AE_MODE_NORMAL);
                m_p3aHal->destroyInstance(VHDR_HAL_NAME);
                m_p3aHal = NULL;

            }
        }
        else
        {
            VHDR_ERR("wrong mode(%u)",mVhdrMode);
        }

        //====== Set State ======

        mVhdrMode = 0;
        SetVHdrState(VHDR_STATE_NONE);
    }
    else
    {
        VHDR_LOG("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    VHDR_LOG("-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::GetSensorInfo()
{
    VHDR_LOG("mSensorIdx(%u)",mSensorIdx);

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&mSensorStaticInfo);

    m_pHalSensor = m_pHalSensorList->createSensor(VHDR_HAL_NAME,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        VHDR_ERR("m_pHalSensorList->createSensor fail");
        return VHDR_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&mSensorDynamicInfo))
    {
        VHDR_ERR("querySensorDynamicInfo fail");
        return VHDR_RETURN_API_FAIL;
    }

    m_pHalSensor->destroyInstance(VHDR_HAL_NAME);
    m_pHalSensor = NULL;

    return VHDR_RETURN_NO_ERROR;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData)
{
    VHDR_LOG("mode(%u)",mVhdrMode);

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Get Sensor Info ======

    err = GetSensorInfo();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        VHDR_ERR("GetSensorInfo fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }

    //====== Create 3A Object ======

    m_p3aHal = IHal3A::createInstance(ConvertCameraVer(aConfigData.cameraVer),static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
    if(m_p3aHal == NULL)
    {
        VHDR_ERR("m_p3aHal create fail");
        return VHDR_RETURN_NULL_OBJ;
    }

    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        //> set to VHDR driver

        m_pVHdrDrv->SetSensorInfo(mSensorDev,mSensorDynamicInfo.TgInfo);

        //====== Add 3A Callback ======

        m_p3aHal->addCallbacks(m_pVHdrCb3A);

        //====== Set AE Target Mode ======

        m_p3aHal->SetAETargetMode(AE_MODE_AOE_TARGET);

        //====== Config LCS ======

        FrameOutputParam_T flareInfo;
        m_p3aHal->getRTParams(flareInfo);

        // set flare info befoer configure LCS
        m_pVHdrDrv->SetFlareInfo(flareInfo.i2FlareGain,flareInfo.i2FlareOffset);

        err = m_pVHdrDrv->ConfigLcs();
        if(err != VHDR_RETURN_NO_ERROR)
        {
            VHDR_ERR("ConfigLcs fail(%d)", err);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Config LCSO ======

        err = m_pVHdrDrv->ConfigLcso();
        if(err != VHDR_RETURN_NO_ERROR)
        {
            VHDR_ERR("ConfigLcso fail(%d)", err);
            return VHDR_RETURN_API_FAIL;
        }

        //====== Config RMG ======

        MUINT32 isLeFirst = static_cast<MUINT32>(mSensorStaticInfo.iHDR_First_IS_LE);
        VHDR_LOG("isLeFirst(%u)",isLeFirst);    // 0 : first two rows are SE,  1 : first two rows are LE

        m_pVHdrDrv->ConfigRmg(isLeFirst);    // IHDR/IHDR_D configure with RMG

        //====== Enable VHDR ======

        m_pVHdrDrv->EnableLcs(MTRUE);
        m_pVHdrDrv->EnableLce(MTRUE);
        m_pVHdrDrv->EnableRmg(MTRUE);

        //====== Set Configure Done ======

        m_pVHdrDrv->SetLcsoThreadState(VHDR_STATE_ALIVE);
    }
    else if(mVhdrMode == SENSOR_VHDR_MODE_MVHDR)
    {
        //====== Create 3A Object ======

        m_p3aHal = IHal3A::createInstance(ConvertCameraVer(aConfigData.cameraVer),static_cast<MINT32>(mSensorIdx),VHDR_HAL_NAME);
        if(m_p3aHal == NULL)
        {
            VHDR_ERR("m_p3aHal create fail");
            return VHDR_RETURN_NULL_OBJ;
        }

        //====== Set AE Target Mode ======

        m_p3aHal->SetAETargetMode(AE_MODE_MVHDR_TARGET);
    }
    else
    {
        VHDR_ERR("wrong mode(%u)",mVhdrMode);
    }

    VHDR_LOG("-");
    return VHDR_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::DoVHdr(const MINT64 &aTimeStamp)
{
    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        m_pVHdrDrv->UpdateLcsoTimeStamp(aTimeStamp);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::GetLceiDmaInfo(LCEI_DMA_INFO *apLceiDmaInfo,const MINT64 &aTimeStamp)
{
    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        if(mSensorStaticInfo.iHDRSupport != MTRUE)
        {
            VHDR_ERR("not support iVHDR(%d)",mSensorStaticInfo.iHDRSupport);
            apLceiDmaInfo->memID= VHDR_LCSO_SYNC_FAIL;
        }
        else
        {
            m_pVHdrDrv->GetLceiDmaInfo(apLceiDmaInfo,aTimeStamp);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MINT32 VHdrHalImp::UpdateVHdr()
{
    if(g_debugDump > 0)
    {
        VHDR_LOG("+");
    }

    MINT32 err = VHDR_RETURN_NO_ERROR;

    //====== Updade Flare for LCS ======

    if(m_p3aHal != NULL)
    {
        FrameOutputParam_T flareInfo;
        m_p3aHal->getRTParams(flareInfo);
        m_pVHdrDrv->SetFlareInfo(flareInfo.i2FlareGain,flareInfo.i2FlareOffset);
    }
    else
    {
        VHDR_ERR("m_p3aHal is NULL");
    }

    //====== Update LCS ======

#if 0   //no need. use callback to set
    err = m_pVHdrDrv->UpdateLcs();
    if(err != VHDR_RETURN_NO_ERROR)
    {
        VHDR_ERR("UpdateLcs fail(%d)", err);
        return VHDR_RETURN_API_FAIL;
    }
#endif

    if(g_debugDump > 0)
    {
        VHDR_LOG("-");
    }
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::SendCommand(VHDR_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    if(mVhdrMode == SENSOR_VHDR_MODE_IVHDR)
    {
        switch(aCmd)
        {
            case VHDR_CMD_SET_STATE:
                m_pVHdrDrv->SetLcsoThreadState((VHDR_STATE_ENUM)arg1);
                break;
            case VHDR_CMD_SET_VR_EIS_ON_OFF:
                m_pVHdrDrv->SetVideoSizeInfo(aCmd,arg1);
                break;
            case VHDR_CMD_SET_VIDEO_SIZE:
                m_pVHdrDrv->SetVideoSizeInfo(aCmd,arg1,arg2);
                break;
            case VHDR_CMD_SET_ZOOM_RATIO:
                m_pVHdrDrv->SetVideoSizeInfo(aCmd,arg1);
                break;
            case VHDR_CMD_SET_PASS1_OUT_SIZE:
                m_pVHdrDrv->SetVideoSizeInfo(aCmd,arg1,arg2);
                break;
            default:
                VHDR_ERR("wrong cmd(%d)",aCmd);
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::SetVHdrState(const VHDR_STATE_ENUM &aState)
{
    mState = aState;
    VHDR_LOG("aState(%d),mState(%d)",aState,mState);
}

/*******************************************************************************
*
********************************************************************************/
VHDR_STATE_ENUM VHdrHalImp::GetVHdrState()
{
    return mState;
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::ChangeThreadSetting()
{
    //> set name

    ::prctl(PR_SET_NAME,"VHdrThread", 0, 0, 0);

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
IHal3A::E_VER VHdrHalImp::ConvertCameraVer(const VHDR_CAMERA_VER_ENUM &aVer)
{
    switch(aVer)
    {
        case VHDR_CAMERA_VER_1 : return IHal3A::E_Camera_1;
        case VHDR_CAMERA_VER_3 : return IHal3A::E_Camera_3;
        default : VHDR_ERR("unknown ver(%d) return E_Camera_1",aVer);
                  return IHal3A::E_Camera_1;
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID VHdrHalImp::VHdrSync3A()
{
    ::sem_post(&mVsyncSem);
    VHDR_LOG("post mVsyncSem");
}

/*******************************************************************************
*
********************************************************************************/
VHdrHalCallBack::VHdrHalCallBack(MVOID *arg)
{
    m_pVHdrHalImp = arg;
}

/*******************************************************************************
*
********************************************************************************/
VHdrHalCallBack::~VHdrHalCallBack()
{
    m_pVHdrHalImp = NULL;
}

/*******************************************************************************
*
********************************************************************************/
void VHdrHalCallBack::doNotifyCb(int32_t _msgType,int32_t _ext1,int32_t _ext2,int32_t _ext3, MINTPTR _ext4)
{
    VHDR_LOG("msgType(%d)",_msgType);
    VHdrHalImp *_this = reinterpret_cast<VHdrHalImp *>(m_pVHdrHalImp);

    if(_msgType == eID_NOTIFY_VSYNC_DONE)
    {
        _this->VHdrSync3A();
    }

    VHDR_LOG("-");
}

/*******************************************************************************
*
********************************************************************************/
void VHdrHalCallBack::doDataCb(int32_t _msgType,void *_data,uint32_t _size)
{
}

#endif
