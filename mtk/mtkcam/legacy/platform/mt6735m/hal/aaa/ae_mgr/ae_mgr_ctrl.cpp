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
#define LOG_TAG "ae_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <camera_custom_nvram.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <camera_custom_AEPlinetable.h>
#include <mtkcam/common.h>
using namespace NSCam;
#include <mtkcam/common/faces.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/hal/sensor_hal.h>
#include <nvram_drv_mgr.h>
#include <ae_tuning_custom.h>
#include <isp_mgr.h>
#include <isp_tuning.h>
#include <camera_feature.h>
#include <isp_tuning_cam_info.h>
#include <isp_tuning_mgr.h>
#include <aaa_sensor_mgr.h>
#include "camera_custom_hdr.h"
#include <camera_feature.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <ae_mgr.h>
#include <mtkcam/featureio/nvbuf_util.h>
//#include "aaa_state_flow_custom.h"

using namespace NS3Av3;
using namespace NSIspTuning;
using namespace NSIspTuningv3;
using namespace NSFeature;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::UpdateSensorParams(AE_SENSOR_PARAM_T strSensorParams)
{
    if(m_SensorQueueCtrl.uInputIndex >= AE_SENSOR_MAX_QUEUE) {
        m_SensorQueueCtrl.uInputIndex = 0;
    }

    if(strSensorParams.u8FrameDuration < strSensorParams.u8ExposureTime) {
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration = (MUINT32) (strSensorParams.u8ExposureTime / 1000);
        MY_LOG("[%s()] i4SensorDev = %d line:%d Frame Duration less than EXposure time: %lld %lld\n", __FUNCTION__, m_eSensorDev, __LINE__,
        strSensorParams.u8FrameDuration, strSensorParams.u8ExposureTime);
    } else {
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration = (MUINT32) (strSensorParams.u8FrameDuration / 1000);
    }
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4ExposureTime = (MUINT32) (strSensorParams.u8ExposureTime / 1000);
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4Sensitivity = (MUINT32) (strSensorParams.u4Sensitivity);

    MY_LOG("[%s()] i4SensorDev = %d line:%d Idx:%d %d FrameDuration:%d %lld Exposure Time:%d %lld ISO:%d %d\n", __FUNCTION__, m_eSensorDev, __LINE__,
        m_SensorQueueCtrl.uInputIndex, m_SensorQueueCtrl.uOutputIndex,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration, strSensorParams.u8FrameDuration,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4ExposureTime, strSensorParams.u8ExposureTime,
        m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4Sensitivity, strSensorParams.u4Sensitivity);

    m_SensorQueueCtrl.uInputIndex++;
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4FrameDuration = 0;
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4ExposureTime = 0;
    m_SensorQueueCtrl.rSensorParamQueue[m_SensorQueueCtrl.uInputIndex].u4Sensitivity= 0;

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::bBlackLevelLock(MBOOL bLockBlackLevel)
{
    if(m_bLockBlackLevel != bLockBlackLevel) {
        MY_LOG("[%s()] i4SensorDev = %d line:%d BlackLevel:%d %d \n", __FUNCTION__, m_eSensorDev, __LINE__, m_bLockBlackLevel, bLockBlackLevel);
        m_bLockBlackLevel = bLockBlackLevel;
        AAASensorMgr::getInstance().setSensorOBLock((ESensorDev_T)m_eSensorDev, m_bLockBlackLevel);
    }

    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MRESULT AeMgr::getSensorParams(AE_SENSOR_PARAM_T &a_rSensorInfo)
{
    a_rSensorInfo = m_rSensorCurrentInfo;
    return S_AE_OK;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT64 AeMgr::getSensorRollingShutter() const
{
    MUINT32 tline = 0, vsize = 0;
    AAASensorMgr::getInstance().getRollingShutter((ESensorDev_T)m_eSensorDev, tline, vsize);

    return (MINT64) tline * vsize;
}

