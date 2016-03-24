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

#define LOG_TAG "MtkCam/fdNodeImp"
#define FD_PIPE (1)

//
#include <mtkcam/Log.h>
#include <sys/time.h>
#include <cutils/properties.h>

#include <mtkcam/v3/pipeline/IPipelineNode.h>
#include "../BaseNode.h"
#include <mtkcam/v3/hwnode/FDNode.h>
#if (FD_PIPE)
#include <mtkcam/featureio/fd_hal_base.h>
#include <mtkcam/common/faces.h>
// GS
#include <mtkcam/featureio/gs_hal_base.h>
// ASD
#include <mtkcam/featureio/asd_hal_base.h>
#include <mtkcam/hal/sensor_hal.h>
//
#endif
//
#include <utils/RWLock.h>
#include <utils/Thread.h>
//
#include <sys/prctl.h>

//for sensor
#include <math.h>
#include <android/sensor.h>
#include <mtkcam/utils/SensorListener.h>


//
#include <mtkcam/metadata/IMetadataProvider.h>
#include <mtkcam/metadata/client/mtk_metadata_tag.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/v3/hal/IHal3A.h>

using namespace android;
#include "mtkcam/hal/IHalSensor.h"

using namespace NSCam;
using namespace NSCam::v3;
using namespace NSCam::Utils::Sync;
using namespace NS3Av3;


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

#define ASD_DEFAULT_WIDTH (320)
#define MHAL_ASD_WORKING_BUF_SIZE       (160*120*2*11+200*1024)
#define max(a,b)  ((a) < (b) ? (b) : (a))
#define min(a,b)  ((a) < (b) ? (a) : (b))

#define ENABLE_FD_ASYNC_MODE (0)
#if ENABLE_FD_ASYNC_MODE
#include <semaphore.h>
#define FD_SKIP_NUM (0)
#else
#define FD_SKIP_NUM (0)
#endif
#define DUMP_SENSOR_LOG (0)

struct FDImage {
    int w;
    int h;
    MUINT8* AddrY;
    MUINT8* AddrU;
    MUINT8* AddrV;
    MINT32 format;
    MINT32 planes;
};

/******************************************************************************
 *
 ******************************************************************************/
//
//  [Input]
//      Image/Yuv
//      Meta/Request
//
//  [Output]
//      Meta/Result
//
namespace {
class FdNodeImp
    : public BaseNode
    , public FdNode
    , protected Thread
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                                            Definitions.
    typedef android::sp<IPipelineFrame>                     QueNode_T;
    typedef android::List<QueNode_T>                        Que_T;
    //typedef android::Vector<android::sp<IMetaStreamInfo> >  MetaStreamInfoSetT;
    //typedef android::Vector<android::sp<IImageStreamInfo> > ImageStreamInfoSetT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Definitions.

    /**
     * Initialization Parameters.
     */



protected:  ////                    Data Members. (Config)
    mutable RWLock                  mConfigRWLock;
    sp<IMetaStreamInfo>             mpOutMetaStreamInfo_Result;
    sp<IMetaStreamInfo>             mpInMetaStreamInfo_Request;
    sp<IImageStreamInfo>            mpInImageStreamInfo_Yuv;
    //ImageStreamInfoSetT             mpInImageStreamInfo_Yuv;

protected:  ////                    Data Members. (Request Queue)
    mutable Mutex                   mRequestQueueLock;
    mutable Mutex                   mResultLock;
    Condition                       mRequestQueueCond;
    MBOOL                           mbRequestDrained;
    Condition                       mbRequestDrainedCond;
    Que_T                           mRequestQueue;

protected:  ////                    Data Members. (Request Queue)
    IHal3A*                         p3AHal;

protected:  ////                    Operations.
    MERROR                          onDequeRequest(
                                        android::sp<IPipelineFrame>& rpFrame
                                    );
    MVOID                           onProcessFrame(
                                        android::sp<IPipelineFrame> const& pFrame
                                    );

    MVOID                           waitForRequestDrained();

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
    //
    MINT32                          doBufferAnalysis(int BufWidth, int BufHeight, int plane);
    MVOID                           ReturnFDResult(
                                        IMetadata*              pOutMetadataResult,
                                        IMetadata*              pInpMetadataRequest
                                    );
    MVOID                           RunFaceDetection();
    #if ENABLE_FD_ASYNC_MODE
    // +++ FD Async mode
    static MVOID*                   FDHalThreadLoop(MVOID*);

    MVOID                           setFDLock(MBOOL val);

    // --- FD Async mode
    #endif

    MINT32                          doGestureDetect(MUINT8* pImage, MINT32 RotateInfo);

    MINT32                          getGestureResult();

    MINT32                          onInitFDProc();

    // ASD function
    class ASDImp
    {
        public:
                                        ASDImp()
                                            : mASDWorkingBuffer(NULL)
                                            , mASDWorkingBufferSize(0)
                                            , mpASDHalObj(NULL)
                                            , mpHal3A(NULL)
                                        {MY_LOGD("ASDImp +");}
                                        ~ASDImp()
                                        {MY_LOGD("ASDImp -");}
        struct ASDInfo {
            MUINT32     Scene;
            //TBD
            MUINT32     HDR;
        };
        MINT32                          updateASD(MUINT8* pRGBImg, MINT32 BufWidth, MINT32 BufHeight, MINT32 FaceNum, ASDInfo* Result);

        MINT32                          initASD(MINT32 id);

        MINT32                          uninitASD();

        private:
        MUINT8*                         mASDWorkingBuffer;
        MUINT32                         mASDWorkingBufferSize;
        MUINT32                         mSensorType;
        halASDBase*                     mpASDHalObj;
        IHal3A*                         mpHal3A;
        MINT32                          mHalInited;
        mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM             mSceneCur;
        MINT32                          mFDNodeID;
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Operations.
                                    FdNodeImp();

                                    ~FdNodeImp();

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

private:
    // crop-related
    MRect                           mActiveArray;
    MtkCameraFaceMetadata*          mpDetectedFaces;
    MtkCameraFaceMetadata*          mpDetectedGestures;
    MBOOL                           mbInited;
    MBOOL                           mbDoFD;
    MUINT32                         mFDSkip;
    MUINT8*                         mFDImgBuffer;// = new unsigned char[640*480];
    MUINT8*                         mFDWorkingBuffer;
    MUINT32                         mFDWorkingBufferSize;
    halFDBase*                      mpFDHalObj;
    halGSBase*                      mpGSHalObj;
    MINT32                          mImageWidth;
    MINT32                          mImageHeight;
    MUINT8*                         mPureYBuf;
    struct FDImage                  mDupImage;
    // SD
    MINT32                          mSD_Result;
    MINT32                          mSDEnable;
    MINT32                          mPrevSD;
    // GS
    MINT32                          mGDEnable;
    // ASD
    MINT32                          mASDEnable;
    ASDImp*                         mASDObj;
    ASDImp::ASDInfo                 mASDInfo;

    MINT32                          mFDProcInited;

