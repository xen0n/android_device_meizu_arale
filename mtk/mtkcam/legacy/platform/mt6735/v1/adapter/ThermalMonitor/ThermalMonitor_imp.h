/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
#ifndef THERMAL_MONITOR_IMP_H
#define THERMAL_MONITOR_IMP_H
//-----------------------------------------------------------------------------
#include <utils/Thread.h>
//
#include <ThermalMonitor/ThermalMonitor.h>
//-----------------------------------------------------------------------------
class ThermalMonitorImp : public ThermalMonitor
                               , public Thread
{
    public:
        ThermalMonitorImp(
            IHal3A::E_VER   aaaVersion,
            MINT32          sensorIdx);
        virtual ~ThermalMonitorImp();
    //
    public:
        virtual MBOOL   setFrameRate(MUINT32 fps);
        virtual MBOOL   start();
        virtual MBOOL   stop();
        virtual MBOOL   exit();
        virtual MBOOL   isRunning();
    //
    private:
        virtual bool    threadLoop();
    //
    private:
        #define FILENAME_THERMAL    "/proc/driver/cl_vrt"
        #define SLEEP_TIME_US       (1*1000*1000)
        #define ADJUST_RATIO        (0.8)
        //
        typedef enum
        {
            STATE_EXIT,
            STATE_START,
            STATE_STOP,
            STATE_WAIT
        }STATE_E;
        //
        mutable Mutex   mLock;
        Condition       mCond;
        STATE_E         mState;
        FILE*           mFpThermal;
        MUINT32         mOrigFps;
        MUINT32         mCurFps;
        IHal3A*         mpHal3a;
};
//-----------------------------------------------------------------------------
#endif

