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

//! \file  eis_hal_r.h

#ifndef _EIS_HAL_R_H_
#define _EIS_HAL_R_H_

#include <mtkcam/featureio/eis_hal_base.h>

typedef enum EIS_HAL_STATE
{
    EIS_HAL_STATE_NONE   = 0x0000,
    EIS_HAL_STATE_ALIVE  = 0x0001,
    EIS_HAL_STATE_UNINIT = 0x0002
}EIS_HAL_STATE_ENUM;

/**
  *@class EisHalImp
  *@brief Implementation of EisHal class(R chip)
*/
class EisHalImp : public EisHal_R
{
public:

    /**
         *@brief EisHalImp constructor
       */
    EisHalImp(const MUINT32 &aSensorIdx);

    /**
         *@brief EisHalImp destructor
       */
    ~EisHalImp() {}

    /**
         *@brief Create EisHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisHal object
       */
    static EisHal_R *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init();

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEis(const EIS_HAL_CONFIG_DATA &aEisConfig);

    /**
         *@brief Execute EIS
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoEis(EIS_HAL_CONFIG_DATA *apEisConfig = NULL,MINT64 aTimeStamp = -1);

    /**
         *@brief Stop EisDrv thread
       */
    virtual MVOID EisThreadStop();

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] a_CMV_X_Int : EIS algo result of X-direction integer part
         *@param[out] a_CMV_X_Flt  : EIS algo result of X-direction float part
         *@param[out] a_CMV_Y_Int : EIS algo result of Y-direction integer part
         *@param[out] a_CMV_Y_Flt  : EIS algo result of Y-direction float part
         *@param[out] a_TarWidth    : EIS width crop size
         *@param[out] a_TarHeight   : EIS height crop size
       */
    virtual MVOID GetEisResult(MUINT32 &a_CMV_X_Int,
                                   MUINT32 &a_CMV_X_Flt,
                                   MUINT32 &a_CMV_Y_Int,
                                   MUINT32 &a_CMV_Y_Flt,
                                   MUINT32 &a_TarWidth,
                                   MUINT32 &a_TarHeight);

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y  : y-direction global motion vector between two frames
         *@param[out] aEisInW  : width of EIS input image (optional)
         *@param[out] aEisInH  : height of EIS input image (optional)
       */
    virtual MVOID GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW=NULL,MUINT32 *aEisInH=NULL);

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx);

    /**
         *@brief Command API
         *@param[in] EIS_CMD_ENUM : command enum
         *@param[in] arg1 : argument
         *@param[in] arg2 : argument
         *@param[in] arg3 : argument
       */
    virtual MVOID SendCommand(EIS_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2 = -1, MINT32 arg3 = -1);

    /**
         *@brief Return EIS HW statistic result
         *@param[in,out] a_pEIS_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat);

private:

    /**
         *@brief Get sensor info
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo();

    /**
         *@brief Get EIS customize info
         *@param[out] a_pDataOut : EIS_TUNING_PARA_STRUCT
       */
    MVOID GetEisCustomize(EIS_TUNING_PARA_STRUCT *a_pDataOut);

    /**
         *@brief Dump EIS HW statistic info
         *@param[in] aEisStat : EIS_STATISTIC_STRUCT
       */
    MVOID DumpStatistic(const EIS_STATISTIC_STRUCT &aEisStat);

    /**
         *@brief Create IMem buffer
         *@param[in,out] memSize : memory size, will align to L1 cache
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 CreateMemBuf(MUINT32 &memSize,const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Destroy IMem buffer
         *@param[in] bufCnt : how many buffer
         *@param[in,out] bufInfo : IMem object
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 DestroyMemBuf(const MUINT32 &bufCnt, IMEM_BUF_INFO *bufInfo);

    /**
         *@brief Prepare EIS pass1 result
         *@param[in] cmvX : EIS result
         *@param[in] cmvY : EIS result
       */
    MVOID PrepareEisResult(const MINT32 &cmvX,const MINT32 &cmvY);

    /***************************************************************************************/

    volatile MINT32 mUsers;
    mutable Mutex mLock;
    mutable Mutex mP1Lock;

    // EIS driver object
    EisDrv_R *m_pEisDrv;

    // EIS algo object
    MTKEis *m_pEisAlg;
    EIS_SET_PROC_INFO_STRUCT mEisAlgoProcData;

    // IMEM
    IMemDrv *m_pIMemDrv;
    IMEM_BUF_INFO mEisDbgBuf;

    // EIS member variable
    MUINT32 mEisInput_W;
    MUINT32 mEisInput_H;
    MUINT32 mP1Target_W;
    MUINT32 mP1Target_H;

    // EIS result
    MUINT32 mCmvX_Int;
    MUINT32 mCmvX_Flt;
    MUINT32 mCmvY_Int;
    MUINT32 mCmvY_Flt;
    MINT32 mGMV_X;
    MINT32 mGMV_Y;

    // member variable
    MUINT32 mFrameCnt;
    MUINT32 mEisPass1Only;
    MUINT32 mIsEisConfig;
    MUINT32 mMemAlignment;
    MBOOL   mEisSupport;

    // sensor
    IHalSensorList *m_pHalSensorList;
    IHalSensor *m_pHalSensor;
    MUINT32 mSensorIdx;
    MUINT32 mSensorDev;
    SensorStaticInfo mSensorStaticInfo;
    SensorDynamicInfo mSensorDynamicInfo;
};

/**
  *@class EisHalObj
  *@brief singleton object for each EisHal which is seperated by sensor index
*/
template<const MUINT32 aSensorIdx>
class EisHalObj : public EisHalImp
{
public:
    static EisHal_R *GetInstance()
    {
        static EisHalObj<aSensorIdx> singleton;

        return &singleton;
    }

    EisHalObj() : EisHalImp(aSensorIdx) {}

    ~EisHalObj() {}

};

#endif

