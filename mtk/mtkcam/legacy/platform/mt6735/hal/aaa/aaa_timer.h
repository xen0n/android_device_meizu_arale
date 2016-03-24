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
* @file aaa_timer.h
* @brief Declarations of 3A Timer
*/

#include <sys/time.h>
#include <cutils/log.h>

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

namespace NS3Av3 {
class AaaTimer {
public:

    inline MINT32 getUsTime() //it's not accurate to find the elapsed time of an algorithm/process
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    AaaTimer()
        : mInfo(MNULL)
        , mIdx(0)
        , mLogEn(MFALSE)
        , mTime_ms(0.0)
    {}

    AaaTimer(const char* info, MINT32 sensorDevId, MBOOL logEn)
        : mInfo(info), mIdx(sensorDevId), mLogEn(logEn), mTime_ms(0)
    {
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
    }

    MVOID start(const char* info, MINT32 sensorDevId, MBOOL logEn, MBOOL bClrTime = 1) //used by global/static variables
    {
        mInfo = info;
        mIdx = sensorDevId;
        mLogEn = logEn;
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] timer start. bClrTime=%d\n", __FUNCTION__, mInfo, mIdx, bClrTime);
        if (bClrTime) mTime_ms = 0.0;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
    }
    double End()
    {
        struct timespec ts_end;
        struct timespec ts_elapsed;
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        ts_elapsed.tv_sec = ts_end.tv_sec - ts_start.tv_sec;
        ts_elapsed.tv_nsec = ts_end.tv_nsec - ts_start.tv_nsec;

        if (ts_elapsed.tv_nsec < 0)
        {
            ts_elapsed.tv_nsec += 1000000000;
            ts_elapsed.tv_sec -= 1;
        }

        double ts_elapsed_tv_msec = ((double)ts_elapsed.tv_nsec)/1000000;
        ts_elapsed_tv_msec += ts_elapsed.tv_sec*1000;
        mTime_ms += ts_elapsed_tv_msec;
        /*
        MINT32 endTime = getUsTime();
        double t = ((double)(endTime - mStartTime)) / 1000;
        mTime_ms += t;
        */
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): current=%f,   sum=%f\n", __FUNCTION__, mInfo, mIdx, ts_elapsed_tv_msec, mTime_ms);
        return mTime_ms;
    }

    double printTime()
    {
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): sum=%f\n", __FUNCTION__, mInfo, mIdx, mTime_ms);
        return mTime_ms;
    }

    ~AaaTimer()
    {
    }

protected:
    const char* mInfo;
    struct timespec ts_start;
    MINT32 mIdx;
    MBOOL mLogEn;
    double mTime_ms;
};
};

