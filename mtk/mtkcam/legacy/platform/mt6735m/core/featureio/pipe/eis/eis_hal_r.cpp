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

//! \file  eis_hal_r.cpp

#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <queue>

using namespace std;

#include "eis_drv_base.h"
#include <mtkcam/drv_common/imem_drv.h>


#include "mtkcam/v1/config/PriorityDefs.h"
#include "mtkcam/hal/IHalSensor.h"
#include <mtkcam/Log.h>
using namespace NSCam;
using namespace android;

#include "eis_hal_r.h"
#include "camera_custom_eis.h"

/*******************************************************************************
*
********************************************************************************/
#define EIS_DEBUG

#ifdef EIS_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#undef  LOG_TAG
#define LOG_TAG "EisHal"

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

#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define DEBUG_DUMP_FRAMW_NUM 10

/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump_R = 0;

/*******************************************************************************
*
********************************************************************************/
EisHal_R *EisHal_R::CreateInstance(char const *userName,const MUINT32 &aSensorIdx)
{
    EIS_LOG_R("%s",userName);
    return EisHalImp::GetInstance(aSensorIdx);
}

/*******************************************************************************
*
********************************************************************************/
EisHal_R *EisHalImp::GetInstance(const MUINT32 &aSensorIdx)
{
    EIS_LOG_R("sensorIdx(%u)",aSensorIdx);

    switch(aSensorIdx)
    {
        case 0 : return EisHalObj<0>::GetInstance();
        case 1 : return EisHalObj<1>::GetInstance();
        case 2 : return EisHalObj<2>::GetInstance();
        case 3 : return EisHalObj<3>::GetInstance();
        default :
            EIS_WRN_R("R-chip limit is 4 sensors, use 0");
            return EisHalObj<0>::GetInstance();
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::DestroyInstance(char const *userName)
{
    EIS_LOG_R("%s",userName);
}

/*******************************************************************************
*
********************************************************************************/
EisHalImp::EisHalImp(const MUINT32 &aSensorIdx) : EisHal_R()
{
    mUsers = 0;

    //> EIS driver object
	m_pEisDrv = NULL;

    //> member variable
    mEisInput_W = 0;
    mEisInput_H = 0;
    mP1Target_W = 0;
    mP1Target_H = 0;
    mCmvX_Int = 0;
    mCmvX_Flt = 0;
    mCmvY_Int = 0;
    mCmvY_Flt = 0;
    mGMV_X = 0;
    mGMV_Y = 0;
    mFrameCnt = 0;
    mEisPass1Only = 1;
    mIsEisConfig = 0;
    mSensorIdx = aSensorIdx;
    mEisSupport = MTRUE;
    mMemAlignment = 0;

    //> EIS algo
    m_pEisAlg = NULL;

    //> IMEM
    m_pIMemDrv = NULL;

    mEisDbgBuf.memID = -5;
    mEisDbgBuf.virtAddr = mEisDbgBuf.phyAddr = mEisDbgBuf.size = 0;

    //> sensor
    m_pHalSensorList = NULL;
    m_pHalSensor = NULL;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::Init()
{
    //====== Check Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        android_atomic_inc(&mUsers);
        EIS_LOG_R("snesorIdx(%u) has %d users",mSensorIdx,mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    //====== Dynamic Debug ======

#if (EIS_DEBUG_FLAG)

    EIS_INF_R("EIS_DEBUG_FLAG on");
    g_debugDump_R = 1;

#else

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dumpR", value, "0");
    g_debugDump_R = atoi(value);

#endif

    EIS_LOG_R("mSensorIdx(%u) init", mSensorIdx);

    //====== Create Sensor Object ======

    m_pHalSensorList = IHalSensorList::get();
    if(m_pHalSensorList == NULL)
    {
        EIS_ERR_R("IHalSensorList::get fail");
        goto create_fail_exit_R;
    }

    if(EIS_RETURN_NO_ERROR != GetSensorInfo())
    {
        EIS_ERR_R("GetSensorInfo fail");
        goto create_fail_exit_R;
    }

    //====== Create EIS Driver ======

    m_pEisDrv = EisDrv_R::CreateInstance(mSensorIdx);

    if(m_pEisDrv == NULL)
    {
        EIS_ERR_R("EisDrv_R::createInstance fail");
        goto create_fail_exit_R;
    }

    err = m_pEisDrv->Init();
    if(err != EIS_RETURN_NO_ERROR)
    {
        EIS_ERR_R("EisDrv_R::Init fail");
        goto create_fail_exit_R;
    }

    //====== Create EIS Algorithm Object ======

    EIS_LOG_R("TG(%d)",mSensorDynamicInfo.TgInfo);

    if(mSensorDynamicInfo.TgInfo == CAM_TG_1)
    {
        m_pEisAlg = MTKEis::createInstance();

        if(m_pEisAlg == NULL)
        {
            EIS_ERR_R("MTKEis::createInstance fail");
            goto create_fail_exit_R;
        }
    }

    if(mSensorDynamicInfo.TgInfo == CAM_TG_1 && m_pEisAlg == NULL)
    {
        EIS_ERR_R("m_pEisAlg is NULL");
        goto create_fail_exit_R;
    }

    //====== Create IMem Object ======

    m_pIMemDrv = IMemDrv::createInstance();

    if(m_pIMemDrv == NULL)
    {
        EIS_ERR_R("Null IMemDrv Obj");
        goto create_fail_exit_R;
    }

    if(MTRUE != m_pIMemDrv->init())
    {
        EIS_ERR_R("m_pIMemDrv->init fail");
        goto create_fail_exit_R;
    }

    if(g_debugDump_R >= 2)
    {
        //====== EIS Debug Buffer ======

        //MUINT32 eisMemSize = EIS_LOG_BUFFER_SIZE;
        MUINT32 eisMemSize = 0x1000;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO,NULL,&eisMemSize);
        if(err != S_EIS_OK || eisMemSize == 0)
        {
            EIS_ERR_R("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
        }
        else
        {
            CreateMemBuf(eisMemSize,1,&mEisDbgBuf);
            if(mEisDbgBuf.virtAddr == 0 && mEisDbgBuf.phyAddr == 0)
            {
                EIS_ERR_R("mEisDbgBuf create IMem fail");
                return EIS_RETURN_MEMORY_ERROR;
            }

            EIS_LOG_R("mEisDbgBuf : memID(%d),size(%u),virAdd(0x%x),phyAddr(0x%x)",mEisDbgBuf.memID,
                                                                               mEisDbgBuf.size,
                                                                               mEisDbgBuf.virtAddr,
                                                                               mEisDbgBuf.phyAddr);
        }
    }

    //====== Increase User Count ======

    android_atomic_inc(&mUsers);

    EIS_LOG_R("-");
    return EIS_RETURN_NO_ERROR;

create_fail_exit_R:

    if(m_pEisDrv != NULL)
    {
        m_pEisDrv->Uninit();
        m_pEisDrv->DestroyInstance();
        m_pEisDrv = NULL;
    }

    if(m_pHalSensorList != NULL)
    {
        m_pHalSensorList = NULL;
    }

    if(m_pEisAlg != NULL)
    {
        m_pEisAlg->EisReset();
        m_pEisAlg->destroyInstance();
        m_pEisAlg = NULL;
    }

    if(m_pIMemDrv != NULL)
    {
        m_pIMemDrv->destroyInstance();
        m_pIMemDrv = NULL;
    }

    EIS_LOG_R("-");
    return EIS_RETURN_NULL_OBJ;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::Uninit()
{
    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        EIS_LOG_R("mSensorIdx(%u) has 0 user",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = EIS_RETURN_NO_ERROR;

        EIS_LOG_R("mSensorIdx(%u) uninit", mSensorIdx);

        //====== Release EIS Driver Object ======

        //err = m_pEisDrv->EnableEis(MFALSE);   // ISP will reset TOP EN bit
        //if(err != EIS_RETURN_NO_ERROR)
        //{
        //    EIS_ERR_R("Disable EIS fail(0x%x)",err);
        //}

     	if(m_pEisDrv != NULL)
        {
            EIS_LOG_R("m_pEisDrv uninit");
            m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_UNINIT_CHECK);
            m_pEisDrv->Uninit();
        	m_pEisDrv->DestroyInstance();
        	m_pEisDrv = NULL;
    	}

        //======  Release EIS Algo Object ======

        if(g_debugDump_R >= 2)
        {
            if(mSensorDynamicInfo.TgInfo == CAM_TG_1)
            {
                EIS_SET_LOG_BUFFER_STRUCT pEisAlgoLogInfo;

                pEisAlgoLogInfo.Eis_Log_Buf_Addr = (void*)mEisDbgBuf.virtAddr;
                pEisAlgoLogInfo.Eis_Log_Buf_Size = mEisDbgBuf.size;

            	    err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SAVE_LOG,&pEisAlgoLogInfo, NULL);
        	    if(err != S_EIS_OK)
        	    {
        	        EIS_ERR_R("EisFeatureCtrl(EIS_FEATURE_SAVE_LOG) fail(0x%x)",err);
        	    }
            }
        }

        EIS_LOG_R("TG(%d)",mSensorDynamicInfo.TgInfo);

        if(m_pEisAlg != NULL && mSensorDynamicInfo.TgInfo == CAM_TG_1)
        {
            EIS_LOG_R("m_pEisAlg uninit");
            m_pEisAlg->EisReset();
            m_pEisAlg->destroyInstance();
            m_pEisAlg = NULL;
        }

        //====== Destroy Sensor Object ======

        if(m_pHalSensorList != NULL)
        {
            m_pHalSensorList = NULL;
        }

        //======  Release Memory and IMem Object ======

        if(g_debugDump_R >= 2)
        {
            //>  free EIS debug buffer

            DestroyMemBuf(1,&mEisDbgBuf);

            mEisDbgBuf.memID = -5;
            mEisDbgBuf.virtAddr = mEisDbgBuf.phyAddr = mEisDbgBuf.size = 0;
        }

        if(m_pIMemDrv != NULL)
        {
            EIS_LOG_R("m_pIMemDrv uninit");
            if(MTRUE != m_pIMemDrv->uninit())
            {
                EIS_ERR_R("m_pIMemDrv->uninit fail");
            }
            m_pIMemDrv->destroyInstance();
            m_pIMemDrv = NULL;
        }

        //======  Reset Member Variable ======

        mEisInput_W = 0;
        mEisInput_H = 0;
        mP1Target_W = 0;
        mP1Target_H = 0;
        mFrameCnt = 0; // first frmae
        mEisPass1Only = 1;
        mIsEisConfig = 0;
        mCmvX_Int = 0;
        mCmvX_Flt = 0;
        mCmvY_Int = 0;
        mCmvY_Flt = 0;
        mGMV_X = 0;
        mGMV_Y = 0;
        mMemAlignment = 0;
    }
    else
    {
        EIS_LOG_R("mSensorIdx(%u) has %d users",mSensorIdx,mUsers);
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    EIS_LOG_R("Cnt(%d),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

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
MINT32 EisHalImp::DestroyMemBuf(const MUINT32 & bufCnt, IMEM_BUF_INFO *bufInfo)
{
    EIS_LOG_R("Cnt(%d)", bufCnt);

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
MINT32 EisHalImp::GetSensorInfo()
{
    EIS_LOG_R("mSensorIdx(%u)",mSensorIdx);

    mSensorDev = m_pHalSensorList->querySensorDevIdx(mSensorIdx);
    m_pHalSensorList->querySensorStaticInfo(mSensorDev,&mSensorStaticInfo);

    m_pHalSensor = m_pHalSensorList->createSensor(LOG_TAG,1,&mSensorIdx);
    if(m_pHalSensor == NULL)
    {
        EIS_ERR_R("m_pHalSensorList->createSensor fail");
        return EIS_RETURN_API_FAIL;
    }

    if(MFALSE == m_pHalSensor->querySensorDynamicInfo(mSensorDev,&mSensorDynamicInfo))
    {
        EIS_ERR_R("querySensorDynamicInfo fail");
        return EIS_RETURN_API_FAIL;
    }

    m_pHalSensor->destroyInstance(LOG_TAG);
    m_pHalSensor = NULL;

    return EIS_RETURN_NO_ERROR;
}

#if 1
#define CIF_WIDTH 352
#define CIF_HEIGHT 320

#define D1_WIDTH 792
#define D1_HEIGHT 528

#define HD_720P_WIDTH 1536
#define HD_720P_HEIGHT 864

#define HD_1080P_WIDTH 2112
#define HD_1080P_HEIGHT 1188
extern MUINT32 gDivV,gDivH,gEisWinNum;
typedef struct
{
    MBOOL  pathCDRZ;    // 0 : before, 1 : after
    MINT32 IIR_DS;  // 1 or 2 or 4
    MINT32 RPNum_H; // 1~16
    MINT32 RPNum_V; // 1~8
    MINT32 MBNum_H; // 1~4
    MINT32 MBNum_V; // 1~8
    MINT32 AD_Knee;
    MINT32 AD_Clip;
    MINT32 Gain_H;  // horizaontal gain control, 0~3
    MINT32 IIR_Gain_H;  // 3 or 4
    MINT32 IIR_Gain_V;  // 3 or 4
    MINT32 FIR_Gain_H;  // 16 or 32
    MINT32 LMV_TH_X_Cent;
    MINT32 LMV_TH_X_Surrd;
    MINT32 LMV_TH_Y_Cent;
    MINT32 LMV_TH_Y_Surrd;
    MINT32 FL_Offset_H; // -47~48
    MINT32 FL_Offset_V; // -64~65
    MINT32 MB_Offset_H;
    MINT32 MB_Offset_V;
    MINT32 MB_Intv_H;
    MINT32 MB_Intv_V;
}EIS_GET_PROC_INFO_STRUCT, *P_EIS_GET_PROC_INFO_STRUCT;



extern void EisGetProcInfo(P_EIS_CONFIG_IMAGE_INFO_STRUCT pEisImgInfo,P_EIS_GET_PROC_INFO_STRUCT pGetProcInfo);

#endif


/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::ConfigEis(const EIS_HAL_CONFIG_DATA &aEisConfig)
{

    EIS_GET_PROC_INFO_STRUCT  eisHWSetting;
    EIS_CONFIG_IMAGE_INFO_STRUCT eisImageSize;


    if(mEisSupport == MFALSE)
    {
        EIS_LOG_R("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    MINT32 err = EIS_RETURN_NO_ERROR;

    static EIS_SET_ENV_INFO_STRUCT eisAlgoInitData;

    //====== Get Pass1 Memory Alignment ======

    mMemAlignment = aEisConfig.memAlignment;

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mMemAlignment(%u)",mMemAlignment);
    }

    //====== Check EIS Configure Scenario ======

    switch(aEisConfig.configSce)
    {
        case EIS_SCE_EIS :
        case EIS_SCE_MFBLL :
        case EIS_SCE_MT :
        case EIS_SCE_MAV :
            mEisPass1Only = 1;
        break;
        case EIS_SCE_EIS_PLUS:
            mEisPass1Only = 1;
            EIS_ERR_R("no EIS PLUS");
        break;
        default :
            EIS_ERR_R("wrong EIS config scenario(%d)",aEisConfig.configSce);
            return EIS_RETURN_INVALID_PARA;
    }

    //====== Check Sensor Type ======

    EIS_SENSOR_ENUM sensorType;

    switch(aEisConfig.sensorType)
    {
        case NSSensorType::eRAW:
            sensorType = EIS_RAW_SENSOR;
            break;
        case NSSensorType::eYUV:
            sensorType = EIS_YUV_SENSOR;
            break;
        default:
            EIS_ERR_R("not support sensor type(%u), use RAW setting",aEisConfig.sensorType);
            sensorType = EIS_RAW_SENSOR;
            break;
    }

    //====== EIS Algo Init ======

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mIsEisConfig(%u)",mIsEisConfig);
    }

    if(mIsEisConfig == 0)
    {
        EIS_LOG_R("Sce(%d)",aEisConfig.configSce);

        //> get EIS customize setting

        GetEisCustomize(&eisAlgoInitData.eis_tuning_data);

        eisAlgoInitData.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;   // RAW domain

        //> init EIS algo

        err = m_pEisAlg->EisInit(&eisAlgoInitData);
        if(err != S_EIS_OK)
        {
            EIS_ERR_R("EisInit fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        mEisInput_W = aEisConfig.p1ImgW;
        mEisInput_H = aEisConfig.p1ImgH;

        if(m_pEisDrv->Get2PixelMode())
        {
            mEisInput_W >>= 1;
        }

        mEisInput_W -= 4;   //ryan wang request to -4
        mEisInput_H -= 4;   //ryan wang request to -4

        mP1Target_W = (mEisInput_W / (EIS_FACTOR / 100.0));
        mP1Target_H = (mEisInput_H / (EIS_FACTOR / 100.0));


        // set image size info
        eisImageSize.InputWidth   = mEisInput_W;
        eisImageSize.InputHeight  = mEisInput_H;
        eisImageSize.TargetWidth  = mP1Target_W;
        eisImageSize.TargetHeight = mP1Target_H;

        // query setting from EIS algorithm
        //err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_PROC_INFO, &eisImageSize, &eisHWSetting);
        EisGetProcInfo(&eisImageSize, &eisHWSetting);

        if(g_debugDump_R >= 2)
        {
            EIS_SET_LOG_BUFFER_STRUCT eisAlgoLogInfo;

            eisAlgoLogInfo.Eis_Log_Buf_Addr = (void*)mEisDbgBuf.virtAddr;
            eisAlgoLogInfo.Eis_Log_Buf_Size = mEisDbgBuf.size;

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO, &eisAlgoLogInfo, NULL);
            if(err != S_EIS_OK)
            {
                EIS_ERR_R("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
            }
        }
    }

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("EIS tuning_data");
        EIS_LOG_R("sensitivity(%d)",eisAlgoInitData.eis_tuning_data.sensitivity);
        EIS_LOG_R("filter_small_motion(%u)",eisAlgoInitData.eis_tuning_data.filter_small_motion);
        EIS_LOG_R("advtuning_data.new_tru_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.new_tru_th);
        EIS_LOG_R("advtuning_data.vot_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_th);
        EIS_LOG_R("advtuning_data.votb_enlarge_size(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.votb_enlarge_size);
        EIS_LOG_R("advtuning_data.min_s_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.min_s_th);
        EIS_LOG_R("advtuning_data.vec_th(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.vec_th);
        EIS_LOG_R("advtuning_data.spr_offset(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_offset);
        EIS_LOG_R("advtuning_data.spr_gain1(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain1);
        EIS_LOG_R("advtuning_data.spr_gain2(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.spr_gain2);
        EIS_LOG_R("advtuning_data.vot_his_method(%d)",eisAlgoInitData.eis_tuning_data.advtuning_data.vot_his_method);
        EIS_LOG_R("advtuning_data.smooth_his_step(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.smooth_his_step);
        EIS_LOG_R("advtuning_data.eis_debug(%u)",eisAlgoInitData.eis_tuning_data.advtuning_data.eis_debug);

        for(MINT32 i = 0; i < 4; ++i)
        {
            EIS_LOG_R("gmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_pan_array[i]);
            EIS_LOG_R("gmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.gmv_sm_array[i]);
            EIS_LOG_R("cmv_pan_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_pan_array[i]);
            EIS_LOG_R("cmv_sm_array[%d]=%u",i,eisAlgoInitData.eis_tuning_data.advtuning_data.cmv_sm_array[i]);
        }
    }


    //===== EIS HW Config ======

    err = m_pEisDrv->ConfigEisReg(aEisConfig.configSce,mSensorDynamicInfo.TgInfo);
    if(err != EIS_RETURN_NO_ERROR)
    {
        EIS_ERR_R("ConfigEisReg fail(0x%x)",err);
        return EIS_RETURN_API_FAIL;
    }

    //====== Enable EIS ======

    if(mIsEisConfig == 0)
    {
        err = m_pEisDrv->EnableEis(MTRUE);
        if(err != EIS_RETURN_NO_ERROR)
        {
            EIS_ERR_R("Enable EIS fail(0x%x)",err);
            return EIS_RETURN_API_FAIL;
        }

        //> set EISO thread state
        m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_ALIVE);

        //> turn on eis configure one-time flag
        mIsEisConfig = 1;
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHalImp::DoEis(EIS_HAL_CONFIG_DATA *apEisConfig,MINT64 aTimeStamp)
{
    MINT32 err = EIS_RETURN_NO_ERROR;

    if(mEisSupport == MFALSE)
    {
        EIS_LOG_R("mSensorIdx(%u) not support EIS",mSensorIdx);
        return EIS_RETURN_NO_ERROR;
    }

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("mEisPass1Only(%u)",mEisPass1Only);
    }

    //====== EIS Process ======

    if(aTimeStamp <= 0)
    {
        m_pEisDrv->GetEisHwStatistic(NULL,0);   // MW drop frame
    }
    else
    {
        //====== Setting EIS Algo Process Data ======

        EIS_RESULT_INFO_STRUCT   eisCMVResult;

        {
            //> EIS input image size
            Mutex::Autolock lock(mP1Lock);

            mEisInput_W = apEisConfig->p1ImgW;
            mEisInput_H = apEisConfig->p1ImgH;

            if(m_pEisDrv->Get2PixelMode())
            {
                mEisInput_W >>= 1;
            }

            mEisInput_W -= 4;   //ryan wang request to -4
            mEisInput_H -= 4;   //ryan wang request to -4

            mP1Target_W = (mEisInput_W / (EIS_FACTOR / 100.0));
            mP1Target_H = (mEisInput_H / (EIS_FACTOR / 100.0));

            mEisAlgoProcData.eis_image_size_config.InputWidth   = mEisInput_W;
            mEisAlgoProcData.eis_image_size_config.InputHeight  = mEisInput_H;
            mEisAlgoProcData.eis_image_size_config.TargetWidth  = mP1Target_W;
            mEisAlgoProcData.eis_image_size_config.TargetHeight = mP1Target_H;
        }

        if(g_debugDump_R >= 1)
        {
            EIS_LOG_R("EisIn(%u,%u),P1T(%u,%u)",mEisInput_W,mEisInput_H,mP1Target_W,mP1Target_H);
        }

        //> get EIS HW statistic

        if(EIS_RETURN_EISO_MISS == m_pEisDrv->GetEisHwStatistic(&mEisAlgoProcData.eis_state,aTimeStamp))
        {
            EIS_WRN_R("EISO data miss");
            return EIS_RETURN_NO_ERROR;
        }

        if(g_debugDump_R == 3)
        {
            DumpStatistic(mEisAlgoProcData.eis_state);
        }

            //> get EIS HW setting of eis_op_vert and eis_op_hori
        mEisAlgoProcData.DivH = m_pEisDrv->GetEisDivH();
        mEisAlgoProcData.DivV = m_pEisDrv->GetEisDivV();

        //> get MB number
        mEisAlgoProcData.EisWinNum = m_pEisDrv->GetEisMbNum();

        //> get Acc & Gyro info
        mEisAlgoProcData.sensor_info.GyroValid = 0;
        mEisAlgoProcData.sensor_info.Gvalid    = 0;

        //====== EIS Algorithm ======

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO, &mEisAlgoProcData, NULL);
        if(err != S_EIS_OK)
        {
            EIS_ERR_R("EisAlg:EIS_FEATURE_SET_PROC_INFO fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        err = m_pEisAlg->EisMain(&eisCMVResult);
        if(err != S_EIS_OK)
        {
            EIS_ERR_R("EisAlg:EisMain fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        if(g_debugDump_R >= 1)
        {
            EIS_LOG_R("eisCMVResult: %d %d",eisCMVResult.CMV_X,eisCMVResult.CMV_Y);
        }
        //====== Get GMV ======

        EIS_GMV_INFO_STRUCT eisGMVResult;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV, NULL, &eisGMVResult);
        if(err != S_EIS_OK)
        {
            EIS_ERR_R("EisAlg:EIS_FEATURE_GET_ORI_GMV fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        //====== Save EIS CMV and GMV =======

        if(m_pEisDrv->Get2PixelMode())
        {
            if(g_debugDump_R > 0)
            {
                EIS_LOG_R("eisGMVResult.EIS_GMVx *= 2");
            }

            eisGMVResult.EIS_GMVx *= 2;
        }

        mGMV_X = eisGMVResult.EIS_GMVx;
        mGMV_Y = eisGMVResult.EIS_GMVy;
        if(g_debugDump_R >= 1)
        {
            EIS_LOG_R("GMV: (%d  %d)", mGMV_X, mGMV_Y);
        }

        //====== Prepare EIS Result ======

        PrepareEisResult(eisCMVResult.CMV_X,eisCMVResult.CMV_Y);

        //====== Get First Frame Info ======

        mFrameCnt = m_pEisDrv->GetFirstFrameInfo();

        if(g_debugDump_R >= 1)
        {
            EIS_LOG_R("mFrameCnt(%u)",mFrameCnt);
        }

        //====== Not The First Frame ======

        if(mFrameCnt == 0)
        {
            EIS_LOG_R("not first frame");
            mFrameCnt = 1;

            // move to EIS_DRV to handle this
        #if 0
            err = m_pEisDrv->SetFirstFrame(0);
            if(err != EIS_RETURN_NO_ERROR)
            {
                EIS_ERR_R("set first frame fail(0x%08x)",err);
            }
            else
            {
                 mFrameCnt = 1;
            }
        #endif
        }

        //====== Dynamic Debug ======

        if(g_debugDump_R >= 1 && mFrameCnt < DEBUG_DUMP_FRAMW_NUM)
        {
            if(g_debugDump_R == 3)
            {
                m_pEisDrv->DumpReg();
            }

            if(mEisPass1Only == 1)
            {
                ++mFrameCnt;
            }
        }
    }

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("-");
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::EisThreadStop()
{
    m_pEisDrv->SetEisoThreadState(EIS_SW_STATE_UNINIT);
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::PrepareEisResult(const MINT32 &cmvX, const MINT32 &cmvY)
{
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("cmvX(%d),cmvY(%d)",cmvX,cmvY);
    }

    Mutex::Autolock lock(mP1Lock);

    //====== Boundary Checking ======

    if(cmvX < 0)
    {
        EIS_ERR_R("cmvX should not be negative(%u), fix to 0",cmvX);

        mCmvX_Int = mCmvX_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_X = cmvX / 256.0;
        MINT32 tempFinalCmvX = cmvX;
        if((tempCMV_X + (MFLOAT)mP1Target_W) > (MFLOAT)mEisInput_W)
        {
            EIS_LOG_R("cmvX too large(%u), fix to %u",cmvX,(mEisInput_W - mP1Target_W));

            tempFinalCmvX = (mEisInput_W - mP1Target_W);
        }

        if(m_pEisDrv->Get2PixelMode())
        {
            if(g_debugDump_R > 0)
            {
                EIS_LOG_R("tempFinalCmvX *= 2");
            }

            tempFinalCmvX *= 2;
        }

        mCmvX_Int = (tempFinalCmvX & (~0xFF)) >> intPartShift;
        mCmvX_Flt = (tempFinalCmvX & (0xFF)) << floatPartShift;
    }

    if(cmvY < 0)
    {
        EIS_ERR_R("cmvY should not be negative(%u), fix to 0",cmvY);

        mCmvY_Int = mCmvY_Flt = 0;
    }
    else
    {
        MFLOAT tempCMV_Y = cmvY / 256.0;
        MINT32 tempFinalCmvY = cmvY;
        if((tempCMV_Y + (MFLOAT)mP1Target_H) > (MFLOAT)mEisInput_H)
        {
            EIS_LOG_R("cmvY too large(%u), fix to %u",cmvY,(mEisInput_H - mP1Target_H));

            tempFinalCmvY = (mEisInput_H - mP1Target_H);
        }

        mCmvY_Int = (tempFinalCmvY & (~0xFF)) >> intPartShift;
        mCmvY_Flt = (tempFinalCmvY & (0xFF)) << floatPartShift;
    }
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("X(%u,%u),Y(%u,%u)",mCmvX_Int,mCmvX_Flt,mCmvY_Int,mCmvY_Flt);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisResult(MUINT32 &a_CMV_X_Int,
                                 MUINT32 &a_CMV_X_Flt,
                                 MUINT32 &a_CMV_Y_Int,
                                 MUINT32 &a_CMV_Y_Flt,
                                 MUINT32 &a_TarWidth,
                                 MUINT32 &a_TarHeight)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG_R("mSensorIdx(%u) not support EIS",mSensorIdx);
        a_CMV_X_Int = 0;
        a_CMV_X_Flt = 0;
        a_CMV_Y_Int = 0;
        a_CMV_Y_Flt = 0;
        a_TarWidth  = 0;
        a_TarHeight = 0;
        return;
    }

    {
        Mutex::Autolock lock(mP1Lock);

        a_CMV_X_Int = mCmvX_Int;
        a_CMV_X_Flt = mCmvX_Flt;
        a_CMV_Y_Int = mCmvY_Int;
        a_CMV_Y_Flt = mCmvY_Flt;
        a_TarWidth  = mP1Target_W;
        a_TarHeight = mP1Target_H;
    }

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("X(%u,%u),Y(%u,%u)",a_CMV_X_Int,a_CMV_X_Flt,a_CMV_Y_Int,a_CMV_Y_Flt);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisGmv(MINT32 &aGMV_X, MINT32 &aGMV_Y, MUINT32 *aEisInW, MUINT32 *aEisInH)
{
    if(mEisSupport == MFALSE)
    {
        EIS_LOG_R("mSensorIdx(%u) not support EIS",mSensorIdx);
        return;
    }

    aGMV_X = mGMV_X;
    aGMV_Y = mGMV_Y;

    {
        Mutex::Autolock lock(mP1Lock);

        if(aEisInW != NULL)
        {
            *aEisInW = mEisInput_W;
            if(g_debugDump_R >= 1)
            {
                EIS_LOG_R("aEisInW(%u)",*aEisInW);
            }
        }

        if(aEisInH != NULL)
        {
            *aEisInH = mEisInput_H;
            if(g_debugDump_R >= 1)
            {
                EIS_LOG_R("aEisInH(%u)",*aEisInH);
            }
        }
    }

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("GMV(%d,%d)",aGMV_X,aGMV_Y);
    }
}

/*******************************************************************************
*
********************************************************************************/
MBOOL EisHalImp::GetEisSupportInfo(const MUINT32 &aSensorIdx)
{
    mEisSupport = m_pEisDrv->GetEisSupportInfo(aSensorIdx);
    return mEisSupport;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::SendCommand(EIS_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    switch(aCmd)
    {
        case EIS_CMD_SET_STATE:
            m_pEisDrv->SetEisoThreadState((EIS_SW_STATE_ENUM)arg1);
            break;
        case EIS_CMD_CHECK_STATE:
            m_pEisDrv->SetEisoThreadState((EIS_SW_STATE_ENUM)arg1);
            break;
        default:
            EIS_ERR_R("wrong cmd(%d)",aCmd);
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat)
{
    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        a_pEIS_Stat->i4LMV_X[i]    = mEisAlgoProcData.eis_state.i4LMV_X[i];
        a_pEIS_Stat->i4LMV_Y[i]    = mEisAlgoProcData.eis_state.i4LMV_Y[i];
        a_pEIS_Stat->i4LMV_X2[i]   = mEisAlgoProcData.eis_state.i4LMV_X2[i];
        a_pEIS_Stat->i4LMV_Y2[i]   = mEisAlgoProcData.eis_state.i4LMV_Y2[i];
        a_pEIS_Stat->NewTrust_X[i] = mEisAlgoProcData.eis_state.NewTrust_X[i];
        a_pEIS_Stat->NewTrust_Y[i] = mEisAlgoProcData.eis_state.NewTrust_Y[i];
        a_pEIS_Stat->SAD[i]        = mEisAlgoProcData.eis_state.SAD[i];
        a_pEIS_Stat->SAD2[i]       = mEisAlgoProcData.eis_state.SAD2[i];
        a_pEIS_Stat->AVG_SAD[i]    = mEisAlgoProcData.eis_state.AVG_SAD[i];
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut)
{
    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("+");
    }

    EIS_Customize_Para_t customSetting;

    get_EIS_CustomizeData(&customSetting);

    a_pDataOut->sensitivity         = (EIS_SENSITIVITY_ENUM)customSetting.sensitivity;
    a_pDataOut->filter_small_motion = customSetting.filter_small_motion;
    a_pDataOut->adv_shake_ext          = customSetting.adv_shake_ext;  // 0 or 1
    a_pDataOut->stabilization_strength = customSetting.stabilization_strength;  // 0.5~0.95

    a_pDataOut->advtuning_data.new_tru_th        = customSetting.new_tru_th;         // 0~100
    a_pDataOut->advtuning_data.vot_th            = customSetting.vot_th;             // 1~16
    a_pDataOut->advtuning_data.votb_enlarge_size = customSetting.votb_enlarge_size;  // 0~1280
    a_pDataOut->advtuning_data.min_s_th          = customSetting.min_s_th;           // 10~100
    a_pDataOut->advtuning_data.vec_th            = customSetting.vec_th;             // 0~11   should be even
    a_pDataOut->advtuning_data.spr_offset        = customSetting.spr_offset;         //0 ~ MarginX/2
    a_pDataOut->advtuning_data.spr_gain1         = customSetting.spr_gain1;          // 0~127
    a_pDataOut->advtuning_data.spr_gain2         = customSetting.spr_gain2;          // 0~127

    a_pDataOut->advtuning_data.gmv_pan_array[0] = customSetting.gmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[1] = customSetting.gmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[2] = customSetting.gmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.gmv_pan_array[3] = customSetting.gmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.gmv_sm_array[0] = customSetting.gmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[1] = customSetting.gmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[2] = customSetting.gmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.gmv_sm_array[3] = customSetting.gmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.cmv_pan_array[0] = customSetting.cmv_pan_array[0];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[1] = customSetting.cmv_pan_array[1];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[2] = customSetting.cmv_pan_array[2];   //0~5
    a_pDataOut->advtuning_data.cmv_pan_array[3] = customSetting.cmv_pan_array[3];   //0~5

    a_pDataOut->advtuning_data.cmv_sm_array[0] = customSetting.cmv_sm_array[0];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[1] = customSetting.cmv_sm_array[1];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[2] = customSetting.cmv_sm_array[2];    //0~5
    a_pDataOut->advtuning_data.cmv_sm_array[3] = customSetting.cmv_sm_array[3];    //0~5

    a_pDataOut->advtuning_data.vot_his_method  = (EIS_VOTE_METHOD_ENUM)customSetting.vot_his_method; //0 or 1
    a_pDataOut->advtuning_data.smooth_his_step = customSetting.smooth_his_step; // 2~6

    a_pDataOut->advtuning_data.eis_debug = customSetting.eis_debug;

    if(g_debugDump_R >= 1)
    {
        EIS_LOG_R("-");
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHalImp::DumpStatistic(const EIS_STATISTIC_STRUCT &aEisStat)
{
    EIS_LOG_R("+");

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,(LMV_X,LMV_Y)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X[i],aEisStat.i4LMV_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,(LMV_X2,LMV_Y2)=(%d,%d)",(i/4),(i%4),aEisStat.i4LMV_X2[i],aEisStat.i4LMV_Y2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,MinSAD(%u)",(i/4),(i%4),aEisStat.SAD[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,(NewTrust_X,NewTrust_Y)=(%u,%u)",(i/4),(i%4),aEisStat.NewTrust_X[i],aEisStat.NewTrust_Y[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,MinSAD2(%u)",(i/4),(i%4),aEisStat.SAD2[i]);
    }

    for(MUINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_INF_R("MB%d%d,AvgSAD(%u)",(i/4),(i%4),aEisStat.AVG_SAD[i]);
    }

    EIS_LOG_R("-");
}

