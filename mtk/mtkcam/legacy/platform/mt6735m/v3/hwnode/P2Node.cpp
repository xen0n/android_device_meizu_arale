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

#define LOG_TAG "MtkCam/P2Node"
//
#include <mtkcam/Log.h>
#include "BaseNode.h"
#include "hwnode_utilities.h"
#include <mtkcam/v3/hwnode/P2Node.h>
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>
#include <sys/resource.h>
//
#include <mtkcam/v3/hal/IHal3A.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IPortEnum.h>
#include <mtkcam/iopipe/SImager/IImageTransform.h>
//
#include <vector>
#include <list>
//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
//
#include <mtkcam/featureio/eis_hal_base.h>
//
#include <mtkcam/Trace.h>
//
#include <cutils/properties.h>
//

using namespace android;
using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;

using namespace std;
using namespace NSIoPipe;
using namespace NSIoPipe::NSPostProc_FrmB;
using namespace NS3Av3;

/******************************************************************************
 *
 ******************************************************************************/
#define P2THREAD_NAME_ENQUE ("Cam@P2Enq")
#define P2THREAD_NAME_COPY  ("Cam@P2Copy")
#define P2THREAD_POLICY     (SCHED_OTHER)
#define P2THREAD_PRIORITY   (0)
//
#define WAITBUFFER_TIMEOUT (1000000000L)
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

//
#if 0
#define FUNC_START     MY_LOGD("+")
#define FUNC_END       MY_LOGD("-")
#else
#define FUNC_START
#define FUNC_END
#endif
/******************************************************************************
 *
 ******************************************************************************/
#define SUPPORT_3A               (1)
#define FD_PORT_SUPPORT          (0)
#define FORCE_EIS_ON             (0)

#define DEBUG_LOG                (0)
/******************************************************************************
 *
 ******************************************************************************/
static const PortID PORT_IMGI  (EPortType_Memory, EPipePortIndex_IMGI , 0);
static const PortID PORT_WDMAO (EPortType_Memory, EPipePortIndex_WDMAO, 1);
static const PortID PORT_WROTO (EPortType_Memory, EPipePortIndex_WROTO, 1);
#if FD_PORT_SUPPORT
static const PortID PORT_IMG2O (EPortType_Memory, EPipePortIndex_IMG2O, 1);
#endif


inline
MBOOL isStream(sp<IStreamInfo> pStreamInfo, StreamId_T streamId ) {
    return pStreamInfo.get() && pStreamInfo->getStreamId() == streamId;
}


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
updateEntry(
    IMetadata* pMetadata,
    MUINT32 const tag,
    T const& val
)
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }

    IMetadata::IEntry entry(tag);
    entry.push_back(val, Type2Type<T>());
    pMetadata->update(tag, entry);
}


/******************************************************************************
 *
 ******************************************************************************/
class StreamControl
{
    public:
        typedef enum
        {
            eStreamStatus_NOT_USED = (0x00000000UL),
            eStreamStatus_FILLED   = (0x00000001UL),
            eStreamStatus_ERROR    = (0x00000001UL << 1),
        } eStreamStatus_t;

    public:

        virtual                         ~StreamControl() {};

    public:

        virtual MERROR                  getInfoIOMapSet(
                                            sp<IPipelineFrame> const& pFrame,
                                            IPipelineFrame::InfoIOMapSet& rIOMapSet
                                        ) const                                   = 0;

        // query in/out stream function
        virtual MBOOL                   isInImageStream(
                                            StreamId_T const streamId
                                        ) const                                   = 0;

        virtual MBOOL                   isInMetaStream(
                                            StreamId_T const streamId
                                        ) const                                   = 0;

        // image stream related
        virtual MERROR                  acquireImageStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer>& rpStreamBuffer
                                        )                                         = 0;

        virtual MVOID                   releaseImageStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            MUINT32 const status
                                        ) const                                   = 0;

        virtual MERROR                  acquireImageBuffer(
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            sp<IImageBuffer>& rpImageBuffer
                                        ) const                                   = 0;

        virtual MVOID                   releaseImageBuffer(
                                            sp<IImageStreamBuffer> const pStreamBuffer,
                                            sp<IImageBuffer> const pImageBuffer
                                        ) const                                   = 0;

        // meta stream related
        virtual MERROR                  acquireMetaStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer>& rpStreamBuffer
                                        )                                         = 0;

        virtual MVOID                   releaseMetaStream(
                                            sp<IPipelineFrame> const& pFrame,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            MUINT32 const status
                                        ) const                                   = 0;

        virtual MERROR                  acquireMetadata(
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            IMetadata*& rpMetadata
                                        ) const                                   = 0;

        virtual MVOID                   releaseMetadata(
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            IMetadata* const pMetadata
                                        ) const                                   = 0;

        // frame control related
        virtual MVOID                   onPartialFrameDone(
                                            sp<IPipelineFrame> const& pFrame
                                        )                                         = 0;

        virtual MVOID                   onFrameDone(
                                            sp<IPipelineFrame> const& pFrame
                                        )                                         = 0;

};


class MetaHandle
    : public VirtualLightRefBase
{
    public:
        typedef enum
        {
            STATE_NOT_USED,
            STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITE_FAIL,
        } BufferState_t;

    public:
        static sp<MetaHandle>           create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId
                                        );
                                        ~MetaHandle();
    protected:
                                        MetaHandle(
                                            StreamControl* pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IMetaStreamBuffer> const pStreamBuffer,
                                            BufferState_t const init_state,
                                            IMetadata * const pMeta
                                        )
                                            : mpStreamCtrl(pCtrl)
                                            , mpFrame(pFrame)
                                            , mStreamId(streamId)
                                            , mpStreamBuffer(pStreamBuffer)
                                            , mpMetadata(pMeta)
                                            , muState(init_state)
                                        {}

    public:
        IMetadata*                      getMetadata() { return mpMetadata; }

#if 0
        virtual MERROR                  waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
                                        )                                                   = 0;
#endif
        MVOID                           updateState(
                                            BufferState_t const state
                                        );
    private:
        Mutex                           mLock;
        //Condition                       mCond;
        StreamControl* const            mpStreamCtrl;
        sp<IPipelineFrame> const        mpFrame;
        StreamId_T const                mStreamId;
        sp<IMetaStreamBuffer> const     mpStreamBuffer;
        IMetadata* const                mpMetadata;
        MUINT32                         muState;
};


class BufferHandle
    : public VirtualLightRefBase
{
    public:
        typedef enum
        {
            STATE_NOT_USED,
            STATE_READABLE,
            STATE_WRITABLE,
            STATE_WRITE_OK = STATE_READABLE,
            STATE_WRITE_FAIL,
        } BufferState_t;

    public:
        virtual                         ~BufferHandle() {}

    public:
        virtual IImageBuffer*           getBuffer()                                         = 0;

        virtual MERROR                  waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout = WAITBUFFER_TIMEOUT
                                        )                                                   = 0;
        virtual MVOID                   updateState(
                                            BufferState_t const state
                                        )                                                   = 0;
};


class StreamBufferHandle
    : public BufferHandle
{
    public:
        static sp<BufferHandle>         create(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId
                                        );
                                        ~StreamBufferHandle();
    protected:
                                        StreamBufferHandle(
                                            StreamControl* pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            StreamId_T const streamId,
                                            sp<IImageStreamBuffer> const pStreamBuffer
                                        )
                                            : mpStreamCtrl(pCtrl)
                                            , mpFrame(pFrame)
                                            , mStreamId(streamId)
                                            , mpStreamBuffer(pStreamBuffer)
                                            , muState(STATE_NOT_USED)
                                            , mpImageBuffer(NULL)
                                        {}

    public:
        IImageBuffer*                   getBuffer() { return mpImageBuffer.get(); }
        MERROR                          waitState(
                                            BufferState_t const state,
                                            nsecs_t const nsTimeout
                                        );
        MVOID                           updateState(
                                            BufferState_t const state
                                        );

    private:
        Mutex                           mLock;
        Condition                       mCond;
        StreamControl* const            mpStreamCtrl;
        sp<IPipelineFrame> const        mpFrame;
        StreamId_T const                mStreamId;
        sp<IImageStreamBuffer> const    mpStreamBuffer;
        MUINT32                         muState;
        sp<IImageBuffer>                mpImageBuffer;
};


class FrameLifeControl
    : public VirtualLightRefBase
{
    public:
                                        FrameLifeControl(
                                            StreamControl* const pCtrl,
                                            sp<IPipelineFrame> const& pFrame,
                                            MBOOL const enableLog
                                            )
                                            : mbEnableLog(enableLog)
                                            , mpStreamControl(pCtrl)
                                            , mpFrame(pFrame)
                                        {
                                            CAM_TRACE_ASYNC_BEGIN("process(p2)", mpFrame->getFrameNo());
                                            MY_LOGD_IF(mbEnableLog, "frame %zu +", mpFrame->getFrameNo());
                                        }

                                        ~FrameLifeControl() {
                                            if( mpStreamControl )
                                                mpStreamControl->onFrameDone(mpFrame);
                                            MY_LOGD_IF(mbEnableLog, "frame %zu -", mpFrame->getFrameNo());
                                            CAM_TRACE_ASYNC_END("process(p2)", mpFrame->getFrameNo());
                                        }

    public:
        MVOID                           onPartialFrameDone() {
                                            if( mpStreamControl )
                                                mpStreamControl->onPartialFrameDone(mpFrame);
                                        }

    private:
        MBOOL const                     mbEnableLog;
        StreamControl* const            mpStreamControl; //stream control & dispatch
        sp<IPipelineFrame> const        mpFrame;
};


class Job
    : public VirtualLightRefBase
{
    public:
        virtual             ~Job() {};

    public:
        virtual MERROR      execute() = 0;

        // for processor
        virtual MVOID       finishAsync(MBOOL success) = 0;
};


class JobQueue
{
    public:
        MVOID               add(sp<Job> pJob) { mvPending.push_back(pJob); }
        size_t              size() const { return mvPending.size(); }
        sp<Job>             get();
        MVOID               clear() { mvPending.clear(); }

    private:
        vector<sp<Job> >    mvPending;
};


