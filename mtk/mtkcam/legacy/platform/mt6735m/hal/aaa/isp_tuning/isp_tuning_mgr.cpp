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
#define LOG_TAG "isp_tuning_mgr"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <cutils/properties.h>
#include <stdlib.h>
#include <aaa_types.h>
#include <aaa_log.h>
//#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <camera_custom_nvram.h>
#include <isp_tuning.h>
#include <awb_param.h>
#include <af_param.h>
#include <flash_param.h>
#include <ae_param.h>
#include <isp_tuning_cam_info.h>
#include <paramctrl_if.h>
#include "isp_tuning_mgr.h"

using namespace NSIspTuning;
using namespace NSIspTuningv3;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr&
IspTuningMgr::
getInstance()
{
    static  IspTuningMgr singleton;
    return  singleton;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr::
IspTuningMgr()
    : m_pParamctrl_Main(MNULL)
    , m_pParamctrl_Sub(MNULL)
    , m_pParamctrl_Main2(MNULL)
    , m_i4SensorDev(0)
    , m_bDebugEnable(MFALSE)
    , m_i4IspProfile(0)
    , m_i4SensorIdx(0)
{


}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IspTuningMgr::
~IspTuningMgr()
{

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::init(MINT32 i4SensorDev, MINT32 const i4SensorIdx)
{
    m_i4SensorDev = i4SensorDev;
    m_i4SensorIdx = i4SensorIdx;

    MY_LOG("m_i4SensorDev= %d, m_i4SensorIdx = %d\n", m_i4SensorDev, m_i4SensorIdx);

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.isp_tuning_mgr.enable", value, "0");
    m_bDebugEnable = atoi(value);

    if (i4SensorDev & ESensorDev_Main) {
        if (!m_pParamctrl_Main) {
            m_pParamctrl_Main = IParamctrl::createInstance(ESensorDev_Main, m_i4SensorIdx);
            m_pParamctrl_Main->init();
        }
        else {
            MY_ERR("m_pParamctrl_Main is not NULL");
            return MFALSE;
        }
    }

    if (i4SensorDev & ESensorDev_Sub) {
        if (!m_pParamctrl_Sub) {
            m_pParamctrl_Sub = IParamctrl::createInstance(ESensorDev_Sub, m_i4SensorIdx);
            m_pParamctrl_Sub->init();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is not NULL");
            return MFALSE;
        }
    }

    if (i4SensorDev & ESensorDev_MainSecond) {
        if (!m_pParamctrl_Main2) {
            m_pParamctrl_Main2 = IParamctrl::createInstance(ESensorDev_MainSecond, m_i4SensorIdx);
            m_pParamctrl_Main2->init();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is not NULL");
    return MFALSE;
}
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::uninit()
{
    if (m_pParamctrl_Main) {
        m_pParamctrl_Main->uninit();
        m_pParamctrl_Main->destroyInstance();
        m_pParamctrl_Main = MNULL;
    }

    if (m_pParamctrl_Sub) {
        m_pParamctrl_Sub->uninit();
        m_pParamctrl_Sub->destroyInstance();
        m_pParamctrl_Sub = MNULL;
    }

    if (m_pParamctrl_Main2) {
        m_pParamctrl_Main2->uninit();
        m_pParamctrl_Main2->destroyInstance();
        m_pParamctrl_Main2 = MNULL;
    }

    m_i4SensorDev = 0;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspProfile(MINT32 const i4IspProfile)
{
    MY_LOG_IF(m_bDebugEnable,"setIspProfile: %d\n", i4IspProfile);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspProfile(i4IspProfile);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspProfile(i4IspProfile);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspProfile(i4IspProfile);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    m_i4IspProfile = i4IspProfile;

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSceneMode(MUINT32 const u4Scene)
{
    MY_LOG_IF(m_bDebugEnable,"setSceneMode: %d\n", u4Scene);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setSceneMode(static_cast<EIndex_Scene_T>(u4Scene));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEffect(MUINT32 const u4Effect)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setEffect(static_cast<EIndex_Effect_T>(u4Effect));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setOperMode(MINT32 const i4OperMode)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setOperMode(static_cast<EOperMode_T>(i4OperMode));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MINT32 IspTuningMgr::getOperMode()
{
    MINT32 operMode = EOperMode_Normal;

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             operMode = m_pParamctrl_Main->getOperMode();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            operMode = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            operMode = m_pParamctrl_Sub->getOperMode();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            operMode = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            operMode = m_pParamctrl_Main2->getOperMode();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            operMode = -1;
        }
    }

    if (operMode == -1)
        MY_ERR("Err IspTuningMgr::getOperMode()\n");

    return operMode;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicBypass(MBOOL i4Bypass)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicBypass(static_cast<MBOOL>(i4Bypass));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicBypass()
{
    MINT32 bypass = MFALSE;

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             bypass = m_pParamctrl_Main->isDynamicBypass();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            bypass = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            bypass = m_pParamctrl_Sub->isDynamicBypass();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            bypass = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            bypass = m_pParamctrl_Main2->isDynamicBypass();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            bypass = -1;
        }
    }

    if (bypass == -1)
        MY_ERR("Err IspTuningMgr::getDynamicBypass()\n");

    return bypass;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setDynamicCCM(MBOOL bdynamic_ccm)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicCCM(static_cast<MBOOL>(bdynamic_ccm));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MINT32 IspTuningMgr::getDynamicCCM()
{
    MINT32 bypass = MFALSE;

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             bypass = m_pParamctrl_Main->isDynamicCCM();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            bypass = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            bypass = m_pParamctrl_Sub->isDynamicCCM();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            bypass = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            bypass = m_pParamctrl_Main2->isDynamicCCM();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            bypass = -1;
        }
    }

    if (bypass == -1)
        MY_ERR("Err IspTuningMgr::getDynamicCCM()\n");

    return bypass;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::enableDynamicShading(MBOOL const fgEnable)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->enableDynamicShading(static_cast<MBOOL>(fgEnable));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setSensorMode(MINT32 const i4SensorMode)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setSensorMode(static_cast<ESensorMode_T>(i4SensorMode));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MINT32 IspTuningMgr::getSensorMode()
{
    MINT32 sensorMode = ESensorMode_Capture;

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
             sensorMode = m_pParamctrl_Main->getSensorMode();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            sensorMode = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            sensorMode = m_pParamctrl_Sub->getSensorMode();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            sensorMode = -1;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            sensorMode = m_pParamctrl_Main2->getSensorMode();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            sensorMode = -1;
        }
    }

    if (sensorMode == -1)
        MY_ERR("Err IspTuningMgr::getSensorMode()\n");

    return sensorMode;

}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setZoomRatio(MINT32 const i4ZoomRatio_x100)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
}

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
}

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setZoomRatio(i4ZoomRatio_x100);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBInfo(AWB_INFO_T const &rAWBInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAWBInfo(rAWBInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
}

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAWBInfo(rAWBInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAWBInfo(rAWBInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
}

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAEInfo(AE_INFO_T const &rAEInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAEInfo(rAEInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAEInfo(rAEInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAEInfo(rAEInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAFInfo(AF_INFO_T const &rAFInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAFInfo(rAFInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAFInfo(rAFInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAFInfo(rAFInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setFlashInfo(FLASH_INFO_T const &rFlashInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setFlashInfo(rFlashInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setFlashInfo(rFlashInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setFlashInfo(rFlashInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setPureOBCInfo(const ISP_NVRAM_OBC_T *pOBCInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setPureOBCInfo(pOBCInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setPureOBCInfo(pOBCInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setPureOBCInfo(pOBCInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

MBOOL IspTuningMgr::setIndex_Shading(MINT32 const i4IDX)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIndex_Shading(i4IDX);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIndex_Shading(i4IDX);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIndex_Shading(i4IDX);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

MBOOL IspTuningMgr::getIndex_Shading(MVOID*const pCmdArg)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getIndex_Shading(pCmdArg);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getIndex_Shading(pCmdArg);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getIndex_Shading(pCmdArg);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getCamInfo(RAWIspCamInfo& rCamInfo) const
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            rCamInfo = m_pParamctrl_Main->getCamInfo();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            rCamInfo = m_pParamctrl_Sub->getCamInfo();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            rCamInfo = m_pParamctrl_Main2->getCamInfo();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::getColorCorrectionTransform()\n");
        return MFALSE;
    }
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Edge(MUINT32 const u4Index)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Edge(static_cast<EIndex_Isp_Edge_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Hue(MUINT32 const u4Index)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Hue(static_cast<EIndex_Isp_Hue_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Sat(MUINT32 const u4Index)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Sat(static_cast<EIndex_Isp_Saturation_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Bright(MUINT32 const u4Index)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Bright(static_cast<EIndex_Isp_Brightness_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspUserIdx_Contrast(MUINT32 const u4Index)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspUserIdx_Contrast(static_cast<EIndex_Isp_Contrast_T>(u4Index));
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validate(MINT32 const i4FrameID, MBOOL const fgForce)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validate(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validate(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validate(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrame(MINT32 const i4FrameID, MBOOL const fgForce)
{
//    NS3Av3::AaaTimer localTimer("validatePerFrame", m_i4SensorDev, (NS3Av3::Hal3A::sm_3ALogEnable & EN_3A_TIMER_LOG));
    MY_LOG_IF(m_bDebugEnable,"%s(): m_i4SensorDev = %d\n", __FUNCTION__, m_i4SensorDev);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validatePerFrame(i4FrameID, fgForce);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
//    localTimer.printTime();
    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::validatePerFrameP2(MINT32 flowType, const RAWIspCamInfo& rIspCamInfo, void* pRegBuf)
{
    MY_LOG_IF(m_bDebugEnable,"%s(): m_i4SensorDev = %d\n", __FUNCTION__, m_i4SensorDev);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->validatePerFrameP2(flowType, rIspCamInfo, pRegBuf);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->validatePerFrameP2(flowType, rIspCamInfo, pRegBuf);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->validatePerFrameP2(flowType, rIspCamInfo, pRegBuf);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getDebugInfo(NSIspExifDebug::IspExifDebugInfo_T& rIspExifDebugInfo)
{
    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getDebugInfo(rIspExifDebugInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getDebugInfo(rIspExifDebugInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }

    if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getDebugInfo(rIspExifDebugInfo);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }

    return MTRUE;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setAWBGain(AWB_GAIN_T& rIspAWBGain)
{

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setAWBGain(rIspAWBGain);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setAWBGain(rIspAWBGain);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setAWBGain(rIspAWBGain);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setAWBGain()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setIspAEPreGain2(MINT32 i4SensorIndex, AWB_GAIN_T& rNewIspAEPreGain2)
{

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setIspAEPreGain2(i4SensorIndex, rNewIspAEPreGain2);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setIspAEPreGain2()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::forceValidate()
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->forceValidate();
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->forceValidate();
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->forceValidate();
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::forceValidate()\n");
        return MFALSE;
    }

    return MTRUE;
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getColorCorrectionTransform(MFLOAT& M11, MFLOAT& M12, MFLOAT& M13,
                                                 MFLOAT& M21, MFLOAT& M22, MFLOAT& M23,
                                                 MFLOAT& M31, MFLOAT& M32, MFLOAT& M33)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::getColorCorrectionTransform()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionTransform(MFLOAT M11, MFLOAT M12, MFLOAT M13,
                                                 MFLOAT M21, MFLOAT M22, MFLOAT M23,
                                                 MFLOAT M31, MFLOAT M32, MFLOAT M33)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setColorCorrectionTransform(M11, M12, M13, M21, M22, M23, M31, M32, M33);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setColorCorrectionTransform()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setColorCorrectionMode(MINT32 i4ColorCorrectionMode)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setColorCorrectionMode(i4ColorCorrectionMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setColorCorrectionMode()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setEdgeMode(MINT32 i4EdgeMode)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setEdgeMode(i4EdgeMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setEdgeMode(i4EdgeMode);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setEdgeMode(i4EdgeMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setEdgeMode()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setNoiseReductionMode(MINT32 i4NRMode)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setNoiseReductionMode(i4NRMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setNoiseReductionMode(i4NRMode);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setNoiseReductionMode(i4NRMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setNoiseReductionMode()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setToneMapMode(MINT32 i4ToneMapMode)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setToneMapMode(i4ToneMapMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setToneMapMode(i4ToneMapMode);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setToneMapMode(i4ToneMapMode);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setToneMapMode()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Red(MFLOAT*& p_in_red, MFLOAT*& p_out_red, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::getTonemapCurve_Red()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Green(MFLOAT*& p_in_green, MFLOAT*& p_out_green, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::getTonemapCurve_Green()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::getTonemapCurve_Blue(MFLOAT*& p_in_blue, MFLOAT*& p_out_blue, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->getTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::getTonemapCurve_Blue()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Red(MFLOAT *p_in_red, MFLOAT *p_out_red, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Red(p_in_red, p_out_red, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setTonemapCurve_Red()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Green(MFLOAT *p_in_green, MFLOAT *p_out_green, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Green(p_in_green, p_out_green, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setTonemapCurve_Green()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::setTonemapCurve_Blue(MFLOAT *p_in_blue, MFLOAT *p_out_blue, MINT32 *pCurvePointNum)
{
    MY_LOG_IF(m_bDebugEnable,"%s()\n", __FUNCTION__);

    if (m_i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            m_pParamctrl_Main->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            m_pParamctrl_Sub->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (m_i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            m_pParamctrl_Main2->setTonemapCurve_Blue(p_in_blue, p_out_blue, pCurvePointNum);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::setTonemapCurve_Blue()\n");
        return MFALSE;
    }

    return MTRUE;
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
MBOOL IspTuningMgr::convertPtPairsToGMA(MINT32 const i4SensorDev, const MFLOAT* inPtPairs, MUINT32 u4NumOfPts, MINT32* outGMA)
{
    MY_LOG_IF(m_bDebugEnable,"%s(), i4SensorDev(0x%x)\n", __FUNCTION__, i4SensorDev);
    if (i4SensorDev & ESensorDev_Main) {
        if (m_pParamctrl_Main) {
            return m_pParamctrl_Main->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            MY_ERR("m_pParamctrl_Main is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_Sub) {
        if (m_pParamctrl_Sub) {
            return m_pParamctrl_Sub->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            MY_ERR("m_pParamctrl_Sub is NULL");
            return MFALSE;
        }
    }
    else if (i4SensorDev & ESensorDev_MainSecond) {
        if (m_pParamctrl_Main2) {
            return m_pParamctrl_Main2->convertPtPairsToGMA(inPtPairs, u4NumOfPts, outGMA);
        }
        else {
            MY_ERR("m_pParamctrl_Main2 is NULL");
            return MFALSE;
        }
    }
    else {
        MY_ERR("Err IspTuningMgr::convertPtPairsToGMA()\n");
        return MFALSE;
    }
}
