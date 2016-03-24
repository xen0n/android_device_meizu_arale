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
* @file eis_hal.h
*
* EIS Hal Header File
*
*/

#ifndef _EIS_HAL_H_
#define _EIS_HAL_H_

#include "mtkcam/featureio/eis_type.h"
#include "mtkcam/UITypes.h"

// EIS ratio
#define EIS_FACTOR 120

/**
  *@brief EIS hal class used by scenario
*/
class EisHal
{
public:

    /**
         *@brief EisHal constructor
       */
    EisHal() {};

    /**
         *@brief Create EisHal object
         *@param[in] userName : user name,i.e. who create EIS HAL object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisHal object
       */
    static EisHal *CreateInstance(char const *userName, const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS HAL object
       */
    virtual MVOID DestroyInstance(char const *userName) = 0;

    /**
         *@brief Initialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig) = 0;

    /**
         *@brief Execute EIS
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoEis(const EIS_PASS_ENUM &aEisPass,EIS_HAL_CONFIG_DATA *apEisConfig = NULL,MINT64 aTimeStamp = -1) = 0;

    /**
         *@brief Get EIS Plus GPU infomation
         *@param[in] aGridX : pointer to array
         *@param[in] aGridY : pointer to array
       */
    virtual MVOID SetEisPlusGpuInfo(MINT32 * const aGridX, MINT32 * const aGridY) = 0;

    /**
         *@brief Set EIS Pass2 User
         *@param[in] userName : user name
       */
    virtual MVOID SetEisP2User(char const *userName) = 0;

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
                                   MUINT32 &a_TarHeight) = 0;

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] P_EIS_PLUS_RESULT_INFO_STRUCT : EISPlus result
       */
    virtual MVOID GetEisPlusResult(P_EIS_PLUS_RESULT_INFO_STRUCT apEisPlusResult) = 0;

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y  : y-direction global motion vector between two frames
         *@param[out] aEisInW  : width of EIS input image (optional)
         *@param[out] aEisInH  : height of EIS input image (optional)
       */
    virtual MVOID GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW=NULL,MUINT32 *aEisInH=NULL) = 0;

    /**
         *@brief Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_REG_INFO
       */
    virtual MVOID GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo) = 0;

    /**
         *@brief Get EIS Plus GPU infomation
         *@param[in,out] aEisPlusGpu : EIS_PLUS_GPU_INFO
       */
    virtual MVOID GetEisPlusGpuInfo(EIS_PLUS_GPU_INFO *aEisPlusGpu) = 0;

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx) = 0;

    /**
         *@brief Get EIS plus crop ratio
         *@return
         *-EIS plus crop ratio
       */
    virtual MUINT32 GetEisPlusCropRatio() = 0;

    /**
         *@brief Get EIS pass1 result at pass2
         *@param[in] userName : user name
         *@param[in,out] apEisResult : pointer to EIS_P1_RESULT_INFO
         *@param[in] aTimeStamp : time stamp of current frame
         *@param[in] lastUser : last user or not
       */
    virtual MVOID GetEisInfoAtP2(char const *userName, EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser) = 0;

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    virtual MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush) = 0;

    /**
         *@brief Command API
         *@param[in] EIS_CMD_ENUM : command enum
         *@param[in] arg1 : argument
         *@param[in] arg2 : argument
         *@param[in] arg3 : argument
       */
    virtual MVOID SendCommand(EIS_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2 = -1, MINT32 arg3 = -1) = 0;

#if EIS_ALGO_READY

    /**
         *@brief Return EIS HW statistic result
         *@param[out] a_pEIS_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat) = 0;

#endif

    /**
     *@brief  Get input width/height of EIS HW
   */
virtual MVOID GetEisInputSize(MUINT32 *aWidth, MUINT32 *aHeight) = 0;

/**
     *@brief  Return  EIS HW setting of EOS_OP_HORI
     *@return
     *-EOS_OP_HORI
   */
virtual MUINT32 GetEisDivH() = 0;

/**
     *@brief  Return  EIS HW setting of EIS_OP_VERT
     *@return
     *-EIS_OP_VERT
   */
virtual MUINT32 GetEisDivV() = 0;

protected:

    /**
         *@brief EisHal destructor
       */
    virtual ~EisHal() {};
};

class EisHal_R
{
public:

    /**
         *@brief EisHal constructor
       */
    EisHal_R() {};

