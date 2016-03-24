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
* @file vhdr_drv.h
*
* VHDR Driver Header File
*
*/

#ifndef _VHDR_DRV_H_
#define _VHDR_DRV_H_

#include "mtkcam/featureio/vhdr_type.h"

/**
  *@brief VHDR driver class
*/
class VHdrDrv
{
public:

    /**
         *@brief  VHdrDrv constructor
       */
    VHdrDrv() {}

    /**
         *@brief VHdrDrv destructor
       */
    virtual ~VHdrDrv() {}

    /**
         *@brief Create VHdrDrv object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-VHdrDrv object
       */
    static VHdrDrv *CreateInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy VHdrDrv object
       */
    virtual MVOID DestroyInstance() = 0;

    /**
         *@brief Initialization function
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Unitialization function
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Set Sensor info
         *@param[in] aFlareGain : flare gain
         *@param[in] aFlareOffset : flare offset
       */
    virtual MVOID SetFlareInfo(const MINT16 &aFlareGain,const MINT16 &aFlareOffset) = 0;

    /**
         *@brief Set sensor info
         *@param[in] aSensorDev : sensor device
         *@param[in] aSensorTg : flare sensor TG
       */
    virtual MVOID SetSensorInfo(const MUINT32 &aSensorDev,const MUINT32 &aSensorTg) = 0;

    /**
         *@brief Set configure done
         *@param[in] aState : state enum
       */
    virtual MVOID SetLcsoThreadState(VHDR_STATE_ENUM aState) = 0;

    /**
         *@brief Set configure done
         *@param[in] aCmd : VHDR_CMD_ENUM
         *@param[in] arg1
         *@param[in] arg2
         *@param[in] arg3
       */
    virtual MVOID SetVideoSizeInfo(const VHDR_CMD_ENUM &aCmd,MINT32 arg1,MINT32 arg2 = -1,MINT32 arg3 = -1) = 0;

    /**
         *@brief Config LCS and LCSO
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLcs() = 0;

    /**
         *@brief Config LCSO
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigLcso() = 0;

    /**
         *@brief Config RMG
         *@param[in] aLeFirst : 0 : first two rows are SE,  1 : first two rows are LE
       */
    virtual MVOID ConfigRmg(const MUINT32 &aLeFirst) = 0;

    /**
         *@brief Update LCS
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 UpdateLcs() = 0;

     /**
         *@brief Update LCSO time stamp according to pass1 image
         *@param[in] aTimeStamp : time stamp of  pass1 image
       */
    virtual MVOID UpdateLcsoTimeStamp(const MINT64 &aTimeStamp) = 0;

    /**
         *@brief Get LCEI DMA setting
         *@param[in,out] apLceiDmaInfo : LCEI_DMA_INFO
         *@param[in] aTimeStamp : time stamp of pass1 image which is going to run pass2
       */
    virtual MVOID GetLceiDmaInfo(LCEI_DMA_INFO *apLceiDmaInfo,const MINT64 &aTimeStamp) = 0;

    /**
         *@brief Enable LCS and LCSO
         *@param[in] aEn : MFALSE - disable, MTRUE - enable
         *@return
         *-VHDR_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 EnableLcs(const MBOOL &aEn) = 0;

    /**
         *@brief Enable LCE
         *@param[in] aEn : MFALSE - disable, MTRUE - enable
       */
    virtual MVOID EnableLce(const MBOOL &aEn) = 0;

    /**
         *@brief Enable RMG
         *@param[in] aEn : MFALSE - disable, MTRUE - enable
       */
    virtual MVOID EnableRmg(const MBOOL &aEn) = 0;
};

#endif

