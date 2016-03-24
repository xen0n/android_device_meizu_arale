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

#define LOG_TAG "MtkCam/P1NodeImp"
//
#include <mtkcam/Log.h>
#include "BaseNode.h"
#include "HwEventIrq.h"
#include <mtkcam/v3/hwnode/P1Node.h>
#include <mtkcam/v3/stream/IStreamInfo.h>
#include <mtkcam/v3/stream/IStreamBuffer.h>
#include <mtkcam/v3/utils/streambuf/IStreamBufferPool.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
#include <system/thread_defs.h>

//
#include <mtkcam/v3/hal/IHal3A.h>
#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <vector>
#include <iopipe/CamIO/PortMap.h>/*#include <iopipe_FrmB/CamIO/PortMap_FrmB.h>*/
#include <mtkcam/featureio/eis_hal.h>
#include <mtkcam/algorithm/libeis/MTKEis.h>
//
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
//
#include "Profile.h"
#include <mtkcam/Trace.h>
#include <cutils/properties.h>
#include <utils/Atomic.h>
//
#include <mtkcam/imageio/ispio_utility.h> //(SUPPORT_SCALING_CROP)
#include <mtkcam/metadata/IMetadataProvider.h>
#include "hwnode_utilities.h"
//
#include <mtkcam/utils/imagebuf/IIonImageBufferHeap.h>
#include <mtkcam/utils/imagebuf/IDummyImageBufferHeap.h>
//
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
//using namespace NSCam::Utils;
using namespace NSCam::Utils::Sync;
using namespace NSCam::NSIoPipe;
using namespace NSCam::NSIoPipe::NSCamIOPipe;
using namespace NS3Av3;
using namespace NSImageio;
using namespace NSIspio;

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
#define SUPPORT_3A              (1)
#define SUPPORT_ISP             (1)
#define SUPPORT_PERFRAME_CTRL   (0)
#define SUPPORT_EIS             (1)

#define SUPPORT_SCALING_CROP    (1)
#define SUPPORT_SCALING_CROP_IMGO   (SUPPORT_SCALING_CROP && (0))
#define SUPPORT_SCALING_CROP_RRZO   (SUPPORT_SCALING_CROP && (1))

#define FORCE_EIS_ON                (SUPPORT_EIS && (0))
#define DISABLE_BLOB_DUMMY_BUF      (0)

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_IN             MY_LOGD_IF(1<=mLogLevel, "+");
#define FUNCTION_OUT            MY_LOGD_IF(1<=mLogLevel, "-");
#define PUBLIC_API_IN           MY_LOGD_IF(1<=mLogLevel, "API +");
#define PUBLIC_API_OUT          MY_LOGD_IF(1<=mLogLevel, "API -");
#define MY_LOGD1(...)           MY_LOGD_IF(1<=mLogLevel, __VA_ARGS__)
#define MY_LOGD2(...)           MY_LOGD_IF(2<=mLogLevel, __VA_ARGS__)

#define P1THREAD_POLICY         (SCHED_OTHER)
#define P1THREAD_PRIORITY       (ANDROID_PRIORITY_FOREGROUND-2)

#define P1SOFIDX_INIT_VAL       (0)

/******************************************************************************
 *
 ******************************************************************************/
namespace {
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class AAAResult {
    protected:
        struct info{
            sp<IPipelineFrame> spFrame;
            IMetadata          resultVal;
            MUINT32            mFlag;
            info()
                : spFrame(0)
                , resultVal()
                , mFlag(0)
                {}
        };

    protected:
        enum KeyType {
            KeyType_StrobeState = 1,
            KeyType_Rest        = 2, //1,2,4,8,...
        };

    protected:
        mutable Mutex              mLock;
        KeyedVector<MUINT32, info> mData; //key: magicnum, val: info
        MUINT32                    mAllKey;

    public:

        AAAResult()
            : mLock()
            , mData()
            , mAllKey(KeyType_Rest)
            //, mAllKey(KeyType_StrobeState|KeyType_Rest)
            {}

        void add(MUINT32 magicNum, MUINT32 key, MUINT32 val)
        {
             Mutex::Autolock lock(mLock);
             if(key != MTK_FLASH_STATE) {
                 //unSupported
                 return;
             }

             IMetadata::IEntry entry(MTK_FLASH_STATE);
             entry.push_back(val, Type2Type< MUINT8 >()); //{MTK_FLASH_STATE, MUINT8}
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.resultVal.update(MTK_FLASH_STATE, entry);

    data.mFlag |= KeyType_StrobeState;
             }
        }

        void add(MUINT32 magicNum, sp<IPipelineFrame> pframe, IMetadata &rVal)
        {
             Mutex::Autolock lock(mLock);
             ssize_t i = mData.indexOfKey(magicNum);
             if(i < 0) {
                 info data;
                 data.spFrame = pframe;
                 data.resultVal = rVal;

data.mFlag |= KeyType_Rest;
                 mData.add(magicNum, data);
             } else {
                 info& data = mData.editValueFor(magicNum);
                 data.spFrame = pframe;
                 data.resultVal += rVal;
                 data.mFlag |= KeyType_Rest;
             }
        }

        const info& valueFor(const MUINT32& magicNum) const {
            return mData.valueFor(magicNum);
        }

        bool isCompleted(MUINT32 magicNum) {
            Mutex::Autolock lock(mLock);
            return (mData.valueFor(magicNum).mFlag & mAllKey) == mAllKey;
        }

        void removeItem(MUINT32 key) {
            Mutex::Autolock lock(mLock);
            mData.removeItem(key);
        }

        void clear() {
            debug();
            Mutex::Autolock lock(mLock);
            mData.clear();
        }

        void debug() {
            Mutex::Autolock lock(mLock);
            for(size_t i = 0; i < mData.size(); i++) {
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_StrobeState) == 0,
                           "No strobe result: (%d)", mData.keyAt(i));
                MY_LOGW_IF((mData.valueAt(i).mFlag & KeyType_Rest) == 0,
                           "No rest result: (%d)", mData.keyAt(i));
            }
        }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class Storage {

    protected:
        typedef DefaultKeyedVector<MINTPTR, sp<IImageBuffer> >  MapType;
        MapType                    mvStorageQ;
        mutable Mutex              mStorageLock;
        MINT32                     mLogEnable;
    public:
                                   Storage()
                                       : mvStorageQ()
                                       , mStorageLock()
                                       , mLogEnable(0)
                                       {}

        virtual                   ~Storage(){};

        void                       init(MINT32 logEnable)
                                   {
                                       mvStorageQ.clear();
                                       mLogEnable = logEnable;
                                   }

        void                       uninit()
                                   {
                                       mvStorageQ.clear();
                                   }

        void                       enque(sp<IImageStreamBuffer> const& key, sp<IImageBuffer> &value) {
                                       Mutex::Autolock lock(mStorageLock);
                                       MY_LOGD_IF(mLogEnable, "Storage-enque::(key)0x%x/(val)0x%x",
                                           key.get(), value.get());
                                       MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%x/%d/%d/%d/%d/%d",
                                        value->getBufPA(0),value->getImgSize().w, value->getImgSize().h,
                                        value->getBufStridesInBytes(0), value->getBufSizeInBytes(0), value->getPlaneCount());

                                       mvStorageQ.add(reinterpret_cast<MINTPTR>(key.get()), value);
                                   };


        sp<IImageBuffer>           deque(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           mvStorageQ.removeItem(key); //should un-mark
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%x/(val)0x%x",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "(val-pa)0x%x",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
        sp<IImageBuffer>           query(MINTPTR key) {
                                       Mutex::Autolock lock(mStorageLock);
                                       sp<IImageBuffer> pframe = mvStorageQ.valueFor(key);
                                       if (pframe != NULL)
                                       {
                                           MY_LOGD_IF(mLogEnable, "Storage-deque::(key)0x%x/(val)0x%x",
                                            key, pframe.get());
                                           MY_LOGD_IF(mLogEnable, "Info::(val-pa)0x%x",
                                            pframe->getBufPA(0));
                                           return pframe;
                                       }
                                       return NULL;
                                   }
};

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class DummyBuffer
    : public RefBase
{
    public:
        MINT32                  mLogLevel;
        MUINT32                 mSize;
        MUINT32                 mSlot;
        MUINT32                 mTotal;
        MINT32                  mHeapId;
        MUINTPTR                mVAddr;
        MUINTPTR                mPAddr;
        sp<IImageBuffer>        mDummyBuffer;

    public:
                                DummyBuffer(
                                    char const* szName,
                                    MINT32 enLog)
                                    : mLogLevel(enLog)
                                    , mSize(0)
                                    , mSlot(0)
                                    , mTotal(0)
                                    , mHeapId(-1)
                                    , mVAddr(0)
                                    , mPAddr(0)
                                    , mDummyBuffer(NULL)
                                    , mUsingCount(0)
                                    , mName(szName)
                                {
                                    MY_LOGD1("");
                                }

        virtual                 ~DummyBuffer()
                                {
                                    MY_LOGD1("");
                                };

        MERROR                  init(
                                    sp<IImageStreamInfo> const& imgStreamInfo,
                                    int numHardwareBuffer);

        MERROR                  uninit(void);

        MERROR                  count(MBOOL isIncrease)
                                {
                                    if (mDummyBuffer == NULL) {
                                        MY_LOGE("dummy buffer not create");
                                        return BAD_VALUE;
                                    }
                                    if (isIncrease) {
                                        mUsingCount ++;
                                    } else {
                                        mUsingCount --;
                                    }
                                    return OK;
                                };

    private:
        MINT32                  mUsingCount;
        android::String8        mName;
};

//#if (SUPPORT_SCALING_CROP)
#ifdef MAX
#undef MAX
#endif
#ifdef MIN
#undef MIN
#endif
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#ifdef ALIGN_UPPER
#undef ALIGN_UPPER
#endif
#ifdef ALIGN_LOWER
#undef ALIGN_LOWER
#endif
#define ALIGN_UPPER(x,a)    (((x)+((typeof(x))(a)-1))&~((typeof(x))(a)-1))
#define ALIGN_LOWER(x,a)    (((x))&~((typeof(x))(a)-1))

#ifdef RESIZE_RATIO_MAX_10X
#undef RESIZE_RATIO_MAX_10X
#endif
#define RESIZE_RATIO_MAX_10X   (4)

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MBOOL
tryGetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T & rVal
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return MFALSE;
    }
    //
    IMetadata::IEntry entry = pMetadata->entryFor(tag);
    if( !entry.isEmpty() ) {
        rVal = entry.itemAt(0, Type2Type<T>());
        return MTRUE;
    }
    return MFALSE;
}

/******************************************************************************
 *
 ******************************************************************************/
template <typename T>
inline MVOID
trySetMetadata(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    //
    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}

/******************************************************************************
 *
 ******************************************************************************/