    //Sensor
    SensorListener                  *mpSensorListener;
    MBOOL                           mAccEnable;
    #if ENABLE_FD_ASYNC_MODE
    // +++ FD Async mode
    pthread_t                       mFDHalThread;
    sem_t                           semFD;
    mutable Mutex                   mFDLock;
    MBOOL                           mIsFDBusy;
    MBOOL                           mStopFD;
    // --- FD Async mode
    #endif

    // For Log
    MINT32                          mLogLevel;
};
};  //namespace

//static MFLOAT                          mAccInfo[3];
static int                          grotation;

/******************************************************************************
 *
 ******************************************************************************/
static MVOID
mySensorListener(ASensorEvent event)
{
    static MUINT32 accCnt = 1;

    switch(event.type)
    {
        case ASENSOR_TYPE_ACCELEROMETER:
        {
            float X,Y,Z;
            float OneEightyOverPi = 57.29577957855f;
            float magnitude;
            float angle;
            int ori = 0;
            X = -(event.acceleration.x);
            Y = -(event.acceleration.y);
            Z = -(event.acceleration.z);
            magnitude = X*X + Y*Y;
            if(DUMP_SENSOR_LOG)
            {
                MY_LOGD("[%u] Acc(%f,%f,%f,%lld)",
                        accCnt++,
                        event.acceleration.x,
                        event.acceleration.y,
                        event.acceleration.z,
                        event.timestamp);
            }
            if(magnitude * 4 >= Z*Z) {
                angle = atan2(-Y, X) * OneEightyOverPi;
                ori = 180 - round(angle);

                while(ori >= 360) {
                    ori -= 360;
                }
                while(ori < 0) {
                    ori += 360;
                }

                if(ori >= 45 && ori < 135) {
                    grotation = 90;
                } else if (ori >= 135 && ori < 225) {
                    grotation = 180;
                } else if (ori >= 225 && ori < 315) {
                    grotation = 270;
                } else {
                    grotation = 0;
                }
            } else {
                MY_LOGW("magnitude too small, cannot trust");
            }
            /*
            mAccInfo[0] = event.acceleration.x;
            mAccInfo[1] = event.acceleration.y;
            mAccInfo[2] = event.acceleration.z;
            */
            break;
        }
        case ASENSOR_TYPE_MAGNETIC_FIELD:
        {
            if(DUMP_SENSOR_LOG)
            {
                MY_LOGD("Mag");
            }
            break;
        }
        case ASENSOR_TYPE_GYROSCOPE:
        {
            if(DUMP_SENSOR_LOG)
            {
                MY_LOGD("Light");
            }

            break;
        }
        case ASENSOR_TYPE_LIGHT:
        {
            if(DUMP_SENSOR_LOG)
            {
                MY_LOGD("Light");
            }
            break;
        }
        case ASENSOR_TYPE_PROXIMITY:
        {
            if(DUMP_SENSOR_LOG)
            {
                MY_LOGD("Proxi");
            }
            break;
        }
        default:
        {
            MY_LOGW("unknown type(%d)",event.type);
            break;
        }
    }
}



/******************************************************************************
 *
 ******************************************************************************/
FdNodeImp::
FdNodeImp()
    : BaseNode()
    , FdNode()
    //
    , mConfigRWLock()
    //
    , mRequestQueueLock()
    , mResultLock()
    , mRequestQueueCond()
    , mbRequestDrained(MFALSE)
    , mbRequestDrainedCond()
    , mRequestQueue()
    //
    , mpDetectedFaces(NULL)
    , mpDetectedGestures(NULL)
    , mbInited(MFALSE)
    , mbDoFD(MFALSE)
    , mFDSkip(0)
    , mFDImgBuffer(NULL)
    , mFDWorkingBuffer(NULL)
    , mFDWorkingBufferSize(0)
    , mpFDHalObj(NULL)
    , mpGSHalObj(NULL)
    , mImageWidth(0)
    , mImageHeight(0)
    , mPureYBuf(NULL)
    , mSD_Result(0)
    , mSDEnable(0)
    , mPrevSD(0)
    , mGDEnable(0)
    , mASDEnable(0)
    , mFDProcInited(0)
{
    mASDObj = new ASDImp();
    #if ENABLE_FD_ASYNC_MODE
    mAllocator = NULL;
    #endif
}

/******************************************************************************
 *
 ******************************************************************************/
FdNodeImp::
~FdNodeImp()
{
    MY_LOGD("FDNode -");
    delete mASDObj;
}

MINT32
FdNodeImp::
onInitFDProc()
{
    //====== Create Sensor Listener Object ======

    mpSensorListener = SensorListener::createInstance();

    if(MTRUE != mpSensorListener->setListener(mySensorListener))
    {
        MY_LOGE("setListener fail");
    }
    else
    {
        MY_LOGD("setListener success");
    }

    mAccEnable  = mpSensorListener->enableSensor(SensorListener::SensorType_Acc,33);
    MY_LOGD("EN:(Acc)=(%d)",mAccEnable);

    grotation = 0;
    return 0;
}

MVOID
FdNodeImp::
RunFaceDetection()
{
    int RotateInfo;
    MUINT8* pDstY = NULL;
    if(mAccEnable) {
        RotateInfo = grotation;
    } else {
        RotateInfo = 360;
    }

    p3AHal = IHal3A::createInstance(IHal3A::E_Camera_3, getOpenId(), getNodeName());
    #if 0
    if(p3AHal)
    {
        p3AHal->setFDEnable(true);
    }
    #endif


/*
        char str_dstfile_0[128];
        FILE *cfptr_1;
        int read_size;
        sprintf(str_dstfile_0, "/system/data/640x480_Y_9_1.raw");
        if((cfptr_1=fopen(str_dstfile_0,"rb")) ==NULL)
        {
            printf("File could not be opened \n");
        }
        else
        {
            read_size = fread(FDImgBuffer, sizeof(unsigned char), 640*480, cfptr_1);
            fclose(cfptr_1);
        }
*/

    int srcWidth = mDupImage.w;
    int srcHeight = mDupImage.h;
    int numFace = 0;
    int Group = 0;
    if(mImageWidth == 0 || mImageHeight == 0) {
        mpFDHalObj->halFDInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize, 1, mSDEnable);
        mpGSHalObj->halGSInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize);
    } else if(mImageWidth != srcWidth || mImageHeight != srcHeight || mSDEnable != mPrevSD) {
        mpFDHalObj->halFDUninit();
        mpFDHalObj->halFDInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize, 1, mSDEnable);
        if(mImageWidth != srcWidth || mImageHeight != srcHeight) {
            mpGSHalObj->halGSUninit();
            mpGSHalObj->halGSInit(srcWidth, srcHeight, mFDWorkingBuffer, mFDWorkingBufferSize);
        }
    }
    mPrevSD = mSDEnable;
    mImageWidth = srcWidth;
    mImageHeight = srcHeight;

    {
        Group = doBufferAnalysis(srcWidth, srcHeight, mDupImage.planes);
        //mpFDHalObj->halFDBufferCreate((MUINT8 *)mFDImgBuffer, (MUINT8 *)pInpImageBufferYuv->getBufVA(0), Group);
        mpFDHalObj->halFTBufferCreate2((MUINT8 *)mFDImgBuffer, (MUINT8 *)mDupImage.AddrY, (MUINT8 *)mDupImage.AddrU, (MUINT8 *)mDupImage.AddrV, mDupImage.planes, srcWidth, srcHeight);
    }
    if(mDupImage.planes == 1) {
        //pDstY = mPureYBuf;
        //mpFDHalObj->halFDYUYV2ExtractY(pDstY, (MUINT8 *)mDupImage.AddrY, srcWidth, srcHeight);
        MY_LOGE("[ERROR] Image planes = 1 is not supported in this platform!!!!");
        return ;
    } else {
        pDstY = (MUINT8 *)mDupImage.AddrY;
    }

    // Do FD
    mpFDHalObj->halFDDo(0, (MUINT8 *)mFDImgBuffer, pDstY,  mSDEnable, RotateInfo, NULL);

    {
        #if ENABLE_FD_ASYNC_MODE
        Mutex::Autolock _l(mResultLock);
        #endif
        // reset face number
        mpDetectedFaces->number_of_faces = 0;

        numFace = mpFDHalObj->halFDGetFaceResult(mpDetectedFaces);

        MY_LOGD_IF(mLogLevel > 0, "NumFace = %d, ", numFace);

        if(mSDEnable) {
            mSD_Result = mpFDHalObj->halSDGetSmileResult();
            MY_LOGD("Smile Result:%d", mSD_Result);
        }
    }

    // Do GD
    if(numFace>0)
    {
        doGestureDetect(pDstY, RotateInfo);
        getGestureResult();
    }

