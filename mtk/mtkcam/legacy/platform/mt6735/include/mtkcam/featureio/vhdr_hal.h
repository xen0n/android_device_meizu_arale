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

/**
* @file vhdr_hal.h
*
* VHDR Hal Header File
*
*/

#ifndef _VHDR_HAL_H_
#define _VHDR_HAL_H_

#include "mtkcam/featureio/vhdr_type.h"


/**
  *@brief VHDR HAL class used by scenario
*/
class VHdrHal
{
public:

    /**
         *@brief VHdrHal constructor
       */
    VHdrHal() {};

    /**
         *@brief Create VHdrHal object
         *@param[in] userName : user name,i.e. who create EIS HAL object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrHal object
       */
    static VHdrHal *CreateInstance(char const *userName, const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy VHdrHal object
         *@param[in] userName : user name,i.e. who destroy VHdrHal object
       */
    virtual MVOID DestroyInstance(char const *userName) = 0;

    /**
         *@brief Initialization function
         *@param[in] aMode : VHDR mode
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init(const MUINT32 &aMode) = 0;

    /**
         *@brief Unitialization function
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Congif VHDR
         *@param[in] aConfigData : config data
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigVHdr(const VHDR_HAL_CONFIG_DATA &aConfigData) = 0;

    /**
         *@brief Congif VHDR
         *@param[in] aTimeStamp : time stamp of pass1 image
       */
    virtual MVOID DoVHdr(const MINT64 &aTimeStamp) = 0;

    /**
         *@brief Get LCEI DMA setting
         *@param[in,out] apLceiDmaInfo : LCEI_DMA_INFO
         *@param[in] aTimeStamp : time stamp of pass1 image which is going to run pass2
       */
    virtual MVOID GetLceiDmaInfo(LCEI_DMA_INFO *apLceiDmaInfo,const MINT64 &aTimeStamp) = 0;

    /**
         *@brief Command API
         *@param[in] VHDR_CMD_ENUM : command enum
         *@param[in] arg1 : argument
         *@param[in] arg2 : argument
         *@param[in] arg3 : argument
       */
    virtual MVOID SendCommand(VHDR_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2 = -1, MINT32 arg3 = -1) = 0;

protected:

    /**
         *@brief VHdrHal destructor
       */
    virtual ~VHdrHal() {};
};


#endif

