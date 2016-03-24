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
#include <unistd.h>
#include <utils/threads.h>
#include <sys/prctl.h>  // For prctl()/PR_SET_NAME.
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <fcntl.h>
#define MTK_LOG_ENABLE 1
#define LOG_TAG "MtkCam/MfllShot"
#include "../inc/MfllCommon.h"
//
#include <mtkcam/iopipe/SImager/ISImager.h>

using namespace android;

#if TEST_ALGORITHM
#include <core/featureio/pipe/mfll/mfll_hal.h>
#endif  //TEST_ALGORITHM

#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/hwutils/HwMisc.h>

#include <mtkcam/camshot/ICamShot.h>
#include <mtkcam/camshot/IBurstShot.h>
#include <mtkcam/camshot/CapBufMgr.h>

using namespace NSCamNode;
//
#include <mtkcam/camshot/_callbacks.h>
#include <mtkcam/camshot/_params.h>
#include <mtkcam/camnode/IspSyncControl.h>
//
#include "../inc/CamShotImp.h"
#include "../inc/MfllShot.h"
#include "../inc/BestShotSelection.h"

// custom tuning
#include <camera_custom_nvram.h>
#include <nvbuf_util.h>

// exif
#include <debug_exif/dbg_id_param.h>
#include <debug_exif/cam/dbg_cam_param.h>
#include <mtkcam/drv/isp_drv.h>
#include <utils/Debug.h>

// nr
#include <mtkcam/featureio/capturenr.h>


namespace NSCamShot {

/******************************************************************************
 *  Porting
 *******************************************************************************/
#define MTRUE       1
#define MFALSE      0

#define DONT_MEMCPY 1


#define IMEM_OK     0

#define MFLL_HAL_TAG           "{MfllShot} "
#define MY_LOGI(fmt, arg...)    do {ALOGI(MFLL_HAL_TAG fmt, ##arg);printf(MFLL_HAL_TAG fmt "\n", ##arg);} while (0)
#define MY_LOGD(fmt, arg...)    do {ALOGD(MFLL_HAL_TAG fmt, ##arg);printf(MFLL_HAL_TAG fmt "\n", ##arg);} while (0)
#define MY_LOGE(fmt, arg...)    do {ALOGE("error" MFLL_HAL_TAG fmt, ##arg);printf(MFLL_HAL_TAG fmt "\n", ##arg);} while (0)
#define FUNCTION_LOG_START      MY_LOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        do {if(!ret) MY_LOGE("[%s] fail", __FUNCTION__); MY_LOGD("[%s] - X. ret=%d", __FUNCTION__, ret);} while(0)
#define FUNCTION_LOG_END_MUM    MY_LOGD("[%s] - X.", __FUNCTION__)

//
#define ANNOUNCE(mutex)         announce(mutex, #mutex)
#define WAIT(mutex)             wait(mutex, #mutex)

#define PTHREAD_MUTEX_INITIALIZER_LOCK  {1}
#define PTHREAD_MUTEX_INITIALIZER_UNLOCK {0}
#define msleep(ms)              usleep((ms)*1000)

#define CHECK_OBJECT(x)         do{ if (x == NULL) { MY_LOGE("%s is Null Object", #x); ret = MFALSE; goto lbExit;}}while(0)
#define CHECK_RET(fmt, arg...)  do{ if( !ret ){ MY_LOGE(fmt, ##arg); ret = MFALSE; goto lbExit; }}while(0)

#define makeExifHeader(...)

#define ais_exp_th0 reserved[0]  //workaround for nvram struct frozen
#define ais_iso_th0 reserved[1]  //workaround for nvram struct frozen


/******************************************************************************
 *  MFLL NODE
 *******************************************************************************/
//systrace
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define MFLL_TRACE_CALL()                      ATRACE_CALL()
#define MFLL_TRACE_NAME(name)                  ATRACE_NAME(name)
#define MFLL_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define MFLL_TRACE_END()                       ATRACE_END()
#else
#define MFLL_TRACE_CALL()
#define MFLL_TRACE_NAME(name)
#define MFLL_TRACE_BEGIN(name)
#define MFLL_TRACE_END()
#endif

using namespace android;
using namespace NSCam::NSIoPipe::NSSImager;
using namespace NSCam::NSIoPipe::NSPostProc;
using namespace NS3A;

/******************************************************************************
 * enum
 *******************************************************************************/
enum MFLL_DUMP {
    MFLL_DUMP_RAW       = 0x01,
    MFLL_DUMP_YUV       = 0x02,
    MFLL_DUMP_MFB       = 0x04,
    MFLL_DUMP_MIXING    = 0x08,
    MFLL_DUMP_JPEG      = 0x10,
    MFLL_DUMP_POSTVIEW  = 0x20,
    MFLL_DUMP_EXIF      = 0x40,
    MFLL_DUMP_15RAW     = 0x80,
};


/******************************************************************************
 * struct
 *******************************************************************************/
struct EIS_DATA_STRUCT
{
    MINT32 eis_version; /* check with EIS_DATA_VER, should be the same or return err code */
    MINT32 eis_data1[2 * EIS_MAX_WIN_NUM];
    MINT32 eis_data2[2 * EIS_MAX_WIN_NUM];
    MINT32 eis_data3[2];
};


/******************************************************************************
 * static utility functions
 *******************************************************************************/
static
MINT32 Transform2Degree(MUINT32 transform)
{
    transform &= (eTransform_ROT_90 | eTransform_ROT_180 | eTransform_ROT_270);
    switch(transform) {
        case 0: return 0;
        case eTransform_ROT_90: return 90;
        case eTransform_ROT_180: return 180;
        case eTransform_ROT_270: return 270;
        default:
            MY_LOGE("unsupported transform %x", transform);
            return 0;
    }
}

static
MUINT32 Degree2Transform(MINT32 degree)
{
    while(degree < 0) {
        degree += 360;
    }
    while(degree > 360) {
        degree -= 360;
    }
    switch(degree) {
        case 0: return 0;
        case 90: return eTransform_ROT_90;
        case 180: return eTransform_ROT_180;
        case 270: return eTransform_ROT_270;
        default:
            MY_LOGE("unsupported degree %d", degree);
            return 0;
    }
}


static
MBOOL
allocImageBuffer(IImageBufferHeap **pHeap, sp<IImageBuffer>* pBuf, MUINT32 width, MUINT32 height, int const format)
{
    using namespace NSCam::Utils::Format;

    MBOOL ret = MTRUE;
    FUNCTION_LOG_START;

    MSize imgSize(width, height);
    MUINT32 bufStridesInBytes[3] = {0};
    for (int i = 0; i < (int)queryPlaneCount(format); i++) {
        bufStridesInBytes[i] = (queryPlaneWidthInPixels(format,i, imgSize.w) * queryPlaneBitsPerPixel(format, i) + 7) / 8;
    }

    MY_LOGD("allocImageBuffer - width=%d, height=%d, format=%d", width, height, format);


    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(format
                                            , imgSize
                                            , bufStridesInBytes
                                            , bufBoundaryInBytes
                                            , Utils::Format::queryPlaneCount(format)
                                            );



    *pHeap = IIonImageBufferHeap::create(LOG_TAG, imgParam);
    CHECK_OBJECT(*pHeap);
    *pBuf = (*pHeap)->createImageBuffer();
    CHECK_OBJECT(*pBuf);

    // get VA & PA
    ret = ret && (*pBuf)->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);


    // flush
    (*pBuf)->syncCache(eCACHECTRL_INVALID);  //hw->cpu


     MY_LOGD("allocImageBuffer - bufSize=%zu, imgSize=[%d,%d], plane=%zu, stride=[%zu,%zu,%zu]], boundary=[%zu,%zu,%zu]"
            , imgParam.bufSize
            , imgParam.imgSize.w, imgParam.imgSize.h
            , Utils::Format::queryPlaneCount(format)
            , imgParam.bufStridesInBytes[0]
            , imgParam.bufStridesInBytes[1]
            , imgParam.bufStridesInBytes[2]
            , imgParam.bufBoundaryInBytes[0]
            , imgParam.bufBoundaryInBytes[1]
            , imgParam.bufBoundaryInBytes[2]
            );

    MY_LOGD("<YUV> ImgBitsPerPixel:%zu BufSizeInBytes:%zu %d", (*pBuf)->getImgBitsPerPixel(), (*pBuf)->getBufSizeInBytes(0), imgSize.size());

lbExit:
    FUNCTION_LOG_END;
    return  ret;
}


static
MVOID
freeImageBuffer(sp<IImageBuffer> *pImageBuffer)
{
    /* avoid release again */
    if ((*pImageBuffer) == NULL) {
        return;
    }

    FUNCTION_LOG_START;

    if( !pImageBuffer->get()->unlockBuf(LOG_TAG) )
    {
        MY_LOGE("unlock Buffer failed\n");
    }
    *pImageBuffer = NULL;

    FUNCTION_LOG_END_MUM;
}


static
MBOOL
allocBlobBuffer(IImageBufferHeap **pHeap, sp<IImageBuffer> *pBuf, MUINT32 size)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    //
    IImageBufferAllocator::ImgParam imgParam(size, 0);

    *pHeap = IIonImageBufferHeap::create(LOG_TAG, imgParam);
    CHECK_OBJECT(*pHeap);
    *pBuf = (*pHeap)->createImageBuffer();
    CHECK_OBJECT(pBuf->get());
    //
    ret = ret && (*pBuf)->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    CHECK_RET("lock fail");

    // flush
    (*pBuf)->syncCache(eCACHECTRL_INVALID);  //hw->cpu

lbExit:
    FUNCTION_LOG_END;
    return  ret;
}


static
MBOOL
allocJpegBuffer(IImageBufferHeap **pHeap, sp<IImageBuffer> *pBuf, MUINT32 w, MUINT32 h)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MINT32 bufBoundaryInBytes = 0;
    IImageBufferAllocator::ImgParam imgParam(MSize(w, h)
                                            , w * h * 6 / 5
                                            , bufBoundaryInBytes
                                            );

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    IImageBuffer *jpegBuffer = NULL;
    CHECK_OBJECT(allocator);
    jpegBuffer = allocator->alloc_ion(LOG_TAG, imgParam);
    CHECK_OBJECT(jpegBuffer);
    *pBuf = jpegBuffer;
    jpegBuffer->decStrong(jpegBuffer);

    ret = ret && (*pBuf)->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    CHECK_RET("lock fail");

    // flush
    (*pBuf)->syncCache(eCACHECTRL_INVALID);  //hw->cpu

lbExit:
    FUNCTION_LOG_END;
    return  MTRUE;
}


/******************************************************************************
 * CamShotImp
 *******************************************************************************/