//*********************************************************//
    //if(mpDetectedFaces->number_of_faces ==0)
    //    mpDetectedFaces->number_of_faces = 1;
//*********************************************************//

    if(p3AHal)
    {
        if(mpDetectedFaces!=0) {
            //mpFDHalObj->halFDGetFaceResult(mpDetectedFaces, 0);
            p3AHal->setFDInfo(mpDetectedFaces);
            //mpFDHalObj->halFDGetFaceResult(mpDetectedFaces, 1);
        }
        p3AHal->destroyInstance(getNodeName());
    }

    // Do ASD
    if(mASDEnable) {
        MINT32 ASDWidth = ASD_DEFAULT_WIDTH;
        MINT32 ASDHeight = ASDWidth * srcHeight / srcWidth;
        if(ASDWidth != 320 || ASDHeight != 240) {
            MY_LOGD("The ASD Buffer size is not usual : %dx%d", ASDWidth, ASDHeight);
        }
        mASDObj->updateASD((MUINT8 *)mFDImgBuffer, ASDWidth,ASDHeight, mpDetectedFaces->number_of_faces, &mASDInfo);
    }
}


#if ENABLE_FD_ASYNC_MODE
MVOID
FdNodeImp::
setFDLock(MBOOL val)
{
    Mutex::Autolock _l(mFDLock);

    mIsFDBusy = val;
    return;
}

MVOID*
FdNodeImp::
FDHalThreadLoop(MVOID* arg)
{
    FdNodeImp *_FDNode = reinterpret_cast<FdNodeImp*>(arg);
    while(1) {
        sem_wait(&_FDNode->semFD);
        if(_FDNode->mStopFD) break;
        _FDNode->RunFaceDetection();
        _FDNode->setFDLock(MFALSE);
    }
    return NULL;
}
#endif


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FdNodeImp::
init(InitParams const& rParams)
{

    MY_LOGD("FdNode Initial !!!");
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mOpenId  = rParams.openId;
        mNodeId  = rParams.nodeId;
        mNodeName= rParams.nodeName;
    }
    // Get user log level
    char cLogLevel[PROPERTY_VALUE_MAX];
    ::property_get("debug.camera.log", cLogLevel, "0");
    mLogLevel = atoi(cLogLevel);
    if ( mLogLevel == 0 ) {
        ::property_get("debug.camera.log.fdnode", cLogLevel, "0");
        mLogLevel = atoi(cLogLevel);
    }

    mpDetectedFaces = new MtkCameraFaceMetadata;
    if ( NULL != mpDetectedFaces )
    {
        MtkCameraFace *faces = new MtkCameraFace[15];
        MtkFaceInfo *posInfo = new MtkFaceInfo[15];

        if ( NULL != faces &&  NULL != posInfo)
        {
            mpDetectedFaces->faces = faces;
            mpDetectedFaces->posInfo = posInfo;
            mpDetectedFaces->number_of_faces = 0;
        } else {
            MY_LOGE("Fail to allocate Faceinfo buffer");
            return UNKNOWN_ERROR;
        }
    } else {
        MY_LOGE("Fail to allocate FaceMetadata buffer");
        return UNKNOWN_ERROR;
    }
    mFDImgBuffer = new unsigned char[640*480*2];

    mFDWorkingBufferSize = 20971520; //20M: 1024*1024*20
    mFDWorkingBuffer = new unsigned char[mFDWorkingBufferSize];

    mpFDHalObj = halFDBase::createInstance(HAL_FD_OBJ_FDFT_SW);
    mImageWidth = 0;
    mImageHeight = 0;

    mFDSkip = FD_SKIP_NUM;

    mpDetectedGestures = new MtkCameraFaceMetadata;
    if ( NULL != mpDetectedGestures )
    {
        MtkCameraFace *faces = new MtkCameraFace[15];
        MtkFaceInfo *posInfo = new MtkFaceInfo[15];

        if ( NULL != faces &&  NULL != posInfo)
        {
            mpDetectedGestures->faces = faces;
            mpDetectedGestures->posInfo = posInfo;
            mpDetectedGestures->number_of_faces = 0;
        } else {
            MY_LOGE("Fail to allocate Gestureinfo buffer");
            return UNKNOWN_ERROR;
        }
    } else {
        MY_LOGE("Fail to allocate GSMetadata buffer");
        return UNKNOWN_ERROR;
    }

    mpGSHalObj = halGSBase::createInstance(HAL_GS_OBJ_SW);

    mASDObj->initASD(getOpenId());

    mFDProcInited = MFALSE;

    #if ENABLE_FD_ASYNC_MODE
    // +++ FD Async mode
    mIsFDBusy = MFALSE;
    mStopFD = MFALSE;

    #if 1
    mDupImage.AddrY = new unsigned char[(640*480*3)/2];
    mDupImage.AddrU = mDupImage.AddrY + 640*480;
    mDupImage.AddrV = mDupImage.AddrU + ((640*480) >> 2);
    #else
    mAllocator = IImageBufferAllocator::getInstance();
    IImageBufferAllocator::ImgParam imgParam(640*480*2,0);
    mDupImage.pImg = mAllocator->alloc("FDTempBuf", imgParam);
    if ( mDupImage.pImg.get() == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return MFALSE;
    }
    if ( !mDupImage.pImg->lockBuf( "FDTempBuf", (eBUFFER_USAGE_HW_CAMERA_READ | eBUFFER_USAGE_SW_MASK)) )
    {
        MY_LOGE("lock Buffer failed\n");
        return MFALSE;
    }
    MY_LOGE("allocator buffer : %X", mDupImage.pImg->getBufVA(0));
    mDupImage.pImg->syncCache(eCACHECTRL_INVALID);
    mDupImage.AddrY = (MUINT8 *)mDupImage.pImg->getBufVA(0);
    mDupImage.AddrU = mDupImage.AddrY + 640*480;
    mDupImage.AddrV = mDupImage.AddrU + ((640*480) >> 2);
    #endif

    sem_init(&semFD, 0, 0);
    pthread_create(&mFDHalThread, NULL, FDHalThreadLoop, this);
    // --- FD Async mode
    #endif

    mbInited = MTRUE;

    MERROR err = run();

    //
    {
        //sp<IMetadataProvider> pMetadataProvider = IMetadataProvider::create(getOpenId());
        sp<IMetadataProvider> pMetadataProvider = NSMetadataProviderManager::valueFor(getOpenId());
        if( ! pMetadataProvider.get() ) {
            MY_LOGE(" ! pMetadataProvider.get() ");
            return DEAD_OBJECT;
        }

        IMetadata static_meta = pMetadataProvider->geMtktStaticCharacteristics();
        IMetadata::IEntry active_array_entry = static_meta.entryFor(MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION);
        if( !active_array_entry.isEmpty() ) {
            mActiveArray = active_array_entry.itemAt(0, Type2Type<MRect>());
            MY_LOGD_IF(1,"FD Node: active array(%d, %d, %dx%d)",
                    mActiveArray.p.x, mActiveArray.p.y, mActiveArray.s.w, mActiveArray.s.h);
        } else {
            MY_LOGE("no static info: MTK_SENSOR_INFO_ACTIVE_ARRAY_REGION");
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
FdNodeImp::
uninit()
{
    flush();
    //
    {
        RWLock::AutoWLock _l(mConfigRWLock);
        //
        mpOutMetaStreamInfo_Result = 0;
        mpInMetaStreamInfo_Request = 0;
        mpInImageStreamInfo_Yuv = 0;
    }

    requestExit();

    join();

    #if ENABLE_FD_ASYNC_MODE
    // +++ FD Async mode
    mStopFD = MTRUE;
    sem_post(&semFD);
    pthread_join(mFDHalThread, NULL);
    sem_destroy(&semFD);
    #if 1
    if(mDupImage.AddrY != NULL) {
        delete [] mDupImage.AddrY;
        mDupImage.AddrY = NULL;
    }
    #else
    if(mDupImage.pImg != NULL && mAllocator != NULL) {
        mDupImage.pImg->unlockBuf("FDTempBuf");
        mAllocator->free(mDupImage.pImg.get());
        mDupImage.pImg = NULL;
    }
    #endif
    // --- FD Async mode
    #endif

    // Delete face metadata buffer
    if ( mpDetectedFaces != NULL )
    {
        if ( mpDetectedFaces->faces != NULL )
        {
            delete [] mpDetectedFaces->faces;
            mpDetectedFaces->faces = NULL;
        }
        if ( mpDetectedFaces->posInfo != NULL)
        {
            delete [] mpDetectedFaces->posInfo;
            mpDetectedFaces->posInfo = NULL;
        }
        delete mpDetectedFaces;
        mpDetectedFaces = NULL;
    }

    if ( mFDImgBuffer != NULL ) {
        delete [] mFDImgBuffer;
        mFDImgBuffer = NULL;
    }
    if ( mFDWorkingBuffer != NULL ) {
        delete [] mFDWorkingBuffer;
        mFDWorkingBuffer = NULL;
    }

    if ( mpFDHalObj != NULL ) {
        mpFDHalObj->halFDUninit();
        mpFDHalObj->destroyInstance();
        mpFDHalObj = NULL;
    }

    // Delete gesture metadata buffer
    if ( mpDetectedGestures != NULL )
    {
        if ( mpDetectedGestures->faces != NULL )
        {
            delete [] mpDetectedGestures->faces;
            mpDetectedGestures->faces = NULL;
        }
        if ( mpDetectedGestures->posInfo != NULL)
        {
            delete [] mpDetectedGestures->posInfo;
            mpDetectedGestures->posInfo = NULL;
        }
        delete mpDetectedGestures;
        mpDetectedGestures = NULL;
    }

    if(mpGSHalObj != NULL) {
        mpGSHalObj->halGSUninit();
        mpGSHalObj->destroyInstance();
        mpGSHalObj = NULL;
    }

    mASDObj->uninitASD();

    mImageWidth = 0;
    mImageHeight = 0;

    if(mFDProcInited) {
        if(mpSensorListener != NULL)
        {
            MY_LOGD("mpSensorListener uninit");
            mpSensorListener->disableSensor(SensorListener::SensorType_Acc);
            mpSensorListener->destroyInstance();
            mpSensorListener = NULL;
        }
        mFDProcInited = MFALSE;
    }

    mbInited = MFALSE;
    MY_LOGD("FDnode uninit finish");
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FdNodeImp::
config(ConfigParams const& rParams)
{
    if  ( ! rParams.pInAppMeta.get() ) {
        return BAD_VALUE;
    }
    //if  ( ! rParams.pInHalMeta.get() ) {
    //    return BAD_VALUE;
    //}
    if  ( ! rParams.pOutAppMeta.get() ) {
        return BAD_VALUE;
    }
    //if  ( ! rParams.pOutHalMeta.get() ) {
    //    return BAD_VALUE;
    //}
//    if  (  0 == rParams.vInImage.size() ) {
//        return BAD_VALUE;
//    }
    //if  (  0 == rParams.vOutImage.size() ) {
    //    return BAD_VALUE;
    //}
    //
    //
    RWLock::AutoWLock _l(mConfigRWLock);
    //
    mpInMetaStreamInfo_Request = rParams.pInAppMeta;
    mpInImageStreamInfo_Yuv    = rParams.vInImage;
    mpOutMetaStreamInfo_Result = rParams.pOutAppMeta;
    //
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FdNodeImp::
queue(
    android::sp<IPipelineFrame> pFrame
)
{
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
    mRequestQueueCond.broadcast();
    //
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FdNodeImp::
waitForRequestDrained()
{
    //
    Mutex::Autolock _l(mRequestQueueLock);
    if( !mbRequestDrained ) {
        MY_LOGD("wait for request drained");
        mbRequestDrainedCond.wait(mRequestQueueLock);
    }
    //
}


/******************************************************************************
 *
 ******************************************************************************/
MERROR
FdNodeImp::
flush()
{
    MY_LOGD_IF(mLogLevel > 0, "+");
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
    }
    //
    // 2. wait enque thread
    waitForRequestDrained();
    //
    // 3. clear working buffer

    MY_LOGD_IF(mLogLevel > 0, "-");

    //return INVALID_OPERATION;
    return OK;
}

/******************************************************************************
 *
 ******************************************************************************/
MERROR
FdNodeImp::
onDequeRequest(
    android::sp<IPipelineFrame>& rpFrame
)
{
    Mutex::Autolock _l(mRequestQueueLock);

    //
    //  Wait until the queue is not empty or this thread will exit.

    mbDoFD = MFALSE;

    while ( mRequestQueue.empty() && ! exitPending() )
    {
        // set dained flag
        mbRequestDrained = MTRUE;
        mbRequestDrainedCond.signal();
        //
        mbDoFD = MTRUE;
        status_t status = mRequestQueueCond.wait(mRequestQueueLock);

        if  ( OK != status ) {
            MY_LOGW(
                "wait status:%d:%s, mRequestQueue.size:%zu, exitPending:%d",
                status, ::strerror(-status), mRequestQueue.size(), exitPending()
            );
        }
    }

    //
    if  ( exitPending() ) {
        MY_LOGW("[exitPending] mRequestQueue.size:%zu", mRequestQueue.size());
        return DEAD_OBJECT;
    }
    if  (
            mRequestQueue.size() == 1
#if (FD_SKIP_NUM>0)
        &&  mFDSkip >= FD_SKIP_NUM
#endif
        )
    {
        mbDoFD = MTRUE;
        mFDSkip = 0;
    }
    else
    {
        mFDSkip++;
    }
    //
    //  Here the queue is not empty, take the first request from the queue.
    mbRequestDrained = MFALSE;
    rpFrame = *mRequestQueue.begin();
    mRequestQueue.erase(mRequestQueue.begin());
    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
// Ask this object's thread to exit. This function is asynchronous, when the
// function returns the thread might still be running. Of course, this
// function can be called from a different thread.
void
FdNodeImp::
requestExit()
{
    MY_LOGD_IF(mLogLevel > 0, "+");
    Mutex::Autolock _l(mRequestQueueLock);

    Thread::requestExit();

    mRequestQueueCond.signal();
    MY_LOGD_IF(mLogLevel > 0, "-");
}


/******************************************************************************
 *
 ******************************************************************************/
// Good place to do one-time initializations
status_t
FdNodeImp::
readyToRun()
{
    ::prctl(PR_SET_NAME, (unsigned long)"Cam@FdNodeImp", 0, 0, 0);
    //
    //mi4ThreadId = ::gettid();
    //  thread policy & priority
    //  Notes:
    //      Even if pthread_create() with SCHED_OTHER policy, a newly-created thread
    //      may inherit the non-SCHED_OTHER policy & priority of the thread creator.
    //      And thus, we must set the expected policy & priority after a thread creation.
    //MY_LOGW("Not Implement !!! Need set thread priority & policy");

    int const policy    = SCHED_OTHER;
    int const priority  = 0;
    //
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value
    sched_setscheduler(0, policy, &sched_p);
    //setpriority(PRIO_PROCESS, 0, priority);
    ::sched_getparam(0, &sched_p);
    //
    //
    MY_LOGD(
        "Tid: %d, policy: %d, priority: %d"
        , ::gettid(), ::sched_getscheduler(0)
        , sched_p.sched_priority
    );

    return OK;
}


/******************************************************************************
 *
 ******************************************************************************/
bool
FdNodeImp::
threadLoop()
{
    sp<IPipelineFrame> pFrame;
    if  (
            OK == onDequeRequest(pFrame)
        &&  pFrame != 0
        )
    {
        onProcessFrame(pFrame);
        return true;
    }

    MY_LOGD("FDnode exit threadloop");

    return  false;
}


/******************************************************************************
 *
 ******************************************************************************/
MINT32
FdNodeImp::
doBufferAnalysis(int BufWidth, int BufHeight, int plane)
{
    if( (BufWidth==640) && (BufHeight == 480) && (plane==2))
        return 0;
    if( (BufWidth==640) && (plane==3))
        return 10;
    else if ( (BufWidth*3 == BufHeight*4) && (plane==2) )
        return 1;
    else if ( (BufWidth*3 == BufHeight*4) && (plane==3) )
        return 2;
    else if ( ( BufWidth*9 == BufHeight*16) && (plane==2) )
        return 3;
    else if ( ( BufWidth*9 == BufHeight*16) && (plane==3) )
        return 4;
    else if ( ( BufWidth*3 == BufHeight*5) && (plane==2) )
        return 5;
    else if ( ( BufWidth*3 == BufHeight*5) && (plane==3) )
        return 6;
    else if ( (plane==2) )
        return 3;
    else if ( (plane==3) )
        return 4;
    else
        return 9;

    return plane;

}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
FdNodeImp::
doGestureDetect(
    MUINT8* pImage,
    MINT32 RotateInfo
)
{
    if(mGDEnable) {
        if(mpGSHalObj == NULL || mpDetectedFaces == NULL || mpDetectedGestures == NULL || pImage == NULL)
            return -1;
        mpGSHalObj->halGSDo(pImage, RotateInfo);
    }
    return 0;
}

/******************************************************************************
 *
 ******************************************************************************/
MINT32
FdNodeImp::
getGestureResult(
)
{
    MINT32 GD_Result;
    if(mGDEnable) {
        #if ENABLE_FD_ASYNC_MODE
        Mutex::Autolock _l(mResultLock);
        #endif
        if(mpDetectedFaces == NULL || mpDetectedGestures == NULL)
            return -1;
        GD_Result = mpGSHalObj->halGSGetGestureResult(mpDetectedGestures);
        /////////////////////////////////////////////////////////////////////
        // cpchen: filter GS results with FD results: no gesture inside face regions
        /////////////////////////////////////////////////////////////////////
        bool bEnableGSFilterWithFD = true;
        float fIntersetAreaRatio = 0.25f;
        float fMaxRelativeRatio = 3.0f;
        if( bEnableGSFilterWithFD )
        {
           int newCount = 0;
           for (int gi = 0; gi < mpDetectedGestures->number_of_faces; ++gi)
           {
              // gesture rectangle
              int gx0 = mpDetectedGestures->faces[gi].rect[0];
              int gy0 = mpDetectedGestures->faces[gi].rect[1];
              int gx1 = mpDetectedGestures->faces[gi].rect[2];
              int gy1 = mpDetectedGestures->faces[gi].rect[3];
              int garea = (gx1 - gx0) * (gy1 - gy0);

              bool bIsOverlap = false;
              for (int fi = 0; fi < mpDetectedFaces->number_of_faces; ++fi)
              {
                 // face rectangle
                 int fx0 = mpDetectedFaces->faces[fi].rect[0];
                 int fy0 = mpDetectedFaces->faces[fi].rect[1];
                 int fx1 = mpDetectedFaces->faces[fi].rect[2];
                 int fy1 = mpDetectedFaces->faces[fi].rect[3];
                 int farea = (fx1 - fx0) * (fy1 - fy0);

                 // interset rectangle
                 int ix0 = max(gx0, fx0);
                 int iy0 = max(gy0, fy0);
                 int ix1 = min(gx1, fx1);
                 int iy1 = min(gy1, fy1);
                 int iarea = 0;
                 if ((ix1 > ix0) && (iy1 > iy0))
                    iarea = (ix1 - ix0) * (iy1 - iy0);

                 // overlap determination
                 float minArea = min(garea, farea);
                 float overlapRatio = (float)iarea / minArea;
                 float relativeRatio = (float)farea / garea;

                 if (overlapRatio >= fIntersetAreaRatio)
                 {
                    bIsOverlap = true;
                    break;
                 }
              } // end of for each face rectangle

              // skip overlapped gesture rectangles, move non-overlapped gesture rectangles forward
              if (!bIsOverlap)
              {
                 mpDetectedGestures->faces[newCount].rect[0] = mpDetectedGestures->faces[gi].rect[0];
                 mpDetectedGestures->faces[newCount].rect[1] = mpDetectedGestures->faces[gi].rect[1];
                 mpDetectedGestures->faces[newCount].rect[2] = mpDetectedGestures->faces[gi].rect[2];
                 mpDetectedGestures->faces[newCount].rect[3] = mpDetectedGestures->faces[gi].rect[3];
                 mpDetectedGestures->faces[newCount].score = mpDetectedGestures->faces[gi].score;
                 mpDetectedGestures->faces[newCount].id = mpDetectedGestures->faces[gi].id;
                 mpDetectedGestures->faces[newCount].left_eye[0] = mpDetectedGestures->faces[gi].left_eye[0];
                 mpDetectedGestures->faces[newCount].left_eye[1] = mpDetectedGestures->faces[gi].left_eye[1];
                 mpDetectedGestures->faces[newCount].right_eye[0] = mpDetectedGestures->faces[gi].right_eye[0];
                 mpDetectedGestures->faces[newCount].right_eye[1] = mpDetectedGestures->faces[gi].right_eye[1];
                 mpDetectedGestures->faces[newCount].mouth[0] = mpDetectedGestures->faces[gi].mouth[0];
                 mpDetectedGestures->faces[newCount].mouth[1] = mpDetectedGestures->faces[gi].mouth[1];
                 mpDetectedGestures->posInfo[newCount].rop_dir = mpDetectedGestures->posInfo[gi].rop_dir;
                 mpDetectedGestures->posInfo[newCount].rip_dir = mpDetectedGestures->posInfo[gi].rip_dir;
                 ++newCount;
              }
           }
           // number of gesture rectangles after filtering
           mpDetectedGestures->number_of_faces = newCount;
           GD_Result = newCount;

           // debug message
           if (GD_Result == 0)
              MY_LOGD("Scenario GD: Gesture detected but filtered out by face!!!");
        }
        /////////////////////////////////////////////////////////////////////

        /////////////////////////////////////////////////////////////////////
        // cpchen: face is a prerequiste of gesture shot, no face no gesture shot
        /////////////////////////////////////////////////////////////////////
        bool bEnableGSPrerequisteWithFD = true;
        if (bEnableGSPrerequisteWithFD && mpDetectedFaces->number_of_faces == 0)
        {
           mpDetectedGestures->number_of_faces = 0;
           GD_Result = 0;

           // debug message
           MY_LOGD("Scenario GD: Gesture detected but no face!");
        }
        /////////////////////////////////////////////////////////////////////

        MY_LOGD("Scenario GD Result: %d",GD_Result );
    }
    return 0;
}

/******************************************************************************
*
******************************************************************************/
MINT32
FdNodeImp::
ASDImp::
updateASD(MUINT8* pRGBImg, MINT32 BufWidth, MINT32 BufHeight, MINT32 FaceNum, ASDInfo* Result)
{
    #warning [FixME][error:] Need Porting ASD!!!!!
    #if 0
    MUINT32 u4Scene = 0;
    ASDInfo_T ASDInfo;
    MUINT32 ASDWidth = BufWidth;
    MUINT32 ASDHeight = BufHeight;

    if (mASDWorkingBuffer == NULL || mpASDHalObj == NULL) {
        MY_LOGE("The ASD module is not init");
        return -1;
    }

    mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_3, mFDNodeID, LOG_TAG);
    //mpHal3A->getASDInfo(ASDInfo);
    mpHal3A->send3ACtrl(E3ACtrl_GetAsdInfo, (MINTPTR)(&ASDInfo), 0);
    if (mpHal3A)
    {
        mpHal3A->destroyInstance(LOG_TAG);
    }
    if(!mHalInited) {

        mpASDHalObj->mHalAsdInit((void*)&ASDInfo, mASDWorkingBuffer, (mSensorType==SENSOR_TYPE_RAW)?0:1, ASDWidth/2, ASDHeight/2);
        mHalInited = 1;
    }
    mpASDHalObj->mHalAsdDecider((void*)&ASDInfo, FaceNum ,mSceneCur);

    Result->Scene = mSceneCur;

    mpASDHalObj->mHalAsdDoSceneDet((void*)pRGBImg, ASDWidth, ASDHeight);
    #endif

    return 0;
}


/******************************************************************************
*
******************************************************************************/
MINT32
FdNodeImp::
ASDImp::
initASD(MINT32 id)
{
    #warning [FixME][error:] Need Porting ASD!!!!!
    #if 0
    //Please fixed me
    mSensorType = SENSOR_TYPE_RAW;
    //Create Working Buffer
    //mASDWorkingBuffer = new unsigned char(MHAL_ASD_WORKING_BUF_SIZE);
    mASDWorkingBuffer = (MUINT8*)malloc(MHAL_ASD_WORKING_BUF_SIZE);
    if(mASDWorkingBuffer == NULL)
    {
        MY_LOGW("memory is not enough");
        return -1;
    }
    mASDWorkingBufferSize = MHAL_ASD_WORKING_BUF_SIZE;

    mpASDHalObj = halASDBase::createInstance(HAL_ASD_OBJ_AUTO);

    mHalInited = 0;

    mFDNodeID = id;

    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;
    #endif
    return 0;
}

/******************************************************************************
*
******************************************************************************/
MINT32
FdNodeImp::
ASDImp::
uninitASD()
{
    if (mpASDHalObj != NULL)
    {
        mpASDHalObj->mHalAsdUnInit();
        mpASDHalObj->destroyInstance();
        mpASDHalObj = NULL;
    }
    if (mASDWorkingBuffer != NULL) {
        free(mASDWorkingBuffer);
        mASDWorkingBuffer = NULL;
    }
    mASDWorkingBufferSize = 0;

    mHalInited = 0;

    mSceneCur = mhal_ASD_DECIDER_UI_AUTO;
    return 0;
}



/******************************************************************************
 *
 ******************************************************************************/
MVOID
FdNodeImp::
ReturnFDResult(
    IMetadata*              pOutMetadataResult,
    IMetadata*              pInpMetadataRequest
)
{
    MRect cropRegion;
    MRect face_rect;
    MINT32 face_landmark;
    MINT32 face_id;
    MUINT8 face_score;

    #if ENABLE_FD_ASYNC_MODE
    Mutex::Autolock _l(mResultLock);
    #endif

    if(mpDetectedFaces->number_of_faces == 0)
        return;
    IMetadata::IEntry entry = pInpMetadataRequest->entryFor(MTK_SCALER_CROP_REGION);
    if( !entry.isEmpty() ) {
        cropRegion = entry.itemAt(0, Type2Type<MRect>());
    }
    else
        MY_LOGW("GetCropRegion Fail!");

    MY_LOGD_IF(mLogLevel > 0, "CropRegion: p.x:%d, p.y:%d, s.w:%d, s.h:%d, ",cropRegion.p.x, cropRegion.p.y, cropRegion.s.w, cropRegion.s.h);

    //Push_back Rectangle (face_rect)
    IMetadata::IEntry face_rect_tag(MTK_STATISTICS_FACE_RECTANGLES);
    for(int i=0; i<mpDetectedFaces->number_of_faces;i++)
    {
        //Hard code, Fixme
        //face_rect.p.x = (mpDetectedFaces->faces[i].rect[0]+1000) * 2560/2000;  //Left
        //face_rect.p.y = (mpDetectedFaces->faces[i].rect[1]+1000) * 1920/2000;  //Top
        //face_rect.s.w = (mpDetectedFaces->faces[i].rect[2]+1000) * 2560/2000;  //Right
        //face_rect.s.h = (mpDetectedFaces->faces[i].rect[3]+1000) * 1920/2000;  //Bottom
        face_rect.p.x = ((mpDetectedFaces->faces[i].rect[0]+1000) * cropRegion.s.w/2000) + cropRegion.p.x;  //Left
        face_rect.p.y = ((mpDetectedFaces->faces[i].rect[1]+1000) * cropRegion.s.h/2000) + cropRegion.p.y;  //Top
        face_rect.s.w = ((mpDetectedFaces->faces[i].rect[2]+1000) * cropRegion.s.w/2000) + cropRegion.p.x;  //Right
        face_rect.s.h = ((mpDetectedFaces->faces[i].rect[3]+1000) * cropRegion.s.h/2000) + cropRegion.p.y;  //Bottom
        mpDetectedFaces->faces[i].id = -1;
        mpDetectedFaces->faces[i].left_eye[0] = 0;
        mpDetectedFaces->faces[i].left_eye[1] = 0;
        mpDetectedFaces->faces[i].right_eye[0] = 0;
        mpDetectedFaces->faces[i].right_eye[1] = 0;
        mpDetectedFaces->faces[i].mouth[0] = 0;
        mpDetectedFaces->faces[i].mouth[1] = 0;
        mpDetectedFaces->faces[i].score = 100;
        face_rect_tag.push_back(face_rect, Type2Type<MRect>());
    }
    pOutMetadataResult->update(MTK_STATISTICS_FACE_RECTANGLES, face_rect_tag);

    //Push_back Landmark (face_landmark)
    IMetadata::IEntry face_landmark_tag(MTK_STATISTICS_FACE_LANDMARKS);
    for(int i=0; i<mpDetectedFaces->number_of_faces;i++)
    {
        face_landmark = (mpDetectedFaces->faces[i].left_eye[0]);  //left_eye_x
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
        face_landmark = (mpDetectedFaces->faces[i].left_eye[1]);  //left_eye_y
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
        face_landmark = (mpDetectedFaces->faces[i].right_eye[0]);  //right_eye_x
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
        face_landmark = (mpDetectedFaces->faces[i].right_eye[1]);  //right_eye_y
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
        face_landmark = (mpDetectedFaces->faces[i].mouth[0]);  //mouth_x
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
        face_landmark = (mpDetectedFaces->faces[i].mouth[1]);  //mouth_y
        face_landmark_tag.push_back(face_landmark, Type2Type<MINT32>());
    }
    pOutMetadataResult->update(MTK_STATISTICS_FACE_LANDMARKS, face_landmark_tag);

    //Push_back IDs
    IMetadata::IEntry face_id_tag(MTK_STATISTICS_FACE_IDS);
    for(int i=0; i<mpDetectedFaces->number_of_faces;i++)
    {
        face_id = mpDetectedFaces->faces[i].id;
        face_id_tag.push_back(face_id, Type2Type<MINT32>());
    }
    pOutMetadataResult->update(MTK_STATISTICS_FACE_IDS, face_id_tag);

    //Push_back Score
    IMetadata::IEntry face_score_tag(MTK_STATISTICS_FACE_SCORES);
    for(int i=0; i<mpDetectedFaces->number_of_faces;i++)
    {
        face_score = mpDetectedFaces->faces[i].score;
        face_score_tag.push_back(face_score, Type2Type<MUINT8>());
    }
    pOutMetadataResult->update(MTK_STATISTICS_FACE_SCORES, face_score_tag);

    //Check Metadata Content
    IMetadata::IEntry entry_check = pOutMetadataResult->entryFor(MTK_STATISTICS_FACE_RECTANGLES);
    MRect checkRect = entry_check.itemAt(0, Type2Type<MRect>());

    //printf("Check: p.x:%d, p.y:%d, s.w:%d, s.h:%d, \n",checkRect.p.x, checkRect.p.y, checkRect.s.w, checkRect.s.h);

}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
FdNodeImp::
onProcessFrame(
    android::sp<IPipelineFrame> const& pFrame
)
{

    MY_LOGD_IF(mLogLevel > 0, "[onProcessFrame] In FrameNo : %d", pFrame->getFrameNo());
    StreamId_T const        streamIdOutMetaResult       = mpOutMetaStreamInfo_Result->getStreamId();
    sp<IMetaStreamBuffer>   pOutMetaStreamBufferResult  = NULL;
    IMetadata*              pOutMetadataResult          = NULL;
    //
    StreamId_T const        streamIdInpMetaRequest      = mpInMetaStreamInfo_Request->getStreamId();
    sp<IMetaStreamBuffer>   pInpMetaStreamBufferRequest = NULL;
    IMetadata*              pInpMetadataRequest         = NULL;
    //
    StreamId_T const        streamIdInpImageYuv         = mpInImageStreamInfo_Yuv->getStreamId();
    sp<IImageStreamBuffer>  pInpImageStreamBufferYuv    = NULL;
    sp<IImageBufferHeap>    pInpImageBufferHeapYuv      = NULL;
    sp<IImageBuffer>        pInpImageBufferYuv          = NULL;
    //
    IStreamInfoSet const& rStreamInfoSet= pFrame->getStreamInfoSet();
    IStreamBufferSet& rStreamBufferSet  = pFrame->getStreamBufferSet();
    //
    MINT32 success = 0;
    MINT32 SDEn = 0;
    MINT32 GDEn = 0;
    MINT32 RotateInfo = 0;
    MINT32 ASDEn = 0;

    if(!mbInited)
        goto lbExit;

    if(!mFDProcInited) {
        onInitFDProc();
        mFDProcInited = MTRUE;
    }

    //
    ////////////////////////////////////////////////////////////////////////////
    //  Ensure buffers available.
    ////////////////////////////////////////////////////////////////////////////
    //  Output Meta Stream: Result
    {
        StreamId_T const streamId = streamIdOutMetaResult;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pOutMetaStreamBufferResult
        );
        //Should check the returned error code!!!
        if( err != OK ) {
            MY_LOGW("cannot get output meta: streamId %#"PRIxPTR" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }
    //
    //  Input Meta Stream: Request
    {
        StreamId_T const streamId = streamIdInpMetaRequest;
        MERROR const err = ensureMetaBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInpMetaStreamBufferRequest
        );
        //Should check the returned error code!!!
        if( err != OK ) {
            MY_LOGW("cannot get input meta: streamId %#"PRIxPTR" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }
    //
    //  Input Image Stream: YUV
    {
        StreamId_T const streamId = streamIdInpImageYuv;
        MERROR const err = ensureImageBufferAvailable_(
            pFrame->getFrameNo(),
            streamId,
            rStreamBufferSet,
            pInpImageStreamBufferYuv
        );
        //Should check the returned error code!!!
        if( err != OK ) {
            MY_LOGW("cannot get input YUV: streamId %#"PRIxPTR" of frame %d",
                    streamId,
                    pFrame->getFrameNo());
            goto lbExit;
        }
    }

    success = 1;

    ////////////////////////////////////////////////////////////////////////////
    //  Prepare buffers before using.
    ////////////////////////////////////////////////////////////////////////////
    {
        //  Output Meta Stream: Result
        {
            pOutMetadataResult = pOutMetaStreamBufferResult->tryWriteLock(getNodeName());
        }
        //
        //  Input Meta Stream: Request
        {
            pInpMetadataRequest = pInpMetaStreamBufferRequest->tryReadLock(getNodeName());
        }
        //
        //  Input Image Stream: YUV
        {
            //  Query the group usage.
            MUINT const groupUsage = pInpImageStreamBufferYuv->queryGroupUsage(getNodeId());
            //
            pInpImageBufferHeapYuv = pInpImageStreamBufferYuv->tryReadLock(getNodeName());
            pInpImageBufferYuv = pInpImageBufferHeapYuv->createImageBuffer();
            //pInpImageBufferYuv->lockBuf(getNodeName(), groupUsage);

#if 0 //workaround for group usage
            pInpImageBufferYuv->lockBuf(getNodeName(), groupUsage);
#else
            MUINT const usage = eBUFFER_USAGE_SW_READ_OFTEN |
                eBUFFER_USAGE_HW_CAMERA_READWRITE
                ;
             MY_LOGW_IF(mLogLevel > 0, "[FIXME] lock usage %p", usage);
             pInpImageBufferYuv->lockBuf(getNodeName(), usage);
#endif
        }
    }

//*********************************************************************//
#if (0)
       MY_LOGD("Dump1! \n");
    static int Count=0;
    Count++;
    if((Count%3) ==0)
    //if(1)
    {
       char szFileName[100];
       //sprintf(szFileName, "data/fd_data/640x480_YY_%d.raw", Count);
       sprintf(szFileName, "/sdcard/640x480_Y1_%d.raw", Count);

       FILE * pRawFp = fopen(szFileName, "wb");
       if (NULL == pRawFp )
       {
           MY_LOGD("Can't open file to save RAW Image\n");
           while(1);
       }
       int i4WriteCnt = fwrite((void *)pInpImageBufferYuv->getBufVA(0),1, (640*480),pRawFp);
       fflush(pRawFp);
       fclose(pRawFp);

    }
#endif
//*********************************************************************//

    //****************************************************//
    //Test
    //SDEn =  1; //Pass
    //GDEn =  1; //Pass
    //ASDEn = 1; //Pass
    //****************************************************//
    mSDEnable = SDEn;
    mGDEnable = GDEn;
    mASDEnable = ASDEn;

#if ENABLE_FD_ASYNC_MODE
    if(!mIsFDBusy) {
        mDupImage.w = pInpImageBufferYuv->getImgSize().w;
        mDupImage.h = pInpImageBufferYuv->getImgSize().h;
        mDupImage.planes = pInpImageBufferYuv->getPlaneCount();
        if(mDupImage.planes == 3) {
            memcpy(mDupImage.AddrY, (void *)pInpImageBufferYuv->getBufVA(0), pInpImageBufferYuv->getImgSize().w*pInpImageBufferYuv->getImgSize().h);
            memcpy(mDupImage.AddrU, (void *)pInpImageBufferYuv->getBufVA(1), (pInpImageBufferYuv->getImgSize().w*pInpImageBufferYuv->getImgSize().h) >> 2);
            memcpy(mDupImage.AddrV, (void *)pInpImageBufferYuv->getBufVA(2), (pInpImageBufferYuv->getImgSize().w*pInpImageBufferYuv->getImgSize().h) >> 2);
        } else if (mDupImage.planes == 1) {
            memcpy(mDupImage.AddrY, (void *)pInpImageBufferYuv->getBufVA(0), pInpImageBufferYuv->getImgSize().w*pInpImageBufferYuv->getImgSize().h*2);

        } else if(mDupImage.planes == 2) {
            MY_LOGW("FD node could not be here, not support buffer plane == 2");

        }
        setFDLock(MTRUE);
        sem_post(&semFD);
    }
#else
    ////////////////////////////////////////////////////////////////////////////
    //  Access buffers.
    ////////////////////////////////////////////////////////////////////////////
    if (mbDoFD){
        mDupImage.w = pInpImageBufferYuv->getImgSize().w;
        mDupImage.h = pInpImageBufferYuv->getImgSize().h;
        mDupImage.planes = pInpImageBufferYuv->getPlaneCount();
        mDupImage.AddrY = (MUINT8 *)pInpImageBufferYuv->getBufVA(0);
        mDupImage.AddrU = (MUINT8 *)pInpImageBufferYuv->getBufVA(1);
        mDupImage.AddrV = (MUINT8 *)pInpImageBufferYuv->getBufVA(2);

        RunFaceDetection();
    }
#endif

    ReturnFDResult(pOutMetadataResult, pInpMetadataRequest);

    ////////////////////////////////////////////////////////////////////////////
    //  Release buffers after using.
    ////////////////////////////////////////////////////////////////////////////
lbExit:
    //
    //  Output Meta Stream: Result
    {
        StreamId_T const streamId = streamIdOutMetaResult;
        //
        if  ( pOutMetaStreamBufferResult != NULL )
        {
            //Buffer Producer must set this status.
            pOutMetaStreamBufferResult->markStatus(
                success ?
                STREAM_BUFFER_STATUS::WRITE_OK :
                STREAM_BUFFER_STATUS::WRITE_ERROR
            );
            pOutMetaStreamBufferResult->unlock(getNodeName(), pOutMetadataResult);
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }
    //
    //  Input Meta Stream: Request
    {
        StreamId_T const streamId = streamIdInpMetaRequest;
        //
        if  ( pInpMetaStreamBufferRequest != NULL )
        {
            pInpMetaStreamBufferRequest->unlock(getNodeName(), pInpMetadataRequest);
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }
    //
    //  Input Image Stream: YUV
    {
        StreamId_T const streamId = streamIdInpImageYuv;
        //
        if  ( pInpImageStreamBufferYuv != NULL )
        {
            pInpImageBufferYuv->unlockBuf(getNodeName());
            pInpImageStreamBufferYuv->unlock(getNodeName(), pInpImageBufferHeapYuv.get());
            //
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::USED |
                IUsersManager::UserStatus::RELEASE
            );
        }
        else
        {
            rStreamBufferSet.markUserStatus(
                streamId, getNodeId(),
                IUsersManager::UserStatus::RELEASE
            );
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    //  Apply buffers to release.
    ////////////////////////////////////////////////////////////////////////////
    rStreamBufferSet.applyRelease(getNodeId());

    ////////////////////////////////////////////////////////////////////////////
    //  Dispatch
    ////////////////////////////////////////////////////////////////////////////
    onDispatchFrame(pFrame);
}


/******************************************************************************
 *
 ******************************************************************************/
#if (1)
sp<FdNode>
FdNode::
createInstance()
{
    return new FdNodeImp();

}

#endif
