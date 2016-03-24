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
#define LOG_TAG "awb_state"

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

#include <isp_tuning.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <awb_state.h>

using namespace NS3A;
using namespace NSIspTuning;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
template <ESensorDev_T const eSensorDev>
class AwbStateMgrDev : public AwbStateMgr
{
public:
    static
    AwbStateMgr*
    getInstance()
    {
        static AwbStateMgrDev<eSensorDev> singleton;
        return &singleton;
    }

    AwbStateMgrDev()
        : AwbStateMgr(eSensorDev)
    {}

    virtual ~AwbStateMgrDev() {}
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
AwbStateMgr*
AwbStateMgr::getInstance(MINT32 const i4SensorDev)
{
    switch (i4SensorDev)
    {
    case ESensorDev_Main: //  Main Sensor
        return  AwbStateMgrDev<ESensorDev_Main>::getInstance();
    case ESensorDev_MainSecond: //  Main Second Sensor
        return  AwbStateMgrDev<ESensorDev_MainSecond>::getInstance();
    case ESensorDev_Sub: //  Sub Sensor
        return  AwbStateMgrDev<ESensorDev_Sub>::getInstance();
    default:
        MY_LOG("i4SensorDev = %d", i4SensorDev);
        return  AwbStateMgrDev<ESensorDev_Main>::getInstance();
    }
}



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAwbState
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//IAwbState*
//IAwbState::sm_pCurrState =
//IAwbState::getStateInstance(eState_InactiveNotAuto);

//IAwbState*
//IAwbState::
//getCurrStateInstance()
//{
//    return  IAwbState::sm_pCurrState;
//}

IAwbState*
IAwbState::
getStateInstance(EAWBState_T const eState)
{
    switch  (eState)
    {
#define INSTANTIATE_STATE(_name_)\
    case eState_##_name_:\
        {\
            static  _name_ singleton;\
            return  &singleton;\
        }

    INSTANTIATE_STATE(InactiveNotAuto);
    INSTANTIATE_STATE(InactiveAuto);
    INSTANTIATE_STATE(Searching);
    INSTANTIATE_STATE(Converged);
    INSTANTIATE_STATE(Locked);
    default:
        break;
    };

    return  MNULL;
}


IAwbState*
IAwbState::
transitState(EAWBState_T const eNewState)
{
    IAwbState*const pNewState = getStateInstance(eNewState);
    if  ( ! pNewState )
    {
        MY_ERR("[IAwbState::transitState()] pNewState==NULL");
        return  this;
    }

    MY_LOG("[IAwbState::transitState] %s --> %s", this->getName(), pNewState->getName());

    return  pNewState;
}


IAwbState::
IAwbState(char const*const pcszName, EAWBState_T eAWBState)
    : m_pcszName(pcszName)
    , m_eAWBState(eAWBState)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  InactiveNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
InactiveNotAuto::
InactiveNotAuto()
    : IAwbState("InactiveNotAuto", eState_InactiveNotAuto)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBModeAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
InactiveNotAuto::
sendIntent(intent2type<eIntent_AWBModeAuto>)
{
    MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBModeAuto>");
    // State transition: eState_InactiveNotAuto --> eState_InactiveAuto
    return transitState(eState_InactiveAuto);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  InactiveAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
InactiveAuto::
InactiveAuto()
    : IAwbState("InactiveAuto", eState_InactiveAuto)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBModeNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
InactiveAuto::
sendIntent(intent2type<eIntent_AWBModeNotAuto>)
{
    MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBModeNotAuto>");
    // State transition: eState_InactiveAuto --> eState_InactiveNotAuto
    return transitState(eState_InactiveNotAuto);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBScan
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
InactiveAuto::
sendIntent(intent2type<eIntent_AWBScan>)
{
    MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBScan>");
    // State transition: eState_InactiveAuto --> eState_Searching
    return transitState(eState_Searching);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBScanFinished
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
InactiveAuto::
sendIntent(intent2type<eIntent_AWBScanFinished>)
{
    MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBScanFinished>");
    // State transition: eState_InactiveAuto --> eState_Converged
    return transitState(eState_Converged);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBLockOn
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
InactiveAuto::
sendIntent(intent2type<eIntent_AWBLockOn>)
{
    MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBLockOn>");
    // State transition: eState_InactiveAuto --> eState_Locked
    return transitState(eState_Locked);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Searching
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Searching::
Searching()
    : IAwbState("Searching", eState_Searching)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBModeNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Searching::
sendIntent(intent2type<eIntent_AWBModeNotAuto>)
{
    MY_LOG("[Searching::sendIntent]<eIntent_AWBModeNotAuto>");
    // State transition: eState_Searching --> eState_InactiveNotAuto
    return transitState(eState_InactiveNotAuto);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBScanFinished
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Searching::
sendIntent(intent2type<eIntent_AWBScanFinished>)
{
    MY_LOG("[Searching::sendIntent]<eIntent_AWBScanFinished>");
    // State transition: eState_Searching --> eState_Converged
    return transitState(eState_Converged);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBLockOn
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Searching::
sendIntent(intent2type<eIntent_AWBLockOn>)
{
    MY_LOG("[Searching::sendIntent]<eIntent_AWBLockOn>");
    // State transition: eState_Searching --> eState_Locked
    return transitState(eState_Locked);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Converged
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Converged::
Converged()
    : IAwbState("Converged", eState_Converged)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBModeNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Converged::
sendIntent(intent2type<eIntent_AWBModeNotAuto>)
{
    MY_LOG("[Converged::sendIntent]<eIntent_AWBModeNotAuto>");
    // State transition: eState_Searching --> eState_InactiveNotAuto
    return transitState(eState_InactiveNotAuto);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBScan
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Converged::
sendIntent(intent2type<eIntent_AWBScan>)
{
    MY_LOG("[Converged::sendIntent]<eIntent_AWBScan>");
    // State transition: eState_Converged --> eState_Searching
    return transitState(eState_Searching);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBLockOn
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Converged::
sendIntent(intent2type<eIntent_AWBLockOn>)
{
    MY_LOG("[Converged::sendIntent]<eIntent_AWBLockOn>");
    // State transition: eState_Converged --> eState_Locked
    return transitState(eState_Locked);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Locked
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Locked::
Locked()
    : IAwbState("Locked", eState_Locked)
{
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBModeNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Locked::
sendIntent(intent2type<eIntent_AWBModeNotAuto>)
{
    MY_LOG("[Locked::sendIntent]<eIntent_AWBModeNotAuto>");
    // State transition: eState_Locked --> eState_InactiveNotAuto
    return transitState(eState_InactiveNotAuto);
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  eIntent_AWBLockOff
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
IAwbState*
Locked::
sendIntent(intent2type<eIntent_AWBLockOff>)
{
    MY_LOG("[Locked::sendIntent]<eIntent_AWBLockOff>");
    // State transition: eState_Locked --> eState_Searching
    return transitState(eState_Searching);
}

