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
* @file eis_drv.h
*
* EIS Driver Header File
*
*/

#ifndef _EIS_DRV_H_
#define _EIS_DRV_H_

#include "mtkcam/featureio/eis_type.h"
#include "mtkcam/drv/imem_drv.h"

#define MAX_FEO_SIZE (EIS_FE_MAX_INPUT_W*EIS_FE_MAX_INPUT_H/16/16)

/**
  *@brief FEO register info (driver use)
*/
typedef struct FEO_COFIG
{
    MUINT32 inputW;
    MUINT32 inputH;
    MUINT32 xSize;
    MUINT32 ySize;
    MUINT32 stride;
    IMEM_BUF_INFO memInfo;
}FEO_COFIG_DATA;


/**
  *@brief EIS driver class used by EIS_Hal
*/
class EisDrv
{
public:

    /**
         *@brief  EisDrv constructor
       */
    EisDrv() {}

    /**
         *@brief EisDrv destructor
       */
    virtual ~EisDrv() {}

    /**
         *@brief Create EisDrv object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisDrv object
       */
    static EisDrv *CreateInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisDrv object
       */
    virtual MVOID DestroyInstance() = 0;

    /**
         *@brief Initial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Uninitial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure EIS and related register value
         *@param[in] aSce : EIS configure scenario
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEisReg(const EIS_SCENARIO_ENUM &aSce,const MUINT32 &aSensorTg) = 0;

    /**
         *@brief Configure FEO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigFeo() = 0;

    /**
         *@brief Enable/Disable EIS
         *@param[in] aEn : MTRUE (enable) / MFALSE (disable)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 EnableEis(const MBOOL &aEn) = 0;

    /**
         *@brief Set configure done
         *@param[in] aState : state enum
       */
    virtual MVOID SetEisoThreadState(EIS_SW_STATE_ENUM aState) = 0;

    /**
         *@brief To indicate the first frame
         *@param[in] aFirst : 1 (is the first frame) / 0 (is not the first frame)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 SetFirstFrame(const MUINT32 &aFirst) = 0;

    /**
         *@brief To indicate the first frame
         *@param[in] aInputW : FE input width
         *@param[in] aInputH : FE input height
         *@param[in] aBlockNum : FE block number
       */
    virtual MVOID SetFeBlockNum(const MUINT32 &aInputW, const MUINT32 &aInputH,const MUINT32 &aBlockNum) = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-1 : not first frame, 0 : first frame
       */
    virtual MUINT32 GetFirstFrameInfo() = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not 2-pixel mode, 1 : 2-pixel mode
       */
    virtual MUINT32 Get2PixelMode() = 0;

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

    /**
         *@brief  Return  Total MB number
         *@return
         *-MBNum_V * MB_Num_H
       */
    virtual MUINT32 GetEisMbNum() = 0;

    /**
         *@brief  Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_COFIG_DATA
       */
    virtual MVOID GetFeoRegInfo(FEO_COFIG_DATA *aFeoRegInfo) = 0;

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx) = 0;

#if EIS_ALGO_READY

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_STRUCT *apEisStat,const MINT64 &aTimeStamp) = 0;

#else

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_T *apEisStat,const MINT64 &aTimeStamp) = 0;

#endif

    /**
         *@brief  Get statistic of FEO
         *@param[in,out] FEO_STAT_DATA : FEO statistic data
       */
    virtual MVOID GetFeoStatistic(FEO_STAT_DATA *aFeoStatData) = 0;

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    virtual MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush) = 0;

    /**
         *@brief  Dump EIS register setting
         *@param[in] aEisPass : EIS pass ENUM
         *@details Debug usage
       */
    virtual MVOID DumpReg(const EIS_PASS_ENUM &aEisPass) = 0;

#if EIS_WORK_AROUND

    virtual MUINT32 GetTgRrzRatio() = 0;

#endif
};

class EisDrv_R
{
public:

    /**
         *@brief  EisDrv constructor
       */
    EisDrv_R() {}

    /**
         *@brief EisDrv destructor
       */
    virtual ~EisDrv_R() {}

