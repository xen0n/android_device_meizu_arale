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
#define LOG_TAG "CamShot/BurstShot"
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
//
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>

#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/IBurstShot.h>
//
//
#include "../inc/CamShotImp.h"
#include "../inc/BurstShot.h"
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
class IBurstShotBridge : public IBurstShot
{
    friend  class   IBurstShot;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  ////
    //mutable android::Mutex      mLock;
    //android::Mutex&             getLockRef()    { return mLock; }
    MUINT32                     mu4InitRefCount;

protected:  ////    Implementor.
    BurstShot*const            mpBurstShotImp;
    inline  BurstShot const*   getImp() const  { return mpBurstShotImp; }
    inline  BurstShot*         getImp()        { return mpBurstShotImp; }

protected:  ////    Constructor/Destructor.
                    IBurstShotBridge(BurstShot*const pBurstShot);
                    ~IBurstShotBridge();

private:    ////    Disallowed.
                    IBurstShotBridge(IBurstShotBridge const& obj);
    IBurstShotBridge&  operator=(IBurstShotBridge const& obj);

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
                            IImageBuffer const *pBufPrv_0=NULL, IImageBuffer const *pBufPrv_1=NULL) { return MTRUE; }
    virtual MBOOL   stop();


public:     ////    Settings.
    virtual MBOOL   setShotParam(ShotParam const & rParam);
    virtual MBOOL   setJpegParam(JpegParam const & rParam);

public:     ////    buffer setting.
    virtual MBOOL   registerImageBuffer(ECamShotImgBufType, IImageBuffer const *pImgBuffer);



public:     ////    Old style commnad.
    virtual MBOOL
    sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

public:     ////    Info.
    virtual MBOOL   setPrvBufHdl(MVOID* pCamBufHdl);
    virtual MBOOL   setCapBufMgr(MVOID* pCapBufMgr);

public:
    virtual MBOOL   registerCap3AParam(NS3A::CaptureParam_T* pCapParams, MUINT32 const length);
    virtual MBOOL   setIspProfile(NS3A::EIspProfile_T const profile);

};


/*******************************************************************************
*
********************************************************************************/
IBurstShot*
IBurstShot::
createInstance(EShotMode const eShotMode, char const*const szCamShotName)
{
    BurstShot* pBurstShotImp = new BurstShot(eShotMode, "BurstShot");
    if  ( ! pBurstShotImp )
    {
        MY_LOGE("[IBurstShot] fail to new BurstShot");
        return  NULL;
    }
    //
    IBurstShotBridge*  pIBurstShot = new IBurstShotBridge(pBurstShotImp);
    if  ( ! pIBurstShot )
    {
        MY_LOGE("[IBurstShot] fail to new IBurstShotBridge");
        delete  pBurstShotImp;
        return  NULL;
    }
    //
    return  pIBurstShot;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IBurstShotBridge::
destroyInstance()
{
    delete  mpBurstShotImp;  //  Firstly, delete the implementor here instead of destructor.
    delete  this;       //  Finally, delete myself.
}


/*******************************************************************************
*
********************************************************************************/
IBurstShotBridge::
IBurstShotBridge(BurstShot*const pBurstShot)
    : IBurstShot()
    //, mLock()
    , mu4InitRefCount(0)
    , mpBurstShotImp(pBurstShot)
{
}


/*******************************************************************************
*
********************************************************************************/
IBurstShotBridge::
~IBurstShotBridge()
{
}


/*******************************************************************************
*
********************************************************************************/
char const*
IBurstShotBridge::
getCamShotName() const
{
    return  getImp()->getCamShotName();
}


/*******************************************************************************
*
********************************************************************************/
EShotMode
IBurstShotBridge::
getShotMode() const
{
    return  getImp()->getShotMode();
}


/*******************************************************************************
*
********************************************************************************/
MINT32
IBurstShotBridge::
getLastErrorCode() const
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->getLastErrorCode();
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
init()
{
    MBOOL   ret = MTRUE;
    //Mutex::Autolock _lock(mLock);

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
IBurstShotBridge::
uninit()
{
    MBOOL   ret = MTRUE;
    //Mutex::Autolock _lock(mLock);

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
IBurstShotBridge::
setCallbacks(CamShotNotifyCallback_t notify_cb, CamShotDataCallback_t data_cb, MVOID* user)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->setCallbacks(notify_cb, data_cb, user);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
isNotifyMsgEnabled(MINT32 const i4MsgTypes) const
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->isNotifyMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IBurstShotBridge::
enableNotifyMsg(MINT32 const i4MsgTypes)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->enableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IBurstShotBridge::
disableNotifyMsg(MINT32 const i4MsgTypes)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->disableNotifyMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
isDataMsgEnabled(MINT32 const i4MsgTypes) const
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->isDataMsgEnabled(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IBurstShotBridge::
enableDataMsg(MINT32 const i4MsgTypes)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->enableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
IBurstShotBridge::
disableDataMsg(MINT32 const i4MsgTypes)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->disableDataMsg(i4MsgTypes);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
start(SensorParam const & rSensorParam, MUINT32 u4ShotCount)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->start(rSensorParam, u4ShotCount);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
startAsync(SensorParam const & rSensorParam)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->startAsync(rSensorParam);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
startOne(SensorParam const & rSensorParam)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->startOne(rSensorParam);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->startOne(rSensorParam, pImgBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
stop()
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->stop();
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
setShotParam(ShotParam const & rParam)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->setShotParam(rParam);
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
setJpegParam(JpegParam const & rParam)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->setJpegParam(rParam);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->registerImageBuffer(eBufType, pImgBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
setPrvBufHdl(MVOID* pCamBufHdl)
{
    return  getImp()->setPrvBufHdl(pCamBufHdl);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
setCapBufMgr(MVOID* pCapBufMgr)
{
    return  getImp()->setCapBufMgr(pCapBufMgr);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    //Mutex::Autolock _lock(mLock);
    return  getImp()->sendCommand(cmd, arg1, arg2, arg3);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
registerCap3AParam(NS3A::CaptureParam_T* pCapParams, MUINT32 const length)
{
    return  getImp()->registerCap3AParam(pCapParams, length);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
IBurstShotBridge::
setIspProfile(NS3A::EIspProfile_T const profile)
{
    return  getImp()->setIspProfile(profile);
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

