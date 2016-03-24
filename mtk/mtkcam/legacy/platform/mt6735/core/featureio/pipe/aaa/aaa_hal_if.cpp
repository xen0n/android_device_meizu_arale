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

#include <mtkcam/featureio/aaa_hal_if.h>

//#include <mtkcam/core/featureio/pipe/aaa/ResultBufMgr/ResultBufMgr.h>
//#include "aaa_hal_template.h"

using namespace NS3A;

#define EN_3A_CB_LOG (0)

#if 0
/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const u4SensorOpenIndex)
{
    //static Hal3AIf singleton;
    //return &singleton;
    return Hal3ATemplate::createInstance(i4SensorDevId);
}

#else

#include <stdlib.h>
#include <stdio.h>
#include <cutils/log.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <state_mgr/aaa_state.h>
#include <state_mgr/aaa_state_mgr.h>
#include "aaa_hal_yuv.h"
//
/*NeedUpdate*///#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/hal/IHalSensor.h>

using namespace NS3A;
using namespace android;

/*******************************************************************************
*
********************************************************************************/
Hal3AIf*
Hal3AIf::createInstance(MINT32 const i4SensorOpenIndex)
{
#ifdef USING_MTK_LDVT
    static Hal3AIf singleton;
    return &singleton;
#endif


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

#if 0
    SensorHal* pSensorHal = SensorHal::createInstance();
    if ( !pSensorHal )
    {
        return NULL;
    }
    //
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(i4SensorDevId),
                            SENSOR_CMD_SET_SENSOR_DEV,
                            0,
                            0,
                            0
                           );
    //
    pSensorHal->init();
    //
    pSensorHal->sendCommand(static_cast<halSensorDev_e>(i4SensorDevId),
                             SENSOR_CMD_GET_SENSOR_TYPE,
                             reinterpret_cast<int>(&eSensorType),
                             0,
                             0
                            );
    //
    if  ( pSensorHal )
    {
        pSensorHal->uninit();
        pSensorHal->destroyInstance();
        pSensorHal = NULL;
    }

    if (eSensorType == SENSOR_TYPE_RAW) {
#ifdef USING_MTK_LDVT
    static Hal3AIf singleton;
    return &singleton;
#else
        return Hal3A::createInstance(i4SensorDevId);
#endif
    }
    else if (eSensorType == SENSOR_TYPE_YUV)
    {
#ifdef USING_MTK_LDVT
         static Hal3AIf singleton;
         return &singleton;
#else
        return Hal3AYuv::createInstance(i4SensorDevId);
#endif
    }
    else {
#if 1
    static Hal3AIf singleton;
    return &singleton;
#else
    return NULL;//Hal3AYuv::getInstance();
#endif
    }
#endif
}

/*******************************************************************************
*
********************************************************************************/

CallBackSet::
CallBackSet()
    : m_Mutex()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

CallBackSet::
~CallBackSet()
{
    Mutex::Autolock autoLock(m_Mutex);

    m_CallBacks.clear();
}

void
CallBackSet::
doNotifyCb (
   int32_t _msgType,
   int32_t _ext1,
   int32_t _ext2,
   int32_t _ext3,
   MINTPTR _ext4
)
{
    Mutex::Autolock autoLock(m_Mutex);

    if (EN_3A_CB_LOG) MY_LOG("[%s] _msgType(%d) _ext1(0x%08x) _ext2(0x%08x) _ext3(0x%08x) _ext4(0x%08x)", __FUNCTION__, _msgType, _ext1, _ext2, _ext3, _ext3);
    List<I3ACallBack*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        I3ACallBack* pCb = *it;
        if (pCb)
        {
            if (EN_3A_CB_LOG) MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, pCb);
            pCb->doNotifyCb(_msgType, _ext1, _ext2, _ext3, _ext4);
        }
    }
}

void
CallBackSet::
doDataCb (
   int32_t  _msgType,
   void*    _data,
   uint32_t _size
)
{
    Mutex::Autolock autoLock(m_Mutex);

    if (EN_3A_CB_LOG) MY_LOG("[%s] _msgType(%d) _data(0x%08x) _size(0x%08x)", __FUNCTION__, _msgType, _data, _size);
    List<I3ACallBack*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++)
    {
        I3ACallBack* pCb = *it;
        if (pCb)
        {
            if (EN_3A_CB_LOG) MY_LOG("[%s] cb(0x%08x)", __FUNCTION__, pCb);
            pCb->doDataCb(_msgType, _data, _size);
        }
    }
}

MINT32
CallBackSet::
addCallback(I3ACallBack* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;

    if (cb == NULL)
    {
        MY_ERR("[%s] NULL callback!", __FUNCTION__);
        return -m_CallBacks.size();
    }

    List<I3ACallBack*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
            break;
    }

    if (i4Cnt == (MINT32) m_CallBacks.size())
    {
        // not exist, add
        m_CallBacks.push_back(cb);
        return m_CallBacks.size();
    }
    else
    {
        // already exists
        return -m_CallBacks.size();
    }
}

MINT32
CallBackSet::
removeCallback(I3ACallBack* cb)
{
    Mutex::Autolock autoLock(m_Mutex);

    MINT32 i4Cnt = 0;
    MINT32 i4Size = m_CallBacks.size();

    List<I3ACallBack*>::iterator it;
    for (it = m_CallBacks.begin(); it != m_CallBacks.end(); it++, i4Cnt++)
    {
        if (cb == *it)
        {
            m_CallBacks.erase(it);
            // successfully removed
            return m_CallBacks.size();
        }
    }

    // cannot be found
    return -i4Size;
}


#endif
