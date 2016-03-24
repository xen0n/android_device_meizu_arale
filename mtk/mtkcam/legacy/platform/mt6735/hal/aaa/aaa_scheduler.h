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

#ifndef _AAA_SCHEDULER_H_
#define _AAA_SCHEDULER_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

#include <sys/time.h>
#include <cutils/log.h>

#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <aaa_scheduling_custom.h>
#include "aaa_timer.h"

using namespace android;

namespace NS3Av3
{

struct WorkContainerPerFrame
{
    //   member variables
    WorkPerFrame mWorkPerFrame;


    //   member functions

    MVOID reset()
    {
        mWorkPerFrame.AAOJobs = mWorkPerFrame.AwbJobs = mWorkPerFrame.AfJobs = mWorkPerFrame.FlickerJobs = mWorkPerFrame.LscJobs = 0;
        mWorkPerFrame.AeFlareJobs = E_AE_IDLE;
    }
    MVOID importWork(WorkPerFrame& rWorkPerFrame)
    {
        mWorkPerFrame = rWorkPerFrame;
    }
    MVOID printWorkPerFrame() const
    {
        ALOGD("[WorkPerFrame: (AAO, Awb, Af, Flk, Lsc, AeFlare) = (%d,%d,%d,%d,%d,%d)]\n"
            , mWorkPerFrame.AAOJobs
            , mWorkPerFrame.AwbJobs
            , mWorkPerFrame.AfJobs
            , mWorkPerFrame.FlickerJobs
            , mWorkPerFrame.LscJobs
            , mWorkPerFrame.AeFlareJobs);
    }
    WorkContainerPerFrame(WorkPerFrame& rWorkPerFrame)
    {
        importWork(rWorkPerFrame);
        //printWorkPerFrame();
    }
    WorkContainerPerFrame()
    {
        reset();
    }
};

class WorkContainerPerCycle
{

public:
    //   member functions

    MVOID resetWork()
    {
        mValidFrameIdx = 0;
        for (int i=0; i<MAX_FRAME_PER_CYCLE; i++)
            mWorkContainerPerFrame[i].reset();
    }
    MVOID importWork(WorkPerCycle& rWorkPerCycle)
    {
        mValidFrameIdx = rWorkPerCycle.mValidFrameIdx;
        for (int i=0; i<MAX_FRAME_PER_CYCLE; i++)
            mWorkContainerPerFrame[i].importWork(rWorkPerCycle.mWorkPerFrame[i]);
    }
    MVOID printWorkPerCycle() const
    {
        ALOGD("mValidFrameIdx = %d\n", mValidFrameIdx);
        for (int i=0; i<MAX_FRAME_PER_CYCLE; i++)
        {
            ALOGD("FrameIdx (%d) in Cycle: ", i+1); //frameIdx starts from 1
            mWorkContainerPerFrame[i].printWorkPerFrame();
        }
    }
    WorkContainerPerCycle(WorkPerCycle& rWorkPerCycle)
    {
        importWork(rWorkPerCycle);
        //printWorkPerCycle();
    }
    WorkContainerPerCycle()
    {
        resetWork();
    }
    inline WorkPerFrame getWorkPerFrame(MUINT32 frameIdx) const //frameIdx starts from 1
    {
        return mWorkContainerPerFrame[frameIdx-1].mWorkPerFrame;
    }

protected:
    //   member variables

    WorkContainerPerFrame mWorkContainerPerFrame[MAX_FRAME_PER_CYCLE];
    MUINT32 mValidFrameIdx;

};

class AAA_Scheduler : public WorkContainerPerCycle
{
public:
    MVOID importWork();
    MUINT32 queryFramesPerCycle(MUINT32 fps);
    long long int getVsTimeOutLimit_ns();
    MVOID  setSchdParams(MUINT32 sensorDevId, MUINT32 fps, MBOOL logEn, MUINT32 time_mrg_ms = 3);
    MUINT32 jobAssignAndTimerStart(E_Job_3A eJob, MBOOL bClrTime = 1);
    MVOID jobTimerEnd(E_Job_3A eJob);
    MBOOL  makeJobTimeSummary(); //get and print (if exceed threshold)
    inline MUINT32 getValidFrameIdx() const { return mValidFrameIdx; } //frameIdx starts from 1
    inline MVOID setFrameIdx(MUINT32 frameIdx) { mFrameIdx = frameIdx; } //frameIdx starts from 1
    inline MUINT32 getFrameIdx() const { return mFrameIdx; } //frameIdx starts from 1

    AAA_Scheduler()
        : mfps(30)
        , mNormalizeM(1)
        , m_time_mrg_ms(3)
        , mSensorDevId(0)
        , mAllLogEn(MFALSE)
        , mFrameIdx(1)
    {}

private:
    NS3Av3::AaaTimer jobTimer[E_Job_NUM];
    MUINT32 timerEn[E_Job_NUM];
    double jobTimeResult[E_Job_NUM];
    MUINT32 mfps;
    MUINT32 mNormalizeM;
    MUINT32 m_time_mrg_ms;
    MUINT32 mSensorDevId;
    MBOOL mAllLogEn;
    MUINT32 mFrameIdx; //frameIdx starts from 1

};

}

#endif /* _AAA_SCHEDULER_H_ */