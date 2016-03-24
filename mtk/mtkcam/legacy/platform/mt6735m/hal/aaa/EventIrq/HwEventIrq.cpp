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
* @file HwEventIrq.cpp
* @brief Declarations of Hw IRQ Event waiting interface
*/
#define LOG_TAG "HwIRQ3A"

#include "IEventIrq.h"

#include <string>
#include <aaa_types.h>
#include <aaa_log.h>

#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>

namespace NS3Av3
{
using namespace android;
using namespace NSIspDrv_FrmB;
/******************************************************************************
 *  Default EventIrq
 ******************************************************************************/
class HwEventIrq : public IEventIrq
{
public:
    /**
     * @brief Create instance of IEventIrq
     * @param [in] rCfg config of sensor, tg, and event for listening.
     * @param [in] strUser username
     */
    static HwEventIrq*       createInstance(const ConfigParam& rCfg, const char* strUser);

    /**
     * @brief Destroy instance of IEventIrq
     * @param [in] strUser username
     */
    virtual MVOID               destroyInstance(const char* strUser);

    /**
     * @brief Register to listen the event
     * @return
     * - MINT32 0 for success
     */
    virtual MINT32              mark();

    /**
     * @brief Wait for the event
     * @param [out] rDuration
     * @return
     * - 0 for blocking wait case
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              wait(Duration& rDuration);

    /**
     * @brief Query for the event
     * @param [out] rDuration
     * @return
     * - 0 for indicating the event not yet happened
     * - -1 indicates error
     * - other values indicate the number of event happened
     */
    virtual MINT32              query(Duration& rDuration);

protected:
    HwEventIrq();
    virtual ~HwEventIrq(){}

    MVOID                       init(const ConfigParam& rCfg, const char* strUser);
    MVOID                       uninit(const char* strUser);

    MINT32                      m_i4User;
    mutable Mutex               m_Lock;
    //mutable Mutex               m_EventMtx;
    ConfigParam                 m_rCfgParam;
    std::string                 m_strName;

    IspDrv*                     m_pIspDrv;
    ISP_DRV_WAIT_IRQ_STRUCT     m_rWaitIrq;
};

template<HwEventIrq::E_Event_Type eType>
class HwEventIrqType : public HwEventIrq
{
public:
    static HwEventIrqType* getInstance()
    {
        static HwEventIrqType<eType> singleton;
        return &singleton;
    }

};

HwEventIrq::
HwEventIrq()
    : m_i4User(0)
    , m_Lock()
    //, m_EventMtx()
    , m_pIspDrv(NULL)
{
    MY_LOG("[%s]", __FUNCTION__);
}

HwEventIrq*
HwEventIrq::
createInstance(const ConfigParam& rCfg, const char* strUser)
{
    HwEventIrq* pHwEventIrq = NULL;
    switch (rCfg.eEventType)
    {
        case E_Event_Vsync:
        pHwEventIrq = HwEventIrqType<E_Event_Vsync>::getInstance();
        break;
        case E_Event_Af:
        pHwEventIrq = HwEventIrqType<E_Event_Af>::getInstance();
        break;
    case E_Event_Vsync_Sensor:
        pHwEventIrq = HwEventIrqType<E_Event_Vsync_Sensor>::getInstance();
        break;
    }

    if (pHwEventIrq)
    {
        pHwEventIrq->init(rCfg, strUser);
        return pHwEventIrq;
    }

    static HwEventIrq _rHwEventIrq;
    _rHwEventIrq.init(rCfg, strUser);
    return &_rHwEventIrq;
}

MVOID
HwEventIrq::
destroyInstance(const char* strUser)
{
    uninit(strUser);
}

MVOID
HwEventIrq::
init(const ConfigParam& rCfg, const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User > 0)
    {
        MY_LOG_IF(1, "[%s] m_i4User(%d), m_strName(%s), strUser(%s)", __FUNCTION__, m_i4User, m_strName.c_str(), strUser);
        m_i4User ++;
        return;
    }

