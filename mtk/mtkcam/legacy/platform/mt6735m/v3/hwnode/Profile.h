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

#include <utils/Timers.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace v3 {


class DurationProfile {
public:
                        DurationProfile(char const*const szSubjectName)
                            : msSubjectName(szSubjectName)
                            , mi4Count(0)
                            , mnsStart(0)
                            , mnsEnd(0)
                            , mnsTotal(0)
                            , mnsWarning(1000000000LL)
                            {}

                        DurationProfile(char const*const szSubjectName, nsecs_t nsWarning)
                            : msSubjectName(szSubjectName)
                            , mi4Count(0)
                            , mnsStart(0)
                            , mnsEnd(0)
                            , mnsTotal(0)
                            , mnsWarning(nsWarning)
                            {}

    virtual            ~DurationProfile(){}

    virtual void        reset() { mi4Count = mnsStart = mnsEnd = mnsTotal = 0; }

    virtual void        pulse_up() { mnsStart = ::systemTime(); }
    virtual void        pulse_down()
                        {
                            mnsEnd = ::systemTime();
                            if (mnsStart != 0)
                            {
                                nsecs_t duration = mnsEnd - mnsStart;
                                mnsTotal += duration;
                                mi4Count++;
                                print_overtime(duration);
                                mnsStart = 0;
                            }
                        }
    virtual int64_t     getAvgDuration() const//in milli-seconds
                        {
                            return mi4Count == 0 ? 0 : ::ns2ms(mnsTotal)/mi4Count;
                        }
    virtual float       getFps() const
                        {
                            return mnsTotal == 0 ? 0 : ((float)mi4Count/mnsTotal)*1000000000LL;
                        }

protected:
    virtual void        print_overtime(nsecs_t duration) const
                        {
                             if (duration > mnsWarning) {
                                 CAM_LOGW("[%s] duration(%lld) > (%lld)", msSubjectName, duration, mnsWarning);
                             }
                        }


protected:
    char const*         msSubjectName;
    int32_t             mi4Count;
    nsecs_t             mnsStart;
    nsecs_t             mnsEnd;
    nsecs_t             mnsTotal;
    nsecs_t             mnsWarning;
};
};
};

