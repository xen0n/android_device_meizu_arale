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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FACE_EFFECT_HAL_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FACE_EFFECT_HAL_H_
#include <IEffectHal.h>
#include <EffectHalBase.h>

#include <pthread.h>
#include <utils/List.h>
#include <utils/threads.h>

#include <mtkcam/common/faces.h>
#include <mtkcam/featureio/fd_hal_base.h>
// GS
#include <mtkcam/featureio/gs_hal_base.h>
// ASD
#include <mtkcam/featureio/asd_hal_base.h>
#include <mtkcam/hal/sensor_hal.h>

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {

struct FaceImage {
    int w;
    int h;
    MUINT8* AddrY;
    MUINT8* AddrU;
    MUINT8* AddrV;
    MUINT8* PAddrY;
    MINT32 format;
    MINT32 planes;
    sp<IImageBuffer> pImg;
};

typedef enum
{
    FaceHalState_Created = 0,
    FaceHalState_Inited,
    FaceHalState_Prepared,
    FaceHalState_Started,    
    FaceHalState_Unknown
} FaceHalState_T;
class FaceEffectHal : public EffectHalBase
{
public:
                                FaceEffectHal() {}
    //virtual                     ~FaceEffectHal() {}
    virtual                     ~FaceEffectHal();
public: //EffectHalBase
    typedef android::sp<EffectRequest>   Request_T;
    virtual bool allParameterConfigured();
    
    virtual android::status_t   initImpl();
    virtual android::status_t   uninitImpl();
    //non-blocking
    virtual android::status_t   prepareImpl();
    virtual android::status_t   releaseImpl();

    virtual android::status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const;
    virtual android::status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
//    virtual android::status_t   getCaptureRequirementImpl(EffectCaptureRequirement  & requirement) const;
    virtual android::status_t   setParameterImpl(android::String8 &key, android::String8 &object);
    virtual android::status_t   startImpl(uint64_t *uid=NULL);
    virtual android::status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL);
    //non-blocking
    virtual android::status_t   updateEffectRequestImpl(const android::sp<EffectRequest> request);
    virtual MBOOL               isParameterValid(const char *param);
    virtual android::status_t   setParametersImpl(android::sp<EffectParameter> parameter);
    
    
private:
    // internal function
    MVOID                       ReturnResult(Request_T &rFrame, MtkCameraFaceMetadata info, int SD_Detected, int GD_Detected, int Scene);
    MVOID                       RunFaceEffect(Request_T &rFrameIn);
    MBOOL                       onDequeue(Request_T &rFrameIn);
    static MVOID*               onThreadLoop(MVOID* arg);
    MBOOL                       onFlush();
    MINT32                      getGestureResult(MtkCameraFaceMetadata *pGestureInfo, MtkCameraFaceMetadata *pFaceInfo, int FDEnable);
    MINT32                      updateASD(MUINT8* pRGBImg, MINT32 BufWidth, MINT32 BufHeight, MINT32 FaceNum, mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM &Scene, void *ASDInfo);

private:
    //const EffectListener*       mpListener;
    // Thread Control +
    pthread_t           mFaceThread;
    List<Request_T>     mInputQ;
    mutable Mutex       mFrameQueueLock;
    mutable Mutex       mFDRunningLock;
    Condition           mInputQueueCond;
    // Thread Control -
    bool                mStop;
    bool                mDoFD;
    
    BasicParameters mFaceEffectHalParam;
    android::String8 mSupportedParameters;
    sp<EffectResult>    mResult;
    //IImageBufferAllocator*          mAllocator;
    
    mutable Mutex       mFDLock;
    mutable Mutex       mResultLock;
    struct FaceImage    mDupImage;
    int                 mImageWidth;
    int                 mImageHeight;
    int                 mFDEnable;
    int                 mSDEnable;
    int                 mPrevSD;
    MINT32              mSD_Result;
    MUINT8*             mPureYBuf;
    MUINT8*             mFTImgBuffer;
    
    // feature pipe
    halFDBase*          mpFDHalObj;
    unsigned char*      mFDWorkingBuffer;
    int                 mFDWorkingBufferSize;
    MtkCameraFaceMetadata           mFaceInfo;
    // ASD
    halASDBase*         mpASDHalObj;
    mhal_ASD_DECIDER_UI_SCENE_TYPE_ENUM             mSceneCur;
    MUINT32             mHDR;
    MUINT8*             mASDWorkingBuffer;
    MUINT32             mASDWorkingBufferSize;
    MUINT32             mSensorType;
    MINT32              mASDHalInited;
    int                 mASDEnable;
    // GD_Detected
    halGSBase*          mpGSHalObj;
    MtkCameraFaceMetadata           mGestureInfo;
    int                 mGDEnable;
    
    int                 mFDSDUsed;
    int                 mGDUsed;
    int                 mASDUsed;
};

}   //namespace NSCam {

#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FACE_EFFECT_HAL_H_