MfllShot::
MfllShot(EShotMode const eShotMode, char const*const szCamShotName, MUINT32 mfb)
    : CamShotImp(eShotMode, szCamShotName)
    ////
    , mMfbMode(mfb)
    , mbDoShutterCb(MTRUE)
    , mpPrvBufHandler(NULL)
    , mpCapBufMgr(NULL)

    // copy from singleshot
    , mRotPicWidth(0)
    , mRotPicHeight(0)
    , mRotThuWidth(0)
    , mRotThuHeight(0)

    ////
    , mRegistedBufferType(ECamShot_BUF_TYPE_NONE)

    ////
    , mpMfllHal(NULL)
    , mpBurstShot(NULL)

    ////
    , mfgIsForceBreak(MFALSE)
    , mShutterCBDone(MFALSE)
    , mRawCBDone(MFALSE)
    , mJpegCBDone(MFALSE)
    , mPass1Index(0)
    , mPass2Index(0)
    , mEisId(0)

    ////    legency
    , mpIMemDrv(NULL)
    , mTotalBufferSize(0)
    , mRaw_Width(0)
    , mRaw_Height(0)
    , mMemoryCapBufRaw({0})
    , mMemoryRaw({{0}})             //array
    , mMemoryRawInCaptureOrder({{0}})
    , mMemoryRawB({0})
    , mMemoryRawR({{0}})             //array

    , mYuv_Width(0)
    , mYuv_Height(0)
    , mMemoryYuvB({0})

    , mYuvQ_Width(0)
    , mYuvQ_Height(0)
    , mMemoryQYuvB({0})
    , mMemoryQYuvR({{0}})            //array

    , mMemoryLocalMotion({0})
    , mMemoryYuv({0})
    , mMemoryWeightingMap({0})
    , mMemoryFinalWeightingMapIndex(0)

    , mMemoryYuvMixing({0})

    // ptr images
    , mPtrYuvBase({0})
    , mPtrYuvGolden({0})
    , mPtrYuvBlended({0})

    , mMemoryJpeg({0})
    , mFullJpegRealSize(0)

    , mThunmbnail_Width(0)
    , mThunmbnail_Height(0)
    , mMemoryJpeg_Thumbnail({0})
    , mThumbnailJpegRealSize(0)

    , mPostview_Width(0)
    , mPostview_Height(0)
    , mMemoryPostview({0})

    , mPrivateData(NULL)
    , mPrivateDataSize(0)

    ////    thread
    , mThreadMemory(0)
    , mThreadCapture(0)
    , mThreadCapturePostProc(0)
    , mThreadRawToYuy2(0)
    , mThreadYuy2ToYv16(0)
    , mThreadMfll(0)
    , mThreadMemc(0)
    , mThreadBlending(0)
    , mThreadPostview(0)
    , mThreadFull(0)
    , mThreadThumbnail(0)

    ////    mutex
    // memory
    , mTriggerAllocateMemory(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_Pass1(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_QYuvB(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_YuvB(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_Postview(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mMemoryReady_Jpeg(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTriggerReleaseMemory(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mReleaseMemoryDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    // capture
    , mTriggerCapture(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mCaptureDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mGraphDone(PTHREAD_MUTEX_INITIALIZER_LOCK)    //internal
    // postview
    , mTriggerPostview(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mPostviewDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    // full
    , mTriggerFullJpeg(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mFullJpegDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    // thumbnail
    , mTriggerThumbnailJpeg(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mThumbnailJpegDone(PTHREAD_MUTEX_INITIALIZER_LOCK)

    // tuning
    , mSingleFrame(MFALSE)
    , mReferenceFrameLength(3)
    , mTuningData()

    // debug
    , mIsSetDumpFileName(MFALSE)
    , mDump(0)
    //, mDumpFilename()
    , mSdcardIndex(0)

    // eis
    , mGmv({{0}})
    , mAccGmvX(0)
    , mAccGmvY(0)
    , mRawToQyuvFactor(0.5)

    // exif
    , mDebugInfo(NULL)

    // nr
    , muNRType(0)

    // skip mfb
    , mSkipMfb(MFALSE)

    // bss
    , mEnableBss(MTRUE)
    // MRP usage
    , mReduceMemoryUsage((enum eMrpMode)MFLL_REDUCE_MEMORY_USAGE)
    , mTriggerAllocateYuvBase(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTriggerAllocateMfbRelatedBuffers(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTriggerAllocateMemc(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTriggerAllocateYuvMixing(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mTriggerAllocateJpeg(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mAllocateYuvBaseDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mAllocateMfbRelatedBuffersDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mAllocateMemcDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mAllocateYuvMixingDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
    , mAllocateJpegDone(PTHREAD_MUTEX_INITIALIZER_LOCK)
{
    FUNCTION_LOG_START;

    ////    mutex
    for(MUINT32 ref=0; ref<MAX_REFERENCE_FRAMES; ref++) {
        // memory
        mMemoryReady_QYuvR[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemoryReady_CapBuf[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemoryReady_CapBuf[MAX_REFERENCE_FRAMES] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemoryReady_Memc[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemoryReady_Mfb[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        // capture
        mCapP2Ready[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mCapP2Ready[MAX_REFERENCE_FRAMES] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mYuy2ToYv16Done = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mQrReady[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        // memc
        mTriggerMemc[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mMemcDone[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        // blending
        mTriggerBlending[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
        mBlendingDone[ref] = PTHREAD_MUTEX_INITIALIZER_LOCK;
    }

    memset(&mDbgMfllInfo, 0, sizeof(mDbgMfllInfo));
    memset(&mDbgAisInfo, 0, sizeof(mDbgAisInfo));
    memset(&mDbgEisInfo, 0, sizeof(mDbgEisInfo));

    FUNCTION_LOG_END_MUM;
}


MBOOL
MfllShot::
init()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    // settings for dump buffer
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    //
    property_get("mediatek.mfll.dump.all", value, "0");
    MBOOL dumpAll = atoi(value) || TEST_DUMP;
    //
    property_get("mediatek.mfll.dump.raw", value, "0");
    MBOOL dumpRaw = atoi(value) || dumpAll || TEST_DUMP_RAW;
    property_get("mediatek.mfll.dump.yuv", value, "0");
    MBOOL dumpYuv = atoi(value) || dumpAll || TEST_DUMP_YUV;
    property_get("mediatek.mfll.dump.mfb", value, "0");
    MBOOL dumpMfb = atoi(value) || dumpAll || TEST_DUMP_MFB;
    property_get("mediatek.mfll.dump.mixer", value, "0");
    MBOOL dumpMixer = atoi(value) || dumpAll || TEST_DUMP_MIXER;
    property_get("mediatek.mfll.dump.jpeg", value, "0");
    MBOOL dumpJpeg = atoi(value) || dumpAll || TEST_DUMP_JPEG;
    property_get("mediatek.mfll.dump.postview", value, "0");
    MBOOL dumpPostview = atoi(value) || dumpAll || TEST_DUMP_POSTVIEW;
    property_get("mediatek.mfll.dump.exif", value, "0");
    MBOOL dumpExif = atoi(value) || TEST_DUMP_EXIF;
    property_get("mediatek.mfll.dump.15raw", value, "0");
    MBOOL dump15Raw = atoi(value) || TEST_DUMP_15RAW;

    //
    if(dumpRaw)
        mDump |= MFLL_DUMP_RAW;
    if(dumpYuv)
        mDump |= MFLL_DUMP_YUV;
    if(dumpMfb)
        mDump |= MFLL_DUMP_MFB;
    if(dumpMixer)
        mDump |= MFLL_DUMP_MIXING;
    if(dumpJpeg)
        mDump |= MFLL_DUMP_JPEG;
    if(dumpPostview)
        mDump |= MFLL_DUMP_POSTVIEW;
    if(dumpExif)
        mDump |= MFLL_DUMP_EXIF;
    if(dump15Raw)
        mDump |= MFLL_DUMP_15RAW;
    MY_LOGD("[updateInfo] - mDump=0x%x", mDump);

    //
    property_get("mediatek.mfll.dump.sdcard", value, TEST_DUMP_TO_SDCARD);
    mSdcardIndex = atoi(value);
    if(mSdcardIndex == 1) {
        MY_LOGD("mkdir /storage/sdcard1/DCIM/Camera");
        if(!NSCam::Utils::makePath("/storage/sdcard1/DCIM/Camera",0660)) {
            MY_LOGE("makePath '/storage/sdcard1/DCIM/Camera' fail");
        }
    }

    // if skipmfb == 1, output should as same as singleshot
    property_get("mediatek.mfll.skipmfb", value, "0");
    mSkipMfb = atoi(value);
    if(mSkipMfb) {
        MY_LOGD("singleout - force skip mfb mSkipMfb(%d)", mSkipMfb);
    }
    MY_LOGD("[init] mSkipMfb(%d)", mSkipMfb);

    // BSS
    if(mMfbMode == MFB_MODE_AIS) {
        mEnableBss = CUST_MFLL_ENABLE_BSS_FOR_AIS;
    } else {
        mEnableBss = CUST_MFLL_ENABLE_BSS_FOR_MFLL;
    }
    property_get("mediatek.mfll.bss", value, "-1");
    if(atoi(value) != -1) {
        mEnableBss = atoi(value);
    }
    MY_LOGD("[init] mMfbMode(%d) mEnableBss(%d)", mMfbMode, mEnableBss);

    //
    mDebugInfo = IDbgInfoContainer::createInstance();

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
uninit()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    //
    if( mpPrvBufHandler )
    {
        // no need to destroy, since this is passed from adapter
        mpPrvBufHandler = NULL;
    }

    //
    if(mpMfllHal != NULL) {
        mpMfllHal->uninit();
        mpMfllHal->destroyInstance(LOG_TAG);
        mpMfllHal = NULL;
    }

    //
    if(mPrivateDataSize != 0) {
        MY_LOGD("delete(mPrivateData)");
        operator delete(mPrivateData);
        mPrivateDataSize = 0;
    }

    //
    if(mDebugInfo != NULL) {
        mDebugInfo->destroyInstance();
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
start(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    mSensorParam = rSensorParam;
    //
    dumpSensorParam(mSensorParam);
    MY_LOGE("didn't support MfllShot::start()");

    FUNCTION_LOG_END;
    //
    return ret;
}


MBOOL
MfllShot::
startOne(SensorParam const & rSensorParam)
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    MUINT32 threadRet;

    /* retrieve property before init */
    readProperty();

    //
    ret = ret && initInStartOne(rSensorParam);

    /* read property again, because user may want to force on something */
    readProperty();

    mrpInit();

    /*******************************************************************************
    * create threads
    ********************************************************************************/
    {
        pthread_attr_t attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, ANDROID_PRIORITY_FOREGROUND};
        //  1.  create threads
        // memory
        pthread_create(&mThreadMemory, &attr, MfllShot::memoryProc, this);
        // capture
        pthread_create(&mThreadCapture, &attr, MfllShot::captureProc, this);
        // capturePostProc
        pthread_create(&mThreadCapturePostProc, &attr, MfllShot::capturePostProc, this);
        if (eShotMode_ZsdMfllShot == meShotMode){
            // rawToYuy2
            pthread_create(&mThreadRawToYuy2, &attr, MfllShot::rawToYuy2Proc, this);
        }
    #if TEST_MEMC_YUY2
        // update p2 buffer
        pthread_create(&mThreadYuy2ToYv16, &attr, MfllShot::yuy2ToYv16Proc, this);
    #endif
        // mfll
        pthread_create(&mThreadMfll, &attr, MfllShot::mfllProc, this);
        // memc
        pthread_create(&mThreadMemc, &attr, MfllShot::memcProc, this);
        // blending
        pthread_create(&mThreadBlending, &attr, MfllShot::blendingProc, this);
        // postview
        pthread_create(&mThreadPostview, &attr, MfllShot::postviewProc, this);
        // full
        pthread_create(&mThreadFull, &attr, MfllShot::fullProc, this);
        // thumbnail
        pthread_create(&mThreadThumbnail, &attr, MfllShot::thumbnailProc, this);
        MY_LOGD("mfllthread all(%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld|%ld)"
                , mThreadMemory
                , mThreadCapture
                , mThreadCapturePostProc
                , mThreadRawToYuy2
                , mThreadYuy2ToYv16
                , mThreadMfll
                , mThreadMemc
                , mThreadBlending
                , mThreadPostview
                , mThreadFull
                , mThreadThumbnail
                );
        MY_LOGD("mfllthread mem(%ld), cap(%ld), capPostProc(%ld), rawToYuy2(%ld), yuy2ToYv16(%ld) mfll(%ld), memc(%ld), blend(%ld), pv(%ld), full(%ld), thumbnail(%ld)"
                , mThreadMemory
                , mThreadCapture
                , mThreadCapturePostProc
                , mThreadRawToYuy2
                , mThreadYuy2ToYv16
                , mThreadMfll
                , mThreadMemc
                , mThreadBlending
                , mThreadPostview
                , mThreadFull
                , mThreadThumbnail
                );
    }

    //flowMfll() is the main flow controller


    /*******************************************************************************
    * capture done
    ********************************************************************************/
    //  2.  join threads
    pthread_join(mThreadMemory, (void**)&threadRet);
    pthread_join(mThreadCapture, (void**)&threadRet);
    pthread_join(mThreadCapturePostProc, (void**)&threadRet);
    if (eShotMode_ZsdMfllShot == meShotMode){
        pthread_join(mThreadRawToYuy2, (void**)&threadRet);
    }
    #if TEST_MEMC_YUY2
    pthread_join(mThreadYuy2ToYv16, (void**)&threadRet);
    #endif
    pthread_join(mThreadMfll, (void**)&threadRet);
    pthread_join(mThreadMemc, (void**)&threadRet);
    pthread_join(mThreadBlending, (void**)&threadRet);
    pthread_join(mThreadPostview, (void**)&threadRet);
    pthread_join(mThreadFull, (void**)&threadRet);
    pthread_join(mThreadThumbnail, (void**)&threadRet);

    mIsSetDumpFileName = MFALSE;

lbExit:
    FUNCTION_LOG_END;
    return  ret;
}


MBOOL
MfllShot::
startOne(SensorParam const & rSensorParam, IImageBuffer const *pImgBuffer)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_LOGE("didn't support MfllShot::startOne()");

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
startAsync(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_LOGE("didn't support MfllShot::startAsync()");

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
stop()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    mfgIsForceBreak = MTRUE;
    MY_LOGE("didn't support MfllShot::stop()");

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
setShotParam(ShotParam const & rParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    mShotParam = rParam;
    dumpShotParam(mShotParam);

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
setJpegParam(JpegParam const & rParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    mJpegParam = rParam;
    dumpJpegParam(mJpegParam);

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
setDump(MUINT32 const & rParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    mDump = rParam;

    MY_LOGD("Dump mode = (%d)", mDump);


    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
setDumpFilePathAndName(const char * const pFilePathAndName)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    snprintf(mDumpFilename, sizeof(mDumpFilename), "%s", pFilePathAndName);


    MY_LOGD("DumpFilePathAndName = (%s)", mDumpFilename);

    mIsSetDumpFileName = MTRUE;

    FUNCTION_LOG_END;
    return ret;
}





MBOOL
MfllShot::
registerImageBuffer(ECamShotImgBufType const eBufType, IImageBuffer const *pImgBuffer)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_LOGD("regist %p to type %d", pImgBuffer, eBufType);

    switch(eBufType) {
        case ECamShot_BUF_TYPE_RAW:
            //mMemoryYuvMixing = pImgBuffer;
            //mRegistedBufferType |= ECamShot_BUF_TYPE_YUV;
            MY_LOGE("didn't support ECamShot_BUF_TYPE_RAW");
            ret = MFALSE;
            break;
        case ECamShot_BUF_TYPE_YUV:
            mMemoryYuvMixing = const_cast<IImageBuffer*>(pImgBuffer);
            mMemoryYuvMixing->incStrong(pImgBuffer);
            mRegistedBufferType |= ECamShot_BUF_TYPE_YUV;
            break;
        case ECamShot_BUF_TYPE_POSTVIEW:
            //mMemoryYuvMixing = pImgBuffer;
            //mRegistedBufferType |= ECamShot_BUF_TYPE_POSTVIEW;
            MY_LOGE("didn't support ECamShot_BUF_TYPE_POSTVIEW");
            ret = MFALSE;
            break;
        case ECamShot_BUF_TYPE_JPEG:
            //mMemoryYuvMixing = pImgBuffer;
            //mRegistedBufferType |= ECamShot_BUF_TYPE_JPEG;
            MY_LOGE("didn't support ECamShot_BUF_TYPE_JPEG");
            ret = MFALSE;
            break;
        case ECamShot_BUF_TYPE_THUMBNAIL:
            //mMemoryYuvMixing = pImgBuffer;
            //mRegistedBufferType |= ECamShot_BUF_TYPE_THUMBNAIL;
            MY_LOGE("didn't support ECamShot_BUF_TYPE_THUMBNAIL");
            ret = MFALSE;
            break;
    }
    MY_LOGD("regist(0x%x), registed(0x%x)", eBufType, mRegistedBufferType);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllShot::
setPrvBufHdl(MVOID* pCamBufHdl)
{
    mpPrvBufHandler = (ICamBufHandler*)pCamBufHdl;
    MY_LOGD("prv buf handle %p", (void*)mpPrvBufHandler);
    return MTRUE;
}


MBOOL
MfllShot::
setCapBufMgr(MVOID* pCapBufMgr)
{
    mpCapBufMgr = (CapBufMgr*)pCapBufMgr;
    return MTRUE;
}


MBOOL
MfllShot::
sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MY_LOGD("command(0x%x) arg(%d,%d,%d)", cmd, arg1, arg2, arg3);

    switch(cmd) {
        case ECamShot_CMD_SET_NRTYPE:
            muNRType = arg1;    //ECamShot_NRTYPE_HWNR, ECamShot_NRTYPE_SWNR
            MY_LOGD("NR type 0x%x", muNRType);
            break;

        default:
            MY_LOGE("unsupported command 0x%x", cmd);
            ret = MFALSE;
            break;
    }

    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//  mfll specific functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// thread
//------------------------------------------------------------------------------
MVOID*
MfllShot::
memoryProc(void *arg)
{
    ::prctl(PR_SET_NAME,"memoryProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowMemory();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
captureProc(void *arg)
{
    ::prctl(PR_SET_NAME,"captureProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowCapture();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}

MVOID*
MfllShot::
capturePostProc(void *arg)
{
    ::prctl(PR_SET_NAME,"capturePostProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowCapturePostProc();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}

MVOID*
MfllShot::
rawToYuy2Proc(void *arg)
{
    ::prctl(PR_SET_NAME,"rawToYuy2Proc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowRawToYuy2();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
yuy2ToYv16Proc(void *arg)
{
    ::prctl(PR_SET_NAME,"yuy2ToYv16Proc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowYuy2ToYv16();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
mfllProc(void *arg)
{
    ::prctl(PR_SET_NAME,"mfllProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowMfll();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
memcProc(void *arg)
{
    ::prctl(PR_SET_NAME, "memcProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowMemc();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
blendingProc(void *arg)
{
    ::prctl(PR_SET_NAME, "blendingProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowBlending();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
postviewProc(void *arg)
{
    ::prctl(PR_SET_NAME,"postviewProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowPostview();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
fullProc(void *arg)
{
    ::prctl(PR_SET_NAME,"fullProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowFull();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


MVOID*
MfllShot::
thumbnailProc(void *arg)
{
    ::prctl(PR_SET_NAME,"thumbnailProc", 0, 0, 0);
    FUNCTION_LOG_START;
    MUINTPTR     ret = MTRUE;

    MfllShot *self = (MfllShot*)arg;
    CHECK_OBJECT(self);
    ret = ret && self->flowThumbnail();

lbExit:
    FUNCTION_LOG_END_MUM;
    return (MVOID*)ret;
}


//------------------------------------------------------------------------------
// flow
//------------------------------------------------------------------------------
MBOOL
MfllShot::
flowMemory()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    ret = ret && allocateMemory();

    FUNCTION_LOG_END;
    return ret;
}


#if TEST_MEMC_YUY2
static void yuy2_to_yv16(MUINT8* pSrc, MUINT8* pDst, MUINT32 Width, MUINT32 Height)
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;

    MUINT8* pTemp = new MUINT8[Width*Height*2];
    MUINT8* pDstLuma = pTemp;
    MUINT8* pDstCb   = pDstLuma + Width*Height;
    MUINT8* pDstCr   = pDstCb   + (Width>>1)*Height;
    MINT32 i;
    MINT32 pixelnum  = Width*Height;

    for(i=0; i<pixelnum; i++)
    {
        *pDstLuma++ = *pSrc++;
        if(i&0x1) //Cr
        {
            *pDstCr++ = *pSrc++;
        }
        else //Cb
        {
            *pDstCb++ = *pSrc++;
        }
    }
    memcpy(pDst, pTemp, sizeof(MUINT8)*pixelnum*2);
    delete pTemp;

    FUNCTION_LOG_END_MUM;
}
#endif


MBOOL
MfllShot::
flowCapture()
{
    MFLL_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;
    MUINT32 capBufLen = 0;

    if (eShotMode_ZsdMfllShot == meShotMode){
        WAIT(&mTriggerCapture);
        CHECK_OBJECT(mpCapBufMgr);
        mpCapBufMgr->getBufLen(capBufLen,CapBufMgr::CAP_BUF_LEN_MAX);
        if (capBufLen < (mReferenceFrameLength + 1))
            MY_LOGE("ZSD frame number (%d) < reference frame (%d)", capBufLen, mReferenceFrameLength);
        for(MUINT32 i=0; i < capBufLen; i++) {
            ret = ret && mpCapBufMgr->dequeBuf(mMemoryCapBufRaw[i]);
            MY_LOGD("dequeBuf[%d]:%p from CapBufMgr:",i, (void *)mMemoryCapBufRaw[i]);
            CHECK_OBJECT(mMemoryCapBufRaw[i]);
            ANNOUNCE(&mMemoryReady_CapBuf[i]);
        }
    } else {
        ShotParam burstShotParam;

        WAIT(&mTriggerCapture);

        if(mDump & MFLL_DUMP_15RAW){
           doCapture15Raw();
        }

#if TEST_CAPTURE
        //
        mpBurstShot = IBurstShot::createInstance(static_cast<EShotMode>(meShotMode), "MfllShot");
        CHECK_OBJECT(mpBurstShot);
        mpBurstShot->init();

        // set isp profile
        mpBurstShot->setIspProfile(muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_MFB_Capture_EE_Off_SWNR : EIspProfile_MFB_Capture_EE_Off);

        // alloc a buffer for pass1 out
        for(MUINT32 i=0; i<mReferenceFrameLength+1; i++) {
            ret = ret && mpBurstShot->registerImageBuffer(ECamShot_BUF_TYPE_RAW, mMemoryRaw[i].get());
            CHECK_RET("register pass1 buffer [%d] fail, (%p)", i, mMemoryRaw[i].get());
            MY_LOGD("register pass1 buffer [%d], (%p)", i, mMemoryRaw[i].get());
        }

        // alloc a buffer for pass2 out
        WAIT(&mMemoryReady_QYuvB);
        ret = ret && mpBurstShot->registerImageBuffer(ECamShot_BUF_TYPE_YUV, mMemoryQYuvB.get());
        CHECK_RET("register pass2 buffer [base] fail, (%p)", mMemoryQYuvB.get());
        MY_LOGD("register pass2 buffer [base], (%p)", mMemoryQYuvB.get());
        for(MUINT32 i=0; i<mReferenceFrameLength; i++) {
            WAIT(&mMemoryReady_QYuvR[i]);
            ret = ret && mpBurstShot->registerImageBuffer(ECamShot_BUF_TYPE_YUV, mMemoryQYuvR[i].get());
            CHECK_RET("register pass2 buffer [ref %d] fail, (%p)", i, mMemoryQYuvR[i].get());
            MY_LOGD("register pass2 buffer [ref %d] (%p)", i, mMemoryQYuvR[i].get());
        }

        // notify
        mpBurstShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
        mpBurstShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_RAW
                                    | NSCamShot::ECamShot_DATA_MSG_YUV
                                    | NSCamShot::ECamShot_DATA_MSG_EIS
                                    );
        mpBurstShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);

        // execute transform / zoom at doMixing()
        burstShotParam = mShotParam;
        burstShotParam.u4PictureTransform = 0;
        burstShotParam.u4ZoomRatio = 100;
        burstShotParam.ePictureFmt =  muNRType==ECamShot_NRTYPE_SWNR ? eImgFmt_I420 : eImgFmt_YUY2;

        //
        ret = ret && mpBurstShot->setShotParam(burstShotParam);
        ret = ret && mpBurstShot->setJpegParam(mJpegParam);
        ret = ret && mpBurstShot->setPrvBufHdl((MVOID*)mpPrvBufHandler);
        CHECK_RET("fail before pBurstShot->start");
        ret = ret && mpBurstShot->start(mSensorParam, mReferenceFrameLength+1);
        CHECK_RET("fail in pBurstShot->start");
        //
        ret = ret && mpBurstShot->uninit();
        mpBurstShot->destroyInstance();
    }
#else
    msleep(100);
#endif

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
MfllShot::
flowCapturePostProc()
{
    MFLL_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

#if TEST_MEMC_YUY2
    WAIT(&mYuy2ToYv16Done);
#endif

    // EIS
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        doEis(ref, mGmv[ref][0], mGmv[ref][1]);
    }

    // bss
#if TEST_BSS
    doBss();
#endif

    if(mDump & MFLL_DUMP_RAW) {
        //pass1
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        NSCam::SensorStaticInfo sensorInfo;
        querySensorInfo(mSensorParam.u4OpenID, sensorInfo);

        MUINT32 sensorbit = 10;
        switch(sensorInfo.rawSensorBit) {
            case RAW_SENSOR_8BIT:  sensorbit = 8; break;
            case RAW_SENSOR_10BIT: sensorbit = 10; break;
            case RAW_SENSOR_12BIT: sensorbit = 12; break;
            case RAW_SENSOR_14BIT: sensorbit = 14; break;
        }

        //
        //IMG_20140101_003010_exp0066_iso0000_0__4192x3104_10_0.raw //use CTO naming role
        #define UPDATE_RAW_FILENAME(id) \
        snprintf(filename, sizeof(filename), "%s_%02d_exp%04d_iso%04d_%d__%dx%d_%d_%d.raw"     \
            , mDumpFilename                                                 \
            , 0                                                             \
            , mDbgAisInfo.calculatedExposure                                \
            , mDbgAisInfo.calculatedIso                                     \
            , id                                                            \
            , mRaw_Width                                                    \
            , mRaw_Height                                                   \
            , sensorbit                                                     \
            , sensorInfo.sensorFormatOrder                                  \
            )
        UPDATE_RAW_FILENAME(0);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryRawB->saveToFile(filename);
        //
        UPDATE_RAW_FILENAME(1);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryRawR[0]->saveToFile(filename);
        //
        UPDATE_RAW_FILENAME(2);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryRawR[1]->saveToFile(filename);
        //
        UPDATE_RAW_FILENAME(3);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryRawR[2]->saveToFile(filename);
        #undef UPDATE_RAW_FILENAME
    }

    if(mDump & MFLL_DUMP_YUV) {
        //pass2
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        //
        snprintf(filename, sizeof(filename), "%s_%02d_mMemoryQYuvB_%04dx%04d.yv16", mDumpFilename, 1, mYuvQ_Width, mYuvQ_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryQYuvB->saveToFile(filename);
        //
        snprintf(filename, sizeof(filename), "%s_%02d_mMemoryQYuvR0_%04dx%04d.yv16", mDumpFilename, 1, mYuvQ_Width, mYuvQ_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryQYuvR[0]->saveToFile(filename);
        //
        snprintf(filename, sizeof(filename), "%s_%02d_mMemoryQYuvR1_%04dx%04d.yv16", mDumpFilename, 1, mYuvQ_Width, mYuvQ_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryQYuvR[1]->saveToFile(filename);
        //
        snprintf(filename, sizeof(filename), "%s_%02d_mMemoryQYuvR2_%04dx%04d.yv16", mDumpFilename, 1, mYuvQ_Width, mYuvQ_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryQYuvR[2]->saveToFile(filename);
    }

#if TEST_BITTURE
    //
    mMemoryQYuvB->unlockBuf(LOG_TAG);
    mMemoryQYuvB->loadFromFile("/sdcard/input/qdip0.yuv_1600x1200.yuy2");
    mMemoryQYuvB->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    //
    mMemoryQYuvR[0]->unlockBuf(LOG_TAG);
    mMemoryQYuvR[0]->loadFromFile("/sdcard/input/qdip1.yuv_1600x1200.yuy2");
    mMemoryQYuvR[0]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    //
    mMemoryQYuvR[1]->unlockBuf(LOG_TAG);
    mMemoryQYuvR[1]->loadFromFile("/sdcard/input/qdip2.yuv_1600x1200.yuy2");
    mMemoryQYuvR[1]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    //
    mMemoryQYuvR[2]->unlockBuf(LOG_TAG);
    mMemoryQYuvR[2]->loadFromFile("/sdcard/input/qdip3.yuv_1600x1200.yuy2");
    mMemoryQYuvR[2]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
#endif

#if TEST_DBGINFO
    updateRegToDebugInfo(MF_REGTABLE_BEFORE_MFB);
#endif

    if(mDump & MFLL_DUMP_EXIF) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        snprintf(filename, sizeof(filename), "%s_%02d_reg.txt", mDumpFilename, 8);
        MY_LOGD("save obc/pgn/g2g to %s", filename);
        dumpRegAsTextFile(filename);
    }

    /* Returns reference RAW buffers back to CapBufMgr ASAP */
    for (int i = 0; i < mReferenceFrameLength; i++)
        zsdReleaseBuffer(i + 1);

    /* release raw buffers except base raw if necessary. */
    mrpReleaseRefRawBuffers();

    /* prepare buffers for ME/MC */
    mrpAllocateMemcBuffer();

    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        ANNOUNCE(&mQrReady[ref]);
    }

    /* prepare buffers for YUV base */
    mrpAllocateYuvBaseBuffer();

    // 2. convert RawB to YuvB  //do this after base image has been determined
    MY_LOGD("flowCapture doEncodeYuvB %d", ret);
    ret = ret && doEncodeYuvB();

    /* encode YUV golden as soon as possible if not using reduce memory plan */
    if (mReduceMemoryUsage != eMrpMode_LowestMemory) {
        ret = ret && doEncodeNormalYuvB();
        /* Returns base RAW buffer to CapBufMgr if only if it's not necessary anymore */
        zsdReleaseBuffer(0);
    }

    ANNOUNCE(&mCaptureDone);
lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
MfllShot::
flowRawToYuy2()
{
    MFLL_TRACE_CALL();
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    for(MUINT32 i=0; i < mReferenceFrameLength+1; i++) {
        WAIT(&mMemoryReady_CapBuf[i]);

        if(i == mReferenceFrameLength){
            ret = ret && handleNotifyCallback(ECamShot_NOTIFY_MSG_EOF, 0, 0);
        }

        if (DONT_MEMCPY){
            MY_LOGD("Skip  MEMCPY");
            mMemoryRaw[i] = mMemoryCapBufRaw[i];
        } else {
            MY_LOGD("Do  MEMCPY");
            memcpy((void*)mMemoryRaw[i]->getBufVA(0), (void*)mMemoryCapBufRaw[i]->getBufVA(0), mMemoryRaw[i]->getBufSizeInBytes(0));
        }

        //keep raw in order
        mMemoryRawInCaptureOrder[mPass1Index] = mMemoryRaw[i];
        if (mPass1Index == 0){
            mMemoryRawB = mMemoryRaw[i];
        } else {
            mMemoryRawR[mPass1Index-1] = mMemoryRaw[i];
        }

        //backup private data
        MUINT32 magicNum = 0;
        MBOOL isRrzo = MFALSE;
        MVOID* pPrivateData = NULL;
        MUINT32 privateDataSize = 0;

        IspSyncControl* pIspSyncCtrl = NULL;
        pIspSyncCtrl = IspSyncControl::createInstance(mSensorParam.u4OpenID);
        CHECK_OBJECT(pIspSyncCtrl);

        //
        pIspSyncCtrl->queryImgBufInfo(mMemoryCapBufRaw[i]
                                    , magicNum
                                    , isRrzo
                                    , pPrivateData
                                    , privateDataSize
                                    );
        if(mPrivateDataSize == 0) {
            mPrivateDataSize = privateDataSize;
            mPrivateData = operator new(mPrivateDataSize);
            memcpy(mPrivateData, pPrivateData, mPrivateDataSize);
            MY_LOGD("mPrivateDataSize = %d", mPrivateDataSize);
        }


#if 0
        // TODO add dbg info
        {   //dbginfo
            if(mPass2Index == 0) {
                IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(msg.ext2);
                pDbgInfo->copyTo(mDebugInfo);
            }
        }
#endif
        //get Eis Statistic data
        if(mEisId > 0) {
            pIspSyncCtrl->getEisStatistic(mMemoryCapBufRaw[i], mEisStat[mEisId-1]);
        }
        ++mEisId;
        pIspSyncCtrl->destroyInstance();
        ++mPass1Index;

        if(i==0){
            MFLL_TRACE_NAME("encode_QYuvB");
            //encode raw to yuy2 for QYuvB
            WAIT(&mMemoryReady_QYuvB);
            ret = ret && encodeRawToYuv(mMemoryRaw[0],mMemoryQYuvB,EIspProfile_VSS_MFB_PostProc_EE_Off,mPrivateData);
            CHECK_RET("fail encodeRawToYuv of mMemoryQYuvB");
            ANNOUNCE(&mCapP2Ready[mPass2Index]);
            ++mPass2Index;
        } else {
            MFLL_TRACE_NAME("encode_QYuvR");
            //encode raw to yuy2 for QYuvR[MAX_REFERENCE_FRAMES]
            WAIT(&mMemoryReady_QYuvR[i-1]);
            ret = ret && encodeRawToYuv(mMemoryRaw[i],mMemoryQYuvR[i-1],EIspProfile_VSS_MFB_PostProc_EE_Off,mPrivateData);
            CHECK_RET("fail encodeRawToYuv of mMemoryQYuvR[ref %d], (%p)", i, mMemoryQYuvR[i].get());
            ANNOUNCE(&mCapP2Ready[mPass2Index]);
            ++mPass2Index;
        }

    }
lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
MfllShot::
flowYuy2ToYv16()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mCapP2Ready[0]);
    yuy2_to_yv16((MUINT8*)mMemoryQYuvB->getBufVA(0), (MUINT8*)mMemoryQYuvB->getBufVA(0), mYuvQ_Width, mYuvQ_Height);
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        WAIT(&mCapP2Ready[ref+1]);
        yuy2_to_yv16((MUINT8*)mMemoryQYuvR[ref]->getBufVA(0), (MUINT8*)mMemoryQYuvR[ref]->getBufVA(0), mYuvQ_Width, mYuvQ_Height);
    }
    ANNOUNCE(&mYuy2ToYv16Done);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowMfll()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    //  1.  allocate memory
    WAIT(&mMemoryReady_Pass1);

    //  2.  capture
    ANNOUNCE(&mTriggerCapture);
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        WAIT(&mQrReady[ref]);
        ANNOUNCE(&mTriggerMemc[ref]);
    }

    //  3.  blending
    WAIT(&mCaptureDone);
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        ANNOUNCE(&mTriggerBlending[ref]);
        MY_LOGD("announce mTriggerBlending[%d]", ref);
        WAIT(&mBlendingDone[ref]);

        /* if reduce memory, release memory ASAP */
        mrpReleaseQYuvBuffer(ref);
    }

    /* release Mfb related buffers */
    mrpReleaseMfbRelatedBuffers();

    /* if reduce memory plan is on */
    if (mReduceMemoryUsage == eMrpMode_LowestMemory) {
        // 4. encode golden YUV for mixing if necessary
        ret = ret && doEncodeNormalYuvB();
        /* Returns base RAW buffer to CapBufMgr if only if it's not necessary anymore */
        zsdReleaseBuffer(0);
    }

    /* allocate output YUV buffer if necessary */
    mrpAllocateYuvMixingBuffer();

    //  5.  mixing
    MY_LOGD("flowMfll mixing, mSkipMfb(%d)", mSkipMfb);
    if(mSkipMfb) {
        ret = ret && doSingleFrameOutput();
    } else {
        ret = ret && doMixing();
    }
    handleDataCallback(ECamShot_DATA_MSG_YUV
                        , 0
                        , (MUINTPTR)mDebugInfo
                        , mMemoryYuvMixing.get()
                        );

    /* ask for buffer if necessary */
    mrpAllocateJpegBuffer();

    //  6.  postview & encode
    MY_LOGD("flowMfll postview");
    ANNOUNCE(&mTriggerFullJpeg);
    ANNOUNCE(&mTriggerPostview);
    WAIT(&mPostviewDone);
    if(mi4DataMsgSet & ECamShot_BUF_TYPE_POSTVIEW) {
        handleDataCallback(ECamShot_DATA_MSG_POSTVIEW
                            , 0
                            , 0
                            , mMemoryPostview.get()
                            );
    }

    //  7.  update MFLL debug info
    ret = ret && setDebugInfo(mDebugInfo);

    //  8.  save jpeg
    MY_LOGD("flowMfll save jpeg");
    ANNOUNCE(&mTriggerThumbnailJpeg);
    WAIT(&mFullJpegDone);
    WAIT(&mThumbnailJpegDone);
    if(mi4DataMsgSet & ECamShot_DATA_MSG_JPEG) {
        handleDataCallback(ECamShot_DATA_MSG_JPEG
                            , (MUINTPTR)mMemoryJpeg_Thumbnail.get()
                            , (MUINTPTR)mDebugInfo
                            , mMemoryJpeg.get()
                            );
    }


lbExit:
    //  8.  release memory
    releaseMemory();

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowMemc()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MY_LOGD("mReferenceFrameLength=%d", mReferenceFrameLength);
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        MY_LOGD("wait mTriggerMemc[%d]", ref);
        WAIT(&mTriggerMemc[ref]);
        MY_LOGD("wait mTriggerMemc[%d] ok", ref);
        ret = ret && doMemc(ref);

        /* release ME/MC working buffer in the last time */
        mrpReleaseMemcWorkingBuffer(ref+1);

        ANNOUNCE(&mMemcDone[ref]);
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowBlending()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        MY_LOGD("wait mTriggerBlending[%d]", ref);
        WAIT(&mTriggerBlending[ref]);
        WAIT(&mMemcDone[ref]);

        /* the first time to trigger MFB, make sure memory is ready */
        if (ref == 0)
            mrpAllocateMfbRelatedBuffers();

        if(mSkipMfb) {
            MY_LOGD("Skip MFB for give up MEMC");
        } else {
            ret = ret && doMfb(ref);
        }
        ANNOUNCE(&mBlendingDone[ref]);
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowPostview()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mTriggerPostview);
    ret = ret && doPostview();
    ANNOUNCE(&mPostviewDone);

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowFull()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mTriggerFullJpeg);
    ret = ret && doFullJpeg();
    ANNOUNCE(&mFullJpegDone);

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
flowThumbnail()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mTriggerThumbnailJpeg);
    ret = ret && doThumbnailJpeg();
    ANNOUNCE(&mThumbnailJpegDone);

    FUNCTION_LOG_END;
    return ret;
}

//------------------------------------------------------------------------------
// utility: misc
//------------------------------------------------------------------------------
void MfllShot::readProperty(void)
{
#if MFLL_READ_PROPERTY
    char *p = new char[PROPERTY_VALUE_MAX];
    int  v = 0;

    /* Memory Reduce Plan on/off property */
    property_get("mediatek.mfll.mrp", p, "-1");
    v = atoi(p);
    if (v != -1) {
        switch (v) {
        case eMrpMode_None:
            mReduceMemoryUsage = eMrpMode_None;
            break;
        case eMrpMode_Balance:
            mReduceMemoryUsage = eMrpMode_Balance;
            break;
        case eMrpMode_LowestMemory:
            mReduceMemoryUsage = eMrpMode_LowestMemory;
            break;
        default:
            mReduceMemoryUsage = (enum eMrpMode)MFLL_REDUCE_MEMORY_USAGE;
            break;
        }
        MY_LOGD("Force Memory Reduce Plan = %d", mReduceMemoryUsage);
    }

    delete [] p;
#endif
}



//------------------------------------------------------------------------------
// utility: memory
//------------------------------------------------------------------------------
MBOOL
MfllShot::
announce(pthread_mutex_t *mutex, char *note)
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    pthread_mutex_unlock(mutex);
    MUINT32 *p = (MUINT32*)mutex;
    MY_LOGD("announce %s %p v=%08x", note, mutex, *p);

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
wait(pthread_mutex_t *mutex, char *note)
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MUINT32 *p = (MUINT32*)mutex;
    MY_LOGD("wait %s %p start v=%08x", note, mutex, *p);
    pthread_mutex_lock(mutex);
    pthread_mutex_unlock(mutex);
    MY_LOGD("wait %s %p ready v=%08x", note, mutex, *p);

    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: memory
//------------------------------------------------------------------------------
/**
 *  Allocate memory is an art, with Memory Reduce Plan, we have to control the
 *  lifetime of these buffers.
 *
 *  Allocation sequence:
 *  1)  RAW buffers x 4 (if necessary, not while using ZSD)
 *  2)  QYUV buffers x 4
 *
 *  3)  ME/MC working buffer
 *        [MRP]: This buffer should be created until 3 RAW buffers have been
 *               released.
 *
 *  4)  YUV base frame buffer
 *        [MRP]: This buffer should be created until 3 RAW buffers have been
 *               released.
 *
 *      YUV golden frame buffer (if necessary, not while using MRP)
 *
 *  5)  MFB weighting table x 2
 *        [MRP]: These buffers should be created until the first ME/MC has been
 *               done (before the first time of MFB).
 *
 *  6)  Output YUV (if necessary, not while user registered one)
 *        [MRP]: This buffer should be created until ready to do mixing.
 *
 *  7)  Post view buffers
 *  8)  JPEG buffers
 *        [MRP]: This buffer should be created after mixing has been done.
 */
MBOOL
MfllShot::
allocateMemory()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MUINT32 WorkingBufferSize = 0;

    // Raw Buffer
    MY_LOGD("prepare Raw buffer");

    //
    CHECK_OBJECT(mpMfllHal);
    mpMfllHal->queryWorkingBufferSize(WorkingBufferSize);
    IImageBufferHeap *pHeap;

    //0-3
    // mMemoryRawB;             15M
    // mMemoryRawR;             15M
    MFLL_TRACE_BEGIN("aRaw");
    if(eShotMode_ZsdMfllShot != meShotMode){
        for(MUINT32 ref=0; ref<mReferenceFrameLength+1; ref++) {
            ret = ret && allocImageBuffer(&pHeap, &mMemoryRaw[ref], mRaw_Width, mRaw_Height, eImgFmt_BAYER10);
        }
    } else {
        if (!DONT_MEMCPY){
            for(MUINT32 ref=0; ref<mReferenceFrameLength+1; ref++) {
                ret = ret && allocImageBuffer(&pHeap, &mMemoryRaw[ref], mRaw_Width, mRaw_Height, eImgFmt_BAYER10);
            }
        }
    }
    ANNOUNCE(&mMemoryReady_Pass1);
    MFLL_TRACE_END();

    //4-8
    // mMemoryQYuvB;            6M
    // mMemoryQYuvR[3];         6M
    {
        MFLL_TRACE_NAME("aQYuv");
        ret = ret && allocImageBuffer(&pHeap, &mMemoryQYuvB, mYuvQ_Width, mYuvQ_Height, eImgFmt_YUY2);
        ANNOUNCE(&mMemoryReady_QYuvB);
        for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
            ret = ret && allocImageBuffer(&pHeap, &mMemoryQYuvR[ref], mYuvQ_Width, mYuvQ_Height, eImgFmt_YUY2);
            ANNOUNCE(&mMemoryReady_QYuvR[ref]);
        }
    }

    //11-14
    // mMemoryLocalMotion;      30M
    {
        WAIT(&mTriggerAllocateMemc); // only wait in MRP plan
        MFLL_TRACE_NAME("aLM");

        ret = ret && allocBlobBuffer(&pHeap, &mMemoryLocalMotion, WorkingBufferSize);

        ANNOUNCE(&mAllocateMemcDone);
        for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
            ANNOUNCE(&mMemoryReady_Memc[ref]);
        }
    }


    //9
    // mMemoryYuvB;             24M
    {
        WAIT(&mTriggerAllocateYuvBase);
        MFLL_TRACE_NAME("aYuvB");
        ret = ret && allocImageBuffer(&pHeap, &mMemoryYuvB, mYuv_Width, mYuv_Height, eImgFmt_YUY2);
        mPtrYuvBase = mMemoryYuvB;

        /**
         *  If to reduce memory usage, the golden YUV frame buffer will
         *  reuse mMemoryYuvB after MFB done to avoid memory usage peak
         */
        if (mReduceMemoryUsage == eMrpMode_LowestMemory)
            mPtrYuvGolden = mMemoryYuvB;
        else {
            ret = ret && allocImageBuffer(&pHeap, &mMemoryYuvGolden, mYuv_Width, mYuv_Height, eImgFmt_YUY2);
            mPtrYuvGolden = mMemoryYuvGolden;
        }
    }
    ANNOUNCE(&mAllocateYuvBaseDone);
    ANNOUNCE(&mMemoryReady_YuvB);


    //10
    // mMemoryWeightingMap;     12M
    WAIT(&mTriggerAllocateMfbRelatedBuffers);
    {
        MFLL_TRACE_NAME("aWM");
        ret = ret && allocImageBuffer(&pHeap, &mMemoryWeightingMap[0], mYuv_Width, mYuv_Height, eImgFmt_Y8);
        ret = ret && allocImageBuffer(&pHeap, &mMemoryWeightingMap[1], mYuv_Width, mYuv_Height, eImgFmt_Y8);
    }

    // mMemoryYuv;              24M
    {
        MFLL_TRACE_NAME("aYuv");
        ret = ret && allocImageBuffer(&pHeap, &mMemoryYuv, mYuv_Width, mYuv_Height, eImgFmt_YUY2);
        for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
            ANNOUNCE(&mMemoryReady_Mfb[ref]);
        }
    }
    ANNOUNCE(&mAllocateMfbRelatedBuffersDone);


    //
    if(!(mRegistedBufferType&ECamShot_BUF_TYPE_YUV)) {
        WAIT(&mTriggerAllocateYuvMixing);
        MFLL_TRACE_NAME("aMix");
        ret = ret && allocImageBuffer(&pHeap, &mMemoryYuvMixing, mRotPicWidth, mRotPicHeight, eImgFmt_YUY2);
    }
    ANNOUNCE(&mAllocateYuvMixingDone);


    if(mi4DataMsgSet & ECamShot_DATA_MSG_JPEG) {
        WAIT(&mTriggerAllocateJpeg);

        MFLL_TRACE_NAME("aJpeg");
        //
        // mMemoryJpeg;             24M
        ret = ret && allocJpegBuffer(&pHeap
                                    , &mMemoryJpeg
                                    , mRotPicWidth
                                    , mRotPicHeight);

        //
        // mMemoryJpeg_Thumbnail;   ??M
        ret = ret && allocJpegBuffer(&pHeap
                                    , &mMemoryJpeg_Thumbnail
                                    , mRotThuWidth
                                    , mRotThuHeight);

        ANNOUNCE(&mMemoryReady_Jpeg);
    }
    ANNOUNCE(&mAllocateJpegDone);


    if(mi4DataMsgSet & ECamShot_BUF_TYPE_POSTVIEW) {
        MFLL_TRACE_NAME("aPost");
        //
        //IMEM_BUF_INFO mMemoryPostview;        ??M
        ret = ret && allocImageBuffer(&pHeap, &mMemoryPostview
                                        , mPostview_Width
                                        , mPostview_Height
                                        , mShotParam.ePostViewFmt
                                        );
        ANNOUNCE(&mMemoryReady_Postview);
    }

    // create buffer thread
    // WAIT buffer ready
    //
    if(!ret) {
        MY_LOGE("error: can't allocate memory");
        goto lbExit;
    }

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
releaseMemory()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    // 6. release memory
#if TEST_MEMORY
    // mMemoryLocalMotion;                  30M
    freeImageBuffer(&mMemoryLocalMotion);

    // free buffers
    if(eShotMode_ZsdMfllShot != meShotMode){
        for(MUINT32 ref=0; ref<mReferenceFrameLength+1; ref++) {
            freeImageBuffer(&mMemoryRaw[ref]);
        }
        freeImageBuffer(&mMemoryRawB);
        for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
            freeImageBuffer(&mMemoryRawR[ref]);
        }
    } else {
        if (!DONT_MEMCPY){
            for(MUINT32 ref=0; ref<mReferenceFrameLength+1; ref++) {
                freeImageBuffer(&mMemoryRaw[ref]);
            }
            freeImageBuffer(&mMemoryRawB);
            for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
                freeImageBuffer(&mMemoryRawR[ref]);
            }
        }
    }

    //
    // mMemoryYuvB;                         24M
    freeImageBuffer(&mMemoryYuvB);
    freeImageBuffer(&mMemoryYuvGolden);

    //
    // mMemoryQYuvB;                        6M
    // mMemoryQYuvR[3];                     6M
    freeImageBuffer(&mMemoryQYuvB);
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++) {
        freeImageBuffer(&mMemoryQYuvR[ref]);
    }

    //
    // mMemoryYuv;                          24M
    freeImageBuffer(&mMemoryYuv);

    //
    // mMemoryWeightingMap;                 12M
    freeImageBuffer(&mMemoryWeightingMap[0]);
    freeImageBuffer(&mMemoryWeightingMap[1]);

    // mMemoryYuvMixing                     24M
    if(!(mRegistedBufferType&ECamShot_BUF_TYPE_YUV))
        freeImageBuffer(&mMemoryYuvMixing);

    if(mi4DataMsgSet & ECamShot_DATA_MSG_JPEG) {
        // mMemoryJpeg;                         24M
        freeImageBuffer(&mMemoryJpeg);
        //
        // mMemoryJpeg_Thumbnail;               ??M
        freeImageBuffer(&mMemoryJpeg_Thumbnail);
    }

    if(mi4DataMsgSet & ECamShot_DATA_MSG_POSTVIEW) {
        //
        // mMemoryPostview;                     ??M
        freeImageBuffer(&mMemoryPostview);
    }
#else
    msleep(50);
#endif

    FUNCTION_LOG_END;
    return ret;
}

MBOOL MfllShot::zsdReleaseBuffer(int index)
{
    if (meShotMode != eShotMode_ZsdMfllShot)
        return MFALSE;

    if (mMemoryCapBufRaw[index]) {
        mpCapBufMgr->enqueBuf(mMemoryCapBufRaw[index]);
        mMemoryCapBufRaw[index] = NULL;
        MY_LOGD("%s - Returns ZSD buffer %d", __FUNCTION__, index);
    }
    else {
        MY_LOGD("%s - ZSD buffer %d has been returned twice, ignored.", __FUNCTION__, index);
    }

    return MTRUE;
}

/* We should unlock all lockers if not using MRP */
void MfllShot::mrpInit(void)
{
    if (mReduceMemoryUsage != eMrpMode_None)
        return;

    ANNOUNCE(&mTriggerAllocateYuvBase);
    ANNOUNCE(&mTriggerAllocateMfbRelatedBuffers);
    ANNOUNCE(&mTriggerAllocateMemc);
    ANNOUNCE(&mTriggerAllocateYuvMixing);
    ANNOUNCE(&mTriggerAllocateJpeg);

    ANNOUNCE(&mAllocateYuvBaseDone);
    ANNOUNCE(&mAllocateMfbRelatedBuffersDone);
    ANNOUNCE(&mAllocateMemcDone);
    ANNOUNCE(&mAllocateYuvMixingDone);
    ANNOUNCE(&mAllocateJpegDone);
}

MBOOL MfllShot::mrpReleaseRefRawBuffers(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    /* No need to release RAW buffers if using ZSD mode */
    if (meShotMode == eShotMode_ZsdMfllShot)
        return MTRUE;

    /* If specify not copy RAW, don't release */
    if (DONT_MEMCPY)
        return MTRUE;

    MFLL_TRACE_NAME("mrpRefRaw");

    /* release un-necessary RAW buffers if using */
    for(MUINT32 ref=0; ref<mReferenceFrameLength; ref++)
        freeImageBuffer(&mMemoryRawR[ref]);

    return MTRUE;
}

MBOOL MfllShot::mrpReleaseQYuvBuffer(unsigned int index)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    MFLL_TRACE_NAME("mrpQyuv");
    freeImageBuffer(&mMemoryQYuvR[index]);

    return MTRUE;
}

MBOOL MfllShot::mrpReleaseMemcWorkingBuffer(unsigned int index)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    MFLL_TRACE_NAME("mprMemc");
    if (index == mReferenceFrameLength) {
        freeImageBuffer(&mMemoryLocalMotion);
    }

    return MTRUE;
}

MBOOL MfllShot::mrpReleaseMfbRelatedBuffers(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    /* release not un-necessary weighting table */
    freeImageBuffer(&mMemoryWeightingMap[mMemoryFinalWeightingMapIndex == 0 ? 1 : 0]);

    return MTRUE;
}

MBOOL MfllShot::mrpReleaseMixingRelatedBuffers(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    /**
     *  After mixing, blended/golden buffer is un-necessary anymore
     */
    freeImageBuffer(&mPtrYuvBase);
    freeImageBuffer(&mPtrYuvBlended);
    freeImageBuffer(&mPtrYuvGolden);
    freeImageBuffer(&mMemoryWeightingMap[0]);
    freeImageBuffer(&mMemoryWeightingMap[1]);

    return MTRUE;
}

MBOOL MfllShot::mrpAllocateMemcBuffer(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    ANNOUNCE(&mTriggerAllocateMemc);
    WAIT(&mAllocateMemcDone);
    return MTRUE;
}

MBOOL MfllShot::mrpAllocateYuvBaseBuffer(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    ANNOUNCE(&mTriggerAllocateYuvBase);
    WAIT(&mAllocateYuvBaseDone);
    return MTRUE;
}

MBOOL MfllShot::mrpAllocateMfbRelatedBuffers(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    ANNOUNCE(&mTriggerAllocateMfbRelatedBuffers);
    WAIT(&mAllocateMfbRelatedBuffersDone);
    return MTRUE;
}

MBOOL MfllShot::mrpAllocateYuvMixingBuffer(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    ANNOUNCE(&mTriggerAllocateYuvMixing);
    WAIT(&mAllocateYuvMixingDone);
    return MTRUE;
}

MBOOL MfllShot::mrpAllocateJpegBuffer(void)
{
    if (mReduceMemoryUsage == eMrpMode_None)
        return MFALSE;

    ANNOUNCE(&mTriggerAllocateJpeg);
    WAIT(&mAllocateJpegDone);
    return MTRUE;
}


//------------------------------------------------------------------------------
// utility: capture
//------------------------------------------------------------------------------
MBOOL
MfllShot::
initInStartOne(SensorParam const & rSensorParam)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    //
    MFBLL_PIPE_INIT_INFO MfbllPipeInitInfo = {};
    MUINT32 exp = 0;
    MUINT32 iso = 0;
    MBOOL needUpdateAE = MFALSE;
    MUINT32 originalExp = 0;
    MUINT32 originalIso = 0;
    MUINT32 frameNumber = 1;

#if TEST_OFFLINE
    //trigger sensor
    MY_LOGD("NODETEST sensor list get\n");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MY_LOGD("NODETEST sensor search\n");
    pHalSensorList->searchSensors();
    MY_LOGD("NODETEST search sensor end\n");
#endif

    //
    mSensorParam = rSensorParam;

    //
    dumpSensorParam(mSensorParam);
    dumpShotParam(mShotParam);
    dumpJpegParam(mJpegParam);

    /*******************************************************************************
    * init buffer
    ********************************************************************************/
#if TEST_BITTURE_LDVT
    mRaw_Width = 4192;
    mRaw_Height = 3104;
    mShotParam.u4PictureWidth = 5280;
    mShotParam.u4PictureHeight = 960;
    mShotParam.u4PictureTransform = 0;
    mShotParam.u4PostViewTransform = 0;
#elif TEST_BITTURE
    mRaw_Width = 4192;
    mRaw_Height = 3104;
    mShotParam.u4PictureWidth = 3200;
    mShotParam.u4PictureHeight = 2400;
    mShotParam.u4PictureTransform = 0;
    mShotParam.u4PostViewTransform = 0;
#elif TEST_OFFLINE
    mRaw_Width = 4192;
    mRaw_Height = 3104;
    mShotParam.u4PictureWidth = 4192;
    mShotParam.u4PictureHeight = 3104;
    mShotParam.u4PictureTransform = 0;
    mShotParam.u4PostViewTransform = 0;
#else
    {
        NSCam::SensorStaticInfo sensorInfo;
        querySensorInfo(mSensorParam.u4OpenID, sensorInfo);
        mRaw_Width = sensorInfo.captureWidth;
        mRaw_Height = sensorInfo.captureHeight;
    }
#endif

    // sensor size
    MY_LOGD("sensor_Width=%d, sensor_Height=%d", mRaw_Width, mRaw_Height);
    MY_LOGD("JPEG_Width=%d, JPEG_Height=%d", mShotParam.u4PictureWidth, mShotParam.u4PictureHeight);
    // Raw buffer
    MY_LOGD("mRaw_Width=%d, mRaw_Height=%d", mRaw_Width, mRaw_Height);


    // Yuv buffer
    mYuv_Width = mRaw_Width;
    mYuv_Height = mRaw_Height;


    MY_LOGD("mYuv_Width=%d, mYuv_Height=%d", mYuv_Width, mYuv_Height);
    // YuvQ buffer
    mYuvQ_Width = mYuv_Width / 2;
    mYuvQ_Width &= (~0x01);
    mYuvQ_Height = mYuv_Height / 2;
    mYuvQ_Height &= (~0x01);
    MY_LOGD("mYuvQ_Width=%d, mYuvQ_Height=%d", mYuvQ_Width, mYuvQ_Height);
    // Raw -> Quyv factor
    mRawToQyuvFactor = (MFLOAT)mYuvQ_Width / mRaw_Width;
    MY_LOGD("mRawToQyuvFactor=%f", mRawToQyuvFactor);

    if( mShotParam.u4PictureTransform == eTransform_ROT_90 ||
            mShotParam.u4PictureTransform == eTransform_ROT_270 )
    {
        mRotPicWidth  = mShotParam.u4PictureHeight;
        mRotPicHeight = mShotParam.u4PictureWidth;
        mRotThuWidth  = mJpegParam.u4ThumbHeight;
        mRotThuHeight = mJpegParam.u4ThumbWidth;
    }
    else
    {
        mRotPicWidth  = mShotParam.u4PictureWidth;
        mRotPicHeight = mShotParam.u4PictureHeight;
        mRotThuWidth  = mJpegParam.u4ThumbWidth;
        mRotThuHeight = mJpegParam.u4ThumbHeight;
    }
    MY_LOGD("mShotParam.u4PictureTransform=%d", mShotParam.u4PictureTransform);
    MY_LOGD("mRotPicWidth=%d, mRotPicHeight=%d", mRotPicWidth, mRotPicHeight);
    MY_LOGD("mRotThuWidth=%d, mRotThuHeight=%d", mRotThuWidth, mRotThuHeight);


    mPostview_Width = mShotParam.u4PostViewWidth;
    mPostview_Height = mShotParam.u4PostViewHeight;


    /*******************************************************************************
    * init tuning
    ********************************************************************************/
    ret = ret && readTuningData(mTuningData, mSensorParam.u4OpenID);

#if TEST_ALGORITHM
    {
        // init mpMfllHal
        MfbllPipeInitInfo.input_width = mYuv_Width;
        MfbllPipeInitInfo.input_height = mYuv_Height;
        //MfbllPipeInitInfo.iso_value = rCap3AParam.u4RealISO;
        //MfbllPipeInitInfo.iso_high_th = mTuningData.mfll_iso_th;
        //MfbllPipeInitInfo.iso_low_th = mTuningData.mfll_iso_th;
        MfbllPipeInitInfo.max_frame_number = mTuningData.max_frame_number;
        MfbllPipeInitInfo.bss_clip_th = mTuningData.bss_clip_th;
        MfbllPipeInitInfo.memc_bad_mv_range = mTuningData.memc_bad_mv_range;
        //MfbllPipeInitInfo.memc_bad_mv_rate_th = mTuningData.memc_bad_mv_rate_th;
        MfbllPipeInitInfo.memc_bad_mv_rate_th = (mTuningData.memc_bad_mv_rate_th / 255.0) * ((mYuv_Width*mYuv_Height)>>8);

        //
        mpMfllHal = new MfbllHal();
        CHECK_OBJECT(mpMfllHal);
        mpMfllHal->init(&MfbllPipeInitInfo);

        {
            // read 3a data
            IHal3A *p3A = NULL;
            p3A = IHal3A::createInstance(IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
            if(p3A) {
                CaptureParam_T rCap3AParam;
                p3A->getCaptureParams(rCap3AParam);
                p3A->destroyInstance(LOG_TAG);
                originalExp = rCap3AParam.u4Eposuretime;
                originalIso = rCap3AParam.u4RealISO;
            }
        }

        ret = ret && MfllShot::getCaptureInfo(mMfbMode, mSensorParam.u4OpenID, frameNumber, exp, iso, needUpdateAE);
        if(mMfbMode == MFB_MODE_AIS) {
            ret = ret && MfllShot::updateAE(mSensorParam.u4OpenID, exp, iso);
        }
        mReferenceFrameLength = frameNumber - 1;
        MY_LOGD("mReferenceFrameLength(%d)", mReferenceFrameLength);
    }
#endif

    //debug
    if (!mIsSetDumpFileName)
        updateDumpFilename();
    //multi-pass NR (e.g. swnr, hwnr)
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("mediatek.mfll.nr", value, "-1");
        MINT32 nr = atoi(value);
        if(nr != -1) {
            muNRType = nr;
            MY_LOGD("force muNRType as %d", muNRType);
        }
    }


#if TEST_DBGINFO
    //exif
    {
        memset(&mDbgMfllInfo, 0, sizeof(mDbgMfllInfo));
        memset(&mDbgAisInfo, 0, sizeof(mDbgAisInfo));
        memset(&mDbgEisInfo, 0, sizeof(mDbgEisInfo));

        //[MFLL]
        mDbgMfllInfo.mfbMode = mMfbMode;
        mDbgMfllInfo.maxFrameNumber = MAX_CAPTURE_FRAMES;
        mDbgMfllInfo.processingNumber = frameNumber;
        mDbgMfllInfo.exposure = exp;
        mDbgMfllInfo.iso = iso;
        mDbgMfllInfo.iso_hi_th = mTuningData.mfll_iso_th;
        mDbgMfllInfo.iso_low_th = mTuningData.mfll_iso_th;
        mDbgMfllInfo.rawWidth = mRaw_Width;
        mDbgMfllInfo.rawHeight = mRaw_Height;

        mDbgMfllInfo.memc.width = MfbllPipeInitInfo.input_width;
        mDbgMfllInfo.memc.height = MfbllPipeInitInfo.input_height;
        mDbgMfllInfo.memc.inputFormat = 0;      //update this in doMemc()   //mMemoryQYuvB->getImgFormat();
        mDbgMfllInfo.memc.outputFormat = 0;     //update this in doMemc()   //mMemoryQYuvR[0]->getImgFormat();
        mDbgMfllInfo.memc.skip = 0;             //update this in doMemc()
        mDbgMfllInfo.memc.badMvRange = MfbllPipeInitInfo.memc_bad_mv_range;
        mDbgMfllInfo.memc.badMbCount[0] = 0;    //update this in doMemc()
        mDbgMfllInfo.memc.badMbCount[1] = 0;    //update this in doMemc()
        mDbgMfllInfo.memc.badMbCount[2] = 0;    //update this in doMemc()
        mDbgMfllInfo.memc.badMbTh = MfbllPipeInitInfo.memc_bad_mv_rate_th;

        //[AIS]
        mDbgAisInfo.exposure = originalExp;
        mDbgAisInfo.iso = originalIso;
        mDbgAisInfo.calculatedExposure = exp;
        mDbgAisInfo.calculatedIso = iso;
        mDbgAisInfo.exposure_th0 = mTuningData.reserved[0];
        mDbgAisInfo.iso_th0 = mTuningData.reserved[1];
        mDbgAisInfo.exposure_th1 = mTuningData.ais_exp_th;
        //[AIS->Advanced]
        mDbgAisInfo.advanced.enable = mTuningData.ais_advanced_tuning_en;
        mDbgAisInfo.advanced.maxExposureGain = mTuningData.ais_advanced_max_exposure;
        mDbgAisInfo.advanced.maxIsoGain = mTuningData.ais_advanced_max_iso;
    }
#endif

lbExit:
    FUNCTION_LOG_END;
    return  ret;
}


MBOOL
MfllShot::
captureDoneCallback(MVOID *user) {
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MfllShot* self = (MfllShot*)user;
    CHECK_OBJECT(self);
    self->ANNOUNCE(&self->mGraphDone);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotNotifyCb(MVOID* user, NSCamShot::CamShotNotifyInfo const msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MfllShot *self = reinterpret_cast <MfllShot*>(user);
    CHECK_OBJECT(self);

    ret = ret && self->fgCamShotNotifyCbImp(msg);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotNotifyCbImp(NSCamShot::CamShotNotifyInfo const& msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MY_LOGD("[fgCamShotNotifyCbImp] msgType(%d)", msg.msgType);
    ret = ret && handleNotifyCallback(msg.msgType, msg.ext1, msg.ext2);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotDataCb(MVOID* user, NSCamShot::CamShotDataInfo  const msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MfllShot *self = reinterpret_cast<MfllShot *>(user);
    CHECK_OBJECT(self);

    ret = ret && self->fgCamShotDataCbImp(msg);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotDataCbImp(NSCamShot::CamShotDataInfo  const& msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MY_LOGD("[fgCamShotDataCbImp] msgType(%d), mi4DataMsgSet(%d)", msg.msgType, mi4DataMsgSet);

    switch(msg.msgType) {
        case NSCamShot::ECamShot_DATA_MSG_RAW:
            MY_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_RAW");

            {   //keep raw in order
                mMemoryRawInCaptureOrder[mPass1Index] = msg.pBuffer;
                switch(mPass1Index) {
                    case 0:
                        mMemoryRawB = msg.pBuffer;
                        MY_LOGD("mMemoryRawB = %p", msg.pBuffer);
                        break;
                    default:
                        mMemoryRawR[mPass1Index-1] = msg.pBuffer;
                        MY_LOGD("mMemoryRawR[%d] = %p", mPass1Index-1, msg.pBuffer);
                }
            }

            {   //backup private data
                MUINT32 magicNum = 0;
                MBOOL isRrzo = MFALSE;
                MVOID* pPrivateData = NULL;
                MUINT32 privateDataSize = 0;

                IspSyncControl* pIspSyncCtrl = NULL;
                pIspSyncCtrl = IspSyncControl::createInstance(mSensorParam.u4OpenID);
                CHECK_OBJECT(pIspSyncCtrl);

                //
                pIspSyncCtrl->queryImgBufInfo(msg.pBuffer
                                            , magicNum
                                            , isRrzo
                                            , pPrivateData
                                            , privateDataSize
                                            );
                if(mPrivateDataSize == 0) {
                    mPrivateDataSize = privateDataSize;
                    mPrivateData = operator new(mPrivateDataSize);
                    memcpy(mPrivateData, pPrivateData, mPrivateDataSize);
                    MY_LOGD("mPrivateDataSize = %d", mPrivateDataSize);
                }

                pIspSyncCtrl->destroyInstance();
            }

            ++mPass1Index;
            break;

        case NSCamShot::ECamShot_DATA_MSG_YUV:
            MY_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_YUV");

            {   //dbginfo
                if(mPass2Index == 0) {
                    IDbgInfoContainer* pDbgInfo = reinterpret_cast<IDbgInfoContainer*>(msg.ext2);
                    pDbgInfo->copyTo(mDebugInfo);
                }
            }
            ANNOUNCE(&mCapP2Ready[mPass2Index]);
            ++mPass2Index;

            #if 0
            if(mPass2Index == mReferenceFrameLength+1){
                // quick captrue to preview
                IHal3A *p3A = NULL;
                p3A = IHal3A::createInstance(IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
                if(p3A) {
                    CaptureParam_T rCap3AParam;
                    p3A->getCaptureParams(rCap3AParam);
                    MY_LOGD("current exp(%d)iso(%d))",rCap3AParam.u4Eposuretime,rCap3AParam.u4RealISO);
                    rCap3AParam.u4Eposuretime = 33000;
                    MY_LOGD("force exp to (%d)",rCap3AParam.u4Eposuretime);
                    p3A->send3ACtrl(E3ACtrl_SetSensorDirectly, (MINTPTR) &rCap3AParam, MFALSE);
                    p3A->destroyInstance(LOG_TAG);
                }
            }
            #endif
            break;

        case NSCamShot::ECamShot_DATA_MSG_EIS:
            MY_LOGD("[fgCamShotDataCb] ECamShot_DATA_MSG_EIS");
            if(mEisId > 0) {
                EIS_STATISTIC_STRUCT* pEisStat = (EIS_STATISTIC_STRUCT*)msg.ext1;
                mEisStat[mEisId-1] = *pEisStat;
            }
            ++mEisId;
            break;
    }

    ret = ret && handleDataCallback(msg.msgType, msg.ext1, msg.ext2, msg.pBuffer);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotDataCbFor15Raw(MVOID* user, NSCamShot::CamShotDataInfo  const msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MfllShot *self = reinterpret_cast<MfllShot *>(user);
    CHECK_OBJECT(self);

    ret = ret && self->fgCamShotDataCbImpFor15Raw(msg);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
fgCamShotDataCbImpFor15Raw(NSCamShot::CamShotDataInfo  const& msg)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    MY_LOGD("[fgCamShotDataCbImpFor15Raw] msgType(%d), mi4DataMsgSet(%d)", msg.msgType, mi4DataMsgSet);

    switch(msg.msgType) {
        case NSCamShot::ECamShot_DATA_MSG_RAW:
            MY_LOGD("[fgCamShotDataCbImpFor15Raw] ECamShot_DATA_MSG_RAW");
            //keep raw in order
            mMemoryRawInCaptureOrderFor15Raw[mPass1Index] = msg.pBuffer;
            ++mPass1Index;
            break;
    }

    //ret = ret && handleDataCallback(msg.msgType, msg.ext1, msg.ext2, msg.pBuffer);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doBss()
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MUINT32 roiRatio = 95;
    MUINT32 roiW = mYuvQ_Width  * roiRatio / 100;
    MUINT32 roiH = mYuvQ_Height * roiRatio / 100;
    MUINT32 roiX = mYuvQ_Width * (100-roiRatio) / 100 / 2;
    MUINT32 roiY = mYuvQ_Height * (100-roiRatio) / 100 / 2;

    //1. config
    MF_BSS_PARAM_STRUCT param = {
        MF_BSS_ON                    : mEnableBss,
        MF_BSS_ROI_WIDTH             : roiW,    //1520
        MF_BSS_ROI_HEIGHT            : roiH,    //900
        MF_BSS_ROI_X0                : roiX,    //40
        MF_BSS_ROI_Y0                : roiY,    //150
        MF_BSS_SCALE_FACTOR          : CUST_MFLL_BSS_SCALE_FACTOR,
        MF_BSS_CLIP_TH0              : CUST_MFLL_BSS_CLIP_TH0,
        MF_BSS_CLIP_TH1              : CUST_MFLL_BSS_CLIP_TH1,
        MF_BSS_ZERO                  : CUST_MFLL_BSS_ZERO,
    };

    BSS_INPUT_DATA input;
    input.inMEWidth = mYuvQ_Width;
    input.inMEHeight = mYuvQ_Height;
    input.Proc1QBImg = (UINT8*)mMemoryQYuvB->getBufVA(0);
    input.Proc1QR1Img = (UINT8*)mMemoryQYuvR[0]->getBufVA(0);
    input.Proc1QR2Img = (UINT8*)mMemoryQYuvR[1]->getBufVA(0);
    input.Proc1QR3Img = (UINT8*)mMemoryQYuvR[2]->getBufVA(0);
    for(MUINT32 i=0; i<4; i++) {    //@todo don't use magic number 4
        if(i!=0) {
            MINT32 gmvX;
            MINT32 gmvY;
            MUINT32 ref = i - 1;
            input.gmv[i].x = mGmv[ref][0] * mRawToQyuvFactor;
            input.gmv[i].y = mGmv[ref][1] * mRawToQyuvFactor;
        } else {
            input.gmv[i].x = 0;
            input.gmv[i].y = 0;
        }
    }

    BSS_OUTPUT_DATA output;

    MY_LOGD("[bss] before raw(%p,%p,%p,%p), yuv(%p,%p,%p,%p), qyuv_gmv([%d,%d],[%d,%d],[%d,%d])"
            // raw
            , (UINT8*)mMemoryRawB->getBufVA(0)
            , (UINT8*)mMemoryRawR[0]->getBufVA(0)
            , (UINT8*)mMemoryRawR[1]->getBufVA(0)
            , (UINT8*)mMemoryRawR[2]->getBufVA(0)
            // yuv
            , (UINT8*)mMemoryQYuvB->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[0]->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[1]->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[2]->getBufVA(0)
            // gmv
            , input.gmv[1].x, input.gmv[1].y
            , input.gmv[2].x, input.gmv[2].y
            , input.gmv[3].x, input.gmv[3].y
            );

    //2. do bss
    BestShotSelection *bss = BestShotSelection::createInstance();
    bss->init(param);
    bss->sort(&input, &output);
    bss->destroyInstance();

    #if 1
    //3. reorder image
    //raw
    mMemoryRawB = mMemoryRawInCaptureOrder[output.originalOrder[0]];
    mMemoryRawR[0] = mMemoryRawInCaptureOrder[output.originalOrder[1]];
    mMemoryRawR[1] = mMemoryRawInCaptureOrder[output.originalOrder[2]];
    mMemoryRawR[2] = mMemoryRawInCaptureOrder[output.originalOrder[3]];

    if(meShotMode == eShotMode_ZsdMfllShot) {
        /* If using ZSD shot, also re-order capture RAW buffers */
        mMemoryCapBufRaw[0] = mMemoryRawB.get();
        for (int i = 0; i < mReferenceFrameLength; i++)
            mMemoryCapBufRaw[i + 1] = mMemoryRawR[i].get();
    }

    //yuv
    sp<IImageBuffer>    rMemoryQYuvInCaptureOrder[MAX_REFERENCE_FRAMES+1];
    rMemoryQYuvInCaptureOrder[0] = mMemoryQYuvB;
    rMemoryQYuvInCaptureOrder[1] = mMemoryQYuvR[0];
    rMemoryQYuvInCaptureOrder[2] = mMemoryQYuvR[1];
    rMemoryQYuvInCaptureOrder[3] = mMemoryQYuvR[2];
    mMemoryQYuvB = rMemoryQYuvInCaptureOrder[output.originalOrder[0]];
    mMemoryQYuvR[0] = rMemoryQYuvInCaptureOrder[output.originalOrder[1]];
    mMemoryQYuvR[1] = rMemoryQYuvInCaptureOrder[output.originalOrder[2]];
    mMemoryQYuvR[2] = rMemoryQYuvInCaptureOrder[output.originalOrder[3]];

    //4. reorder eis statistics
    mGmv[0][0] = output.gmv[1].x / mRawToQyuvFactor;
    mGmv[0][1] = output.gmv[1].y / mRawToQyuvFactor;
    mGmv[1][0] = output.gmv[2].x / mRawToQyuvFactor;
    mGmv[1][1] = output.gmv[2].y / mRawToQyuvFactor;
    mGmv[2][0] = output.gmv[3].x / mRawToQyuvFactor;
    mGmv[2][1] = output.gmv[3].y / mRawToQyuvFactor;

    MY_LOGD("[bss] after raw(%p,%p,%p,%p), yuv(%p,%p,%p,%p), qyuv_gmv([%d,%d],[%d,%d],[%d,%d])"
            // raw
            , (UINT8*)mMemoryRawB->getBufVA(0)
            , (UINT8*)mMemoryRawR[0]->getBufVA(0)
            , (UINT8*)mMemoryRawR[1]->getBufVA(0)
            , (UINT8*)mMemoryRawR[2]->getBufVA(0)
            // yuv
            , (UINT8*)mMemoryQYuvB->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[0]->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[1]->getBufVA(0)
            , (UINT8*)mMemoryQYuvR[2]->getBufVA(0)
            // gmv
            , output.gmv[1].x, output.gmv[1].y
            , output.gmv[2].x, output.gmv[2].y
            , output.gmv[3].x, output.gmv[3].y
            );

    #endif

    //5. dbginfo
    mDbgMfllInfo.bss.enable = param.MF_BSS_ON;
    mDbgMfllInfo.bss.roiWidth = param.MF_BSS_ON;
    mDbgMfllInfo.bss.roiHeight = param.MF_BSS_ON;
    mDbgMfllInfo.bss.scaleFactor = param.MF_BSS_ON;
    mDbgMfllInfo.bss.clipTh0 = param.MF_BSS_ON;
    mDbgMfllInfo.bss.clipTh1 = param.MF_BSS_ON;
    mDbgMfllInfo.bss.inputSequenceAfterBss[0] = output.originalOrder[0];
    mDbgMfllInfo.bss.inputSequenceAfterBss[1] = output.originalOrder[1];
    mDbgMfllInfo.bss.inputSequenceAfterBss[2] = output.originalOrder[2];
    mDbgMfllInfo.bss.inputSequenceAfterBss[3] = output.originalOrder[3];

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doCapture15Raw()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    ShotParam burstShotParam;

    const MUINT32   frames = 15;
    sp<IImageBuffer>    mMemoryRawEx[frames];
    sp<IImageBuffer>    mMemoryQYuvR[frames];
    IImageBufferHeap *pHeap;

#if TEST_CAPTURE
    //
    mpBurstShot = IBurstShot::createInstance(static_cast<EShotMode>(meShotMode), "MfllShot");
    CHECK_OBJECT(mpBurstShot);
    mpBurstShot->init();

    // set isp profile
    if (eShotMode_ZsdMfllShot == meShotMode){
        mpBurstShot->setIspProfile(muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_VSS_MFB_Capture_EE_Off_SWNR : EIspProfile_VSS_MFB_Capture_EE_Off);
    } else {
        mpBurstShot->setIspProfile(muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_MFB_Capture_EE_Off_SWNR : EIspProfile_MFB_Capture_EE_Off);
    }

    //register memory
    for(MUINT32 ref=0; ref<frames; ref++) {
        ret = ret && allocImageBuffer(&pHeap, &mMemoryRawEx[ref], mRaw_Width, mRaw_Height, eImgFmt_BAYER10);
        ret = ret && mpBurstShot->registerImageBuffer(ECamShot_BUF_TYPE_RAW, mMemoryRawEx[ref].get());
        //ret = ret && allocImageBuffer(&pHeap, &mMemoryQYuvEx[ref], mYuvQ_Width, mYuvQ_Height, eImgFmt_YUY2);
        //ret = ret && mpBurstShot->registerImageBuffer(ECamShot_BUF_TYPE_YUV, mMemoryQYuvEx[ref].get());
    }

    // notify
    mpBurstShot->enableNotifyMsg( NSCamShot::ECamShot_NOTIFY_MSG_EOF );
    mpBurstShot->enableDataMsg(NSCamShot::ECamShot_DATA_MSG_RAW
                                //| NSCamShot::ECamShot_DATA_MSG_YUV
                                | NSCamShot::ECamShot_DATA_MSG_EIS
                                );
    //mpBurstShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    mpBurstShot->setCallbacks(NULL, fgCamShotDataCbFor15Raw, this);

    // execute transform / zoom at doMixing()
    burstShotParam = mShotParam;
    burstShotParam.u4PictureTransform = 0;
    burstShotParam.u4ZoomRatio = 100;
    //
    ret = ret && mpBurstShot->setShotParam(burstShotParam);
    ret = ret && mpBurstShot->setJpegParam(mJpegParam);
    ret = ret && mpBurstShot->setPrvBufHdl((MVOID*)mpPrvBufHandler);
    CHECK_RET("fail before pBurstShot->start");
    ret = ret && mpBurstShot->start(mSensorParam, frames);
    CHECK_RET("fail in pBurstShot->start");
    //
    ret = ret && mpBurstShot->uninit();
    mpBurstShot->destroyInstance();
    //
    mPass1Index = 0;

    #if 1
    if(mDump & MFLL_DUMP_RAW) {
        //pass1
        char filename[128] = {0};
        NSCam::SensorStaticInfo sensorInfo;
        querySensorInfo(mSensorParam.u4OpenID, sensorInfo);

        MUINT32 sensorbit = 10;
        switch(sensorInfo.rawSensorBit) {
            case RAW_SENSOR_8BIT:  sensorbit = 8; break;
            case RAW_SENSOR_10BIT: sensorbit = 10; break;
            case RAW_SENSOR_12BIT: sensorbit = 12; break;
            case RAW_SENSOR_14BIT: sensorbit = 14; break;
        }

        //
        //IMG_20140101_003010_exp0066_iso0000_0__4192x3104_10_0.raw //use CTO naming role
        #define UPDATE_RAW_FILENAME(id) \
        snprintf(filename, sizeof(filename), "%s_%02d_exp%04d_iso%04d_%d__%dx%d_%d_%d.raw"     \
            , mDumpFilename                                                 \
            , 0                                                             \
            , mDbgAisInfo.calculatedExposure                                \
            , mDbgAisInfo.calculatedIso                                     \
            , id                                                            \
            , mRaw_Width                                                    \
            , mRaw_Height                                                   \
            , sensorbit                                                     \
            , sensorInfo.sensorFormatOrder                                  \
            )
        for(MUINT32 i=0; i<15; i++) {
            UPDATE_RAW_FILENAME(i);
            MY_LOGD("flowCapture saveToFile %s", filename);
            mMemoryRawInCaptureOrderFor15Raw[i]->saveToFile(filename);
            mMemoryRawInCaptureOrderFor15Raw[i] = NULL; //release memory
        }
        #undef UPDATE_RAW_FILENAME
    }
    #endif
#else
    msleep(100);
#endif

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: blending
//------------------------------------------------------------------------------
MBOOL
MfllShot::
doMemc(MUINT32 ref)
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;
    MY_LOGD("doMemc(%d) start", ref);

    MUINT8* baseBufAddr;
    MUINT8* refBufAddr;
    MUINT32 badMvCount;

    MINT32  gmvX = 0;
    MINT32  gmvY = 0;
#if TEST_EIS
    //ret = ret && doEis(ref, gmvX, gmvY);
    gmvX = mGmv[ref][0] * mRawToQyuvFactor;
    gmvY = mGmv[ref][1] * mRawToQyuvFactor;
#endif

#if TEST_MEMC
    //
    WAIT(&mMemoryReady_Memc[ref]);

    ret = ret && mpMfllHal->setWorkingBuffer((MUINT8*)mMemoryLocalMotion->getBufVA(0));
    CHECK_RET("setWorkingBuffer fail");
    //
    baseBufAddr = (MUINT8*)mMemoryQYuvB->getBufVA(0);
    refBufAddr = (MUINT8*)mMemoryQYuvR[ref]->getBufVA(0);
    CHECK_OBJECT(baseBufAddr);
    CHECK_OBJECT(refBufAddr);
    //
    if(ref == 0) {
        ret = ret && mMemoryQYuvB->syncCache(eCACHECTRL_INVALID);  //hw->cpu
        CHECK_RET("flush cache hw->cpu fail");
    }
    ret = ret && mMemoryQYuvR[ref]->syncCache(eCACHECTRL_INVALID);  //hw->cpu
    CHECK_RET("flush cache hw->cpu fail");
    if(ret) {
        MBOOL isMemcSuccessed = mpMfllHal->doMemc(gmvX, gmvY, baseBufAddr, refBufAddr, &badMvCount);
        if(!isMemcSuccessed) {
            mSkipMfb = MTRUE;
            MY_LOGD("singleout - doMemc[%d] mSkipMfb(%d)", ref, mSkipMfb);
        }
    }
    ret = ret && mMemoryQYuvR[ref]->syncCache(eCACHECTRL_FLUSH);    //cpu->hw
    CHECK_RET("flush cache cpu->hw fail");

    if(mDump & MFLL_DUMP_YUV) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH];
    #if TEST_MEMC_YUY2
        snprintf(filename, sizeof(filename), "%s_%02d_memc%d__%04dx%04d.yuy2", mDumpFilename, 3, ref, mYuvQ_Width, mYuvQ_Height);
    #else
        snprintf(filename, sizeof(filename), "%s_%02d_memc%d__%04dx%04d.yv16", mDumpFilename, 3, ref, mYuvQ_Width, mYuvQ_Height);
    #endif
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryQYuvR[ref]->saveToFile(filename);
    }
#else
    msleep(200);
#endif

    //dbginfo
    if(mSkipMfb) {
        mDbgMfllInfo.memc.skip = MTRUE;
    }

    /* Only get format once, or sometimes occurrs NULL access exception */
    if (mDbgMfllInfo.memc.inputFormat == 0) {
        MY_LOGD("%s: set dbg info with idx(%d)", __FUNCTION__, ref);
        mDbgMfllInfo.memc.inputFormat = mMemoryQYuvB->getImgFormat();
        mDbgMfllInfo.memc.outputFormat = mMemoryQYuvR[0]->getImgFormat();
    }

    mDbgMfllInfo.memc.badMbCount[ref] = badMvCount;

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doEis(MUINT32 ref, MINT32 &gmvX, MINT32 &gmvY)
{
    MFLL_TRACE_CALL();

    MBOOL   ret = MTRUE;
    FUNCTION_LOG_START;

    EIS_DATA_STRUCT motion_data;
    for(MUINT32 j=0; j<EIS_MAX_WIN_NUM; j++)
    {
        motion_data.eis_data1[2*j]  = mEisStat[ref].i4LMV_X[j];
        motion_data.eis_data1[2*j+1]= mEisStat[ref].i4LMV_Y[j];
        motion_data.eis_data2[2*j]    = mEisStat[ref].NewTrust_X[j];
        motion_data.eis_data2[2*j+1]= mEisStat[ref].NewTrust_Y[j];
        MY_LOGD("win(%d) lmv(%d,%d) trust(%d,%d)"
                , j
                , motion_data.eis_data1[2*j]
                , motion_data.eis_data1[2*j+1]
                , motion_data.eis_data2[2*j]
                , motion_data.eis_data2[2*j+1]
                );
    }
    gmvX = 0;
    gmvY = 0;

    ////////////////////////////////////////////////////////////////////////////////
    // copy from generate_EIS_GMV()
    //
    MINT32  eis_trust_th  = CUST_MFLL_EIS_TRUST_TH;
    MINT32  eis_op_step_h = CUST_MFLL_EIS_OP_STEP_H;
    MINT32  eis_op_step_v = CUST_MFLL_EIS_OP_STEP_V;

    MY_LOGD("eis_trush_th(%d)", CUST_MFLL_EIS_TRUST_TH);
    MY_LOGD("CUST_MFLL_EIS_OP_STEP_H(%d)", CUST_MFLL_EIS_OP_STEP_H);
    MY_LOGD("CUST_MFLL_EIS_OP_STEP_V(%d)", CUST_MFLL_EIS_OP_STEP_V);

    if(ref==0) {
        mAccGmvX = 0;
        mAccGmvY = 0;
    }
    motion_data.eis_data3[0] = 0;
    motion_data.eis_data3[1] = 0;

    //for(int i = 1; i < FrameNum; i++)    //for each frame
    {
        MINT32  sum_x_tmp = 0;
        MINT32  sum_y_tmp = 0;
        MINT32  cnt_x_tmp = 0;
        MINT32  cnt_y_tmp = 0;

        for( int j = 0; j < EIS_MAX_WIN_NUM; j++ )    //for each bin
        {
            MY_LOGD("mv(%d,%d), trust(%d,%d)"
                    , motion_data.eis_data1[2*j]
                    , motion_data.eis_data1[2*j+1]
                    , motion_data.eis_data2[2*j]
                    , motion_data.eis_data2[2*j+1]
                    );
            if( motion_data.eis_data2[2*j] > eis_trust_th )    //if confidence x > confidence threahold
            {
                sum_x_tmp += motion_data.eis_data1[2*j];        //calculate sum of MVX
                cnt_x_tmp++;
            }
            if( motion_data.eis_data2[2*j+1] > eis_trust_th ) //if confidence y > confidence threahold
            {
                sum_y_tmp += motion_data.eis_data1[2*j+1];    //calculate sum of MVY
                cnt_y_tmp++;
            }
        }
        MY_LOGD("sum(%d,%d), cnt(%d,%d)", sum_x_tmp, sum_y_tmp, cnt_x_tmp, cnt_y_tmp);

        if( cnt_x_tmp == 0 )
            sum_x_tmp = 0;
        else
            sum_x_tmp = sum_x_tmp * eis_op_step_h / cnt_x_tmp;

        if( cnt_y_tmp == 0 )
            sum_y_tmp = 0;
        else
            sum_y_tmp = sum_y_tmp * eis_op_step_v / cnt_y_tmp;

        mAccGmvX += (MFLOAT)sum_x_tmp/256;
        mAccGmvY += (MFLOAT)sum_y_tmp/256;
        MY_LOGD("acc(%f,%f)", mAccGmvX, mAccGmvY);

        motion_data.eis_data3[0] = (MINT32)((mAccGmvX>=0)?(mAccGmvX + 0.5):(mAccGmvX - 0.5));
        motion_data.eis_data3[1] = (MINT32)((mAccGmvY>=0)?(mAccGmvY + 0.5):(mAccGmvY - 0.5));
    }
    // copy from generate_EIS_GMV()
    ////////////////////////////////////////////////////////////////////////////////

    gmvX = motion_data.eis_data3[0];
    gmvY = motion_data.eis_data3[1];
    MY_LOGD("gmv ref(%d) calc(%d,%d) final(%d,%d)", ref, motion_data.eis_data3[0], motion_data.eis_data3[1], gmvX, gmvY);

#if TEST_DBGINFO
    mDbgEisInfo[ref].gmvX = gmvX;
    mDbgEisInfo[ref].gmvY = gmvY;
    for(MUINT32 win=0; win<EIS_MAX_WIN_NUM; win++) {
        mDbgEisInfo[ref].mvX[win] = mEisStat[ref].i4LMV_X[win];
        mDbgEisInfo[ref].mvY[win] = mEisStat[ref].i4LMV_Y[win];
        mDbgEisInfo[ref].trustX[win] = mEisStat[ref].NewTrust_X[win];
        mDbgEisInfo[ref].trustY[win] = mEisStat[ref].NewTrust_Y[win];
    }
#endif

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doMfb(MUINT32 ref)
{
    MFLL_TRACE_CALL();
    EIspProfile_T profile;

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mMemoryReady_Mfb[ref]);

#if TEST_MFB
    // buffers
    sp<IImageBuffer> base;
    sp<IImageBuffer> reference;
    sp<IImageBuffer> weighting_in;
    sp<IImageBuffer> weighting_out;
    sp<IImageBuffer> blended;
    // input ports
    NSCam::NSIoPipe::NSPostProc::Input minput_imgi;     //base frame
    NSCam::NSIoPipe::NSPostProc::Input minput_vipi;     //reference frame
    NSCam::NSIoPipe::NSPostProc::Input minput_vip3i;    //weighting map
    // output ports
    NSCam::NSIoPipe::NSPostProc::Output moutput_mfbo;   //weighting map
    NSCam::NSIoPipe::NSPostProc::Output moutput_img3o;  //output frame
    //
    IFeatureStream* mpIMfbBlending = NULL;
    IHal3A *p3A = NULL;
    //
    QParams params;
    QParams dequeParams;
    const MINT64 timeout = 5000000000;  //5s,unit is nsec

    MY_LOGD("doMfb(ref=%d)", ref);

#if TEST_3A
    // update 3A profile
    p3A = IHal3A::createInstance(IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
    if(!p3A) {
        MY_LOGE("IHal3A::createInstance fail");
        ret = MFALSE;
        goto lbExit;
    }

    {

        if (eShotMode_ZsdMfllShot == meShotMode){
            profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_VSS_MFB_Blending_All_Off_SWNR : EIspProfile_VSS_MFB_Blending_All_Off);
        } else {
            profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_MFB_Blending_All_Off_SWNR : EIspProfile_MFB_Blending_All_Off);
        }

        ParamIspProfile_T _3A_profile(profile
                                    , 0  //magicNum
                                    , MFALSE
                                    , ParamIspProfile_T::EParamValidate_P2Only
                                    );
        p3A->setIspProfile(_3A_profile);
    }
    p3A->destroyInstance(LOG_TAG);
#endif

    // create MFB
    if (eShotMode_ZsdMfllShot == meShotMode){
        mpIMfbBlending = IFeatureStream::createInstance("MFB_Blending", EFeatureStreamTag_MFB_Bld_Vss, mSensorParam.u4OpenID);
    } else {
        mpIMfbBlending = IFeatureStream::createInstance("MFB_Blending", EFeatureStreamTag_MFB_Bld, mSensorParam.u4OpenID);
    }
    CHECK_OBJECT(mpIMfbBlending);

    // init MFB
    ret = ret && mpIMfbBlending->init();
    CHECK_RET("mpIMfbBlending init fail");

    // select memory buffer
    if (ref%2 == 0) {
            base = mMemoryYuvB;
            blended = mMemoryYuv;
    }
    else {
            base = mMemoryYuv;
            blended = mMemoryYuvB;
    }
    mPtrYuvBase = base;
    mPtrYuvBlended = blended;
    reference = mMemoryQYuvR[ref];
    weighting_in = mMemoryWeightingMap[ref%2];
    weighting_out = mMemoryWeightingMap[(ref+1)%2];
    mMemoryFinalWeightingMapIndex = (ref+1)%2;

    #if TEST_BITTURE_LDVT
    switch(ref) {
        case 0:
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB1/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB1/dump_mc1.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
        case 1:
            //imgi
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB2/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            //vipi
            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB2/dump_mc2.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            //vip3i
            weighting_in->unlockBuf(LOG_TAG);
            weighting_in->loadFromFile("/sdcard/input/MFB2/BLD_W.dat_3200x2400.y");
            weighting_in->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
        case 2:
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB3/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB3/dump_mc3.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            weighting_in->unlockBuf(LOG_TAG);
            weighting_in->loadFromFile("/sdcard/input/MFB3/BLD_W.yuv_3200x2400.y");
            weighting_in->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
    }
    #elif TEST_BITTURE
    switch(ref) {
        case 0:
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB1/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB1/dump_mc1.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
        case 1:
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB2/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB2/dump_mc2.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            weighting_in->unlockBuf(LOG_TAG);
            weighting_in->loadFromFile("/sdcard/input/MFB2/BLD_W.dat_3200x2400.y");
            weighting_in->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
        case 2:
            base->unlockBuf(LOG_TAG);
            base->loadFromFile("/sdcard/input/MFB3/dip_base.yuv_3200x2400.yuy2");
            base->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            reference->unlockBuf(LOG_TAG);
            reference->loadFromFile("/sdcard/input/MFB3/dump_mc3.yuv_1600x1200.yuy2");
            reference->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

            weighting_in->unlockBuf(LOG_TAG);
            weighting_in->loadFromFile("/sdcard/input/MFB3/BLD_W.yuv_3200x2400.y");
            weighting_in->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
            break;
    }
    #endif

    if(mDump & MFLL_DUMP_MFB) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        // base
        snprintf(filename, sizeof(filename), "%s_%02d_mfb%d_input_base__%04dx%04d.yuy2", mDumpFilename, 4, ref, mYuv_Width, mYuv_Height);
        MY_LOGD("doMfb saveToFile %s", filename);
        base->saveToFile(filename);
        // ref
        snprintf(filename, sizeof(filename), "%s_%02d_mfb%d_input_ref__%04dx%04d.yuy2", mDumpFilename, 4, ref, mYuvQ_Width, mYuvQ_Height);
        MY_LOGD("doMfb saveToFile %s", filename);
        reference->saveToFile(filename);
        // input weighting
        snprintf(filename, sizeof(filename), "%s_%02d_mfb%d_input_weight__%04dx%04d.y", mDumpFilename, 4, ref, mYuv_Width, mYuv_Height);
        MY_LOGD("doMfb saveToFile %s", filename);
        weighting_in->saveToFile(filename);
    }

    // input ports
    params.mvIn.clear();

    // - imgi = yuv base frame
    minput_imgi.mBuffer = base.get();
    minput_imgi.mPortID.inout = 0; //in
    minput_imgi.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMGI;
    minput_imgi.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    minput_imgi.mCropRect.p_integral.x = 0;
    minput_imgi.mCropRect.p_integral.y = 0;
    minput_imgi.mCropRect.p_fractional.x = 0;
    minput_imgi.mCropRect.p_fractional.y = 0;
    minput_imgi.mCropRect.s.w = mYuv_Width;
    minput_imgi.mCropRect.s.h = mYuv_Height;
    params.mvIn.push_back(minput_imgi);
    MY_LOGD("minput_imgi.mBuffer = %p", minput_imgi.mBuffer);

    // - vipi = reference frame
    minput_vipi.mBuffer = reference.get();
    minput_vipi.mPortID.inout = 0; //in
    minput_vipi.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_VIPI;
    minput_vipi.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    minput_vipi.mCropRect.p_integral.x = 0;
    minput_vipi.mCropRect.p_integral.y = 0;
    minput_vipi.mCropRect.p_fractional.x = 0;
    minput_vipi.mCropRect.p_fractional.y = 0;
    minput_vipi.mCropRect.s.w = mYuvQ_Width;
    minput_vipi.mCropRect.s.h = mYuvQ_Height;
    params.mvIn.push_back(minput_vipi);
    MY_LOGD("minput_vipi.mBuffer = %p", minput_vipi.mBuffer);

    // - vip3i = weighting map
    if(ref > 0) {
        minput_vip3i.mBuffer = weighting_in.get();
        minput_vip3i.mPortID.inout = 0; //in
        minput_vip3i.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_VIP3I;
        minput_vip3i.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
        minput_vip3i.mCropRect.p_integral.x = 0;
        minput_vip3i.mCropRect.p_integral.y = 0;
        minput_vip3i.mCropRect.p_fractional.x = 0;
        minput_vip3i.mCropRect.p_fractional.y = 0;
        minput_vip3i.mCropRect.s.w = mYuv_Width;
        minput_vip3i.mCropRect.s.h = mYuv_Height;
        params.mvIn.push_back(minput_vip3i);
        MY_LOGD("minput_vip3i.mBuffer = %p", minput_vip3i.mBuffer);
    }


    // output ports
    params.mvOut.clear();

    // - mfbo = weighting map
    moutput_mfbo.mBuffer = weighting_out.get();
    moutput_mfbo.mPortID.inout = 1; //out
    moutput_mfbo.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_MFBO;
    moutput_mfbo.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvOut.push_back(moutput_mfbo);
    MY_LOGD("moutput_mfbo.mBuffer = %p", moutput_mfbo.mBuffer);

    // - img3o = blending output
    moutput_img3o.mBuffer = blended.get();
    moutput_img3o.mPortID.inout = 1; //out
    moutput_img3o.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG3O;
    moutput_img3o.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvOut.push_back(moutput_img3o);
    MY_LOGD("moutput_img3o.mBuffer = %p", moutput_img3o.mBuffer);

    // enqueue
    //params.mFrameNo = ref;
    params.mFrameNo = 0;
    mpIMfbBlending->enque(params);

    // dequeue
    dequeParams.mFrameNo = ref;
    ret = ret && mpIMfbBlending->deque(dequeParams, timeout);
    if(!ret) {
        MY_LOGE("deque fail, frameNo(0x%x)", dequeParams.mFrameNo);
        MY_LOGE("[MFB BLD] deque magicNum(%d)", dequeParams.mFrameNo);
        MY_LOGE("[MFB BLD] deque mvIn.size(%lu), mvOut.size(%lu)"
                , dequeParams.mvIn.size()
                , dequeParams.mvOut.size());
        goto lbExit;
    }

#if TEST_DBGINFO
    if(ref==0) {
        updateRegToDebugInfo(MF_REGTABLE_IN_MFB);
    }
#endif

    if(mDump & MFLL_DUMP_MFB) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH];
        // output weighting
        snprintf(filename, sizeof(filename), "%s_%02d_mfb%d_output_weight__%04dx%04d.y", mDumpFilename, 4, ref, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        weighting_out->saveToFile(filename);
        // mfb output
        snprintf(filename, sizeof(filename), "%s_%02d_mfb%d_output_mfb__%04dx%04d.yuy2", mDumpFilename, 4, ref, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        blended->saveToFile(filename);
    }

lbExit:
    if(mpIMfbBlending) {
        mpIMfbBlending->uninit();
        mpIMfbBlending->destroyInstance("MFB_Blending");
    }
#else
    msleep(30);
#endif //TEST_MFB

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doEncodeYuvB()
{
    MFLL_TRACE_CALL();
    EIspProfile_T profile;

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mMemoryReady_Pass1);
    WAIT(&mMemoryReady_YuvB);
    if (eShotMode_ZsdMfllShot == meShotMode){
        profile = EIspProfile_VSS_MFB_PostProc_EE_Off;
    } else {
        profile = EIspProfile_MFB_PostProc_EE_Off;
    }
    ret = ret && encodeRawToYuv(mMemoryRawB
                                , mMemoryYuvB
                                , profile
                                , mPrivateData);

    if(mDump & MFLL_DUMP_YUV) {
        //pass2
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        //
        snprintf(filename, sizeof(filename), "%s_%02d_YuvB_%04dx%04d.yuy2", mDumpFilename, 2, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryYuvB->saveToFile(filename);
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doEncodeNormalYuvB()
{
    MFLL_TRACE_CALL();
    EIspProfile_T profile;

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    WAIT(&mMemoryReady_Pass1);
    WAIT(&mMemoryReady_YuvB);

    if (eShotMode_ZsdMfllShot == meShotMode){
        profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_VSS_MFB_PostProc_ANR_EE_SWNR : EIspProfile_VSS_MFB_PostProc_ANR_EE);
    } else {
        profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_MFB_PostProc_ANR_EE_SWNR : EIspProfile_MFB_PostProc_ANR_EE);
    }
    ret = ret && encodeRawToYuv(mMemoryRawB
                                , mPtrYuvGolden
                                , profile
                                , mPrivateData
                                );
#if TEST_DBGINFO
    updateRegToDebugInfo(MF_REGTABLE_BASE);
#endif

    if(mDump & MFLL_DUMP_YUV) {
        //pass2
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        //
        snprintf(filename, sizeof(filename), "%s_%02d_NormalYuvB_%04dx%04d.yuy2", mDumpFilename, 5, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mPtrYuvGolden->saveToFile(filename);
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
encodeRawToYuv(sp<IImageBuffer> &rawBuffer, sp<IImageBuffer> &yuvBuffer, EIspProfile_T profile, void* privateData)
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    //
    NSCam::NSIoPipe::NSPostProc::Input minput;
    NSCam::NSIoPipe::NSPostProc::Output moutput;
    QParams params;
    QParams dequeParams;
    //
    INormalStream* mpINormalStream = NULL;
    IHal3A *p3A = NULL;
    //
    MSize srcSize = rawBuffer->getImgSize();
    MSize dstSize = yuvBuffer->getImgSize();
    NSCamHW::Rect srcRect(0, 0, srcSize.w, srcSize.h);
    NSCamHW::Rect dstRect(0, 0, dstSize.w, dstSize.h);
    NSCamHW::Rect cropRect = MtkCamUtils::calCrop(srcRect, dstRect, 100);
    MY_LOGD("srcRect xywh(%d,%d,%d,%d)", srcRect.x, srcRect.y, srcRect.w, srcRect.h);
    MY_LOGD("dstRect xywh(%d,%d,%d,%d)", dstRect.x, dstRect.y, dstRect.w, dstRect.h);
    MY_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.x, cropRect.y, cropRect.w, cropRect.h);

#if TEST_3A
    p3A = IHal3A::createInstance(IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
    if(!p3A) {
        MY_LOGE("IHal3A::createInstance fail");
        ret = MFALSE;
        goto lbExit;
    }

    {
        ParamIspProfile_T _3A_profile(profile
                                    , 0  //magicNum
                                    , MFALSE
                                    , ParamIspProfile_T::EParamValidate_P2Only
                                    );
        p3A->setIspProfile(_3A_profile);
    }
    p3A->destroyInstance(LOG_TAG);
#endif

    // init
    if (eShotMode_ZsdMfllShot == meShotMode){
        mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance("mfll_P2iopipe", ENormalStreamTag_Vss, mSensorParam.u4OpenID);
      } else {
        mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance("mfll_P2iopipe", ENormalStreamTag_Cap, mSensorParam.u4OpenID);
    }
    CHECK_OBJECT(mpINormalStream);
    ret = ret && mpINormalStream->init();
    CHECK_RET("mpINormalStream->init");

    //crop
    {
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = srcSize;
        crop1.mResizeDst  = srcSize;
        //
        MCrpRsInfo crop2;
        crop2.mGroupID    = 2;
        crop2.mCropRect.p_integral.x = cropRect.x; //0
        crop2.mCropRect.p_integral.y = cropRect.y; //0
        crop2.mCropRect.p_fractional.x = 0;
        crop2.mCropRect.p_fractional.y = 0;
        crop2.mCropRect.s.w = cropRect.w;  //mRaw_Width
        crop2.mCropRect.s.h = cropRect.h;  //mRaw_Height
        //crop2.mResizeDst = MSize(0,0);
        params.mvCropRsInfo.push_back(crop1);
        params.mvCropRsInfo.push_back(crop2);
    }

    //
    params.mvIn.clear();
    minput.mBuffer = rawBuffer.get();
    //minput.mPortID=((inPorts.itemAt(inputIdx, Type2Type< IMetadata >())).entryFor(MTK_IOPIPE_INFO_PORT_ID)).itemAt(0,Type2Type< MINT32 >());
    minput.mPortID.inout = 0; //in
    minput.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMGI;
    minput.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput);

    //
    params.mvOut.clear();
    moutput.mBuffer = yuvBuffer.get();
    moutput.mPortID.inout = 1; //out
    moutput.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_WDMAO;
    moutput.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvOut.push_back(moutput);

    //
    if(mPrivateDataSize!=0 && mPrivateData!=NULL) {
        params.mpPrivaData = mPrivateData;
    } else {
        MY_LOGE("must init mPrivateData first, mPrivateData=%p, mPrivateDataSize=%d", mPrivateData, mPrivateDataSize);
    }

    // enqueue buffer
    params.mFrameNo = 0;    //i;
    mpINormalStream->enque(params);

    // dequeue buffer
    dequeParams.mFrameNo = 0;   //i;
    ret = ret && mpINormalStream->deque(dequeParams, 5000000000);   //5s,unit is nsec
    CHECK_RET("mpINormalStream->deque");
    MY_LOGD("deque result magicNum(%d)", dequeParams.mFrameNo);
    MY_LOGD("deque mvIn.size(%lu),mvOut.size(%lu)", dequeParams.mvIn.size(), dequeParams.mvOut.size());

lbExit:
    if(mpINormalStream) {
        mpINormalStream->uninit();
        mpINormalStream->destroyInstance("mfll_P2iopipe");
    }
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doSingleFrameOutput()
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MY_LOGD("singleout - doSingleFrameOutput()");
    IImageBuffer *pImgSrc = mMemoryYuvGolden.get();
    IImageBuffer *pImgDst = mMemoryYuvMixing.get();

    /* Rectangles */
    NSCamHW::Rect cropRect;
    NSCamHW::Rect srcRect;
    NSCamHW::Rect dstRect;
    /* Size */
    MSize srcSize;
    MSize dstSize;

    //calculate crop
    {

        srcSize   =  pImgSrc->getImgSize();
        dstSize   =  pImgDst->getImgSize();
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.w = srcSize.w;
        srcRect.h = srcSize.h;

        dstRect.x = 0;
        dstRect.y = 0;

        /**
         *  if using 90 or 270 degree, we need to exchange width and height.
         *  Crop window always follows source rectangle, therefore we will
         *  exchange destination rectangle size.
         */
        if (mShotParam.u4PictureTransform == eTransform_ROT_90 ||
            mShotParam.u4PictureTransform == eTransform_ROT_270 ) {
            dstRect.w = dstSize.h;
            dstRect.h = dstSize.w;
        }
        else {
            dstRect.w = dstSize.w;
            dstRect.h = dstSize.h;
        }

        MFLOAT fSrcRatio = srcRect.w / (MFLOAT)(srcRect.h);
        MFLOAT fDesRatio = dstRect.w / (MFLOAT)(dstRect.h);

        if( fSrcRatio > fDesRatio)
        {
            dstRect.h = srcRect.h;
            dstRect.w = dstRect.h * fDesRatio;
        }
        else
        {
            dstRect.w = srcRect.w;
            dstRect.h = dstRect.w / fDesRatio;
        }

        MY_LOGD("singleout - srcRect(x,y,w,h)=(%d,%d,%d,%d)", srcRect.x, srcRect.y, srcRect.w, srcRect.h);
        MY_LOGD("singleout - dstRect(x,y,w,h)=(%d,%d,%d,%d)", dstRect.x, dstRect.y, dstRect.w, dstRect.h);
    }

    cropRect = MtkCamUtils::calCrop(srcRect, dstRect, mShotParam.u4ZoomRatio);

    ret = ret && convertImage(mMemoryYuvB
                            , mMemoryYuvMixing
                            , mShotParam.u4PictureTransform
                            , MTRUE
                            , MRect(MPoint(cropRect.x,cropRect.y), MSize(cropRect.w, cropRect.h))
                            );

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doMixing()
{
    MFLL_TRACE_CALL();
    EIspProfile_T profile;

    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;
#if TEST_MIXING
    // declaration
    // - input ports
    NSCam::NSIoPipe::NSPostProc::Input minput_imgi;     //mfb
    NSCam::NSIoPipe::NSPostProc::Input minput_vipi;     //normal capture
    NSCam::NSIoPipe::NSPostProc::Input minput_vip3i;    //weighting map (mfb)
    NSCam::NSIoPipe::NSPostProc::Input minput_ufdi;     //weighting map (mixer)
    // - output ports
    NSCam::NSIoPipe::NSPostProc::Output moutput_wroto;  //mixing output (rotate)
    MSize srcSize(mYuv_Width, mYuv_Height);
    MSize dstSize(mYuv_Width, mYuv_Height);
    NSCamHW::Rect srcRect(0, 0, srcSize.w, srcSize.h);
    NSCamHW::Rect dstRect(0, 0, dstSize.w, dstSize.h);
    NSCamHW::Rect cropRect = MtkCamUtils::calCrop(srcRect, dstRect, 100);
    MY_LOGD("srcRect xywh(%d,%d,%d,%d)", srcRect.x, srcRect.y, srcRect.w, srcRect.h);
    MY_LOGD("dstRect xywh(%d,%d,%d,%d)", dstRect.x, dstRect.y, dstRect.w, dstRect.h);
    MY_LOGD("zoom(%d)", mShotParam.u4ZoomRatio);
    MY_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.x, cropRect.y, cropRect.w, cropRect.h);

    // - queue
    QParams params;
    QParams dequeParams;
    //
    IFeatureStream* mpIMfbMixing = NULL;
    IHal3A *p3A = NULL;

    // - output buffer
    sp<IImageBuffer>    pMemoryYuvMixing = NULL;
    IImageBufferHeap *pHeap;
    if( mShotParam.u4PictureTransform == eTransform_ROT_90 ||
            mShotParam.u4PictureTransform == eTransform_ROT_270 )
    {
        allocImageBuffer(&pHeap, &pMemoryYuvMixing, mYuv_Height, mYuv_Width, muNRType==ECamShot_NRTYPE_SWNR ? eImgFmt_I420 : eImgFmt_YUY2);
    }
    else
    {

        allocImageBuffer(&pHeap, &pMemoryYuvMixing, mYuv_Width, mYuv_Height, muNRType==ECamShot_NRTYPE_SWNR ? eImgFmt_I420 : eImgFmt_YUY2);
    }

    MY_LOGD("moutput_wroto size=(%d,%d) trans(%d)", pMemoryYuvMixing->getImgSize().w, pMemoryYuvMixing->getImgSize().h, mShotParam.u4PictureTransform);
    moutput_wroto.mBuffer = pMemoryYuvMixing.get();

#if TEST_3A
    // update 3A profile
    p3A = IHal3A::createInstance(IHal3A::E_Camera_1, mSensorParam.u4OpenID, LOG_TAG);
    if(!p3A)
    {
        MY_LOGE("IHal3A::createInstance fail");
        ret = MFALSE;
        goto lbExit;
    }

    {
        if (eShotMode_ZsdMfllShot == meShotMode){
            profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_VSS_MFB_PostProc_Mixing_SWNR : EIspProfile_VSS_MFB_PostProc_Mixing);
        } else {
            profile = (muNRType==ECamShot_NRTYPE_SWNR ? EIspProfile_MFB_PostProc_Mixing_SWNR : EIspProfile_MFB_PostProc_Mixing);
        }

        ParamIspProfile_T _3A_profile(profile
                                    , 0  //magicNum
                                    , MFALSE
                                    , ParamIspProfile_T::EParamValidate_P2Only
                                    );
        p3A->setIspProfile(_3A_profile);
    }
    p3A->destroyInstance(LOG_TAG);
#endif

    // init
    if (eShotMode_ZsdMfllShot == meShotMode){
        mpIMfbMixing = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("MFB_Mixing", EFeatureStreamTag_MFB_Mix_Vss, mSensorParam.u4OpenID);
    } else {
        mpIMfbMixing = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("MFB_Mixing", EFeatureStreamTag_MFB_Mix, mSensorParam.u4OpenID);
    }
    CHECK_OBJECT(mpIMfbMixing);
    ret = ret && mpIMfbMixing->init();
    CHECK_RET("mpIMfbMixing init fail");

    if(TEST_BITTURE_LDVT) {
        // blending yuv = imgi
        mPtrYuvBlended->unlockBuf(LOG_TAG);
        mPtrYuvBlended->loadFromFile("/sdcard/input_mixer/imgi_h_5280x960.yuy2");
        mPtrYuvBlended->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

        // normal yuv = vipi
        mPtrYuvGolden->unlockBuf(LOG_TAG);
        mPtrYuvGolden->loadFromFile("/sdcard/input_mixer/vipi_h_5280x960.yuy2");
        mPtrYuvGolden->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

        // weighting = vip3i, ufdi
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->unlockBuf(LOG_TAG);
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->loadFromFile("/sdcard/input_mixer/ufdi_h_5280x960.y");
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    }
    else if(TEST_BITTURE) {
        // blending yuv
        mPtrYuvBlended->unlockBuf(LOG_TAG);
        mPtrYuvBlended->loadFromFile("/sdcard/input/Mixer/Result211_blend.yuv_3200x2400.yuy2");
        mPtrYuvBlended->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

        // normal yuv
        mPtrYuvGolden->unlockBuf(LOG_TAG);
        mPtrYuvGolden->loadFromFile("/sdcard/input/Mixer/Result211_sf.yuv_3200x2400.yuy2");
        mPtrYuvGolden->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);

        // weighting
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->unlockBuf(LOG_TAG);
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->loadFromFile("/sdcard/input/Mixer/BLD_W.dat_3200x2400.y");
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READ|eBUFFER_USAGE_SW_MASK);
    }

    if(mDump & MFLL_DUMP_MIXING) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH];
        // blending yuv
        snprintf(filename, sizeof(filename), "%s_%02d_mix_input_mfb__%04dx%04d.yuy2", mDumpFilename, 6, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mPtrYuvBlended->saveToFile(filename);
        // normal yuv
        snprintf(filename, sizeof(filename), "%s_%02d_mix_input_normal__%04dx%04d.yuy2", mDumpFilename, 6, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mPtrYuvGolden->saveToFile(filename);
        // weighting
        snprintf(filename, sizeof(filename), "%s_%02d_mix_input_weight__%04dx%04d.y", mDumpFilename, 6, mYuv_Width, mYuv_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->saveToFile(filename);
    }

    MY_LOGD("===[MFB MIX] go single MFB Mixing enqueue ===\n");

    //no crop on mixing
    {
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = srcSize;
        crop1.mResizeDst  = srcSize;
        MY_LOGD("crop1 xywh(%d,%d,%d,%d)"
                , crop1.mCropRect.p_integral.x
                , crop1.mCropRect.p_integral.y
                , crop1.mCropRect.s.w
                , crop1.mCropRect.s.h
                );
        //
        MCrpRsInfo crop2;
        crop2.mGroupID    = 2;
        crop2.mCropRect.p_integral.x = cropRect.x; //0
        crop2.mCropRect.p_integral.y = cropRect.y; //0
        crop2.mCropRect.p_fractional.x = 0;
        crop2.mCropRect.p_fractional.y = 0;
        crop2.mCropRect.s.w = cropRect.w;  //mRaw_Width
        crop2.mCropRect.s.h = cropRect.h;  //mRaw_Height
        //crop2.mResizeDst = MSize(0,0);
        MY_LOGD("crop2 xywh(%d,%d,%d,%d)"
                , crop2.mCropRect.p_integral.x
                , crop2.mCropRect.p_integral.y
                , crop2.mCropRect.s.w
                , crop2.mCropRect.s.h
                );

        params.mvCropRsInfo.push_back(crop1);
        params.mvCropRsInfo.push_back(crop2);
    }

    // input ports
    params.mvIn.clear();
    // - mfb output
    minput_imgi.mBuffer = mPtrYuvBlended.get();
    minput_imgi.mPortID.inout =  0; //in
    minput_imgi.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMGI;
    minput_imgi.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput_imgi);
    MY_LOGD("minput_imgi.mBuffer = {VA=%p, size=%zu}", (void*)mPtrYuvBlended->getBufVA(0), mPtrYuvBlended->getBufSizeInBytes(0));
    MY_LOGD("minput_imgi.mCropRect.s.w = %d", minput_imgi.mCropRect.s.w);
    MY_LOGD("minput_imgi.mCropRect.s.h = %d", minput_imgi.mCropRect.s.h);

    // - normal capture
    minput_vipi.mBuffer = mPtrYuvGolden.get();
    minput_vipi.mPortID.inout = 0; //in
    minput_vipi.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_VIPI;
    minput_vipi.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput_vipi);
    MY_LOGD("minput_vipi.mBuffer = {VA=%p, size=%zu}", (void*)mPtrYuvGolden->getBufVA(0), mPtrYuvGolden->getBufSizeInBytes(0));
    MY_LOGD("minput_vipi.mCropRect.s.w = %d", minput_vipi.mCropRect.s.w);
    MY_LOGD("minput_vipi.mCropRect.s.h = %d", minput_vipi.mCropRect.s.h);

    // - weighting map (mfb)
    minput_vip3i.mBuffer = mMemoryWeightingMap[mMemoryFinalWeightingMapIndex].get();
    minput_vip3i.mPortID.inout = 0; //in
    minput_vip3i.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_VIP3I;
    minput_vip3i.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput_vip3i);
    MY_LOGD("minput_vip3i.mBuffer = {VA=%p, size=%zu}"
            , (void*)mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->getBufVA(0)
            , mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->getBufSizeInBytes(0)
            );
    MY_LOGD("minput_vip3i.mCropRect.s.w = %d", minput_vip3i.mCropRect.s.w);
    MY_LOGD("minput_vip3i.mCropRect.s.h = %d", minput_vip3i.mCropRect.s.h);

    // - weighting map (mixer)
    minput_ufdi.mBuffer = mMemoryWeightingMap[mMemoryFinalWeightingMapIndex].get();
    minput_ufdi.mPortID.inout = 0; //in
    minput_ufdi.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_UFDI;
    minput_ufdi.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
    params.mvIn.push_back(minput_ufdi);
    MY_LOGD("minput_ufdi.mBuffer = {VA=%p, size=%zu}"
            , (void*)mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->getBufVA(0)
            , mMemoryWeightingMap[mMemoryFinalWeightingMapIndex]->getBufSizeInBytes(0)
            );
    MY_LOGD("minput_ufdi.mCropRect.s.w = %d", minput_ufdi.mCropRect.s.w);
    MY_LOGD("minput_ufdi.mCropRect.s.h = %d", minput_ufdi.mCropRect.s.h);

    // output ports
    // - wdmao or wrot for mixing output
    params.mvOut.clear();
    MY_LOGD("moutput_wroto size=(%d,%d) trans(%d)", pMemoryYuvMixing->getImgSize().w, pMemoryYuvMixing->getImgSize().h, mShotParam.u4PictureTransform);
    moutput_wroto.mBuffer = pMemoryYuvMixing.get();
#if TEST_BITTURE
    MY_LOGD("use img3o");
    moutput_wroto.mPortID.inout = 1; //out
    moutput_wroto.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_IMG3O;    //skip mdp
    moutput_wroto.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
#else   //wroto
    MY_LOGD("use wroto");
    moutput_wroto.mTransform = mShotParam.u4PictureTransform;
    moutput_wroto.mPortID.inout = 1; //out
    moutput_wroto.mPortID.index = NSCam::NSIoPipe::NSPostProc::EPipePortIndex_WROTO;    //use mdp
    moutput_wroto.mPortID.type = NSCam::NSIoPipe::EPortType_Memory;
#endif
    params.mvOut.push_back(moutput_wroto);    //depend on sel

    // enqueue
    params.mFrameNo = 0;
    mpIMfbMixing->enque(params);
    ret = ret && mpIMfbMixing->deque(dequeParams, 5000000000);    //5s,unit is nsec

    /* release mixing relateb buffers after used */
    mrpReleaseMixingRelatedBuffers();

#if TEST_DBGINFO
    updateRegToDebugInfo(MF_REGTABLE_AFTER_MFB);
#endif

    if(!ret)
    {
        MY_LOGE("deque fail, frameNo(0x%x)", dequeParams.mFrameNo);
        MY_LOGE("[MFB MIX] deque magicNum(%d)", dequeParams.mFrameNo);
        MY_LOGE("[MFB MIX] deque mvIn.size(%lu), mvOut.size(%lu)", dequeParams.mvIn.size(), dequeParams.mvOut.size());
        goto lbExit;
    }

    if(mDump & MFLL_DUMP_MIXING)
    {
        //pass2
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        //
        if(muNRType==ECamShot_NRTYPE_SWNR){
            snprintf(filename, sizeof(filename), "%s_%02d_mix_output__%04dx%04d.i420", mDumpFilename, 6, pMemoryYuvMixing->getImgSize().w, pMemoryYuvMixing->getImgSize().h);
        } else {
            snprintf(filename, sizeof(filename), "%s_%02d_mix_output__%04dx%04d.yuy2", mDumpFilename, 6, pMemoryYuvMixing->getImgSize().w, pMemoryYuvMixing->getImgSize().h);
        }
        MY_LOGD("flowCapture saveToFile %s", filename);
        pMemoryYuvMixing->saveToFile(filename);
    }


    //calculate crop
    {

        srcSize   =  pMemoryYuvMixing->getImgSize();
        dstSize   =  mMemoryYuvMixing->getImgSize();
        srcRect.x = 0;
        srcRect.y = 0;
        srcRect.w = srcSize.w;
        srcRect.h = srcSize.h;

        dstRect.x = 0;
        dstRect.y = 0;
        dstRect.w = dstSize.w;
        dstRect.h = dstSize.h;


        MFLOAT fSrcRatio = srcRect.w / (MFLOAT)(srcRect.h);
        MFLOAT fDesRatio = dstRect.w / (MFLOAT)(dstRect.h);

        if( fSrcRatio > fDesRatio)
        {
            dstRect.h = srcRect.h;
            dstRect.w = dstRect.h * fDesRatio;

        }
        else
        {
            dstRect.w = srcRect.w;
            dstRect.h = dstRect.w / fDesRatio;
        }

    }

    cropRect = MtkCamUtils::calCrop(srcRect, dstRect, mShotParam.u4ZoomRatio);


    MY_LOGD("mShotParam.u4PictureTransform = %d ", mShotParam.u4PictureTransform);
    MY_LOGD("srcRect xywh(%d,%d,%d,%d)", srcRect.x, srcRect.y, srcRect.w, srcRect.h);
    MY_LOGD("dstRect xywh(%d,%d,%d,%d)", dstRect.x, dstRect.y, dstRect.w, dstRect.h);
    MY_LOGD("zoom(%d)", mShotParam.u4ZoomRatio);
    MY_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.x, cropRect.y, cropRect.w, cropRect.h);



    //NR
    MY_LOGD("muNRType(%d)", muNRType);
    switch(muNRType)
    {
        case ECamShot_NRTYPE_NONE:
            {
                 ret = ret && convertImage(pMemoryYuvMixing
                            , mMemoryYuvMixing
                            , 0
                            , MTRUE
                            , MRect(MPoint(cropRect.x,cropRect.y), MSize(cropRect.w, cropRect.h))
                            );
            }
            break;
        case ECamShot_NRTYPE_HWNR:
            {
                // allcoate internal buffer
                sp<IImageBuffer> pInternalBuf;
                {
                    MY_LOGD("allocate memory");
                    // allocate internal memory
                    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();

                    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                    MUINT32 bufStrideInByte[3] = { pMemoryYuvMixing->getBufStridesInBytes(0), 0, 0 };

                    IImageBufferAllocator::ImgParam imgParam(
                            pMemoryYuvMixing->getImgFormat(),
                            pMemoryYuvMixing->getImgSize(),
                            bufStrideInByte,
                            bufBoundaryInBytes,
                            pMemoryYuvMixing->getPlaneCount()
                            );

                    pInternalBuf = allocator->alloc_ion(LOG_TAG, imgParam);
                }
                if ( !pInternalBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
                {
                    MY_LOGE("lock Buffer failed\n");
                    goto lbExit;
                }
                if ( !pInternalBuf->syncCache( eCACHECTRL_INVALID ) )
                {
                    MY_LOGE("syncCache failed\n");
                    goto lbExit;
                }
                // 1st run
                if (eShotMode_ZsdMfllShot == meShotMode){
                    profile = EIspProfile_VSS_MFB_MultiPass_ANR_1;
                } else {
                    profile = EIspProfile_MFB_MultiPass_ANR_1;
                }
                if( !doHwNR(mSensorParam.u4OpenID
                            , pMemoryYuvMixing.get()
                            , pInternalBuf.get()
                            , NULL
                            , MRect(MPoint(0),pMemoryYuvMixing->getImgSize())
                            , 0
                            , profile ))
                {

                    MY_LOGE("1st run NR failed");
                    goto lbExit;
                }

                if(mDump & MFLL_DUMP_MIXING)
                {
                    //pass2
                    char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
                    //
                    snprintf(filename, sizeof(filename), "%s_%02d_doHwNR1_output__%04dx%04d.yuy2", mDumpFilename, 6, pInternalBuf->getImgSize().w, pInternalBuf->getImgSize().h);
                    MY_LOGD("flowCapture saveToFile %s", filename);
                    pInternalBuf->saveToFile(filename);
                }

                if (eShotMode_ZsdMfllShot == meShotMode){
                    profile = EIspProfile_VSS_MFB_MultiPass_ANR_2;
                } else {
                    profile = EIspProfile_MFB_MultiPass_ANR_2;
                }
                //2nd run
                if( !doHwNR(mSensorParam.u4OpenID
                            , pInternalBuf.get()
                            , mMemoryYuvMixing.get()
                            , NULL
                            , MRect(MPoint(cropRect.x,cropRect.y), MSize(cropRect.w, cropRect.h))
                            , 0
                            , profile) )
                {

                    MY_LOGE("2nd run NR failed");
                    goto lbExit;
                }

                if(mDump & MFLL_DUMP_MIXING)
                {
                    //pass2
                    char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
                    //
                    snprintf(filename, sizeof(filename), "%s_%02d_doHwNR2_output__%04dx%04d.yuy2", mDumpFilename, 6, mMemoryYuvMixing->getImgSize().w, mMemoryYuvMixing->getImgSize().h);
                    MY_LOGD("flowCapture saveToFile %s", filename);
                    mMemoryYuvMixing->saveToFile(filename);
                }

                if( pInternalBuf.get() )
                {
                    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
                    if( !pInternalBuf->unlockBuf( LOG_TAG ) )
                    {

                        MY_LOGE("unlock pInternalBuf failed\n");
                        ret = MFALSE;
                    }
                    allocator->free(pInternalBuf.get());
                    pInternalBuf = NULL;
                }
             }

            break;

        case ECamShot_NRTYPE_SWNR:
            {
                ret = ret && pMemoryYuvMixing->syncCache(eCACHECTRL_INVALID);  //hw->cpu
                CHECK_RET("flush cache hw->cpu fail");

                SwNR *swnr = new SwNR(mSensorParam.u4OpenID);
                ret = ret && swnr->doSwNR(pMemoryYuvMixing.get());
                delete swnr;
                CHECK_RET("do swnr fail");

                ret = ret && pMemoryYuvMixing->syncCache(eCACHECTRL_FLUSH);    //cpu->hw
                CHECK_RET("flush cache cpu->hw fail");

                if(mDump & MFLL_DUMP_MIXING)
                 {
                     //pass2
                     char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
                     //
                     snprintf(filename, sizeof(filename), "%s_swnr_input__%04dx%04d.i420", mDumpFilename, pMemoryYuvMixing->getImgSize().w, pMemoryYuvMixing->getImgSize().h);
                     MY_LOGD("flowCapture saveToFile %s", filename);
                     pMemoryYuvMixing->saveToFile(filename);
                 }

                ret = ret && convertImage(pMemoryYuvMixing
                            , mMemoryYuvMixing
                            , 0
                            , MTRUE
                            , MRect(MPoint(cropRect.x,cropRect.y), MSize(cropRect.w, cropRect.h))
                            );

                 if(mDump & MFLL_DUMP_MIXING)
                 {
                     //pass2
                     char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
                     //
                     snprintf(filename, sizeof(filename), "%s_swnr_output__%04dx%04d.yuy2", mDumpFilename, mMemoryYuvMixing->getImgSize().w, mMemoryYuvMixing->getImgSize().h);
                     MY_LOGD("flowCapture saveToFile %s", filename);
                     mMemoryYuvMixing->saveToFile(filename);
                 }
            }
            break;
    }




#else
    msleep(200);
#endif //TEST_MIXING


    #if TEST_BITTURE
    MY_LOGD("while 1 for dump register");
    while(1);
    #endif

lbExit:
    if(mpIMfbMixing) {
        mpIMfbMixing->uninit();
        mpIMfbMixing->destroyInstance("MFB_Mixing");
    }

    if( pMemoryYuvMixing != NULL )
    {
        freeImageBuffer(&pMemoryYuvMixing);

    }

    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: postview
//------------------------------------------------------------------------------
MBOOL
MfllShot::
doPostview()
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    if(!(mi4DataMsgSet & ECamShot_BUF_TYPE_POSTVIEW)) {
        MY_LOGD("skip do doPostview()");
        FUNCTION_LOG_END;
        return ret;
    }

    WAIT(&mMemoryReady_Postview);

#if TEST_POSTVIEW
    //  1.  create postview from mixing output
    //ret = ret && convertImage(mMemoryYuvMixing, mMemoryPostview, mShotParam.u4PostViewTransform);

    MINT32 degree = Transform2Degree(mShotParam.u4PictureTransform);
    MUINT32 inverseTransform = Degree2Transform(-degree);
    inverseTransform |= (mShotParam.u4PictureTransform & (eTransform_FLIP_H|eTransform_FLIP_V));

    ret = ret && convertImage(mMemoryYuvMixing
                            , mMemoryPostview
                            , inverseTransform
                            );

    if(mDump & MFLL_DUMP_POSTVIEW) {
        //pass2
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        //
        snprintf(filename, sizeof(filename), "%s_%02d_postview_output__%04dx%04d.yv12", mDumpFilename, 7, mPostview_Width, mPostview_Height);
        MY_LOGD("flowCapture saveToFile %s", filename);
        mMemoryPostview->saveToFile(filename);
    }
#else
    msleep(50);
#endif

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
MfllShot::
convertImage(sp<IImageBuffer> &pSrcImgBufInfo
            , sp<IImageBuffer> &pJpgImgBufInfo
            , MUINT32 const u4Transform
            , MBOOL bIsCrop
            , NSCam::MRect cropRect
            )
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    ISImager *pISImager = NULL;

#if TEST_JPEG
    MY_LOGD("[createJpegImg] - u4Transform=%d", u4Transform);
    CHECK_OBJECT(pSrcImgBufInfo.get());
    CHECK_OBJECT(pJpgImgBufInfo.get());

    // Create Instance
    pISImager = ISImager::createInstance(pSrcImgBufInfo.get());
    CHECK_OBJECT(pISImager);

    // init setting
    ret = ret && pISImager->setTransform(u4Transform);
    CHECK_RET("pISmage setTransform fail");

    MY_LOGD("Src WH(%d %d)", pSrcImgBufInfo->getImgSize().w, pSrcImgBufInfo->getImgSize().h);
    MY_LOGD("Des WH(%d %d)", pJpgImgBufInfo->getImgSize().w, pJpgImgBufInfo->getImgSize().h);

    MY_LOGD("cropRect xywh(%d,%d,%d,%d)", cropRect.p.x, cropRect.p.y, cropRect.s.w, cropRect.s.h);
    ret = ret && pISImager->setCropROI(cropRect);
    ret = ret && pISImager->setTargetImgBuffer(pJpgImgBufInfo.get());
    CHECK_RET("pISmage config fail");
    ret = ret && pISImager->execute();
    CHECK_RET("pISImager->execute fail");
#else
    msleep(50);
#endif  //TEST_JPEG

lbExit:
    if(pISImager)
        pISImager->destroyInstance();
    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: jpeg
//------------------------------------------------------------------------------
MBOOL
MfllShot::
doFullJpeg()
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;


    if(!(mi4DataMsgSet & ECamShot_BUF_TYPE_JPEG)) {
        MY_LOGD("skip do doFullJpeg()");
        FUNCTION_LOG_END;
        return ret;
    }

    //  1.  create jpeg bitstreawm
    NSCamShot::JpegParam    yuvJpegParam(mJpegParam.u4Quality, MFALSE);
    ret = ret && encodeJpeg(mMemoryYuvMixing
                            , yuvJpegParam
                            , 0
                            , mMemoryJpeg
                            , mFullJpegRealSize);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
doThumbnailJpeg()
{
    MFLL_TRACE_CALL();

    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    if(!(mi4DataMsgSet & ECamShot_BUF_TYPE_JPEG)) {
        MY_LOGD("skip do doFullJpeg()");
        FUNCTION_LOG_END;
        return ret;
    }

#if TEST_JPEG
    //  1.  create jpeg bitstreawm
    NSCamShot::JpegParam    yuvJpegParam(mJpegParam.u4ThumbQuality, MTRUE);
    ret = ret && encodeJpeg(mMemoryYuvMixing
                            , yuvJpegParam
                            , 0
                            , mMemoryJpeg_Thumbnail
                            , mThumbnailJpegRealSize);
#else
    msleep(50);
#endif  //TEST_JPEG

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
encodeJpeg(sp<IImageBuffer> &pSrcImgBufInfo
            , NSCamShot::JpegParam const & rJpgParm
            , MUINT32 const u4Transform
            , sp<IImageBuffer> &pJpgImgBufInfo
            , MUINT32 & u4JpegSize)
{
    FUNCTION_LOG_START;
    MBOOL    ret = MTRUE;

    ISImager *pISImager = NULL;
    MRect crop;
    MSize const srcSize(pSrcImgBufInfo->getImgSize());
    MSize const dstSize =
        ((u4Transform & eTransform_ROT_90)||(u4Transform & eTransform_ROT_270)) ?
        MSize( pJpgImgBufInfo->getImgSize().h, pJpgImgBufInfo->getImgSize().w ) :
        pJpgImgBufInfo->getImgSize();

#if TEST_JPEG
    // (0). debug
    MY_LOGD("[createJpegImg] - u4Transform=%d", u4Transform);
    u4JpegSize = 0;
    CHECK_OBJECT(pSrcImgBufInfo.get());

    // (1). Create Instance
    pISImager = ISImager::createInstance(pSrcImgBufInfo.get());
    CHECK_OBJECT(pISImager);

    // init setting
    ret = ret && pISImager->setTransform(u4Transform);
    ret = ret && pISImager->setEncodeParam(rJpgParm.fgIsSOI, rJpgParm.u4Quality);

    // crop to keep aspect ratio
#define align2(x)   (((x) + 1) & (~1))
    if( srcSize.w * dstSize.h > srcSize.h * dstSize.w ) {
        crop.s.w = align2(dstSize.w * srcSize.h / dstSize.h);
        crop.s.h = align2(srcSize.h);
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    } else if( srcSize.w * dstSize.h < srcSize.h * dstSize.w ) {
        crop.s.w = align2(srcSize.w);
        crop.s.h = align2(dstSize.h * srcSize.w / dstSize.w);
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    }
    pISImager->setCropROI(crop);
#undef align2
    //ret = ret && pISImager->setCropROI(MRect(MPoint(0,0), MPoint(pSrcImgBufInfo->getImgSize().w, pSrcImgBufInfo->getImgSize().h)));
    ret = ret && pISImager->setTargetImgBuffer(pJpgImgBufInfo.get());
    CHECK_RET("error before execute");
    ret = ret && pISImager->execute();
    CHECK_RET("error after execute");
    pISImager->destroyInstance();

    // update jpeg size
    u4JpegSize = pJpgImgBufInfo->getBitstreamSize();
#else
    msleep(50);
#endif  //TEST_JPEG

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: tuning
//------------------------------------------------------------------------------
MBOOL
MfllShot::
readTuningData(NVRAM_CAMERA_FEATURE_MFLL_STRUCT &rTuningData, MUINT32 sensorId)
{
    FUNCTION_LOG_START;
    MBOOL     ret = MTRUE;

    MY_LOGD("readTuningData - from nvram");
    int err;
    NVRAM_CAMERA_FEATURE_STRUCT* pNvram;

    MUINT sensorDev = SENSOR_DEV_NONE;
    {
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        CHECK_OBJECT(pHalSensorList);
        sensorDev = pHalSensorList->querySensorDevIdx(sensorId);
    }

    err = NvBufUtil::getInstance().getBufAndRead(CAMERA_NVRAM_DATA_FEATURE, sensorDev, (void*&)pNvram);
    ret = ret && !err;
    CHECK_RET("getBufAndRead fail, err=%d", err);

    // MFLL/AIS (4)
    rTuningData.max_frame_number = pNvram->mfll.max_frame_number;
    rTuningData.bss_clip_th = pNvram->mfll.bss_clip_th;
    rTuningData.memc_bad_mv_range = pNvram->mfll.memc_bad_mv_range;
    rTuningData.memc_bad_mv_rate_th = pNvram->mfll.memc_bad_mv_rate_th;

    // MFLL (1)
    rTuningData.mfll_iso_th = pNvram->mfll.mfll_iso_th;

    // AIS (4)
    rTuningData.ais_exp_th = pNvram->mfll.ais_exp_th;
    rTuningData.ais_advanced_tuning_en = pNvram->mfll.ais_advanced_tuning_en;
    rTuningData.ais_advanced_max_iso = pNvram->mfll.ais_advanced_max_iso;
    rTuningData.ais_advanced_max_exposure = pNvram->mfll.ais_advanced_max_exposure;

    // reserved (2)
    rTuningData.ais_exp_th0 = pNvram->mfll.ais_exp_th0;
    rTuningData.ais_iso_th0 = pNvram->mfll.ais_iso_th0;

    MY_LOGD("readTuningData - max_frame(%d), clip(%d), bad_mv_range(%d), bad_mv_rate_th(%d)"
            , rTuningData.max_frame_number
            , rTuningData.bss_clip_th
            , rTuningData.memc_bad_mv_range
            , rTuningData.memc_bad_mv_rate_th
            );
    MY_LOGD("readTuningData - mfll_iso_th(%d)"
            , rTuningData.mfll_iso_th
            );
    MY_LOGD("readTuningData - ais_exp_th(%d), adv_en(%d), adv_iso(%d), adv_exp(%d)"
            , rTuningData.ais_exp_th
            , rTuningData.ais_advanced_tuning_en
            , rTuningData.ais_advanced_max_iso
            , rTuningData.ais_advanced_max_exposure
            );
    MY_LOGD("readTuningData - ais_exp_th0(%d), ais_iso_th0(%d) "
            , rTuningData.ais_exp_th0
            , rTuningData.ais_iso_th0
            );


lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
getCaptureInfo(MUINT32 mfbMode, MUINT32 sensorId, MUINT32 &frameNumber, MUINT32 &exp, MUINT32 &iso,MBOOL &needUpdateAE)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;
    MY_LOGD("mfbMode(%d), sensorId(%d)", mfbMode, sensorId);

    // if there is any error occured, let frameNumber as 1 and ISmartShot will use ISingleshot rather than IMfllShot.
    NVRAM_CAMERA_FEATURE_MFLL_STRUCT rTuningData = {0};
    CaptureParam_T rCap3AParam;
    MINT32 needFlash = 1;
    MBOOL isRawSensor = MFALSE;
    frameNumber = 1;
    //
    IHal3A *p3A = NULL;

#if !(MTK_CAM_MFB_SUPPORT_LEVEL > 0)
    MY_LOGD("MFB not support");
    frameNumber = 1;
    return ret;
#endif

    // isRawSensor
    NSCam::SensorStaticInfo sensorInfo;
    ret = ret && querySensorInfo(sensorId, sensorInfo);
    CHECK_RET("can't querySensorInfo");
    MY_LOGD("sensorInfo.sensorType %d", sensorInfo.sensorType);

    // read tuning data
    MfllShot::readTuningData(rTuningData, sensorId);

    // read 3a data
    p3A = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);
    CHECK_OBJECT(p3A);
    p3A->getCaptureParams(rCap3AParam);
    needFlash = p3A->isNeedFiringFlash();
    p3A->destroyInstance(LOG_TAG);
    //
    exp = rCap3AParam.u4Eposuretime;
    iso = rCap3AParam.u4RealISO;
    // for debug
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("mediatek.mfll.exp", value, "-1");
        MINT32 forceExp = atoi(value);
        if(forceExp!=-1) {
            exp = forceExp * 1000;
        }
        property_get("mediatek.mfll.iso", value, "-1");
        MINT32 forceIso = atoi(value);
        if(forceIso!=-1) {
            iso = forceIso;
        }
    }
    MY_LOGD("precapture exp(%d) iso(%d)", exp, iso);

    // modify p-line for ais
    if(mfbMode == MFB_MODE_AIS) {
        MUINT32 total = iso * exp;

        if(exp > rTuningData.ais_exp_th0){// AIS should work
            exp = rTuningData.ais_exp_th0; //AIS stage1
            if (exp == 0) {
                MY_LOGE("ais_exp_th0 is 0");
                exp = 33000;
            }
            iso = total / exp;

            if(iso > rTuningData.ais_iso_th0){
                iso = rTuningData.ais_iso_th0;
                if (iso == 0) {
                    MY_LOGE("ais_iso_th0 is 0");
                    iso = 400;
                }
                exp = total / iso;
            }

            if(exp > rTuningData.ais_exp_th) {//AIS stage2
                exp = rTuningData.ais_exp_th;
                if (exp == 0) {
                    MY_LOGE("ais_exp_th is 0");
                    exp = 33000;
                }
                iso = total / exp;
            }
            MY_LOGD("ais exp(%d) iso(%d)", exp, iso);
        }
        if(rTuningData.ais_advanced_tuning_en) {
            if(iso > rTuningData.ais_advanced_max_iso) {
                iso = rTuningData.ais_advanced_max_iso;
                exp = total / iso;
                MY_LOGD("ais adv iso-limited exp(%d) iso(%d)", exp, iso);

                if(exp > rTuningData.ais_advanced_max_exposure) {
                    exp = rTuningData.ais_advanced_max_exposure;
                    MY_LOGD("ais adv exp-limited exp(%d) iso(%d)", exp, iso);
                }
            }
        }
    }
    MY_LOGD("final exp(%d) iso(%d)", exp, iso);


    // get frame number
    if(mfbMode!=MFB_MODE_OFF) {
        // flash
        if(needFlash) {
            MY_LOGD("disable MFLL since flash is on");
            frameNumber = 1;
        }
        // didn't support yuv sensor
        else if(sensorInfo.sensorType != SENSOR_TYPE_RAW) {
            MY_LOGD("disable MFLL since sensor type %d isn't supported", sensorInfo.sensorType);
            frameNumber = 1;
        }
        // eng mode
        else if(mfbMode == MFB_MODE_MFLL_FOR_ENGMODE) {
            MY_LOGD("enable MFLL for eng mode");
            frameNumber = rTuningData.max_frame_number;
        }
        // ais
        else if(mfbMode == MFB_MODE_AIS) {
            needUpdateAE = 1;
            if(iso>=rTuningData.mfll_iso_th || TEST_AQS_TUNING || TEST_AQS_15RAW) {
                MY_LOGD("enable AIS since iso(%d) higher than %d", iso, rTuningData.mfll_iso_th);
                frameNumber = rTuningData.max_frame_number;
            } else {
                MY_LOGD("disable AIS since iso(%d) lower than %d", iso, rTuningData.mfll_iso_th);
                ret = ret && MfllShot::updateAE(sensorId, exp, iso);
                frameNumber = 1;

            }
        }
        // mfll
        else if(mfbMode == MFB_MODE_MFLL) {
            if(iso>=rTuningData.mfll_iso_th || TEST_AQS_TUNING || TEST_AQS_15RAW) {
                MY_LOGD("enable MFLL since iso(%d) higher than %d", iso, rTuningData.mfll_iso_th);
                frameNumber = rTuningData.max_frame_number;
            } else {
                MY_LOGD("disable MFLL since iso(%d) lower than %d", iso, rTuningData.mfll_iso_th);
                frameNumber = 1;
            }
        }
    } else {
        frameNumber = 1;
    }

    // for debug
    {
        char value[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("mediatek.mfll.force", value, "-1");
        MINT32 force = atoi(value);
        if(force!=-1) {
            MY_LOGD("force enable mfb %d", force);
            frameNumber = 4;
        }
    }


lbExit:
    MY_LOGD("mfbMode(%d), frameNumber(%d), iso_th(%d), iso(%d)", mfbMode, frameNumber, rTuningData.mfll_iso_th, iso);
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
updateAE(MUINT32 sensorId, MUINT32 exp, MUINT32 iso)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    CaptureParam_T rOriCap3AParam;
    CaptureParam_T rNewCap3AParam;

    // init 3a
    IHal3A *p3A = IHal3A::createInstance(IHal3A::E_Camera_1, sensorId, LOG_TAG);
    CHECK_OBJECT(p3A);

    // get ori
    p3A->getCaptureParams(rOriCap3AParam);
    MY_LOGD("original u4Eposuretime(%d) u4AfeGain(%d) realIso(%d)", rOriCap3AParam.u4Eposuretime, rOriCap3AParam.u4AfeGain, rOriCap3AParam.u4RealISO);

    // calc new
    rNewCap3AParam = rOriCap3AParam;
    rNewCap3AParam.u4Eposuretime = exp;
    rNewCap3AParam.u4AfeGain = rOriCap3AParam.u4AfeGain * iso / rOriCap3AParam.u4RealISO;
    MY_LOGD("update u4Eposuretime(%d) u4AfeGain(%d) realIso(%d)", rNewCap3AParam.u4Eposuretime, rNewCap3AParam.u4AfeGain, iso);

    // udpate new
    p3A->updateCaptureParams(rNewCap3AParam);

    // destroy 3a
    p3A->destroyInstance(LOG_TAG);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
querySensorInfo(MUINT32 sensorId, NSCam::SensorStaticInfo &sensorInfo)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MINT32 const sensorNum = pHalSensorList->queryNumberOfSensors();

    if(sensorId >= sensorNum)
        ret = MFALSE;
    CHECK_RET("wrong sensor idx(0x%x), sensorNum(%d)", sensorId, sensorNum);

    pHalSensorList->querySensorStaticInfo(pHalSensorList->querySensorDevIdx(sensorId)
                                        , &sensorInfo
                                        );

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


//------------------------------------------------------------------------------
// utility: exif
//------------------------------------------------------------------------------
MBOOL
MfllShot::
updateDumpFilename()
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    time_t timer;
    struct tm* tm_info;
    char buffer[128] = {0};

    time(&timer);
    tm_info = localtime(&timer);
    strftime(buffer, 128, "IMG_%Y%m%d_%H%M%S", tm_info);
    snprintf(mDumpFilename, sizeof(mDumpFilename), "/storage/sdcard%d/DCIM/Camera/%s", mSdcardIndex, buffer);

lbExit:
    FUNCTION_LOG_END;
    return ret;
}

//------------------------------------------------------------------------------
// utility: exif
//------------------------------------------------------------------------------
inline void setDebugTag(DEBUG_MF_INFO_T &rMfDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    rMfDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_CMN_MID, a_i4ID, 0);
    rMfDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MBOOL
MfllShot::
setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr) const
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    MVOID* pBuf = NULL;
    {   //save mfll, bss, memc, ais as tag-value format
        DEBUG_MF_INFO_T& rMfDebugInfo = *(DEBUG_MF_INFO_T*)(pBuf = pDbgInfoCtnr->queryMemory(DEBUG_CAM_MF_MID, sizeof(DEBUG_MF_INFO_T)));
        CHECK_OBJECT(pBuf);

        memset(&rMfDebugInfo, 0, sizeof(DEBUG_MF_INFO_T));
        //[software info]
        // - mfll
        setDebugTag(rMfDebugInfo, MF_TAG_VERSION,           MF_DEBUG_TAG_VERSION);
        setDebugTag(rMfDebugInfo, MF_TAG_MFB_MODE,          mDbgMfllInfo.mfbMode);
        setDebugTag(rMfDebugInfo, MF_TAG_MAX_FRAME_NUMBER,  mDbgMfllInfo.maxFrameNumber);
        setDebugTag(rMfDebugInfo, MF_TAG_PROCESSING_NUMBER, mDbgMfllInfo.processingNumber);
        setDebugTag(rMfDebugInfo, MF_TAG_EXPOSURE,          mDbgMfllInfo.exposure);
        setDebugTag(rMfDebugInfo, MF_TAG_ISO,               mDbgMfllInfo.iso);
        setDebugTag(rMfDebugInfo, MF_TAG_ISO_HI_TH,         mDbgMfllInfo.iso_hi_th);
        setDebugTag(rMfDebugInfo, MF_TAG_ISO_LOW_TH,        mDbgMfllInfo.iso_low_th);
        setDebugTag(rMfDebugInfo, MF_TAG_RAW_WIDTH,         mDbgMfllInfo.rawWidth);
        setDebugTag(rMfDebugInfo, MF_TAG_RAW_HEIGHT,        mDbgMfllInfo.rawHeight);
        // - bss
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_ENABLE,        mDbgMfllInfo.bss.enable);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_ROI_WIDTH,     mDbgMfllInfo.bss.roiWidth);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_ROI_HEIGHT,    mDbgMfllInfo.bss.roiHeight);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_SCALE_FACTOR,  mDbgMfllInfo.bss.scaleFactor);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_CLIP_TH0,      mDbgMfllInfo.bss.clipTh0);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_CLIP_TH1,      mDbgMfllInfo.bss.clipTh1);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT0,        mDbgMfllInfo.bss.inputSequenceAfterBss[0]);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT1,        mDbgMfllInfo.bss.inputSequenceAfterBss[1]);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT2,        mDbgMfllInfo.bss.inputSequenceAfterBss[2]);
        setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT3,        mDbgMfllInfo.bss.inputSequenceAfterBss[3]);
        //setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT4,        mDbgMfllInfo.bss.inputSequenceAfterBss[4]);
        //setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT5,        mDbgMfllInfo.bss.inputSequenceAfterBss[5]);
        //setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT6,        mDbgMfllInfo.bss.inputSequenceAfterBss[6]);
        //setDebugTag(rMfDebugInfo, MF_TAG_BSS_INPUT7,        mDbgMfllInfo.bss.inputSequenceAfterBss[7]);
        // - memc
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_WIDTH,        mDbgMfllInfo.memc.width);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_HEIGHT,       mDbgMfllInfo.memc.height);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_INPUT_FORMAT, mDbgMfllInfo.memc.inputFormat);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_OUTPUT_FORMAT,mDbgMfllInfo.memc.outputFormat);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_SKIP,         mDbgMfllInfo.memc.skip);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_BAD_MB_COUNT0,mDbgMfllInfo.memc.badMbCount[0]);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_BAD_MB_COUNT1,mDbgMfllInfo.memc.badMbCount[1]);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_BAD_MB_COUNT2,mDbgMfllInfo.memc.badMbCount[2]);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_BAD_MB_TH,    mDbgMfllInfo.memc.badMbTh);
        setDebugTag(rMfDebugInfo, MF_TAG_MEMC_BAD_MV_RANGE, mDbgMfllInfo.memc.badMvRange);
        // - ais
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_EXPOSURE,      mDbgAisInfo.exposure);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_ISO,                   mDbgAisInfo.iso);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_CALCULATED_EXPOSURE, mDbgAisInfo.calculatedExposure);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_CALCULATED_ISO,        mDbgAisInfo.calculatedIso);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_EXPOSURE_TH0,          mDbgAisInfo.exposure_th0);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_ISO_TH0,               mDbgAisInfo.iso_th0);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_EXPOSURE_TH1,          mDbgAisInfo.exposure_th1);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_ADVANCED_ENABLE,       mDbgAisInfo.advanced.enable);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_ADVANCED_MAX_EXPOSURE, mDbgAisInfo.advanced.maxExposureGain);
        setDebugTag(rMfDebugInfo, MF_TAG_AIS_ADVANCED_MAX_ISO,      mDbgAisInfo.advanced.maxIsoGain);
        // - reg
        MUINT32 tabId = 0;
        MUINT32 regId = 0;
        tabId = MF_REGTABLE_BASE; regId = 0;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BASE_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        tabId = MF_REGTABLE_BEFORE_MFB; regId = 0;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        tabId = MF_REGTABLE_AFTER_MFB; regId = 0;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        // - reg mfb
        setDebugTag(rMfDebugInfo, MF_TAG_IN_MFB_CAM_MFB_LL_CON2,           mDbgMfllInfo.reg_mfb[0]);
        setDebugTag(rMfDebugInfo, MF_TAG_IN_MFB_CAM_MFB_LL_CON3,           mDbgMfllInfo.reg_mfb[1]);
        setDebugTag(rMfDebugInfo, MF_TAG_IN_MFB_CAM_MFB_LL_CON4,           mDbgMfllInfo.reg_mfb[2]);
        setDebugTag(rMfDebugInfo, MF_TAG_IN_MFB_CAM_MFB_LL_CON5,           mDbgMfllInfo.reg_mfb[3]);
        setDebugTag(rMfDebugInfo, MF_TAG_IN_MFB_CAM_MFB_LL_CON6,           mDbgMfllInfo.reg_mfb[4]);

        // - reg mixer3
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON2,           mDbgMfllInfo.reg_mixer3[0]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON3,           mDbgMfllInfo.reg_mixer3[1]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON4,           mDbgMfllInfo.reg_mixer3[2]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON5,           mDbgMfllInfo.reg_mixer3[3]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON6,           mDbgMfllInfo.reg_mixer3[4]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MIX3_CTRL_0,           mDbgMfllInfo.reg_mixer3[5]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MIX3_CTRL_1,           mDbgMfllInfo.reg_mixer3[6]);
        setDebugTag(rMfDebugInfo, MF_TAG_AFTER_MFB_CAM_MIX3_SPARE,            mDbgMfllInfo.reg_mixer3[7]);
    }

    {   //save gmv, eis, reg as hex list
        DEBUG_RESERVEC_INFO_T rRerserveADebugInfo;

        //gmv
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_MAX_FRAME >= MAX_REFERENCE_FRAMES);
        for(MUINT32 frame=0; frame<MAX_REFERENCE_FRAMES; frame++) {
            rRerserveADebugInfo.gmvData[frame][MF_GMV_DEBUG_TAG_GMV_X] = mDbgEisInfo[frame].gmvX;
            rRerserveADebugInfo.gmvData[frame][MF_GMV_DEBUG_TAG_GMV_Y] = mDbgEisInfo[frame].gmvY;
        }

        //eis
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_MAX_FRAME >= MAX_REFERENCE_FRAMES);
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_EIS_DEBUG_TAG_WINDOW >= EIS_MAX_WIN_NUM);
        for(MUINT32 frame=0; frame<MAX_REFERENCE_FRAMES; frame++) {
            for(MUINT32 window=0; window<EIS_MAX_WIN_NUM; window++) {
                rRerserveADebugInfo.eisData[frame][window][MF_EIS_DEBUG_TAG_MV_X] = mDbgEisInfo[frame].mvX[window];
                rRerserveADebugInfo.eisData[frame][window][MF_EIS_DEBUG_TAG_MV_Y] = mDbgEisInfo[frame].mvY[window];
                rRerserveADebugInfo.eisData[frame][window][MF_EIS_DEBUG_TAG_TRUST_X] = mDbgEisInfo[frame].trustX[window];
                rRerserveADebugInfo.eisData[frame][window][MF_EIS_DEBUG_TAG_TRUST_Y] = mDbgEisInfo[frame].trustY[window];
            }
        }

        pBuf = pDbgInfoCtnr->queryMemory(DEBUG_CAM_RESERVE3_MID, sizeof(DEBUG_RESERVEC_INFO_T));
        memcpy(pBuf, &rRerserveADebugInfo, sizeof(rRerserveADebugInfo));
    }

    if(mDump & MFLL_DUMP_EXIF) {
        char filename[MFB_MAX_DUMP_FILENAME_LENGTH] = {0};
        snprintf(filename, sizeof(filename), "%s_%02d_exif.txt", mDumpFilename, 8);
        MY_LOGD("setDebugInfo saveToFile %s", filename);
        dumpDebugInfoAsTextFile(filename);
    }

