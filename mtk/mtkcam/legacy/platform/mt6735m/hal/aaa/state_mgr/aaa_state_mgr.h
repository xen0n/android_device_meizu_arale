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
* @file aaa_state_mgr.h
* @brief Declarations of 3A state machine
*/

#ifndef _AAA_STATE_MGR_H_
#define _AAA_STATE_MGR_H_

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <hal/aaa/aaa_hal_if.h>
#include <hal/aaa/aaa_hal_raw.h>
#include <hal/aaa/state_mgr/aaa_state.h>

using namespace android;

namespace NS3Av3
{

//class IState; //forward declaration

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  State Manager
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State Manager. It handles 3A intents to current state.
 */
class StateMgr
{
    friend class Hal3ARaw;
public:
#if 0
    static StateMgr& getInstance()
    {
        static StateMgr singleton;
        return singleton;
    }
#endif

    StateMgr();

    StateMgr(MINT32 sensorDevId);

    ~StateMgr();

    //================================================================
    //========================== 3A State Machine ==========================
    //================================================================
    inline MVOID updateFrameCount()
    {
        if (++m_i4FrameCount >= 0x7FFFFFFF) { m_i4FrameCount = 0; }
    }

    MVOID resetFrameCount();

    inline MINT32 getFrameCount()
    {
        return m_i4FrameCount;
    }

    struct StateStatus
    {
        EState_T eCurrState;
        EState_T eNextState;
        EState_T ePrevState;
        StateStatus()
            : eCurrState(eState_Uninit)
            , eNextState(eState_Invalid)
            , ePrevState(eState_Invalid)
        {}
    };
    inline StateStatus getStateStatus() { return mStateStatus; }
    inline MVOID setNextState(EState_T eNext) { mStateStatus.eNextState = eNext; }

    MRESULT transitState(EState_T const eCurrState, EState_T const eNewState);


    //================================================================
    //========================== AF Sub State =============================
    //================================================================
    struct AFStateCntSet //centralize all AF state flag and counter here
    {
        MINT32 PreAFFrmCnt;
        MINT32 AFFrmCnt;
        MINT32 PostAFFrmCnt;
        MINT32 PreAF_bNeedToTurnOnLamp;
        MINT32 AF_bNeedToTurnOffLamp;
        MINT32 bLampAlreadyOnBeforeSingleAF;
        MINT32 bNeedToDoPrecapAF;
        inline MVOID resetAll()
        {
            PreAFFrmCnt = AFFrmCnt = PostAFFrmCnt = 0;
            bNeedToDoPrecapAF = bLampAlreadyOnBeforeSingleAF = PreAF_bNeedToTurnOnLamp = AF_bNeedToTurnOffLamp = 0;
        }
        AFStateCntSet()
        {
            resetAll();
        }
    };

    /**
      * @brief proceed AF state by one stage
      */
    inline MVOID proceedAFState()
    {
        if (m_eAFState < IState::eAFState_Num)
        {
            m_eAFState = static_cast<IState::EAFState_T>(static_cast<int>(m_eAFState)+1);
        }
    }
    /**
      * @brief reset AF state to the beginning state
      */
    inline MVOID resetAFState()
    {
        m_eAFState = static_cast<IState::EAFState_T>(0);
    }
    /**
      * @brief get current AF state
      * @return current AF state; please refer to IState::EAFState_T
      */
    inline IState::EAFState_T getAFState()
    {
        return m_eAFState;
    }


    //================================================================
    //========================== Precapture Sub State ========================
    //================================================================
    /**
      * @brief proceed Precap state by one stage
      */
    inline MVOID proceedPrecapState()
    {
        if (m_ePrecapState < IState::ePrecap_Num)
        {
            m_ePrecapState = static_cast<IState::EPrecapState_T>(static_cast<int>(m_ePrecapState)+1);
        }
    }
    /**
      * @brief reset Precap state to the beginning state
      */
    inline MVOID resetPrecapState()
    {
        m_ePrecapState = static_cast<IState::EPrecapState_T>(0);
    }
    /**
      * @brief get current Precap state
      * @return current Precap state; please refer to IState::EPrecapState_T
      */
    inline IState::EPrecapState_T getPrecapState()
    {
        return m_ePrecapState;
    }


protected:
    //restrict the user set of the following variables of StateMgr
    //derived class is OK to use. EX: StateMgrN3D
    //friend class is OK to use EX: Hal3A (Note: Hal3ADev is not OK)

    MRESULT sendCmd(ECmd_T eCmd); // cannot be used by IState and its derived classes


    IState* mpIState[eState_Number];
    IState* mpCurrentState;
    mutable Mutex m_Lock;
    IState::EAFState_T       m_eAFState; // its usage/methods are constrained
    IState::EPrecapState_T   m_ePrecapState; // its usage/methods are constrained
    StateStatus              mStateStatus;
    MINT32                   m_i4FrameCount;
    MINT32                   mSensorDevId;

public:
    //add state flags here
    EState_T             mePrevStateForAe; /*NeedUpdate*/
    sem_t                mSemAF;
    AFStateCntSet            mAFStateCntSet;
    MVOID                *mpAAOBuf, *mpAFOBuf;
    MBOOL                m_bHasAEEverBeenStable;
    MINT32               m_bAFStartWait;
    MUINT8               m_uAEBlockAreaY[120*90];
    MBOOL                mbIsRecording;
};

}

#endif //_AAA_STATE_MGR_H_


