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
#define LOG_TAG "CamShot/StereoShot"
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#define MY_LOGV(fmt, arg...)    CAM_LOGV("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    CAM_LOGD("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    CAM_LOGI("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    CAM_LOGW("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    CAM_LOGE("[%s] " fmt, __FUNCTION__, ##arg)
//
using namespace NSCam;
//#include <common/hw/hwstddef.h>
//
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>

#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/IStereoShot.h>
//
//
#include "../inc/CamShotImp.h"
#include "../inc/StereoShot.h"
//
#include <utils/Mutex.h>
using namespace android;


/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class IStereoShotBridge : public IStereoShot
{
    friend  class   IStereoShot;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    mutable android::Mutex      mLock;
    android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    StereoShot*const            mpStereoShotImp;
    inline  StereoShot const*   getImp() const  { return mpStereoShotImp; }
    inline  StereoShot*         getImp()        { return mpStereoShotImp; }

protected:  ////    Constructor/Destructor.
                    IStereoShotBridge(StereoShot*const pStereoShot);
                    ~IStereoShotBridge();

private:    ////    Disallowed.
                    IStereoShotBridge(IStereoShotBridge const& obj);
    IStereoShotBridge&  operator=(IStereoShotBridge const& obj);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Instantiation.
    virtual MVOID   destroyInstance();
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Attributes.
    virtual char const* getCamShotName() const;
    virtual EShotMode     getShotMode() const;
    virtual MINT32      getLastErrorCode() const;

public:     ////    Callbacks.
    virtual MVOID   setCallbacks(CamShotNotifyCallback_t notify_cb, CamShotDataCallback_t data_cb, MVOID* user);
    //
    //  notify callback
    virtual MBOOL   isNotifyMsgEnabled(MINT32 const i4MsgTypes) const;
    virtual MVOID   enableNotifyMsg(MINT32 const i4MsgTypes);
    virtual MVOID   disableNotifyMsg(MINT32 const i4MsgTypes);
    //
    //  data callback
    virtual MBOOL   isDataMsgEnabled(MINT32 const i4MsgTypes) const;
    virtual MVOID   enableDataMsg(MINT32 const i4MsgTypes);
    virtual MVOID   disableDataMsg(MINT32 const i4MsgTypes);

public:     ////    Operations.
    virtual MBOOL   start(SensorParam const & rSensorParam, MUINT32 u4ShotCount=0xFFFFFFFF);
    virtual MBOOL   startAsync(SensorParam const & rSensorParam) ;
    virtual MBOOL   startOne(SensorParam const & rSensorParam);
    virtual MBOOL   startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer);
    virtual MBOOL   startOne(SensorParam const & rSensorParam,
                            IImageBuffer const *pImgBuf_0, IImageBuffer const *pImgBuf_1,
                            IImageBuffer const *pBufPrv_0=NULL, IImageBuffer const *pBufPrv_1=NULL);
    virtual MBOOL   stop();


public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);

public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType, IImageBuffer const *pImgBuffer);

public:     ////    Info.
    virtual MBOOL   setPrvBufHdl(MVOID* pCamBufHdl);
    virtual MBOOL   setCapBufMgr(MVOID* pCapBufMgr);

public:     ////    Old style commnad.
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

};


/*******************************************************************************
*
********************************************************************************/
IStereoShot*
IStereoShot::
createInstance(EShotMode const eShotMode, char const*const szCamShotName)
{
    StereoShot* pStereoShotImp = new StereoShot(eShotMode, "StereoShot");
    if  ( ! pStereoShotImp )
    {
        MY_LOGE("[IStereoShot] fail to new StereoShot");
        return  NULL;
    }
    //
    IStereoShotBridge*  pIStereoShot = new IStereoShotBridge(pStereoShotImp);
    if  ( ! pIStereoShot )
    {
        MY_LOGE("[IStereoShot] fail to new IStereoShotBridge");
        delete  pStereoShotImp;
        return  NULL;
    }
    //
    return  pIStereoShot;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
destroyInstance()
{
    delete  mpStereoShotImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
IStereoShotBridge::
IStereoShotBridge(StereoShot*const pStereoShot)
    : IStereoShot()
    , mLock()
    , mu4InitRefCount(0)
    , mpStereoShotImp(pStereoShot)
{
}


/*******************************************************************************
*
********************************************************************************/
IStereoShotBridge::
~IStereoShotBridge()
{
}


/*******************************************************************************
*
********************************************************************************/
char const*
IStereoShotBridge::
getCamShotName() const
{
    return  getImp()->getCamShotName();
}


/*******************************************************************************
*
********************************************************************************/
EShotMode
IStereoShotBridge::
getShotMode() const
{
    return  getImp()->getShotMode();
}


/*******************************************************************************
*
********************************************************************************/
MINT32
IStereoShotBridge::
getLastErrorCode() const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
init()
{
    MBOOL   ret = MTRUE;
    Mutex::Autolock _lock(mLock);

    if  ( 0 != mu4InitRefCount )
    {
        mu4InitRefCount++;
    }
    else if ( (ret = getImp()->init()) )
    {
        mu4InitRefCount = 1;
    }
    MY_LOGD("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
uninit()
{
    MBOOL   ret = MTRUE;
    Mutex::Autolock _lock(mLock);

    if  ( 0 < mu4InitRefCount )
    {
        mu4InitRefCount--;
        if  ( 0 == mu4InitRefCount )
        {
            ret = getImp()->uninit();
        }
    }
    MY_LOGD("- mu4InitRefCount(%d), ret(%d)", mu4InitRefCount, ret);
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
setCallbacks(CamShotNotifyCallback_t notify_cb, CamShotDataCallback_t data_cb, MVOID* user)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setCallbacks(notify_cb, data_cb, user);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
isNotifyMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isNotifyMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
enableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
disableNotifyMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
isDataMsgEnabled(MINT32 const i4MsgTypes) const
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->isDataMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
enableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->enableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IStereoShotBridge::
disableDataMsg(MINT32 const i4MsgTypes)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->disableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
start(SensorParam const & rSensorParam, MUINT32 u4ShotCount)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->start(rSensorParam);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
startAsync(SensorParam const & rSensorParam)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->startAsync(rSensorParam);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
startOne(SensorParam const & rSensorParam)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->startOne(rSensorParam);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->startOne(rSensorParam, pImgBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
startOne(
    SensorParam const & rSensorParam,
    IImageBuffer const *pImgBuf_0,
    IImageBuffer const *pImgBuf_1,
    IImageBuffer const *pBufPrv_0,
    IImageBuffer const *pBufPrv_1
)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->startOne(rSensorParam, pImgBuf_0, pImgBuf_1, pBufPrv_0, pBufPrv_1);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
stop()
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->stop();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
setShotParam(ShotParam const & rParam)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setShotParam(rParam);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
setJpegParam(JpegParam const & rParam)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->setJpegParam(rParam);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->registerImageBuffer(eBufType, pImgBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
setPrvBufHdl(MVOID* pCamBufHdl)
{
    return  getImp()->setPrvBufHdl(pCamBufHdl);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
setCapBufMgr(MVOID* pCapBufMgr)
{
    return  getImp()->setCapBufMgr(pCapBufMgr);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IStereoShotBridge::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    Mutex::Autolock _lock(mLock);
    return  getImp()->sendCommand(cmd, arg1, arg2, arg3);
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