lbExit:
    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
updateRegToDebugInfo(MFB_REGTABLE regtable)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    IspDrv *pISPDrvObj = NULL;
    pISPDrvObj = IspDrv::createInstance();
    CHECK_OBJECT(pISPDrvObj);
    ret = ret && pISPDrvObj->init(LOG_TAG);
    CHECK_RET("pISPDrvObj->init() fail");

    {
        MUINT32 reg[] = {
            //anr
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_CON1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_CON2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_CON3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_YAD1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_YAD2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_4LUT1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_4LUT2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_4LUT3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_PTY),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_CAD),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_PTC),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_LCE1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_LCE2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_HP1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_HP2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_HP3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_ACTY),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_ANR_ACTC),
            //ee
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SRK_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_CLIP_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_FLT_CTRL_1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_FLT_CTRL_2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_01),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_02),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_03),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_04),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_05),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_06),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_EDTR_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_07),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_08),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_09),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_10),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_GLUT_CTRL_11),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_OUT_EDGE_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_Y_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_EDGE_CTRL_1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_EDGE_CTRL_2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_EDGE_CTRL_3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_SPECL_CTRL),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_CORE_CTRL_1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_SEEE_SE_CORE_CTRL_2),
        };
        memcpy(mDbgMfllInfo.reg[regtable], reg, sizeof(reg));
    }

    if(regtable == MF_REGTABLE_IN_MFB) {
        MUINT32 reg[] = {
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON4),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON5),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON6),
        };
        memcpy(mDbgMfllInfo.reg_mfb, reg, sizeof(reg));
    }

    if(regtable == MF_REGTABLE_AFTER_MFB) {
        MUINT32 reg[] = {
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON2),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON3),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON4),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON5),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MFB_LL_CON6),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MIX3_CTRL_0),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MIX3_CTRL_1),
            ISP_READ_REG_NOPROTECT(pISPDrvObj, CAM_MIX3_SPARE),
        };
        memcpy(mDbgMfllInfo.reg_mixer3, reg, sizeof(reg));
    }

