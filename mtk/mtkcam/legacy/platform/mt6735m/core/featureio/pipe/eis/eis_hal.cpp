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

//! \file  eis_hal.cpp
#define MTK_LOG_ENABLE 1
#include <stdlib.h>
#include <stdio.h>
#include <utils/threads.h>
#include <cutils/log.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include <mtkcam/Log.h>
#include "eis_drv_base.h"
#include <mtkcam/drv_common/imem_drv.h>
#include "eis_hal.h"
#include "camera_custom_eis.h"

/*******************************************************************************
*
********************************************************************************/
#define EIS_DEBUG

#ifdef EIS_DEBUG

#undef __func__
#define __func__ __FUNCTION__

#define LOG_TAG "EISHal"
#define EIS_LOG(fmt, arg...)    CAM_LOGD("[%s]" fmt, __func__, ##arg)
#define EIS_WRN(fmt, arg...)    CAM_LOGW("[%s] WRN(%5d):" fmt, __func__, __LINE__, ##arg)
#define EIS_ERR(fmt, arg...)    CAM_LOGE("[%s] %s ERR(%5d):" fmt, __func__,__FILE__, __LINE__, ##arg)

#else
#define EIS_LOG(a,...)
#define EIS_ERR(a,...)
#endif

#define intPartShift 8
#define floatPartShift (31 - intPartShift)
#define L1_CACHE_BYTES 32
/*******************************************************************************
*
********************************************************************************/
static MINT32 g_debugDump = 0;
static MFLOAT gAccInfo[3] = {0};
static MFLOAT gGyroInfo[3] = {0};

/*******************************************************************************
*
********************************************************************************/
void mySensorListener(ASensorEvent event)
{
    static MUINT32 accCnt = 1;
    static MUINT32 gyroCnt = 1;

    switch(event.type)
    {
        case ASENSOR_TYPE_ACCELEROMETER:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("[%u] Acc(%f,%f,%f,%lld)",
                        accCnt++,
                        event.acceleration.x,
                        event.acceleration.y,
                        event.acceleration.z,
                        event.timestamp);
            }

            gAccInfo[0] = event.acceleration.x;
            gAccInfo[1] = event.acceleration.y;
            gAccInfo[2] = event.acceleration.z;

            break;
        }
        case ASENSOR_TYPE_MAGNETIC_FIELD:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Mag");
            }
            break;
        }
        case ASENSOR_TYPE_GYROSCOPE:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("[%u] Gyro(%f,%f,%f,%lld)",
                        gyroCnt++,
                        event.vector.x,
                        event.vector.y,
                        event.vector.z,
                        event.timestamp);
            }

            gGyroInfo[0] = event.vector.x;
            gGyroInfo[1] = event.vector.y;
            gGyroInfo[2] = event.vector.z;

            break;
        }
        case ASENSOR_TYPE_LIGHT:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Light");
            }
            break;
        }
        case ASENSOR_TYPE_PROXIMITY:
        {
            if(g_debugDump >= 2)
            {
                EIS_LOG("Proxi");
            }
            break;
        }
        default:
        {
            EIS_WRN("unknown type(%d)",event.type);
            break;
        }
    }

}


/*******************************************************************************
*
********************************************************************************/
EisHalBase *EisHalBase::createInstance(char const *userName)
{
    EIS_LOG("%s",userName);
    return EisHal::getInstance();
}