template< typename _PROCESSOR_ >
class ProcessorJob
    : public Job
{
    public:
        static sp<Job>      create(
                                sp<FrameLifeControl> const pFrameLifeCtrl,
                                sp<_PROCESSOR_> const pProcessor,
                                typename _PROCESSOR_::FrameParams const params
                            ) {
                                return new ProcessorJob(pFrameLifeCtrl, pProcessor, params);
                            }

    protected:
                            ProcessorJob(
                                sp<FrameLifeControl> const pFrameLifeCtrl,
                                sp<_PROCESSOR_> const pProcessor,
                                typename _PROCESSOR_::FrameParams const params
                            )
                                : Job()
                                , mpFrameLifeCtrl(pFrameLifeCtrl)
                                , mpProcessor(pProcessor)
                                , mParams(params)
                                , muStatus(ST_FAIL)
                            {
                            }

        virtual             ~ProcessorJob()
                            {
                                mpProcessor->onFinish(mParams, muStatus == ST_OK);

                                // to release any sp<>
                                mParams = typename _PROCESSOR_::FrameParams();
                                //
                                // signal that a job is finished
                                if( mpFrameLifeCtrl.get() )
                                    mpFrameLifeCtrl->onPartialFrameDone();
                            }

    public:
        inline MERROR       execute() {
                                MERROR const ret = mpProcessor->onExecute(this, mParams);
                                if( !mpProcessor->isAsync() && ret == OK )
                                    muStatus = ST_OK;
                                return ret;
                            }

        inline MVOID        finishAsync(MBOOL success) {
                                if( success )
                                    muStatus = ST_OK;
                            }

    private:                // frame life control
        sp<FrameLifeControl> const                   mpFrameLifeCtrl;

    private:                // processor related
        sp<_PROCESSOR_> const                        mpProcessor;
        typename _PROCESSOR_::FrameParams            mParams;

    private:
        enum
        {
            ST_FAIL,
            ST_OK,
        };
        MUINT32                                      muStatus;
};


class P2Processor
    : public VirtualLightRefBase
{
    public:
        struct InitParams
        {
            MINT32                          openId;
            P2Node::ePass2Type              type;
            MRect                           activeArray;
        };

        struct FrameInput
        {
            PortID                          mPortId;
            sp<BufferHandle>                mHandle;
        };

        struct FrameOutput
        {
            PortID                          mPortId;
            sp<BufferHandle>                mHandle;
            MINT32                          mTransform;
        };

        struct FrameParams
        {
            FrameInput           in;
            Vector<FrameOutput>  vOut;
            //
            MBOOL                bResized;
            //
            sp<MetaHandle>       inApp;
            sp<MetaHandle>       inHal;
            sp<MetaHandle>       outApp;
            sp<MetaHandle>       outHal;
        };

    private: //private use structures
        struct eis_region
        {
            MUINT32 x_int;
            MUINT32 x_float;
            MUINT32 y_int;
            MUINT32 y_float;
            MSize   s;
        };

#if 0
        struct vector_f
        {
            MINT32  x_int;
            MINT32  y_int;
            MUINT32 x_float;
            MUINT32 y_float;
        };
#endif

        struct crop_infos
        {
            // port
            MBOOL                isResized;
            //
            MSize                sensor_size;
            // p1 crop infos
            MRect                crop_p1_sensor;
            MSize                dstsize_resizer;
            MRect                crop_dma;
            //
            simpleTransform      tranActive2Sensor;
            simpleTransform      tranSensor2Resized;
            //
            // target crop: cropRegion
            // not applied eis's mv yet, but the crop area is already reduced by
            // EIS ratio.
            // _a: active array coordinates
            // _s: sensor coordinates
            // active array coordinates
            MRect                crop_a;
            //MPoint               crop_a_p;
            //MSize                crop_a_size;
            // sensor coordinates
            //MPoint               crop_s_p;
            //MSize                crop_s_size;
            // resized coordinates
            //
            MBOOL                isEisEabled;
            vector_f             eis_mv_a; //active array coor.
            vector_f             eis_mv_s; //sensor coor.
            vector_f             eis_mv_r; //resized coor.

            MVOID                       dump() const {
                                            MY_LOGD("isResized %d", isResized);
                                            MY_LOGD("p1 info (%d,%d,%dx%d), (%dx%d), (%d,%d,%dx%d)",
                                                    crop_p1_sensor.p.x,
                                                    crop_p1_sensor.p.y,
                                                    crop_p1_sensor.s.w,
                                                    crop_p1_sensor.s.h,
                                                    dstsize_resizer.w,
                                                    dstsize_resizer.h,
                                                    crop_dma.p.x,
                                                    crop_dma.p.y,
                                                    crop_dma.s.w,
                                                    crop_dma.s.h
                                                   );
                                            MY_LOGD("tran active to sensor o %d, %d, s %dx%d -> %dx%d",
                                                    tranActive2Sensor.tarOrigin.x,
                                                    tranActive2Sensor.tarOrigin.y,
                                                    tranActive2Sensor.oldScale.w,
                                                    tranActive2Sensor.oldScale.h,
                                                    tranActive2Sensor.newScale.w,
                                                    tranActive2Sensor.newScale.h
                                                   );
                                            MY_LOGD("tran sensor to resized o %d, %d, s %dx%d -> %dx%d",
                                                    tranSensor2Resized.tarOrigin.x,
                                                    tranSensor2Resized.tarOrigin.y,
                                                    tranSensor2Resized.oldScale.w,
                                                    tranSensor2Resized.oldScale.h,
                                                    tranSensor2Resized.newScale.w,
                                                    tranSensor2Resized.newScale.h
                                                   );
                                            MY_LOGD("modified active crop %d, %d, %dx%d",
                                                    crop_a.p.x,
                                                    crop_a.p.y,
                                                    crop_a.s.w,
                                                    crop_a.s.h
                                                   );
                                            MY_LOGD("isEisOn %d", isEisEabled);
                                            MY_LOGD("mv in active %d/%d, %d/%d",
                                                    eis_mv_a.p.x, eis_mv_a.pf.x,
                                                    eis_mv_a.p.y, eis_mv_a.pf.y
                                                    );
                                            MY_LOGD("mv in sensor %d/%d, %d/%d",
                                                    eis_mv_s.p.x, eis_mv_s.pf.x,
                                                    eis_mv_s.p.y, eis_mv_s.pf.y
                                                    );
                                            MY_LOGD("mv in resized %d/%d, %d/%d",
                                                    eis_mv_r.p.x, eis_mv_r.pf.x,
                                                    eis_mv_r.p.y, eis_mv_r.pf.y
                                                    );
                                        }
        };

    public:
        static sp<P2Processor>          create(MBOOL const enableLog, InitParams const& params);
                                        ~P2Processor();

    protected:
                                        P2Processor(
                                                MBOOL const enableLog,
                                                InitParams const& params,
                                                INormalStream* const pPipe,
                                                IHal3A* p3A
                                                )
                                            : mbEnableLog(enableLog)
                                            , mInitParams(params)
                                            , mpPipe(pPipe)
                                            , mp3A(p3A)
                                            , muEnqueCnt(0)
                                            , muDequeCnt(0)
                                        {}

    public:
        MERROR                          waitIdle();

    public:                             // used by job
        MBOOL                           isAsync() { return MTRUE; }

        MERROR                          onExecute(
                                            sp<Job> const pJob,
                                            FrameParams const params
                                        );

        MERROR                          onFinish(
                                            FrameParams const params,
                                            MBOOL const success
                                        );

    protected:
        MERROR                          checkParams(FrameParams const params) const;

        MERROR                          getCropInfos(
                                            IMetadata* const inApp,
                                            IMetadata* const inHal,
                                            MBOOL const isResized,
                                            crop_infos & cropInfos
                                        ) const;

        MVOID                           queryCropRegion(
                                            IMetadata* const meta_request,
                                            MBOOL const isEisOn,
                                            MRect& targetCrop
                                        ) const;

        MVOID                           updateCropRegion(
                                            MRect const crop,
                                            IMetadata* meta_result
                                        ) const;

        MVOID                           calcCrop_viewangle(
                                            crop_infos const& cropInfos,
                                            MSize const& dstSize,
                                            MCropRect& result
                                        ) const;

        MBOOL                           isEISOn(
                                            IMetadata* const inApp
                                        ) const;

        MBOOL                           queryEisRegion(
                                            IMetadata* const inHal,
                                            eis_region& region
                                        ) const;

        MBOOL                           refineBoundary(
                                            MSize const& bufSize,
                                            MCropRect& crop
                                        ) const;

        static MVOID                    pass2CbFunc(QParams& rParams);

        MVOID                           handleDeque(QParams& rParams);

    private:
        MBOOL const                     mbEnableLog;
        mutable Mutex                   mLock;
        mutable Condition               mCondJob;
        //
        InitParams const                mInitParams;
        //
        INormalStream* const            mpPipe;
        IHal3A* const                   mp3A;
        //
        MUINT32                         muEnqueCnt;
        MUINT32                         muDequeCnt;
        vector<sp<Job> >                mvRunning;
};


class MDPProcessor
    : public VirtualLightRefBase
{
    public:
        struct FrameInput
        {
            sp<BufferHandle>                mHandle;
        };

        struct FrameOutput
        {
            sp<BufferHandle>                mHandle;
            MINT32                          mTransform;
        };

        struct FrameParams
        {
            FrameInput           in;
            Vector<FrameOutput>  vOut;
            MRect                crop;
        };
    public:
        static sp<MDPProcessor>         create(MBOOL const enableLog);
                                        ~MDPProcessor() {}
    protected:
                                        MDPProcessor(MBOOL const enableLog)
                                        : mbEnableLog(enableLog)
                                        {}

    public:
        MERROR                          waitIdle() { return OK; } // since is synchronous

    public:                             // used by job
        MBOOL                           isAsync() { return MFALSE; }

        MERROR                          onExecute(
                                            sp<Job> const pJob,
                                            FrameParams const params
                                        );

        MERROR                          onFinish(
                                            FrameParams const params,
                                            MBOOL const success
                                        );
    private:
        MBOOL const                     mbEnableLog;

};


/******************************************************************************
 *
 ******************************************************************************/
class P2NodeImp
    : public BaseNode
    , public P2Node
    , public StreamControl
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;

