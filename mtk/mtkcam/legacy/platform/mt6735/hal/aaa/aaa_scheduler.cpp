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


#include "aaa_scheduler.h"
#define MTK_LOG_ENABLE 1
#ifndef LOG_TAG
#define LOG_TAG "aaa_scheduler"
#endif

#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG       (1)
#endif

using namespace NS3Av3;

MVOID AAA_Scheduler::importWork()
{
    ALOGD("[AAA_Scheduler::importWork]+, mNormalizeM=%d, mSensorDevId=%d\n", mNormalizeM, mSensorDevId);
    WorkPerCycle work = getWorkPerCycle(mNormalizeM, mSensorDevId); //WARMING: cycle index will increment for each call
    WorkContainerPerCycle::importWork(work);
    if (mAllLogEn) WorkContainerPerCycle::printWorkPerCycle();
    //reset jobTimeResult
    for (int i=0; i<E_Job_NUM; i++)
    {
        jobTimeResult[i] = 0.0;
        timerEn[i] = 0;
    }
    //ALOGD("[AAA_Scheduler::importWork]-\n");
}

MUINT32 AAA_Scheduler::queryFramesPerCycle(MUINT32 fps)
{
     //might be customized
    return queryFramesPerCycle_custom(fps);
}

long long int AAA_Scheduler::getVsTimeOutLimit_ns()
{
    long long int ret = ::getVsTimeOutLimit_ns(mNormalizeM, mfps);
    if (mAllLogEn)
        ALOGD("[%s] (mfps, mNormalizeM, TimeOutLimit) = (%d, %d, %d)\n"
            , __FUNCTION__
            , mfps
            , mNormalizeM
            , ret);
    return ret;
}


MVOID AAA_Scheduler::setSchdParams(MUINT32 sensorDevId, MUINT32 fps, MBOOL logEn, MUINT32 time_mrg_ms)
{
    mSensorDevId = sensorDevId;
    mfps = fps;
    mNormalizeM = queryFramesPerCycle(fps);
    mAllLogEn = logEn;
    m_time_mrg_ms = time_mrg_ms;
    resetCycleCtr(sensorDevId);
    if (logEn)
        ALOGD("[%s] (sensorDevId, fps, mNormalizeM, logEn, time_mrg_ms) = (%d, %d, %d, %d, %d)\n", __FUNCTION__
            , sensorDevId
            , fps
            , mNormalizeM
            , logEn
            , time_mrg_ms);
}

MUINT32 AAA_Scheduler::jobAssignAndTimerStart(E_Job_3A eJob, MBOOL bClrTime)
{
    if (mAllLogEn)
        ALOGD("[%s]+, (mNormalizeM, mFrameIdx, eJob, bClrTime, mSensorDevId) = (%d, %d, %d, %d, %d)\n", __FUNCTION__
            , mNormalizeM
            , mFrameIdx
            , eJob
            , bClrTime
            , mSensorDevId);

    WorkPerFrame aWork = getWorkPerFrame(mFrameIdx);

    MUINT32 ret;
    switch(eJob)
    {
    case E_Job_AAO:
        if ((timerEn[eJob] = ret = aWork.AAOJobs))                 jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_Awb:
        if ((timerEn[eJob] = ret = aWork.AwbJobs))                 jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_Af:
        if ((timerEn[eJob] = ret = aWork.AfJobs))                  jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_Flicker:
        if ((timerEn[eJob] = ret = aWork.FlickerJobs))             jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_Lsc:
        if ((timerEn[eJob] = ret = aWork.LscJobs))                 jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_AeFlare:
        if ((timerEn[eJob] = ret = aWork.AeFlareJobs))             jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    case E_Job_IspValidate:
        //during initial drop frames, mValidFrameIdx = M, and mFrameIdx = 1, so IspValidate need to be disabled by getFrameCount < 0
        //the first validate occurs at about 2+(ValidateFrameIdx at first cycle)th Vsync
        if ((timerEn[eJob] = ret = (mFrameIdx == mValidFrameIdx))) jobTimer[eJob].start(Job3AName(eJob), mSensorDevId, mAllLogEn, bClrTime);
        if (mAllLogEn)                                           ALOGD("[%s] ret = %d\n", __FUNCTION__, ret);
        return ret;
    default:
        ALOGD("[%s] wrong eJob = %d\n", __FUNCTION__, eJob);
        return 0;
    }
}

MVOID AAA_Scheduler::jobTimerEnd(E_Job_3A eJob)
{
    if (timerEn[eJob]) jobTimeResult[eJob] = jobTimer[eJob].End();
}

MBOOL AAA_Scheduler::makeJobTimeSummary() //get and print (if exceed threshold), frameIdx starts from 1
{
    //use timerEn[eJob], mAllLogEn, jobTimeResult[eJob] to make Job time summary for this frame or cycle
    if (!mAllLogEn) return MTRUE;

    ALOGD("[%s] mFrameIdx = %d, mValidFrameIdx = %d\n", __FUNCTION__, mFrameIdx, mValidFrameIdx);
    for (int i=0; i<E_Job_NUM; i++)
        if (timerEn[i]) jobTimer[i].printTime();

    return MTRUE;
}