inline MBOOL
isEISOn(
    IMetadata* const inApp
)
{
    if (inApp == NULL) {
        return false;
    }
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if(!tryGetMetadata<MUINT8>(inApp,
        MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode)) {
        MY_LOGW_IF(1, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
#if FORCE_EIS_ON
    eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
#endif
    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoFull(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize)
{
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        return false;
    }
    if ((querySrcRect.size().w > bufferSize.w || // cannot over buffer size
        querySrcRect.size().h > bufferSize.h) ||
        (((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
        ) {
        MY_LOGW("calculateCropInfoFull input invalid "
            "pixelMode(%d) sensorSize(%dx%d) bufferSize(%dx%d) "
            "querySrcRect_size(%dx%d) querySrcRect_start(%d,%d)", pixelMode,
            sensorSize.w, sensorSize.h, bufferSize.w, bufferSize.h,
            querySrcRect.size().w, querySrcRect.size().h,
            querySrcRect.leftTop().x, querySrcRect.leftTop().y);
        return false;
    }
    // TODO: query the valid value, currently do not crop in IMGO
    resultDstSize = MSize(sensorSize.w, sensorSize.h);
    resultSrcRect = MRect(MPoint(0, 0), resultDstSize);

    return true;
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL calculateCropInfoResizer(
    MUINT32 pixelMode,
    MSize const& sensorSize,
    MSize const& bufferSize,
    MRect const& querySrcRect,
    MRect& resultSrcRect,
    MSize& resultDstSize
)
{
    if ((querySrcRect.size().w == sensorSize.w) &&
        (querySrcRect.size().h == sensorSize.h)) {
        return false;
    }
    if ((((querySrcRect.leftTop().x + querySrcRect.size().w) > sensorSize.w) ||
        ((querySrcRect.leftTop().y + querySrcRect.size().h) > sensorSize.h))
        ) {
        MY_LOGW("calculateCropInfoResizer input invalid "
            "pixelMode(%d) sensorSize(%dx%d) bufferSize(%dx%d) "
            "querySrcRect_size(%dx%d) querySrcRect_start(%d,%d)", pixelMode,
            sensorSize.w, sensorSize.h, bufferSize.w, bufferSize.h,
            querySrcRect.size().w, querySrcRect.size().h,
            querySrcRect.leftTop().x, querySrcRect.leftTop().y);
        return false;
    }
    //
    MPoint::value_type src_crop_x = querySrcRect.leftTop().x;
    MPoint::value_type src_crop_y = querySrcRect.leftTop().y;
    MSize::value_type src_crop_w = querySrcRect.size().w;
    MSize::value_type src_crop_h = querySrcRect.size().h;
    MSize::value_type dst_size_w = 0;
    MSize::value_type dst_size_h = 0;
    if (querySrcRect.size().w < bufferSize.w) {
        dst_size_w = querySrcRect.size().w;
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;
        NSImageio::NSIspio::ISP_QuerySize(
                                NSImageio::NSIspio::EPortIndex_RRZO,
                                NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                eImgFmt_FG_BAYER10,
                                dst_size_w,
                                queryRst,
                                pixelMode == 0 ?
                                NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
                                NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
                                );
        dst_size_w = MIN((MSize::value_type)queryRst.x_pix, sensorSize.w);
        src_crop_w = dst_size_w;
        if (src_crop_w > querySrcRect.size().w) {
            if ((src_crop_x + src_crop_w) > sensorSize.w) {
                src_crop_x = sensorSize.w - src_crop_w;
            }
        }
    } else {
        if ((src_crop_w * RESIZE_RATIO_MAX_10X) > (bufferSize.w * 10)) {
            MY_LOGW("calculateCropInfoResizer resize width invalid "
                    "(%d):(%d)", src_crop_w, bufferSize.w);
            return false;
        }
        dst_size_w = bufferSize.w;
    }
    if (querySrcRect.size().h < bufferSize.h) {
        dst_size_h = querySrcRect.size().h;
        dst_size_h = MIN(ALIGN_UPPER(dst_size_h, 2), sensorSize.h);
        src_crop_h = dst_size_h;
        if (src_crop_h > querySrcRect.size().h) {
            if ((src_crop_y + src_crop_h) > sensorSize.h) {
                src_crop_y = sensorSize.h - src_crop_h;
            }
        }
    } else {
        if ((src_crop_h * RESIZE_RATIO_MAX_10X) > (bufferSize.h * 10)) {
            MY_LOGW("calculateCropInfoResizer resize height invalid "
                    "(%d):(%d)", src_crop_h, bufferSize.h);
            return false;
        }
        dst_size_h = bufferSize.h;
    }
    resultDstSize = MSize(dst_size_w, dst_size_h);
    resultSrcRect = MRect(MPoint(src_crop_x, src_crop_y),
                            MSize(src_crop_w, src_crop_h));
    return true;
}
//#endif

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  .
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
class P1NodeImp
    : public BaseNode
    , public P1Node
    , public IHal3ACb
    , protected Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    struct QueNode_T {
        MUINT32                            magicNum;
        MUINT32                            sofIdx;
        sp<IPipelineFrame>                 appFrame;
        sp<IImageStreamBuffer>             halFrame_full;
        sp<IImageStreamBuffer>             halFrame_resizer;
        sp<IImageBuffer>                   img_full;
        sp<IImageBuffer>                   img_resizer;
        #if (SUPPORT_SCALING_CROP)
        MSize                              dstSize_full;
        MSize                              dstSize_resizer;
        MRect                              cropRect_full;
        MRect                              cropRect_resizer;
        #endif
        QueNode_T()
            : magicNum(0)
            , sofIdx(P1SOFIDX_INIT_VAL)
            , appFrame(NULL)
            , halFrame_full(NULL)
            , halFrame_resizer(NULL)
            , img_full(NULL)
            , img_resizer(NULL)
        {}
    };

    typedef Vector<QueNode_T> Que_T;

protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    MBOOL                           mInit;

    SortedVector<StreamId_T>        mInStreamIds;
    sp<IMetaStreamInfo>             mInAppMeta;
    sp<IMetaStreamInfo>             mOutAppMeta;
    sp<IMetaStreamInfo>             mInHalMeta;
    sp<IMetaStreamInfo>             mOutHalMeta;
    ImageStreamInfoSetT             mvOutImage_full;
    sp<IImageStreamInfo>            mOutImage_resizer;
    SensorParams                    mSensorParams;
    sp<IImageStreamBufferPoolT>     mpStreamPool_full;
    sp<IImageStreamBufferPoolT>     mpStreamPool_resizer;

    Storage                         mImageStorage;

protected:  ////                    Data Members. (System capability)
    static const int                mNumInMeta = 2;
    static const int                mNumOutMeta = 3;
    int                             m3AProcessedDepth;
    int                             mNumHardwareBuffer;
    int                             mDelayframe;

protected:  ////
    MUINT32                         mlastNum;
    mutable Mutex                   mlastNumLock;

protected:  ////                    Data Members. (Hardware)
    mutable Mutex                   mHardwareLock;
    MBOOL                           mActive;
    INormalPipe_FrmB*               mpCamIO;
    IHal3A*                         mp3A;
    HwEventIrq*                     mpIEventIrq;
    #if SUPPORT_EIS
    EisHal_R*                       mpEIS;
    #endif
    #if SUPPORT_SCALING_CROP
    MRect                           mActiveArray;
    MUINT32                         mPixelMode;
    #endif
    sp<DummyBuffer>                 mDummyBufferFull;
    sp<DummyBuffer>                 mDummyBufferResizer;

protected:  ////                    Data Members. (Queue: Request)
    mutable Mutex                   mRequestQueueLock;
    Que_T                           mRequestQueue;

protected:  ////                    Data Members. (Queue: 3Alist)
    mutable Mutex                   mControls3AListLock;
    List<MetaSet_T>                 mControls3AList;
    Condition                       mControls3AListCond;

protected:  ////
    AAAResult                       m3AStorage;

protected:  ////                    Data Members. (Queue: Processing)
    mutable Mutex                   mProcessingQueueLock;
    Condition                       mProcessingQueueCond;
    Que_T                           mProcessingQueue;

protected:  ////                    Data Members. (Queue: drop)
    mutable Mutex                   mDropQueueLock;
    Vector<MUINT>                   mDropQueue;

protected:  ////                    Data Members.
    mutable Mutex                   mThreadLock;
    Condition                       mThreadCond;
    Que_T                           mdummyQueue;

protected:  ////                    Data Members.
    DurationProfile                 mDequeThreadProfile;

protected:  ////                    Data Members.
    mutable Mutex                   mPublicLock;

protected:  ////                    Data Members.
    MINT32                          mInFlightRequestCnt;

protected:
    MINT32                          mLogLevel;

protected:  ////                    Operations.
    MVOID                           onProcessFrame(
                                        MBOOL initial,
                                        MUINT32 magicNum = 0,
                                        MUINT32 sofIdx = P1SOFIDX_INIT_VAL
                                    );

    MVOID                           onProcess3AResult(
                                        MUINT32 magicNum,
                                        MUINT32 key,
                                        MUINT32 val
                                    );

    MVOID                           onProcessEIS(
                                        QueNode_T const &pFrame,
                                        IMetadata &resultEIS,
                                        QBufInfo const &deqBuf
                                    );

    MVOID                           onProcessEnqueFrame(
                                        QueNode_T &pFrame
                                    );

    MERROR                          onProcessDequedFrame(
                                    );

    MERROR                          onProcessDropFrame(
                                    );

    MBOOL                           getProcessingFrame_ByAddr(
                                        IImageBuffer* const imgBuffer,
                                        MUINT32 magicNum,
                                        QueNode_T &pFrame
                                    );

    QueNode_T                       getProcessingFrame_ByNumber(
                                        MUINT32 magicNum
                                    );


    MVOID                           onHandleFlush(
                                        MBOOL wait
                                    );

    MVOID                           onReturnProcessingFrame(
                                        QueNode_T const& pFrame,
                                        QBufInfo const &deqBuf,
                                        MetaSet_T const &result3A,
                                        IMetadata const &resultEIS
                                    );

    MVOID                           onFlushProcessingFrame(
                                        QueNode_T const& pFrame
                                    );

    MVOID                           onFlushRequestFrame(
                                        QueNode_T const& pFrame
                                    );

    MVOID                           createNode(sp<IPipelineFrame> appframe,
                                               Que_T *Queue,
                                               Mutex *QueLock,
                                               List<MetaSet_T> *list,
                                               Mutex *listLock
                                    );

    MVOID                           createNode(Que_T &Queue);

protected:  ////                    Hardware Operations.
    MERROR                          hardwareOps_start(
                                    );

    MERROR                          hardwareOps_enque(
                                        QueNode_T &pFrame,
                                        MBOOL dummy,
                                        MBOOL block
                                    );

    MERROR                          hardwareOps_deque(
                                        QBufInfo &deqBuf,
                                        MetaSet_T &result3A
                                    );

    MERROR                          hardwareOps_stop(
                                    );

    MERROR                          hwDummyBuffer_init(
                                        void
                                    );

    MERROR                          hwDummyBuffer_acquire(
                                        sp<DummyBuffer> const& dummyBuffer,
                                        size_t index,
                                        char const* szName,
                                        sp<IImageBuffer> & imageBuffer
                                    );

    MERROR                          hwDummyBuffer_return(
                                        void
                                    );

    MERROR                          hwDummyBuffer_release(
                                        void
                                    );

    MERROR                          hwDummyBuffer_uninit(
                                        void
                                    );

    MVOID                           generateAppMeta(
                                        sp<IPipelineFrame> const &request,
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata &appMetadata
                                    );

    MVOID                           generateHalMeta(
                                        MetaSet_T const &result3A,
                                        QBufInfo const &deqBuf,
                                        IMetadata const &resultEIS,
                                        IMetadata const &inHalMetadata,
                                        IMetadata &halMetadata
                                    );

    MVOID                           createFullBuf(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        MUINT32 num,
                                        MUINT32 idx,
                                        QBufInfo &enBuf
                                    );

    MVOID                           createResizerBuf(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        MUINT32 num,
                                        MUINT32 idx,
                                        QBufInfo &enBuf
                                    );

    #if (SUPPORT_SCALING_CROP)
    MVOID                           createFullBuf(
                                        QueNode_T const& node,
                                        QBufInfo &enBuf
                                    );

    MVOID                           createResizerBuf(
                                        QueNode_T const& node,
                                        QBufInfo &enBuf
                                    );

    MVOID                           prepareCropInfo(
                                       IMetadata* pMetadata,
                                       QueNode_T& node
                                   );
    #endif

    MERROR                          check_config(
                                        ConfigParams const& rParams
                                    );

protected:  ///

    MERROR                          lockMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        IMetadata &rMetadata
                                    );

    MERROR                          returnLockedMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        MBOOL success = MTRUE
                                    );

    MERROR                          returnUnlockedMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId
                                    );

    MERROR                          lock_and_returnMetadata(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        IMetadata &metadata
                                    );


    MERROR                          lockImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>  &pOutpImageStreamBuffer,
                                        sp<IImageBuffer> &rImageBuffer
                                    );

    MERROR                          returnLockedImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        MBOOL success = MTRUE
                                    );

    MERROR                          returnUnlockedImageBuffer(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId
                                    );

    MERROR                          lockImageBuffer(
                                        sp<IImageStreamBuffer> const& pStreamBuffer,
                                        sp<IImageBuffer> &pImageBuffer
                                    );

    MERROR                          returnLockedImageBuffer(
                                        sp<IImageBuffer> const &pImageBuffer,
                                        sp<IImageStreamBuffer> const &pStreamBuffer,
                                        sp<IImageStreamBufferPoolT> const &pStreamPool
                                    );

    MERROR                          returnUnlockedImageBuffer(
                                        sp<IImageStreamBuffer> const& pStreamBuffer,
                                        sp<IImageStreamBufferPoolT> const &pStreamPool
                                    );

    MUINT32                         get_and_increase_magicnum()
                                    {
                                        Mutex::Autolock _l(mlastNumLock);
                                        MUINT32 ret = mlastNum++;
                                        //skip num = 0 as 3A would callback 0 when request stack is empty
                                        //skip -1U as a reserved number to indicate that which would never happen in 3A queue
                                        if(ret==0 || ret==-1U) ret = mlastNum++;
                                        return ret;
                                    }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations in base class Thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    // Ask this object's thread to exit. This function is asynchronous, when the
    // function returns the thread might still be running. Of course, this
    // function can be called from a different thread.
    virtual void                    requestExit();

    // Good place to do one-time initializations
    virtual status_t                readyToRun();

private:
    // Derived class must implement threadLoop(). The thread starts its life
    // here. There are two ways of using the Thread object:
    // 1) loop: if threadLoop() returns true, it will be called again if
    //          requestExit() wasn't called.
    // 2) once: if threadLoop() returns false, the thread will exit upon return.
    virtual bool                    threadLoop();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    P1NodeImp();
    virtual                        ~P1NodeImp();
    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.

    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        sp<IPipelineFrame> pFrame
                                    );