    /**
         *@brief Create EisDrv object
         *@param[in] aSensorIdx : sensor index
         *@return
         *-EisDrv object
       */
    static EisDrv_R *CreateInstance(const MUINT32 &aSensorIdx);

    /**
         *@brief Destroy EisDrv object
       */
    virtual MVOID DestroyInstance() = 0;

    /**
         *@brief Initial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Init() = 0;

    /**
         *@brief Uninitial function
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 Uninit() = 0;

    /**
         *@brief Configure EIS and related register value
         *@param[in] aSce : EIS configure scenario
         *@param[in] aSensorTg : sensor TG info for debuging usage
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigEisReg(const EIS_SCENARIO_ENUM &aSce,const MUINT32 &aSensorTg) = 0;

    /**
         *@brief Configure FEO
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 ConfigFeo() = 0;

    /**
         *@brief Enable/Disable EIS
         *@param[in] aEn : MTRUE (enable) / MFALSE (disable)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 EnableEis(const MBOOL &aEn) = 0;

    /**
         *@brief Set configure done
         *@param[in] aState : state enum
       */
    virtual MVOID SetEisoThreadState(EIS_SW_STATE_ENUM aState) = 0;

    /**
         *@brief To indicate the first frame
         *@param[in] aFirst : 1 (is the first frame) / 0 (is not the first frame)
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 SetFirstFrame(const MUINT32 &aFirst) = 0;

    /**
         *@brief To indicate the first frame
         *@param[in] aInputW : FE input width
         *@param[in] aInputH : FE input height
         *@param[in] aBlockNum : FE block number
       */
    virtual MVOID SetFeBlockNum(const MUINT32 &aInputW, const MUINT32 &aInputH,const MUINT32 &aBlockNum) = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-1 : not first frame, 0 : first frame
       */
    virtual MUINT32 GetFirstFrameInfo() = 0;

    /**
         *@brief Get first frame or not
         *@return
         *-0 : not 2-pixel mode, 1 : 2-pixel mode
       */
    virtual MUINT32 Get2PixelMode() = 0;

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

    /**
         *@brief  Return  Total MB number
         *@return
         *-MBNum_V * MB_Num_H
       */
    virtual MUINT32 GetEisMbNum() = 0;

    /**
         *@brief  Get FEO register setting
         *@param[in,out] aFeoRegInfo : FEO_COFIG_DATA
       */
    virtual MVOID GetFeoRegInfo(FEO_COFIG_DATA *aFeoRegInfo) = 0;

    /**
         *@brief Get EIS HW support or not
         *@param[in] aSensorIdx : sensor index
         *@return
         *-MTRUE indicates EIS HW is supported, MFALSE indicates not supported
       */
    virtual MBOOL GetEisSupportInfo(const MUINT32 &aSensorIdx) = 0;

#if EIS_ALGO_READY

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_STRUCT *apEisStat,const MINT64 &aTimeStamp) = 0;

#else

    /**
         *@brief  Get statistic of EIS HW
         *@param[in,out] apEisStat : EIS_STATISTIC_T struct
         *@param[in] aTimeStamp : time stamp of  pass1 image
         *@return
         *-EIS_RETURN_NO_ERROR indicates success, otherwise indicates fail
       */
    virtual MINT32 GetEisHwStatistic(EIS_STATISTIC_T *apEisStat,const MINT64 &aTimeStamp) = 0;

#endif

    /**
         *@brief  Get statistic of FEO
         *@param[in,out] FEO_STAT_DATA : FEO statistic data
       */
    virtual MVOID GetFeoStatistic(FEO_STAT_DATA *aFeoStatData) = 0;

    /**
         *@brief  Flush memory
         *@param[in] aDma : DMA port : EISO or FEO
          *@param[in] aFlush : flush type
       */
    virtual MVOID FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush) = 0;

    /**
         *@brief  Dump EIS register setting
         *@param[in] aEisPass : EIS pass ENUM
         *@details Debug usage
       */
    virtual MVOID DumpReg(const EIS_PASS_ENUM &aEisPass) = 0;

#if EIS_WORK_AROUND

    virtual MUINT32 GetTgRrzRatio() = 0;

#endif
};

#endif // _EIS_DRV_H_

