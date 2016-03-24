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
#define LOG_TAG "aaa_hal_if"

#include <hal/aaa/aaa_hal_if.h>
#include <hal/aaa/aaa_hal_flowCtrl.h>

//#include <mtkcam/hal/aaa/ResultBufMgr/ResultBufMgr.h>
//#include "aaa_hal_template.h"

using namespace NS3Av3;

#define EN_3A_CB_LOG (0)

#if 0
/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const u4SensorOpenIndex)
{
    static Hal3AIf singleton;
    return &singleton;
}

#else

#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
//#include <mtkcam/hal/aaa/aaa_hal.h>
//#include <mtkcam/hal/aaa/state_mgr/aaa_state.h>
//#include <mtkcam/hal/aaa/state_mgr/aaa_state_mgr.h>
//#include <mtkcam/hal/aaa/aaa_hal_yuv.h>
//
/*NeedUpdate*///#include <mtkcam/drv/sensor_hal.h>
//#include <mtkcam/hal/IHalSensor.h>

using namespace NS3Av3;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const i4SensorOpenIndex)
{
    //temporary
    return Hal3AFlowCtrl::createInstance(i4SensorOpenIndex);

#if 0


    IHalSensorList*const pHalSensorList = IHalSensorList::get();
    if (!pHalSensorList) return NULL;

    MINT32 i4SensorDevId = pHalSensorList->querySensorDevIdx(i4SensorOpenIndex);
    MINT32 eSensorType = pHalSensorList->queryType(i4SensorOpenIndex); // sensor hal defined

    if (eSensorType == NSSensorType::eRAW)
        return Hal3A::createInstance(i4SensorDevId, i4SensorOpenIndex);
    else if (eSensorType == NSSensorType::eYUV)
        return Hal3AYuv::createInstance(i4SensorDevId, i4SensorOpenIndex);
    else
    {
        static Hal3AIf singleton;
        return &singleton;
    }
#endif
}

void NS3Av3::UtilConvertCamInfo(const NSIspTuning::RAWIspCamInfo_U& rCamInfoU, NSIspTuning::RAWIspCamInfo& rCamInfoT)
{
    rCamInfoT.u4Id                  = rCamInfoU.u4Id                    ;
    rCamInfoT.eIspProfile           = rCamInfoU.eIspProfile             ;
    rCamInfoT.eSensorMode           = rCamInfoU.eSensorMode             ;
    rCamInfoT.eIdx_Scene            = rCamInfoU.eIdx_Scene              ;
    rCamInfoT.eIdx_Effect           = rCamInfoU.eIdx_Effect             ;
    rCamInfoT.rIspUsrSelectLevel    = rCamInfoU.rIspUsrSelectLevel      ;
    rCamInfoT.u4ISOValue            = rCamInfoU.u4ISOValue              ;
    rCamInfoT.eIdx_ISO              = rCamInfoU.eIdx_ISO                ;
    rCamInfoT.eIdx_PCA_LUT          = rCamInfoU.eIdx_PCA_LUT            ;
    rCamInfoT.eIdx_CCM              = rCamInfoU.eIdx_CCM                ;
    rCamInfoT.eIdx_Shading_CCT      = rCamInfoU.eIdx_Shading_CCT        ;
    rCamInfoT.rAWBInfo              = rCamInfoU.rAWBInfo                ;
    rCamInfoT.rAEInfo               = rCamInfoU.rAEInfo                 ;
    rCamInfoT.rAFInfo               = rCamInfoU.rAFInfo                 ;
    rCamInfoT.rFlashInfo            = rCamInfoU.rFlashInfo              ;
    rCamInfoT.rIspIntInfo           = rCamInfoU.rIspIntInfo             ;
    rCamInfoT.i4ZoomRatio_x100      = rCamInfoU.i4ZoomRatio_x100        ;
    rCamInfoT.i4LightValue_x10      = rCamInfoU.i4LightValue_x10        ;
    rCamInfoT.fgRPGEnable           = rCamInfoU.fgRPGEnable             ;
    rCamInfoT.eColorCorrectionMode  = rCamInfoU.eColorCorrectionMode    ;
    rCamInfoT.rMtkCCM               = rCamInfoU.rMtkCCM                 ;
    rCamInfoT.rMtkGGM               = rCamInfoU.rMtkGGM                 ;
    rCamInfoT.rColorCorrectionTransform = rCamInfoU.rColorCorrectionTransform;
    rCamInfoT.eEdgeMode             = rCamInfoU.eEdgeMode               ;
    rCamInfoT.eNRMode               = rCamInfoU.eNRMode                 ;
    rCamInfoT.eToneMapMode          = rCamInfoU.eToneMapMode            ;
    rCamInfoT.fgToneMapInvert       = rCamInfoU.fgToneMapInvert         ;
}
void NS3Av3::UtilConvertCamInfo(const NSIspTuning::RAWIspCamInfo& rCamInfoT, NSIspTuning::RAWIspCamInfo_U& rCamInfoU)
{
    rCamInfoU.u4Id                  = rCamInfoT.u4Id                    ;
    rCamInfoU.eIspProfile           = rCamInfoT.eIspProfile             ;
    rCamInfoU.eSensorMode           = rCamInfoT.eSensorMode             ;
    rCamInfoU.eIdx_Scene            = rCamInfoT.eIdx_Scene              ;
    rCamInfoU.eIdx_Effect           = rCamInfoT.eIdx_Effect             ;
    rCamInfoU.rIspUsrSelectLevel    = rCamInfoT.rIspUsrSelectLevel      ;
    rCamInfoU.u4ISOValue            = rCamInfoT.u4ISOValue              ;
    rCamInfoU.eIdx_ISO              = rCamInfoT.eIdx_ISO                ;
    rCamInfoU.eIdx_PCA_LUT          = rCamInfoT.eIdx_PCA_LUT            ;
    rCamInfoU.eIdx_CCM              = rCamInfoT.eIdx_CCM                ;
    rCamInfoU.eIdx_Shading_CCT      = rCamInfoT.eIdx_Shading_CCT        ;
    rCamInfoU.rAWBInfo              = rCamInfoT.rAWBInfo                ;
    rCamInfoU.rAEInfo               = rCamInfoT.rAEInfo                 ;
    rCamInfoU.rAFInfo               = rCamInfoT.rAFInfo                 ;
    rCamInfoU.rFlashInfo            = rCamInfoT.rFlashInfo              ;
    rCamInfoU.rIspIntInfo           = rCamInfoT.rIspIntInfo             ;
    rCamInfoU.i4ZoomRatio_x100      = rCamInfoT.i4ZoomRatio_x100        ;
    rCamInfoU.i4LightValue_x10      = rCamInfoT.i4LightValue_x10        ;
    rCamInfoU.fgRPGEnable           = rCamInfoT.fgRPGEnable             ;
    rCamInfoU.eColorCorrectionMode  = rCamInfoT.eColorCorrectionMode    ;
    rCamInfoU.rMtkCCM               = rCamInfoT.rMtkCCM                 ;
    rCamInfoU.rMtkGGM               = rCamInfoT.rMtkGGM                 ;
    rCamInfoU.rColorCorrectionTransform = rCamInfoT.rColorCorrectionTransform;
    rCamInfoU.eEdgeMode             = rCamInfoT.eEdgeMode               ;
    rCamInfoU.eNRMode               = rCamInfoT.eNRMode                 ;
    rCamInfoU.eToneMapMode          = rCamInfoT.eToneMapMode            ;
    rCamInfoU.fgToneMapInvert       = rCamInfoT.fgToneMapInvert         ;
}

#endif