public:     ////                    Operations.

    virtual void                    doNotifyCb (
                                        MINT32  _msgType,
                                        MINTPTR _ext1,
                                        MINTPTR _ext2,
                                        MINTPTR _ext3
                                    );

    static void                     doNotifyDropframe(MUINT magicNum, void* cookie);

};
};  //namespace


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::
P1NodeImp()
    : BaseNode()
    , P1Node()
    , mConfigRWLock()
    , mInit(MTRUE)

    , mImageStorage()
    //
    , m3AProcessedDepth(3)
    , mNumHardwareBuffer(3)
    , mDelayframe(3)
    , mlastNum(1)
    , mlastNumLock()
    , mHardwareLock()
    , mActive(MFALSE)
    , mpCamIO(NULL)
    , mp3A(NULL)
    , mpIEventIrq(NULL)
    #if SUPPORT_EIS
    , mpEIS(NULL)
    #endif
    //
    , mPixelMode(0)
    //
    , mDummyBufferFull(NULL)
    , mDummyBufferResizer(NULL)
    //
    , mRequestQueueLock()
    , mRequestQueue()
    //
    , mControls3AListLock()
    , mControls3AList()
    , mControls3AListCond()
    //
    , m3AStorage()
    //
    , mProcessingQueueLock()
    , mProcessingQueueCond()
    , mProcessingQueue()
    //
    , mDropQueueLock()
    , mDropQueue()
    //
    , mThreadLock()
    , mThreadCond()
    //
    , mdummyQueue()
    //
    , mDequeThreadProfile("P1Node::deque", 15000000LL)
    , mInFlightRequestCnt(0)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = ::atoi(cLogLevel);
    if ( 0 == mLogLevel ) {
        ::property_get("debug.camera.log.p1node", cLogLevel, "0");
        mLogLevel = ::atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::
~P1NodeImp()
{
    MY_LOGD("");
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
init(InitParams const& rParams)
{
    FUNCTION_IN

    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mOpenId  = rParams.openId;
        mNodeId  = rParams.nodeId;
        mNodeName= rParams.nodeName;
    }

    MERROR err = run();

    FUNCTION_OUT

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
uninit()
{
    PUBLIC_API_IN

    // flush the left frames if exist
    onHandleFlush(MFALSE);

    requestExit();

    hwDummyBuffer_uninit();

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
check_config(ConfigParams const& rParams)
{
    CAM_TRACE_CALL();

    if (rParams.pInAppMeta == NULL ) {
        MY_LOGE("in metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutAppMeta == NULL) {
        MY_LOGE("out app metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pOutHalMeta == NULL) {
        MY_LOGE("out hal metadata is null");
        return BAD_VALUE;
    }

    if (rParams.pvOutImage_full.size() == 0 && rParams.pOutImage_resizer == NULL) {
        MY_LOGE("image is empty");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_full == NULL && rParams.pStreamPool_resizer == NULL) {
        MY_LOGE("image pool is empty");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_full != NULL && rParams.pvOutImage_full.size() == 0) {
        MY_LOGE("wrong full input");
        return BAD_VALUE;
    }

    if (rParams.pStreamPool_resizer != NULL && rParams.pOutImage_resizer == NULL) {
        MY_LOGE("wrong resizer input");
        return BAD_VALUE;
    }

    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        if(rParams.pInAppMeta != 0) {
            mInAppMeta = rParams.pInAppMeta;
            mInStreamIds.add(mInAppMeta->getStreamId());
        }
        //
        if(rParams.pInHalMeta != 0) {
            mInHalMeta = rParams.pInHalMeta;
            mInStreamIds.add(mInHalMeta->getStreamId());
        }
        if(rParams.pOutAppMeta != 0) mOutAppMeta                   = rParams.pOutAppMeta;
        if(rParams.pOutHalMeta != 0) mOutHalMeta                   = rParams.pOutHalMeta;
        if(rParams.pvOutImage_full.size() != 0)   mvOutImage_full  = rParams.pvOutImage_full;
        if(rParams.pOutImage_resizer != 0)   mOutImage_resizer     = rParams.pOutImage_resizer;
        if(rParams.pStreamPool_full != 0)  mpStreamPool_full       = rParams.pStreamPool_full;
        if(rParams.pStreamPool_resizer != 0)  mpStreamPool_resizer = rParams.pStreamPool_resizer;
                                      mSensorParams                = rParams.sensorParams;
    }

    MY_LOGD1("[Config] In Meta Id: 0x%x, Out APP Meta Id: 0x%x, Out Hal Meta Id: 0x%x",
            mInAppMeta->getStreamId(), mOutAppMeta->getStreamId(), mOutHalMeta->getStreamId());

    for(size_t i = 0; i < mvOutImage_full.size(); i++) {
        MY_LOGD1("[Config] full image Id: 0x%x",  mvOutImage_full[i]->getStreamId());
    }
    if (mOutImage_resizer != NULL) {
        MY_LOGD1("[Config] resizer image Id: 0x%x",  mOutImage_resizer->getStreamId());
    }

#if (SUPPORT_SCALING_CROP)
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta = pMetadataProvider->geMtktStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, mActiveArray) ) {
            MY_LOGD_IF(1,"active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }
    }
#endif

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
config(ConfigParams const& rParams)
{
    PUBLIC_API_IN

    Mutex::Autolock _l(mPublicLock);
    //(1) check
    MERROR err = check_config(rParams);
    if (err != OK) {
        return err;
    }

    //(2) configure hardware

    if(mActive) {
        MY_LOGD("active=%d", mActive);
        onHandleFlush(MFALSE);
        err = hwDummyBuffer_uninit();
        if (err != OK) {
            return err;
        }
    }

    err = hwDummyBuffer_init();
    if (err != OK) {
        return err;
    }

    err = hardwareOps_start();
    if (err != OK) {
        return err;
    }

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
queue(
    sp<IPipelineFrame> pFrame
)
{
    PUBLIC_API_IN

    CAM_TRACE_CALL();
    MY_LOGD1("active=%d", mActive);
    if (!mActive) {
        hardwareOps_start();
    }

    {
        Mutex::Autolock _l(mControls3AListLock);

        //block condition 1: if pipeline is full
        while (mControls3AList.size() > (size_t)m3AProcessedDepth) {
            MY_LOGD1("wait: %d > %d", mControls3AList.size(), (size_t)m3AProcessedDepth);
            status_t status = mControls3AListCond.wait(mControls3AListLock);
            MY_LOGD1("wait-");
            if  ( OK != status ) {
                MY_LOGW(
                    "wait status:%d:%s, mControls3AList.size:%zu",
                    status, ::strerror(-status), mControls3AList.size()
                );
            }
        }
        //compensate to the number of mProcessedDepth
        while(mControls3AList.size() < (size_t)m3AProcessedDepth) {
            createNode(NULL, NULL, NULL, &mControls3AList, NULL);
        }

        //push node from appFrame
        createNode(pFrame, &mRequestQueue, &mRequestQueueLock, &mControls3AList, NULL);
        android_atomic_inc(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
flush()
{
    PUBLIC_API_IN

    CAM_TRACE_CALL();

    Mutex::Autolock _l(mPublicLock);

    onHandleFlush(MFALSE);

    //[TODO]
    //wait until deque thread going back to waiting state;
    //in case next node receives queue() after flush()

    PUBLIC_API_OUT

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
requestExit()
{
    FUNCTION_IN

    //let deque thread back
    Thread::requestExit();
    {
        Mutex::Autolock _l(mThreadLock);
        mThreadCond.broadcast();
    }
    join();

    //let enque thread back
    Mutex::Autolock _l(mControls3AListLock);
    mControls3AListCond.broadcast();

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P1NodeImp::
readyToRun()
{
    // set name
    ::prctl(PR_SET_NAME, (unsigned long)"Cam@P1NodeImp", 0, 0, 0);

    // set normal
    struct sched_param sched_p;
    sched_p.sched_priority = 0;
    ::sched_setscheduler(0, P1THREAD_POLICY, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, P1THREAD_PRIORITY);   //  Note: "priority" is nice value.
    //
    ::sched_getparam(0, &sched_p);
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P1NodeImp::
threadLoop()
{
    // check if going to leave thread
    CAM_TRACE_CALL();
    {
        Mutex::Autolock _l(mThreadLock);

        if (!mActive) {
            MY_LOGD("wait+");
            mThreadCond.wait(mThreadLock);
            MY_LOGD("wait-");
        }

        if (exitPending()) {
            MY_LOGD("leaving");
            return true;
        }
    }

    // deque buffer, and handle frame and metadata
    onProcessDequedFrame();


    // trigger point for the first time
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        if (mInit) {
            onProcessFrame(MTRUE);
            mInit = MFALSE;
        }
    }

    onProcessDropFrame();
    return true;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessFrame(
    MBOOL initial,
    MUINT32 magicNum,
    MUINT32 sofIdx
)
{
    FUNCTION_IN

    //(1)
    if(!initial) { // [TODO] && VALID 3A PROCESSED NOTIFY
        QueNode_T node;
        bool exist = false;
        {
            Mutex::Autolock _l(mRequestQueueLock);
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                if ((*it).magicNum == magicNum) {
                    node = *it;
                    node.sofIdx = sofIdx;
                    mRequestQueue.erase(it);
                    exist = true;
                    break;
                }
            }
        }
        if (exist) {
            onProcessEnqueFrame(node);
            Mutex::Autolock _ll(mProcessingQueueLock);
            mProcessingQueue.push_back(node);
        } else {
            MY_LOGW_IF(magicNum!=0, "no: %d", magicNum);
            Mutex::Autolock _l(mRequestQueueLock);
            String8 str;
            str += String8::format("[req/size(%d)]: ", mRequestQueue.size());
            Que_T::iterator it = mRequestQueue.begin();
            for(; it != mRequestQueue.end(); it++) {
                str += String8::format("%d ", (*it).magicNum);
            }
            MY_LOGD("%s", str.string());
        }
    }

    //(2)
    {
        Mutex::Autolock _l(mControls3AListLock);
        if (!mControls3AList.empty()) {
            mControls3AList.erase(mControls3AList.begin());
        }
        mControls3AListCond.broadcast();

        //dump
        String8 str("[3A]: ");
        List<MetaSet_T>::iterator it = mControls3AList.begin();
        for (; it != mControls3AList.end(); it++) {
            str += String8::format("%d ", it->halMeta.entryFor(MTK_P1NODE_PROCESSOR_MAGICNUM).itemAt(0, Type2Type< MINT32 >()));
        }
        MY_LOGD1("%s", str.string());

    }
    //(3)
    #if SUPPORT_3A
    if (mp3A) {
        mp3A->set(mControls3AList);
    }
    #endif
    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessEnqueFrame(
    QueNode_T &pFrame
)
{
    FUNCTION_IN

    //(1)
    //pass request directly if it's a reprocessing one
    //[TODO]
    //if( mInHalMeta == NULL) {
    //    onDispatchFrame(pFrame);
    //    return;
    //}

    //(2)
    hardwareOps_enque(pFrame, MFALSE, MTRUE);

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
P1NodeImp::QueNode_T
P1NodeImp::
getProcessingFrame_ByNumber(MUINT32 magicNum)
{
    FUNCTION_IN
    QueNode_T frame;

    Mutex::Autolock _l(mProcessingQueueLock);
    if (mProcessingQueue.empty()) {
        MY_LOGE("mProcessingQueue is empty");
        return frame;
    }

    #if 1
        Que_T::iterator it = mProcessingQueue.begin();
        for (; it != mProcessingQueue.end(); it++) {
            frame = *it;
            if (frame.magicNum == magicNum) {
                break;
            }
        }
        if (it == mProcessingQueue.end()) {
            MY_LOGE("cannot find the right node for num: %d", magicNum);
            return frame;
        }
        else {
            frame = *it;
            mProcessingQueue.erase(it);
            mProcessingQueueCond.broadcast();
        }
    #else
        frame = *mProcessingQueue.begin();
        mProcessingQueue.erase(mProcessingQueue.begin());
        mProcessingQueueCond.broadcast();
    #endif

    FUNCTION_OUT
    //
    return frame;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P1NodeImp::
getProcessingFrame_ByAddr(IImageBuffer* const imgBuffer,
                          MUINT32 magicNum,
                          QueNode_T &frame
)
{
    FUNCTION_IN

    MBOOL ret = MFALSE;
    if (imgBuffer == NULL) {
        MY_LOGE("imgBuffer == NULL");
        return ret;
    }

    // get the right node from mProcessingQueue
    Mutex::Autolock _l(mProcessingQueueLock);
    if (mProcessingQueue.empty()) {
        MY_LOGE("ProQ is empty");
        return ret;
    }

    Que_T::iterator it = mProcessingQueue.begin();
    for (; it != mProcessingQueue.end(); it++) {
        frame = *it;
        if (imgBuffer == (*it).img_full.get() ||
            imgBuffer == (*it).img_resizer.get()) {
            if (frame.magicNum == magicNum) {
                ret = MTRUE;
            } else {
                #if SUPPORT_PERFRAME_CTRL
                MY_LOGE("magicNum from driver(%d), should(%d)",
                       magicNum, frame.magicNum);
                #else
                if((magicNum & 0x40000000) != 0) {
                    MY_LOGW("magicNum from driver(%b) is uncertain",
                          magicNum);
                    ret = MFALSE;
                } else {
                    ret = MTRUE;
                    MY_LOGW("magicNum from driver(%d), should(%d)",
                          magicNum, frame.magicNum);
                }
                #endif
            }
            break;
        } else {
            continue;
        }
    }

    if (it == mProcessingQueue.end()) {
        MY_LOGE("no node with imagebuf(0x%x), PA(0x%x), num(%d)",
                 imgBuffer, imgBuffer->getBufPA(0), magicNum);
        for (Que_T::iterator it = mProcessingQueue.begin(); it != mProcessingQueue.end(); it++) {
            MY_LOGW("[proQ] num(%d)", (*it).magicNum);
            MY_LOGW_IF((*it).img_full!=NULL, "[proQ] imagebuf(0x%x), PA(0x%x)",
                (*it).img_full.get(), (*it).img_full->getBufPA(0));
            MY_LOGW_IF((*it).img_resizer!=NULL, "[proQ] imagebuf(0x%x), PA(0x%x)",
                (*it).img_resizer.get(), (*it).img_resizer->getBufPA(0));
        }
        for (Que_T::iterator it = mRequestQueue.begin(); it != mRequestQueue.end(); it++) {
            MY_LOGW("[reqQ] magic %d:", (*it).magicNum);
        }
    }
    else {
        frame = *it;
        mProcessingQueue.erase(it);
        mProcessingQueueCond.broadcast();
        MY_LOGD1("magic: %d", magicNum);
    }

    FUNCTION_OUT
    //
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDropFrame()
{
    Vector<QueNode_T > nodeQ;
    {
        Mutex::Autolock _l(mDropQueueLock);
        for(size_t i = 0; i < mDropQueue.size(); i++) {
            QueNode_T node = getProcessingFrame_ByNumber(mDropQueue[i]);
            nodeQ.push_back(node);
            MY_LOGD("drop: %d", mDropQueue[i]);
        }
        mDropQueue.clear();
    }

    for(size_t i = 0; i < nodeQ.size(); i++) {
         onFlushProcessingFrame(nodeQ[i]);
    }

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
onProcessDequedFrame()
{
    FUNCTION_IN

    CAM_TRACE_CALL();

    QBufInfo deqBuf;
    MetaSet_T result3A;
    if(hardwareOps_deque(deqBuf, result3A) != OK) {
        return BAD_VALUE;
    }

    QueNode_T pFrame;
    MBOOL match = getProcessingFrame_ByAddr(deqBuf.mvOut[0].mBuffer,
                                          deqBuf.mvOut[0].mMetaData.mMagicNum_hal,
                                          pFrame);
    MERROR ret;
    if (match == MFALSE) {
        onFlushProcessingFrame(pFrame);
        ret = BAD_VALUE;
    }
    else {
        IMetadata resultEIS;
        IMetadata inAPP;
        if(OK == lockMetadata(pFrame.appFrame, mInAppMeta->getStreamId(), inAPP)){
            if (isEISOn(&inAPP)) {
                onProcessEIS(pFrame, resultEIS, deqBuf);
            }
        }
        onReturnProcessingFrame(pFrame, deqBuf, result3A, resultEIS);
        ret = OK;
    }

    FUNCTION_OUT

    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onHandleFlush(
    MBOOL wait
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    //wake up queue thread.
    {
        Mutex::Autolock _l(mControls3AListLock);
        mControls3AListCond.broadcast();
    }

    //stop hardware
    if (!wait) {
        hardwareOps_stop(); //include hardware and 3A
    }

    //(1) clear request queue
    {
        Mutex::Autolock _l(mRequestQueueLock);
        while(!mRequestQueue.empty()) {
            QueNode_T pFrame = *mRequestQueue.begin();
            mRequestQueue.erase(mRequestQueue.begin());
            onFlushRequestFrame(pFrame);
        }
    }

    //(2) clear processing queue
    //     wait until processing frame coming out
    if (wait) {
        Mutex::Autolock _l(mProcessingQueueLock);
        while(!mProcessingQueue.empty()) {
            mProcessingQueueCond.wait(mProcessingQueueLock);
        }
    } else {
        // must guarantee hardware has been stopped.
        Mutex::Autolock _l(mProcessingQueueLock);
        while(!mProcessingQueue.empty()) {
            QueNode_T pFrame = *mProcessingQueue.begin();
            mProcessingQueue.erase(mProcessingQueue.begin());
            onFlushProcessingFrame(pFrame);
        }
    }

    //(3) clear dummy queue
    while(!mdummyQueue.empty()) {
        QueNode_T pFrame = *mdummyQueue.begin();
        onFlushProcessingFrame(pFrame);
        mdummyQueue.erase(mdummyQueue.begin());
        //
        hwDummyBuffer_return();
    }


    //(4) clear drop frame queue
    onProcessDropFrame();


    //(5) clear all
    mRequestQueue.clear(); //suppose already clear
    mProcessingQueue.clear(); //suppose already clear
    mControls3AList.clear();
    mImageStorage.uninit();
    m3AStorage.clear();
    mlastNum = 1;

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onReturnProcessingFrame(
    QueNode_T const &pFrame,
    QBufInfo const &deqBuf,
    MetaSet_T const &result3A,
    IMetadata const &resultEIS
)
{
    FUNCTION_IN

    CAM_TRACE_CALL();


    if (pFrame.appFrame != 0) {

        // Out Image Stream
        Vector<StreamId_T> streamId_Images;
        if (mOutImage_resizer != NULL) {
            streamId_Images.push_back(mOutImage_resizer->getStreamId());
        }
        for(size_t i = 0; i < mvOutImage_full.size(); i++) {
            streamId_Images.push_back(mvOutImage_full[i]->getStreamId());
        }

        for(size_t i = 0; i < streamId_Images.size(); i++) {
            StreamId_T const streamId_Image = streamId_Images[i];
            returnLockedImageBuffer(pFrame.appFrame, streamId_Image);
        }

        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InAppMeta);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL){
            IMetadata appMetadata;
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            generateAppMeta(pFrame.appFrame, result3A, deqBuf, appMetadata);
            m3AStorage.add(pFrame.magicNum, pFrame.appFrame, appMetadata);
            appMetadata = m3AStorage.valueFor(pFrame.magicNum).resultVal;
            if(m3AStorage.isCompleted(pFrame.magicNum)) {
                lock_and_returnMetadata(pFrame.appFrame, streamId_OutAppMeta, appMetadata);
                m3AStorage.removeItem(pFrame.magicNum);
            }
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL){
            IMetadata inHalMetadata;
            IMetadata outHalMetadata;
            StreamId_T const streamId_OutHalMeta = mOutHalMeta->getStreamId();

            if (mInHalMeta != NULL) lockMetadata(pFrame.appFrame, mInHalMeta->getStreamId(), inHalMetadata);
            generateHalMeta(result3A, deqBuf, resultEIS, inHalMetadata, outHalMetadata);
            lock_and_returnMetadata(pFrame.appFrame, streamId_OutHalMeta, outHalMetadata);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InHalMeta);
        }

        // Apply buffers to release
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return OK]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));

    }
    else {

        if (pFrame.halFrame_full != 0) {
            returnLockedImageBuffer(pFrame.img_full, pFrame.halFrame_full, mpStreamPool_full);
        }

        if (pFrame.halFrame_resizer != 0) {
            returnLockedImageBuffer(pFrame.img_resizer, pFrame.halFrame_resizer, mpStreamPool_resizer);
        }
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onFlushRequestFrame(
    QueNode_T const& pFrame
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    if (pFrame.appFrame != 0) {

        // Out Image Stream
        Vector<StreamId_T> streamId_Images;
        if (mOutImage_resizer != NULL) {
            streamId_Images.push_back(mOutImage_resizer->getStreamId());
        }
        for(size_t i = 0; i < mvOutImage_full.size(); i++) {
            streamId_Images.push_back(mvOutImage_full[i]->getStreamId());
        }

        for(size_t i = 0; i < streamId_Images.size(); i++) {
            StreamId_T const streamId_Image = streamId_Images[i];
            returnUnlockedImageBuffer(pFrame.appFrame, streamId_Image);
        }


        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_InAppMeta);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_InHalMeta);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL) {
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_OutAppMeta);
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL) {
            StreamId_T const streamId_OutHalMeta = mOutHalMeta->getStreamId();
            returnUnlockedMetadata(pFrame.appFrame, streamId_OutHalMeta);
        }

        // Apply buffers to release.
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return flush]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }
    else {
        if (pFrame.halFrame_full != 0) {
            returnUnlockedImageBuffer(pFrame.halFrame_full, mpStreamPool_full);
        }
        if (pFrame.halFrame_resizer != 0) {
            returnUnlockedImageBuffer(pFrame.halFrame_resizer, mpStreamPool_resizer);
        }
    }


    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onFlushProcessingFrame(
    QueNode_T const& pFrame
)
{
    FUNCTION_IN
    CAM_TRACE_CALL();

    if (pFrame.appFrame != 0) {
        MBOOL success = MFALSE;

        // Out Image Stream
        Vector<StreamId_T>      streamId_Images;
        if (mOutImage_resizer != NULL) {
            streamId_Images.push_back(mOutImage_resizer->getStreamId());
        }
        for(size_t i = 0; i < mvOutImage_full.size(); i++) {
            streamId_Images.push_back(mvOutImage_full[i]->getStreamId());
        }

        for(size_t i = 0; i < streamId_Images.size(); i++) {
            StreamId_T const streamId_Image = streamId_Images[i];
            returnLockedImageBuffer(pFrame.appFrame, streamId_Image, success);
        }

        // APP in Meta Stream
        if (mInAppMeta != NULL) {
            StreamId_T const streamId_InAppMeta = mInAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InAppMeta, success);
        }

        // HAL in Meta Stream
        if (mInHalMeta != NULL) {
            StreamId_T const streamId_InHalMeta = mInHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_InHalMeta, success);
        }

        // APP out Meta Stream
        if (mOutAppMeta != NULL) {
            StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_OutAppMeta, success);
        }

        // HAL out Meta Stream
        if (mOutHalMeta != NULL) {
            StreamId_T const streamId_OutHalMeta  = mOutHalMeta->getStreamId();
            returnLockedMetadata(pFrame.appFrame, streamId_OutHalMeta, success);
        }

        // Apply buffers to release
        IStreamBufferSet& rStreamBufferSet  = pFrame.appFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());

        // dispatch to next node
        onDispatchFrame(pFrame.appFrame);
        MY_LOGD1("[return flush]: (%d, %d)", pFrame.appFrame->getFrameNo(), pFrame.magicNum);
        android_atomic_dec(&mInFlightRequestCnt);
        ATRACE_INT("P1_request_cnt", android_atomic_acquire_load(&mInFlightRequestCnt));
    }
    else {
        if (pFrame.halFrame_full != 0) {
            returnLockedImageBuffer(pFrame.img_full, pFrame.halFrame_full, mpStreamPool_full);
        }
        else if ((mDummyBufferFull != NULL) && (pFrame.img_full != 0)) {
            if (pFrame.img_full.get() ==
                (*mdummyQueue.begin()).img_full.get()) {
                pFrame.img_full->unlockBuf(getNodeName());
                //MY_LOGD("dummy buffer unlockBuf - full");
            }
        }
        if (pFrame.halFrame_resizer != 0) {
            returnLockedImageBuffer(pFrame.img_resizer, pFrame.halFrame_resizer, mpStreamPool_resizer);
        }
        else if ((mDummyBufferResizer != NULL) && (pFrame.img_resizer != 0)) {
            if (pFrame.img_resizer.get() ==
                (*mdummyQueue.begin()).img_resizer.get()) {
                pFrame.img_resizer->unlockBuf(getNodeName());
                //MY_LOGD("dummy buffer unlockBuf - resizer");
            }
        }
    }

    FUNCTION_OUT
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcess3AResult(
    MUINT32 magicNum,
    MUINT32 key,
    MUINT32 val
)
{
    MY_LOGD2("%d", magicNum);

    if(magicNum == 0) return;

    m3AStorage.add(magicNum, key, val);
    if(m3AStorage.isCompleted(magicNum)) {
        sp<IPipelineFrame> spFrame = m3AStorage.valueFor(magicNum).spFrame;
        StreamId_T const streamId_OutAppMeta = mOutAppMeta->getStreamId();
        IMetadata appMetadata = m3AStorage.valueFor(magicNum).resultVal;
        lock_and_returnMetadata(spFrame, streamId_OutAppMeta, appMetadata);
        m3AStorage.removeItem(magicNum);

        IStreamBufferSet& rStreamBufferSet  = spFrame->getStreamBufferSet();
        rStreamBufferSet.applyRelease(getNodeId());
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
onProcessEIS(
    QueNode_T const &pFrame,
    IMetadata &resultEIS,
    QBufInfo const &deqBuf
)
{
    if(pFrame.appFrame ==NULL) {
        return;
    }
    #if SUPPORT_EIS
    if (deqBuf.mvOut.size() == 0) {
        MY_LOGE("deqBuf is empty");
        return;
    }
    MUINT64 timestamp = deqBuf.mvOut[0].mMetaData.mTimeStamp;
    EIS_HAL_CONFIG_DATA config;
    config.p1ImgW = mSensorParams.size.w; // [AWARE] need to revise by platform
    config.p1ImgH = mSensorParams.size.h; //
    #if 1 // use RRZO DstSize
    for (size_t i = 0; i < deqBuf.mvOut.size(); i++) {
        if (deqBuf.mvOut[i].mPortID == PORT_RRZO) {
            config.p1ImgW = deqBuf.mvOut[i].mMetaData.mDstSize.w;
            config.p1ImgH = deqBuf.mvOut[i].mMetaData.mDstSize.h;
            break;
        } else {
            continue;
        }
    }
    #endif
    mpEIS->DoEis(EIS_PASS_1, &config, timestamp);

    MUINT32 X_INT, Y_INT, X_FLOAT, Y_FLOAT, WIDTH, HEIGHT;
    mpEIS->GetEisResult(X_INT, X_FLOAT, Y_INT, Y_FLOAT, WIDTH, HEIGHT);
    IMetadata::IEntry entry(MTK_EIS_REGION);
    entry.push_back(X_INT, Type2Type< MINT32 >());
    entry.push_back(X_FLOAT, Type2Type< MINT32 >());
    entry.push_back(Y_INT, Type2Type< MINT32 >());
    entry.push_back(Y_FLOAT, Type2Type< MINT32 >());
    entry.push_back(WIDTH, Type2Type< MINT32 >());
    entry.push_back(HEIGHT, Type2Type< MINT32 >());
    resultEIS.update(MTK_EIS_REGION, entry);
    MY_LOGD("(%dx%d) %d, %d, %d, %d, %d, %d", config.p1ImgW, config.p1ImgH,
        X_INT, X_FLOAT, Y_INT, Y_FLOAT, WIDTH, HEIGHT);
    #endif
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyCb(
    MINT32  _msgType,
    MINTPTR _ext1,
    MINTPTR _ext2,
    MINTPTR /*_ext3*/
)
{
    switch(_msgType)
    {
        case IHal3ACb::eID_NOTIFY_3APROC_FINISH:
            onProcessFrame(MFALSE, (MUINT32)_ext1, (MUINT32)_ext2);
            break;
        case IHal3ACb::eID_NOTIFY_CURR_RESULT:
            //onProcess3AResult((MUINT32)_ext1,(MUINT32)_ext2, (MUINT32)_ext3); //magic, key, val
            break;
        default:
            break;
    }
}


/******************************************************************************
 *
 ******************************************************************************/
void
P1NodeImp::
doNotifyDropframe(MUINT magicNum, void* cookie)
{
   if (cookie == NULL) {
       MY_LOGE("return cookie is NULL");
       return;
   }

   Mutex::Autolock _l(reinterpret_cast<P1NodeImp*>(cookie)->mDropQueueLock);
   reinterpret_cast<P1NodeImp*>(cookie)->mDropQueue.push_back(magicNum);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
init(sp<IImageStreamInfo> const& imgStreamInfo, int numHardwareBuffer)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    if (imgStreamInfo == NULL) {
        MY_LOGE("imgStreamInfo is NULL");
        return BAD_VALUE;
    }
    mSize = 0;
    for (size_t i = 0; i < imgStreamInfo->getBufPlanes().size(); i++) {
        mSize += imgStreamInfo->getBufPlanes()[i].sizeInBytes;
    }
    mSlot = numHardwareBuffer;
    mTotal = mSize + (sizeof(MINTPTR) * (mSlot - 1));
    IImageBufferAllocator::ImgParam imgParam(mTotal, 0);
    sp<IIonImageBufferHeap> pHeap =
        IIonImageBufferHeap::create(mName.string(), imgParam);
    if (pHeap == NULL) {
        MY_LOGE("[%s] image buffer heap create fail", mName.string());
        return BAD_VALUE;
    }
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    sp<IImageBuffer> pImageBuffer = pHeap->createImageBuffer();
    if (pImageBuffer == NULL) {
        MY_LOGE("[%s] image buffer create fail", mName.string());
        return BAD_VALUE;
    }
    if (!(pImageBuffer->lockBuf(mName.string(), usage))) {
        MY_LOGE("[%s] image buffer lock fail", mName.string());
        return BAD_VALUE;
    }
    mVAddr = pHeap->getBufVA(0);
    mPAddr = pHeap->getBufPA(0);
    mHeapId = pHeap->getHeapID();
    mDummyBuffer = pImageBuffer;
    mUsingCount = 0;
    MY_LOGD1("DummyBufferHeap[%s] Len(%d, %d, %d) VA(%p) PA(%p) ID(%d)",
        mName.string(), mSize, mSlot, mTotal, mVAddr, mPAddr, mHeapId);

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
DummyBuffer::
uninit(void)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    if (mUsingCount > 0) {
        MY_LOGE("dummy buffer [%s] is using (%d) - should not uninit",
            mName.string(), mUsingCount);
        return BAD_VALUE;
    }
    if (mDummyBuffer != NULL) {
        mDummyBuffer->unlockBuf(mName.string());
        mDummyBuffer = NULL;
        mVAddr = 0;
        mPAddr = 0;
        mSize = 0;
        mSlot = 0;
        mTotal = 0;
        mUsingCount = 0;
    }

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_init(void)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    #if DISABLE_BLOB_DUMMY_BUF
    return err;
    #endif
    //
    if ((mDummyBufferFull != NULL) || (mDummyBufferResizer != NULL)) {
        err = hwDummyBuffer_release();
        if (err != OK) {
            MY_LOGE("init - release dummy buffer fail");
            return err;
        }
    }
    //
    if (mvOutImage_full.size() != 0) {
        sp<DummyBuffer> pDummyBuffer =
            new DummyBuffer("Hal:Image:P1:Fulldummy", mLogLevel);
        err = pDummyBuffer->init(mvOutImage_full[0], mNumHardwareBuffer);
        if (err != OK) {
            MY_LOGE("init - full dummy buffer fail");
            return err;
        }
        mDummyBufferFull = pDummyBuffer;
    }
    //
    if (mOutImage_resizer != NULL) {
        sp<DummyBuffer> pDummyBuffer =
            new DummyBuffer("Hal:Image:P1:Resizedummy", mLogLevel);
        err = pDummyBuffer->init(mOutImage_resizer, mNumHardwareBuffer);
        if (err != OK) {
            MY_LOGE("init - resizer dummy buffer fail");
            return err;
        }
        mDummyBufferResizer = pDummyBuffer;
    }

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_acquire(
    sp<DummyBuffer> const& dummyBuffer,
    size_t index,
    char const* szName,
    sp<IImageBuffer> & imageBuffer
)
{
    FUNCTION_IN

    MERROR err = OK;
    MUINT const usage = (GRALLOC_USAGE_SW_READ_OFTEN |
                        GRALLOC_USAGE_HW_CAMERA_READ |
                        GRALLOC_USAGE_HW_CAMERA_WRITE);
    //
    if (dummyBuffer == NULL) {
        MY_LOGE("acquire [%s] - dummy buffer NULL", szName);
        return BAD_VALUE;
    }
    //
    if (index >= dummyBuffer->mSlot) {
        MY_LOGE("acquire [%s] - dummy buffer index out of range (%d >= %d)",
            szName, index, dummyBuffer->mSlot);
        return BAD_VALUE;
    }
    // use eImgFmt_BLOB for dummy buffer, plane count is 1
    size_t bufStridesInBytes[] = {dummyBuffer->mSize, 0, 0};
    size_t bufBoundaryInBytes[] = {0, 0, 0};
    MINT32 const memId[] = {dummyBuffer->mHeapId, 0, 0};
    MUINTPTR const cVAddr = dummyBuffer->mVAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const virtAddr[] = {cVAddr, 0, 0};
    MUINTPTR const cPAddr = dummyBuffer->mPAddr + (sizeof(MINTPTR) * (index));
    MUINTPTR const phyAddr[] = {cPAddr, 0, 0};
    IImageBufferAllocator::ImgParam imgParam =
                        IImageBufferAllocator::ImgParam(dummyBuffer->mSize, 0);
    PortBufInfo_dummy portBufInfo = PortBufInfo_dummy(
                                        memId,
                                        virtAddr,
                                        phyAddr,
                                        1);
    sp<IImageBufferHeap> imgBufHeap = IDummyImageBufferHeap::create(
                                        szName,
                                        imgParam,
                                        portBufInfo,
                                        (1 <= mLogLevel) ? true : false);
    if (imgBufHeap == NULL) {
        MY_LOGE("acquire [%s] - image buffer heap create fail", szName);
        return UNKNOWN_ERROR;
    }
    sp<IImageBuffer> imgBuf = imgBufHeap->createImageBuffer();
    if (imgBuf == NULL) {
        MY_LOGE("acquire [%s] - image buffer create fail", szName);
        return BAD_VALUE;
    }
    if (!(imgBuf->lockBuf(szName, usage))) {
        MY_LOGE("acquire [%s] - image buffer lock fail", szName);
        return BAD_VALUE;
    }
    imageBuffer = imgBuf;
    dummyBuffer->count(true);

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_return(void)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    if (mDummyBufferFull != NULL) {
        err = mDummyBufferFull->count(false);
        if (err != OK) {
            MY_LOGE("set count - full dummy buffer fail");
            return err;
        }
    }
    //
    if (mDummyBufferResizer != NULL) {
        err = mDummyBufferResizer->count(false);
        if (err != OK) {
            MY_LOGE("set count - resizer dummy buffer fail");
            return err;
        }
    }

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_release(void)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    if (mDummyBufferFull != NULL) {
        err = mDummyBufferFull->uninit();
        if (err != OK) {
            MY_LOGE("release - full dummy buffer fail");
            return err;
        }
        mDummyBufferFull = NULL;
    }
    //
    if (mDummyBufferResizer != NULL) {
        err = mDummyBufferResizer->uninit();
        if (err != OK) {
            MY_LOGE("release - resizer dummy buffer fail");
            return err;
        }
        mDummyBufferResizer = NULL;
    }

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hwDummyBuffer_uninit(void)
{
    FUNCTION_IN

    MERROR err = OK;
    //
    err = hwDummyBuffer_release();
    if (err != OK) {
        MY_LOGE("uninit - free dummy buffer fail");
        return err;
    }

    FUNCTION_OUT
    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_start()
{
#if SUPPORT_ISP
    FUNCTION_IN
    CAM_TRACE_CALL();

    Mutex::Autolock _l(mHardwareLock);

    mActive = MTRUE;
    mInit   = MTRUE;

    mDequeThreadProfile.reset();
    mImageStorage.init(mLogLevel);

    //
    CAM_TRACE_BEGIN("isp init");
    mpCamIO = INormalPipe_FrmB::createInstance(getOpenId(), getNodeName(), 1); // burstQnum --> for slow motion
    if(!mpCamIO || !mpCamIO->init())
    {
        MY_LOGE("hardware init fail");
        return DEAD_OBJECT;
    }
    CAM_TRACE_END();

    //
    IHalSensor::ConfigParam sensorCfg =
    {
        (MUINT)getOpenId(),                 /* index            */
        mSensorParams.size,                 /* crop */
        mSensorParams.mode,                 /* scenarioId       */
        0,                                  /* isBypassScenario */
        1,                                  /* isContinuous     */
        MFALSE,                             /* iHDROn           */
        mSensorParams.fps,                  /* framerate        */
        0,                                  /* two pixel on     */
        0,                                  /* debugmode        */
    };

    vector<IHalSensor::ConfigParam> vSensorCfg;
    vSensorCfg.push_back(sensorCfg);

    //
    vector<portInfo> vPortInfo;
    if (mvOutImage_full.size() != 0)
    {
        portInfo OutPort(
                PORT_IMGO,
                (EImageFormat)mvOutImage_full[0]->getImgFormat(),
                mvOutImage_full[0]->getImgSize(),
                MRect(MPoint(0,0), mSensorParams.size),
                mvOutImage_full[0]->getBufPlanes().itemAt(0).rowStrideInBytes,
                0, //pPortCfg->mStrideInByte[1],
                0, //pPortCfg->mStrideInByte[2],
                0, // pureraw
                MTRUE);              //packed

        vPortInfo.push_back(OutPort);
    }


    if (mOutImage_resizer != NULL)
    {
        portInfo OutPort(
                PORT_RRZO,
                (EImageFormat)mOutImage_resizer->getImgFormat(),
                mOutImage_resizer->getImgSize(),
                MRect(MPoint(0,0), mSensorParams.size),
                mOutImage_resizer->getBufPlanes().itemAt(0).rowStrideInBytes,
                0, //pPortCfg->mStrideInByte[1],
                0, //pPortCfg->mStrideInByte[2],
                0, // pureraw
                MTRUE);              //packed

        vPortInfo.push_back(OutPort);
    }

    QInitParam halCamIOinitParam(
               0,                           /*sensor test pattern */
               10,                          /* bit depth*/
               vSensorCfg,
               vPortInfo);

    halCamIOinitParam.m_DropCB = doNotifyDropframe;
    halCamIOinitParam.m_returnCookie = this;


    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3a createinstance");
    mp3A = IHal3A::createInstance(IHal3A::E_Camera_3, getOpenId(), getNodeName());
    mp3A->setSensorMode(mSensorParams.mode);
    CAM_TRACE_END();
    #endif

    //
    CAM_TRACE_BEGIN("isp config");
    if(!mpCamIO->configPipe(halCamIOinitParam))
    {
        MY_LOGE("hardware config pipe fail");
        return BAD_VALUE;
    }
    CAM_TRACE_END();

    #if SUPPORT_EIS
    CAM_TRACE_BEGIN("eis config");
    mpEIS = EisHal_R::CreateInstance(LOG_TAG, getOpenId());
    mpEIS->Init();
    EIS_HAL_CONFIG_DATA  config;
    config.sensorType = IHalSensorList::get()->queryType(getOpenId());
    //config.memAlignment //[AWARE] may need to modify by platform
    config.configSce = EIS_SCE_EIS;

    mpEIS->ConfigEis(EIS_PASS_1, config);
    CAM_TRACE_END();
    #endif


    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3A start");
    if (mp3A) {
        mp3A->start();
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
        mp3A->attachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
        //m3AProcessedDepth = mp3A->getCapacity();
    }
    CAM_TRACE_END();
    #endif


    //register 3 real frames and 3 dummy frames
    //[TODO] in case that delay frame is above 3 but memeory has only 3, pending aquirefromPool
    CAM_TRACE_BEGIN("create node");
    {
        for (int i = 0; i < mNumHardwareBuffer; i++) {
            createNode(NULL, &mProcessingQueue, &mProcessingQueueLock,
                             &mControls3AList, &mControls3AListLock);
            hardwareOps_enque(mProcessingQueue.editItemAt(mProcessingQueue.size()-1), MFALSE, MFALSE);
        }
        for (int i = 0; i < mNumHardwareBuffer; i++) {
            createNode(mdummyQueue);
            hardwareOps_enque(mdummyQueue.editItemAt(mdummyQueue.size()-1), MTRUE, MFALSE);
        }
        // Due to pipeline latency, delay frame should be above 3
        // if delay frame is more than 3, add node to mProcessingQueue here.
        for (int i = 0; i < mDelayframe - mNumHardwareBuffer; i++) {
            createNode(NULL, &mProcessingQueue, &mProcessingQueueLock,
                             &mControls3AList, &mControls3AListLock);
        }
    }
    CAM_TRACE_END();
    //
    CAM_TRACE_BEGIN("isp start");
    if(!mpCamIO->start()) {
        MY_LOGE("hardware start fail");
        return BAD_VALUE;
    }
    CAM_TRACE_END();


    MINT32 tgInfo = HwEventIrq::E_NONE;
    IHalSensorList* const pIHalSensorList = IHalSensorList::get();
    IHalSensor* pHalSensor = pIHalSensorList->createSensor(LOG_TAG, getOpenId());
    MUINT32 sensorDev = (MUINT32)pIHalSensorList->querySensorDevIdx(getOpenId());
    SensorDynamicInfo senInfo;
    MINT32 i4SensorDevId = pHalSensor->querySensorDynamicInfo(sensorDev, &senInfo);
    pHalSensor->destroyInstance(LOG_TAG);
    MY_LOGD("sensorDev = %d, senInfo.TgInfo = %d", sensorDev, senInfo.TgInfo);
    switch (senInfo.TgInfo)
    {
        case CAM_TG_1:
            tgInfo = HwEventIrq::E_TG1;
            break;
        case CAM_TG_2:
            tgInfo = HwEventIrq::E_TG2;
            break;
        default:
            break;
    }

    HwEventIrq::ConfigParam irqConfig(sensorDev, tgInfo, HwEventIrq::E_Event_Vsync);
    mpIEventIrq = HwEventIrq::createInstance(irqConfig, "P1Node");


    {
        Mutex::Autolock _l(mThreadLock);
        mThreadCond.broadcast();
    }

    FUNCTION_OUT

    return OK;
#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createFullBuf(sp<IImageBuffer> const &pImageBuffer, MUINT32 num, MUINT32 idx, QBufInfo &enBuf)
{
    if (mvOutImage_full.size() == 0) {
        return;
    }

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_IMGO,
        pImageBuffer.get(),
        mvOutImage_full[0]->getImgSize(),
        MRect(MPoint(0, 0), mSensorParams.size),
        num,
        idx);

    enBuf.mvOut.push_back(rBufInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createResizerBuf(sp<IImageBuffer> const &pImageBuffer, MUINT32 num, MUINT32 idx, QBufInfo &enBuf)
{
    if (mOutImage_resizer == NULL) {
        return;
    }

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo(
        PORT_RRZO,
        pImageBuffer.get(),
        mOutImage_resizer->getImgSize(),
        MRect(MPoint(0, 0), mSensorParams.size),
        num,
        idx);

    enBuf.mvOut.push_back(rBufInfo);
}

#if (SUPPORT_SCALING_CROP)
/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createFullBuf(QueNode_T const& node, QBufInfo &enBuf)
{
    if (mvOutImage_full.size() == 0) {
        return;
    }

    MSize dstSize = node.dstSize_full;
    MRect cropRect = node.cropRect_full;

    if (mInit) {
        MY_LOGD("createFullBuf mInit : (%d)", mInit);
        dstSize = mvOutImage_full[0]->getImgSize();
        cropRect = MRect(MPoint(0, 0), mSensorParams.size);
    }
    /*
    MY_LOGD("[CropInfo] (%d,%d-%dx%d)(%dx%d)", cropRect.p.x, cropRect.p.y,
        cropRect.s.w, cropRect.s.h, dstSize.w, dstSize.h);
    */

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_IMGO,
        node.img_full.get(),
        dstSize,
        cropRect,
        node.magicNum,
        node.sofIdx);

    enBuf.mvOut.push_back(rBufInfo);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
createResizerBuf(QueNode_T const& node, QBufInfo &enBuf)
{
    if (mOutImage_resizer == NULL) {
        return;
    }

    MSize dstSize = node.dstSize_resizer;
    MRect cropRect = node.cropRect_resizer;

    if (mInit) {
        MY_LOGD("createResizerBuf mInit : (%d)", mInit);
        dstSize = mOutImage_resizer->getImgSize();
        cropRect = MRect(MPoint(0, 0), mSensorParams.size);
    }
    /*
    MY_LOGD("[CropInfo] (%d,%d-%dx%d)(%dx%d)", cropRect.p.x, cropRect.p.y,
        cropRect.s.w, cropRect.s.h, dstSize.w, dstSize.h);
    */

    NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo (
        PORT_RRZO,
        node.img_resizer.get(),
        dstSize,
        cropRect,
        node.magicNum,
        node.sofIdx);

    enBuf.mvOut.push_back(rBufInfo);
}
#endif

/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateAppMeta(sp<IPipelineFrame> const &request, MetaSet_T const &result3A,
                QBufInfo const &deqBuf, IMetadata &appMetadata)
{
    //[3A/Flash/sensor section]
    appMetadata = result3A.appMeta;

    //[request section]
    // android.request.frameCount
    {
        IMetadata::IEntry entry(MTK_REQUEST_FRAME_COUNT);
        entry.push_back( request->getFrameNo(), Type2Type< MINT32 >());
        appMetadata.update(MTK_REQUEST_FRAME_COUNT, entry);
    }
    // android.request.metadataMode
    {
        IMetadata::IEntry entry(MTK_REQUEST_METADATA_MODE);
        entry.push_back(MTK_REQUEST_METADATA_MODE_FULL, Type2Type< MUINT8 >());
        appMetadata.update(MTK_REQUEST_METADATA_MODE, entry);
    }

    //[sensor section]
    // android.sensor.timestamp
    {
        MINT64 frame_duration = 0; //IMetadata::IEntry entry(MTK_SENSOR_FRAME_DURATION);
                                   //should get from control.
        MINT64 timestamp = deqBuf.mvOut[0].mMetaData.mTimeStamp - frame_duration;
        IMetadata::IEntry entry(MTK_SENSOR_TIMESTAMP);
        entry.push_back(timestamp, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_TIMESTAMP, entry);
    }

    //[sensor section]
    // android.sensor.rollingshutterskew
    // [TODO] should query from sensor
    {
        IMetadata::IEntry entry(MTK_SENSOR_ROLLING_SHUTTER_SKEW);
        entry.push_back(33000000, Type2Type< MINT64 >());
        appMetadata.update(MTK_SENSOR_ROLLING_SHUTTER_SKEW, entry);
    }


}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P1NodeImp::
generateHalMeta(MetaSet_T const &result3A, QBufInfo const &deqBuf, IMetadata const &resultEIS, IMetadata const &inHalMetadata, IMetadata &halMetadata)
{
    if (deqBuf.mvOut.size() == 0) {
        MY_LOGE("deqBuf is empty");
        return;
    }

    //3a tuning
    halMetadata = result3A.halMeta;

    //eis
    halMetadata += resultEIS;

    // in hal meta
    halMetadata += inHalMetadata;

    //rrzo
    for (size_t i = 0; i < deqBuf.mvOut.size(); i++) {
        if (deqBuf.mvOut[i].mPortID == PORT_RRZO) {
            //crop region
            {
                IMetadata::IEntry entry(MTK_P1NODE_SCALAR_CROP_REGION);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mCrop_s, Type2Type< MRect >());
                halMetadata.update(MTK_P1NODE_SCALAR_CROP_REGION, entry);
            }
            {
                IMetadata::IEntry entry(MTK_P1NODE_DMA_CROP_REGION);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mCrop_d, Type2Type< MRect >());
                halMetadata.update(MTK_P1NODE_DMA_CROP_REGION, entry);
            }

            {
                IMetadata::IEntry entry(MTK_P1NODE_RESIZER_SIZE);
                entry.push_back(deqBuf.mvOut[i].mMetaData.mDstSize, Type2Type< MSize >());
                halMetadata.update(MTK_P1NODE_RESIZER_SIZE, entry);
            }
            /*
            MY_LOGD("[CropInfo] CropS(%d, %d, %dx%d) "
                "CropD(%d, %d, %dx%d) DstSize(%dx%d)",
                deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().x,
                deqBuf.mvOut[i].mMetaData.mCrop_s.leftTop().y,
                deqBuf.mvOut[i].mMetaData.mCrop_s.size().w,
                deqBuf.mvOut[i].mMetaData.mCrop_s.size().h,
                deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().x,
                deqBuf.mvOut[i].mMetaData.mCrop_d.leftTop().y,
                deqBuf.mvOut[i].mMetaData.mCrop_d.size().w,
                deqBuf.mvOut[i].mMetaData.mCrop_d.size().h,
                deqBuf.mvOut[i].mMetaData.mDstSize.w,
                deqBuf.mvOut[i].mMetaData.mDstSize.h);
            */
        } else {
            continue;
        }
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_enque(
    QueNode_T &node,
    MBOOL dummy,
    MBOOL block
)
{
#if SUPPORT_ISP

    FUNCTION_IN

    QBufInfo enBuf;

    //(1) prepare buffer
    if (node.appFrame == NULL) {
        if (mpStreamPool_full != 0) {
            sp<IImageStreamBuffer> streamBuffer;
            sp<IImageBuffer> imageBuffer;
            if ((dummy) && (mDummyBufferFull != NULL)
                && (mdummyQueue.size() > 0))
            {
                if (OK != hwDummyBuffer_acquire(mDummyBufferFull,
                                        (mdummyQueue.size() - 1),
                                        getNodeName(),
                                        imageBuffer)
                )
                {
                    MY_LOGE("full image buffer heap fail");
                    return BAD_VALUE;
                }
                node.halFrame_full = NULL;
                node.img_full = imageBuffer;
            }
            else
            {
                MERROR err = mpStreamPool_full->acquireFromPool(getNodeName(),streamBuffer, ::s2ns(300));
                if( err != OK )
                {
                    if( err == TIMED_OUT )
                        MY_LOGW("acquire timeout");
                    else
                        MY_LOGE("acquire failed");

                    mpStreamPool_full->dumpPool();
                    return BAD_VALUE;
                }

                lockImageBuffer(streamBuffer, imageBuffer);
                node.halFrame_full = streamBuffer;
                node.img_full = imageBuffer;
            }
            //
            #if (SUPPORT_SCALING_CROP_IMGO)
            createFullBuf(node, enBuf);
            #else
            createFullBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
            #endif
        }
        if (mpStreamPool_resizer != 0) {
            sp<IImageStreamBuffer> streamBuffer;
            sp<IImageBuffer> imageBuffer;
            if ((dummy) && (mDummyBufferResizer != NULL)
                && (mdummyQueue.size() > 0))
            {
                if (OK != hwDummyBuffer_acquire(mDummyBufferResizer,
                                        (mdummyQueue.size() - 1),
                                        getNodeName(),
                                        imageBuffer)
                )
                {
                    MY_LOGE("full image buffer heap fail");
                    return BAD_VALUE;
                }
                node.halFrame_resizer = NULL;
                node.img_resizer = imageBuffer;
            }
            else
            {
                MERROR err = mpStreamPool_resizer->acquireFromPool(getNodeName(), streamBuffer, ::s2ns(300));
                if( err != OK )
                {
                    if( err == TIMED_OUT )
                        MY_LOGW("acquire timeout");
                    else
                        MY_LOGE("acquire failed");

                    mpStreamPool_resizer->dumpPool();
                    return BAD_VALUE;
                }
                lockImageBuffer(streamBuffer, imageBuffer);
                node.halFrame_resizer = streamBuffer;
                node.img_resizer = imageBuffer;
            }
            //
            #if (SUPPORT_SCALING_CROP_RRZO)
            createResizerBuf(node, enBuf);
            #else
            createResizerBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
            #endif
        }
    }
    else {
        if (mOutImage_resizer != NULL) {
            sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
            sp<IImageBuffer> imageBuffer;
            if (OK == lockImageBuffer(node.appFrame, mOutImage_resizer->getStreamId(),
                                      pOutpImageStreamBuffer, imageBuffer)) {
                node.img_resizer = imageBuffer;
                #if (SUPPORT_SCALING_CROP_RRZO)
                createResizerBuf(node, enBuf);
                #else
                createResizerBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
                #endif
            }
        }

        for (size_t i = 0; i < mvOutImage_full.size(); i++) {
            sp<IImageStreamBuffer>  pOutpImageStreamBuffer;
            sp<IImageBuffer> imageBuffer;
            if (OK != lockImageBuffer(node.appFrame, mvOutImage_full[i]->getStreamId(),
                                      pOutpImageStreamBuffer, imageBuffer)) {
                continue;
            }
            node.img_full = imageBuffer;
            #if (SUPPORT_SCALING_CROP_IMGO)
            createFullBuf(node, enBuf);
            #else
            createFullBuf(imageBuffer, node.magicNum, node.sofIdx, enBuf);
            #endif
        }
    }


    //(2)
    //very hardware-dependent
    //to ensure that status is not in drop status.
    MINT32 status = _normal_status;
    while(block) {
        if (!mActive) break;

        if (MTRUE != mpCamIO->sendCommand(EPIPECmd_GET_CUR_FRM_STATUS, (MINTPTR)&status, 0, 0)) {
            MY_LOGE("error");
            break;
        }
        if (status == _drop_frame_status || status == _1st_enqloop_status) {
            HwEventIrq::Duration duration;
            if (mpIEventIrq) {
                MY_LOGW("wait for status: %d", status);
                mpIEventIrq->wait(duration);
                MY_LOGD("wait-");
            }
        } else {
            break;
        }
    }

    //enque
    if (!dummy) {
        CAM_TRACE_FMT_BEGIN("enq #(%d/%d)",
            node.appFrame != NULL ? node.appFrame->getFrameNo() : 0,
            node.magicNum);
        if(!mpCamIO->enque(enBuf)) {
            MY_LOGE("enque fail");
            CAM_TRACE_FMT_END();
            return BAD_VALUE;
        }
        CAM_TRACE_FMT_END();
    } else {
        if(!mpCamIO->DummyFrame(enBuf)) {
            MY_LOGE("enque dummy fail");
            return BAD_VALUE;
        }
    }

    FUNCTION_OUT
    return OK;

#else
    return OK;
#endif
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_deque(QBufInfo &deqBuf, MetaSet_T &result3A)
{

#if SUPPORT_ISP

    FUNCTION_IN

    if (!mActive) {
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mHardwareLock);
    if (!mActive) {
        return BAD_VALUE;
    }


    {
        // deque buffer, and handle frame and metadata
        if (mpStreamPool_full!=NULL) {
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo;
            rBufInfo.mPortID = PORT_IMGO;
            deqBuf.mvOut.push_back(rBufInfo);
        }
        if (mpStreamPool_resizer!=NULL) {
            NSCam::NSIoPipe::NSCamIOPipe::BufInfo rBufInfo;
            rBufInfo.mPortID = PORT_RRZO;
            deqBuf.mvOut.push_back(rBufInfo);
        }

        mDequeThreadProfile.pulse_down();
        MY_LOGD1("%ld, %f", mDequeThreadProfile.getAvgDuration(), mDequeThreadProfile.getFps());
        if(!mpCamIO->deque(deqBuf)) {
            if (deqBuf.mvOut.at(0).mMetaData.m_bDummyFrame) {
                MY_LOGW("dummy");
                return BAD_VALUE;
            } else {
                MY_LOGE("deque fail");
                return BAD_VALUE;
            }
        }
        mDequeThreadProfile.pulse_up();
    }


    //

    static bool shouldPrint = false;
    if (shouldPrint) {
        for(size_t i = 0; i < deqBuf.mvOut.size(); i++) {
            char filename[256];
            sprintf(filename, "/data/P1_%d_%d_%d.raw", deqBuf.mvOut.at(i).mMetaData.mMagicNum_hal,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().w,
                deqBuf.mvOut.at(i).mBuffer->getImgSize().h);
            NSCam::Utils::saveBufToFile(filename, (unsigned char*)deqBuf.mvOut.at(i).mBuffer->getBufVA(0), deqBuf.mvOut.at(i).mBuffer->getBufSizeInBytes(0));
            shouldPrint = false;
        }
    }



#if SUPPORT_3A
    if (mActive && mp3A) {
        mp3A->notifyP1Done(deqBuf.mvOut[0].mMetaData.mMagicNum_hal);
        mp3A->get(deqBuf.mvOut[0].mMetaData.mMagicNum_hal, result3A);
    }
#endif


    FUNCTION_OUT

    return OK;
#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
hardwareOps_stop()
{
#if SUPPORT_ISP
    CAM_TRACE_CALL();

    FUNCTION_IN

    //(1) handle active flag
    if (!mActive) {
        MY_LOGD("active=%d", mActive);
        return OK;
    }
    mActive = MFALSE;

    //(2.1) stop EIS thread
    #if SUPPORT_EIS
    CAM_TRACE_BEGIN("eis thread stop");
    if(mpEIS) {
        mpEIS->EisThreadStop();
    }
    CAM_TRACE_END();
    #endif

    //(2.2) stop 3A
    #if SUPPORT_3A
    CAM_TRACE_BEGIN("3A stop");
    if (mp3A) {
        mp3A->detachCb(IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
        mp3A->detachCb(IHal3ACb::eID_NOTIFY_CURR_RESULT, this);
        mp3A->stop();
        mp3A->destroyInstance(getNodeName());
        mp3A = NULL;
    }
    CAM_TRACE_END();
    #endif

    //(3) stop isp
    //
    {
        CAM_TRACE_BEGIN("isp stop");
        Mutex::Autolock _l(mHardwareLock);
        //
        if (mpIEventIrq) {
            mpIEventIrq->destroyInstance("VSIrq");
        }
        if(!mpCamIO || !mpCamIO->stop()) {
            MY_LOGE("hardware stop fail");
            return BAD_VALUE;
        }
        CAM_TRACE_END();

#if SUPPORT_EIS
        if(mpEIS) {
            mpEIS->Uninit();
            mpEIS->DestroyInstance(LOG_TAG);
            mpEIS = NULL;
        }
#endif

        //
        CAM_TRACE_BEGIN("isp destroy");
        if(!mpCamIO->uninit() )
        {
            MY_LOGE("hardware uninit fail");
            return BAD_VALUE;
        }
        mpCamIO->destroyInstance(getNodeName());
        mpCamIO = NULL;
        CAM_TRACE_END();
    }
    //
    FUNCTION_OUT

    return OK;

#else
    return OK;
#endif

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, IMetadata &rMetadata)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IMetaStreamBuffer> pMetaStreamBuffer = NULL;
    MERROR const err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer
    );
    if (err != OK) {
        return err;
    }
    rMetadata = *pMetaStreamBuffer->tryReadLock(getNodeName());

    return err;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, MBOOL success)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is  NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    sp<IMetaStreamBuffer> pMetaStreamBuffer = NULL;
    MERROR const err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer,
        MFALSE
    );
    if (err != OK) {
        return err;
    }

    if  ( 0 > mInStreamIds.indexOf(streamId) ) {
        if (success) {
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        } else {
            pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }

    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );

    return err;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is NULL");
        return INVALID_OPERATION;
    }

    //  Input Meta Stream: Request
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::RELEASE
    );

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lock_and_returnMetadata(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, IMetadata &metadata)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    //
    sp<IMetaStreamBuffer>   pMetaStreamBuffer  = NULL;
    MERROR err = ensureMetaBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pMetaStreamBuffer
    );
    if (err != OK) {
        return err;
    }
    IMetadata* pMetadata = pMetaStreamBuffer->tryWriteLock(getNodeName());
    if (pMetadata == NULL) {
        MY_LOGE("pMetadata == NULL");
        return BAD_VALUE;
    }

    *pMetadata = metadata;

    pMetaStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
    pMetaStreamBuffer->unlock(getNodeName(), pMetadata);
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId,
                  sp<IImageStreamBuffer>  &pOutpImageStreamBuffer, sp<IImageBuffer> &rImageBuffer)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    MERROR const err = ensureImageBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pOutpImageStreamBuffer
    );
    if (err != OK) {
        return err;
    }

    MUINT const groupUsage = pOutpImageStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap>  pOutpImageBufferHeap = pOutpImageStreamBuffer->tryWriteLock(getNodeName());
    if (pOutpImageBufferHeap == NULL) {
        MY_LOGE("pOutpImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    rImageBuffer = pOutpImageBufferHeap->createImageBuffer();
    rImageBuffer->lockBuf(getNodeName(), groupUsage);

    mImageStorage.enque(pOutpImageStreamBuffer, rImageBuffer);


    MY_LOGD1("stream buffer: 0x%x, heap: 0x%x, buffer: 0x%x, usage: 0x%x",
        pOutpImageStreamBuffer.get(), pOutpImageBufferHeap.get(), rImageBuffer.get(), groupUsage);

    return err;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId, MBOOL success)
{
    if (pFrame == NULL) {
        //MY_LOGE("input is NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();

    sp<IImageStreamBuffer>  pOutpImageStreamBuffer = NULL;
    MERROR const err = ensureImageBufferAvailable_(
        pFrame->getFrameNo(),
        streamId,
        rStreamBufferSet,
        pOutpImageStreamBuffer,
        MFALSE
    );
    if (err != OK) {
        return err;
    }

    sp<IImageBuffer> pOutpImageBuffer =  mImageStorage.deque(reinterpret_cast<MINTPTR>(pOutpImageStreamBuffer.get()));
    if (pOutpImageBuffer == NULL) {
        MY_LOGE("pImageBuffer == NULL");
        return BAD_VALUE;
    }

    if  ( 0 > mInStreamIds.indexOf(streamId) ) {
        if (success) {
            pOutpImageStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_OK);
        } else {
            pOutpImageStreamBuffer->markStatus(STREAM_BUFFER_STATUS::WRITE_ERROR);
        }
    }

    pOutpImageBuffer->unlockBuf(getNodeName());
    pOutpImageStreamBuffer->unlock(getNodeName(), pOutpImageBuffer->getImageBufferHeap());
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::USED |
        IUsersManager::UserStatus::RELEASE
    );


    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedImageBuffer(sp<IPipelineFrame> const& pFrame, StreamId_T const streamId)
{
    if (pFrame == NULL) {
        //MY_LOGE("nput is  NULL");
        return INVALID_OPERATION;
    }

    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
        streamId, getNodeId(),
        IUsersManager::UserStatus::RELEASE
    );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
lockImageBuffer(sp<IImageStreamBuffer> const& pStreamBuffer, sp<IImageBuffer> &pImageBuffer)
{
    if (pStreamBuffer == NULL) {
        return BAD_VALUE;
    }
    MUINT const usage = GRALLOC_USAGE_SW_READ_OFTEN | GRALLOC_USAGE_HW_CAMERA_READ | GRALLOC_USAGE_HW_CAMERA_WRITE;
    sp<IImageBufferHeap>  pOutpImageBufferHeap = pStreamBuffer->tryWriteLock(getNodeName());
    if (pOutpImageBufferHeap == NULL) {
        MY_LOGE("pOutpImageBufferHeap == NULL");
        return BAD_VALUE;
    }
    pImageBuffer = pOutpImageBufferHeap->createImageBuffer();
    pImageBuffer->lockBuf(getNodeName(), usage);

    mImageStorage.enque(pStreamBuffer, pImageBuffer);

    MY_LOGD1("streambuffer: 0x%x, heap: 0x%x, buffer: 0x%x, usage: 0x%x",
        pStreamBuffer.get(), pOutpImageBufferHeap.get(), pImageBuffer.get(), usage);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnLockedImageBuffer(sp<IImageBuffer> const &pImageBuffer,
                        sp<IImageStreamBuffer> const &pStreamBuffer,
                        sp<IImageStreamBufferPoolT> const &pStreamPool)
{
    if (pImageBuffer == NULL || pStreamBuffer == NULL || pStreamPool == NULL) {
        MY_LOGE_IF(pImageBuffer == NULL,  "pImageBuffer == NULL");
        MY_LOGE_IF(pStreamBuffer == NULL, "pStreamBuffer == NULL");
        MY_LOGE_IF(pStreamPool == NULL,   "pStreamPool == NULL");
        return BAD_VALUE;
    }

    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());

    if(pStreamPool != NULL) {
        pStreamPool->releaseToPool(getNodeName(), pStreamBuffer);
    }

    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
P1NodeImp::
returnUnlockedImageBuffer(sp<IImageStreamBuffer> const &pStreamBuffer,
                          sp<IImageStreamBufferPoolT> const &pStreamPool)
{
    if (pStreamBuffer == NULL || pStreamPool == NULL ) {
        MY_LOGE_IF(pStreamBuffer == NULL, "pStreamBuffer == NULL");
        MY_LOGE_IF(pStreamPool == NULL,   "pStreamPool == NULL");
        return BAD_VALUE;
    }

    pStreamPool->releaseToPool(getNodeName(), pStreamBuffer);

    return OK;
}

#if (SUPPORT_SCALING_CROP)
/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
prepareCropInfo(IMetadata* pMetadata,
           QueNode_T& node)
{
    if (mvOutImage_full.size() > 0) {
        node.dstSize_full = mvOutImage_full[0]->getImgSize();
        node.cropRect_full = MRect(MPoint(0, 0), mSensorParams.size);
    } else {
        node.dstSize_full = MSize(0, 0);
        node.cropRect_full = MRect(MPoint(0, 0), MSize(0, 0));
    }
    if (mOutImage_resizer != NULL) {
        node.dstSize_resizer = mOutImage_resizer->getImgSize();
        node.cropRect_resizer = MRect(MPoint(0, 0), mSensorParams.size);
    } else {
        node.dstSize_resizer= MSize(0, 0);
        node.cropRect_resizer = MRect(MPoint(0, 0), MSize(0, 0));
    }
    //
    if (pMetadata != NULL) {
        MRect cropRect_metadata;    // get from metadata
        MRect cropRect_control;     // set to node

        if( !tryGetMetadata<MRect>(pMetadata, MTK_SCALER_CROP_REGION,
            cropRect_metadata) ) {
            MY_LOGI("Metadata exist - no MTK_SCALER_CROP_REGION, "
                "crop size set to full(%dx%d) resizer(%dx%d)",
                node.dstSize_full.w, node.dstSize_full.h,
                node.dstSize_resizer.w, node.dstSize_resizer.h);
        } else {
            simpleTransform tranActive2Sensor = simpleTransform(
                    MPoint(0,0), mActiveArray.size(), mSensorParams.size);
            cropRect_control.p = transform(tranActive2Sensor,
                                            cropRect_metadata.leftTop());
            cropRect_control.s = transform(tranActive2Sensor,
                                            cropRect_metadata.size());

            #if SUPPORT_EIS
            if (mpEIS)
            {
                MBOOL isEisOn = false;
                MRect const requestRect = MRect(cropRect_control);
                MSize const sensorSize = MSize(mSensorParams.size);
                MPoint const requestCenter=
                    MPoint((requestRect.p.x + (requestRect.s.w >> 1)),
                            (requestRect.p.y + (requestRect.s.h >> 1)));
                isEisOn = isEISOn(pMetadata);
                cropRect_control.s = mpEIS->QueryMinSize(isEisOn, sensorSize,
                                                        requestRect.size());

                if (cropRect_control.s.w != requestRect.size().w)
                {
                    MSize::value_type half_len =
                        ((cropRect_control.s.w + 1) >> 1);
                    if (requestCenter.x < half_len) {
                        cropRect_control.p.x = 0;
                    } else if ((requestCenter.x + half_len) > sensorSize.w) {
                        cropRect_control.p.x = sensorSize.w -
                                                cropRect_control.s.w;
                    } else {
                        cropRect_control.p.x = requestCenter.x - half_len;
                    }
                }
                if (cropRect_control.s.w != requestRect.size().w)
                {
                    MSize::value_type half_len =
                        ((cropRect_control.s.h + 1) >> 1);
                    if (requestCenter.y < half_len) {
                        cropRect_control.p.y = 0;
                    } else if ((requestCenter.y + half_len) > sensorSize.h) {
                        cropRect_control.p.y = sensorSize.h -
                                                cropRect_control.s.h;
                    } else {
                        cropRect_control.p.y = requestCenter.y - half_len;
                    }
                }
            }
            #endif
            /*
            MY_LOGD("[CropInfo] metadata(%d, %d, %dx%d) "
                "control(%d, %d, %dx%d) "
                "active(%d, %d, %dx%d) "
                "sensor(%dx%d)",
                cropRect_metadata.leftTop().x,
                cropRect_metadata.leftTop().y,
                cropRect_metadata.size().w, cropRect_metadata.size().h,
                cropRect_control.leftTop().x,
                cropRect_control.leftTop().y,
                cropRect_control.size().w, cropRect_control.size().h,
                mActiveArray.leftTop().x,
                mActiveArray.leftTop().y,
                mActiveArray.size().w, mActiveArray.size().h,
                mSensorParams.size.w, mSensorParams.size.h);
            */
            // TODO: check more case about crop region
            if ((cropRect_control.size().w < 0) ||
                (cropRect_control.size().h < 0) ||
                (cropRect_control.leftTop().x < 0) ||
                (cropRect_control.leftTop().y < 0) ||
                (cropRect_control.leftTop().x >= mSensorParams.size.w) ||
                (cropRect_control.leftTop().y >= mSensorParams.size.h)) {
                MY_LOGW("Metadata exist - invalid cropRect_control"
                    "(%d, %d, %dx%d) sensor(%dx%d)",
                    cropRect_control.leftTop().x,
                    cropRect_control.leftTop().y,
                    cropRect_control.size().w, cropRect_control.size().h,
                    mSensorParams.size.w, mSensorParams.size.h);
                return;
            }
            if ((cropRect_control.p.x + cropRect_control.s.w) >
                mSensorParams.size.w) {
                cropRect_control.s.w = mSensorParams.size.w -
                                        cropRect_control.p.x;
            }
            if ((cropRect_control.p.y + cropRect_control.s.h) >
                mSensorParams.size.h) {
                cropRect_control.s.h = mSensorParams.size.h -
                                        cropRect_control.p.y;
            }
            // calculate the crop region validity
            calculateCropInfoFull(mSensorParams.pixelMode,
                                    mSensorParams.size,
                                    (mvOutImage_full.size() > 0) ?
                                    (mvOutImage_full[0]->getImgSize()) :
                                    (MSize(0, 0)),
                                    cropRect_control,
                                    node.cropRect_full,
                                    node.dstSize_full);
            calculateCropInfoResizer(mSensorParams.pixelMode,
                                    mSensorParams.size,
                                    (mOutImage_resizer != NULL) ?
                                    (mOutImage_resizer->getImgSize()) :
                                    (MSize(0, 0)),
                                    cropRect_control,
                                    node.cropRect_resizer,
                                    node.dstSize_resizer);
        }
    }
}
#endif

/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
createNode(sp<IPipelineFrame> appframe,
           Que_T *Queue,
           Mutex *QueLock,
           List<MetaSet_T> *list,
           Mutex *listLock)
{
    //create queue node
    MUINT32 newNum = get_and_increase_magicnum();
    MetaSet_T metaInfo;
    bool gotMetaInfo = false;

    // create list for 3A
    if (list!=NULL) {
        //fill in App metadata
        if (appframe != NULL) {
            if (mInAppMeta != NULL) {
                lockMetadata(appframe, mInAppMeta->getStreamId(),
                    metaInfo.appMeta);
                gotMetaInfo = true;
            }
            if (mInHalMeta != NULL) {
                lockMetadata(appframe, mInHalMeta->getStreamId(),
                    metaInfo.halMeta);
            }
        }
        //fill in hal metadata
        IMetadata::IEntry entry1(MTK_P1NODE_PROCESSOR_MAGICNUM );
        entry1.push_back(newNum, Type2Type< MINT32 >());
        metaInfo.halMeta.update(MTK_P1NODE_PROCESSOR_MAGICNUM, entry1);

        IMetadata::IEntry entry2(MTK_HAL_REQUEST_REPEAT);
        entry2.push_back(0, Type2Type< MUINT8 >());
        metaInfo.halMeta.update(MTK_HAL_REQUEST_REPEAT, entry2);


        MUINT8 isdummy =  appframe == NULL ? 1 : 0;
        IMetadata::IEntry entry3(MTK_HAL_REQUEST_DUMMY);
        entry3.push_back(isdummy, Type2Type< MUINT8 >());
        metaInfo.halMeta.update(MTK_HAL_REQUEST_DUMMY, entry3);


        if(listLock != NULL) {
            Mutex::Autolock _l(*listLock);
            (*list).push_back(metaInfo);
        } else {
            (*list).push_back(metaInfo);
        }

    }

    if(Queue!=NULL){
        Mutex::Autolock _l(*QueLock);
        QueNode_T node;
        node.magicNum = newNum;
        node.sofIdx = P1SOFIDX_INIT_VAL;
        node.appFrame = appframe;
        #if (SUPPORT_SCALING_CROP)
        prepareCropInfo((gotMetaInfo) ? (&metaInfo.appMeta) : (NULL), node);
        #endif
        (*Queue).push_back(node);
    }

    if (appframe != NULL) {
        MY_LOGD1("[New Request] frameNo: %u, magic Num: %d", appframe->getFrameNo(), newNum);
    } else {
        MY_LOGD1("[New Request: dummy] magic Num: %d", newNum);
    }
}


/******************************************************************************
 *
 *****************************************************************************/
MVOID
P1NodeImp::
createNode(Que_T &Queue)
{
    MUINT32 newNum = get_and_increase_magicnum();
    {
        QueNode_T node;
        node.magicNum = newNum;
        node.sofIdx = P1SOFIDX_INIT_VAL;
        node.appFrame = NULL;
        Queue.push_back(node);
    }

    MY_LOGD1("[New Request: dummy] magic Num: %d", newNum);
}


/******************************************************************************
 *
 ******************************************************************************/
sp<P1Node>
P1Node::
createInstance()
{
    return new P1NodeImp();

}

