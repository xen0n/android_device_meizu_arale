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
* @file aaa_state.h
* @brief Declarations of 3A state machine
*/

#ifndef _AAA_STATE_H_
#define _AAA_STATE_H_

#include <utils/threads.h>

using namespace android;

namespace NS3Av3
{

typedef enum {
     eIntent_CameraPreviewStart = ECmd_CameraPreviewStart,
     eIntent_CameraPreviewEnd = ECmd_CameraPreviewEnd,
     eIntent_PrecaptureStart = ECmd_PrecaptureStart,
     eIntent_PrecaptureEnd = ECmd_PrecaptureEnd,
     eIntent_CaptureStart = ECmd_CaptureStart,
     eIntent_CaptureEnd = ECmd_CaptureEnd,
     eIntent_RecordingStart = ECmd_RecordingStart,
     eIntent_RecordingEnd = ECmd_RecordingEnd,
     eIntent_VsyncUpdate = ECmd_Update,
     eIntent_AFUpdate = ECmd_AFUpdate,
     eIntent_AFStart = ECmd_AFStart,
     eIntent_AFEnd = ECmd_AFEnd,
     eIntent_Init = ECmd_Init,
     eIntent_Uninit = ECmd_Uninit
} EIntent_T;

typedef enum
{
    eState_Invalid=-1,
    eState_Uninit=0,
    eState_Init,
    eState_CameraPreview,
    eState_Precapture,
    eState_Capture,
    eState_AF,
    eState_Number
} EState_T;


#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_STATE(String) \
       do { \
           aee_system_exception( \
               "3A State", \
               NULL, \
               DB_OPT_DEFAULT, \
               String); \
       } while(0)
#else
#define AEE_ASSERT_3A_STATE(String)
#endif

template<EIntent_T eIntent> struct intent2type { enum {v=eIntent}; };

class StateMgr;
//class AAA_Scheduler;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IState
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State Base Class
 */

class IState
{
public:
    IState(char const*const pcszName, MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual ~IState() {};


public:     //    Interfaces
    /**
      * @brief send intent eIntent_CameraPreviewStart to 3A state machine, prepare 3A for preview start
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CameraPreviewStart", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CameraPreviewStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_CameraPreviewEnd to 3A state machine, stop 3A preview process
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CameraPreviewEnd", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CameraPreviewEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_PrecaptureStart to 3A state machine, preprocess of capture 3A, enter precapture state
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_PrecaptureStart", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: PrecaptureStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_PrecaptureEnd to 3A state machine, leave from precapture state
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_PrecaptureEnd", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: PrecaptureEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_CaptureStart to 3A state machine, do 3A process before capture (EX: set AE sensor, shutter)
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CaptureStart", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CaptureStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_CaptureEnd to 3A state machine, do 3A process after capture (EX: set AWB, flare ISP HW)
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_CaptureEnd", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: CaptureEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_RecordingStart to 3A state machine, prepare 3A for recording process
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_RecordingStart", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: RecordingStart");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_RecordingEnd to 3A state machine, stop recording 3A process
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_RecordingEnd", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: RecordingEnd");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_VsyncUpdate to 3A state machine. This intent is executed RIGHT AFTER wait VS irq. It deals with 3A process each frame, based on current 3A state
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_VsyncUpdate", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_AFUpdate to 3A state machine. This intent is executed by AF thread, RIGHT AFTER wait AF irq. It deals with AF process each frame, base on current 3A state
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFUpdate", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_AFStart to 3A state machine. Enter AF state for continuous AF process
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFStart", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_AFEnd to 3A state machine. leave from AF state then go to previous/next state (depend on the situation)
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_AFEnd", m_pcszName);
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_Init to 3A state machine. Init 3A hal
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_Init>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_Init", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: Init");
            return  E_3A_UNSUPPORT_COMMAND;
        }
    /**
      * @brief send intent eIntent_Uninit to 3A state machine. Uninit 3A hal
      */
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>) {
            MY_ERR("[%s]E_3A_UNSUPPORT_COMMAND: eIntent_Uninit", m_pcszName);
            AEE_ASSERT_3A_STATE("3A_UNSUPPORT_COMMAND: Uninit");
            return  E_3A_UNSUPPORT_COMMAND;
        }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////    Operations.
    virtual char const* getName() const { return m_pcszName; }
    //static void setSensorDev(MINT32 sensorDev) {sm_SensorDev = sensorDev;}

protected:  ////    Data Members.
    Hal3ARaw*           m_pHal3A;
    char const*const    m_pcszName;
    MINT32              m_SensorDevId;
    StateMgr*           m_pStateMgr;
    //AAA_Scheduler*      m_pScheduler;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  States
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++



//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AF States
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:

    typedef enum
    {
        eAFState_PreAF = 0,
        eAFState_AF,
        eAFState_PostAF,
        eAFState_Num
    } EAFState_T;
    // here we define the first AFState to be 0,
    // and the state listed here should be ordered

    typedef enum
    {
        ePrecap_AF_BeforeFlash = 0,
        ePrecap_AE,
        ePrecap_Flash,
        ePrecap_AF_AfterFlash,
        ePrecap_Num
    } EPrecapState_T;
    // here we define the first PrecapState to be 0,
    // and the state listed here should be ordered




private:

    friend class StateMgr;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Frame count
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

public:


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//    3A log control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    MINT32  m_3ALogEnable;
};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateUninit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Uninit. wait for init by camera process
 */
struct StateUninit : public IState
{
    StateUninit(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_Init>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateInit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Init. wait for 3A preview start
 */
struct StateInit : public IState
{
    StateInit(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: camera preview. it can update preview 3A per frame, wait for capture request, and do touch/continuous AF
 */
struct StateCameraPreview : public IState
{
    StateCameraPreview(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);

    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);

    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);

    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
/*
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);
*/
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
};
#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCamcorderPreview
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Camcorder preview. it can update preview 3A per frame, wait for recording request, and do touch/continuous AF
 */
struct StateCamcorderPreview : public IState
{
    StateCamcorderPreview(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>); // for CTS only
};
#endif
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePrecapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Precapture. preparation stage before 3A ready-to-capture
 */
struct StatePrecapture : public IState
{
    StatePrecapture(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);

    template<EPrecapState_T ePrecapState> struct state2type { enum {v=ePrecapState}; };
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_BeforeFlash>);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AE>, MVOID* pBufInfo);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_Flash>, MVOID* pBufInfo);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_AfterFlash>);

};

#if 0
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCapture
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Capture. do capture 3A process for single/multi shot
 */
struct StateCapture : public IState
{
    StateCapture(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>); // for ZSD capture
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>); // for CTS only
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateRecording
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Recording. do recording 3A process per frame. support touch AF but not continuous AF
 */
struct StateRecording : public IState
{
    StateRecording(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);

    MRESULT exitPreview();
};
#endif


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: touch AF. Touch AF is executed in AF state, then go to previous/next state after AF done
 */
struct StateAF : public IState
{
    StateAF(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_RecordingEnd>);




    template<EAFState_T eAFState> struct state2type { enum {v=eAFState}; };

    //MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_None>);
    //MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_None>);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PreAF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PreAF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo);
    //MRESULT exitPreview();

};






};  //  namespace NS3A
#endif // _AAA_STATE_H_

