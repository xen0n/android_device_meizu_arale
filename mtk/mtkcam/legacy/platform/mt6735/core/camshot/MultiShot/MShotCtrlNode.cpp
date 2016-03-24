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
#define LOG_TAG "MtkCam/MShotCtrl"
//
#include <mtkcam/Log.h>
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt, __func__, ##arg)
#define FUNC_START                  MY_LOGD("+")
#define FUNC_END                    MY_LOGD("-")
//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <list>
using namespace std;
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/camnode/IspSyncControl.h>
#include "../inc/MShotCtrlNode.h"
using namespace NSCamNode;
//
#include <sys/time.h>

#define MODULE_NAME_CTRL    "MShotCtrl"
#define SCHED_POLICY        (SCHED_OTHER)
#define SCHED_PRIORITY      (NICE_CAMERA_CAPTURE)
//
#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
//
#define MULTISHOT_SYNC_FRAME (0)
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

class CtrlNodeImpl : public MShotCtrlNode
{
    public:     /*createInstance()*/
        CtrlNodeImpl(ShotParam const & rShotParam);
        ~CtrlNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

        virtual MVOID   setFps(MUINT32 const jpegfps, MUINT32 const prvFps);
        virtual MBOOL   setIspProfile(NS3A::EIspProfile_T const profile)
        {
            mIspProfile = profile;
            return MTRUE;
        }

    protected:

        MVOID   updateCapP2Ready();
        MBOOL   isDoCapture();
        MVOID   triggerCaptureP2();
        MBOOL   generateIspTuningP2(
                    MUINT32 const magic,
                    EIspProfile_T const profile,
                    MBOOL const rpgon,
                    IDbgInfoContainer* pDbgInfo
                    );
        MBOOL   readyToCapture(MUINT32 const magic) {
            Mutex::Autolock _l(mLock);
            return (m3aReadyMagic != MAGIC_NUM_INVALID) && (magic >= m3aReadyMagic);
        }

        virtual MBOOL _init()                                                              = 0;
        virtual MBOOL _uninit()                                                            = 0;
        virtual MBOOL _onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) = 0;

        MVOID   setFlag(MUINT32 const msg, MBOOL const enable) {
            Mutex::Autolock _l(mLock);
            if( enable )
                muFlag |= msg;
            else
                muFlag &= ~msg;
        }

        MBOOL   getFlag(MUINT32 const msg ) {
            Mutex::Autolock _l(mLock);
            return muFlag & msg;
        }

    protected:

        enum{
            NONE            = 0x0,
            SENSOR_READY    = 0x1,
            DO_CAPTURE      = 0x2,
            DO_PREVIEW      = 0x4,
            FIRST_CAP_FRAME = 0x8,
            FIRST_PRV_FRAME = 0x10,
            CAPPATH_READY   = 0x20,
        };

        IHal3A*               mpHal3A;
        IspSyncControl*       mpIspSyncCtrl;

        // buffer control
        mutable Mutex         mLock; //protect following data
        MBOOL                 mbStop;
        IImageBuffer*         mpP1Buffer;
        IDbgInfoContainer*    mpDbgInfo[2]; //ping-pong buffer
        Condition             mCondTrigger;

        // status flag
        MUINT32               muFlag;

        // 3A related
        MUINT32               mCurMagicNum;
        MUINT32               m3aReadyMagic;

        // following data are not protected by mLock
        // data flow related
        MUINT32               muCurShotCount;
        MBOOL                 mbCapConnected;
        MBOOL                 mbPrvConnected;

        // fps control
        MUINT32               muPrvInterval;  //in us
        MUINT32               muJpegInterval;  //in us
        timeval               mtLastPrv;
        timeval               mtLastCap;

        EIspProfile_T         mIspProfile;
};


class MultiControl : public CtrlNodeImpl //with p1
                   , public I3ACallBack
{
    public:

        MultiControl(ShotParam const & rShotParam)
            : CtrlNodeImpl(rShotParam)
        {};
        ~MultiControl() {};

    protected:

        // flash on
        MBOOL handle_CapFixed(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque);
        // flash off
        MBOOL handle_Update(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque);

        virtual MBOOL _init();
        virtual MBOOL _uninit();
        virtual MBOOL _onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);

        //I3ACallBack
        virtual void doNotifyCb ( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3, MINTPTR _ext4 = 0);

        virtual void doDataCb ( int32_t _msgType, void*   _data, uint32_t _size);

    protected:

        MBOOL                 mbFlashOn;
};