    // init
    m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv)
    {
        MY_ERR("IspDrv::createInstance() fail");
    }
    else
    {
        if (!m_pIspDrv->init(LOG_TAG))
        {
            MY_ERR("pIspDrv->init() fail");
        }
    }

    MINT32 i4UserKey = m_pIspDrv->registerIrq(strUser);
    m_rWaitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
    m_rWaitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT;
    m_rWaitIrq.UserInfo.Status  = (m_rCfgParam.eEventType == E_Event_Vsync || m_rCfgParam.eEventType == E_Event_Vsync_Sensor) ?
                                /*ISP_DRV_IRQ_INT_STATUS_VS1_ST*/ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST : ISP_DRV_IRQ_INT_STATUS_AF_DON_ST/*AF*/;
    m_rWaitIrq.UserInfo.UserKey = i4UserKey;
    m_rWaitIrq.Timeout          = (m_rCfgParam.eEventType == E_Event_Vsync || m_rCfgParam.eEventType == E_Event_Vsync_Sensor) ? 5000 : 500/*AF*/;
    m_rWaitIrq.bDumpReg         = 0;

    m_rCfgParam = rCfg;
    m_strName = strUser;

    m_i4User ++;

    MY_LOG("[%s] m_strName(%s), m_pIspDrv(%p), userKey(%d), cfg(%d, %d, %d)", __FUNCTION__, m_strName.c_str(), m_pIspDrv, i4UserKey, m_rCfgParam.i4SensorDev, m_rCfgParam.i4TgInfo, m_rCfgParam.eEventType);
}

MVOID
HwEventIrq::
uninit(const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    if (m_i4User <= 0)
    {
        return;
    }

    m_i4User --;

    if (m_i4User == 0)
    {
        // uninit
        MY_LOG("[%s] m_strName(%s), strUser(%s)", __FUNCTION__, m_strName.c_str(), strUser);
        if (m_pIspDrv)
        {
            if (!m_pIspDrv->uninit(LOG_TAG))
            {
                MY_ERR("m_pIspDrv->uninit() fail");
            }
        }
        m_pIspDrv = MNULL;
    }
    else
    {
        // do nothing
    }
}

MINT32
HwEventIrq::
mark()
{
    if (!m_pIspDrv->markIrq(m_rWaitIrq))
    {
        MY_ERR("Error");
        return -1;
    }
    else
    {
        MY_LOG("[%s] %s", __FUNCTION__, m_strName.c_str());
        return 0;
    }
}

MINT32
HwEventIrq::
query(Duration& rDuration)
{
    ISP_DRV_WAIT_IRQ_STRUCT rWaitIrq = m_rWaitIrq;

    if (!m_pIspDrv->queryirqtimeinfo(&rWaitIrq))
    {
        MY_ERR("Error");
        return -1;
    }
    else
    {
        rDuration.i4Duration0 = rWaitIrq.TimeInfo.tmark2read_sec*1000000 + rWaitIrq.TimeInfo.tmark2read_usec;
        rDuration.i4Duration1 = rWaitIrq.TimeInfo.tevent2read_sec*1000000 + rWaitIrq.TimeInfo.tevent2read_usec;
        MY_LOG("[%s] %s: T0(%d), T1(%d), EventCnt(%d)", __FUNCTION__, m_strName.c_str(), rDuration.i4Duration0, rDuration.i4Duration1, rWaitIrq.TimeInfo.passedbySigcnt);
        return rWaitIrq.TimeInfo.passedbySigcnt;
    }
}

MINT32
HwEventIrq::
wait(Duration& rDuration)
{
    ISP_DRV_WAIT_IRQ_STRUCT rWaitIrq = m_rWaitIrq;

    if (!m_pIspDrv->waitIrq(&rWaitIrq))
    {
        MY_ERR("Error");
        return -1;
    }
    else
    {
        rDuration.i4Duration0 = rWaitIrq.TimeInfo.tmark2read_sec*1000000 + rWaitIrq.TimeInfo.tmark2read_usec;
        rDuration.i4Duration1 = rWaitIrq.TimeInfo.tevent2read_sec*1000000 + rWaitIrq.TimeInfo.tevent2read_usec;
        MY_LOG("[%s] %s: T0(%d), T1(%d), EventCnt(%d)", __FUNCTION__, m_strName.c_str(), rDuration.i4Duration0, rDuration.i4Duration1, rWaitIrq.TimeInfo.passedbySigcnt);
        return rWaitIrq.TimeInfo.passedbySigcnt;
    }
}

#if (CAM3_3ATESTLVL > CAM3_3AUT)
IEventIrq*
IEventIrq::
createInstance(const ConfigParam& rCfg, const char* strUser)
{
    return HwEventIrq::createInstance(rCfg, strUser);
}
#endif

};
