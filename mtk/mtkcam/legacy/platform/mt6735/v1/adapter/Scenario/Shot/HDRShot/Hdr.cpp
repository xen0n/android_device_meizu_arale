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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "MtkCam/HDRShot"

#include "MyHdr.h"
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.

//
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/exif/IBaseCamExif.h>
#include <mtkcam/exif/CamExif.h>
//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "Hdr.h"
//
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define HDR_TRACE_CALL()                      ATRACE_CALL()
#define HDR_TRACE_NAME(name)                  ATRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define HDR_TRACE_END()                       ATRACE_END()
#else
#define HDR_TRACE_CALL()
#define HDR_TRACE_NAME(name)
#define HDR_TRACE_BEGIN(name)
#define HDR_TRACE_END()
#endif
using namespace android;
using namespace NSShot;

extern "C"
sp<IShot>
createInstance_HdrShot(
    char const*const    pszShotName,
    uint32_t const      u4ShotMode,
    int32_t const       i4OpenId
)
{
    sp<IShot>       pShot = NULL;
    sp<HdrShot>  pImpShot = NULL;
    //
    //  (1.1) new Implementator.
    pImpShot = new HdrShot(pszShotName, u4ShotMode, i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new HdrShot", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] onCreate()", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] new IShot", __FUNCTION__);
        goto lbExit;
    }
    //
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}


/******************************************************************************
 *  This function is invoked when this object is firstly created.
 *  All resources can be allocated here.
 ******************************************************************************/
