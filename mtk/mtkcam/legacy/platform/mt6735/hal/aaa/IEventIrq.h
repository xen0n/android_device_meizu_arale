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
* @file IEventIrq.h
* @brief Declarations of IRQ Event waiting interface
*/

#ifndef __IEVENT_IRQ_H__
#define __IEVENT_IRQ_H__

#include <mtkcam/common.h>

namespace NS3Av3
{
/******************************************************************************
 *  IEventIrq Interface.
 ******************************************************************************/
class IEventIrq
{
public:
    enum E_Event_Type
    {
        E_Event_Vsync = 0,
        E_Event_Af,
        E_Event_Vsync_Sensor
    };

    enum E_TgInfo_Path
    {
        E_TG1 = 0,
        E_TG2
    };

    struct ConfigParam
    {
        ConfigParam()
            : i4SensorDev(0), i4TgInfo(E_TG1), eEventType(E_Event_Vsync) {}
        ConfigParam(MINT32 _i4SensorDev, MINT32 _i4TgInfo, E_Event_Type _eEventType)
            : i4SensorDev(_i4SensorDev), i4TgInfo(_i4TgInfo), eEventType(_eEventType)
            {}
        MINT32 i4SensorDev;
        MINT32 i4TgInfo;
        E_Event_Type eEventType;
    };

    struct Duration
    {
        MINT32 i4Duration0;     // time duration since marked
        MINT32 i4Duration1;     // time duration since the last event happend
    };

    /**
     * @brief Create instance of IEventIrq
     * @param [in] rCfg config of sensor, tg, and event for listening.
     * @param [in] strUser username
     */
    static IEventIrq*       createInstance(const ConfigParam& rCfg, const char* strUser);

    /**
     * @brief Destroy instance of IEventIrq
     * @param [in] strUser username
     */
    virtual MVOID           destroyInstance(const char* strUser) = 0;

    /**
     * @brief Register to listen the event
     * @return
     * - MINT32 0 for success
     */
    virtual MINT32          mark() = 0;

    /**
     * @brief Wait for the event
     * @param [out] rDuration
     * @return
     * - 0 for blocking wait case
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32          wait(Duration& rDuration) = 0;

    /**
     * @brief Query for the event
     * @param [out] rDuration
     * @return
     * - 0 for indicating the event not yet happened
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32          query(Duration& rDuration) = 0;

    virtual MINT32          setTgInfo(MINT32 tgInfo) = 0;
protected:
    virtual ~IEventIrq(){}
};
};
#endif //__IEVENT_IRQ_H__