class ZsdControl : public CtrlNodeImpl //without p1
{
    public:

        ZsdControl(ShotParam const & rShotParam)
            : CtrlNodeImpl(rShotParam)
        {};
        ~ZsdControl() {};

    protected:

        virtual MBOOL _init();
        virtual MBOOL _uninit();
        virtual MBOOL _onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);

    private:
};


/*******************************************************************************
*
********************************************************************************/
MShotCtrlNode*
MShotCtrlNode::
createInstance(ShotParam const & rShotParam, MBOOL const withP1)
{
    if( withP1 )
        return new MultiControl(rShotParam); // with or without flash

    return new ZsdControl(rShotParam);
}


/*******************************************************************************
*
********************************************************************************/
void
MShotCtrlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
MShotCtrlNode::
MShotCtrlNode(ShotParam const & rShotParam)
    : ICamThreadNode( MODULE_NAME_CTRL, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
    , mShotParam(rShotParam)
{
}


/*******************************************************************************
*
********************************************************************************/
MShotCtrlNode::
~MShotCtrlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
CtrlNodeImpl::
CtrlNodeImpl(ShotParam const & rShotParam)
    : MShotCtrlNode(rShotParam)
    , mpHal3A(NULL)
    , mpIspSyncCtrl(NULL)
    , mbStop(MFALSE)
    , mpP1Buffer(NULL)
    , muFlag(FIRST_CAP_FRAME|FIRST_PRV_FRAME|CAPPATH_READY)
    , mCurMagicNum(0)
    , m3aReadyMagic(MAGIC_NUM_INVALID)
    , muCurShotCount(0)
    , mbCapConnected(MFALSE)
    , mbPrvConnected(MFALSE)
    , muPrvInterval(0)
    , muJpegInterval(0)
    , mIspProfile(EIspProfile_VSS_Capture)
{
    // p1 -> p2
    addDataSupport( ENDPOINT_SRC, CONTROL_FULLRAW );
    addDataSupport( ENDPOINT_SRC, CONTROL_RESIZEDRAW );

    addDataSupport( ENDPOINT_DST, CONTROL_PRV_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_CAP_SRC );

    addDataSupport( ENDPOINT_DST, CONTROL_DBGINFO );
    // notify
    addNotifySupport( CONTROL_SHUTTER );
    //
    mpDbgInfo[0] = NULL;
    mpDbgInfo[1] = NULL;
}


/*******************************************************************************
*
********************************************************************************/
CtrlNodeImpl::
~CtrlNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onInit()
{
    MBOOL ret = MFALSE;
    //
    mpIspSyncCtrl = IspSyncControl::createInstance(getSensorIdx());
    if( mpIspSyncCtrl == NULL )
    {
        MY_LOGE("IspSyncCtrl:createInstance fail");
        goto lbExit;
    }
    //
    mpIspSyncCtrl->setPreviewSize(mShotParam.u4PostViewWidth, mShotParam.u4PostViewHeight);
    mpIspSyncCtrl->setCurZoomRatio(mShotParam.u4ZoomRatio);
    //
    mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_1, getSensorIdx(), getName());
    if(mpHal3A == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }
    //
    //
    mbCapConnected = isDataConnected(CONTROL_CAP_SRC);
    mbPrvConnected = isDataConnected(CONTROL_PRV_SRC);
    //
    ret = _init();
    //
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onUninit()
{
    MBOOL ret = _uninit();
    //
    if(mpHal3A)
    {
        mpHal3A->destroyInstance(getName());
        mpHal3A = NULL;
    }
    //
    if(mpIspSyncCtrl)
    {
        mpIspSyncCtrl->destroyInstance();
        mpIspSyncCtrl = NULL;
    }
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onStart()
{
    MBOOL ret = MTRUE;
    //
    mbStop = MFALSE;
    //
    // trigger first frame
    if( isDataConnected(CONTROL_CAP_SRC) )
        updateCapP2Ready();
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onStop()
{
    FUNC_START;
    //
    MBOOL ret;

    {
        Mutex::Autolock _l(mLock);
        mbStop = MTRUE;
    }

    // to escape waiting
    mCondTrigger.signal();

    if( mpP1Buffer != NULL ) {
        handleReturnBuffer(CONTROL_FULLRAW, (MUINTPTR)mpP1Buffer);
        mpP1Buffer = NULL;
    }

    for( uint8_t i = 0; i < 2; i++ )
    {
        if( mpDbgInfo[i] ) {
            mpDbgInfo[i]->destroyInstance();
            mpDbgInfo[i] = NULL;
        }
    }

    ret = syncWithThread(); //wait for jobs done

    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    return _onNotify(msg, ext1, ext2);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL returnBuf = MTRUE;
    //MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            if( getFlag(DO_CAPTURE) ) {
                IImageBuffer* old = NULL;
                {
                    Mutex::Autolock _l(mLock);
#if MULTISHOT_SYNC_FRAME
                    if( mpP1Buffer == NULL && getFlag(CAPPATH_READY) )
                    {
                        // update the latest buffer
                        mpP1Buffer = reinterpret_cast<IImageBuffer*>(buf);
                        MY_LOGD("new 0x%x", mpP1Buffer);

                        returnBuf = MFALSE;
                    }
#else
                    // update the latest buffer
                    old = mpP1Buffer;
                    mpP1Buffer = reinterpret_cast<IImageBuffer*>(buf);
                    MY_LOGD("old 0x%x, new 0x%x", old, mpP1Buffer);

                    returnBuf = MFALSE;
#endif

                    // update debug info
                    if( mpDbgInfo[1] )
                        mpDbgInfo[1]->destroyInstance();
                    mpDbgInfo[1] = mpDbgInfo[0];
                    mpDbgInfo[0] = NULL;

                    // signal p1 buffer ready
                    mCondTrigger.signal();
                }
                if( old )
                    handleReturnBuffer(CONTROL_FULLRAW, (MUINTPTR)old);
            }
            break;
        case CONTROL_RESIZEDRAW:
            if( getFlag(DO_PREVIEW) ) {
                if( !getFlag(FIRST_PRV_FRAME) )
                {
                    timeval tv;
                    ::gettimeofday(&tv, NULL);
                    MINT32 usDiff = (tv.tv_sec - mtLastPrv.tv_sec)*1000000 +
                                    tv.tv_usec - mtLastPrv.tv_usec;
                    if( muPrvInterval <= usDiff )
                    {
                        returnBuf = MFALSE;
                        mtLastPrv = tv;
                    }
                    else
                    {
                        MY_LOGD("skip preview buffer");
                    }
                }
                else
                {
                    setFlag(FIRST_PRV_FRAME, MFALSE);
                    returnBuf = MFALSE;
                    ::gettimeofday(&mtLastPrv, NULL);
                }

                if( !returnBuf )
                    handlePostBuffer(CONTROL_PRV_SRC, buf);
            }
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }

    if( returnBuf )
        handleReturnBuffer(data, buf);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGV("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    switch(data)
    {
        case CONTROL_PRV_SRC:
            handleReturnBuffer(CONTROL_RESIZEDRAW, buf, ext);
            break;
        case CONTROL_CAP_SRC:
            handleReturnBuffer(CONTROL_FULLRAW, buf);
            updateCapP2Ready();
            break;
        case CONTROL_DBGINFO:
            if( buf != 0 )
            {
                IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(buf);
                pDbgInfo->destroyInstance();
            }
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
threadLoopUpdate()
{
    FUNC_START;

    if( isDoCapture() )
    {
        triggerCaptureP2();
    }

lbExit:
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CtrlNodeImpl::
updateCapP2Ready()
{
    MY_LOGD("capture path ready");
    setFlag(CAPPATH_READY, MTRUE);
    {
        Mutex::Autolock _l(mLock);
        if( !mbStop )
        {
            triggerLoop();
        }
        else
        {
            MY_LOGD("already stopped, no more trigger p2");
        }
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
isDoCapture()
{
    MBOOL docap = MFALSE;
    {
        Mutex::Autolock _l(mLock);
        if( mbStop )
            return 0;
    }

    if( getFlag(CAPPATH_READY) )
    {
        docap = MTRUE;
    } else
    {
        return 0;
    }

    // capture fps control
    if( !getFlag(FIRST_CAP_FRAME) )
    {
        // wait here
        timeval tv;
        ::gettimeofday(&tv, NULL);
        MINT32 usDiff = (tv.tv_sec - mtLastCap.tv_sec)*1000000 + tv.tv_usec - mtLastCap.tv_usec;
        MINT32 usWait = muJpegInterval - usDiff;
        if( usWait > 0 )
        {
            MY_LOGD("sleep %d us to capture", usWait);
            ::usleep(usWait);
        }
    }
    else
    {
        MY_LOGD("first");
        setFlag(FIRST_CAP_FRAME, MFALSE);
    }

    {
        Mutex::Autolock _l(mLock);
        if( mbStop )
            return 0;

        if( docap )
        {
            if( mpP1Buffer == NULL )
            {
                MY_LOGD("wait for p1 ready+");
                mCondTrigger.wait(mLock);
                MY_LOGD("wait for p1 ready-");
                if( mpP1Buffer == NULL || mbStop )
                {
                    MY_LOGD("no p1 buffer, going stop");
                    docap = 0;
                }
            }
        }
    }

    return docap;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CtrlNodeImpl::
triggerCaptureP2()
{
    setFlag(CAPPATH_READY, MFALSE);
    // update time
    ::gettimeofday(&mtLastCap, NULL);

    IImageBuffer* buf = NULL;
    IDbgInfoContainer* pDbg = NULL;
    { // get p1 buffer
        Mutex::Autolock _l(mLock);
        buf = mpP1Buffer;
        mpP1Buffer = NULL;

        pDbg = mpDbgInfo[1];
        mpDbgInfo[1] = NULL;
    }

    if( buf == NULL ) {
        MY_LOGE("buf == NULL");
        return;
    }

    if( pDbg )
        handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbg, 0);

    handlePostBuffer(CONTROL_CAP_SRC, (MUINTPTR)buf);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CtrlNodeImpl::
setFps(MUINT32 const jpegfps, MUINT32 const prvFps)
{
    Mutex::Autolock _l(mLock);

    MY_LOGD("fps jpeg %d, prv %d", jpegfps, prvFps);
    if( jpegfps > 0 )
        muJpegInterval = 1000000 / jpegfps;
    else
        muJpegInterval = 0;

    if( prvFps > 0 )
        muPrvInterval = 1000000 / prvFps;
    else
        muPrvInterval = 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiControl::
handle_CapFixed(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    if( mpHal3A == NULL )
        return MFALSE;

    switch(msg)
    {
        case PASS1_START_ISP:
            {
                MUINT32     magicNum = ext1;
                MUINT32     sensorScenario;
                MUINT32     sensorWidth;
                MUINT32     sensorHeight;
                MUINT32     sensorType;
                //
                mpIspSyncCtrl->getSensorInfo(
                        sensorScenario,
                        sensorWidth,
                        sensorHeight,
                        sensorType);
                //
                MY_LOGD("sensor sce 0x%x", sensorScenario);
                //
                mpHal3A->enterCaptureProcess();
                mpHal3A->setSensorMode(sensorScenario);
                mpHal3A->send3ACtrl(E3ACtrl_SetIsAEMultiCapture, 1, 0);
                mpHal3A->sendCommand(ECmd_CaptureStart);
                MY_LOGD("3A setIspProfile %d, # 0x%x",
                        EIspProfile_Preview,
                        magicNum );
                ParamIspProfile_T _3A_profile(
                        EIspProfile_Preview,
                        magicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3A->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            break;

        case PASS1_EOF:
            {
                const MUINT32 newMagicNum = ext1;
                const MUINT32 magicDeque = ext2;

                setFlag(DO_CAPTURE|DO_PREVIEW, MFALSE);
                // buf is ok
                if( magicDeque != MAGIC_NUM_INVALID ) {
                    setFlag(SENSOR_READY, MTRUE);
                    if( readyToCapture(magicDeque) ) {
                        if(mbCapConnected ) {
                            IDbgInfoContainer* pDbgInfo = NULL;
                            setFlag(DO_CAPTURE, MTRUE);

                            if( isDataConnected(CONTROL_DBGINFO) ) {
                                pDbgInfo = IDbgInfoContainer::createInstance();
                            }

                            if( muCurShotCount == 0 ) {
                                handleNotify(CONTROL_SHUTTER);

                                mpHal3A->sendCommand(ECmd_CaptureEnd);
                            }

                            generateIspTuningP2(
                                    SetCap(magicDeque),
                                    mIspProfile, MFALSE,
                                    pDbgInfo
                                    );

                            if( pDbgInfo != NULL ) {
                                Mutex::Autolock _l(mLock);
                                mpDbgInfo[0] = pDbgInfo;
                            }

                            MY_LOGD("curshotcount %d", muCurShotCount);
                            muCurShotCount++;
                        }

                        if(mbPrvConnected) {
                            setFlag(DO_PREVIEW, MTRUE);

                            generateIspTuningP2(
                                    magicDeque,
                                    EIspProfile_Preview, MTRUE,
                                    NULL
                                    );
                        }
                    }
                }

                if( newMagicNum != MAGIC_NUM_INVALID ) {
                    // do 3A update
                    mCurMagicNum = newMagicNum;
                    ParamIspProfile_T _3A_profile(
                            EIspProfile_Preview,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
                }
            }
            break;

        case PASS1_STOP_ISP:
            if(mpHal3A) {
                mpHal3A->send3ACtrl(E3ACtrl_SetIsAEMultiCapture, 0, 0);
                mpHal3A->exitCaptureProcess();
            }
            break;

        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiControl::
handle_Update(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            if(mpHal3A)
            {
                MUINT32     magicNum = ext1;
                MUINT32     sensorScenario;
                MUINT32     sensorWidth;
                MUINT32     sensorHeight;
                MUINT32     sensorType;
                //
                mpIspSyncCtrl->getSensorInfo(
                        sensorScenario,
                        sensorWidth,
                        sensorHeight,
                        sensorType);
                //
                MY_LOGD("sensor sce 0x%x", sensorScenario);
                //
                mpHal3A->setSensorMode(sensorScenario);
                mpHal3A->sendCommand(ECmd_CameraPreviewStart);
                MY_LOGD("3A setIspProfile %d, # 0x%x",
                        EIspProfile_Preview,
                        magicNum );
                ParamIspProfile_T _3A_profile(
                        EIspProfile_Preview,
                        magicNum,
                        MTRUE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3A->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);
            }
            break;

        case PASS1_EOF:
            if(mpHal3A) //check if 3A udpate ready
            {
                const MUINT32 newMagicNum = ext1;
                const MUINT32 magicDeque = ext2;

                setFlag(DO_CAPTURE|DO_PREVIEW, MFALSE);
                // buf is ok
                if( magicDeque != MAGIC_NUM_INVALID ) {
                    setFlag(SENSOR_READY, MTRUE);
                    if( readyToCapture(magicDeque) ) {
                        if(mbCapConnected ) {
                            IDbgInfoContainer* pDbgInfo = NULL;
                            setFlag(DO_CAPTURE, MTRUE);

                            if( isDataConnected(CONTROL_DBGINFO) ) {
                                pDbgInfo = IDbgInfoContainer::createInstance();
                            }

                            if( muCurShotCount == 0 ) {
                                handleNotify(CONTROL_SHUTTER);
                            }

                            generateIspTuningP2(
                                    SetCap(magicDeque),
                                    mIspProfile, MFALSE,
                                    pDbgInfo
                                    );

                            if( pDbgInfo != NULL ) {
                                Mutex::Autolock _l(mLock);
                                mpDbgInfo[0] = pDbgInfo;
                            }

                            MY_LOGD("curshotcount %d", muCurShotCount);
                            muCurShotCount++;
                        }

                        if(mbPrvConnected) {
                            setFlag(DO_PREVIEW, MTRUE);
                        }
                    }
                }
                //
                if( newMagicNum != MAGIC_NUM_INVALID ) {
                    // do 3A update
                    mCurMagicNum = newMagicNum;
                    ParamIspProfile_T _3A_profile(
                            EIspProfile_Preview,
                            mCurMagicNum,
                            MTRUE,
                            ParamIspProfile_T::EParamValidate_All);
                    mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
                }
            }
            break;

        case PASS1_STOP_ISP:
            if(mpHal3A) {
                mpHal3A->sendCommand(ECmd_CameraPreviewEnd);
            }
            break;
        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CtrlNodeImpl::
generateIspTuningP2(
        MUINT32 const magic,
        EIspProfile_T const profile,
        MBOOL const rpgon,
        IDbgInfoContainer* pDbgInfo
        )
{
    MY_LOGD("3A setIspProfile %d, # 0x%x", profile, magic );

    ParamIspProfile_T _3A_profile(
            profile,
            magic,
            rpgon,
            ParamIspProfile_T::EParamValidate_P2Only);
    mpHal3A->setIspProfile(_3A_profile, pDbgInfo);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiControl::
_init()
{
    if (!mpHal3A->addCallbacks(this) )
    {
        MY_LOGE("setCallbacks fail");
    }

    // allocate replace buffer
    mpIspSyncCtrl->addPass1ReplaceBuffer(2,1);
    //
    mbFlashOn = mpHal3A->isNeedFiringFlash();
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiControl::
_uninit()
{
    mpHal3A->removeCallbacks(this);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MultiControl::
_onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MBOOL ret;

    if( mbFlashOn )
        ret = handle_CapFixed(msg, ext1, ext2);
    else
        ret = handle_Update(msg, ext1, ext2);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
void
MultiControl::
doNotifyCb ( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3, MINTPTR _ext4)
{
#define is3aStatus(ext, bit) (ext & (1<<bit))
    //
    if(_msgType == I3ACallBack::eID_NOTIFY_3APROC_FINISH)
    {
        MBOOL keepUpdate = MTRUE;
        MY_LOGD("3APROC_FINISH:0x%08X,0x%08X",
                _ext1,
                _ext2);

        if(mCurMagicNum != _ext1) {
            MY_LOGE("MagicNum:Cur(0x%08X) != Notify(0x%08X)",
                    mCurMagicNum,
                    _ext1);
            return;
        }

        if( getFlag(SENSOR_READY) && mbFlashOn )
            keepUpdate = MFALSE;

        if( is3aStatus(_ext2,I3ACallBack::e3AProcOK) ) {
            mpIspSyncCtrl->send3AUpdateCmd(
                    keepUpdate ? IspSyncControl::UPDATE_CMD_OK : IspSyncControl::UPDATE_CMD_STOP
                    );
        } else {
            mpIspSyncCtrl->send3AUpdateCmd(
                    keepUpdate ? IspSyncControl::UPDATE_CMD_FAIL : IspSyncControl::UPDATE_CMD_STOP
                    );
        }

        if( !mbFlashOn && is3aStatus(_ext2,I3ACallBack::e3APvInitReady) ) {
            Mutex::Autolock _l(mLock);
            if( m3aReadyMagic == MAGIC_NUM_INVALID )
            {
                m3aReadyMagic = mCurMagicNum;
                MY_LOGD("3A ready # 0x%X", m3aReadyMagic);
            }
        } else if( mbFlashOn ) {
            Mutex::Autolock _l(mLock);
            if( m3aReadyMagic == MAGIC_NUM_INVALID )
            {
                m3aReadyMagic = 0; // first-frame can be used
                MY_LOGD("3A ready # 0x%X (flashOn)", m3aReadyMagic);
            }
        }

        return;
    }
#undef is3aStatus
}


/*******************************************************************************
*
********************************************************************************/
void
MultiControl::
doDataCb ( int32_t _msgType, void*   _data, uint32_t _size)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdControl::
_init()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdControl::
_uninit()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ZsdControl::
_onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
    switch(msg)
    {
        case PASS1_START_ISP:
            // do nothing
            break;

        case PASS1_EOF:
            setFlag(DO_CAPTURE, MFALSE);
            if( ext2 != MAGIC_NUM_INVALID ) // wait for sensor dealy
            {
                if(mpHal3A)
                {
                    const MUINT32 magicDeque = ext2;
                    if( mbCapConnected ) {
                        setFlag(DO_CAPTURE, MTRUE);
                        IDbgInfoContainer* pDbgInfo = NULL;
                        if( isDataConnected(CONTROL_DBGINFO) ) {
                            pDbgInfo = IDbgInfoContainer::createInstance();
                        }

                        generateIspTuningP2(
                                SetCap(magicDeque),
                                mIspProfile, MFALSE,
                                pDbgInfo
                                );

                        if( pDbgInfo != NULL ) {
                            Mutex::Autolock _l(mLock);
                            mpDbgInfo[0] = pDbgInfo;
                        }
                        //
                        handleNotify(CONTROL_SHUTTER);
                    }
                }
                break;
            }
            break;

        case PASS1_STOP_ISP:
            // do nothing
            break;
        default:
            break;
    }
    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

