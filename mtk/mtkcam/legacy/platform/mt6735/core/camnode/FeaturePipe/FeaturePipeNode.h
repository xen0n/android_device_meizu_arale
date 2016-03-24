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

#ifndef _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPENODE_H_
#define _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPENODE_H_
//

#include "FeaturePipe.h"
#include <mtkcam/hwutils/CameraProfile.h>

//#define _UNITTEST
//#define _DEBUG_DUMP

using namespace NSCamNode;
using namespace NSIoPipe;
using namespace NSPostProc;
using namespace CPTool;

class DpIspStream;

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode{

typedef struct
{
    ImgRequest   imgReq;
    MBOOL        vfbExtreme;
} TagImgRequest;

typedef struct
{
    ImgRequest   imgReq;
    MUINT32      mFrameNo;
    NodeDataTypes dataType;
} FeaturePipeNodeImgReqJob_s;

typedef struct
{
    TagImgRequest tagImgReq;
    MUINT32      mFrameNo;
} FeaturePipeNodeTagImgReqJob_s;

typedef struct
{
    MUINT32 w;
    MUINT32 h;
    MUINT32 FrameNo;
    MINT32  DirtyBufPA;
    MINT32  DirtyBufVA;
} DirtyBufInfo_s;

typedef enum
{
    NR3D_ERROR_NONE                = 0x00,     // No error.
    NR3D_ERROR_INVALID_GMV         = 0x01,     // GMV is invalid due to drop frame or something.
    NR3D_ERROR_GMV_TOO_LARGE       = 0x02,     // GMV X or Y is larger than a certain value.
    NR3D_ERROR_DROP_FRAME          = 0x04,     // Drop frame.
    NR3D_ERROR_FRAME_SIZE_CHANGED  = 0x08,     // Current frame size is not the same as previous frame. E.g. during DZ.
    NR3D_ERROR_FORCE_SKIP          = 0x10,     // Force skip by user, probably by adb command.
} NR3D_ERROR_ENUM;

typedef enum
{
    NR3D_STATE_STOP         = 0x00,     // NR3D, IMG3O, VIPI all disabled.
    NR3D_STATE_PREPARING    = 0x01,     // IMG3O enabled, to output current frame for next frame use. NR3D, VIPI disabled. (When NR3D is disable, IMG30 output original image without any process.)
    NR3D_STATE_WORKING      = 0x02,     // NR3D, IMG3O, VIPI all enabled.
} NR3D_STATE_ENUM;

#define FEATURE_ON_MASK_BIT_VFB         (0x1)
#define FEATURE_ON_MASK_BIT_EIS         (0x2)
#define FEATURE_ON_MASK_BIT_3DNR        (0x4)

#define SCHED_POLICY                    (SCHED_NORMAL)
//#define SCHED_PRIORITY                  (0)
#define SCHED_PRIORITY                  ANDROID_PRIORITY_FOREGROUND // should match pass2

// mkdbg: debug p2athread timing
//#define FORCE_P2A_MKDBG

#define EIS_IMAGE_SCALE_UP_WIDTH(w)     ((w) * 6 / 5)
#define EIS_IMAGE_SCALE_UP_HEIGHT(h)    ((h) * 6 / 5)

#define FULL_IMAGE_MAX_WIDTH            2304     //(1920 * 6 / 5)  , this is for EIS limitation
#define FULL_IMAGE_MAX_HEIGHT           1296     //(1080 * 6 / 5)  , this is for EIS limitation

//#define NR3D_WORKING_BUFF_WIDTH         (2304 * 6 / 5)  // (896)
//#define NR3D_WORKING_BUFF_HEIGHT        (2304 * 6 / 5)  // (1344 * 6 / 5)   // (512)
#define NR3D_WORKING_BUFF_WIDTH         2752     //(1920 * 6 / 5)  , this is for EIS limitation
#define NR3D_WORKING_BUFF_HEIGHT        2752     //(1080 * 6 / 5)  , this is for EIS limitation


#define P2A_DSIMG_BUFFER_NUM            2
#define P2A_FULLIMG_BUFFER_NUM          3
#define P2A_3DNR_PINGPONG_BUF_NUM       2
/******************************************************************************
 *
 * @class FeaturePipeP2aNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeP2aNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeP2aNode*    createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

public:

    MBOOL enqueWrapper(
            MUINT32 const data,
            MUINTPTR const buf,
            MUINT32 const ext = 0
            );

    MBOOL dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs);

    MBOOL handleP2Done(QParams& rParams);

    MBOOL notifyGraphNodes(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2);
    void  getGraphicBuffer(vector< sp<GraphicBuffer>* >& vpGB);
    MBOOL set(FeaturePipeSetType setType, const MINTPTR setData);
    MBOOL startVideoRecord(MINT32 wd,MINT32 ht,MINT32 fps);
    MBOOL stopVideoRecord();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeP2aNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            MUINT32 mOpenedSensorIndex,
            FeaturePipe* pFeaturePipeObj
            );

    static MVOID p2aCbFunc(QParams& rParams);

    DECLARE_ICAMTHREADNODE_INTERFACES()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    INormalStream*   mpINormalStream;
    IFeatureStream*  mpIFeatureStream;
    MSize            mMaxInputImageSize;
    queue<QParams>   mqJob;
    queue<FeaturePipeNodeImgReqJob_s> m3dnrPrvFrmQueue;
    //QStaData         mQStaData;
    NS3A::IHal3A*    mp3AHal;
    EisHal*    mpEisHalFor3DNR;
    VHdrHal *mpVHdrObj;
    sem_t            mModuleSem;
    queue< QParams > qCallerQParams;

    MUINT32          mPrevFrameWidth;       // Keep track of previous frame width.
    MUINT32          mPrevFrameHeight;      // Keep track of previous frame height.
    MINT32           mNmvX;
    MINT32           mNmvY;
    MUINT32          mCmvX;                 // Current frame CMV X.
    MUINT32          mCmvY;                 // Current frame CMV Y.
    MUINT32          mPrevCmvX;             // Keep track of previous CMV X.
    MUINT32          mPrevCmvY;             // Keep track of previous CMV Y.
    MINT32           m3dnrGainZeroCount;
    MUINT32          m3dnrErrorStatus;
    NR3D_STATE_ENUM  m3dnrStateMachine;

    NSCam::NSIoPipe::NSPostProc::NR3D    *mpNr3dParam;  // For NR3D struct in PostProc.
    NSCam::NSIoPipe::NSPostProc::StaData *mpFeoParam;   // For FEO struct in PostProc.
    NSCam::NSIoPipe::NSPostProc::StaData *mpLceiParam;  // For LCEI struct in PostProc.

    queue<void*>     mqCookie;
    queue<QParams::PFN_CALLBACK_T> mqfnCallback;
    queue< QParams > mqCallbackQParams;
    sem_t            mCallbackSem;
    sp<GraphicBuffer> mFullImgGB[P2A_FULLIMG_BUFFER_NUM];
};

/******************************************************************************
 *
 * @class FeaturePipeSwitcherNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeSwitcherNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeSwitcherNode* createInstance(MUINT32 featureOnMask, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

public:
    MVOID setVfbExtreme(MBOOL mode);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeSwitcherNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            FeaturePipe* pFeaturePipeObj
            );

    DECLARE_ICAMTHREADNODE_INTERFACES()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MBOOL            mVfbExtreme;
    Mutex            mVfbExtremeLock;
    MUINT32          mGpuProcCount;
    queue<MBOOL>     mqGpuToP2b;
    queue<FeaturePipeNodeImgReqJob_s> mqJobFromP2a;
    queue<FeaturePipeNodeImgReqJob_s> mqJobFromGpu;
};

#define GPU_NODE_BUFFER_NUM  3
/******************************************************************************
 *
 * @class FeaturePipeGpuNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeGpuNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeGpuNode*    createInstance(MUINT32 featureOnMask, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

public:

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeGpuNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            FeaturePipe* pFeaturePipeObj);

    DECLARE_ICAMTHREADNODE_INTERFACES()

protected:    //// thread function

    MBOOL threadInit();         //init in thread
    MBOOL threadUninit();       //uninit in thread

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MTKWarp*         mpGpuWarp;
    MUINT32          mInputImageWidth;
    MUINT32          mInputImageHeight;
    MUINT32          mOutputImageWidth;
    MUINT32          mOutputImageHeight;
    MUINT32          mWarpWorkBufSize;
    MUINT8*          mpWorkingBuffer;
    sp<GraphicBuffer> mGfxYuvBuffer;
    sp<GraphicBuffer> mGfxRgbaBuffer[GPU_NODE_BUFFER_NUM];
    queue<FeaturePipeNodeImgReqJob_s> mqJobFullImg;
    queue<FeaturePipeNodeImgReqJob_s> mqJobWarpMap;
    sem_t            mModuleSem;

    void*            mpCpuWarpInputBuffer;//TBD
    void*            mpCpuWarpOutputBuffer;//TBD
};

#define MDP_NODE_BUFFER_NUM     3
/******************************************************************************
 *
 * @class FeaturePipeMdpNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeMdpNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeMdpNode*    createInstance(MUINT32 featureOnMask, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

public:

    MBOOL dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeMdpNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            FeaturePipe* pFeaturePipeObj);

    DECLARE_ICAMTHREADNODE_INTERFACES()

    MINT32 convertRGBA8888(IImageBuffer* srcIImageBuffer, IImageBuffer* dstIImageBuffer);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    MUINT32          mImageWidth;
    MUINT32          mImageHeight;
    queue<FeaturePipeNodeImgReqJob_s> mqFullImgJob;
    queue<QParams>   mqQParamsJob;
    queue<QParams>   mqPostProcQParams;
    sem_t            mModuleSem;
    DpBlitStream     mDpBlit;
    DpIspStream*     mpStream;
};

/******************************************************************************
 *
 * @class FeaturePipeP2bNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeP2bNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeP2bNode*    createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

public:

    MBOOL handleP2Done(QParams& rParams);

    MBOOL dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeP2bNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            MUINT32 mOpenedSensorIndex,
            FeaturePipe* pFeaturePipeObj
            );

    static MVOID p2bCbFunc(QParams& rParams);

    DECLARE_ICAMTHREADNODE_INTERFACES()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    sem_t            mModuleSem;
    IFeatureStream*  mpIFeatureStream;
    queue<QParams>   mqQParamsJob;
    queue<FeaturePipeNodeImgReqJob_s> mqDsImgJob;
    queue<FeaturePipeNodeImgReqJob_s> mqAlphaCLJob;
    queue<FeaturePipeNodeImgReqJob_s> mqAlphaNRJob;
    queue<FeaturePipeNodeImgReqJob_s> mqPCAJob;
    queue<FeaturePipeNodeImgReqJob_s> mqFullImgJob;
    NS3A::IHal3A*    mp3AHal;
    queue< QParams > qCallerQParams;
    queue< QParams > mqCallbackQParams;
};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //_MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPENODE_H_

