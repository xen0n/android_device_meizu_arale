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
#ifndef _AWB_STATE_H_
#define _AWB_STATE_H_

#include <utils/threads.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>

using namespace android;

namespace NS3Av3
{

typedef enum {
     eIntent_AWBModeNotAuto = 0,
     eIntent_AWBModeAuto,
     eIntent_AWBScan,
     eIntent_AWBScanFinished,
     eIntent_AWBLockOn,
     eIntent_AWBLockOff
} EAWBIntent_T;

typedef enum
{
    eState_Begin = 0,
    eState_InactiveNotAuto = eState_Begin,
    eState_InactiveAuto,
    eState_Searching,
    eState_Converged,
    eState_Locked,
    eState_Num
} EAWBState_T;


template<EAWBIntent_T eIntent> struct intent2type { enum {v=eIntent}; };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IAwbState
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class IAwbState
{
public:
    IAwbState(char const*const pcszName, EAWBState_T eAWBState);
    virtual ~IAwbState() {};


public:     //    Interfaces
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBModeNotAuto>) = 0;
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBModeAuto>) = 0;
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBScan>) = 0;
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBScanFinished>) = 0;
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBLockOn>) = 0;
    virtual IAwbState* sendIntent(intent2type<eIntent_AWBLockOff>) = 0;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    virtual char const* getName() const { return m_pcszName; }
    virtual EAWBState_T getState() const { return m_eAWBState; }

protected:  ////    Data Members.
    char const*const    m_pcszName;
    EAWBState_T         m_eAWBState;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  States
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    IAwbState* transitState(EAWBState_T const eNewState);

private:
    static IAwbState*  getStateInstance(EAWBState_T const eState);
    //static IAwbState*  sm_pCurrState;  //  Pointer to the current state.
    //static IAwbState*  getCurrStateInstance();
    friend class AwbStateMgr;
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  InactiveNotAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct InactiveNotAuto : public IAwbState
{
    InactiveNotAuto();
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeNotAuto>) {
        MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBModeNotAuto>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeAuto>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScan>) {
        MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBScan>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScanFinished>) {
        MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBScanFinished>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOn>) {
        MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBLockOn>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOff>) {
        MY_LOG("[InactiveNotAuto::sendIntent]<eIntent_AWBLockOff>");
        return this;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  InactiveAuto
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct InactiveAuto : public IAwbState
{
    InactiveAuto();
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeNotAuto>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeAuto>) {
        MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBModeAuto>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScan>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScanFinished>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOn>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOff>) {
        MY_LOG("[InactiveAuto::sendIntent]<eIntent_AWBLockOff>");
        return this;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Searching
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct Searching : public IAwbState
{
    Searching();
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeNotAuto>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeAuto>) {
        MY_LOG("[Searching::sendIntent]<eIntent_AWBModeAuto>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScan>) {
        MY_LOG("[Searching::sendIntent]<eIntent_AWBScan>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScanFinished>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOn>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOff>) {
        MY_LOG("[Searching::sendIntent]<eIntent_AWBLockOff>");
        return this;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Converged
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct Converged : public IAwbState
{
    Converged();
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeNotAuto>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeAuto>) {
        MY_LOG("[Converged::sendIntent]<eIntent_AWBModeAuto>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScan>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScanFinished>) {
        //MY_LOG("[Converged::sendIntent]<eIntent_AWBScanFinished>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOn>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOff>) {
        MY_LOG("[Converged::sendIntent]<eIntent_AWBLockOff>");
        return this;
    }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Locked
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
struct Locked : public IAwbState
{
    Locked();
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeNotAuto>);
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBModeAuto>) {
        MY_LOG("[Locked::sendIntent]<eIntent_AWBModeAuto>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScan>) {
        MY_LOG("[Locked::sendIntent]<eIntent_AWBScan>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBScanFinished>) {
        MY_LOG("[Locked::sendIntent]<eIntent_AWBScanFinished>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOn>) {
        MY_LOG("[Locked::sendIntent]<eIntent_AWBLockOn>");
        return this;
    }
    virtual IAwbState*  sendIntent(intent2type<eIntent_AWBLockOff>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AWB state manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AwbStateMgr
{
public:
    static AwbStateMgr* getInstance(MINT32 const i4SensorDev = 0);

    AwbStateMgr(MINT32 const i4SensorDev)
    : m_Lock(),
      m_i4SensorDev(i4SensorDev),
      m_pCurrentStat(IAwbState::getStateInstance(eState_InactiveNotAuto))
    {}

    MBOOL sendIntent(EAWBIntent_T eIntent)
    {
        Mutex::Autolock lock(m_Lock);

        EAWBIntent_T eNewIntent = eIntent;

        #define SEND_INTENT(_intent_)\
        case _intent_: m_pCurrentStat = m_pCurrentStat->sendIntent(intent2type<_intent_>());\
                       break;\

        switch (eNewIntent)
        {
        SEND_INTENT(eIntent_AWBModeNotAuto)
        SEND_INTENT(eIntent_AWBModeAuto)
        SEND_INTENT(eIntent_AWBScan)
        SEND_INTENT(eIntent_AWBScanFinished)
        SEND_INTENT(eIntent_AWBLockOn)
        SEND_INTENT(eIntent_AWBLockOff)
        }
        return MTRUE;
    }

    MBOOL reset(EAWBState_T eInitAWBState)
    {
        if ((eInitAWBState < eState_Begin) || (eInitAWBState > eState_Num)) {
            MY_ERR("Unsupport AWB state: %d", eInitAWBState);
            return MFALSE;
        }

        m_pCurrentStat = IAwbState::getStateInstance(eInitAWBState);
        return MTRUE;
    }

    mtk_camera_metadata_enum_android_control_awb_state_t getCurrentState()
    {
         switch (m_pCurrentStat->getState())
         {
         case eState_InactiveNotAuto:
         case eState_InactiveAuto:
         default:
            return MTK_CONTROL_AWB_STATE_INACTIVE;
            break;
         case eState_Searching:
            return MTK_CONTROL_AWB_STATE_SEARCHING;
            break;
         case eState_Converged:
            return MTK_CONTROL_AWB_STATE_CONVERGED;
            break;
         case eState_Locked:
            return MTK_CONTROL_AWB_STATE_LOCKED;
         };
    }

private:
    mutable Mutex m_Lock;
    MINT32 m_i4SensorDev;
    IAwbState* m_pCurrentStat;
};

};  //  namespace NS3Av3
#endif // _AWB_STATE_H_

