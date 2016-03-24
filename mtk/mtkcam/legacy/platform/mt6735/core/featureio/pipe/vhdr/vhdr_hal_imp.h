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

/**
* @file vhdr_hal_imp.h
*
* VHDR Hal Implementation Header File
*
*/


#ifndef _VHDR_HAL_IMP_H_
#define _VHDR_HAL_IMP_H_

#include "mtkcam/featureio/vhdr_hal.h"

class VHdrHalCallBack : public I3ACallBack
{
public:
    VHdrHalCallBack(MVOID *arg);

    ~VHdrHalCallBack();

    virtual void doNotifyCb(int32_t _msgType,
                               int32_t _ext1,
                               int32_t _ext2,
                               int32_t _ext3,
                               MINTPTR _ext4);

    virtual void doDataCb(int32_t _msgType,
                              void*   _data,
                              uint32_t _size);
private:
    MVOID *m_pVHdrHalImp;
};


/**
  *@brief Implementation of VHdrHal class
*/
class VHdrHalImp : public VHdrHal
{
public:

    /**
         *@brief VHdrHalImp constructor
         *@param[in] aSensorIdx : sensor index
       */
    VHdrHalImp(const MUINT32 &aSensorIdx);

    /**
         *@brief VHdrHalImp destructor
       */
    ~VHdrHalImp() {}

    /**
         *@brief Create VHdrHal object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrHal object
       */
    static VHdrHal *GetInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy VHdrHal object
         *@param[in] userName : user name,i.e. who destroy VHdrHal object
       */
    virtual MVOID DestroyInstance(char const *userName);

    /**
         *@brief Initialization function
         *@param[in] aMode : VHDR mode
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init(const MUINT32 &aMode);

    /**
         *@brief Unitialization function
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit();

    /**
         *@brief Congif VHDR
         *@param[in] aConfigData : config data
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData);

    /**
         *@brief Congif VHDR
         *@param[in] aTimeStamp : time stamp of pass1 image
       */
    virtual MVOID DoVHdr(const MINT64 &aTimeStamp);

    /**
         *@brief Get LCEI DMA setting
         *@param[in,out] apLceiDmaInfo : LCEI_DMA_INFO
         *@param[in] aTimeStamp : time stamp of pass1 image which is going to run pass2
       */
    virtual MVOID GetLceiDmaInfo(LCEI_DMA_INFO *apLceiDmaInfo,const MINT64 &aTimeStamp);

    /**
         *@brief Command API
         *@param[in] VHDR_CMD_ENUM : command enum
         *@param[in] arg1 : argument
         *@param[in] arg2 : argument
         *@param[in] arg3 : argument
       */
    virtual MVOID SendCommand(VHDR_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2 = -1, MINT32 arg3 = -1);

    /**
         *@brief Sync VHDR with 3A by VSync
       */
    MVOID VHdrSync3A();

private:

    /**
         *@brief VHDR thread loop
       */
    static MVOID *VHdrThreadLoop(MVOID *arg);

    /**
         *@brief Get sensor info
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 GetSensorInfo();

    /**
         *@brief Update VHDR
         *@detail Call right after pass1 done and before update 3A
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    MINT32 UpdateVHdr();

    /**
         *@brief Set state
         *@param[in] aState : state
       */
    MVOID SetVHdrState(const VHDR_STATE_ENUM &aState);

    /**
         *@brief Get state
         *@return
         *-state
       */
    VHDR_STATE_ENUM GetVHdrState();

    /**
         *@brief Change thread setting
       */
    MVOID ChangeThreadSetting();

    /**
         *@brief convert VHDR_CAMERA_VER_ENUM to E_VER
       */
    IHal3A::E_VER ConvertCameraVer(const VHDR_CAMERA_VER_ENUM &aVer);

    /***************************************************************************************/

    mutable Mutex mLock;

    // member variable
    volatile MINT32 mUsers;
    MUINT32 mVhdrMode;

    // VHdr driver object
    VHdrDrv *m_pVHdrDrv;

    // sensor
    IHalSensorList *m_pHalSensorList;
    IHalSensor *m_pHalSensor;
    MUINT32 mSensorDev;
    MUINT32 mSensorIdx;
    SensorStaticInfo mSensorStaticInfo;
    SensorDynamicInfo mSensorDynamicInfo;

    // state
    VHDR_STATE_ENUM mState;

    // thread
    pthread_t mVsyncThread;
    sem_t mVsyncSem;
    sem_t mVsyncSemEnd;

    // 3A
    IHal3A *m_p3aHal;
    VHdrHalCallBack *m_pVHdrCb3A;
};

/**
  *@class VHdrHalObj
  *@brief singleton object for each VHdrHal which is seperated by sensor index
*/
template<const MUINT32 aSensorIdx>
class VHdrHalObj : public VHdrHalImp
{
public:
    static VHdrHal *GetInstance()
    {
        static VHdrHalObj<aSensorIdx> singleton;

        return &singleton;
    }

    VHdrHalObj() : VHdrHalImp(aSensorIdx) {}

    ~VHdrHalObj() {}

};


#endif

