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
#define LOG_TAG "MtkCam/BShotCtrl"
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
#include <utils/Mutex.h>
using namespace android;
//
#include <mtkcam/featureio/aaa_hal_common.h>
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/camnode/IspSyncControl.h>
#include "../inc/BShotCtrlNode.h"
using namespace NSCamNode;
//
#include <vector>
using namespace std;
//
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <mtkcam/featureio/eis_hal.h>
#define EIS_TEST            (1)
//
#define USE_3A

#define MODULE_NAME_CTRL    "BShotCtrl"
//
#define CHECK_OBJECT(x)  { if (x == NULL) { MY_LOGE("Null %s Object", #x); return MFALSE;}}
//
#define BIT_MAIN_JPEG       (1)
#define BIT_THUMBNAIL_JPEG  (2)
//
/*******************************************************************************
*
********************************************************************************/
namespace NSCamShot {
////////////////////////////////////////////////////////////////////////////////

class BSCtrlNodeImpl : public BShotCtrlNode
                     , public I3ACallBack
{
    public:     /*createInstance()*/
        BSCtrlNodeImpl(ShotParam const & rShotParam);
        ~BSCtrlNodeImpl();

        DECLARE_ICAMNODE_INTERFACES();

        virtual MVOID   setShotCount(MUINT32 const count);

        //I3ACallBack
        virtual void doNotifyCb ( int32_t _msgType, int32_t _ext1, int32_t _ext2, int32_t _ext3, MINTPTR _ext4 = 0);

        virtual void doDataCb ( int32_t _msgType, void*   _data, uint32_t _size);

        virtual MBOOL   registerCap3AParam(CaptureParam_T* pCapParams, MUINT32 const length);

        virtual MBOOL   setIspProfile(EIspProfile_T const profile);

    protected:

        MBOOL handleEOF_CapFixed(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque);
        MBOOL handleEOF_CapParams(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque);

        MBOOL   readyToCapture(MUINT32 const magic) {
            Mutex::Autolock _l(mLock);
            return (m3aReadyMagic != MAGIC_NUM_INVALID) && (magic >= m3aReadyMagic);
        }

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
            NONE         = 0x0,
            SENSOR_READY = 0x1,
            DO_CAPTURE   = 0x2,
        };

        mutable Mutex         mLock;
        IHal3A*               mpHal3A;
        IspSyncControl*       mpIspSyncCtrl;
        MUINT32               muShotCount;
        MUINT32               muCurShotCount; //for burst shot

        // status flag
        MUINT32               muFlag;

        // 3A related
        MUINT32               mCurMagicNum;
        MUINT32               m3aReadyMagic;
        MUINT32               mNextCapMagicNum;

        // 3A cap params
        EIspProfile_T         mIspProfile;
        CaptureParam_T*       mpCapParams;
        MUINT32               muCapParamCount;

    protected:
        EisHal*               mpEisHal;

};


/*******************************************************************************
*
********************************************************************************/
BShotCtrlNode*
BShotCtrlNode::
createInstance(ShotParam const & rShotParam)
{
    return new BSCtrlNodeImpl(rShotParam);
}


/*******************************************************************************
*
********************************************************************************/
void
BShotCtrlNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
BShotCtrlNode::
BShotCtrlNode(ShotParam const & rShotParam)
    : ICamNode(MODULE_NAME_CTRL)
    , mShotParam(rShotParam)
{
}


/*******************************************************************************
*
********************************************************************************/
BShotCtrlNode::
~BShotCtrlNode()
{
}


/*******************************************************************************
*
********************************************************************************/
BSCtrlNodeImpl::
BSCtrlNodeImpl(ShotParam const & rShotParam)
    : BShotCtrlNode(rShotParam)
    , mpHal3A(NULL)
    , mpIspSyncCtrl(NULL)
    , muShotCount(0)
    , muCurShotCount(0)
    , muFlag(NONE)
    , mCurMagicNum(0)
    , m3aReadyMagic(MAGIC_NUM_INVALID)
    , mIspProfile(EIspProfile_Capture)
    , mpCapParams(NULL)
    , muCapParamCount(0)
    //
    , mpEisHal(NULL)

{
    // p1 -> p2
    addDataSupport( ENDPOINT_SRC, CONTROL_FULLRAW );
    //addDataSupport( ENDPOINT_SRC, CONTROL_RESIZEDRAW );

    addDataSupport( ENDPOINT_DST, CONTROL_CAP_SRC );
    //addDataSupport( ENDPOINT_DST, CONTROL_PRV_SRC );
    addDataSupport( ENDPOINT_DST, CONTROL_DBGINFO );
    addDataSupport( ENDPOINT_DST, CONTROL_EISDATA );

    // notify
    addNotifySupport( CONTROL_STOP_PASS1 | CONTROL_SHUTTER);
}


