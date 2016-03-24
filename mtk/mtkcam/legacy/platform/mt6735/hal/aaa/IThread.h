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

/**
* @file IThread.h
* @brief Declarations of Abstraction of thread of 3A Hal
*/
#ifndef __ITHREAD_H__
#define __ITHREAD_H__

#include <aaa_types.h>

namespace NS3Av3
{
class IThread
{
public:
    virtual MBOOL               destroyInstance() = 0;
    virtual MBOOL               postCmd(void* pArg) = 0;
    virtual MBOOL               waitFinished() = 0;
    virtual                     ~IThread(){}
};


class ThreadSensorI2C : public IThread
{
public:
    struct Cmd_T
    {
        MUINT32                 u4MagicNum;
        MUINT32                 u4ExposureTime;
        MUINT32                 u4SensorGain;
        MBOOL                   fgTerminate;
        MINT32                  i4TgInfo;

        Cmd_T()
            : u4MagicNum(0)
            , u4ExposureTime(0)
            , u4SensorGain(0)
            , fgTerminate(1)
            , i4TgInfo(0)
            {}
        Cmd_T(MUINT32 _u4Magic, MUINT32 _u4ExpTime, MUINT32 _u4SensorGain, MINT32 _i4TgInfo)
            : u4MagicNum(_u4Magic)
            , u4ExposureTime(_u4ExpTime)
            , u4SensorGain(_u4SensorGain)
            , fgTerminate(0)
            , i4TgInfo(_i4TgInfo)
            {}
    };
    static ThreadSensorI2C*     createInstance(MUINT32 u4SensorDev);
    virtual MBOOL               destroyInstance() = 0;
    virtual MBOOL               postCmd(void* pArg) = 0;
    virtual MBOOL               waitFinished() = 0;
    virtual                     ~ThreadSensorI2C(){}
};

class ThreadSensorGain : public IThread
{
public:
    struct Cmd_T
    {
        MUINT32                 u4MagicNum;
        MUINT32                 u4ExposureTime;
        MUINT32                 u4SensorGain;
        MBOOL                   fgTerminate;
        MINT32                  i4TgInfo;

        Cmd_T()
            : u4MagicNum(0)
            , u4ExposureTime(0)
            , u4SensorGain(0)
            , fgTerminate(1)
            , i4TgInfo(0)
            {}
        Cmd_T(MUINT32 _u4Magic, MUINT32 _u4ExpTime, MUINT32 _u4SensorGain, MINT32 _i4TgInfo)
            : u4MagicNum(_u4Magic)
            , u4ExposureTime(_u4ExpTime)
            , u4SensorGain(_u4SensorGain)
            , fgTerminate(0)
            , i4TgInfo(_i4TgInfo)
            {}
    };
    static ThreadSensorGain*    createInstance(MUINT32 u4SensorDev);
    virtual MBOOL               destroyInstance() = 0;
    virtual MBOOL               postCmd(void* pArg) = 0;
    virtual MBOOL               waitFinished() = 0;
    virtual                     ~ThreadSensorGain(){}
};

};

#endif //__ITHREAD_H__