bool
HdrShot::
onCreate()
{
    FUNCTION_LOG_START;
    bool ret = true;

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *  This function is invoked when this object is ready to destryoed in the
 *  destructor. All resources must be released before this returns.
 ******************************************************************************/
void
HdrShot::
onDestroy()
{
    FUNCTION_LOG_START;
    uninit();
}


/*******************************************************************************
*
*******************************************************************************/
HdrShot::
HdrShot(char const*const pszShotName, uint32_t const u4ShotMode, int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
    ////    Resolutions.
    , mRaw_Width(0)
    , mRaw_Height(0)
    , mu4W_yuv(0)
    , mu4H_yuv(0)
    , mu4W_small(0)
    , mu4H_small(0)
    , mu4W_se(0)
    , mu4H_se(0)
    , mu4W_dsmap(0)
    , mu4H_dsmap(0)
    , mPostviewWidth(800)
    , mPostviewHeight(600)
    , mPostviewFormat(eImgFmt_YV12)
    , mRotPicWidth(0)
    , mRotPicHeight(0)
    , mRotThuWidth(0)
    , mRotThuHeight(0)

    , mErrorFlag(0)
    //
    , mMainThread(0)
    , mMemoryAllocateThread(0)
    //
    , mTrigger_alloc_working(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTrigger_alloc_bmap1(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_Capbuf(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_pass2_first(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_pass2_others(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_jpeg_full(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_jpeg_thumbnail(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_working(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_se(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap0(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap1(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_bmap2(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_blending(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_postview(PTHREAD_MUTEX_INITIALIZER_LOCK)

    // pipes
    , mpHdrHal(NULL)
    //, mpCamExif({NULL})
    // Buffers
    , mpIMemDrv(NULL)
    , mpIImageBufAllocator(NULL)
    , mpCapBufMgr(NULL)
    //, std::vector<ImageBufferMap> mvImgBufMap;
    , mpHeap(NULL)
    , mTotalBufferSize(0)
    , mTotalKernelBufferSize(0)
    , mTotalUserBufferSize(0)
    //
    //, mpSourceImgBuf({NULL})
    //, mpSmallImgBuf({NULL})
    //, mpSEImgBuf({NULL})
    //, mWeightingBuf({NULL})
    //, mpBlurredWeightMapBuf({NULL})
    //, mpDownSizedWeightMapBuf({NULL})
    , mBlendingBuf(NULL)
    , mpPostviewImgBuf(NULL)
    //
    , mpHdrWorkingBuf(NULL)
    , mpMavWorkingBuf(NULL)
    //
    , mNormalJpegBuf(NULL)
    , mNormalThumbnailJpegBuf(NULL)
    , mHdrJpegBuf(NULL)
    , mHdrThumbnailJpegBuf(NULL)

    //
    //, HDR_PIPE_SET_BMAP_INFO mHdrSetBmapInfo;
    , OriWeight(NULL)
    , BlurredWeight(NULL)

    //
    //, mu4RunningNumber(0)
    , mu4OutputFrameNum(0)
    //, mu4FinalGainDiff[2]
    , mu4TargetTone(0)
    //, HDR_PIPE_HDR_RESULT_STRUCT mrHdrCroppedResult;
    , mfgIsForceBreak(MFALSE)
    //
    , mHdrState(HDR_STATE_INIT)
    //
    , mShutterCBDone(MFALSE)
    , mRawCBDone(MFALSE)
    , mJpegCBDone(MFALSE)
    , mfgIsSkipthumb(MFALSE)
    //
    , mCaptueIndex(0)
    , mSensorType(0)
    //
    , mNrtype(0)

    // for development
    , mTestMode(0)
    , mDebugMode(0)
    , mPrivateData(NULL)
    , mPrivateDataSize(0)
{
    mHDRShotMode = u4ShotMode;
    for(MUINT32 i=0; i<eMaxOutputFrameNum; i++) {
        //mpCamExif[i] = new CamExif;
        mpSourceRawImgBuf[i] = NULL;
        mpSourceImgBuf[i] = NULL;
        mpSmallImgBuf[i] = NULL;
        mpSEImgBuf[i] = NULL;
        mWeightingBuf[i] = NULL;
        mpBlurredWeightMapBuf[i] = NULL;
        mpDownSizedWeightMapBuf[i] = NULL;
    }

    //setShotParam() default values
    ShotParam param;
    param.mi4PictureWidth = 3264;
    param.mi4PictureHeight = 2448;
    param.mi4PostviewWidth = 800;
    param.mi4PostviewHeight = 600;
    setShotParam(&param, sizeof(ShotParam));

    mu4OutputFrameNum = 3;
    mu4FinalGainDiff[0]    = 2048;
    mu4FinalGainDiff[1]    = 512;
    mu4TargetTone        = 150;
}


/******************************************************************************
 *
 ******************************************************************************/
HdrShot::
~HdrShot()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HdrShot::
setCapBufMgr(MVOID* pCapBufMgr)
{
    mpCapBufMgr = (CapBufMgr*)pCapBufMgr;
    return MTRUE;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
sendCommand(
    uint32_t const  cmd,
    MUINTPTR const  arg1,
    uint32_t const  arg2,
    uint32_t const  arg3
)
{
    FUNCTION_LOG_START;
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2, arg3);
    }

    //
    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
onCmd_reset()
{
    FUNCTION_LOG_START;
    bool ret = true;

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
HdrShot::
onCmd_capture()
{
    FUNCTION_LOG_START;
    bool ret = true;
    SetThreadProp(SCHED_OTHER, -20);
    PrintThreadProp(__FUNCTION__);
    pthread_create(&mMainThread, NULL, HdrShot::onCmd_captureTask, this);
    MUINT32    threadRet = 0;
    pthread_join(mMainThread, (void**)&threadRet);
    mMainThread = 0;
    ret = threadRet;
    FUNCTION_LOG_END;
    return ret;
}


MVOID*
HdrShot::
onCmd_captureTask(MVOID *arg)
{
    FUNCTION_LOG_START;
    MUINTPTR ret = true;

    ::prctl(PR_SET_NAME,"HDR_MAIN", 0, 0, 0);
    PrintThreadProp(__FUNCTION__);
    SetThreadProp(SCHED_OTHER, -20);

    HdrShot *self = (HdrShot*)arg;

    if(!self) {
        MY_ERR("arg is null");
        ret = false;
        goto lbExit;
    }
    ret = self->mainflow();
lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


bool
HdrShot::
mainflow()
{
    FUNCTION_LOG_START;
    bool ret = true;
    CPTLog(Event_HdrShot, CPTFlagStart);

#if (HDR_PROFILE_CAPTURE2)
    MyDbgTimer DbgTmr("capture");
#endif

    //

    if(mHDRShotMode == eShotMode_ZsdHdrShot){
		mpCapBufMgr = ImpShot::mpCapBufMgr;
    }

    ret = ret
        && init()
        && configureForSingleRun()
        && EVBracketCapture()
        && ImageRegistratoin()
        && WeightingMapGeneration()
        && Blending()
        ;

    //error handler
    if(!mTestMode)
    {

        if(!mShutterCBDone) {
            MY_ERR("send fake onCB_Shutter");
            mpShotCallback->onCB_Shutter(true,0);
        }

        #if 0   //95 hdr didn't use raw callback to save raw image.
        if(!mRawCBDone) {
            MY_ERR("send fake onCB_RawImage");
            MUINT32    u4ExifHeaderSize = 512;
            MUINT32    u4JpegSize = 512;
            MUINT8    puImageBuffer[1024];

            mpShotCallback->onCB_RawImage(0
                                        , u4ExifHeaderSize+u4JpegSize
                                        , reinterpret_cast<uint8_t const*>(puImageBuffer)
                                        );
        }
        #endif

        if(!mJpegCBDone) {
            MY_ERR("send fake onCB_CompressedImage");
            MUINT32    u4ExifHeaderSize = 512;
            MUINT8    puExifHeaderBuf[512];
            MUINT32    u4JpegSize = 512;
            MUINT8     puJpegBuf[512];
            MUINT32    u4Index = 0;
            MBOOL    bFinal = MTRUE;

            mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize,
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,    //header size
                                         puExifHeaderBuf,    //header buf
                                         u4Index,            //callback index
                                         bFinal                //final image
                                         );
        }
    }


    //@TODO list
    //#cancel
    //#multi-thread
    //#fail
    //#document
    //#full frame
    //#speed up do_DownScaleWeightMap()

lbExit:
    //  ()  HDR finished, clear HDR setting.
    do_HdrSettingClear();
    // Don't know exact time of lbExit in HDR flow, so release all again
    // (there is protection in each release function).
    releaseSourceRawImgBuf();
    releaseSourceImgBuf();
    releaseSmallImgBuf();
    releaseSEImgBuf();
    releaseHdrWorkingBuf();
    releaseOriWeightMapBuf();
    releaseDownSizedWeightMapBuf();
    releaseBlurredWeightMapBuf();
    releasePostviewImgBuf();

    releaseNormalJpegBuf();
    releaseNormalThumbnailJpegBuf();
    releaseHdrJpegBuf();
    releaseHdrThumbnailJpegBuf();

    releaseBlendingBuf();

    #if (HDR_PROFILE_CAPTURE2)
    DbgTmr.print("HdrProfiling2:: HDRFinish Time");
    #endif

    CPTLog(Event_HdrShot, CPTFlagEnd);

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
void
HdrShot::
onCmd_cancel()
{
    FUNCTION_LOG_START;
    //mfgIsForceBreak = MTRUE;
    // never cancel since hdr 8M only take 1.4s
    FUNCTION_LOG_END_MUM;
}


/*******************************************************************************
*
*******************************************************************************/
bool
HdrShot::
setShotParam(void const* pParam, size_t const size)
{
    FUNCTION_LOG_START;
    bool ret = true;

    if(!ImpShot::setShotParam(pParam, size)) {
        MY_ERR("[HDR] HdrShot->setShotParam() fail.");
        ret = false;
    }

    FUNCTION_LOG_END;
    return ret;
}