/*******************************************************************************
*
********************************************************************************/
BSCtrlNodeImpl::
~BSCtrlNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
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
#ifdef USE_3A
    mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_1, getSensorIdx(), getName());
    if(mpHal3A == NULL)
    {
        MY_LOGE("IHal3A:createInstance fail");
        goto lbExit;
    }
    //
    if (!mpHal3A->addCallbacks(this) )
    {
        MY_LOGE("setCallbacks fail");
        goto lbExit;
    }
#endif
    //
    //
    mpIspSyncCtrl->addPass1ReplaceBuffer(muShotCount > 3 ? (muShotCount-3) : 0, 0);
    //
    ret = MTRUE;
    //
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
onUninit()
{
    #if EIS_TEST
    if(mpEisHal)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(LOG_TAG);
        mpEisHal = NULL;
    }
    #endif
    //
    if(mpHal3A)
    {
        mpHal3A->removeCallbacks(this);
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
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
onStart()
{
    MBOOL ret = MTRUE;
    //
    muCurShotCount = 0;
    //
    if( muShotCount <= 0 ){
        MY_LOGE("wrong shot count %d", muShotCount);
        return MFALSE;
    }
    //
    if( muCapParamCount > 0 ) {
        MY_LOGD("cap param %d, addr 0x%x", muCapParamCount, mpCapParams);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
onStop()
{
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //
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
                MY_LOGD("sce 0x%x, magic %d", sensorScenario, magicNum);
                //
                mpHal3A->enterCaptureProcess();
                mpHal3A->setSensorMode(sensorScenario);
                mNextCapMagicNum = magicNum;
                if( muCapParamCount > 0) {
                    MY_LOGD("update cap param %d", muCurShotCount);
                    mpHal3A->updateCaptureParams(mpCapParams[muCurShotCount]);
                }

                mpHal3A->send3ACtrl(E3ACtrl_SetIsAEMultiCapture, 1, 0);
                mpHal3A->sendCommand(ECmd_CaptureStart);

                MY_LOGD("3A setIspProfile %d, # 0x%x", mIspProfile, magicNum );
                ParamIspProfile_T _3A_profile(
                        mIspProfile,
                        magicNum,
                        MFALSE,
                        ParamIspProfile_T::EParamValidate_All);
                mpHal3A->setIspProfile(_3A_profile);
                mpIspSyncCtrl->send3AUpdateCmd(IspSyncControl::UPDATE_CMD_OK);

                //
                #if EIS_TEST
                //
                if(isDataConnected(CONTROL_EISDATA))
                {
                    mpEisHal = EisHal::CreateInstance(LOG_TAG,getSensorIdx());
                    if(mpEisHal)
                    {
                        mpEisHal->Init();
                        if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                        {
                            EIS_PASS_ENUM eisPass = EIS_PASS_1;
                            EIS_HAL_CONFIG_DATA  eisConfig;
                            MINT32 eisErr;
                            eisConfig.sensorType    = sensorType;
                            eisConfig.configSce     = EIS_SCE_MFBLL;
                            eisConfig.memAlignment  = 2;
                            eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                            if(eisErr != EIS_RETURN_NO_ERROR)
                            {
                                MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                            }
                        }
                    }
                    else
                    {
                        MY_LOGE("mpEisHal is NULL");
                    }
                }
                #endif
            }
            break;

        case PASS1_EOF:
            if( muCapParamCount > 0 && mpCapParams )
                handleEOF_CapParams(msg, ext1, ext2);
            else
                handleEOF_CapFixed(msg, ext1, ext2);

            #if EIS_TEST
            if(mpEisHal)
            {
                // do algo.
                mpEisHal->DoEis(EIS_PASS_1);

                if( getFlag(DO_CAPTURE) )
                {
                    #if 1
                    // gmv
                    MUINT32 tarWidth = 0, tarHeight = 0;
                    IspSyncControl::EIS_CROP_INFO eisCrop;
                    eisCrop.enable = MFALSE;
                    mpEisHal->GetEisResult(
                            eisCrop.xInt,
                            eisCrop.xFlt,
                            eisCrop.yInt,
                            eisCrop.yFlt,
                            tarWidth,
                            tarHeight);
                    //@todo return eisCrop
                    #endif

                    #if 1
                    // hw statistics
                    EIS_STATISTIC_STRUCT* pEisStat = new EIS_STATISTIC_STRUCT;
                    mpEisHal->GetEisStatistic(pEisStat);
                    handlePostBuffer(CONTROL_EISDATA, (MUINTPTR)pEisStat, 0);
                    #endif
                }
            }
            #endif
            break;

        case PASS1_CONFIG_FRAME:
            #if EIS_TEST
            if(mpEisHal)
            {
                if(mpEisHal->GetEisSupportInfo(getSensorIdx()))
                {
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

                    EIS_PASS_ENUM eisPass = EIS_PASS_1;
                    EIS_HAL_CONFIG_DATA  eisConfig;
                    MINT32 eisErr;
                    eisConfig.sensorType    = sensorType;
                    eisConfig.configSce     = EIS_SCE_MFBLL;
                    eisConfig.memAlignment  = 2;
                    eisErr = mpEisHal ->ConfigEis(eisPass, eisConfig);
                    if(eisErr != EIS_RETURN_NO_ERROR)
                    {
                        MY_LOGE("mpEisHal ->configEIS err(%d)",eisErr);
                    }
                }
            }
            #endif
            break;

        case PASS1_STOP_ISP:
            if(mpEisHal) {
                mpEisHal->SendCommand(EIS_CMD_SET_STATE,EIS_SW_STATE_UNINIT);
            }
            if(mpHal3A) {
                mpHal3A->send3ACtrl(E3ACtrl_SetIsAEMultiCapture, 0, 0);
                mpHal3A->exitCaptureProcess();
            }
            //
            if(mpEisHal) {
                mpEisHal->SendCommand(EIS_CMD_CHECK_STATE,EIS_SW_STATE_UNINIT_CHECK);
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
BSCtrlNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MUINT32  dstdata = 0;
    MBOOL    doPost  = MFALSE;
    //
    switch(data)
    {
        case CONTROL_FULLRAW:
            dstdata = CONTROL_CAP_SRC;
            doPost  = getFlag(DO_CAPTURE);
            break;
        default:
            MY_LOGE("not support yet: %d", data);
            break;
    }
    //
    if( doPost )
    {
        handlePostBuffer(dstdata, buf);
    }
    else
    {
        handleReturnBuffer(data, buf);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGV("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    switch(data)
    {
        case CONTROL_CAP_SRC:
            handleReturnBuffer(CONTROL_FULLRAW, buf, ext);
            break;
        case CONTROL_DBGINFO:
            if( buf != 0 )
            {
                IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(buf);
                if( pDbgInfo )
                    pDbgInfo->destroyInstance();
            }
            break;
        case CONTROL_EISDATA:
            if( buf != 0 )
            {
                EIS_STATISTIC_STRUCT* pEisStat = reinterpret_cast<EIS_STATISTIC_STRUCT*>(buf);
                delete pEisStat;
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
BSCtrlNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
}


/*******************************************************************************
*
********************************************************************************/
void
BSCtrlNodeImpl::
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

        if( getFlag(SENSOR_READY) && muCapParamCount == 0)
        {
            MY_LOGD("no more update");
            keepUpdate = MFALSE;
        }

        if( is3aStatus(_ext2,I3ACallBack::e3AProcOK) ) {
            mpIspSyncCtrl->send3AUpdateCmd(
                    keepUpdate ? IspSyncControl::UPDATE_CMD_OK : IspSyncControl::UPDATE_CMD_STOP
                    );
        } else {
            mpIspSyncCtrl->send3AUpdateCmd(
                    keepUpdate ? IspSyncControl::UPDATE_CMD_FAIL : IspSyncControl::UPDATE_CMD_STOP
                    );
        }

        {
            Mutex::Autolock _l(mLock);
            if( m3aReadyMagic == MAGIC_NUM_INVALID )
            {
                m3aReadyMagic = _ext1;
                MY_LOGD("3A ready # 0x%X", m3aReadyMagic);
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
BSCtrlNodeImpl::
doDataCb ( int32_t _msgType, void*   _data, uint32_t _size)
{
}


/*******************************************************************************
*
********************************************************************************/
MVOID
BSCtrlNodeImpl::
setShotCount(MUINT32 const count)
{
    MY_LOGD("set shotcount %d", count);
    muShotCount = count;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
registerCap3AParam(CaptureParam_T* pCapParams, MUINT32 const length)
{
    mpCapParams = pCapParams;
    muCapParamCount = length;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
handleEOF_CapFixed(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque)
{
    if( mpHal3A == NULL )
        return MTRUE;

    setFlag(DO_CAPTURE, MFALSE);
    // buf is ok
    if( magicDeque != MAGIC_NUM_INVALID ) {
        setFlag(SENSOR_READY, MTRUE);
        if( readyToCapture(magicDeque) ) {
            IDbgInfoContainer* pDbgInfo = NULL;
            setFlag(DO_CAPTURE, MTRUE);

            if( muCurShotCount == 0 ) {
                mpHal3A->sendCommand(ECmd_CaptureEnd);

                handleNotify(CONTROL_SHUTTER);
            }

            if( isDataConnected(CONTROL_DBGINFO) ) {
                pDbgInfo = IDbgInfoContainer::createInstance();
            }

            MY_LOGD("count %d: 3A setIspProfile %d, # 0x%x",
                    muCurShotCount,
                    mIspProfile,
                    SetCap(magicDeque) );
            ParamIspProfile_T _3A_profile(
                    mIspProfile,
                    SetCap(magicDeque),
                    MFALSE,
                    ParamIspProfile_T::EParamValidate_P2Only);
            mpHal3A->setIspProfile(_3A_profile, pDbgInfo);

            if( pDbgInfo != NULL )
                handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);

            MY_LOGD("curshotcount %d", muCurShotCount);
            muCurShotCount++;
            if( muCurShotCount == muShotCount ) {
                MY_LOGD("stop p1");
                // stop pass1
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
                return MTRUE;
            }
        }
    }

    if( newMagicNum != MAGIC_NUM_INVALID ) {
        // do 3A update
        mCurMagicNum = newMagicNum;
        ParamIspProfile_T _3A_profile(
                mIspProfile,
                mCurMagicNum,
                MFALSE,
                ParamIspProfile_T::EParamValidate_All);
        mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
handleEOF_CapParams(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque)
{
    if( mpHal3A == NULL )
        return MFALSE;

    setFlag(DO_CAPTURE, MFALSE);
    // buf is ok
    if( magicDeque != MAGIC_NUM_INVALID ) {
        setFlag(SENSOR_READY, MTRUE);
        if( readyToCapture(magicDeque) ) {
            IDbgInfoContainer* pDbgInfo = NULL;
            setFlag(DO_CAPTURE, MTRUE);

            // calculate statistics
            mpHal3A->sendCommand(ECmd_CaptureEnd);

            if( muCurShotCount == 0 ) {
                handleNotify(CONTROL_SHUTTER);
            }

            if( isDataConnected(CONTROL_DBGINFO) ) {
                pDbgInfo = IDbgInfoContainer::createInstance();
            }

            MY_LOGD("current count %d: 3A setIspProfile %d, # 0x%x",
                    muCurShotCount,
                    mIspProfile,
                    SetCap(magicDeque) );
            ParamIspProfile_T _3A_profile(
                    mIspProfile,
                    SetCap(magicDeque),
                    MFALSE,
                    ParamIspProfile_T::EParamValidate_P2Only);
            mpHal3A->setIspProfile(_3A_profile, pDbgInfo);

            if( pDbgInfo != NULL )
                handlePostBuffer(CONTROL_DBGINFO, (MUINTPTR)pDbgInfo, 0);

            muCurShotCount++;
            if( muCurShotCount == muShotCount )
            {
                MY_LOGD("stop p1");
                // stop pass1
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
                return MTRUE;
            }
            else
            {
                // next capture param setting
                if( muCurShotCount < muShotCount )
                {
                    // reset ready #
                    m3aReadyMagic = MAGIC_NUM_INVALID;

                    MY_LOGD("update cap param %d, # %d", muCurShotCount, newMagicNum);
                    mpHal3A->updateCaptureParams(mpCapParams[muCurShotCount]);
                    mpHal3A->sendCommand(ECmd_CaptureStart);
                }
            }
        }
    }

    if( newMagicNum != MAGIC_NUM_INVALID ) {
        // do 3A update
        mCurMagicNum = newMagicNum;
        ParamIspProfile_T _3A_profile(
                mIspProfile,
                mCurMagicNum,
                MFALSE,
                ParamIspProfile_T::EParamValidate_All);
        mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
BSCtrlNodeImpl::
setIspProfile(EIspProfile_T const profile)
{
    mIspProfile = profile;
    return MTRUE;
}



////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamShot

