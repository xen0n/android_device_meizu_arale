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
#define LOG_TAG "MtkCam/P1Node"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;

#include <semaphore.h>
#include <vector>
#include <list>
using namespace std;
//
#include <utils/Mutex.h>
#include <utils/Condition.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
using namespace NSCam::NSIoPipe::NSCamIOPipe;
#include <core/iopipe/CamIO/PortMap.h>
using namespace NSCam::NSIoPipe;
//
#include <mtkcam/imageio/ispio_utility.h>
//
#include <aee.h>
//
#include "./inc/IspSyncControlHw.h"
#include <mtkcam/camnode/pass1node.h>
//
#include <pthread.h>
#include <sys/prctl.h>

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)

#define MY_LOGV2(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD2(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI2(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW2(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE2(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA2(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF2(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START          MY_LOGD("+")
#define FUNC_END            MY_LOGD("-")

#define ENABLE_CAMIOCONTROL_LOG (1)
#define ENABLE_BUFCONTROL_LOG   (0)

#define AEE_ASSERT(String)    \
    do {                      \
        aee_system_exception( \
            LOG_TAG,          \
            NULL,             \
            DB_OPT_DEFAULT,   \
            String);          \
    } while(0)

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "pass1"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS1)
/*******************************************************************************
*   utilities
********************************************************************************/

/*******************************************************************************
*
********************************************************************************/
typedef struct HwPortConfig{
    PortID          mPortID;
    EImageFormat    mFmt;
    MSize           mSize;
    MRect           mCrop;
    MBOOL           mPureRaw;
    MUINT32         mStrideInByte[3];
}HwPortConfig_t;


/*******************************************************************************
*
********************************************************************************/
MVOID
Pass1NodeInitCfg::
dump() const
{
    MY_LOGD2("scen(%u), rbc(%d), bit(%u), fps(%d), rawType(%u)",
             muScenario, muRingBufCnt, muBitDepth, muFrameRate, muRawType );
}

// ring-buffer control
enum
{
    Status_Init      = 0x0,
    Status_GoingStop = 0x1,
    Status_Stopped   = 0x2, //all buf are moved to offline
    Status_Running   = 0x4,
};

typedef enum _BUF_OP_
{
    BUF_OP_ERROR      = -1,
    BUF_OP_NONE       = 0,
    BUF_OP_ENQUE      = 1,
    BUF_OP_REPLACE    = 2,
} BUF_OP;

// Same setting as ispio
typedef enum _FULL_RAW_TYPE_
{
    FULL_PROCESS_RAW      = 0,
    FULL_PURE_RAW         = 1,
} FULL_RAW_TYPE;
/*******************************************************************************
 *
 ********************************************************************************/
class Pass1NodeImpl : public Pass1Node
{
    private:

        class ringbuffer
        {
            typedef struct _buffer
            {
                MUINT32       muIndex;
                MUINT32       muStatus;
                IImageBuffer* mBuffer;
                _buffer*      mpNext;
            } buffer_t;

            typedef struct _st_buf
            {
                IImageBuffer* mBuffer;
                MUINT32       muStatus;
            } st_buffer_t;

            enum
            {
                STATUS_EMPTY             = 0xFF,
                STATUS_PENDING           = 0,
                STATUS_HW                = 1,
                STATUS_DEQUE             = 2,
                STATUS_FAKEDEQUE         = 3, // dequed by two port
                // to maintain registered buffer
                STATUS_RELEASE           = 4,
            };

            public:
                ringbuffer(MUINT32 nodetype, PortID portId, MUINT32 fakeNodeType)
                    : mDataType(nodetype)
                    , mPortID(portId)
                    , muFakeDataType(fakeNodeType)
                    , muCount(0)
                    , muStatus(Status_Init)
                    , mpHead(NULL)
                    , mpHwDeque(NULL)
                    , mpHwEnque(NULL)
                    , mpBufHdl(NULL)
                    , mbIsReg(MFALSE)
                {}

                ~ringbuffer() {}

                MBOOL         waitForDeque();
                MBOOL         start(); //close ring/update status
                MBOOL         stop();
                MBOOL         addRingBuffer(MBOOL last);
                MBOOL         addOfflineBuf();
                MBOOL         queryInitBuffers(vector<BufInfo> * pvBufinfo) const;
                BUF_OP        getBufOp(
                                IImageBuffer* pImgBuf,
                                vector<BufInfo> * pvBufinfo,
                                MBOOL* doDropInfo = NULL
                                );
                MBOOL         updateDeque(IImageBuffer* pImgBuf);
                MBOOL         returnBuffers(); //clean up
                MBOOL         isAllBufferReturned() const;
                MVOID         dump() const; //debug

                MVOID         setBufHandler(ICamBufHandler* pBufHdl);
                PortID        getPortID() { return mPortID; };

            private: // utility

                MBOOL         transitBufStatus(MUINT32& status, bool enque);
                MINT32        searchOffline(IImageBuffer* const pImgBuf);

            private:
                mutable Mutex         mBufferLock;
                mutable Condition     mCondHwBuf;
                MUINT32 const         mDataType;
                PortID const          mPortID;
                MUINT32 const         muFakeDataType;
                MUINT32               muCount;
                MUINT32               muStatus;
                buffer_t*             mpHead;
                buffer_t*             mpHwDeque;
                buffer_t*             mpHwEnque;
                vector<st_buffer_t>   mvOfflineBuf;

                ICamBufHandler*       mpBufHdl;
                MBOOL                 mbIsReg;
        };

    public: // ctor & dtor
        Pass1NodeImpl(Pass1NodeInitCfg const initcfg);
        ~Pass1NodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations

        MBOOL    getHwPortConfig(list<HwPortConfig_t>* plPortCfg);
        MBOOL    allocBuffers(list<HwPortConfig_t> &  plPortCfg);
        MBOOL    startHw(list<HwPortConfig_t> & plPortCfg);
        MBOOL    stopHw();
        MBOOL    processBufOp(BUF_OP const op, QBufInfo const & halCamIOQBuf);
        MBOOL    dequeLoop();
        MBOOL    dropFrame(QBufInfo const & qBufInfo);

        MUINT32  getRingBufCnt() const { return mInitCfg.muRingBufCnt; }

    private:

        // sensor related
        //     note: raw sensor's format depends on pass1 bit depth
        //           should querySensorInfo before getOutputFmt.
        MBOOL           querySensorInfo();
        MBOOL           getSensorSize(MSize* pSize) const;
        MUINT32         getSensorDelay() const;
        MBOOL           getOutputFmt(PortID port, EImageFormat* pFmt) const;
        MBOOL           getSensorPixelMode(MUINT32* pPixelMode) const;

        // hw-sw mapping
        MBOOL           setupPort(MBOOL& cfgImgo, MBOOL& cfgRrzo, MBOOL& fakeResized);
        PortID          mapToPortID(MUINT32 const nodeDataType) const;
        MUINT32         mapToNodeDataType(PortID const portId, MBOOL bIsPureRaw = MFALSE) const;
        MUINT32         mapToFakeDataType(PortID const portId) const;

        // ************ buffer control ************ //
        MVOID           startBufControl();
        MVOID           stopBufControl();
        MBOOL           keepLooping() const;
        MBOOL           enqueBuffer(MUINT32 data = 0, IImageBuffer* pImgBuf = NULL);
        MBOOL           checkDeque(vector<BufInfo> const & QbufInfo);
        ringbuffer*     getRingBuffer(PortID const portID) const;
        //
        MBOOL           configFrame(MUINT32 const magicNum);
        // **************************************** //

        static void*    doThreadAllocBuf(void* arg);

    private:
        //     init config
        Pass1NodeInitCfg const      mInitCfg;
        //     counter
        MUINT32                     muFrameCnt;
        MUINT32                     muSensorDelay;

        //     infos
        SensorStaticInfo            mSensorInfo;
        //
        //     CamIO
        IHalCamIO*                  mpCamIO;
        Mutex                       mlLockEnqueStop; //to protect enqeu/stop operation
        sem_t                       mSemStopped;
        MBOOL                       mbCfgImgo;
        MBOOL                       mbCfgRrzo;
        MUINT32                     mHwToSw[3]; //map imgo(process), rrzo, imgo(pure) to sw
        PortID                      mSwToHw[2]; //map full, resize to hw
        MUINT32                     mHwToFake[2]; //map imgo, rrzo to fake
        MINT32                      mIspEnquePeriod;
        MUINT32                     mLastDequeMagicNumber;
        MINT64                      mLastDequeTimestamp;
        //
        // ************ buffer control ************ //
        ringbuffer*                 mpRingImgo;
        ringbuffer*                 mpRingRrzo;
        // **************************************** //
        IspSyncControlHw*           mpIspSyncCtrlHw;

        // *** allocate buffer in another thread *** //
        bool                        mRet;
        pthread_t                   mThreadHandle;

        struct threadData
        {
            Pass1NodeImpl*          pSelf;
            list<HwPortConfig_t>*   plPortCfg;
        };
};


/*******************************************************************************
 *
 ********************************************************************************/
Pass1Node*
Pass1Node::
createInstance(Pass1NodeInitCfg const initcfg)
{
    return new Pass1NodeImpl(initcfg);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
Pass1Node::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
Pass1Node::
    Pass1Node()
: ICamThreadNode( MODULE_NAME, ContinuousTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
Pass1Node::
~Pass1Node()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
Pass1NodeImpl::
    Pass1NodeImpl(Pass1NodeInitCfg const initcfg)
    : Pass1Node()
    , mInitCfg(initcfg)
    , mpCamIO(NULL)
    , mbCfgImgo(MFALSE)
    , mbCfgRrzo(MFALSE)
    , mIspEnquePeriod(1)
    , mLastDequeMagicNumber(0)
    , mLastDequeTimestamp(0)
    , mpRingImgo(NULL)
    , mpRingRrzo(NULL)
    , mpIspSyncCtrlHw(NULL)
{
    //DATA
    addDataSupport( ENDPOINT_DST, PASS1_FULLRAW );
    addDataSupport( ENDPOINT_DST, PASS1_RESIZEDRAW );
    addDataSupport( ENDPOINT_DST, PASS1_PURERAW );
    //NODECAT_BASIC_NOTIFY
    addNotifySupport(
        PASS1_START_ISP    |
        PASS1_STOP_ISP     |
        PASS1_SOF          |
        PASS1_EOF          |
        PASS1_CONFIG_FRAME );
}


/*******************************************************************************
 *
 ********************************************************************************/
Pass1NodeImpl::
~Pass1NodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    MSize sensorSize;
    //
    if( sem_init(&mSemStopped, 0, 1) != 0 )
    {
        MY_LOGE("sem init fail");
        goto lbExit;
    }
    //
    // query sensor info & build SW/HW port mapping
    if( !querySensorInfo() )
    {
        MY_LOGE("init failed");
        goto lbExit;
    }
    //
    mpIspSyncCtrlHw = IspSyncControlHw::createInstance(getSensorIdx());
    if(!mpIspSyncCtrlHw)
    {
        MY_LOGE("create IspSyncControlHw failed");
        goto lbExit;
    }
    //
    if( !getSensorSize( &sensorSize ) )
        goto lbExit;
    //
    NSImageio::NSIspio::querySuggBurstQNum(
                            mInitCfg.muFrameRate,
                            mIspEnquePeriod);
    //
    mpIspSyncCtrlHw->setIspEnquePeriod(mIspEnquePeriod);
    mpIspSyncCtrlHw->setSensorInfo(
            mInitCfg.muScenario,
            sensorSize.w,
            sensorSize.h,
            mSensorInfo.sensorType);
    //
    mpCamIO = (IHalCamIO*)INormalPipe::createInstance(getSensorIdx(), getName(), mIspEnquePeriod);
    if( !mpCamIO )
    {
        MY_LOGE("create NormalPipe failed");
        goto lbExit;
    }
    //
    if( !mpCamIO->init() )
    {
        MY_LOGE("camio init failed");
        goto lbExit;
    }
    //
    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;

    if(mpIspSyncCtrlHw)
    {
        mpIspSyncCtrlHw->destroyInstance();
        mpIspSyncCtrlHw = NULL;
    }

    if( !mpCamIO->uninit() )
    {
        MY_LOGE("uninit failed");
        ret = MFALSE;
    }

    mpCamIO->destroyInstance(getName());
    mpCamIO = NULL;

    if( sem_destroy(&mSemStopped) < 0 )
    {
        MY_LOGE("sem destroy failed");
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;

    mInitCfg.dump();

    mpIspSyncCtrlHw->clearTable();

    list<HwPortConfig_t> lHwPortCfg;
    if( !getHwPortConfig(&lHwPortCfg) )
    {
        MY_LOGE("getHwPortConfig failed");
        goto lbExit;
    }

#if 0
    CAM_TRACE_BEGIN("alloc");
    if( !allocBuffers(lHwPortCfg) )
    {
        MY_LOGE("alloc buffers failed");
        goto lbExit;
    }
    CAM_TRACE_END();
#endif

    if( !startHw(lHwPortCfg) )
    {
        MY_LOGE("startHw failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret;

    stopBufControl();     //ask to stop

    if( sem_wait(&mSemStopped) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    syncWithThread();

    FUNC_END;
    return sem_post(&mSemStopped) == 0;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    switch( msg )
    {
        case CONTROL_STOP_PASS1:
            stopBufControl();
            break;

        case LATE_STOP:
            if( mpRingImgo ) {
                delete mpRingImgo;
                mpRingImgo = NULL;
            }

            if( mpRingRrzo ) {
                delete mpRingRrzo;
                mpRingRrzo = NULL;
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
Pass1NodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    //should not happen
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;

    IImageBuffer*  pBuf = (IImageBuffer *)buf;
    MY_LOGV("data(%d), buf(0x%x), va/pa(0x%x/0x%x)",
            data, pBuf, pBuf->getBufVA(0), pBuf->getBufPA(0));

    if( !enqueBuffer(data,  pBuf) )
    {
        MY_LOGE("enque buffer failed");
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
Pass1NodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
    char dumppath[256];
    sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt )               \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%d_%dx%d_%d_%d.%s",            \
                dumppath,                                      \
                #type,                                         \
                getSensorIdx(),                                \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                buffer->getBufStridesInBytes(0),               \
                muFrameCnt,                                    \
                fileExt                                        \
               );                                              \
        buffer->saveToFile(filename);                          \
    }while(0)

    if(!makePath(dumppath,0660))
    {
        MY_LOGE("makePath [%s] fail",dumppath);
        return;
    }

    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_PURERAW:
            DUMP_IImageBuffer( PASS1_PURERAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        default:
            MY_LOGE("not handle this yet data(%d)", data);
            break;
    }
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
threadLoopUpdate()
{
    MBOOL ret = MTRUE;
    //
    if( keepLooping() )
    {
        // deque
        ret = dequeLoop();

        // try to keep ring buffer running
        enqueBuffer();
    }
    else
    {
        ret = stopHw();
        syncWithThread();
    }
    //
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
getSensorSize(MSize* pSize) const
{
    MBOOL ret = MTRUE;
    // sensor size
#define scenario_case(scenario, KEY, pSize)       \
        case scenario:                            \
            (pSize)->w = mSensorInfo.KEY##Width;  \
            (pSize)->h = mSensorInfo.KEY##Height; \
            break;
    switch(mInitCfg.muScenario)
    {
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, pSize );
        scenario_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, pSize );
        scenario_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, pSize );
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM1, SensorCustom1, pSize);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM2, SensorCustom2, pSize);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM3, SensorCustom3, pSize);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM4, SensorCustom4, pSize);
        scenario_case( SENSOR_SCENARIO_ID_CUSTOM5, SensorCustom5, pSize);
        default:
            MY_LOGE("not support sensor scenario(0x%x)", mInitCfg.muScenario);
            ret = MFALSE;
            break;
    }
#undef scenario_case

lbExit:
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MUINT32
Pass1NodeImpl::
getSensorDelay() const
{
    MUINT32 delay = 0;
#define scenario_delay_case(scenario, KEY, _delay) \
        case scenario:                             \
            _delay = mSensorInfo.KEY##DelayFrame;  \
            break;

    switch(mInitCfg.muScenario)
    {
        scenario_delay_case( SENSOR_SCENARIO_ID_NORMAL_PREVIEW, preview, delay );
        scenario_delay_case( SENSOR_SCENARIO_ID_NORMAL_CAPTURE, capture, delay );
        scenario_delay_case( SENSOR_SCENARIO_ID_NORMAL_VIDEO, video, delay );
        scenario_delay_case( SENSOR_SCENARIO_ID_SLIM_VIDEO1, video1, delay );
        scenario_delay_case( SENSOR_SCENARIO_ID_SLIM_VIDEO2, video2, delay );
        scenario_delay_case( SENSOR_SCENARIO_ID_CUSTOM1, Custom1, delay);
        scenario_delay_case( SENSOR_SCENARIO_ID_CUSTOM2, Custom2, delay);
        scenario_delay_case( SENSOR_SCENARIO_ID_CUSTOM3, Custom3, delay);
        scenario_delay_case( SENSOR_SCENARIO_ID_CUSTOM4, Custom4, delay);
        scenario_delay_case( SENSOR_SCENARIO_ID_CUSTOM5, Custom5, delay);
        default:
            MY_LOGE("not support sensor scenario(0x%x)", mInitCfg.muScenario);
            break;
    }
#undef scenario_delay_case

    return delay;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
getHwPortConfig(list<HwPortConfig_t>* plPortCfg)
{
    CAM_TRACE_CALL();

    MBOOL ret = MFALSE;
    MBOOL fakeResized = false;
    MSize sensorSize;
    MUINT32 pixelMode = 0;
    //
    if( !getSensorSize(&sensorSize) )
        goto lbExit;
    //
    if( !setupPort(mbCfgImgo, mbCfgRrzo, fakeResized ) )
        goto lbExit;
    //
    if( !getSensorPixelMode(&pixelMode) )
        goto lbExit;

    //TODO: should query from iopipe
    if( mbCfgImgo )
    {
        EImageFormat fmt;
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;

        if( !getOutputFmt(PORT_IMGO, &fmt))
            goto lbExit;

        NSImageio::NSIspio::ISP_QuerySize(
                                NSImageio::NSIspio::EPortIndex_IMGO,
                                NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                sensorSize.w,
                                queryRst,
                                pixelMode == 0 ?  NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
                                                  NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
                                );

        HwPortConfig_t full = {
            mapToPortID(PASS1_FULLRAW),
            fmt,
            sensorSize,
            MRect( MPoint(0, 0), sensorSize ),
            (mInitCfg.muRawType != SENSOR_PROCESS_RAW ), //if raw type != 1 -> pure-raw
            { queryRst.stride_byte, 0, 0 }
        };

        plPortCfg->push_back(full);

        mpRingImgo = new ringbuffer(
                PASS1_FULLRAW,
                mapToPortID(PASS1_FULLRAW),
                fakeResized ? PASS1_RESIZEDRAW : 0
                );
    }

    if( mbCfgRrzo )
    {
        // rrz just supports raw sensor
        EImageFormat fmt;
        MRect crop;
        MSize size;
        NSImageio::NSIspio::ISP_QUERY_RST queryRst;

        if( !getOutputFmt(PORT_RRZO, &fmt) )
            goto lbExit;

        //
        #define min(x,y) ((x)>(y) ? (y) : (x))
        #define max(x,y) ((x)<(y) ? (y) : (x))
        //
        mpIspSyncCtrlHw->getInitialRrzoSize(
                            fmt,
                            pixelMode,
                            crop,
                            size);
        NSImageio::NSIspio::ISP_QuerySize(
                                NSImageio::NSIspio::EPortIndex_RRZO,
                                NSImageio::NSIspio::ISP_QUERY_X_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_PIX|
                                NSImageio::NSIspio::ISP_QUERY_STRIDE_BYTE,
                                fmt,
                                size.w,
                                queryRst,
                                pixelMode == 0 ?  NSImageio::NSIspio::ISP_QUERY_1_PIX_MODE :
                                                  NSImageio::NSIspio::ISP_QUERY_2_PIX_MODE
                                );
        HwPortConfig_t resized = {
            mapToPortID(PASS1_RESIZEDRAW),
            fmt,
            size,
            crop,
            MFALSE,
            { queryRst.stride_byte, 0, 0 }
        };

        plPortCfg->push_back(resized);

        //create ringbuffer
        mpRingRrzo = new ringbuffer( PASS1_RESIZEDRAW, mapToPortID(PASS1_RESIZEDRAW), 0 );
    }

    ret = MTRUE;

lbExit:
    if( !ret ) {
        MY_LOGE("getHwPortConfig error!");
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
allocBuffers(list<HwPortConfig_t> & lPortCfg)
{
    CAM_TRACE_CALL();

    MBOOL ret = MFALSE;

    list< HwPortConfig_t >::const_iterator pConfig = lPortCfg.begin();
    while( pConfig != lPortCfg.end() )
    {
        MUINT32 nodedatatype = mapToNodeDataType(pConfig->mPortID);
        ICamBufHandler* pBufHdl = getBufferHandler(nodedatatype);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", nodedatatype);
            goto lbExit;
        }

        //alloc ringbuffer
        AllocInfo allocinfo(
                    pConfig->mSize.w,
                    pConfig->mSize.h,
                    pConfig->mFmt,
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK,
                    pConfig->mStrideInByte);

        pBufHdl->updateFlag( nodedatatype, FLAG_BUFFER_ONESHOT );

        for(MUINT32 i = 0; i < getRingBufCnt() ; i++ )
        {
            if( !pBufHdl->requestBuffer(nodedatatype, allocinfo) )
            {
                MY_LOGE("request buffer failed: data %d", nodedatatype);
                goto lbExit;
            }
        }

        ringbuffer* pRing = getRingBuffer(pConfig->mPortID);
        pRing->setBufHandler(pBufHdl);
        //
        for(MUINT32 i = 0; i < getRingBufCnt() ; i++ )
        {
            if( !pRing->addRingBuffer(getRingBufCnt() - i == 1) )
            {
                MY_LOGE("add ring buffer failed");
                goto lbExit;
            }
        }

        MUINT32 rep_count = mpIspSyncCtrlHw->getPass1ReplaceCnt(nodedatatype == PASS1_FULLRAW);
        // alloc replace buffer
        for(MUINT32 i = 0; i < rep_count ; i++ )
        {
            if( !pBufHdl->requestBuffer(nodedatatype, allocinfo) )
            {
                MY_LOGE("request buffer failed");
                goto lbExit;
            }
        }
        //
        for(MUINT32 i = 0; i < rep_count ; i++ )
        {
            if( !pRing->addOfflineBuf() )
            {
                MY_LOGE("add offline buffer failed");
                goto lbExit;
            }
        }
#if 0
        pRing->dump();
#endif
        pConfig++;
    }
    ret = MTRUE;
lbExit:
    if( !ret ) {
        MY_LOGE("allocBuffers failed");
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
startHw(list<HwPortConfig_t> & plPortCfg)
{
    CAM_TRACE_CALL();

    MBOOL ret = MFALSE;
    MUINT32 newMagicNum;
    MBOOL threadrunning = MFALSE;
    //
    //  create a thread to alloc buffers
    threadData th_data = {this, &plPortCfg};
    if( pthread_create(&mThreadHandle, NULL, doThreadAllocBuf, &th_data) != 0 )
    {
        MY_LOGE("pthread create failed");
        goto lbExit;
    }
    threadrunning = MTRUE;
    //
    if( !mpIspSyncCtrlHw->lockHw(IspSyncControlHw::HW_PASS1) )
    {
        MY_LOGE("isp sync lock pass1 failed");
        goto lbExit;
    }
    //
    //
    { //configPipe
        MSize sensorSize;
        vector<IHalSensor::ConfigParam> vSensorCfg;

        if( !getSensorSize( &sensorSize ) ) {
            MY_LOGE("get sensor size failed");
            return MFALSE;
        }
        //prepare sensor config
        IHalSensor::ConfigParam sensorCfg =
        {
            (MUINT)getSensorIdx(),          /* index            */
            sensorSize,                     /* crop             */
            mInitCfg.muScenario,            /* scenarioId       */
            0,                              /* isBypassScenario */
            1,                              /* isContinuous     */
            mpIspSyncCtrlHw->getHdrState(), /* iHDROn           */
            mInitCfg.muFrameRate,           /* framerate        */
            0,                              /* two pixel on     */
            0,                              /* debugmode        */
        };
        //
        vSensorCfg.push_back(sensorCfg);
        MY_LOGD("sensor %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d, twopxl %d",
                sensorCfg.crop.w,
                sensorCfg.crop.h,
                sensorCfg.scenarioId,
                sensorCfg.isBypassScenario,
                sensorCfg.isContinuous,
                sensorCfg.HDRMode,
                sensorCfg.framerate,
                sensorCfg.twopixelOn);
        //
        vector<portInfo> vPortInfo;
        list<HwPortConfig_t>::const_iterator pPortCfg;
        for( pPortCfg = plPortCfg.begin(); pPortCfg != plPortCfg.end(); pPortCfg++ )
        {
            //get one IImageBuffer for stride
            ringbuffer* pRing = getRingBuffer( pPortCfg->mPortID );
            //
            portInfo OutPort(
                    pPortCfg->mPortID,
                    pPortCfg->mFmt,
                    pPortCfg->mSize, //dst size
                    pPortCfg->mCrop, //crop
                    pPortCfg->mStrideInByte[0],
                    pPortCfg->mStrideInByte[1],
                    pPortCfg->mStrideInByte[2],
                    pPortCfg->mPureRaw, // pureraw
                    MTRUE               //packed
                    );
            vPortInfo.push_back(OutPort);
            //
            MY_LOGD(" config portID(0x%x), fmt(%u), size(%dx%d), crop(%u,%u,%u,%u)",
                    OutPort.mPortID, OutPort.mFmt, OutPort.mDstSize.w, OutPort.mDstSize.h,
                    OutPort.mCropRect.p.x, OutPort.mCropRect.p.y,
                    OutPort.mCropRect.s.w, OutPort.mCropRect.s.h);
            MY_LOGD("stride(%u,%u,%u), pureRaw(%u), pack(%d)",
                    OutPort.mStride[0], OutPort.mStride[1], OutPort.mStride[2],
                    OutPort.mPureRaw, OutPort.mPureRawPak);
        }
        //
        QInitParam halCamIOinitParam(
                (mInitCfg.muRawType == SENSOR_TEST_PATTERN_RAW), // 2: sensor uses pattern
                mInitCfg.muBitDepth,
                vSensorCfg,
                vPortInfo);
        //
        CAM_TRACE_BEGIN("configP1");
        if( !mpCamIO->configPipe(halCamIOinitParam) ) {
            MY_LOGE("configPipe failed");
            goto lbExit;
        }
        CAM_TRACE_END();
    }
    //
    //config frame
    newMagicNum = mpIspSyncCtrlHw->getMagicNum(MTRUE);
    if( !configFrame(newMagicNum) ) {
        MY_LOGE("configFrame failed");
        goto lbExit;
    }
    //
    handleNotify(PASS1_START_ISP, newMagicNum, 0);
    //
    if( threadrunning )
    {
        MY_LOGD("wait alloc done +");
        int s = pthread_join(mThreadHandle, NULL);
        MY_LOGD("wait alloc done -");
        threadrunning = MFALSE;

        if( s != 0 )
        {
            MY_LOGE("pthread join error: %d", s);
            goto lbExit;
        }

        if( !mRet )
        {
            MY_LOGE("allocbuffer in thread failed");
            goto lbExit;
        }
    }
    //
    {
        //enque
        QBufInfo halCamIOQBuf;
        if( mpRingImgo )
            mpRingImgo->queryInitBuffers( &(halCamIOQBuf.mvOut) );

        if( mpRingRrzo )
            mpRingRrzo->queryInitBuffers( &(halCamIOQBuf.mvOut) );
#if 1
        {
            //dump for debug
            vector<BufInfo>::const_iterator dump_iter = halCamIOQBuf.mvOut.begin();
            while( dump_iter != halCamIOQBuf.mvOut.end() )
            {
                MY_LOGD("portID(0x%x), buf(0x%x), va/pa(0x%x/0x%x)",
                        dump_iter->mPortID,
                        dump_iter->mBuffer,
                        dump_iter->mBuffer->getBufVA(0),
                        dump_iter->mBuffer->getBufPA(0) );
                dump_iter++;
            }
        }
#endif
        //CAM_TRACE_BEGIN("enqueP1");
        if( !mpCamIO->enque(halCamIOQBuf) ) {
            MY_LOGE("enque failed");
            goto lbExit;
        }
        //CAM_TRACE_END();
    }
    //
    mLastDequeMagicNumber = 0;
    mLastDequeTimestamp = 0;
    //
    if( sem_wait( &mSemStopped ) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        goto lbExit;
    }
    //
    CAM_TRACE_BEGIN("startP1");
    if( !mpCamIO->start() ) {
        MY_LOGE("start failed");
        goto lbExit;
    }
    CAM_TRACE_END();
    //
    startBufControl();
    //
    handleNotify(PASS1_SOF, 0, 0);
    //
    muFrameCnt = 0;
    muSensorDelay = getSensorDelay();
    //
    ret = MTRUE;
lbExit:
    if( !ret ) {
        MY_LOGE("startHw failed");

        if( threadrunning ) {
            int s = pthread_join(mThreadHandle, NULL);
        }

        stopBufControl();

        // return all buffers
        if( mpRingImgo ) {
            mpRingImgo->returnBuffers();
            delete mpRingImgo;
            mpRingImgo = NULL;
        }

        if( mpRingRrzo ) {
            mpRingRrzo->returnBuffers();
            delete mpRingRrzo;
            mpRingRrzo = NULL;
        }
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
stopHw()
{
    FUNC_START;
    CAM_TRACE_CALL();
    MBOOL ret;
    //
    handleNotify(PASS1_STOP_ISP, 0, 0);
    //
    CAM_TRACE_BEGIN("p1 stop");
    ret = mpCamIO->stop();
    CAM_TRACE_END();
    //
    if( ret && !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS1) )
    {
        MY_LOGE("isp sync unlock pass1 failed");
        ret = MFALSE;
    }
    //
    if( mpRingImgo ) {
        mpRingImgo->returnBuffers();
    }

    if( mpRingRrzo ) {
        mpRingRrzo->returnBuffers();
    }
    //
    if( !ret )
    {
        MY_LOGE("stop failed");
    }
    sem_post(&mSemStopped);
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
processBufOp(BUF_OP const op, QBufInfo const & halCamIOQBuf)
{
    MBOOL ret = MTRUE;
    switch( op )
    {
        case BUF_OP_NONE:
            break;
        //
        case BUF_OP_ENQUE:
            {
                //CAM_TRACE_NAME("enqueP1");
                ret = mpCamIO->enque(halCamIOQBuf);
#if ENABLE_CAMIOCONTROL_LOG
                vector<BufInfo>::const_iterator dump_iter = halCamIOQBuf.mvOut.begin();
                while( dump_iter != halCamIOQBuf.mvOut.end() )
                {
                    MY_LOGD("enq port 0x%x, buf 0x%x, va 0x%x",
                            dump_iter->mPortID, dump_iter->mBuffer,
                            dump_iter->mBuffer->getBufVA(0));
                    dump_iter++;
                }
#endif
            }
            break;
        //
        case BUF_OP_REPLACE:
            {
                //CAM_TRACE_NAME("replaceP1");
                ret = mpCamIO->replace(halCamIOQBuf.mvOut[0], halCamIOQBuf.mvOut[1]);
#if ENABLE_CAMIOCONTROL_LOG
                MY_LOGD("replace port 0x%x old/new 0x%x/0x%x va 0x%x/0x%x",
                        halCamIOQBuf.mvOut[0].mPortID,
                        halCamIOQBuf.mvOut[0].mBuffer,
                        halCamIOQBuf.mvOut[1].mBuffer,
                        halCamIOQBuf.mvOut[0].mBuffer->getBufVA(0),
                        halCamIOQBuf.mvOut[1].mBuffer->getBufVA(0)
                        );
#endif
            }
            break;
        //
        case BUF_OP_ERROR:
        default:
            MY_LOGE("buf op error");
            ret = MFALSE;
            break;
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
dequeLoop()
{
    //FUNC_START;
    MBOOL ret = MTRUE;

    //prepare to deque
    QBufInfo dequeBufInfo;
    dequeBufInfo.mvOut.reserve(2);
    if( mpRingImgo ) {
        BufInfo OutBuf(mpRingImgo->getPortID(), 0);
        dequeBufInfo.mvOut.push_back(OutBuf);
    }
    if( mpRingRrzo ) {
        BufInfo OutBuf(mpRingRrzo->getPortID(), 0);
        dequeBufInfo.mvOut.push_back(OutBuf);
    }

    for(MUINT32 i=0; i<2; i++)
    {
        MY_LOGD("frame %d: deque+", muFrameCnt);
        ret = mpCamIO->deque(dequeBufInfo);
        MY_LOGD("frame %d: deque-,%d", muFrameCnt, ret);
        //
        if(ret)
        {
            break;
        }
        else
        {
            MY_LOGW("CamIO reset cnt(%d)", i);
            mpCamIO->Reset();
        }
    }
    //
    CAM_TRACE_NAME("dequeP1");
    //
    if(!ret)
    {
        MY_LOGE("deque fail");
        aee_system_exception(
            LOG_TAG,
            NULL,
            DB_OPT_DEFAULT,
            "\nCRDISPATCH_KEY:MtkCam/P1Node:ISP pass1 deque fail");
        return MFALSE;
    }
    //
    muFrameCnt++;
    MUINT32 dequeMagicNum = dequeBufInfo.mvOut.at(0).mMetaData.mMagicNum_hal;
    MUINT32 newMagicNum = mpIspSyncCtrlHw->getMagicNum(MFALSE);
    //
    if(dequeMagicNum & MAGIC_NUM_UNKNOWN_MASK)
    {
        MY_LOGD("unknown frame # 0x%X",dequeMagicNum);
        dequeMagicNum = MAGIC_NUM_INVALID;
    }
    //
    handleNotify(
            PASS1_EOF,
            newMagicNum,
            muSensorDelay == 0 ? dequeMagicNum : MAGIC_NUM_INVALID);
    //
    configFrame(newMagicNum);
    //
    if(!checkDeque(dequeBufInfo.mvOut) )
    {
        MY_LOGE("checkDeque fail");
        AEE_ASSERT("ISP pass1 checkDeque fail");
        dropFrame(dequeBufInfo);
        return MFALSE;
    }
    //
    if( muSensorDelay ||
        dequeMagicNum == MAGIC_NUM_INVALID)
    {
        MY_LOGD("drop frame %d", muSensorDelay);
        dropFrame(dequeBufInfo);
        if(muSensorDelay > 0)
        {
            muSensorDelay--;
        }
        return MTRUE;
    }

    vector<BufInfo>::const_iterator iter;
    for( iter = dequeBufInfo.mvOut.begin(); iter != dequeBufInfo.mvOut.end(); iter++ )
    {
        if(iter->mMetaData.mMagicNum_hal != iter->mMetaData.mMagicNum_tuning)
        {
            MY_LOGE("MagicNum hal(%d) != tuning(%d)",
                    iter->mMetaData.mMagicNum_hal,
                    iter->mMetaData.mMagicNum_tuning);
        }
        //
        mpIspSyncCtrlHw->addPass1Info(
                iter->mMetaData.mMagicNum_hal,
                iter->mBuffer,
                iter->mMetaData,
                iter->mPortID == PORT_RRZO);
        // if sensor raw type is pure raw, and is in PADF mode, send buffer to DefaultCtrl node via different channel
        MBOOL bIsDynamicPureRaw = MFALSE;
        if (iter->mPortID == PORT_IMGO)
        {
            //MY_LOGD("PORT_IMGO, sensor raw type = %d", iter->mMetaData.mRawType);
            // should use this after raw metadata provide pure raw
            if (mInitCfg.muRawType == SENSOR_DYNAMIC_PURE_RAW)
            {
                MY_LOGD("iter->mMetaData.mMagicNum_hal=%d, iter->mMetaData.mRawType=%d", iter->mMetaData.mMagicNum_hal, iter->mMetaData.mRawType);
            }
            if (mInitCfg.muRawType == SENSOR_DYNAMIC_PURE_RAW && iter->mMetaData.mRawType == FULL_PURE_RAW)
            {
                bIsDynamicPureRaw = MTRUE;
            }
        }
        ret = ret && handlePostBuffer( mapToNodeDataType(iter->mPortID, bIsDynamicPureRaw), (MUINTPTR)iter->mBuffer, 0);
        //
        MUINT32 fakedata = mapToFakeDataType(iter->mPortID);
        if( fakedata != 0  ) {
            ret = ret && handlePostBuffer( fakedata, (MUINTPTR)iter->mBuffer, 0);
        }
    }

    //FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
dropFrame(QBufInfo const & qBufInfo)
{
    vector<BufInfo>::const_iterator iter;
    for( iter = qBufInfo.mvOut.begin(); iter != qBufInfo.mvOut.end(); iter++ )
    {
        MUINT32 nodeType = mapToNodeDataType(iter->mPortID);
        MUINT32 fakedata = mapToFakeDataType(iter->mPortID);
        //
        onReturnBuffer( nodeType, (MUINTPTR)iter->mBuffer, 0);
        if( fakedata != 0  ) {
            onReturnBuffer( fakedata, (MUINTPTR)iter->mBuffer, 0);
        }
    }
    return MTRUE;
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
querySensorInfo()
{
    MBOOL ret = MFALSE;

    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    MINT32 const sensorNum = pHalSensorList->queryNumberOfSensors();

    if( getSensorIdx() >= sensorNum )
    {
        MY_LOGE("wrong sensor idx(0x%x), sensorNum(%d)", getSensorIdx(), sensorNum);
        goto lbExit;
    }

    pHalSensorList->querySensorStaticInfo(
            pHalSensorList->querySensorDevIdx(getSensorIdx()),
            &mSensorInfo);
    //
    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
getOutputFmt(PortID port, EImageFormat* pFmt) const
{
    MBOOL ret = MFALSE;
    // sensor fmt
#define case_Format( order_bit, mappedfmt, pFmt) \
        case order_bit:                          \
            (*(pFmt)) = mappedfmt;               \
        break;

    if( mSensorInfo.sensorType == SENSOR_TYPE_YUV )
    {
        switch( mSensorInfo.sensorFormatOrder )
        {
            case_Format( SENSOR_FORMAT_ORDER_UYVY, eImgFmt_UYVY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_VYUY, eImgFmt_VYUY, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YUYV, eImgFmt_YUY2, pFmt);
            case_Format( SENSOR_FORMAT_ORDER_YVYU, eImgFmt_YVYU, pFmt);
            default:
            MY_LOGE("formatOrder not supported, 0x%x", mSensorInfo.sensorFormatOrder);
            goto lbExit;
            break;
        }
        MY_LOGD("sensortype:(0x%x), fmt(0x%x)", mSensorInfo.sensorType, *pFmt);
    }
    else if( mSensorInfo.sensorType == SENSOR_TYPE_RAW )
    {
        if(port == PORT_IMGO) //imgo
        {
            switch( mInitCfg.muBitDepth )
            {
                case_Format(  8, eImgFmt_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_BAYER10, pFmt);
                case_Format( 12, eImgFmt_BAYER12, pFmt);
                case_Format( 14, eImgFmt_BAYER14, pFmt);
                default:
                MY_LOGE("bitdepth not supported, 0x%x", mInitCfg.muBitDepth);
                goto lbExit;
                break;
            }
        }
        else // rrzo
        {
            switch( mInitCfg.muBitDepth )
            {
                case_Format(  8, eImgFmt_FG_BAYER8 , pFmt);
                case_Format( 10, eImgFmt_FG_BAYER10, pFmt);
                case_Format( 12, eImgFmt_FG_BAYER12, pFmt);
                case_Format( 14, eImgFmt_FG_BAYER14, pFmt);
                default:
                MY_LOGE("bitdepth not supported, 0x%x", mInitCfg.muBitDepth);
                goto lbExit;
                break;
            }
        }
        MY_LOGD("sensortype: 0x%x, port(0x%x), fmt(0x%x), order(%d)",
                mSensorInfo.sensorType, port, *pFmt, mSensorInfo.sensorFormatOrder);
    }
    else
    {
        MY_LOGE("sensorType not supported yet(0x%x)", mSensorInfo.sensorType);
        goto lbExit;
    }
    ret = MTRUE;
#undef case_Format

lbExit:
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
getSensorPixelMode(MUINT32* pPixelMode) const
{
    IHalSensor* pSensorHalObj = NULL;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    if( !pHalSensorList ) {
        MY_LOGE("pHalSensorList == NULL");
        return MFALSE;
    }

    pSensorHalObj = pHalSensorList->createSensor(
            LOG_TAG,
            getSensorIdx());
    //
    if( pSensorHalObj == NULL )
    {
        MY_LOGE("pSensorHalObj is NULL");
        return MFALSE;
    }
    // if frame rate is 0, query frame rate from sensor hal
    if (mInitCfg.muFrameRate == 0)
    {
        MY_LOGD("###Frame rate is 0, query current frame rate from sensor");
        pSensorHalObj->sendCommand(
                                pHalSensorList->querySensorDevIdx(getSensorIdx()),
                                SENSOR_CMD_GET_DEFAULT_FRAME_RATE_BY_SCENARIO,
                                (MUINTPTR)&mInitCfg.muScenario,
                                (MUINTPTR)&mInitCfg.muFrameRate,
                                0);
        // mInitCfg is const, force modify frame rate, not a good way, but need to
        MUINT32 *frameRate = (MUINT32*)&mInitCfg.muFrameRate;
        *frameRate = mInitCfg.muFrameRate/10;
        MY_LOGD("###New Frame rate is %d", mInitCfg.muFrameRate);
    }
    //
    pSensorHalObj->sendCommand(
            pHalSensorList->querySensorDevIdx(getSensorIdx()),
            SENSOR_CMD_GET_SENSOR_PIXELMODE,
            (MUINTPTR)&mInitCfg.muScenario,
            (MUINTPTR)&mInitCfg.muFrameRate,
            (MUINTPTR)pPixelMode);
    MY_LOGD("sensorScenario(%d),sensorFps(%d),pixelMode(%d)",
            mInitCfg.muScenario,
            mInitCfg.muFrameRate,
            *pPixelMode);

    pSensorHalObj->destroyInstance(LOG_TAG);

    if( *pPixelMode != 0 && *pPixelMode != 1 )
    {
        MY_LOGE("Un-supported pixel mode %d", *pPixelMode);
        return MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
setupPort(MBOOL& cfgImgo, MBOOL& cfgRrzo, MBOOL& fakeResized)
{
    cfgImgo = (mSensorInfo.sensorType == SENSOR_TYPE_YUV) || isDataConnected(PASS1_FULLRAW) || isDataConnected(PASS1_PURERAW);
    cfgRrzo = (mSensorInfo.sensorType == SENSOR_TYPE_RAW);
    // ideal
    //cfgRrzo = (mSensorInfo.sensorType == SENSOR_TYPE_RAW) && isDataConnected(PASS1_RESIZEDRAW);
    // if resizeraw is connected byt rrzo is not configured, enable fakeResized
    fakeResized = (mSensorInfo.sensorType == SENSOR_TYPE_YUV) && isDataConnected(PASS1_RESIZEDRAW);

    // setup port mapping
    mHwToSw[0] = cfgImgo ? PASS1_FULLRAW : 0;
    mHwToSw[1] = fakeResized ? 0 : PASS1_RESIZEDRAW;
    mHwToSw[2] = cfgImgo ? PASS1_PURERAW : 0;
    mSwToHw[0] = PORT_IMGO;
    mSwToHw[1] = fakeResized ? PORT_IMGO : PORT_RRZO;
    mHwToFake[0] = (fakeResized ? PASS1_RESIZEDRAW : 0);
    mHwToFake[1] = 0;

    MY_LOGD("imgo %d, rrzo %d, fake %d", cfgImgo, cfgRrzo, fakeResized);
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
PortID
Pass1NodeImpl::
mapToPortID(MUINT32 const nodeDataType) const
{
    switch(nodeDataType)
    {
        case PASS1_FULLRAW:
        case PASS1_PURERAW:
            return mSwToHw[0];
        case PASS1_RESIZEDRAW:
            return mSwToHw[1];
        default:
            break;
    }
    MY_LOGE("wrong dataType %d", nodeDataType);
    return PortID();
}


/*******************************************************************************
 *
 ********************************************************************************/
MUINT32
Pass1NodeImpl::
mapToNodeDataType(PortID const portId, MBOOL bIsDynamicPureRaw) const
{
    if( portId == PORT_IMGO )
    {
        if (bIsDynamicPureRaw)
        {
            return mHwToSw[2];
        }
        else
        {
            return mHwToSw[0];
        }
    }
    else if ( portId == PORT_RRZO )
        return mHwToSw[1];
    MY_LOGE("wrong port 0x%x", portId);
    return 0;
}


/*******************************************************************************
 *
 ********************************************************************************/
MUINT32
Pass1NodeImpl::
mapToFakeDataType(PortID const portId) const
{
    if( portId == PORT_IMGO )
        return mHwToFake[0];
    else if ( portId == PORT_RRZO )
        return mHwToFake[1];
    MY_LOGE("wrong port 0x%x", portId);
    return 0;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
Pass1NodeImpl::
startBufControl()
{
    if( mpRingImgo )
        mpRingImgo->start();

    if( mpRingRrzo )
        mpRingRrzo->start();
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
Pass1NodeImpl::
stopBufControl()
{
    MY_LOGD("stop bufcontrol + ");

    // wait enque done
    Mutex::Autolock _l(mlLockEnqueStop);

    if( mpRingImgo )
        mpRingImgo->stop();

    if( mpRingRrzo )
        mpRingRrzo->stop();

    MY_LOGD("stop bufcontrol - ");
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
keepLooping() const
{
    MBOOL doDeque = MTRUE;

    if( mpRingImgo )
        doDeque = doDeque && mpRingImgo->waitForDeque();

    if( mpRingRrzo )
        doDeque = doDeque && mpRingRrzo->waitForDeque();

    return doDeque;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
enqueBuffer(MUINT32 data, IImageBuffer* pImgBuf)
{
    Mutex::Autolock _l(mlLockEnqueStop);
    MBOOL ret = MTRUE;
    if( pImgBuf ) {
        QBufInfo halCamIOQBuf;
        BUF_OP op;
        MBOOL doDropInfo;

        ringbuffer* pRing = getRingBuffer(mapToPortID(data));
        if( pRing ) {
            op = pRing->getBufOp(pImgBuf, &(halCamIOQBuf.mvOut), &doDropInfo);
            ret = ret && processBufOp(op, halCamIOQBuf);
        } else {
            MY_LOGE("cannot find ringbuffer for %d", data);
            return MFALSE;
        }

        // discard pass2 info in IspSyncCtrl
        if( doDropInfo ) {
            mpIspSyncCtrlHw->dropPass2Info(pImgBuf);
        }

    } else {
        if( mpRingImgo )
        {
            QBufInfo halCamIOQBuf;
            BUF_OP op;
            op = mpRingImgo->getBufOp(pImgBuf, &(halCamIOQBuf.mvOut));
            ret = ret && processBufOp(op, halCamIOQBuf);
        }

        if( mpRingRrzo )
        {
            QBufInfo halCamIOQBuf;
            BUF_OP op;
            op = mpRingRrzo->getBufOp(pImgBuf, &(halCamIOQBuf.mvOut));
            ret = ret && processBufOp(op, halCamIOQBuf);
        }
    }

    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::
checkDeque(vector<BufInfo> const & QbufInfo)
{
    MBOOL ret = MTRUE;
    MUINT32 dequeMagicNum = 0;
    MINT64 timeStamp = 0;
    if( QbufInfo.size() == 0 ) {
        MY_LOGE("q size == 0!");
        return MFALSE;
    }
    //
    vector<BufInfo>::const_iterator iter;
    for( iter = QbufInfo.begin(); iter != QbufInfo.end(); iter++ )
    {
        ringbuffer* pRing = getRingBuffer( iter->mPortID );

        if( pRing == NULL ) {
            ret = MFALSE;
            break;
        }

        if( !pRing->updateDeque( iter->mBuffer ) ) {
            pRing->dump();
            ret = MFALSE;
        }
    }
    //
    for(MUINT32 i=0; i<QbufInfo.size(); i++)
    {
        if(i == 0)
        {
            dequeMagicNum = QbufInfo[i].mMetaData.mMagicNum_hal;
            timeStamp = QbufInfo[i].mMetaData.mTimeStamp;
            //
            if(dequeMagicNum & MAGIC_NUM_UNKNOWN_MASK)
            {
                MY_LOGW("ISP pass1 deque magic number is UNKNOWN");
                return MTRUE;
            }
            //
            if(timeStamp == 0)
            {
                MY_LOGE("ISP pass1 deque timestamp is 0");
                ret = MFALSE;
            }
        }
        else
        {
            if(dequeMagicNum != QbufInfo[i].mMetaData.mMagicNum_hal)
            {
                MY_LOGE("ISP pass1 deque magic number mismatch # 0x%X != # 0x%X",
                        dequeMagicNum,
                        QbufInfo[i].mMetaData.mMagicNum_hal);
                ret = MFALSE;
            }
            //
            if(timeStamp != QbufInfo[i].mMetaData.mTimeStamp)
            {
                MY_LOGE("ISP pass1 deque timestamp mismatch %lld != %lld",
                        timeStamp,
                        QbufInfo[i].mMetaData.mTimeStamp);
                ret = MFALSE;
            }
        }
    }
    //
    if( ret && !(dequeMagicNum & MAGIC_NUM_UNKNOWN_MASK) )
    {
        //MY_LOGD("previous MN # 0x%X and timestamp %lld", mLastDequeMagicNumber, mLastDequeTimestamp);
        //MY_LOGD("current MN # 0x%X and timestamp %lld", dequeMagicNum, timeStamp);
        if(dequeMagicNum < mLastDequeMagicNumber)
        {
            MY_LOGE("current MN # 0x%X < previous MN # 0x%X",
                    dequeMagicNum,
                    mLastDequeMagicNumber);
            ret = MFALSE;
        }
        //
        if(timeStamp <= mLastDequeTimestamp)
        {
            MY_LOGE("current timestamp %lld <= previous timestamp %lld",
                    timeStamp,
                    mLastDequeTimestamp);
            ret = MFALSE;
        }

        if( ret )
        {
            mLastDequeMagicNumber = dequeMagicNum;
            mLastDequeTimestamp = timeStamp;
        }
    }
    //
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
Pass1NodeImpl::ringbuffer*
Pass1NodeImpl::
getRingBuffer(PortID const portID) const
{
    if( portID == PORT_IMGO )
        return mpRingImgo;
    else if( portID == PORT_RRZO )
        return mpRingRrzo;
    return NULL;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
waitForDeque()
{
    Mutex::Autolock _l(mBufferLock);
    // 1. stop deque if going stop
    if( muStatus != Status_Running )
        return MFALSE;
    // 2. check if hw buf is available
    if( mpHwDeque->muStatus == STATUS_HW )
        return MTRUE;
    // 3. wait for enque
    MY_LOGD2("wait for port 0x%x enq buf 0x%x", mPortID, mpHwDeque->mBuffer);
    mCondHwBuf.wait(mBufferLock);
    return (muStatus == Status_Running);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
start()
{
    Mutex::Autolock _l(mBufferLock);
    if( muStatus != Status_Init ) {
        MY_LOGE2("wrong state %d", muStatus);
        return MFALSE;
    }

    muStatus = Status_Running;
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
stop()
{
    Mutex::Autolock _l(mBufferLock);
    if( muStatus != Status_Running ) {
        return MTRUE;
    }

    muStatus = Status_GoingStop;
    mCondHwBuf.signal();
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
addRingBuffer(MBOOL last)
{
    Mutex::Autolock _l(mBufferLock);

    if( muStatus != Status_Init ) {
        MY_LOGE2("wrong state %d", muStatus);
        return MFALSE;
    }

    ImgRequest ImgRequest;
    if( !mpBufHdl->dequeBuffer(mDataType, &ImgRequest) )
    {
        MY_LOGE2("deque buffer failed: data %d", mDataType);
        return MFALSE;
    }
    IImageBuffer* pImgBuf = const_cast<IImageBuffer*>(ImgRequest.mBuffer);

    // add ring
    {
        buffer_t** ppCur = (mpHead == NULL) ? (&mpHead) : &(mpHwEnque->mpNext);

        (*ppCur) = (buffer_t*)malloc(sizeof(buffer_t));
        (*ppCur)->muIndex      = muCount;
        (*ppCur)->muStatus     = STATUS_HW;
        (*ppCur)->mBuffer      = pImgBuf;
        (*ppCur)->mpNext       = NULL;

        muCount++;
        mpHwEnque = (*ppCur);

        if( last ) {
            //close the ring
            mpHwEnque->mpNext = mpHead;
            //initial
            mpHwEnque = mpHead;
            mpHwDeque = mpHead;
        }
    }
    //MY_LOGD2("head, enque 0x%x, 0%x, 0x%x", mpHead, mpHwEnque, mpHwEnque->mpNext);

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
addOfflineBuf()
{
    Mutex::Autolock _l(mBufferLock);

    ImgRequest ImgRequest;
    if( !mpBufHdl->dequeBuffer(mDataType, &ImgRequest) )
    {
        MY_LOGE2("deque buffer failed: data %d", mDataType);
        return MFALSE;
    }
    IImageBuffer* pImgBuf = const_cast<IImageBuffer*>(ImgRequest.mBuffer);

    st_buffer_t one = { pImgBuf, STATUS_PENDING };
    mvOfflineBuf.push_back(one);

    MY_LOGD2("replace buf portID 0x%x, buf 0x%x, va/pa 0x%x/0x%x",
            mPortID, pImgBuf, pImgBuf->getBufVA(0), pImgBuf->getBufPA(0));
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
queryInitBuffers(vector<BufInfo> * pvOut) const
{
    Mutex::Autolock _l(mBufferLock);
    buffer_t* pCur = mpHead;
    MUINT32 i = 0;
    do {
        BufInfo bufinfo (mPortID, pCur->mBuffer, i);
        pvOut->push_back(bufinfo);
        i++;
    } while( mpHead != (pCur = pCur->mpNext) );

    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
BUF_OP
Pass1NodeImpl::ringbuffer::
getBufOp(IImageBuffer* pImgBuf, vector<BufInfo> * pvBufinfo, MBOOL* doDropInfo)
{
    Mutex::Autolock _l(mBufferLock);
    BUF_OP ret = BUF_OP_NONE;

    if( doDropInfo != NULL )
        *doDropInfo = MFALSE;

    // if stopped
    if( muStatus == Status_Stopped )
    {
        // check if in pending list
        MINT32 idx = searchOffline(pImgBuf);
        if( idx != -1 ) {
            st_buffer_t* pStBuf = &mvOfflineBuf[idx];
            if( !transitBufStatus(pStBuf->muStatus, true) ) {
                return BUF_OP_ERROR;
            }

            MY_LOGD2("stop, add pending: buf 0x%x", pImgBuf);
            // check if buffer still available
            if( pStBuf->muStatus == STATUS_RELEASE ) {
                if( doDropInfo != NULL )
                    *doDropInfo = MTRUE;
                if( mpBufHdl->isBufferExisted(mDataType, pImgBuf) )
                    pStBuf->muStatus = STATUS_PENDING;
            }

            if( pStBuf->muStatus == STATUS_PENDING )
            {
                if( doDropInfo != NULL )
                    *doDropInfo = MTRUE;

                if( !mpBufHdl->enqueBuffer(mDataType, pImgBuf) )
                {
                    MY_LOGE2("enque buf failed");
                    return BUF_OP_ERROR;
                }
            }

            ret = BUF_OP_NONE;
            return ret;
        }
        MY_LOGE2("cannot find buf 0x%x", pImgBuf);
        return BUF_OP_ERROR;
    }

    if( pImgBuf ) {
        // 1. check if any buf has been dequeued
        if( mpHwEnque->muStatus == STATUS_HW ) {
            // check if in pending list
            MINT32 idx = searchOffline(pImgBuf);
            if( idx != -1 ) {
                st_buffer_t* pStBuf = &mvOfflineBuf[idx];
                if( !transitBufStatus(pStBuf->muStatus, true) ) {
                    ret = BUF_OP_ERROR;
                    goto lbExit;
                }

                // check if buffer still available
                if( pStBuf->muStatus == STATUS_RELEASE ) {
                    if( doDropInfo != NULL )
                        *doDropInfo = MTRUE;
                    if( mpBufHdl->isBufferExisted(mDataType, pImgBuf) )
                        pStBuf->muStatus = STATUS_PENDING;
                }

                if( pStBuf->muStatus == STATUS_PENDING && doDropInfo != NULL)
                    *doDropInfo = MTRUE;

#if ENABLE_BUFCONTROL_LOG
                MY_LOGD2("add pending: buf 0x%x, size %d, cur enq %d, deq %d (%d)",
                        pImgBuf,
                        mvOfflineBuf.size(),
                        mpHwEnque->muIndex,
                        mpHwDeque->muIndex,
                        mpHwDeque->muStatus
                        );
#endif
            } else {
                MY_LOGE2("not in pending");
                ret = BUF_OP_ERROR;
            }

            goto lbExit;
        }
        // 2. try to enque
        if( mpHwEnque->mBuffer == pImgBuf ) {

            if( !transitBufStatus(mpHwEnque->muStatus, true) ) {
                ret = BUF_OP_ERROR;
                goto lbExit;
            }

            // check if buffer still available
            if( mpHwEnque->muStatus == STATUS_RELEASE ) {
                if( doDropInfo != NULL )
                    *doDropInfo = MTRUE;
                if( mpBufHdl->isBufferExisted(mDataType, pImgBuf) )
                    mpHwEnque->muStatus = STATUS_PENDING;
            }

            if( mpHwEnque->muStatus == STATUS_PENDING ) {
                if( doDropInfo != NULL )
                    *doDropInfo = MTRUE;

                BufInfo bufinfo(mPortID, pImgBuf, mpHwEnque->muIndex);
                pvBufinfo->push_back(bufinfo);

#if ENABLE_BUFCONTROL_LOG
                MY_LOGD2("enq %d port 0x%x, buf 0x%x, va/pa(0x%x/0x%x)",
                        mpHwEnque->muIndex,
                        mPortID,
                        pImgBuf,
                        pImgBuf->getBufVA(0),
                        pImgBuf->getBufPA(0));
#endif
                // update ringbuffer
                mpHwEnque->muStatus = STATUS_HW;
                mpHwEnque = mpHwEnque->mpNext;

                // try to get more buf
                while( mpHwEnque->muStatus == STATUS_PENDING ) {
                    BufInfo bufinfo(mPortID, mpHwEnque->mBuffer, mpHwEnque->muIndex);
                    pvBufinfo->push_back(bufinfo);

                    mpHwEnque->muStatus = STATUS_HW;
                    mpHwEnque = mpHwEnque->mpNext;
                }
                mCondHwBuf.signal();
                ret = BUF_OP_ENQUE;
            }
            goto lbExit;
        }
        // 3. try to replace
        {
            //check if in ringbuffer
            buffer_t* pTemp = mpHwEnque;
            do{
                if( pTemp->mBuffer == pImgBuf ){
                    MY_LOGD2("port 0x%x pending in ring buf 0x%x", mPortID, pImgBuf);
                    if( !transitBufStatus(pTemp->muStatus, true) ) {
                        ret = BUF_OP_ERROR;
                        goto lbExit;
                    }

                    // check if buffer still available
                    if( pTemp->muStatus == STATUS_RELEASE ) {
                        if( doDropInfo != NULL )
                            *doDropInfo = MTRUE;
                        if( mpBufHdl->isBufferExisted(mDataType, pImgBuf) )
                            pTemp->muStatus = STATUS_PENDING;
                    }
                    // do not use the buf in hw ringbuf to replace buffer
#if 1
                    if( pTemp->muStatus == STATUS_PENDING && doDropInfo != NULL)
                        *doDropInfo = MTRUE;
#else
                    if( pTemp->muStatus == STATUS_PENDING ) {
                        st_buffer_t old = { mpHwEnque->mBuffer, mpHwEnque->muStatus };

                        if( doDropInfo != NULL)
                            *doDropInfo = MTRUE;

                        // do replace, and set EMPTY
                        BufInfo buf_old(mPortID, mpHwEnque->mBuffer, mpHwEnque->muIndex);
                        BufInfo buf_new(mPortID, pImgBuf, mpHwEnque->muIndex);
                        pvBufinfo->push_back(buf_old);
                        pvBufinfo->push_back(buf_new);

                        pTemp->muStatus = STATUS_EMPTY;

#if ENABLE_BUFCONTROL_LOG
                        MY_LOGD2("replace3 %d port 0x%x old/new 0x%x/0x%x va/pa(0x%x/0x%x, 0x%x/0x%x)",
                                mpHwEnque->muIndex,
                                mPortID,
                                buf_old.mBuffer,
                                buf_new.mBuffer,
                                buf_old.mBuffer->getBufVA(0),
                                buf_old.mBuffer->getBufPA(0),
                                buf_new.mBuffer->getBufVA(0),
                                buf_new.mBuffer->getBufPA(0));
#endif
                        // move to pending
                        if( old.muStatus != STATUS_EMPTY )
                            mvOfflineBuf.push_back(old);

                        // update ringbuffer
                        mpHwEnque->mBuffer = pImgBuf;
                        mpHwEnque->muStatus = STATUS_HW;
                        mpHwEnque = mpHwEnque->mpNext;

                        mCondHwBuf.signal();
                        ret = BUF_OP_REPLACE;
                    }
#endif
                    goto lbExit;
                }
                pTemp = pTemp->mpNext;
            }while( pTemp != mpHwEnque );
        }

        // not in ring buffer
        {
            MINT32 idx = searchOffline(pImgBuf);
            if( idx != -1 ) {
                st_buffer_t* pStBuf = &mvOfflineBuf[idx];
                //
                if( !transitBufStatus(pStBuf->muStatus, true) ) {
                    ret = BUF_OP_ERROR;
                    goto lbExit;
                }
                // check if buffer still available
                if( pStBuf->muStatus == STATUS_RELEASE ) {
                    if( doDropInfo != NULL )
                        *doDropInfo = MTRUE;
                    if( mpBufHdl->isBufferExisted(mDataType, pImgBuf) )
                        pStBuf->muStatus = STATUS_PENDING;
                }
                //
                if( pStBuf->muStatus == STATUS_PENDING ) {
                    if( doDropInfo != NULL )
                        *doDropInfo = MTRUE;

                    BufInfo buf_old(mPortID, mpHwEnque->mBuffer, mpHwEnque->muIndex);
                    BufInfo buf_new(mPortID, pImgBuf, mpHwEnque->muIndex);
                    pvBufinfo->push_back(buf_old);
                    pvBufinfo->push_back(buf_new);

#if ENABLE_BUFCONTROL_LOG
                    MY_LOGD2("replace1 %d port 0x%x old/new 0x%x/0x%x va/pa(0x%x/0x%x, 0x%x/0x%x)",
                            mpHwEnque->muIndex,
                            mPortID,
                            buf_old.mBuffer,
                            buf_new.mBuffer,
                            buf_old.mBuffer->getBufVA(0),
                            buf_old.mBuffer->getBufPA(0),
                            buf_new.mBuffer->getBufVA(0),
                            buf_new.mBuffer->getBufPA(0));
#endif
                    // update offline
                    if( mpHwEnque->muStatus == STATUS_EMPTY ) {
                        mvOfflineBuf.erase( mvOfflineBuf.begin() + idx );
                    } else {
                        pStBuf->mBuffer = mpHwEnque->mBuffer;
                        pStBuf->muStatus = mpHwEnque->muStatus;
                    }
                    // update ringbuffer
                    mpHwEnque->mBuffer = pImgBuf;
                    mpHwEnque->muStatus = STATUS_HW;
                    mpHwEnque = mpHwEnque->mpNext;

                    mCondHwBuf.signal();
                    ret = BUF_OP_REPLACE;
                }
            } else {
                MY_LOGE2("not in pending");
                ret = BUF_OP_ERROR;
            }
            goto lbExit;
        }

        ret = BUF_OP_ERROR;
        goto lbExit;
    } else {

            // do not use the buf in hw ringbuf to replace buffer
#if 1
            // try to enque pending buf
            if( mpHwEnque->muStatus == STATUS_PENDING )
            {
                while( mpHwEnque->muStatus == STATUS_PENDING ) {
                    BufInfo bufinfo(mPortID, mpHwEnque->mBuffer, mpHwEnque->muIndex);
                    pvBufinfo->push_back(bufinfo);

                    mpHwEnque->muStatus = STATUS_HW;
                    mpHwEnque = mpHwEnque->mpNext;
                }

                mCondHwBuf.signal();
                ret = BUF_OP_ENQUE;
                goto lbExit;
            }
#endif

            // try to replace buf to keep ring running
            if( mvOfflineBuf.size() &&
                    ( mpHwEnque->muStatus == STATUS_DEQUE ||
                      mpHwEnque->muStatus == STATUS_FAKEDEQUE ||
                      mpHwEnque->muStatus == STATUS_RELEASE ||
                      mpHwEnque->muStatus == STATUS_EMPTY) )
            {
                // find one pending buffer
                st_buffer_t* pStBuf = NULL;
                vector<st_buffer_t>::iterator iterBuf = mvOfflineBuf.begin();
                while( iterBuf != mvOfflineBuf.end() ) {
                    if( iterBuf->muStatus == STATUS_PENDING ) {
                        pStBuf = &(*iterBuf);
                        break;
                    }
                    iterBuf++;
                }

                if( pStBuf != NULL ) {
                    IImageBuffer* newImgBuf = pStBuf->mBuffer;

                    BufInfo buf_old(mPortID, mpHwEnque->mBuffer, mpHwEnque->muIndex);
                    BufInfo buf_new(mPortID, newImgBuf, mpHwEnque->muIndex);
                    pvBufinfo->push_back(buf_old);
                    pvBufinfo->push_back(buf_new);

#if ENABLE_BUFCONTROL_LOG
                    MY_LOGD2("replace2 %d port 0x%x old/new 0x%x/0x%x va/pa(0x%x/0x%x, 0x%x/0x%x)",
                            mpHwEnque->muIndex,
                            mPortID,
                            buf_old.mBuffer,
                            buf_new.mBuffer,
                            buf_old.mBuffer->getBufVA(0),
                            buf_old.mBuffer->getBufPA(0),
                            buf_new.mBuffer->getBufVA(0),
                            buf_new.mBuffer->getBufPA(0));
#endif
                    // update offline
                    if( mpHwEnque->muStatus == STATUS_EMPTY ) {
                        mvOfflineBuf.erase(iterBuf);
                    } else {
                        pStBuf->mBuffer = mpHwEnque->mBuffer;
                        pStBuf->muStatus = mpHwEnque->muStatus;
                    }
                    // update ringbuffer
                    mpHwEnque->mBuffer = newImgBuf;
                    mpHwEnque->muStatus = STATUS_HW;
                    mpHwEnque = mpHwEnque->mpNext;

                    mCondHwBuf.signal();
                    ret = BUF_OP_REPLACE;
                }
                goto lbExit;
            }
        }

lbExit:
        if( mPortID == PORT_RRZO )
            CAM_TRACE_INT("P1Cnt0",
                    mpHwEnque->muStatus == STATUS_HW ?
                    muCount : (muCount + mpHwEnque->muIndex - mpHwDeque->muIndex) % muCount );
        else
            CAM_TRACE_INT("P1Cnt1",
                    mpHwEnque->muStatus == STATUS_HW ?
                    muCount : (muCount + mpHwEnque->muIndex - mpHwDeque->muIndex) % muCount );

        if( muStatus != Status_Running && ret != BUF_OP_ERROR )
            ret = BUF_OP_NONE;

        return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
updateDeque(IImageBuffer* pImgBuf)
{
//#if ENABLE_CAMIOCONTROL_LOG
    MY_LOGD2("deq port 0x%x, buf 0x%x, t %lld",
            mPortID, pImgBuf, pImgBuf->getTimestamp());
//#endif

    Mutex::Autolock _l(mBufferLock);
    if( mpHwDeque->mBuffer == pImgBuf ) {
        if( !transitBufStatus( mpHwDeque->muStatus, false ) ) {
            MY_LOGE2("transit status error st %d, en %d", mpHwDeque->muStatus, false);
            return MFALSE;
        }
        //mpHwDeque->muStatus = STATUS_DEQUE;
        mpHwDeque = mpHwDeque->mpNext;
        return MTRUE;
    }

    if( mPortID == PORT_RRZO )
        CAM_TRACE_INT("P1Cnt0",
                mpHwEnque->muStatus == STATUS_HW ?
                muCount : (muCount + mpHwEnque->muIndex - mpHwDeque->muIndex) % muCount );
    else
        CAM_TRACE_INT("P1Cnt1",
                mpHwEnque->muStatus == STATUS_HW ?
                muCount : (muCount + mpHwEnque->muIndex - mpHwDeque->muIndex) % muCount );


    MY_LOGE2("wrong deque buf 0x%x", pImgBuf);
    return MFALSE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
returnBuffers()
{
    MBOOL ret = MTRUE;
    Mutex::Autolock _l(mBufferLock);

    if( mpHead == NULL )
        return MTRUE;

    // update ringbuffer status
    muStatus = Status_Stopped;

    MY_LOGD2("move ring buffers %d", muCount);
    buffer_t* pCur = mpHead;
    do {
        buffer_t* pNext = pCur->mpNext;

        if( pCur == NULL )
        {
            MY_LOGE2("may not start yet");
            break;
        }

        st_buffer_t one;
        one.mBuffer = pCur->mBuffer;
        if(pCur->muStatus == STATUS_HW)
        {
            one.muStatus = STATUS_PENDING;
        }
        else
        {
            one.muStatus = pCur->muStatus;
        }
        mvOfflineBuf.push_back(one);

        muCount--;
        free(pCur);
        //next one
        pCur = pNext;
    } while( pCur != mpHead );

    MY_LOGD2("return offline buffers (%d)+", mvOfflineBuf.size());
    vector<st_buffer_t>::iterator iterPending = mvOfflineBuf.begin();
    while( iterPending != mvOfflineBuf.end() )
    {
        if( iterPending->muStatus == STATUS_PENDING ) {
            ret = ret && mpBufHdl->enqueBuffer(mDataType, iterPending->mBuffer);
        } else if( iterPending->muStatus == STATUS_RELEASE ) {
            MY_LOGD2("already release port %p, buf %p", mPortID, iterPending->mBuffer);
        } else {
            MY_LOGD2("not return yet: port 0x%x buf %p", mPortID, iterPending->mBuffer);
        }
        iterPending++;
    }
    MY_LOGD2("return offline buffers (%d)-", mvOfflineBuf.size());

    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
isAllBufferReturned() const
{
    MBOOL returned = MTRUE;

    vector<st_buffer_t>::const_iterator iterPending = mvOfflineBuf.begin();
    while( iterPending != mvOfflineBuf.end() )
    {
        if( iterPending->muStatus != STATUS_PENDING && iterPending->muStatus != STATUS_RELEASE ) {
            MY_LOGW2("not return yet: port 0x%x buf %p, st 0x%x",
                    mPortID, iterPending->mBuffer, iterPending->muStatus);
            returned = MFALSE;
            break;
        }
        iterPending++;
    }

    if( mpHwEnque->muStatus == STATUS_DEQUE ||
        mpHwEnque->muStatus == STATUS_FAKEDEQUE )
    {
        MY_LOGW2("not return yet: port 0x%x buf %p, st 0x%x",
                mPortID, mpHwEnque->mBuffer, mpHwEnque->muStatus);
        returned = MFALSE;
    }

    return returned;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
Pass1NodeImpl::ringbuffer::
dump() const
{
    if( mpHead == NULL ) {
        MY_LOGW2("empty ring buffer");
        return;
    }

    buffer_t* ptemp = mpHead;
    do {
        MY_LOGW2("dump data %d, port(0x%x) ring %d, st %d, 0x%x",
                 mDataType,
                 mPortID,
                 ptemp->muIndex,
                 ptemp->muStatus,
                 ptemp->mBuffer);
    }while( mpHead != (ptemp = ptemp->mpNext));

    if( mpHwDeque && mpHwEnque ) {
        MY_LOGW2("cur deq %d/enq %d", mpHwDeque->muIndex, mpHwEnque->muIndex);
    }
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
Pass1NodeImpl::ringbuffer::
setBufHandler(ICamBufHandler* pBufHdl)
{
    mpBufHdl = pBufHdl;
    mbIsReg = pBufHdl->isRegistered(mDataType);
    if( mbIsReg )
        MY_LOGD2("registered data %d", mDataType);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
Pass1NodeImpl::ringbuffer::
transitBufStatus(MUINT32& status, bool enque)
{
    MBOOL ret = MTRUE;
    if( enque ) {
        switch(status)
        {
            case STATUS_DEQUE:
                status = mbIsReg ? STATUS_RELEASE : STATUS_PENDING;
                break;
            case STATUS_FAKEDEQUE:
                status = STATUS_DEQUE;
                break;
            default:
                ret = MFALSE;
                break;
        }
    } else {
        switch(status)
        {
            case STATUS_HW:
                status = (muFakeDataType != 0)? STATUS_FAKEDEQUE : STATUS_DEQUE;
                break;
            default:
                ret = MFALSE;
                break;
        }
    }
    if( !ret )
        MY_LOGE2("transitBufStatus fail st %d, en %d", status, enque);
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MINT32
Pass1NodeImpl::ringbuffer::
searchOffline(IImageBuffer* const pImgBuf)
{
    MINT32 c = 0;
    vector<st_buffer_t>::iterator iterBuf = mvOfflineBuf.begin();
    while( iterBuf != mvOfflineBuf.end() ) {
        if( iterBuf->mBuffer == pImgBuf )
            return c;
        c++;
        iterBuf++;
    }
    return -1;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass1NodeImpl::
configFrame(MUINT32 const magicNum)
{
    if( magicNum == MAGIC_NUM_INVALID )
    {
        MY_LOGD("skip config frame");
        return MTRUE;
    }
    //
    MY_LOGV("magic # 0x%X", magicNum);
    //
    QFrameParam currQFrameParam;
    currQFrameParam.mMagicNum = magicNum;
    MRect pass1InCrop;
    MSize pass1OutSize;
    //
    if(mbCfgRrzo) // if rrzo is used
    {
        mpIspSyncCtrlHw->getPass1Crop(
                            magicNum,
                            MTRUE,
                            pass1InCrop,
                            pass1OutSize);
        FrameParam currFrameParam = FrameParam(
                                        PORT_RRZO,
                                        pass1OutSize,
                                        pass1InCrop);
        currQFrameParam.mvOut.push_back(currFrameParam);
    }
    else
    if(mbCfgImgo) // if only use imgo
    {
        mpIspSyncCtrlHw->getPass1Crop(
                            magicNum,
                            MFALSE,
                            pass1InCrop,
                            pass1OutSize);
        FrameParam currFrameParam = FrameParam(
                                        PORT_IMGO,
                                        pass1OutSize,
                                        pass1InCrop);
        currQFrameParam.mvOut.push_back(currFrameParam);
    }
    //
    //NG_TRACE_BEGIN("configFrame");
    // force pure raw if imgo is enabled
    MBOOL purerawEnabled = MFALSE;
    if (mInitCfg.muRawType == SENSOR_DYNAMIC_PURE_RAW)
    {
        purerawEnabled = mpIspSyncCtrlHw->isPass1PureRawEnable();
        if ( purerawEnabled && mbCfgImgo )
        {
            purerawEnabled &= mpIspSyncCtrlHw->getPass1PureRawStatus(magicNum);
            //if ( purerawEnabled )
            //{
            //    MY_LOGD("config format for pure raw, magicNum(%d)", magicNum);
            //}
        }
    }
    // when use flash, Raw should be pure raw
    else if (mInitCfg.muRawType == SENSOR_PURE_RAW && mbCfgImgo)
    {
        purerawEnabled = MTRUE;
    }
    if (mInitCfg.muRawType == SENSOR_DYNAMIC_PURE_RAW)
    {
        MY_LOGD("currQFrameParam.mMagicNum=%d, purerawEnabled=%d", currQFrameParam.mMagicNum, purerawEnabled);
    }
    currQFrameParam.mRawOutFmt = purerawEnabled ? EPipe_PURE_RAW : EPipe_PROCESSED_RAW;
    mpCamIO->configFrame(currQFrameParam);
    //CAM_TRACE_END();
    //
    handleNotify(PASS1_CONFIG_FRAME, magicNum, 0);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
void*
Pass1NodeImpl::
doThreadAllocBuf(void* arg)
{
    ::prctl(PR_SET_NAME,"pass1_allocP1", 0, 0, 0);
    threadData* data = reinterpret_cast<threadData*>(arg);
    data->pSelf->mRet = data->pSelf->allocBuffers(*(data->plPortCfg));
    pthread_exit(NULL);
    return NULL;
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

