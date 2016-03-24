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
#define MTK_LOG_ENABLE 1
#define LOG_TAG "cct_feature"
//
#include <utils/Errors.h>
#include <cutils/log.h>

#include <mtkcam/acdk/cct_feature.h>
#include "sensor_drv.h"
#include "cct_main.h"
#include "cct_imp.h"
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <mtkcam/hal/aaa_hal_base.h>
#include <awb_param.h>
#include "awb_mgr_if.h"
#include <mcu_drv.h>
#include <mtkcam/drv/isp_reg.h>
#include <af_param.h>
#include "af_mgr_if.h"
#include <mtkcam/common.h>
using namespace NSCam;
#include <ae_param.h>
#include "ae_mgr_if.h"
#include "flash_mgr.h"
#include <nvbuf_util.h>


//


/*******************************************************************************
*
********************************************************************************/
#define MY_LOG(fmt, arg...)    ALOGD(fmt, ##arg)
#define MY_ERR(fmt, arg...)    ALOGE("Err: %5d: " fmt, __LINE__, ##arg)

#define FIXED 0
/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::captureFeatureControl(MUINT32 a_u4Ioctl,
                                       MUINT8 *puParaIn,
                                       MUINT32 u4ParaInLen,
                                       MUINT8 *puParaOut,
                                       MUINT32 u4ParaOutLen,
                                       MUINT32 *pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    MUINT32 *pu32In = (MUINT32*)puParaIn;
    MUINT32 *pu32Out = (MUINT32 *)puParaOut;

    switch (a_u4Ioctl) {
    case ACDK_CCT_GET_CAPTURE_FEATURE_PARAS:
        {
            NVRAM_CAMERA_FEATURE_STRUCT* pNvram;
            NVRAM_CAMERA_FEATURE_STRUCT* pCctNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_STRUCT*>(puParaOut);
            err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram, 1);
            *pCctNvram = *pNvram;
        }
        break;
    case ACDK_CCT_SAVE_CAPTURE_FEATURE_PARAS:
        {
            err = NvBufUtil::getInstance().write(CAMERA_NVRAM_DATA_FEATURE, mSensorDev);
        }
        break;
    case ACDK_CCT_APPLY_CAPTURE_FEATURE_PARAS:
        {
            NVRAM_CAMERA_FEATURE_STRUCT* pNvram;
            NVRAM_CAMERA_FEATURE_STRUCT* pCctNvram = reinterpret_cast<NVRAM_CAMERA_FEATURE_STRUCT*>(puParaIn);

            #if 1
            err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram);
            *pNvram = *pCctNvram;
            #else
            //test only
            MUINT32 th_old;
            MUINT32 th_apply;
            MUINT32 th_new;

            err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pNvram);

            //test only
            th_old = pNvram->mfll.mfll_iso_th;
            th_apply = pCctNvram->mfll.mfll_iso_th;

            *pNvram = *pCctNvram;

            //test only
            NVRAM_CAMERA_FEATURE_STRUCT* pReadNvram;
            err = NvBufUtil::getInstance().getBuf(CAMERA_NVRAM_DATA_FEATURE, mSensorDev, (void*&)pReadNvram);
            th_new = pReadNvram->mfll.mfll_iso_th;
            MY_LOG("apply mfll_iso_th old(%d) apply(%d) new(%d)"
                , th_old
                , th_apply
                , th_new
                );
            #endif
        }
        break;
    default:
        ;
    }

    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::sensorCCTFeatureControl(MUINT32 a_u4Ioctl,
                                       MUINT8 *puParaIn,
                                       MUINT32 u4ParaInLen,
                                       MUINT8 *puParaOut,
                                       MUINT32 u4ParaOutLen,
                                       MUINT32 *pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    MUINT32 *pu32In = (MUINT32*)puParaIn;
    MUINT32 *pu32Out = (MUINT32 *)puParaOut;

    switch (a_u4Ioctl) {
#if 1
    case ACDK_CCT_OP_READ_SENSOR_REG:
        err = CCTOReadSensorReg(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_WRITE_SENSOR_REG:
        err = CCTOPWriteSensorReg(puParaIn);
        break;
#endif
    case ACDK_CCT_OP_QUERY_SENSOR:
        err = CCTOPQuerySensor(puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_GET_SENSOR_RESOLUTION:
        err = CCTOPGetSensorRes(puParaOut, pu4RealParaOutLen);
        break;
#if 0
    case ACDK_CCT_OP_GET_LSC_SENSOR_RESOLUTION:
        err = CCTOPGetLSCSensorRes(puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_COUNT:
        err = CCTOPGetEngSensorGroupCount(pu32Out, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_GROUP_PARA:
        err = CCTOPGetEngSensorGroupPara(*pu32In, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_GET_ENG_SENSOR_PARA:
        err = CCTOPGetEngSensorPara(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_SET_ENG_SENSOR_PARA:
        err = CCTOPSetEngSensorPara(puParaIn );
        break;
    case ACDK_CCT_OP_GET_SENSOR_PREGAIN:
        err = CCTOPGetSensorPregain(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_SET_SENSOR_PREGAIN:
        err = CCTOPSetSensorPregain(puParaIn);
        break;
    case ACDK_CCT_OP_GET_SENSOR_INFO:
        err = CCTOPGetSensorInfo(puParaIn, puParaOut, pu4RealParaOutLen);
        break;
#endif
    default:
        ;
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 CctImp::aaaCCTFeatureControl(MUINT32 a_u4Ioctl,
                                    MUINT8 *puParaIn ,
                                    MUINT32 u4ParaInLen,
                                    MUINT8 *puParaOut,
                                    MUINT32 u4ParaOutLen,
                                    MUINT32 *pu4RealParaOutLen
)
{
    int ret;
    MINT32 err = CCTIF_NO_ERROR;
    MINT32 *i32In = (MINT32 *)puParaIn;
    MUINT32 *u32In = (MUINT32 *)puParaIn;

    switch (a_u4Ioctl)
    {
    // AE
    case ACDK_CCT_OP_AE_ENABLE:
        err = NS3A::IAeMgr::getInstance().CCTOPAEEnable(mSensorDev);
        break;
    case ACDK_CCT_OP_AE_DISABLE:
        err = NS3A::IAeMgr::getInstance().CCTOPAEDisable(mSensorDev);
        break;
    case ACDK_CCT_OP_AE_GET_ENABLE_INFO:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetEnableInfo(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_SET_SCENE_MODE:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetAEScene(mSensorDev, *i32In);
        break;
    case ACDK_CCT_OP_DEV_AE_GET_INFO:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetNVRAMParam(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_SCENE_MODE:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetAEScene(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_SET_METERING_MODE:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetMeteringMode(mSensorDev, *i32In);
        break;
    case ACDK_CCT_V2_OP_AE_APPLY_EXPO_INFO:
        err = NS3A::IAeMgr::getInstance().CCTOPAEApplyExpParam(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AE_SELECT_BAND:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetFlickerMode(mSensorDev, *i32In);
        break;
    case ACDK_CCT_V2_OP_AE_GET_AUTO_EXPO_PARA:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetExpParam(mSensorDev, (VOID *)puParaIn, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_BAND:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetFlickerMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AE_GET_METERING_RESULT:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetMeteringMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_APPLY_INFO:
        err = NS3A::IAeMgr::getInstance().CCTOPAEApplyNVRAMParam(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_OP_DEV_AE_SAVE_INFO_NVRAM:
        err = NS3A::IAeMgr::getInstance().CCTOPAESaveNVRAMParam(mSensorDev);
        break;
    case ACDK_CCT_OP_DEV_AE_GET_EV_CALIBRATION:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetCurrentEV(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_LOCK_EXPOSURE_SETTING:
        err = NS3A::IAeMgr::getInstance().CCTOPAELockExpSetting(mSensorDev);
        break;
    case ACDK_CCT_OP_AE_UNLOCK_EXPOSURE_SETTING:
        err = NS3A::IAeMgr::getInstance().CCTOPAEUnLockExpSetting(mSensorDev);
        break;
    case ACDK_CCT_OP_AE_GET_ISP_OB:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetIspOB(mSensorDev, (MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_SET_ISP_OB:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetIspOB(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_GET_ISP_RAW_GAIN:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetIspRAWGain(mSensorDev, (MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AE_SET_ISP_RAW_GAIN:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetIspRAWGain(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_EXP_TIME:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetSensorExpTime(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_EXP_LINE:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetSensorExpLine(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_SET_SENSOR_GAIN:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetSensorGain(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_CAPTURE_MODE:
        err = NS3A::IAeMgr::getInstance().CCTOPAESetCaptureMode(mSensorDev, *u32In);
        break;
    case ACDK_CCT_OP_AE_GET_CAPTURE_PARA:
        err = NS3A::IAeMgr::getInstance().CCTOGetCaptureParams(mSensorDev, (VOID *)puParaOut);
        break;
    case ACDK_CCT_OP_AE_SET_CAPTURE_PARA:
        err = NS3A::IAeMgr::getInstance().CCTOSetCaptureParams(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_OP_DEV_AE_GET_FLARE_CALIBRATION:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetFlareOffset(mSensorDev, *u32In, (MUINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_SAVE_PLINE:
        err = NS3A::IAeMgr::getInstance().CCTOPAESavePlineNVRAM(mSensorDev);
        break;
    case ACDK_CCT_OP_DEV_AE_GET_PLINE:
        err = NS3A::IAeMgr::getInstance().CCTOPAEGetPlineNVRAM(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_DEV_AE_APPLY_PLINE:
        err = NS3A::IAeMgr::getInstance().CCTOPAEApplyPlineNVRAM(mSensorDev, (VOID *)puParaIn);
        break;
    // AWB
    case ACDK_CCT_V2_OP_AWB_ENABLE_AUTO_RUN:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBEnable(mSensorDev);
        break;
    case ACDK_CCT_V2_OP_AWB_DISABLE_AUTO_RUN:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBDisable(mSensorDev);
        break;
    case ACDK_CCT_V2_OP_AWB_GET_AUTO_RUN_INFO:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBGetEnableInfo(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_GET_GAIN:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBGetAWBGain(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_SET_GAIN:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBSetAWBGain(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AWB_APPLY_CAMERA_PARA2:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBApplyNVRAMParam(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AWB_GET_AWB_PARA:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBGetNVRAMParam(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AWB_SAVE_AWB_PARA:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBSaveNVRAMParam(mSensorDev);
        break;
    case ACDK_CCT_OP_AWB_SET_AWB_MODE:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBSetAWBMode(mSensorDev, *i32In);
        break;
    case ACDK_CCT_OP_AWB_GET_AWB_MODE:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBGetAWBMode(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AWB_GET_LIGHT_PROB:
        err = NS3A::IAwbMgr::getInstance().CCTOPAWBGetLightProb(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AWB_APPLY_FLASH_AWB_PARA:
        err = NS3A::IAwbMgr::getInstance().CCTOPFlashAWBApplyNVRAMParam(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_OP_AWB_GET_FLASH_AWB_PARA:
        err = NS3A::IAwbMgr::getInstance().CCTOPFlashAWBGetNVRAMParam(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AWB_SAVE_FLASH_AWB_PARA:
        err = NS3A::IAwbMgr::getInstance().CCTOPFlashAWBSaveNVRAMParam(mSensorDev);
        break;
    // AF
    case ACDK_CCT_V2_OP_AF_OPERATION:
        err = NS3A::IAfMgr::getInstance().CCTOPAFOpeartion(mSensorDev);
        break;
    case ACDK_CCT_V2_OP_MF_OPERATION:
        // send different pos to ensure MF_OPERATION will take effect
        if( *i32In > 0 )
        {
             err = NS3A::IAfMgr::getInstance().CCTOPMFOpeartion(mSensorDev, (*i32In)-1);
        }
        err = NS3A::IAfMgr::getInstance().CCTOPMFOpeartion(mSensorDev, *i32In);
        break;
    case ACDK_CCT_V2_OP_GET_AF_INFO:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetAFInfo(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_GET_BEST_POS:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetBestPos(mSensorDev, (MINT32 *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_CALI_OPERATION:
        err = NS3A::IAfMgr::getInstance().CCTOPAFCaliOperation(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_SET_RANGE:
        err = NS3A::IAfMgr::getInstance().CCTOPAFSetFocusRange(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AF_GET_RANGE:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetFocusRange(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_SAVE_TO_NVRAM:
        err = NS3A::IAfMgr::getInstance().CCTOPAFSaveNVRAMParam(mSensorDev);
        break;
    case ACDK_CCT_V2_OP_AF_READ:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetNVRAMParam(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_V2_OP_AF_APPLY:
        err = NS3A::IAfMgr::getInstance().CCTOPAFApplyNVRAMParam(mSensorDev, (VOID *)puParaIn);
        break;
    case ACDK_CCT_V2_OP_AF_GET_FV:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetFV(mSensorDev, (VOID *)puParaIn, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_OP_AF_ENABLE:
        err = NS3A::IAfMgr::getInstance().CCTOPAFEnable(mSensorDev);
        break;
    case ACDK_CCT_OP_AF_DISABLE:
        err = NS3A::IAfMgr::getInstance().CCTOPAFDisable(mSensorDev);
        break;
    case ACDK_CCT_OP_AF_GET_ENABLE_INFO:
        err = NS3A::IAfMgr::getInstance().CCTOPAFGetEnableInfo(mSensorDev, (VOID *)puParaOut, pu4RealParaOutLen);
        break;
    case ACDK_CCT_AF_INIT:
        err = NS3A::IAfMgr::getInstance().init(mSensorDev, 1/*sensoridx*/);
        MY_LOG("ACDK_CCT_AF_INIT: IAfMgr init() done\n");
        break;
    case ACDK_CCT_AF_UNINIT:
        err = NS3A::IAfMgr::getInstance().uninit(mSensorDev);
        MY_LOG("ACDK_CCT_AF_UNINIT: IAfMgr uninit() done\n");
        break;
    case ACDK_CCT_MCU_NAME_INIT:
        err = NS3A::IAfMgr::getInstance().CCTMCUNameinit(mSensorDev, 1/*sensoridx*/);
        MY_LOG("ACDK_CCT_MCU_NAME_INIT: IAfMgr init() done\n");
        break;
    case ACDK_CCT_MCU_NAME_UNINIT:
        err = NS3A::IAfMgr::getInstance().CCTMCUNameuninit(mSensorDev);
        MY_LOG("ACDK_CCT_MCU_NAME_UNINIT: IAfMgr uninit() done\n");
        break;

    //--------------------------
    //flash
    case ACDK_CCT_OP_STROBE_WRITE_NVRAM:
        MY_LOG("ACDK_CCT_OP_STROBE_WRITE_NVRAM");
        ret = FlashMgr::getInstance().cctWriteNvram(mSensorDev);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_STROBE_SET_NVDATA_META:
        MY_LOG("ACDK_CCT_OP_STROBE_SET_NVDATA_META");
        ret = FlashMgr::getInstance().cctSetNvdataMeta(mSensorDev, puParaIn, u4ParaInLen);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META:
        MY_LOG("ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META");
        ret = FlashMgr::getInstance().cctReadNvramToPcMeta(mSensorDev, puParaOut, pu4RealParaOutLen);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_CONTROL:
        MY_LOG("ACDK_CCT_OP_FLASH_CONTROL line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().cctFlashLightTest(mSensorDev,(VOID *)puParaIn);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_ENABLE:
        MY_LOG("ACDK_CCT_OP_FLASH_ENABLE line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 1); //YosenFlash
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_DISABLE:
        MY_LOG("ACDK_CCT_OP_FLASH_DISABLE line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 0); //YosenFlash
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_GET_INFO:
        MY_LOG("ACDK_CCT_OP_FLASH_GET_INFO line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().cctGetFlashInfo(mSensorDev, (int*)puParaOut);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_SET_MANUAL_FLASH:
        MY_LOG("ACDK_CCT_OP_FLASH_SET_MANUAL_FLASH line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().setManualFlash(mSensorDev, i32In[0], i32In[1]);
        if(ret==1)
            err=0;
        else
            err=-1;
        break;
    case ACDK_CCT_OP_FLASH_CLEAR_MANUAL_FLASH:
        MY_LOG("ACDK_CCT_OP_FLASH_CLEAR_MANUAL_FLASH line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().clearManualFlash(mSensorDev);
        if(ret==1)
            err=0;
        else
            err=-1;

        MY_LOG("ACDK_CCT_OP_FLASH_ENABLE line=%d\n",__LINE__);
        ret = FlashMgr::getInstance().cctFlashEnable(mSensorDev, 1);
        if(ret==1)
            err=0;
        else
            err=-1;

        break;
    //-----------------------------------
#if 0
    //----------------------------
    // flash
    case ACDK_CCT_OP_FLASH_GET_INFO:
        MY_LOG("ACDK_CCT_OP_FLASH_GET_INFO line=%d\n",__LINE__);
        //Temp. Mark
        err = FlashMgr::getInstance()->cctGetFlashInfo(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
        break;


    case ACDK_CCT_OP_FLASH_CONTROL:
        //Temp. Mark
        err = FlashMgr::getInstance()->cctFlashLightTest((VOID *)puParaIn);
        break;
       case ACDK_CCT_OP_FLASH_ENABLE:
           err = FlashMgr::getInstance()->cctFlashEnable(1); //YosenFlash
        break;
    case ACDK_CCT_OP_FLASH_DISABLE:
           err = FlashMgr::getInstance()->cctFlashEnable(0); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_READ_NVRAM:    //5:
        err = FlashMgr::getInstance()->cctReadNvram(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_WRITE_NVRAM:    //6
        err = FlashMgr::getInstance()->cctWriteNvram(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_READ_DEFAULT_NVRAM:    //7
        err = FlashMgr::getInstance()->cctReadDefaultNvram(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_SET_PARAM:    //8
        err = FlashMgr::getInstance()->cctSetParam(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_GET_PARAM:    //9
        err = FlashMgr::getInstance()->cctGetParam(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_GET_NVDATA: //10:
        err = FlashMgr::getInstance()->cctGetNvdata(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_SET_NVDATA: //11:
        err = FlashMgr::getInstance()->cctSetNvdata(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_GET_ENG_Y:    //12:
        err = FlashMgr::getInstance()->cctGetEngY(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_SET_ENG_Y:    //13
        err = FlashMgr::getInstance()->cctSetEngY(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_GET_ENG_RG:    //14
        err = FlashMgr::getInstance()->cctGetEngRg(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_SET_ENG_RG:    //15
        err = FlashMgr::getInstance()->cctSetEngRg(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_GET_ENG_BG:    //16
        err = FlashMgr::getInstance()->cctGetEngBg(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_SET_ENG_BG:    //17
        err = FlashMgr::getInstance()->cctSetEngBg(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_NVDATA_TO_FILE:    //18:
        err = FlashMgr::getInstance()->cctNvdataToFile(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_FILE_TO_NVDATA:    //19,
        err = FlashMgr::getInstance()->cctFileToNvdata(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); //YosenFlash
        break;
    case ACDK_CCT_OP_STROBE_READ_NVRAM_TO_PC_META:    //20,
        err = FlashMgr::getInstance()->cctReadNvramToPcMeta(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
        break;
  case ACDK_CCT_OP_STROBE_SET_NVDATA_META:    //21,
        err = FlashMgr::getInstance()->cctSetNvdataMeta(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
        break;
#endif
    default:
        ;
    }

    return err;

}


/*******************************************************************************
* ISP
********************************************************************************/
MINT32 CctImp::ispCCTFeatureControl(MUINT32 a_u4Ioctl,
                                    MUINT8 *puParaIn,
                                    MUINT32 u4ParaInLen,
                                    MUINT8 *puParaOut,
                                    MUINT32 u4ParaOutLen,
                                    MUINT32 *pu4RealParaOutLen
)
{
    if  ( ! m_pCctCtrl )
        return  CCTIF_NOT_INIT;
    return  m_pCctCtrl->cctFeatureCtrl_isp(a_u4Ioctl, puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
}


MINT32
CctCtrl::
cctFeatureCtrl_isp(
    MUINT32 const a_u4Ioctl,
    MUINT8*const puParaIn,
    MUINT32 const u4ParaInLen,
    MUINT8*const puParaOut,
    MUINT32 const u4ParaOutLen,
    MUINT32*const pu4RealParaOutLen
)
{
#define DO_CCT_CTRL(ctl_cocde)  \
    case ctl_cocde: \
        err = doCctFeatureCtrl<ctl_cocde>(puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen); \
        break

    MINT32 err = CCTIF_NO_ERROR;

    switch (a_u4Ioctl)
    {
    // ISP
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_READ_REG );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_WRITE_REG );
    DO_CCT_CTRL( ACDK_CCT_OP_QUERY_ISP_ID );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_INDEX );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_INDEX );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_TUNING_PARAS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_TUNING_PARAS );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_MFB_MIXER_PARAM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_MFB_MIXER_PARAM );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_ENABLE_DYNAMIC_BYPASS_MODE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_DISABLE_DYNAMIC_BYPASS_MODE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_DYNAMIC_BYPASS_MODE_ON_OFF );

    DO_CCT_CTRL( ACDK_CCT_V2_OP_QUERY_ISP_INDEX_SET );
    DO_CCT_CTRL( ACDK_CCT_D1_OP_ISP_GET_DNG_PARA );
    DO_CCT_CTRL( ACDK_CCT_D1_OP_ISP_SET_DNG_PARA );
#if 1
    // GAMMA
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_SET_GAMMA_BYPASS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AE_GET_GAMMA_BYPASS_FLAG );

    // CCM
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_ENABLE_DYNAMIC_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_DISABLE_DYNAMIC_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_STATUS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_STATUS );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CCM_PARA );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_UPDATE_CCM_PARA );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_CURRENT_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_CURRENT_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_GET_NVRAM_CCM );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_AWB_SET_NVRAM_CCM );
    DO_CCT_CTRL( ACDK_CCT_OP_SET_CCM_MODE );
    DO_CCT_CTRL( ACDK_CCT_OP_GET_CCM_MODE );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_SET_DYNAMIC_CCM_COEFF );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_GET_DYNAMIC_CCM_COEFF );
    DO_CCT_CTRL( ACDK_CCT_OP_SET_CCM_WB );
    DO_CCT_CTRL( ACDK_CCT_OP_GET_CCM_WB );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_CCM_SET_SMOOTH_SWITCH );
    DO_CCT_CTRL( ACDK_CCT_V2_OP_CCM_GET_SMOOTH_SWITCH );
#endif

    // ISP Common Control
    DO_CCT_CTRL( ACDK_CCT_OP_SET_ISP_ON );
    DO_CCT_CTRL( ACDK_CCT_OP_SET_ISP_OFF );
    DO_CCT_CTRL( ACDK_CCT_OP_GET_ISP_ON_OFF );

    // NVRAM
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_LOAD_FROM_NVRAM );
    DO_CCT_CTRL( ACDK_CCT_OP_3A_LOAD_FROM_NVRAM );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SAVE_TO_NVRAM );
    // Shading Table NVRAM
    DO_CCT_CTRL( ACDK_CCT_OP_SDTBL_LOAD_FROM_NVRAM );
    DO_CCT_CTRL( ACDK_CCT_OP_SDTBL_SAVE_TO_NVRAM );

    //  PCA
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_TABLE );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_PARA );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_PARA );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_SET_PCA_SLIDER );
    DO_CCT_CTRL( ACDK_CCT_OP_ISP_GET_PCA_SLIDER );


    // Shading/Defect
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_ON_OFF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_ON_OFF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_PARA);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_PARA);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_INDEX);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_INDEX);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSFAWB_FORCE);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TSF_ONOFF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TSF_ONOFF);
//    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_V3);
//    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_V3);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_SET_SHADING_TABLE_POLYCOEF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_SHADING_TABLE_POLYCOEF);
    DO_CCT_CTRL( ACDK_CCT_V2_OP_ISP_GET_NVRAM_DATA);
    case ACDK_CCT_V2_ISP_DEFECT_TABLE_ON:
    case ACDK_CCT_V2_ISP_DEFECT_TABLE_OFF:
    case ACDK_CCT_OP_SET_CALI_MODE:
        break;

    default:
        err = CCTIF_BAD_CTRL_CODE;
        break;
    }

    return err;
}


/*******************************************************************************
* NVRAM
********************************************************************************/
MINT32 CctImp::nvramCCTFeatureControl(MUINT32 a_u4Ioctl,
                                      MUINT8 *puParaIn,
                                      MUINT32 u4ParaInLen,
                                      MUINT8 *puParaOut,
                                      MUINT32 u4ParaOutLen,
                                      MUINT32 *pu4RealParaOutLen
)
{
    MY_LOG("CctImp::nvramCCTFeatureControl \n");
    if  ( ! m_pCctCtrl )
        return  CCTIF_NOT_INIT;
    return  m_pCctCtrl->cctFeatureCtrl_nvram(a_u4Ioctl, puParaIn, u4ParaInLen, puParaOut, u4ParaOutLen, pu4RealParaOutLen);
}


MINT32
CctCtrl::
cctFeatureCtrl_nvram(
    MUINT32 const a_u4Ioctl,
    MUINT8*const puParaIn,
    MUINT32 const u4ParaInLen,
    MUINT8*const puParaOut,
    MUINT32 const u4ParaOutLen,
    MUINT32*const pu4RealParaOutLen
)
{
    MINT32 err = CCTIF_NO_ERROR;
    switch (a_u4Ioctl)
    {
//    DO_CCT_CTRL( ACDK_CCT_OP_LOAD_FROM_NVRAM );
//    DO_CCT_CTRL( ACDK_CCT_OP_SAVE_TO_NVRAM );
    default:
        err = CCTIF_BAD_CTRL_CODE;
        break;
    }
    return  err;
}