/*******************************************************************************
*
********************************************************************************/
EisHalBase *EisHal::getInstance()
{
    EIS_LOG("+");
    static EisHal singleton;

    if(singleton.init() != 0)
    {
        EIS_LOG("singleton.init() fail ");
        return NULL;
    }

    EIS_LOG("-");
    return &singleton;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::destroyInstance(char const *userName)
{
    EIS_LOG("%s",userName);
    uninit();
}

/*******************************************************************************
*
********************************************************************************/
EisHal::EisHal() : EisHalBase()
{
    mUsers = 0;
    m_pEisDrv = NULL;
    m_pEisAlg = NULL;
    mInput_W = 0;
    mInput_H = 0;
    mTarget_W = 0;
    mTarget_H = 0;
    mCMV_X = 0;
    mCMV_Y = 0;
    mGMV_X = 0;
    mGMV_Y = 0;
    mFirstFlag = 0; // first frame
    mConfigPass = MFALSE;

    mpSensorListener = NULL;
    //IMEM
    m_pIMemDrv = NULL;
    mEisAlgoIMemBuf.memID = -5;
    mEisAlgoIMemBuf.virtAddr = mEisAlgoIMemBuf.phyAddr = mEisAlgoIMemBuf.size = 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHal::init()
{
    EIS_LOG("mUsers(%d)", mUsers);

    //====== Check Reference Count ======

    Mutex::Autolock lock(mLock);

    if(mUsers > 0)
    {
        EIS_LOG("%d has created", mUsers);
        android_atomic_inc(&mUsers);
        return EIS_RETURN_NO_ERROR;
    }

    //====== Dynamic Debug ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

    //====== Create EIS Driver ======

    m_pEisDrv = EisDrvBase::createInstance();

    if(m_pEisDrv == NULL)
    {
        EIS_ERR("EisDrv::createInstance fail");
        goto create_fail_exit;
    }

    //====== Create EIS Algorithm Object ======

    m_pEisAlg = MTKEis::createInstance();

    if(m_pEisAlg == NULL)
    {
        EIS_ERR("EisAlg::createInstance fail");
        goto create_fail_exit;
    }

    if(g_debugDump >= 2)
    {
        //====== EisAlgo Debug Buffer ======

        m_pIMemDrv = IMemDrv::createInstance();
        if(m_pIMemDrv == NULL)
        {
            EIS_ERR("Null IMemDrv Obj");
            return EIS_RETURN_NULL_OBJ;
        }

        //MUINT32 eisMemSize = EIS_LOG_BUFFER_SIZE;
        MUINT32 eisMemSize = 0x1000;

        createMemBuf(eisMemSize,1,&mEisAlgoIMemBuf);
        if(mEisAlgoIMemBuf.virtAddr == 0 && mEisAlgoIMemBuf.phyAddr == 0)
        {
            EIS_ERR("create IMem fail");
            return EIS_RETURN_MEMORY_ERROR;
        }

        EIS_LOG("mEisAlgoIMemBuf : memID(%d),size(%u),virAdd(0x%x),phyAddr(0x%x)",mEisAlgoIMemBuf.memID,
                                                                                  mEisAlgoIMemBuf.size,
                                                                                  mEisAlgoIMemBuf.virtAddr,
                                                                                  mEisAlgoIMemBuf.phyAddr);
    }

    mpSensorListener = SensorListener::createInstance();

    if(MTRUE != mpSensorListener->setListener(mySensorListener))
    {
        EIS_ERR("setListener fail");
    }
    else
    {
        EIS_LOG("setListener success");
    }

    mAccEnable  = mpSensorListener->enableSensor(SensorListener::SensorType_Acc, 33);
    mGyroEnable = mpSensorListener->enableSensor(SensorListener::SensorType_Gyro, 33);
    EIS_LOG("EN:(Acc,Gyro)=(%d,%d)",mAccEnable,mGyroEnable);



    android_atomic_inc(&mUsers);

    EIS_LOG("-");
    return EIS_RETURN_NO_ERROR;

create_fail_exit:

    if(m_pEisDrv != NULL)
    {
        m_pEisDrv->destroyInstance();
        m_pEisDrv = NULL;
    }

    if(m_pEisAlg != NULL)
    {
        m_pEisAlg->destroyInstance();
        m_pEisAlg = NULL;
    }

    EIS_LOG("-");
    return EIS_RETURN_INVALID_DRIVER;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHal::uninit()
{
    EIS_LOG("mUsers(%d)", mUsers);

    Mutex::Autolock lock(mLock);

    //====== Check Reference Count ======

    if(mUsers <= 0)
    {
        EIS_LOG("There is no more user");
        return EIS_RETURN_NO_ERROR;
    }

    //====== Uninitialize ======

    android_atomic_dec(&mUsers);    //decrease referebce count

    if(mUsers == 0)    // there is no user
    {
        MINT32 err = EIS_RETURN_NO_ERROR;

        //====== Dynamic Debug ======

        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.eis.dump", value, "0");
        g_debugDump = atoi(value);

        if(g_debugDump >= 2)
        {
            EIS_SET_LOG_BUFFER_STRUCT pEisAlgoLogInfo;

            //pEisAlgoLogInfo.Eis_Log_Buf_Addr = mEisAlgoIMemBuf.virtAddr;
            //pEisAlgoLogInfo.Eis_Log_Buf_Size = mEisAlgoIMemBuf.size;

            err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SAVE_LOG, &pEisAlgoLogInfo, NULL);
            if(err != S_EIS_OK)
            {
                EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SAVE_LOG) fail(0x%x)",err);
            }
        }

        m_pEisDrv->enableEIS(0);

        if(m_pEisDrv != NULL)
        {
            m_pEisDrv->destroyInstance();
            m_pEisDrv = NULL;
        }

        if(m_pEisAlg != NULL)
        {
            m_pEisAlg->destroyInstance();
            m_pEisAlg = NULL;
        }

        // Next-Gen EIS
        if(mpSensorListener != NULL)
        {
            EIS_LOG("mpSensorListener uninit");
            mpSensorListener->disableSensor(SensorListener::SensorType_Acc);
            mpSensorListener->disableSensor(SensorListener::SensorType_Gyro);
            mpSensorListener->destroyInstance();
            mpSensorListener = NULL;
        }

        if(g_debugDump >= 2)
        {
            //====== Free Memory ======

            destroyMemBuf(1,&mEisAlgoIMemBuf);

            mEisAlgoIMemBuf.memID = -5;
            mEisAlgoIMemBuf.virtAddr = mEisAlgoIMemBuf.phyAddr = mEisAlgoIMemBuf.size = 0;

            if(m_pIMemDrv != NULL)
            {
                m_pIMemDrv->destroyInstance();
                m_pIMemDrv = NULL;
            }
        }

        mFirstFlag = 0; // first frmae
        mConfigPass = MFALSE;
    }
    else
    {
        EIS_LOG("Still %d users", mUsers);
    }

    EIS_LOG("-");
    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHal::createMemBuf(MUINT32 &memSize, MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    MINT32 err = EIS_RETURN_NO_ERROR;
    MUINT32 alingSize = (memSize + L1_CACHE_BYTES - 1) & ~(L1_CACHE_BYTES - 1);

    EIS_LOG("Cnt(%d),Size(%u),alingSize(%u)",bufCnt, memSize, alingSize);

    memSize = alingSize;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            bufInfo[i].size = alingSize;

            if(m_pIMemDrv->allocVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->allocVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if(m_pIMemDrv->mapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->mapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        bufInfo->size = alingSize;

        if(m_pIMemDrv->allocVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->allocVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }

        if(m_pIMemDrv->mapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->mapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("X");
    return err;
}

/******************************************************************************
*
*******************************************************************************/
MINT32 EisHal::destroyMemBuf(MUINT32 bufCnt, IMEM_BUF_INFO *bufInfo)
{
    EIS_LOG("Cnt(%d)", bufCnt);

    MINT32 err = EIS_RETURN_NO_ERROR;

    if(bufCnt > 1)  // more than one
    {
        for(MUINT32 i = 0; i < bufCnt; ++i)
        {
            if(0 == bufInfo[i].virtAddr)
            {
                EIS_LOG("Buffer doesn't exist, i(%d)",i);
                continue;
            }

            if(m_pIMemDrv->unmapPhyAddr(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->unmapPhyAddr() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }

            if (m_pIMemDrv->freeVirtBuf(&bufInfo[i]) < 0)
            {
                EIS_ERR("m_pIMemDrv->freeVirtBuf() error, i(%d)",i);
                err = EIS_RETURN_API_FAIL;
            }
        }
    }
    else
    {
        if(0 == bufInfo->virtAddr)
        {
            EIS_LOG("Buffer doesn't exist");
        }

        if(m_pIMemDrv->unmapPhyAddr(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->unmapPhyAddr() error");
            err = EIS_RETURN_API_FAIL;
        }

        if (m_pIMemDrv->freeVirtBuf(bufInfo) < 0)
        {
            EIS_ERR("m_pIMemDrv->freeVirtBuf() error");
            err = EIS_RETURN_API_FAIL;
        }
    }

    EIS_LOG("X");
    return err;
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
MUINT32 gDivV,gDivH,gEisWinNum;
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



void EisGetProcInfo(P_EIS_CONFIG_IMAGE_INFO_STRUCT pEisImgInfo,P_EIS_GET_PROC_INFO_STRUCT pGetProcInfo)
{
    MUINT32 ImgWidth, ImgHeight;

    MUINT32 EIS_DC_DL, EIS_H_SHR, EIS_V_SHR, Dead_Left, Dead_Upper;
    MUINT32 EIS_RP_H_OFFSET, EIS_RP_V_OFFSET;
    MUINT32 First_Win_Left_Corner, First_Win_Top_Corner;
    MUINT32 Active_H_Size, Active_V_Size;
    MUINT32 EIS_H_WIN_SIZE, EIS_V_WIN_SIZE;
    MUINT32 Best_Eis_Num_H_Rp, Best_Eis_Num_V_Rp;

    ImgWidth = pEisImgInfo->InputWidth;
    ImgHeight = pEisImgInfo->InputHeight;


    if(ImgWidth > HD_720P_WIDTH && ImgHeight > HD_720P_HEIGHT)   // if > 720P, set downsample = 2
    {
        gDivV = 2;
        gDivH = 2;
        pGetProcInfo->IIR_DS = 2;
    }
    else
    {
        gDivV = 1;
        gDivH = 1;
        pGetProcInfo->IIR_DS = 1;
    }

    if(ImgWidth*ImgHeight < 2000000)
    {
        pGetProcInfo->pathCDRZ = 0; //before CRZ
    }
    else
    {
        pGetProcInfo->pathCDRZ = 1; //after CRZ
    }

    if(ImgWidth > D1_WIDTH && ImgHeight > D1_HEIGHT)
    {
        pGetProcInfo->MBNum_H = 4;
        pGetProcInfo->MBNum_V = 8;
    }
    else if(ImgWidth > CIF_WIDTH && ImgHeight > CIF_HEIGHT)
    {
        pGetProcInfo->MBNum_H = 4;
        pGetProcInfo->MBNum_V = 4;
    }
    else
    {
        pGetProcInfo->MBNum_H = 2;
        pGetProcInfo->MBNum_V = 3;
    }

    gEisWinNum = pGetProcInfo->MBNum_H * pGetProcInfo->MBNum_V;

    pGetProcInfo->AD_Knee = 4; //fixed

    pGetProcInfo->AD_Clip = 6; //fixed
    pGetProcInfo->Gain_H = 0;  //fixed (1x)

    if(pGetProcInfo->IIR_DS == 1 )
    {
       pGetProcInfo->IIR_Gain_H = 3;
        pGetProcInfo->IIR_Gain_V = 3;
    }
    else
    {
        pGetProcInfo->IIR_Gain_H = 4;
       pGetProcInfo->IIR_Gain_V = 4;
    }


    pGetProcInfo->LMV_TH_X_Cent = 0; //no used
    pGetProcInfo->LMV_TH_X_Surrd = 0; //no used
    pGetProcInfo->LMV_TH_Y_Cent = 0; //no used
    pGetProcInfo->LMV_TH_Y_Surrd = 0; //no used

    pGetProcInfo->FL_Offset_H = 0; //fixed
    pGetProcInfo->FL_Offset_V = 0; //fixed

//====================Calculate RP window number and RP points==========================//
//Decide EIS_DL
    if(ImgWidth > CIF_WIDTH && ImgHeight > CIF_HEIGHT)
        EIS_DC_DL = 32;
    else
        EIS_DC_DL = 16;


    pGetProcInfo->FIR_Gain_H = EIS_DC_DL;



//Decide EIS_H_SHR & EIS_V_SHR
    if(pGetProcInfo->IIR_DS == 1)
    {
        EIS_H_SHR = 3;
        EIS_V_SHR = 3;
    }
    else
    {
        EIS_H_SHR = 4;
        EIS_V_SHR = 4;
    }
//Decide Dead_left & Dead_upper
    Dead_Left =(MINT32) (1 <<EIS_H_SHR) * 2 + EIS_DC_DL;
    Dead_Upper =(MINT32) (1<< EIS_V_SHR) * 2;


//Decide EIS_RP_H_Offset & EIS_RP_V_Offset
    EIS_RP_H_OFFSET = (Dead_Left + 16*pGetProcInfo->IIR_DS + 8)/pGetProcInfo->IIR_DS;
    EIS_RP_V_OFFSET = (Dead_Upper+ 16*pGetProcInfo->IIR_DS + 8)/pGetProcInfo->IIR_DS;

//Decide First_Win_Left_Corner, First_Win_Top_Corner
    First_Win_Left_Corner = (EIS_RP_H_OFFSET - 16)*pGetProcInfo->IIR_DS;
    First_Win_Top_Corner = (EIS_RP_V_OFFSET - 16)*pGetProcInfo->IIR_DS;

//Decide Active_H_Size, Active_V_Size
    Active_H_Size = ImgWidth - First_Win_Left_Corner - 16;
    Active_V_Size = ImgHeight - First_Win_Top_Corner - 8;


    pGetProcInfo->MB_Offset_H = EIS_RP_H_OFFSET;
    pGetProcInfo->MB_Offset_V = EIS_RP_V_OFFSET;
    pGetProcInfo->MB_Intv_H = Active_H_Size / pGetProcInfo->MBNum_H / (pGetProcInfo->IIR_DS);
    pGetProcInfo->MB_Intv_V = Active_V_Size / pGetProcInfo->MBNum_V / (pGetProcInfo->IIR_DS);

//Decide Best_Eis_Num_H_Rp, Best_Eis_Num_V_Rp;
    Best_Eis_Num_H_Rp = (pGetProcInfo->MB_Intv_H-1)/16 -1;
    Best_Eis_Num_V_Rp = (pGetProcInfo->MB_Intv_V-1)/16 -1;

    if(Best_Eis_Num_H_Rp > 16)
        pGetProcInfo->RPNum_H = 16;
    else
        pGetProcInfo->RPNum_H = Best_Eis_Num_H_Rp;

    if(pGetProcInfo->MBNum_V<5 && Best_Eis_Num_V_Rp>8)
        pGetProcInfo->RPNum_V = 8;
    else if(pGetProcInfo->MBNum_V>4 && Best_Eis_Num_V_Rp>4)
        pGetProcInfo->RPNum_V = 4;
    else
        pGetProcInfo->RPNum_V = Best_Eis_Num_V_Rp;

}
#endif


/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::configEIS(NSHwScenario::EhwMode a_ehwMode, eisHal_config_t a_sEisConfig)
{
    EIS_LOG("HW_Sce(%d)",(MINT32)a_ehwMode);

    MINT32 err = 0;

    EIS_GET_PROC_INFO_STRUCT  eisHWSetting;
    EIS_SET_ENV_INFO_STRUCT   eisAlgoInit;
    EIS_CONFIG_IMAGE_INFO_STRUCT eisImageSize;

    //====== Member Variable Setting ======

    mConfigPass = MFALSE;

    //====== Dynamic Debug ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

    //====== EIS Register Setting ======

    //member variable setting
    mInput_W = a_sEisConfig.imageWidth;
    mInput_H = a_sEisConfig.imageHeight;

    mTarget_W = (mInput_W / (EIS_FACTOR / 100.0));
    mTarget_H = (mInput_H / (EIS_FACTOR / 100.0));

    EIS_LOG("mInput_W(%u),mInput_H(%u),mTarget_W(%u),mTarget_H(%u)",mInput_W,mInput_H,mTarget_W,mTarget_H);

    if(a_ehwMode == NSHwScenario::eHW_VSS)
    {
        eisAlgoInit.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;

        //RAW domain only setting
        m_pEisDrv->setEIS_DB_SEL(1);
        m_pEisDrv->setEISRawSel(1);
    }
    else if(a_ehwMode == NSHwScenario::eHW_ZSD)
    {
        eisAlgoInit.Eis_Input_Path = EIS_PATH_YUV_DOMAIN;

        //YUV domain only setting
        m_pEisDrv->setEIS_DB_SEL(0);
        m_pEisDrv->setEISRawSel(0);
    }
    else
    {
        EIS_LOG("not support right now, use VSS");

        eisAlgoInit.Eis_Input_Path = EIS_PATH_RAW_DOMAIN;

        //RAW domain only setting
        m_pEisDrv->setEIS_DB_SEL(1);
        m_pEisDrv->setEISRawSel(1);
    }

    // get customize setting
    getEISCustomize(&eisAlgoInit.eis_tuning_data);

    // set image size info
    eisImageSize.InputWidth   = mInput_W - 4;   //@ryan wang: sync eis parameters in middleware and algorithm to avoid "wrong boundry" issue
    eisImageSize.InputHeight  = mInput_H - 4;   //@ryan wang: sync eis parameters in middleware and algorithm to avoid "wrong boundry" issue
    eisImageSize.TargetWidth  = mTarget_W;
    eisImageSize.TargetHeight = mTarget_H;

    // init EIS algo
    err = m_pEisAlg->EisInit(&eisAlgoInit);
    if(err != S_EIS_OK)
    {
        EIS_ERR("EisInit fail(0x%x)",err);
        return;
    }

    if(g_debugDump >= 2)
    {

        MUINT32 eisMemSize = 0;
        MINT32 err = EIS_RETURN_NO_ERROR;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_DEBUG_INFO,NULL,&eisMemSize);
        if(err != S_EIS_OK || eisMemSize == 0)
        {
            EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
        }
        else
        {
            createMemBuf(eisMemSize,1,&mEisAlgoIMemBuf);
            if(mEisAlgoIMemBuf.virtAddr == 0 && mEisAlgoIMemBuf.phyAddr == 0)
            {
                EIS_ERR("create IMem fail");

            }

            EIS_LOG("mEisAlgoIMemBuf : memID(%d),size(%u),virAdd(0x%x),phyAddr(0x%x)",mEisAlgoIMemBuf.memID,
                                                                                      mEisAlgoIMemBuf.size,
                                                                                      mEisAlgoIMemBuf.virtAddr,
                                                                                      mEisAlgoIMemBuf.phyAddr);
        }

        EIS_SET_LOG_BUFFER_STRUCT eisAlgoLogInfo;

        eisAlgoLogInfo.Eis_Log_Buf_Addr = (MVOID *)mEisAlgoIMemBuf.virtAddr;
        eisAlgoLogInfo.Eis_Log_Buf_Size = mEisAlgoIMemBuf.size;

        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO, &eisAlgoLogInfo, NULL);
        if(err != S_EIS_OK)
        {
            EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SET_DEBUG_INFO) fail(0x%x)",err);
        }
    }


    // query setting from EIS algorithm
    //err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_PROC_INFO, &eisImageSize, &eisHWSetting);
    EisGetProcInfo(&eisImageSize, &eisHWSetting);

    if(g_debugDump >= 1)
    {
        EIS_LOG("Algo eis_tuning_data");
        EIS_LOG("sensitivity(%d)",eisAlgoInit.eis_tuning_data.sensitivity);
        EIS_LOG("filter_small_motion(%u)",eisAlgoInit.eis_tuning_data.filter_small_motion);
        EIS_LOG("advtuning_data.new_tru_th(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.new_tru_th);
        EIS_LOG("advtuning_data.vot_th(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.vot_th);
        EIS_LOG("advtuning_data.votb_enlarge_size(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.votb_enlarge_size);
        EIS_LOG("advtuning_data.min_s_th(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.min_s_th);
        EIS_LOG("advtuning_data.vec_th(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.vec_th);
        EIS_LOG("advtuning_data.spr_offset(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.spr_offset);
        EIS_LOG("advtuning_data.spr_gain1(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.spr_gain1);
        EIS_LOG("advtuning_data.spr_gain2(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.spr_gain2);
        EIS_LOG("advtuning_data.vot_his_method(%d)",eisAlgoInit.eis_tuning_data.advtuning_data.vot_his_method);
        EIS_LOG("advtuning_data.smooth_his_step(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.smooth_his_step);
        EIS_LOG("advtuning_data.eis_debug(%u)",eisAlgoInit.eis_tuning_data.advtuning_data.eis_debug);

        for(MINT32 i = 0; i < 4; ++i)
        {
            EIS_LOG("gmv_pan_array[%d]=%u",i,eisAlgoInit.eis_tuning_data.advtuning_data.gmv_pan_array[i]);
            EIS_LOG("gmv_sm_array[%d]=%u",i,eisAlgoInit.eis_tuning_data.advtuning_data.gmv_sm_array[i]);
            EIS_LOG("cmv_pan_array[%d]=%u",i,eisAlgoInit.eis_tuning_data.advtuning_data.cmv_pan_array[i]);
            EIS_LOG("cmv_sm_array[%d]=%u",i,eisAlgoInit.eis_tuning_data.advtuning_data.cmv_sm_array[i]);
        }

        EIS_LOG("Algo eisHWSetting");
        EIS_LOG("pathCDRZ(0x%x)",eisHWSetting.pathCDRZ);
        EIS_LOG("IIR_DS(0x%x)",eisHWSetting.IIR_DS);
        EIS_LOG("MBNum_H(0x%x),MBNum_V(0x%x)",eisHWSetting.MBNum_H, eisHWSetting.MBNum_V);
        EIS_LOG("RPNum_H(0x%x),RPNum_V(0x%x)",eisHWSetting.RPNum_H,eisHWSetting.RPNum_V);
        EIS_LOG("AD_Knee(0x%x),AD_Clip(0x%x)",eisHWSetting.AD_Knee, eisHWSetting.AD_Clip);
        EIS_LOG("Gain_H(0x%x),IIR_Gain_H(0x%x),FIR_Gain_H(0x%x)",eisHWSetting.Gain_H, eisHWSetting.IIR_Gain_H, eisHWSetting.FIR_Gain_H);
        EIS_LOG("IIR_Gain_V(0x%x)",eisHWSetting.IIR_Gain_V);
        EIS_LOG("LMV_TH_X_Cent(0x%x),LMV_TH_X_Surrd(0x%x),LMV_TH_Y_Cent(0x%x),LMV_TH_Y_Surrd(0x%x)",eisHWSetting.LMV_TH_X_Cent,eisHWSetting.LMV_TH_X_Surrd,eisHWSetting.LMV_TH_Y_Cent,eisHWSetting.LMV_TH_Y_Surrd);
        EIS_LOG("FL_Offset_H(0x%x),FL_Offset_V(0x%x)",eisHWSetting.FL_Offset_H, eisHWSetting.FL_Offset_V);
        EIS_LOG("MB_Offset_H(0x%x),MB_Offset_V(0x%x)",eisHWSetting.MB_Offset_H,eisHWSetting.MB_Offset_V);
        EIS_LOG("MB_Intv_H(0x%x),MB_Intv_V(0x%x)",eisHWSetting.MB_Intv_H,eisHWSetting.MB_Intv_V);
    }

    // HW setting
    if(mFirstFlag == 0)
    {
        EIS_LOG("first frame");
        m_pEisDrv->setFirstFrame(MTRUE);
    }

    m_pEisDrv->enableEIS(MTRUE);
    m_pEisDrv->setWRPEnable(MTRUE);                 // enable write RP function

    //m_pEisDrv->setEISSel(eisHWSetting.pathCDRZ);    //YUV domain only setting
    m_pEisDrv->setEISSel(1);    //always after CDRZ. modified for ZSDNCC

    m_pEisDrv->setEISImage(mInput_W,mInput_H);
    m_pEisDrv->setEISFilterDS(eisHWSetting.IIR_DS);
    m_pEisDrv->setMBNum(eisHWSetting.MBNum_H, eisHWSetting.MBNum_V);
    m_pEisDrv->setRPNum(eisHWSetting.RPNum_H,eisHWSetting.RPNum_V);
    m_pEisDrv->setADKneeClip(eisHWSetting.AD_Knee, eisHWSetting.AD_Clip);
    m_pEisDrv->setFilter_H(eisHWSetting.Gain_H, eisHWSetting.IIR_Gain_H, eisHWSetting.FIR_Gain_H);
    m_pEisDrv->setFilter_V(eisHWSetting.IIR_Gain_V);
    m_pEisDrv->setLMV_TH(eisHWSetting.LMV_TH_X_Cent,eisHWSetting.LMV_TH_X_Surrd,eisHWSetting.LMV_TH_Y_Cent,eisHWSetting.LMV_TH_Y_Surrd);
    m_pEisDrv->setFLOffset(eisHWSetting.FL_Offset_H, eisHWSetting.FL_Offset_V);
    m_pEisDrv->setMBOffset_H(eisHWSetting.MB_Offset_H);
    m_pEisDrv->setMBOffset_V(eisHWSetting.MB_Offset_V);
    m_pEisDrv->setMBInterval_H(eisHWSetting.MB_Intv_H);
    m_pEisDrv->setMBInterval_V(eisHWSetting.MB_Intv_V);

    err = m_pEisDrv->configStatus();
    if(err != EIS_RETURN_NO_ERROR)
    {
        EIS_ERR("EIS_Drv register setting fail(%d)",err);
        EIS_LOG("*** Do again ***");

        m_pEisDrv->resetConfigStatus();

        if(a_ehwMode == NSHwScenario::eHW_VSS)
        {
            //RAW domain only setting
            m_pEisDrv->setEIS_DB_SEL(1);
            m_pEisDrv->setEISRawSel(1);
        }
        else if(a_ehwMode == NSHwScenario::eHW_ZSD)
        {
            //YUV domain only setting
            m_pEisDrv->setEIS_DB_SEL(0);
            m_pEisDrv->setEISRawSel(0);
        }
        else
        {
            EIS_LOG("not support right now, use VSS");

            //RAW domain only setting
            m_pEisDrv->setEIS_DB_SEL(1);
            m_pEisDrv->setEISRawSel(1);
        }

        // HW setting
        if(mFirstFlag == 0)
        {
            EIS_LOG("first frame");
            m_pEisDrv->setFirstFrame(MTRUE);
        }

        m_pEisDrv->enableEIS(MTRUE);
        m_pEisDrv->setWRPEnable(MTRUE);                 // enable write RP function

        //m_pEisDrv->setEISSel(eisHWSetting.pathCDRZ);    //YUV domain only setting
        m_pEisDrv->setEISSel(1);    //always after CDRZ. modified for ZSDNCC

        m_pEisDrv->setEISImage(mInput_W,mInput_H);
        m_pEisDrv->setEISFilterDS(eisHWSetting.IIR_DS);
        m_pEisDrv->setMBNum(eisHWSetting.MBNum_H, eisHWSetting.MBNum_V);
        m_pEisDrv->setRPNum(eisHWSetting.RPNum_H,eisHWSetting.RPNum_V);
        m_pEisDrv->setADKneeClip(eisHWSetting.AD_Knee, eisHWSetting.AD_Clip);
        m_pEisDrv->setFilter_H(eisHWSetting.Gain_H, eisHWSetting.IIR_Gain_H, eisHWSetting.FIR_Gain_H);
        m_pEisDrv->setFilter_V(eisHWSetting.IIR_Gain_V);
        m_pEisDrv->setLMV_TH(eisHWSetting.LMV_TH_X_Cent,eisHWSetting.LMV_TH_X_Surrd,eisHWSetting.LMV_TH_Y_Cent,eisHWSetting.LMV_TH_Y_Surrd);
        m_pEisDrv->setFLOffset(eisHWSetting.FL_Offset_H, eisHWSetting.FL_Offset_V);
        m_pEisDrv->setMBOffset_H(eisHWSetting.MB_Offset_H);
        m_pEisDrv->setMBOffset_V(eisHWSetting.MB_Offset_V);
        m_pEisDrv->setMBInterval_H(eisHWSetting.MB_Intv_H);
        m_pEisDrv->setMBInterval_V(eisHWSetting.MB_Intv_V);

        err = m_pEisDrv->configStatus();
        if(err != EIS_RETURN_NO_ERROR)
        {
            mConfigPass = MFALSE;
        }
        else
        {
            mConfigPass = MTRUE;
        }
    }
    else
    {
        mConfigPass = MTRUE;
    }

    if(g_debugDump >= 1)
    {
        m_pEisDrv->dumpReg();
    }

    EIS_LOG("-");
}

/*******************************************************************************
*
********************************************************************************/
MINT32 EisHal::doEIS()
{
    if(mConfigPass == MTRUE)
    {
        MINT32 err = EIS_RETURN_NO_ERROR;

        EIS_SET_PROC_INFO_STRUCT sEIS_AlgSetInfo;
        EIS_RESULT_INFO_STRUCT   sEIS_Result;

        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.eis.dump", value, "0");
        g_debugDump = atoi(value);

        if(g_debugDump >= 1)
        {
            EIS_LOG("+");
        }

        //====== Get Statistic from EIS Driver ======

        sEIS_AlgSetInfo.eis_image_size_config.InputWidth   = mInput_W;
        sEIS_AlgSetInfo.eis_image_size_config.InputHeight  = mInput_H;
        sEIS_AlgSetInfo.eis_image_size_config.TargetWidth  = mTarget_W;
        sEIS_AlgSetInfo.eis_image_size_config.TargetHeight = mTarget_H;

        getEISStatistic(&sEIS_AlgSetInfo.eis_state);

        if(g_debugDump >= 1)
        {
            dumpStatistic(sEIS_AlgSetInfo.eis_state);
        }

        //====== EIS Algorithm Calculate CMV ======


        sEIS_AlgSetInfo.DivH = gDivH;
        sEIS_AlgSetInfo.DivV = gDivV;

        //> get MB number

        sEIS_AlgSetInfo.EisWinNum = gEisWinNum;

        //> get Acc & Gyro info

        sEIS_AlgSetInfo.sensor_info.GyroValid = mGyroEnable;
        sEIS_AlgSetInfo.sensor_info.Gvalid    = mAccEnable;

        if(g_debugDump >= 1)
        {
            EIS_LOG("mGyroEnable(%d) , mAccEnable(%d)",mGyroEnable,mAccEnable);
        }

        for(MUINT32 i = 0; i < 3; i++)
        {
            sEIS_AlgSetInfo.sensor_info.AcceInfo[i] = gAccInfo[i];
            sEIS_AlgSetInfo.sensor_info.GyroInfo[i] = gGyroInfo[i];
        }


        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO, &sEIS_AlgSetInfo, NULL);
        if(err != S_EIS_OK)
        {
            EIS_ERR("EisFeatureCtrl(EIS_FEATURE_SET_PROC_INFO) fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        err = m_pEisAlg->EisMain(&sEIS_Result);
        if(err != S_EIS_OK)
        {
            EIS_ERR("EisMain fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        //====== Get GMV ======

        EIS_GMV_INFO_STRUCT eisGMVInfo;
        err = m_pEisAlg->EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV, NULL, &eisGMVInfo);

        if(err != S_EIS_OK)
        {
            EIS_ERR("EisFeatureCtrl(EIS_FEATURE_GET_ORI_GMV) fail(0x%x)",err);
            err = EIS_RETURN_API_FAIL;
            return err;
        }

        //====== Save EIS CMV and GMV =======

        mCMV_X = sEIS_Result.CMV_X;
        mCMV_Y = sEIS_Result.CMV_Y;

        mGMV_X = eisGMVInfo.EIS_GMVx;
        mGMV_Y = eisGMVInfo.EIS_GMVy;

        //====== Not The First Frame ======

        if(mFirstFlag == 0)
        {
            EIS_LOG("not first frame");
            m_pEisDrv->setFirstFrame(MFALSE);
            mFirstFlag = 1;
        }

        //====== Debug ======

        if(g_debugDump >= 1 && mFirstFlag < 3)
        {
            m_pEisDrv->dumpReg();
            ++mFirstFlag;
        }

        if(g_debugDump >= 1)
        {
            EIS_LOG("-");
        }
    }
    else
    {
        EIS_LOG("EIS config fail, do nothing");
    }

    return EIS_RETURN_NO_ERROR;
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::getEISResult(MUINT32 &a_CMV_X_Int,
                               MUINT32 &a_CMV_X_Flt,
                               MUINT32 &a_CMV_Y_Int,
                               MUINT32 &a_CMV_Y_Flt,
                               MUINT32 &a_TarWidth,
                               MUINT32 &a_TarHeight)
{

    EIS_LOG("mCMV_X(%d),mCMV_Y(%d)",mCMV_X,mCMV_Y);

    a_TarWidth  = mTarget_W;
    a_TarHeight = mTarget_H;

    if(mConfigPass == MTRUE)
    {
        //====== Boundary Checking ======

        if(mCMV_X < 0)
        {
            EIS_ERR("mCMV_X should not be negative(%u), fix to 0",mCMV_X);

            mCMV_X = 0;
            a_CMV_X_Int = a_CMV_X_Flt = 0;
        }
        else
        {
            MFLOAT tempCMV_X = mCMV_X / 256.0;
            if((tempCMV_X + (MFLOAT)mTarget_W) > (MFLOAT)mInput_W)
            {
                EIS_ERR("mCMV_X too large(%u), fix to %u",mCMV_X,(mInput_W - mTarget_W));

                mCMV_X = (mInput_W - mTarget_W);
            }

            a_CMV_X_Int = (mCMV_X & (~0xFF)) >> intPartShift;
            a_CMV_X_Flt = (mCMV_X & (0xFF)) << floatPartShift;
        }

        if(mCMV_Y < 0)
        {
            EIS_ERR("mCMV_Y should not be negative(%u), fix to 0",mCMV_Y);

            mCMV_Y = 0;
            a_CMV_Y_Int = a_CMV_Y_Flt = 0;
        }
        else
        {
            MFLOAT tempCMV_Y = mCMV_Y / 256.0;
            if((tempCMV_Y + (MFLOAT)mTarget_H) > (MFLOAT)mInput_H)
            {
                EIS_ERR("mCMV_X too large(%u), fix to %u",mCMV_X,(mInput_H - mTarget_H));

                mCMV_X = (mInput_H - mTarget_H);
            }

            a_CMV_Y_Int = (mCMV_Y & (~0xFF)) >> intPartShift;
            a_CMV_Y_Flt = (mCMV_Y & (0xFF)) << floatPartShift;
        }

        EIS_LOG("X_Int(%u),X_Flt(%u),Y_Int(%u),Y_Flt(%u)",a_CMV_X_Int,
                                                          a_CMV_X_Flt,
                                                          a_CMV_Y_Int,
                                                          a_CMV_Y_Flt);
    }
    else
    {
        EIS_LOG("EIS config fail, do nothing");
        a_CMV_X_Int = 0;
        a_CMV_X_Flt = 0;
        a_CMV_Y_Int = 0;
        a_CMV_Y_Flt = 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::getEISGmv(MUINT32 &a_GMV_X, MUINT32 &a_GMV_Y)
{
    if(mConfigPass == MTRUE)
    {
        a_GMV_X = mGMV_X;
        a_GMV_Y = mGMV_Y;
        EIS_LOG("GMV_X(%d),GMV_Y(%d)",a_GMV_X,a_GMV_Y);
    }
    else
    {
        EIS_LOG("EIS config fail, do nothing");
        a_GMV_X = 0;
        a_GMV_Y = 0;
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::getEISStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat)
{
    EIS_STATISTIC_T eisStat;

    m_pEisDrv->getStatistic(&eisStat);

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        a_pEIS_Stat->i4LMV_X[i]    = eisStat.i4LMV_X[i];
        a_pEIS_Stat->i4LMV_Y[i]    = eisStat.i4LMV_Y[i];
        a_pEIS_Stat->i4LMV_X2[i]   = eisStat.i4LMV_X2[i];
        a_pEIS_Stat->i4LMV_Y2[i]   = eisStat.i4LMV_Y2[i];
        a_pEIS_Stat->NewTrust_X[i] = eisStat.i4NewTrust_X[i];
        a_pEIS_Stat->NewTrust_Y[i] = eisStat.i4NewTrust_Y[i];
        a_pEIS_Stat->SAD[i]        = eisStat.i4SAD[i];
        a_pEIS_Stat->SAD2[i]       = eisStat.i4SAD2[i];
        a_pEIS_Stat->AVG_SAD[i]    = eisStat.i4AVG[i];
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::getEISCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut)
{
    EIS_LOG("+");

    EIS_Customize_Para_t customSetting;

    get_EIS_CustomizeData(&customSetting);

    a_pDataOut->sensitivity = (EIS_SENSITIVITY_ENUM)customSetting.sensitivity;
    a_pDataOut->filter_small_motion = customSetting.filter_small_motion;
    a_pDataOut->adv_shake_ext          = customSetting.adv_shake_ext;  // 0 or 1
    a_pDataOut->stabilization_strength = customSetting.stabilization_strength;  // 0.5~0.95

    a_pDataOut->advtuning_data.new_tru_th = customSetting.new_tru_th; // 0~100
    a_pDataOut->advtuning_data.vot_th = customSetting.vot_th;      // 1~16
    a_pDataOut->advtuning_data.votb_enlarge_size = customSetting.votb_enlarge_size;  // 0~1280
    a_pDataOut->advtuning_data.min_s_th   = customSetting.min_s_th; // 10~100
    a_pDataOut->advtuning_data.vec_th     = customSetting.vec_th;   // 0~11   should be even
    a_pDataOut->advtuning_data.spr_offset = customSetting.spr_offset; //0 ~ MarginX/2
    a_pDataOut->advtuning_data.spr_gain1  = customSetting.spr_gain1; // 0~127
    a_pDataOut->advtuning_data.spr_gain2  = customSetting.spr_gain2; // 0~127
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

    EIS_LOG("-");
}


/*******************************************************************************
*
********************************************************************************/
MVOID EisHal::dumpStatistic(EIS_STATISTIC_STRUCT a_EISStat)
{
    EIS_LOG("+");

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, LMV_X = %d, LMV_Y = %d",(i/4),(i%4),a_EISStat.i4LMV_X[i],a_EISStat.i4LMV_Y[i]);
    }

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, LMV_X2 = %d, LMV_Y2 = %d",(i/4),(i%4),a_EISStat.i4LMV_X2[i],a_EISStat.i4LMV_Y2[i]);
    }

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, MinSAD = %d",(i/4),(i%4),a_EISStat.SAD[i]);
    }

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, MinSAD2 = %d",(i/4),(i%4),a_EISStat.SAD2[i]);
    }

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, AvgSAD = %d",(i/4),(i%4),a_EISStat.AVG_SAD[i]);
    }

    for(MINT32 i = 0; i < EIS_MAX_WIN_NUM; ++i)
    {
        EIS_LOG("MB%d%d, NewTrust_X = %d, NewTrust_Y = %d",(i/4),(i%4),a_EISStat.NewTrust_X[i],a_EISStat.NewTrust_Y[i]);
    }

    EIS_LOG("-");
}