protected:

    class EnqueThread
        : public Thread
    {

    public:

                                    EnqueThread(P2NodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~EnqueThread()
                                    {}

    public:

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

        P2NodeImp*                  mpNodeImp;

    };

    class CopyThread
        : public Thread
    {

    public:

                                    CopyThread(P2NodeImp* pNodeImp)
                                        : mpNodeImp(pNodeImp)
                                    {}

                                    ~CopyThread()
                                    {}

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    //  Thread Interface.
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    public:
                    // Ask this object's thread to exit. This function is asynchronous, when the
                    // function returns the thread might still be running. Of course, this
                    // function can be called from a different thread.
                    virtual void        requestExit();

                    // Good place to do one-time initializations
                    virtual status_t    readyToRun();

    private:
                    // Derived class must implement threadLoop(). The thread starts its life
                    // here. There are two ways of using the Thread object:
                    // 1) loop: if threadLoop() returns true, it will be called again if
                    //          requestExit() wasn't called.
                    // 2) once: if threadLoop() returns false, the thread will exit upon return.
                    virtual bool        threadLoop();

    private:

        P2NodeImp*                  mpNodeImp;

    };

    //
public:     ////                    Operations.

                                    P2NodeImp(ePass2Type const type);

                                    ~P2NodeImp();

    virtual MERROR                  config(ConfigParams const& rParams);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipelineNode Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.


    virtual MERROR                  init(InitParams const& rParams);

    virtual MERROR                  uninit();

    virtual MERROR                  flush();

    virtual MERROR                  queue(
                                        android::sp<IPipelineFrame> pFrame
                                    );

protected:  ////                    Operations.
    MERROR                          onDequeRequest(
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MERROR                          onWaitCopyJob();
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );
    MERROR                          verifyConfigParams(
                                        ConfigParams const & rParams
                                    ) const;

public:     ////                    StreamControl

    MERROR                          getInfoIOMapSet(
                                        sp<IPipelineFrame> const& pFrame,
                                        IPipelineFrame::InfoIOMapSet& rIOMapSet
                                    ) const;

    MBOOL                           isInImageStream(
                                        StreamId_T const streamId
                                    ) const;

    MBOOL                           isInMetaStream(
                                        StreamId_T const streamId
                                    ) const;

    MERROR                          acquireImageStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer>& rpStreamBuffer
                                    );

    MVOID                           releaseImageStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IImageStreamBuffer> const pStreamBuffer,
                                        MUINT32 const status
                                    ) const;

    MERROR                          acquireImageBuffer(
                                        StreamId_T const streamId,
                                        sp<IImageStreamBuffer> const pStreamBuffer,
                                        sp<IImageBuffer>& rpImageBuffer
                                    ) const;

    MVOID                           releaseImageBuffer(
                                        sp<IImageStreamBuffer> const rpStreamBuffer,
                                        sp<IImageBuffer> const pImageBuffer
                                    ) const;

    MERROR                          acquireMetaStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer>& rpStreamBuffer
                                    );

    MVOID                           releaseMetaStream(
                                        sp<IPipelineFrame> const& pFrame,
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        MUINT32 const status
                                    ) const;

    MERROR                          acquireMetadata(
                                        StreamId_T const streamId,
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        IMetadata*& rpMetadata
                                    ) const;

    MVOID                           releaseMetadata(
                                        sp<IMetaStreamBuffer> const pStreamBuffer,
                                        IMetadata* const pMetadata
                                    ) const;

    MVOID                           onPartialFrameDone(
                                        sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           onFrameDone(
                                        sp<IPipelineFrame> const& pFrame
                                    );

public:

    MERROR                          mapToJobs(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MERROR                          mapPortId(
                                        StreamId_T const streamId, // [in]
                                        MUINT32 const transform,   // [in]
                                        MUINT32& rOccupied,        // [in/out]
                                        PortID&  rPortId           // [out]
                                    ) const;

    MVOID                           waitForRequestDrained() const;

    MVOID                           waitForCopyJobDrained() const;

inline MBOOL                        isFullRawLocked(StreamId_T const streamId) const {
                                        for( size_t i = 0; i < mpvInFullRaw.size(); i++ ) {
                                            if( isStream(mpvInFullRaw[i], streamId) )
                                                return MTRUE;
                                        }
                                        return MFALSE;
                                    }

inline MBOOL                        isResizeRawLocked(StreamId_T const streamId) const {
                                        return isStream(mpInResizedRaw, streamId);
                                    }

protected:
    MERROR                          threadSetting();

protected:  ////                    LOGE & LOGI on/off
    MINT32                          mLogLevel;

protected:  ////                    Data Members. (Config)
    ePass2Type const                mType;
    mutable RWLock                  mConfigRWLock;
    // meta
    sp<IMetaStreamInfo>             mpInAppMeta_Request;
    sp<IMetaStreamInfo>             mpInHalMeta_P1;
    sp<IMetaStreamInfo>             mpOutAppMeta_Result;
    sp<IMetaStreamInfo>             mpOutHalMeta_Result;

    // image
    Vector<sp<IImageStreamInfo> >   mpvInFullRaw;
    sp<IImageStreamInfo>            mpInResizedRaw;
    ImageStreamInfoSetT             mvOutImages;
    sp<IImageStreamInfo>            mpOutFd;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    Condition                       mRequestQueueCond;
    Que_T                           mRequestQueue;
    MBOOL                           mbRequestDrained;
    mutable Condition               mbRequestDrainedCond;
    MBOOL                           mbRequestExit;

protected:  ////                    Copy
    mutable Mutex                   mCopyJobLock;
    Condition                       mCopyJobCond;
    MBOOL                           mbCopyDrained;
    mutable Condition               mCopyDrainedCond;
    MBOOL                           mbExitCopy;
    //
private:   ////                     Threads
    sp<EnqueThread>                 mpEnqueThread;
    sp<CopyThread>                  mpCopyThread;

private:
    mutable Mutex                   mP2JobLock;
    sp<P2Processor>                 mpP2Processor;
    sp<MDPProcessor>                mpMdpProcessor;
    JobQueue                        mqP2Job;
    JobQueue                        mqMdpJob;
};


/******************************************************************************
 *
 ******************************************************************************/
android::sp<P2Node>
P2Node::
createInstance(ePass2Type const type)
{
    if( type < 0 ||
        type >= PASS2_TYPE_TOTAL )
    {
        MY_LOGE("not supported p2 type %d", type);
        return NULL;
    }
    //
    return new P2NodeImp(type);
}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
P2NodeImp(ePass2Type const type)
    : BaseNode()
    , P2Node()
    //
    , mType(type)
    , mConfigRWLock()
    //
    , mpInAppMeta_Request()
    , mpInHalMeta_P1()
    , mpOutAppMeta_Result()
    , mpOutHalMeta_Result()
    //
    , mpvInFullRaw()
    , mpInResizedRaw()
    , mvOutImages()
    , mpOutFd()
    //
    , mRequestQueueLock()
    , mRequestQueueCond()
    , mRequestQueue()
    , mbRequestDrained(MFALSE)
    , mbRequestDrainedCond()
    , mbRequestExit(MFALSE)
    //
    , mCopyJobLock()
    , mCopyJobCond()
    , mbCopyDrained(MFALSE)
    , mCopyDrainedCond()
    , mbExitCopy(MFALSE)
    //
    , mpEnqueThread(NULL)
    , mpCopyThread(NULL)
    //
    , mP2JobLock()
    , mpP2Processor(NULL)
    , mpMdpProcessor(NULL)
{
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("debug.camera.log.P2Node", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
P2NodeImp::
~P2NodeImp()
{
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
init(InitParams const& rParams)
{
    FUNC_START;
    //
    mOpenId = rParams.openId;
    mNodeId = rParams.nodeId;
    mNodeName = rParams.nodeName;

    mpEnqueThread = new EnqueThread(this);
    if( mpEnqueThread->run(P2THREAD_NAME_ENQUE) != OK ) {
        return UNKNOWN_ERROR;
    }
    //
    mpCopyThread = new CopyThread(this);
    if( mpCopyThread->run(P2THREAD_NAME_COPY) != OK ) {
        return UNKNOWN_ERROR;
    }
    //
    MY_LOGD("OpenId %d, nodeId %d, name %s",
            getOpenId(), getNodeId(), getNodeName() );
    //
    MRect activeArray;
    {
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }
        IMetadata static_meta = pMetadataProvider->geMtktStaticCharacteristics();
        if( tryGetMetadata<MRect>(&static_meta, MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION, activeArray) ) {
            MY_LOGD_IF(1,"active array(%d, %d, %dx%d)",
                    activeArray.p.x, activeArray.p.y, activeArray.s.w, activeArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
            return UNKNOWN_ERROR;
        }
    }
    //
    {
        mpP2Processor  = NULL;
        //
        P2Processor::InitParams param;
        param.openId      = mOpenId;
        param.type        = mType;
        param.activeArray = activeArray;
        //
        mpP2Processor = P2Processor::create(mLogLevel >= 1, param);
        if( mpP2Processor == NULL )
            return BAD_VALUE;
    }
    //
    {
        mpMdpProcessor = NULL;
        mpMdpProcessor = MDPProcessor::create(mLogLevel >= 1);
        if( mpMdpProcessor == NULL )
            return BAD_VALUE;
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
config(ConfigParams const& rParams)
{
    //
    {
        MERROR const err = verifyConfigParams(rParams);
        if( err != OK ) {
            MY_LOGE("verifyConfigParams failed, err = %d", err);
            return err;
        }
    }
    //
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        // meta
        mpInAppMeta_Request  = rParams.pInAppMeta;
        mpInHalMeta_P1       = rParams.pInHalMeta;
        mpOutAppMeta_Result  = rParams.pOutAppMeta;
        mpOutHalMeta_Result  = rParams.pOutHalMeta;
        // image
        mpvInFullRaw         = rParams.pvInFullRaw;
        mpInResizedRaw       = rParams.pInResizedRaw;
        mvOutImages          = rParams.vOutImage;
        mpOutFd              = rParams.pOutFDImage;
        //
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
uninit()
{
    FUNC_START;
    //
    if ( OK != flush() )
        MY_LOGE("flush failed");
    //
    // exit threads
    mpEnqueThread->requestExit();
    mpCopyThread->requestExit();
    // join
    mpEnqueThread->join();
    mpCopyThread->join();
    //
    mpEnqueThread = NULL;
    mpCopyThread = NULL;
    //
    if( mpP2Processor.get() ) {
        mpP2Processor->waitIdle();
        mpP2Processor = NULL;
    }
    //
    if( mpMdpProcessor.get() ) {
        mpMdpProcessor->waitIdle();
        mpMdpProcessor = NULL;
    }
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
flush()
{
    FUNC_START;
    //
    // 1. clear requests
    {
        Mutex::Autolock _l(mRequestQueueLock);
        //
        Que_T::iterator it = mRequestQueue.begin();
        while ( it != mRequestQueue.end() ) {
            BaseNode::flush(*it);
            it = mRequestQueue.erase(it);
        }
        CAM_TRACE_INT("request(p2)", mRequestQueue.size());
    }
    // 2. clear jobs
    {
        Mutex::Autolock _l(mP2JobLock);
        mqP2Job.clear();
    }
    //
    {
        Mutex::Autolock _l(mCopyJobLock);
        mqMdpJob.clear();
    }
    //
    // 3. wait enque thread
    waitForRequestDrained();
    //
    // 4. wait copy thread
    waitForCopyJobDrained();
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
queue(android::sp<IPipelineFrame> pFrame)
{
    FUNC_START;
    //
    if( ! pFrame.get() ) {
        MY_LOGE("Null frame");
        return BAD_VALUE;
    }

    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Make sure the request with a smaller frame number has a higher priority.
    Que_T::iterator it = mRequestQueue.end();
    for (; it != mRequestQueue.begin(); ) {
        --it;
        if  ( 0 <= (MINT32)(pFrame->getFrameNo() - (*it)->getFrameNo()) ) {
            ++it;   //insert(): insert before the current node
            break;
        }
    }
    mRequestQueue.insert(it, pFrame);
    mRequestQueueCond.signal();
    //
    CAM_TRACE_INT("request(p2)", mRequestQueue.size());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    //
    //  Wait until the queue is not empty or not going exit
    while ( mRequestQueue.empty() && ! mbRequestExit )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu",
                status, ::strerror(-status), mRequestQueue.size()
            );
        }
    }
    //
    if  ( mbRequestExit ) {
        MY_LOGW_IF(!mRequestQueue.empty(), "[flush] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    //
    CAM_TRACE_INT("request(p2)", mRequestQueue.size());
    //
    FUNC_END;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
onWaitCopyJob()
{
    Mutex::Autolock _l(mCopyJobLock);
    while ( !mqMdpJob.size() && ! mbExitCopy )
    {
        // set dained flag
        mbCopyDrained = MTRUE;
        mCopyDrainedCond.signal();
        //
        status_t status = mCopyJobCond.wait(mCopyJobLock);
        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mdp job size:%zu, exitCopy:%d",
                status, ::strerror(-status), mqMdpJob.size(), mbExitCopy
            );
        }
    }
    //
    if  ( mbExitCopy ) {
        MY_LOGW_IF(mqMdpJob.size(), "[exitCopy] mCopyJobList.size:%zu", mqMdpJob.size());
        return DEAD_OBJECT;
    }
    //
    mbCopyDrained = MFALSE;
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //FUNC_START;
    if( mpP2Processor == NULL ) {
        MY_LOGW("may not configured yet");
        BaseNode::flush(pFrame);
        return;
    }
    //
    // 1. map IPipelineFrame to jobs
    if( OK != mapToJobs(pFrame) ) {
        MY_LOGW("map to jobs failed");
        BaseNode::flush(pFrame);
        return;
    }
    //
    // 2. execute p2 jobs
    do {
        sp<Job> pJob = NULL;
        {
            Mutex::Autolock _l(mP2JobLock);
            pJob = mqP2Job.get();
        }
        //
        if( pJob == NULL )
            break;
        //
        MY_LOGE_IF(pJob->execute() != OK, "p2 job execute failed");
    } while( 1 );
    //
    //FUNC_END;
    return;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
verifyConfigParams(
    ConfigParams const & rParams
) const
{
    if  ( ! rParams.pInAppMeta.get() ) {
        MY_LOGE("no in app meta");
        return BAD_VALUE;
    }
    if  ( ! rParams.pInHalMeta.get() ) {
        MY_LOGE("no in hal meta");
        return BAD_VALUE;
    }
    //if  ( ! rParams.pOutAppMeta.get() ) {
    //    return BAD_VALUE;
    //}
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}
    if  (  rParams.pvInFullRaw.size() == 0
            && ! rParams.pInResizedRaw.get() ) {
        MY_LOGE("no in image fullraw or resized raw");
        return BAD_VALUE;
    }
    if  (  0 == rParams.vOutImage.size() ) {
        MY_LOGE("no out yuv image");
        return BAD_VALUE;
    }
    //
#define dumpStreamIfExist(str, stream)                         \
    do {                                                       \
        MY_LOGD_IF(stream.get(), "%s: id %#"PRIxPTR", %s",     \
                str,                                           \
                stream->getStreamId(), stream->getStreamName() \
               );                                              \
    } while(0)
    //
    dumpStreamIfExist("[meta] in app", rParams.pInAppMeta);
    dumpStreamIfExist("[meta] in hal", rParams.pInHalMeta);
    dumpStreamIfExist("[meta] out app", rParams.pOutAppMeta);
    dumpStreamIfExist("[meta] out hal", rParams.pOutHalMeta);
    for( size_t i = 0; i < rParams.pvInFullRaw.size(); i++ ) {
        dumpStreamIfExist("[img] in full", rParams.pvInFullRaw[i]);
    }
    dumpStreamIfExist("[img] in resized", rParams.pInResizedRaw);
    for( size_t i = 0; i < rParams.vOutImage.size(); i++ ) {
        dumpStreamIfExist("[img] out yuv", rParams.vOutImage[i]);
    }
    dumpStreamIfExist("[img] out fd", rParams.pOutFDImage);
#undef dumpStreamIfExist
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
getInfoIOMapSet(
    sp<IPipelineFrame> const& pFrame,
    IPipelineFrame::InfoIOMapSet& rIOMapSet
) const
{
    if( OK != pFrame->queryInfoIOMapSet( getNodeId(), rIOMapSet ) ) {
        MY_LOGE("queryInfoIOMap failed");
        return NAME_NOT_FOUND;
    }
    //
    // do some check
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = rIOMapSet.mImageInfoIOMapSet;
    if( ! imageIOMapSet.size() ) {
        MY_LOGW("no imageIOMap in frame");
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < imageIOMapSet.size(); i++ ) {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[i];
        if( imageIOMap.vIn.size() != 1 || imageIOMap.vOut.size() == 0) {
            MY_LOGE("[img] #%d wrong size vIn %d, vOut %d",
                    i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel >= 1, "frame %zu:[img] #%zu, in %d, out %d",
                pFrame->getFrameNo(), i, imageIOMap.vIn.size(), imageIOMap.vOut.size());
    }
    //
    IPipelineFrame::MetaInfoIOMapSet& metaIOMapSet = rIOMapSet.mMetaInfoIOMapSet;
    if( ! metaIOMapSet.size() ) {
        MY_LOGW("no metaIOMap in frame");
        return BAD_VALUE;
    }
    //
    for( size_t i = 0; i < metaIOMapSet.size(); i++ ) {
        IPipelineFrame::MetaInfoIOMap const& metaIOMap = metaIOMapSet[i];
        if( !mpInAppMeta_Request.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInAppMeta_Request->getStreamId()) ) {
            MY_LOGE("[meta] no in app");
            return BAD_VALUE;
        }
        if( !mpInHalMeta_P1.get() ||
                0 > metaIOMap.vIn.indexOfKey(mpInHalMeta_P1->getStreamId()) ) {
            MY_LOGE("[meta] no in hal");
            return BAD_VALUE;
        }
        MY_LOGD_IF(mLogLevel >= 2, "frame %zu:[meta] #%zu: in %d, out %d",
                pFrame->getFrameNo(), i, metaIOMap.vIn.size(), metaIOMap.vOut.size());
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInImageStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    //
    if( isFullRawLocked(streamId) || isResizeRawLocked(streamId) )
        return MTRUE;
    //
    MY_LOGD_IF(0, "stream id %p is not in-stream", streamId);
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2NodeImp::
isInMetaStream(
    StreamId_T const streamId
) const
{
    RWLock::AutoRLock _l(mConfigRWLock);
    return isStream(mpInAppMeta_Request, streamId) || isStream(mpInHalMeta_P1, streamId);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireImageStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IImageStreamBuffer>& rpStreamBuffer
)
{
    return ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            pFrame->getStreamBufferSet(),
            rpStreamBuffer
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageStream(
    sp<IPipelineFrame> const& pFrame,
    sp<IImageStreamBuffer> const pStreamBuffer,
    MUINT32 const status
) const
{
    IStreamBufferSet& streamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer == NULL ) {
        MY_LOGE("pStreamBuffer == NULL");
        return;
    }
    //
    if( isInImageStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status&eStreamStatus_ERROR) ?
                STREAM_BUFFER_STATUS::WRITE_ERROR :
                STREAM_BUFFER_STATUS::WRITE_OK
                );
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    streamBufferSet.markUserStatus(
            streamId,
            getNodeId(),
            ((status != eStreamStatus_NOT_USED) ? IUsersManager::UserStatus::USED : 0) |
            IUsersManager::UserStatus::RELEASE
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireImageBuffer(
    StreamId_T const streamId,
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer>& rpImageBuffer
) const
{
    if( pStreamBuffer == NULL ) {
        MY_LOGE("pStreamBuffer == NULL");
        return BAD_VALUE;
    }
    //  Query the group usage.
    MUINT const groupUsage = pStreamBuffer->queryGroupUsage(getNodeId());
    sp<IImageBufferHeap>   pImageBufferHeap =
        isInImageStream(streamId) ?
        pStreamBuffer->tryReadLock(getNodeName()) :
        pStreamBuffer->tryWriteLock(getNodeName());

    if (pImageBufferHeap == NULL) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot get ImageBufferHeap",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    rpImageBuffer = pImageBufferHeap->createImageBuffer();
    if (rpImageBuffer == NULL) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot create ImageBuffer",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }
    rpImageBuffer->lockBuf(getNodeName(), groupUsage);

    MY_LOGD_IF(mLogLevel >= 1, "stream %#"PRIxPTR": buffer: %p, usage: %p",
        streamId, rpImageBuffer.get(), groupUsage);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseImageBuffer(
    sp<IImageStreamBuffer> const pStreamBuffer,
    sp<IImageBuffer> const pImageBuffer
) const
{
    if( pStreamBuffer == NULL || pImageBuffer == NULL ) {
        MY_LOGE("pStreamBuffer %p, pImageBuffer %p should not be NULL");
        return;
    }
    //
    pImageBuffer->unlockBuf(getNodeName());
    pStreamBuffer->unlock(getNodeName(), pImageBuffer->getImageBufferHeap());
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireMetaStream(
    android::sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId,
    sp<IMetaStreamBuffer>& rpStreamBuffer
)
{
    return ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            pFrame->getStreamBufferSet(),
            rpStreamBuffer
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseMetaStream(
    android::sp<IPipelineFrame> const& pFrame,
    sp<IMetaStreamBuffer> const pStreamBuffer,
    MUINT32 const status
) const
{
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    StreamId_T const streamId = pStreamBuffer->getStreamInfo()->getStreamId();
    //
    if( pStreamBuffer.get() == NULL ) {
        MY_LOGE("StreamId %d: pStreamBuffer == NULL",
                streamId);
        return;
    }
    //
    //Buffer Producer must set this status.
    if( !isInMetaStream(streamId) ) {
        pStreamBuffer->markStatus(
                (status&eStreamStatus_ERROR) ?
                STREAM_BUFFER_STATUS::WRITE_ERROR :
                STREAM_BUFFER_STATUS::WRITE_OK
                );
    }
    //
    //  Mark this buffer as USED by this user.
    //  Mark this buffer as RELEASE by this user.
    rStreamBufferSet.markUserStatus(
            streamId, getNodeId(),
            ((status != eStreamStatus_NOT_USED) ? IUsersManager::UserStatus::USED : 0) |
            IUsersManager::UserStatus::RELEASE
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
acquireMetadata(
    StreamId_T const streamId,
    sp<IMetaStreamBuffer> const pStreamBuffer,
    IMetadata*& rpMetadata
) const
{
    rpMetadata = isInMetaStream(streamId) ?
        pStreamBuffer->tryReadLock(getNodeName()) :
        pStreamBuffer->tryWriteLock(getNodeName());

    if( rpMetadata == NULL ) {
        MY_LOGE("[node:%d][stream buffer:%s] cannot get metadata",
                getNodeId(), pStreamBuffer->getName());
        return BAD_VALUE;
    }

    MY_LOGD_IF(0,"stream %#"PRIxPTR": stream buffer %p, metadata: %p",
        streamId, pStreamBuffer.get(), rpMetadata);

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
releaseMetadata(
    sp<IMetaStreamBuffer> const pStreamBuffer,
    IMetadata* const pMetadata
) const
{
    if( pMetadata == NULL ) {
        MY_LOGW("pMetadata == NULL");
        return;
    }
    pStreamBuffer->unlock(getNodeName(), pMetadata);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onPartialFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_CALL();
    //FUNC_START;
    IStreamBufferSet&     rStreamBufferSet = pFrame->getStreamBufferSet();
    rStreamBufferSet.applyRelease(getNodeId());
    //FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
onFrameDone(
    sp<IPipelineFrame> const& pFrame
)
{
    CAM_TRACE_CALL();
    //MY_LOGD("frame %u done", pFrame->getFrameNo());
    onDispatchFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
mapToJobs(
    android::sp<IPipelineFrame> const& pFrame
)
{
    //
    // 1. get IOMap
    IPipelineFrame::InfoIOMapSet IOMapSet;
    if( OK != getInfoIOMapSet(pFrame, IOMapSet) ) {
        MY_LOGE("queryInfoIOMap failed");
        return BAD_VALUE;
    }
    //
    // 2. create metadata handle (based on IOMap)
    sp<MetaHandle> pMeta_InApp  = mpInAppMeta_Request.get() ?
        MetaHandle::create(this, pFrame, mpInAppMeta_Request->getStreamId()) : NULL;
    sp<MetaHandle> pMeta_InHal  = mpInHalMeta_P1.get() ?
        MetaHandle::create(this, pFrame, mpInHalMeta_P1->getStreamId()) : NULL;
    sp<MetaHandle> pMeta_OutApp = mpOutAppMeta_Result.get() ?
        MetaHandle::create(this, pFrame, mpOutAppMeta_Result->getStreamId()) : NULL;
    sp<MetaHandle> pMeta_OutHal = mpOutHalMeta_Result.get() ?
        MetaHandle::create(this, pFrame, mpOutHalMeta_Result->getStreamId()) : NULL;
    //
    if( pMeta_InApp  == NULL ||
        pMeta_InHal  == NULL )
    {
        MY_LOGW("meta check failed");
        return BAD_VALUE;
    }
    // 3. create FrameLifeControl
    sp<FrameLifeControl> pFrameLife = new FrameLifeControl(this, pFrame, mLogLevel >= 1);
    // 4. process image IO
    IPipelineFrame::ImageInfoIOMapSet& imageIOMapSet = IOMapSet.mImageInfoIOMapSet;
    for( size_t run_idx = 0 ; run_idx < imageIOMapSet.size(); run_idx++ )
    {
        IPipelineFrame::ImageInfoIOMap const& imageIOMap = imageIOMapSet[run_idx];
        //
        P2Processor::FrameParams param_p2;
        MDPProcessor::FrameParams param_mdp;
        // source
        {
            StreamId_T const streamId = imageIOMap.vIn.keyAt(0);
            param_p2.in.mPortId = PORT_IMGI;
            param_p2.in.mHandle = StreamBufferHandle::create(this, pFrame, streamId);
            //
            {
                RWLock::AutoRLock _l(mConfigRWLock);
                param_p2.bResized = isResizeRawLocked(streamId);
            }
        }
        // destination
        MUINT32 occupied = 0;
        for( size_t i = 0; i < imageIOMap.vOut.size(); i++ )
        {
            StreamId_T const streamId = imageIOMap.vOut.keyAt(i);
            PortID port_p2;
            if( OK == mapPortId(streamId, 0, occupied, port_p2) ) {
                P2Processor::FrameOutput out;
                out.mPortId = port_p2;
                out.mHandle = StreamBufferHandle::create(this, pFrame, streamId);
                out.mTransform = 0;
                //
                param_p2.vOut.push_back(out);
            } else {
                MDPProcessor::FrameOutput out;
                out.mHandle = StreamBufferHandle::create(this, pFrame, streamId);
                out.mTransform = 0;
                //
                param_mdp.vOut.push_back(out);
            }
        }
        //
        param_p2.inApp = pMeta_InApp;
        param_p2.inHal = pMeta_InHal;
        if( run_idx == 0 ) {
            param_p2.outApp = pMeta_OutApp;
            param_p2.outHal = pMeta_OutHal;
        }
        //
        MY_LOGD_IF(mLogLevel >= 1, "job: p2 out %d, copy out %d",
                param_p2.vOut.size(), param_mdp.vOut.size() );
        // create job
        sp<Job> p2job = ProcessorJob<P2Processor>::create(pFrameLife, mpP2Processor, param_p2);
        if( p2job.get() ) {
            Mutex::Autolock _l(mP2JobLock);
            mqP2Job.add(p2job);
        }
        //
        if( param_mdp.vOut.size() > 0 ) {
            param_mdp.in.mHandle = param_p2.vOut[param_p2.vOut.size() - 1].mHandle;
            //
            sp<Job> mdpjob = ProcessorJob<MDPProcessor>::create(pFrameLife, mpMdpProcessor, param_mdp);
            if( mdpjob.get() ) {
                Mutex::Autolock _l(mCopyJobLock);
                mqMdpJob.add(mdpjob);
                mCopyJobCond.signal();
            }
        }
    }
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2NodeImp::
mapPortId(
    StreamId_T const streamId, // [in]
    MUINT32 const transform,   // [in]
    MUINT32& rOccupied,        // [in/out]
    PortID&  rPortId           // [out]
) const
{
    MERROR ret = OK;
#define PORT_WDMAO_USED  (0x1)
#define PORT_WROTO_USED  (0x2)
#define PORT_IMG2O_USED  (0x4)
    if( transform != 0 ) {
        if( !(rOccupied & PORT_WROTO_USED) )
            rOccupied |= PORT_WROTO_USED;
        else
            ret = INVALID_OPERATION;
    }
    else {
#if FD_PORT_SUPPORT
        if( FD_PORT_SUPPORT && isStream(mpOutFd, streamId) ) {
            if( rOccupied & PORT_IMG2O_USED ) {
                MY_LOGW("should not be occupied");
                ret = INVALID_OPERATION;
            } else
                rOccupied |= PORT_IMG2O_USED;
                rPortId = PORT_IMG2O;
        } else
#endif
        if( !(rOccupied & PORT_WDMAO_USED) ) {
            rOccupied |= PORT_WDMAO_USED;
            rPortId = PORT_WDMAO;
        } else if( !(rOccupied & PORT_WROTO_USED) ) {
            rOccupied |= PORT_WROTO_USED;
            rPortId = PORT_WROTO;
        } else
            ret = INVALID_OPERATION;
    }
    MY_LOGD_IF(0, "stream id %#"PRIxPTR", occupied %p",
            streamId, rOccupied);
    return ret;
#undef PORT_WDMAO_USED
#undef PORT_WROTO_USED
#undef PORT_IMG2O_USED
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
waitForRequestDrained() const
{
    FUNC_START;
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
    FUNC_END;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2NodeImp::
waitForCopyJobDrained() const
{
    FUNC_START;
    //
    Mutex::Autolock _l(mCopyJobLock);
    if( !mbCopyDrained ) {
        MY_LOGD("wait for copy job drained");
        mCopyDrainedCond.wait(mCopyJobLock);
    }
    //
    FUNC_END;
}


MERROR
P2NodeImp::
threadSetting()
{
    //
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    MINT tid;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if (P2THREAD_POLICY == SCHED_OTHER) {
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, P2THREAD_POLICY, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, P2THREAD_PRIORITY);   //  Note: "priority" is nice value.
    } else {
        sched_p.sched_priority = P2THREAD_PRIORITY;          //  Note: "priority" is real-time priority.
        ::sched_setscheduler(0, P2THREAD_POLICY, &sched_p);
    }

    MY_LOGD("tid(%d) policy(%d) priority(%d)", ::gettid(), P2THREAD_POLICY, P2THREAD_PRIORITY);

    return OK;

}


/******************************************************************************
 *
 ******************************************************************************/
void
P2NodeImp::EnqueThread::
requestExit()
{
    Mutex::Autolock _l(mpNodeImp->mRequestQueueLock);
    mpNodeImp->mbRequestExit = MTRUE;
    mpNodeImp->mRequestQueueCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P2NodeImp::EnqueThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P2NodeImp::EnqueThread::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            ! exitPending()
        &&  OK == mpNodeImp->onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        mpNodeImp->onProcessFrame(pFrame);

        return true;
    }

    MY_LOGD("exit enque thread");
    return  false;

}


/******************************************************************************
 *
 ******************************************************************************/
void
P2NodeImp::CopyThread::
requestExit()
{
    Mutex::Autolock _l(mpNodeImp->mCopyJobLock);
    mpNodeImp->mbExitCopy = MTRUE;
    mpNodeImp->mCopyJobCond.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
status_t
P2NodeImp::CopyThread::
readyToRun()
{
    return mpNodeImp->threadSetting();
}


/******************************************************************************
 *
 ******************************************************************************/
bool
P2NodeImp::CopyThread::
threadLoop()
{
    while(
            ! exitPending()
        && OK == mpNodeImp->onWaitCopyJob()
        )
    {
        sp<Job> pJob = NULL;
        {
            Mutex::Autolock _l(mpNodeImp->mCopyJobLock);
            pJob = mpNodeImp->mqMdpJob.get();
        }
        //
        if( pJob.get() )
            MY_LOGE_IF(pJob->execute() != OK, "mdp job execute failed");
        else
            MY_LOGW("no mdp job");
        return true;
    }
    MY_LOGD("exit copy thread");
    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<Job>
JobQueue::
get()
{
    sp<Job> pJob = NULL;
    // get a job
    if( mvPending.size() == 0 ) {
        return NULL;
    }
    //
    pJob = mvPending.front();
    mvPending.erase( mvPending.begin() );

    return pJob;
}
/******************************************************************************
 *
 ******************************************************************************/
sp<MetaHandle>
MetaHandle::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{
    // check StreamBuffer here
    sp<IMetaStreamBuffer> pStreamBuffer = NULL;
    if( pCtrl && OK == pCtrl->acquireMetaStream(
                pFrame,
                streamId,
                pStreamBuffer) )
    {
        IMetadata* pMeta = NULL;
        if( OK == pCtrl->acquireMetadata(
                    streamId,
                    pStreamBuffer,
                    pMeta
                    ) )
        {
            BufferState_t const init_state =
                pCtrl->isInMetaStream(streamId) ? STATE_READABLE : STATE_WRITABLE;
            return new MetaHandle(
                    pCtrl,
                    pFrame,
                    streamId,
                    pStreamBuffer,
                    init_state,
                    pMeta
                    );
        }
        else {
            pCtrl->releaseMetaStream(pFrame, pStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
        }
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
MetaHandle::
~MetaHandle()
{
    if( muState != STATE_NOT_USED )
    {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
            StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseMetadata(mpStreamBuffer, mpMetadata);
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, status);
    }
    else
    {
        mpStreamCtrl->releaseMetaStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }

}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
MetaHandle::
updateState(BufferState_t const state)
{
    Mutex::Autolock _l(mLock);
    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#"PRIxPTR" state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#"PRIxPTR" set fail, state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    //mCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<BufferHandle>
StreamBufferHandle::
create(
    StreamControl* const pCtrl,
    sp<IPipelineFrame> const& pFrame,
    StreamId_T const streamId
)
{
    // check StreamBuffer here
    sp<IImageStreamBuffer> pStreamBuffer = NULL;
    if( OK == pCtrl->acquireImageStream(
                pFrame,
                streamId,
                pStreamBuffer) )
    {
        return new StreamBufferHandle(
                pCtrl,
                pFrame,
                streamId,
                pStreamBuffer
                );
    }
    //
    return NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
StreamBufferHandle::
~StreamBufferHandle()
{
    if( muState != STATE_NOT_USED )
    {
        const MUINT32 status = (muState != STATE_WRITE_FAIL) ?
            StreamControl::eStreamStatus_FILLED : StreamControl::eStreamStatus_ERROR;
        //
        mpStreamCtrl->releaseImageBuffer(mpStreamBuffer, mpImageBuffer);
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, status);
    }
    else
    {
        mpStreamCtrl->releaseImageStream(mpFrame, mpStreamBuffer, StreamControl::eStreamStatus_NOT_USED);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
StreamBufferHandle::
waitState(
    BufferState_t const state,
    nsecs_t const nsTimeout
)
{
    Mutex::Autolock _l(mLock);
    if( mpImageBuffer == NULL ) {
        // get buffer from streambuffer
        const MERROR ret = mpStreamCtrl->acquireImageBuffer(mStreamId, mpStreamBuffer, mpImageBuffer);
        // update initial state
        if( ret == OK )
            muState = mpStreamCtrl->isInImageStream(mStreamId) ? STATE_READABLE : STATE_WRITABLE;
        //return ret;
    }
    //
    if( muState != state ) {
        mCond.waitRelative(mLock, nsTimeout);
    }
    return (muState == state) ? OK : TIMED_OUT;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
StreamBufferHandle::
updateState(BufferState_t const state)
{
    Mutex::Autolock _l(mLock);
    if( muState == STATE_NOT_USED ) {
        MY_LOGW("streamId %#"PRIxPTR" state %d -> %d",
            mStreamId, muState, state);
    }
    else {
        MY_LOGW_IF(state == STATE_WRITE_FAIL, "streamId %#"PRIxPTR" set fail: state %d -> %d",
                mStreamId, muState, state);
        muState = state;
    }
    mCond.broadcast();
}


/******************************************************************************
 *
 ******************************************************************************/
sp<P2Processor>
P2Processor::
create(MBOOL const enableLog, InitParams const& params)
{
    INormalStream* pPipe = NULL;
    IHal3A*        p3A   = NULL;
    ENormalStreamTag streamtag = ENormalStreamTag_Stream;
    if( params.type == P2Node::PASS2_STREAM ) {
        streamtag = ENormalStreamTag_Stream;
    }
    else if ( params.type == P2Node::PASS2_TIMESHARING ) {
        streamtag = ENormalStreamTag_Vss;
    }
    //
    pPipe = INormalStream::createInstance(LOG_TAG, streamtag, params.openId, true);
    if( pPipe == NULL ) {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }
    //
    if( ! pPipe->init() )
    {
        MY_LOGE("pipe init failed");
        goto lbExit;
    }
    //
    p3A = IHal3A::createInstance(IHal3A::E_Camera_3, params.openId, LOG_TAG);
    if( p3A == NULL ) {
        MY_LOGE("create 3A failed");
        goto lbExit;
    }
    MY_LOGD("create processor type %d: pipe %p, 3A %p",
            params.type, pPipe, p3A);
    //
lbExit:
    if( ! pPipe || !p3A ) {
        if( pPipe ) {
            pPipe->uninit();
            pPipe->destroyInstance(LOG_TAG);
            pPipe = NULL;
        }
        if( p3A ) {
            p3A->destroyInstance(LOG_TAG);
            p3A = NULL;
        }
    }
    return pPipe ? new P2Processor(enableLog, params, pPipe, p3A) : NULL;
}


/******************************************************************************
 *
 ******************************************************************************/
P2Processor::
~P2Processor()
{
    MY_LOGD("destroy processor %d: %p",
            mInitParams.type, mpPipe);
    //
    if( mpPipe ) {
        if( ! mpPipe->uninit() ) {
            MY_LOGE("pipe uninit failed");
        }
        mpPipe->destroyInstance(LOG_TAG);
    }

    if( mp3A ) {
        mp3A->destroyInstance(LOG_TAG);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Processor::
waitIdle()
{
    Mutex::Autolock _l(mLock);
    while( mvRunning.size() ) {
        MY_LOGD("wait job done %d", mvRunning.size());
        mCondJob.wait(mLock);
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Processor::
onExecute(
    sp<Job> const pJob,
    FrameParams const params
)
{
    CAM_TRACE_NAME("p2");
    //
    MERROR ret = OK;
    //
    if ( OK != (ret = checkParams(params)) )
        return ret;
    // prepare metadata
    IMetadata* pMeta_InApp  = params.inApp->getMetadata();
    IMetadata* pMeta_InHal  = params.inHal->getMetadata();
    IMetadata* pMeta_OutApp = params.outApp.get() ? params.outApp->getMetadata() : NULL;
    IMetadata* pMeta_OutHal = params.outHal.get() ? params.outHal->getMetadata() : NULL;
    //
    if( pMeta_InApp == NULL || pMeta_InHal == NULL ) {
        MY_LOGE("meta: in app %p, in hal %p", pMeta_InApp, pMeta_InHal);
        return BAD_VALUE;
    }
    //
    crop_infos cropInfos;
    if( OK != (ret = getCropInfos(pMeta_InApp, pMeta_InHal, params.bResized, cropInfos)) ) {
        MY_LOGE("getCropInfos failed");
        return ret;
    }
    //
    QParams enqueParams;
    // input
    {
        if( OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW("src buffer err = %d", ret);
            return BAD_VALUE;
        }
        IImageBuffer* pSrc = params.in.mHandle->getBuffer();
        //
        Input src;
        src.mPortID       = params.in.mPortId;
        src.mPortID.group = 0;
        src.mBuffer       = pSrc;
        // update src size
        pSrc->setExtParam(cropInfos.dstsize_resizer);
        //
        enqueParams.mvIn.push_back(src);
    }
    //
    // output
    for( size_t i = 0; i < params.vOut.size(); i++ )
    {
        if( params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE)) ) {
            MY_LOGW("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer* pDst = params.vOut[i].mHandle->getBuffer();
        //
        Output dst;
        dst.mPortID       = params.vOut[i].mPortId;
        dst.mPortID.group = 0;
        dst.mBuffer       = pDst;
        //
        enqueParams.mvOut.push_back(dst);
    }

    if( enqueParams.mvOut.size() == 0 ) {
        MY_LOGW("no dst buffer");
        return BAD_VALUE;
    }

    {
        void* pTuning = NULL;
        unsigned int tuningsize;
        if( !mpPipe->deTuningQue(tuningsize, pTuning) ) {
            MY_LOGW("cannot get tunning buffer");
            return BAD_VALUE;
        }
        //
        MetaSet_T inMetaSet;
        MetaSet_T outMetaSet;
        //
        inMetaSet.appMeta = *pMeta_InApp;
        inMetaSet.halMeta = *pMeta_InHal;
        //
        MBOOL const bGetResult = (pMeta_OutApp || pMeta_OutHal);
        //
        if( pMeta_OutHal ) {
            outMetaSet.halMeta = *pMeta_InHal;
        }
        //
        mp3A->setIsp(0, inMetaSet, pTuning, bGetResult ? &outMetaSet : NULL);
        //
        if( pMeta_OutApp ) {
            *pMeta_OutApp = outMetaSet.appMeta;
            //
            MRect cropRegion = cropInfos.crop_a;
            if( cropInfos.isEisEabled ) {
                cropRegion.p.x += cropInfos.eis_mv_a.p.x;
                cropRegion.p.y += cropInfos.eis_mv_a.p.y;
            }
            //
            updateCropRegion(cropRegion, pMeta_OutApp);
        }
        //
        if( pMeta_OutHal ) {
            *pMeta_OutHal = outMetaSet.halMeta;
        }
        //
        enqueParams.mpTuningData.push_back(pTuning);
    }

    {
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = cropInfos.dstsize_resizer;
        crop1.mResizeDst  = cropInfos.dstsize_resizer;
        MCrpRsInfo crop2;
        crop2.mGroupID = 2;
        //crop2.mCropRect = calCrop(info, dstSize); //TODO
        //crop2.mResizeDst  = MSize(0,0);
        Vector<Output>::const_iterator iter = enqueParams.mvOut.begin();
        while( iter != enqueParams.mvOut.end() ) {
            if( iter->mPortID == PORT_WDMAO
                || iter->mPortID == PORT_WROTO ) {
                calcCrop_viewangle(cropInfos, iter->mBuffer->getImgSize(), crop2.mCropRect);
                crop2.mResizeDst = iter->mBuffer->getImgSize();
                break;
            }
            iter++;
        }
        enqueParams.mvCropRsInfo.push_back(crop1);
        enqueParams.mvCropRsInfo.push_back(crop2);
    }
    //
    // callback
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie     = this;
    //
    enqueParams.mvPrivaData.push_back(NULL);

    // for crop
    enqueParams.mvP1SrcCrop.push_back(cropInfos.crop_p1_sensor);
    enqueParams.mvP1Dst.push_back(cropInfos.dstsize_resizer);
    enqueParams.mvP1DstCrop.push_back(cropInfos.crop_dma);

    //
    MY_LOGD_IF(mbEnableLog, "cnt %d, in %d, out %d",
            muEnqueCnt, enqueParams.mvIn.size(), enqueParams.mvOut.size() );
    //
    { // add job to queue
        Mutex::Autolock _l(mLock);
        mvRunning.push_back(pJob);
        muEnqueCnt++;
    }
    //
    {
        CAM_TRACE_NAME("drv_enq");
        if( !mpPipe->enque(enqueParams) )
        {
            MY_LOGE("enque pass2 failed");
            //
            { // remove job from queue
                Mutex::Autolock _l(mLock);
                vector<sp<Job> >::iterator iter = mvRunning.end();
                while( iter != mvRunning.begin() ) {
                    iter--;
                    if( *iter == pJob ) {
                        mvRunning.erase(iter);
                        break;
                    }
                }

                MY_LOGW("cnt %d execute failed", muDequeCnt);
                muDequeCnt++;
            }
            return UNKNOWN_ERROR;
        }
    }
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Processor::
onFinish(
    FrameParams const params,
    MBOOL const success
)
{
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    for( size_t i = 0; i < params.vOut.size(); i++ )
        params.vOut[i].mHandle->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
                );
    if( params.outApp.get() )
        params.outApp->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);
    if( params.outHal.get() )
        params.outHal->updateState(success ? MetaHandle::STATE_WRITE_OK : MetaHandle::STATE_WRITE_FAIL);
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Processor::
pass2CbFunc(QParams& rParams)
{
    P2Processor* pProcessor = reinterpret_cast<P2Processor*>(rParams.mpCookie);
    pProcessor->handleDeque(rParams);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Processor::
handleDeque(QParams& rParams)
{
    CAM_TRACE_NAME("p2 deque");
    sp<Job> pJob = NULL;
    {
        Mutex::Autolock _l(mLock);
        MY_LOGD_IF(mbEnableLog, "p2 done %d, success %d", muDequeCnt, rParams.mDequeSuccess);
        pJob = mvRunning.front();
        mvRunning.erase(mvRunning.begin());
        muDequeCnt++;
    }
    //
    if( rParams.mpTuningData.size() > 0 ) {
        void* pTuning = rParams.mpTuningData[0];
        if( pTuning ) mpPipe->enTuningQue(pTuning);
    }
    //
    pJob->finishAsync(rParams.mDequeSuccess);
    //
    mCondJob.signal();
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Processor::
checkParams(FrameParams const params) const
{
#define CHECK(val, fail_ret, ...) \
    do{                           \
        if( !(val) )              \
        {                         \
            MY_LOGE(__VA_ARGS__); \
            return fail_ret;      \
        }                         \
    } while(0)
    //
    CHECK( params.in.mHandle.get() , BAD_VALUE , "no src handle" );
    CHECK( params.vOut.size()      , BAD_VALUE , "no dst" );
    CHECK( params.inApp.get()      , BAD_VALUE , "no in app meta" );
    CHECK( params.inHal.get()      , BAD_VALUE , "no in hal meta" );
    //
#undef CHECK
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
P2Processor::
getCropInfos(
    IMetadata* const inApp,
    IMetadata* const inHal,
    MBOOL const isResized,
    crop_infos & cropInfos
) const
{
    if( ! tryGetMetadata<MSize>(inHal, MTK_HAL_REQUEST_SENSOR_SIZE, cropInfos.sensor_size) ) {
        MY_LOGE("cannot get MTK_HAL_REQUEST_SENSOR_SIZE");
        return BAD_VALUE;
    }
    //
    MSize const sensor = cropInfos.sensor_size;
    MSize const active = mInitParams.activeArray.s;
    //
    cropInfos.isResized = isResized;
    // get current p1 buffer crop status
    if(
            !isResized ||
            !( tryGetMetadata<MRect>(inHal, MTK_P1NODE_SCALAR_CROP_REGION, cropInfos.crop_p1_sensor) &&
               tryGetMetadata<MSize>(inHal, MTK_P1NODE_RESIZER_SIZE      , cropInfos.dstsize_resizer) &&
               tryGetMetadata<MRect>(inHal, MTK_P1NODE_DMA_CROP_REGION   , cropInfos.crop_dma)
             )
      ) {
        MY_LOGW_IF(isResized, "cannot find tag, use it as full-size");
        //
        cropInfos.crop_p1_sensor  = MRect( MPoint(0,0), sensor );
        cropInfos.dstsize_resizer = sensor;
        cropInfos.crop_dma        = MRect( MPoint(0,0), sensor );
    }
    //
    // setup transform
    cropInfos.tranActive2Sensor = simpleTransform(
                MPoint(0,0),
                active,
                sensor
            );
    //
    cropInfos.tranSensor2Resized = simpleTransform(
                cropInfos.crop_p1_sensor.p,
                cropInfos.crop_p1_sensor.s,
                cropInfos.dstsize_resizer
            );
    //
    MBOOL const isEisOn = isEISOn(inApp);
    //
    MRect cropRegion; //active array domain
    queryCropRegion(inApp, isEisOn, cropRegion);
    cropInfos.crop_a = cropRegion;
    //
    // query EIS result
    {
        eis_region eisInfo;
        if( isEisOn && queryEisRegion(inHal, eisInfo)) {
            cropInfos.isEisEabled = MTRUE;
            // calculate mv
            vector_f* pMv_s = &cropInfos.eis_mv_s;
            vector_f* pMv_r = &cropInfos.eis_mv_r;
#if 1
            //eis in sensor domain
            pMv_s->p.x  = eisInfo.x_int - (sensor.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_s->pf.x = eisInfo.x_float;
            pMv_s->p.y  = eisInfo.y_int - (sensor.h * (EIS_FACTOR-100)/2/EIS_FACTOR);

            pMv_s->pf.y = eisInfo.y_float;
            //
            cropInfos.eis_mv_r = transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_s);
            //
            MY_LOGD_IF(1, "mv (s->r): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                    pMv_s->p.x,
                    pMv_s->pf.x,
                    pMv_s->p.y,
                    pMv_s->pf.y,
                    pMv_r->p.x,
                    pMv_r->pf.x,
                    pMv_r->p.y,
                    pMv_r->pf.y
                    );
#else
            MSize const resizer = cropInfos.dstsize_resizer;
            //eis in resized domain
            pMv_r->p.x  = eisInfo.x_int - (resizer.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_r->pf.x = eisInfo.x_float;
            pMv_r->p.y  = eisInfo.y_int - (resizer.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
            pMv_r->pf.y = eisInfo.y_float;
            //
            cropInfos.eis_mv_s = inv_transform(cropInfos.tranSensor2Resized, cropInfos.eis_mv_r);
            //
            MY_LOGD_IF(1, "mv (r->s): (%d, %d, %d, %d) -> (%d, %d, %d, %d)",
                    pMv_r->p.x,
                    pMv_r->pf.x,
                    pMv_r->p.y,
                    pMv_r->pf.y,
                    pMv_s->p.x,
                    pMv_s->pf.x,
                    pMv_s->p.y,
                    pMv_s->pf.y
                    );
#endif
            cropInfos.eis_mv_a = inv_transform(cropInfos.tranActive2Sensor, cropInfos.eis_mv_s);
            MY_LOGD_IF(1, "mv in active %d/%d, %d/%d",
                    cropInfos.eis_mv_a.p.x,
                    cropInfos.eis_mv_a.pf.x,
                    cropInfos.eis_mv_a.p.y,
                    cropInfos.eis_mv_a.pf.y
                    );
        }
        else {
            cropInfos.isEisEabled = MFALSE;
            //
            // no need to set 0
            //memset(&cropInfos.eis_mv_a, 0, sizeof(vector_f));
            //memset(&cropInfos.eis_mv_s, 0, sizeof(vector_f));
            //memset(&cropInfos.eis_mv_r, 0, sizeof(vector_f));
        }
    }
    // debug
    //cropInfos.dump();
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Processor::
queryCropRegion(
    IMetadata* const meta_request,
    MBOOL const isEisOn,
    MRect& cropRegion
) const
{
    if( !tryGetMetadata<MRect>(meta_request, MTK_SCALER_CROP_REGION, cropRegion) ) {
        cropRegion.p = MPoint(0,0);
        cropRegion.s = mInitParams.activeArray.s;
        MY_LOGW_IF(mbEnableLog, "no MTK_SCALER_CROP_REGION, crop full size %dx%d",
                cropRegion.s.w, cropRegion.s.h);
    }
    MY_LOGD_IF(mbEnableLog, "control: cropRegion(%d, %d, %dx%d)",
            cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    //
    if( isEisOn ) {
        cropRegion.p.x += (cropRegion.s.w * (EIS_FACTOR-100)/2/EIS_FACTOR);
        cropRegion.p.y += (cropRegion.s.h * (EIS_FACTOR-100)/2/EIS_FACTOR);
        cropRegion.s   = cropRegion.s * 100 / EIS_FACTOR;
        MY_LOGD_IF(mbEnableLog, "EIS: factor %d, cropRegion(%d, %d, %dx%d)",
                EIS_FACTOR,
                cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);
    }
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Processor::
updateCropRegion(
    MRect const crop,
    IMetadata* meta_result
) const
{
    updateEntry<MRect>(meta_result, MTK_SCALER_CROP_REGION, crop);
    //
    MY_LOGD_IF( DEBUG_LOG && mbEnableLog, "result: cropRegion (%d, %d, %dx%d)",
            crop.p.x, crop.p.y, crop.s.w, crop.s.h);
}


/******************************************************************************
 *
 ******************************************************************************/
MVOID
P2Processor::
calcCrop_viewangle(
    crop_infos const& cropInfos,
    MSize const& dstSize,
    MCropRect& result
) const
{
    MBOOL const isResized = cropInfos.isResized;
    // coordinates: s_: sensor
    MRect const s_crop = transform(cropInfos.tranActive2Sensor, cropInfos.crop_a);
    MRect s_viewcrop;
    //
    if( s_crop.s.w * dstSize.h > s_crop.s.h * dstSize.w ) { // pillarbox
        s_viewcrop.s.w = div_round(s_crop.s.h * dstSize.w, dstSize.h);
        s_viewcrop.s.h = s_crop.s.h;
        s_viewcrop.p.x = s_crop.p.x + ((s_crop.s.w - s_viewcrop.s.w) >> 1);
        s_viewcrop.p.y = s_crop.p.y;
    }
    else { // letterbox
        s_viewcrop.s.w = s_crop.s.w;
        s_viewcrop.s.h = div_round(s_crop.s.w * dstSize.h, dstSize.w);
        s_viewcrop.p.x = s_crop.p.x;
        s_viewcrop.p.y = s_crop.p.y + ((s_crop.s.h - s_viewcrop.s.h) >> 1);
    }
    MY_LOGD_IF(0, "s_cropRegion(%d, %d, %dx%d), dst %dx%d, view crop(%d, %d, %dx%d)",
            s_crop.p.x     , s_crop.p.y     ,
            s_crop.s.w     , s_crop.s.h     ,
            dstSize.w      , dstSize.h      ,
            s_viewcrop.p.x , s_viewcrop.p.y ,
            s_viewcrop.s.w , s_viewcrop.s.h
           );
    //
    if( isResized ) {
        MRect r_viewcrop = transform(cropInfos.tranSensor2Resized, s_viewcrop);
        result.s            = r_viewcrop.s;
        result.p_integral   = r_viewcrop.p + cropInfos.eis_mv_r.p;
        result.p_fractional = cropInfos.eis_mv_r.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.dstsize_resizer, result) ) {
            MY_LOGE("[FIXME] need to check crop!");
            cropInfos.dump();
        }
    }
    else {
        result.s            = s_viewcrop.s;
        result.p_integral   = s_viewcrop.p + cropInfos.eis_mv_s.p;
        result.p_fractional = cropInfos.eis_mv_s.pf;

        // make sure hw limitation
        result.s.w &= ~(0x1);
        result.s.h &= ~(0x1);

        // check boundary
        if( refineBoundary(cropInfos.sensor_size, result) ) {
            MY_LOGE("[FIXME] need to check crop!");
            cropInfos.dump();
        }
    }
    //
    MY_LOGD_IF(mbEnableLog, "resized %d, crop %d/%d, %d/%d, %dx%d",
            isResized,
            result.p_integral.x,
            result.p_integral.y,
            result.p_fractional.x,
            result.p_fractional.y,
            result.s.w,
            result.s.h
            );
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2Processor::
isEISOn(
    IMetadata* const inApp
) const
{
    MUINT8 eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_OFF;
    if( !tryGetMetadata<MUINT8>(inApp, MTK_CONTROL_VIDEO_STABILIZATION_MODE, eisMode) ) {
        MY_LOGW_IF(mbEnableLog, "no MTK_CONTROL_VIDEO_STABILIZATION_MODE");
    }
#if FORCE_EIS_ON
    eisMode = MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
#endif
    return eisMode == MTK_CONTROL_VIDEO_STABILIZATION_MODE_ON;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
P2Processor::
queryEisRegion(
    IMetadata* const inHal,
    eis_region& region
) const
{
    IMetadata::IEntry entry = inHal->entryFor(MTK_EIS_REGION);
    if( entry.count() == 6 ) {
        region.x_int   = entry.itemAt(0, Type2Type<MINT32>());
        region.x_float = entry.itemAt(1, Type2Type<MINT32>());
        region.y_int   = entry.itemAt(2, Type2Type<MINT32>());
        region.y_float = entry.itemAt(3, Type2Type<MINT32>());
        region.s.w     = entry.itemAt(4, Type2Type<MINT32>());
        region.s.h     = entry.itemAt(5, Type2Type<MINT32>());
        MY_LOGD_IF(mbEnableLog, "%d, %d, %d, %d, %dx%d",
                region.x_int,
                region.x_float,
                region.y_int,
                region.y_float,
                region.s.w,
                region.s.h);
        return MTRUE;
    }
    //
    MY_LOGW("wrong eis region count %zu", entry.count());
    return MFALSE;
}


/******************************************************************************
 *
 ******************************************************************************/
inline
MBOOL
P2Processor::
refineBoundary(
    MSize const& bufSize,
    MCropRect& crop
) const
{
    MBOOL isRefined = MFALSE;
    MCropRect refined = crop;
    if( crop.p_integral.x < 0 ) {
        refined.p_integral.x = 0;
        isRefined = MTRUE;
    }
    if( crop.p_integral.y < 0 ) {
        refined.p_integral.y = 0;
        isRefined = MTRUE;
    }
    //
    int const carry_x = (crop.p_fractional.x != 0) ? 1 : 0;
    if( (refined.p_integral.x + crop.s.w + carry_x) > bufSize.w ) {
        refined.s.w = bufSize.w - refined.p_integral.x - carry_x;
        isRefined = MTRUE;
    }
    int const carry_y = (crop.p_fractional.y != 0) ? 1 : 0;
    if( (refined.p_integral.y + crop.s.h + carry_y) > bufSize.h ) {
        refined.s.h = bufSize.h - refined.p_integral.y - carry_y;
        isRefined = MTRUE;
    }
    //
    if( isRefined ) {
        MY_LOGE("buf size %dx%d, crop(%d/%d, %d/%d, %dx%d) -> crop(%d/%d, %d/%d, %dx%d)",
                bufSize.w, bufSize.h,
                crop.p_integral.x,
                crop.p_integral.y,
                crop.p_fractional.x,
                crop.p_fractional.y,
                crop.s.w,
                crop.s.h,
                refined.p_integral.x,
                refined.p_integral.y,
                refined.p_fractional.x,
                refined.p_fractional.y,
                refined.s.w,
                refined.s.h
                );
        crop = refined;
    }
    return isRefined;
}


/******************************************************************************
 *
 ******************************************************************************/
sp<MDPProcessor>
MDPProcessor::
create(MBOOL const enableLog)
{
    return new MDPProcessor(enableLog);
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MDPProcessor::
onExecute(
    sp<Job> const /*pJob*/,
    FrameParams const params
)
{
    CAM_TRACE_NAME("mdp");
    //
    MERROR ret = OK;
    //
    IImageBuffer* pSrc = NULL;
    vector<IImageBuffer*> vDst;
    // input
    if( params.in.mHandle.get() )
    {
        if( OK != (ret = params.in.mHandle->waitState(BufferHandle::STATE_READABLE)) ) {
            MY_LOGW("src buffer err = %d", ret);
            return ret;
        }
        pSrc = params.in.mHandle->getBuffer();
    }
    else {
        MY_LOGW("no src");
        return BAD_VALUE;
    }
    //
    // output
    for( size_t i = 0; i < params.vOut.size(); i++ )
    {
        if( params.vOut[i].mHandle == NULL ||
            OK != (ret = params.vOut[i].mHandle->waitState(BufferHandle::STATE_WRITABLE)) ) {
            MY_LOGW("dst buffer err = %d", ret);
            continue;
        }
        IImageBuffer* pDst = params.vOut[i].mHandle->getBuffer();
        //
        if( pDst != NULL )
            vDst.push_back(pDst);
    }
    //
    if( pSrc == NULL || vDst.size() == 0 ) {
        MY_LOGE("wrong mdp in/out: src %p, dst count %d", pSrc, vDst.size());
        return BAD_VALUE;
    }
    //
    MBOOL success = MFALSE;
    {
        MRect   const roi       = params.crop;
        MUINT32 const transform = 0; //TODO
        //
        NSSImager::IImageTransform* pTrans = NSSImager::IImageTransform::createInstance();
        if( !pTrans ) {
            MY_LOGE("!pTrans");
            return UNKNOWN_ERROR;
        }
        //
        success =
            pTrans->execute(
                pSrc,
                vDst[0],
                (vDst.size() > 1 )? vDst[1] : NULL,
                roi,
                transform,
                0xFFFFFFFF
                );

        pTrans->destroyInstance();
        pTrans = NULL;
    }
    //
    return success ? OK : UNKNOWN_ERROR;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
MDPProcessor::
onFinish(
    FrameParams const params,
    MBOOL const success
)
{
    //params.in.mHandle->updateState(BufferHandle::Buffer_ReadDone);
    for( size_t i = 0; i < params.vOut.size(); i++ )
        params.vOut[i].mHandle->updateState(
                success ? BufferHandle::STATE_WRITE_OK : BufferHandle::STATE_WRITE_FAIL
                );
    return OK;
}
