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

#ifndef _AAA_STATE_N3D_H_
#define _AAA_STATE_N3D_H_

#include <utils/threads.h>
#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state.h>

using namespace android;

namespace NS3Av3
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateInit
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Init. wait for 3A preview start
 */
struct StateInitN3d : public StateInit
{
    StateInitN3d(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCameraPreviewN3d
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: camera preview. it can update preview 3A per frame, wait for capture request, and do touch/continuous AF
 */
struct StateCameraPreviewN3d : public StateCameraPreview
{
    StateCameraPreviewN3d(MINT32 sensorDevId, StateMgr* pStateMgr);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StatePrecaptureN3d
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Precapture. preparation stage before 3A ready-to-capture
 */
struct StatePrecaptureN3d : public StatePrecapture
{
    StatePrecaptureN3d(MINT32 sensorDevId, StateMgr* pStateMgr);
    //virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_PrecaptureEnd>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);

    template<EPrecapState_T ePrecapState> struct state2type { enum {v=ePrecapState}; };
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_BeforeFlash>);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AE>, MVOID* pBufInfo);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_Flash>, MVOID* pBufInfo);
    MRESULT  sendPrecapIntent(intent2type<eIntent_VsyncUpdate>, state2type<ePrecap_AF_AfterFlash>);

};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateCaptureN3d
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: Capture. do capture 3A process for single/multi shot
 */
struct StateCaptureN3d : public StateCapture
{
    StateCaptureN3d(MINT32 sensorDevId, StateMgr* pStateMgr);
    //virtual MRESULT  sendIntent(intent2type<eIntent_CaptureStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CaptureEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CameraPreviewStart>);
    virtual MRESULT  sendIntent(intent2type<eIntent_CamcorderPreviewStart>); // for CTS only
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  StateAF
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
/**
 * @brief 3A State: touch AF. Touch AF is executed in AF state, then go to previous/next state after AF done
 */
struct StateAFN3d : public StateAF
{
    StateAFN3d(MINT32 sensorDevId, StateMgr* pStateMgr);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFStart>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFEnd>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_Uninit>);
    virtual MRESULT  sendIntent(intent2type<eIntent_VsyncUpdate>);
    //virtual MRESULT  sendIntent(intent2type<eIntent_AFUpdate>);
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
    //MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PreAF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo);
    //MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_AF>, MVOID* pBufInfo);
    MRESULT  sendAFIntent(intent2type<eIntent_VsyncUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo);
    //MRESULT  sendAFIntent(intent2type<eIntent_AFUpdate>, state2type<eAFState_PostAF>, MVOID* pBufInfo);
    //MRESULT exitPreview();

};


};  //  namespace NS3Av3
#endif // _AAA_STATE_N3D_H_