lbExit:
    //
    if(pISPDrvObj != NULL) {
        MY_LOGD("pISPDrvObj uninit");
        ret = ret && pISPDrvObj->uninit(LOG_TAG);
        if(ret == MFALSE) {
            MY_LOGE("pISPDrvObj uninit fail");
        }
        pISPDrvObj->destroyInstance();
        pISPDrvObj = NULL;
    }

    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
dumpDebugInfoAsTextFile(const char * const filename) const
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    FILE *fd = fopen(filename, "wb");
    CHECK_OBJECT(fd);

    #define SAVE_SECTION_NAME_TO_FILE(fd, name, id)     fprintf(fd, "\n[%s %d]\n", name, id)
    {   //save mfll, bss, memc, ais as tag-value format
        #define SAVE_TAG_HEX_TO_FILE(fd, tag, value)  fprintf(fd, "%-40s:     %08x\n", #tag, value)

        SAVE_SECTION_NAME_TO_FILE(fd, "MFLL", 0);
        //mfll(11)
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_VERSION,           MF_DEBUG_TAG_VERSION);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MFB_MODE,          mDbgMfllInfo.mfbMode);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MAX_FRAME_NUMBER,  mDbgMfllInfo.maxFrameNumber);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_PROCESSING_NUMBER, mDbgMfllInfo.processingNumber);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_EXPOSURE,          mDbgMfllInfo.exposure);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_ISO,               mDbgMfllInfo.iso);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_ISO_HI_TH,         mDbgMfllInfo.iso_hi_th);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_ISO_LOW_TH,        mDbgMfllInfo.iso_low_th);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_RAW_WIDTH,         mDbgMfllInfo.rawWidth);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_RAW_HEIGHT,        mDbgMfllInfo.rawHeight);
        //bss(14)
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_ENABLE,        mDbgMfllInfo.bss.enable);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_ROI_WIDTH,     mDbgMfllInfo.bss.roiWidth);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_ROI_HEIGHT,    mDbgMfllInfo.bss.roiHeight);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_SCALE_FACTOR,  mDbgMfllInfo.bss.scaleFactor);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_CLIP_TH0,      mDbgMfllInfo.bss.clipTh0);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_CLIP_TH1,      mDbgMfllInfo.bss.clipTh1);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT0,        mDbgMfllInfo.bss.inputSequenceAfterBss[0]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT1,        mDbgMfllInfo.bss.inputSequenceAfterBss[1]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT2,        mDbgMfllInfo.bss.inputSequenceAfterBss[2]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT3,        mDbgMfllInfo.bss.inputSequenceAfterBss[3]);
        //SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT4,        mDbgMfllInfo.bss.inputSequenceAfterBss[4]);
        //SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT5,        mDbgMfllInfo.bss.inputSequenceAfterBss[5]);
        //SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT6,        mDbgMfllInfo.bss.inputSequenceAfterBss[6]);
        //SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BSS_INPUT7,        mDbgMfllInfo.bss.inputSequenceAfterBss[7]);
        //memc(10)
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_WIDTH,        mDbgMfllInfo.memc.width);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_HEIGHT,       mDbgMfllInfo.memc.height);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_INPUT_FORMAT, mDbgMfllInfo.memc.inputFormat);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_OUTPUT_FORMAT,mDbgMfllInfo.memc.outputFormat);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_SKIP,         mDbgMfllInfo.memc.skip);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_BAD_MB_COUNT0,mDbgMfllInfo.memc.badMbCount[0]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_BAD_MB_COUNT1,mDbgMfllInfo.memc.badMbCount[1]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_BAD_MB_COUNT2,mDbgMfllInfo.memc.badMbCount[2]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_BAD_MB_TH,    mDbgMfllInfo.memc.badMbTh);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_MEMC_BAD_MV_RANGE, mDbgMfllInfo.memc.badMvRange);
        //ais(10)
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_EXPOSURE,             mDbgAisInfo.exposure);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_ISO,                  mDbgAisInfo.iso);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_CALCULATED_EXPOSURE,  mDbgAisInfo.calculatedExposure);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_CALCULATED_ISO,       mDbgAisInfo.calculatedIso);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_EXPOSURE_TH0,         mDbgAisInfo.exposure_th0);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_ISO_TH0,              mDbgAisInfo.iso_th0);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_EXPOSURE_TH1,         mDbgAisInfo.exposure_th1);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_ADVANCED_ENABLE,      mDbgAisInfo.advanced.enable);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_ADVANCED_MAX_EXPOSURE,mDbgAisInfo.advanced.maxExposureGain);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AIS_ADVANCED_MAX_ISO,     mDbgAisInfo.advanced.maxIsoGain);
        //regs (anr + ee)
        MUINT32 tabId = 0;
        MUINT32 regId = 0;
        tabId = MF_REGTABLE_BASE; regId = 0;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BASE_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        tabId = MF_REGTABLE_BEFORE_MFB; regId = 0;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_BEFORE_MFB_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        tabId = MF_REGTABLE_AFTER_MFB; regId = 0;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_CON1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_CON2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_CON3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_YAD1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_YAD2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_4LUT1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_4LUT2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_4LUT3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_PTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_CAD, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_PTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_LCE1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_LCE2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_HP1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_HP2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_HP3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_ACTY, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_ANR_ACTC, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SRK_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_CLIP_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_FLT_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_FLT_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_01, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_02, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_03, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_04, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_05, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_06, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_EDTR_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_07, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_08, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_09, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_10, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_GLUT_CTRL_11, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_OUT_EDGE_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_Y_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_EDGE_CTRL_3, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_SPECL_CTRL, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_CORE_CTRL_1, mDbgMfllInfo.reg[tabId][regId]); regId++;
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_SEEE_SE_CORE_CTRL_2, mDbgMfllInfo.reg[tabId][regId]); regId++;

        //reg mfb
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_IN_MFB_CAM_MFB_LL_CON2,           mDbgMfllInfo.reg_mfb[0]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_IN_MFB_CAM_MFB_LL_CON3,           mDbgMfllInfo.reg_mfb[1]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_IN_MFB_CAM_MFB_LL_CON4,           mDbgMfllInfo.reg_mfb[2]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_IN_MFB_CAM_MFB_LL_CON5,           mDbgMfllInfo.reg_mfb[3]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_IN_MFB_CAM_MFB_LL_CON6,           mDbgMfllInfo.reg_mfb[4]);

        //reg mixer3
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON2,           mDbgMfllInfo.reg_mixer3[0]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON3,           mDbgMfllInfo.reg_mixer3[1]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON4,           mDbgMfllInfo.reg_mixer3[2]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON5,           mDbgMfllInfo.reg_mixer3[3]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MFB_LL_CON6,           mDbgMfllInfo.reg_mixer3[4]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MIX3_CTRL_0,           mDbgMfllInfo.reg_mixer3[5]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MIX3_CTRL_1,           mDbgMfllInfo.reg_mixer3[6]);
        SAVE_TAG_HEX_TO_FILE(fd, MF_TAG_AFTER_MFB_CAM_MIX3_SPARE,            mDbgMfllInfo.reg_mixer3[7]);

        #undef SAVE_TAG_HEX_TO_FILE
    }

    {   //save gmv, eis, reg as hex list

        //gmv
        #define SAVE_GMV_TO_FILE(fd, v1, v2)                fprintf(fd, "%d, %d\n", v1, v2)
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_MAX_FRAME >= MAX_REFERENCE_FRAMES);
        for(MUINT32 frame=0; frame<MAX_REFERENCE_FRAMES; frame++) {
            SAVE_SECTION_NAME_TO_FILE(fd, "gmv", frame);

            SAVE_GMV_TO_FILE(fd
                            , mDbgEisInfo[frame].gmvX
                            , mDbgEisInfo[frame].gmvY
                            );
        }
        #undef SAVE_GMV_TO_FILE

        //eis
        #define SAVE_EIS_TO_FILE(fd, v1, v2, v3, v4)        fprintf(fd, "%d, %d, %d, %d\n", v1, v2, v3, v4)
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_MAX_FRAME >= MAX_REFERENCE_FRAMES);
        COMPILE_TIME_ASSERT_FUNCTION_SCOPE(MF_EIS_DEBUG_TAG_WINDOW >= EIS_MAX_WIN_NUM);
        for(MUINT32 frame=0; frame<MAX_REFERENCE_FRAMES; frame++) {
            SAVE_SECTION_NAME_TO_FILE(fd, "eis", frame);

            for(MUINT32 window=0; window<EIS_MAX_WIN_NUM; window++) {
                SAVE_EIS_TO_FILE(fd
                                , mDbgEisInfo[frame].mvX[window]
                                , mDbgEisInfo[frame].mvY[window]
                                , mDbgEisInfo[frame].trustX[window]
                                , mDbgEisInfo[frame].trustY[window]
                                );
            }
        }
        #undef SAVE_EIS_TO_FILE
    }
    #undef SAVE_SECTION_NAME_TO_FILE


