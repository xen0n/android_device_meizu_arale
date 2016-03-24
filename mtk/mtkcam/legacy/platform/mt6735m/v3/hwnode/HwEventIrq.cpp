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

#define LOG_TAG "HwEventIrq"

#include <mtkcam/Log.h>
//
#include "HwEventIrq.h"
//
using namespace android;
using namespace NSIspDrv_FrmB;
using namespace NSCam::v3;

/******************************************************************************
 *
 ******************************************************************************/
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


/******************************************************************************
 *
 ******************************************************************************/
HwEventIrq::
HwEventIrq()
    : m_Lock()
    , m_pIspDrv(NULL)
{
}


/******************************************************************************
 *
 ******************************************************************************/
HwEventIrq*
HwEventIrq::
createInstance(const ConfigParam& rCfg, const char* strUser)
{
    HwEventIrq* obj = new HwEventIrq();
    obj->init(rCfg, strUser);
    return obj;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HwEventIrq::
destroyInstance(const char* strUser)
{
    uninit(strUser);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HwEventIrq::
init(const ConfigParam& rCfg, const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    // init
    m_pIspDrv = IspDrv::createInstance();
    if (!m_pIspDrv)
    {
        MY_LOGE("IspDrv::createInstance() fail");
    }
    else
    {
        if (!m_pIspDrv->init(strUser))
        {
            MY_LOGE("pIspDrv->init() fail");
        }
    }

    MINT32 i4UserKey = m_pIspDrv->registerIrq(strUser);
    m_rWaitIrq.Clear            = ISP_DRV_IRQ_CLEAR_WAIT;
    m_rWaitIrq.UserInfo.Type    = ISP_DRV_IRQ_TYPE_INT;
    if (m_rCfgParam.eEventType == E_Event_Vsync) {
        m_rWaitIrq.UserInfo.Status = ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
    }
    m_rWaitIrq.UserInfo.UserKey = i4UserKey;
    m_rWaitIrq.Timeout          = 5000;
    m_rWaitIrq.bDumpReg         = 0;

    m_rCfgParam = rCfg;

    MY_LOGD("m_pIspDrv(%p), userKey(%d), cfg(%d, %d, %d)", m_pIspDrv, i4UserKey, m_rCfgParam.i4SensorDev, m_rCfgParam.i4TgInfo, m_rCfgParam.eEventType);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
HwEventIrq::
uninit(const char* strUser)
{
    Mutex::Autolock lock(m_Lock);

    if (m_pIspDrv){
        if (!m_pIspDrv->uninit(strUser)) {
            MY_LOGE("fail");
        }
        m_pIspDrv = NULL;
    } else {
        // do nothing
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HwEventIrq::
mark()
{
    //Mutex::Autolock lock(m_EventMtx);

    if (!m_pIspDrv->markIrq(m_rWaitIrq))
    {
        MY_LOGE("Error");
        return MFALSE;
    }
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
HwEventIrq::
query(Duration& rDuration)
{
    //Mutex::Autolock lock(m_EventMtx);

    if (!m_pIspDrv->queryirqtimeinfo(&m_rWaitIrq))
    {
        MY_LOGE("Error");
        return -1;
    }
    else
    {
        rDuration.i4Duration0 = m_rWaitIrq.TimeInfo.tmark2read_sec*1000000 + m_rWaitIrq.TimeInfo.tmark2read_usec;
        rDuration.i4Duration1 = m_rWaitIrq.TimeInfo.tevent2read_sec*1000000 + m_rWaitIrq.TimeInfo.tevent2read_usec;
        MY_LOGD("T0(%d), T1(%d), EventCnt(%d)", rDuration.i4Duration0, rDuration.i4Duration1, m_rWaitIrq.TimeInfo.passedbySigcnt);
        return m_rWaitIrq.TimeInfo.passedbySigcnt;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
HwEventIrq::
wait(Duration& rDuration)
{
    //Mutex::Autolock lock(m_EventMtx);

    if (!m_pIspDrv->waitIrq(&m_rWaitIrq))
    {
        MY_LOGE("Error");
        return -1;
    }
    else
    {
        rDuration.i4Duration0 = m_rWaitIrq.TimeInfo.tmark2read_sec*1000000 + m_rWaitIrq.TimeInfo.tmark2read_usec;
        rDuration.i4Duration1 = m_rWaitIrq.TimeInfo.tevent2read_sec*1000000 + m_rWaitIrq.TimeInfo.tevent2read_usec;
        MY_LOGD("T0(%d), T1(%d), EventCnt(%d)", rDuration.i4Duration0, rDuration.i4Duration1, m_rWaitIrq.TimeInfo.passedbySigcnt);
        return m_rWaitIrq.TimeInfo.passedbySigcnt;
    }
}