    /**
         *@brief Create EisHal object
         *@param[in] userName : user name,i.e. who create EIS HAL object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisHal object
       */
    static EisHal_R *CreateInstance(char const *userName, const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisHal object
         *@param[in] userName : user name,i.e. who destroy EIS HAL object
       */
    virtual MVOID DestroyInstance(char const *userName) = 0;

    /**
         *@brief Initialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Unitialization function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure EIS
         *@details Use this API after pass1/pass2 config and before pass1/pass2 start
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] aEisConfig : EIS config data
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig) = 0;

    /**
         *@brief Execute EIS
         *@param[in] aEisPass : indicate pass1 or pass2
         *@param[in] apEisConfig : EIS config data, mainly for pass2
         *@param[in] aTimeStamp : time stamp of pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 DoEis(const EIS_PASS_ENUM &aEisPass,EIS_HAL_CONFIG_DATA *apEisConfig = NULL,MINT64 aTimeStamp = -1) = 0;

    /**
         *@brief Stop EisDrv thread
       */
    virtual MVOID EisThreadStop() = 0;

    /**
         *@brief Get EIS Plus GPU infomation
         *@param[in] aGridX : pointer to array
         *@param[in] aGridY : pointer to array
       */
    virtual MVOID SetEisPlusGpuInfo(MINT32 * const aGridX, MINT32 * const aGridY) = 0;

    /**
         *@brief Set EIS Pass2 User
         *@param[in] userName : user name
       */
    virtual MVOID SetEisP2User(char const *userName) = 0;

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
                                   MUINT32 &a_TarHeight) = 0;

    /**
         *@brief Get EIS algorithm result (CMV)
         *@param[out] P_EIS_PLUS_RESULT_INFO_STRUCT : EISPlus result
       */
    virtual MVOID GetEisPlusResult(P_EIS_PLUS_RESULT_INFO_STRUCT apEisPlusResult) = 0;

    /**
         *@brief Get EIS GMV
         *@details The value is 256x
         *@param[out] aGMV_X : x-direction global motion vector between two frames
         *@param[out] aGMV_Y  : y-direction global motion vector between two frames
         *@param[out] aEisInW  : width of EIS input image (optional)
         *@param[out] aEisInH  : height of EIS input image (optional)
       */
    virtual MVOID GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW=NULL,MUINT32 *aEisInH=NULL) = 0;

    /**
         *@brief Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_REG_INFO
       */
    virtual MVOID GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo) = 0;

/**
         *@brief Get EIS Plus GPU infomation
         *@param[in,out] aEisPlusGpu : EIS_PLUS_GPU_INFO
   */
    virtual MVOID GetEisPlusGpuInfo(EIS_PLUS_GPU_INFO *aEisPlusGpu) = 0;

/**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
     *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
   */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx) = 0;

/**
         *@brief Get EIS plus crop ratio
     *@return
         *-EIS plus crop ratio
   */
    virtual MUINT32 GetEisPlusCropRatio() = 0;

    /**
         *@brief Get EIS pass1 result at pass2
         *@param[in] userName : user name
         *@param[in,out] apEisResult : pointer to EIS_P1_RESULT_INFO
         *@param[in] aTimeStamp : time stamp of current frame
         *@param[in] lastUser : last user or not
       */
    virtual MVOID GetEisInfoAtP2(char const *userName, EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser) = 0;

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    virtual MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush) = 0;

    /**
         *@brief Command API
         *@param[in] EIS_CMD_ENUM : command enum
         *@param[in] arg1 : argument
         *@param[in] arg2 : argument
         *@param[in] arg3 : argument
       */
    virtual MVOID SendCommand(EIS_CMD_ENUM aCmd,MINT32 arg1, MINT32 arg2 = -1, MINT32 arg3 = -1) = 0;

   /**
     *@brief Command API
     *@param[in] isEISOn : Eis is on or off
     *@param[in] sensorSize : sensor size
     *@param[in] requestSize : current request sieze
     *@return
     *-minimum size
    */
    virtual NSCam::MSize QueryMinSize(MBOOL isEISOn, NSCam::MSize sensorSize, NSCam::MSize requestSize) = 0;

#if EIS_ALGO_READY

    /**
         *@brief Return EIS HW statistic result
         *@param[out] a_pEIS_Stat : EIS_STATISTIC_STRUCT
       */
    virtual MVOID GetEisStatistic(EIS_STATISTIC_STRUCT *a_pEIS_Stat) = 0;

#endif

protected:

    /**
         *@brief EisHal destructor
       */
    virtual ~EisHal_R() {};
};

#endif