lbExit:

    if(fd != NULL) {
        fclose(fd);
        fd = NULL;
    }


    FUNCTION_LOG_END;
    return ret;
}


MBOOL
MfllShot::
dumpRegAsTextFile(const char * const filename) const
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    FILE *fd = NULL;
    IspDrv *pISPDrvObj = NULL;

    fd = fopen(filename, "wb");
    CHECK_OBJECT(fd);

    pISPDrvObj = IspDrv::createInstance();
    CHECK_OBJECT(pISPDrvObj);
    ret = ret && pISPDrvObj->init(LOG_TAG);
    CHECK_RET("pISPDrvObj->init() fail");

    //obc
    fprintf(fd, "OBC_OFST_B=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_OFFST0, OBC_OFST_B), 13));
    fprintf(fd, "OBC_OFST_GR=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_OFFST1, OBC_OFST_GR), 13));
    fprintf(fd, "OBC_OFST_GB=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_OFFST2, OBC_OFST_GB), 13));
    fprintf(fd, "OBC_OFST_R=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_OFFST3, OBC_OFST_R), 13));
    fprintf(fd, "OBC_GAIN_B=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_GAIN0, OBC_GAIN_B));
    fprintf(fd, "OBC_GAIN_GR=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_GAIN1, OBC_GAIN_GR));
    fprintf(fd, "OBC_GAIN_GB=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_GAIN2, OBC_GAIN_GB));
    fprintf(fd, "OBC_GAIN_R=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_OBC_GAIN3, OBC_GAIN_R));

    //pgn
    fprintf(fd, "PGN_SATU_B=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_SATU_1, PGN_SATU_B));
    fprintf(fd, "PGN_SATU_GB=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_SATU_1, PGN_SATU_GB));
    fprintf(fd, "PGN_SATU_GR=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_SATU_2, PGN_SATU_GR));
    fprintf(fd, "PGN_SATU_R=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_SATU_2, PGN_SATU_R));
    fprintf(fd, "PGN_GAIN_B=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_GAIN_1, PGN_GAIN_B));
    fprintf(fd, "PGN_GAIN_GB=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_GAIN_1, PGN_GAIN_GB));
    fprintf(fd, "PGN_GAIN_GR=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_GAIN_2, PGN_GAIN_GR));
    fprintf(fd, "PGN_GAIN_R=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_GAIN_2, PGN_GAIN_R));
    fprintf(fd, "PGN_OFST_B=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_OFST_1, PGN_OFST_B), 12));
    fprintf(fd, "PGN_OFST_GB=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_OFST_1, PGN_OFST_GB), 12));
    fprintf(fd, "PGN_OFST_GR=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_OFST_2, PGN_OFST_GR), 12));
    fprintf(fd, "PGN_OFST_R=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_PGN_OFST_2, PGN_OFST_R), 12));

    //g2g
    fprintf(fd, "G2G_CNV_00=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_1, G2G_CNV_00), 13));
    fprintf(fd, "G2G_CNV_01=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_1, G2G_CNV_01), 13));
    fprintf(fd, "G2G_CNV_02=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_2, G2G_CNV_02), 13));
    fprintf(fd, "G2G_CNV_10=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_3, G2G_CNV_10), 13));
    fprintf(fd, "G2G_CNV_11=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_3, G2G_CNV_11), 13));
    fprintf(fd, "G2G_CNV_12=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_4, G2G_CNV_12), 13));
    fprintf(fd, "G2G_CNV_20=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_5, G2G_CNV_20), 13));
    fprintf(fd, "G2G_CNV_21=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_5, G2G_CNV_21), 13));
    fprintf(fd, "G2G_CNV_22=%d\n", Complement2(ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CNV_6, G2G_CNV_22), 13));
    fprintf(fd, "G2G_ACC=%d\n", ISP_READ_BITS_NOPROTECT(pISPDrvObj , CAM_G2G_CTRL, G2G_ACC));

lbExit:
    //
    if(fd != NULL) {
        fclose(fd);
        fd = NULL;
    }

    //
    if(pISPDrvObj != NULL) {
        MY_LOGD("pISPDrvObj uninit");
        ret = ret && pISPDrvObj->uninit(LOG_TAG);
        if(ret == MFALSE) {
            MY_LOGE("pISPDrvObj uninit fail");
        }
        pISPDrvObj->destroyInstance();
        pISPDrvObj = NULL;
    }

    FUNCTION_LOG_END;
    return ret;
}


MINT32
MfllShot::
Complement2(MUINT32 value, MUINT32 digit) const
{
    MINT32 Result;

    if (((value >> (digit - 1)) & 0x1) == 1)    // negative
    {
        Result = 0 - (MINT32)((~value + 1) & ((1 << digit) - 1));
    }
    else
    {
        Result = (MINT32)(value & ((1 << digit) - 1));
    }

    return Result;
}


};  //namespace NSCamShot

