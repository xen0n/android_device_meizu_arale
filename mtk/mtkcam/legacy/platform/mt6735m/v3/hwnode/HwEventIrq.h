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

#ifndef _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWEVENTIRQ_H_
#define _MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWEVENTIRQ_H_

#include <mtkcam/common.h>
#include <utils/Mutex.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>

namespace NSCam {
namespace v3 {

class HwEventIrq
{
public:
    enum E_Event_Type
    {
        E_Event_Vsync = 0,
    };

    struct ConfigParam
    {
        ConfigParam()
            : i4SensorDev(0), i4TgInfo(0), eEventType(E_Event_Vsync) {}
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
     * @brief Create instance of HwEventIrq
     * @param [in] rCfg config of sensor, tg, and event for listening.
     * @param [in] strUser username
     */
    static HwEventIrq*       createInstance(const ConfigParam& rCfg, const char* strUser);

    /**
     * @brief Destroy instance of HwEventIrq
     * @param [in] strUser username
     */
    virtual MVOID           destroyInstance(const char* strUser);

    /**
     * @brief Register to listen the event
     * @return
     * - MTRUE for success
     */
    virtual MBOOL          mark();

    /**
     * @brief Wait for the event
     * @return
     * - 0 for blocking wait case
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32          wait(Duration& rDuration);

    /**
     * @brief Query for the event
     * @return
     * - 0 for indicating the event not yet happened
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32          query(Duration& rDuration);

public:
                            HwEventIrq();
    virtual                ~HwEventIrq(){}

protected:
    MVOID                       init(const ConfigParam& rCfg, const char* strUser);
    MVOID                       uninit(const char* strUser);

    mutable android::Mutex      m_Lock;
    ConfigParam                 m_rCfgParam;

    NSIspDrv_FrmB::IspDrv*      m_pIspDrv;
    NSIspDrv_FrmB::ISP_DRV_WAIT_IRQ_STRUCT     m_rWaitIrq;

};
};
};


#endif //_MTK_HARDWARE_INCLUDE_MTKCAM_V3_HWEVENTIRQ_H_