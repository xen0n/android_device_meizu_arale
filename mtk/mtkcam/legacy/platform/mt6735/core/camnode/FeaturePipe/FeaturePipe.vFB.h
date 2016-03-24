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

#ifndef _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_VFB_H_
#define _MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_VFB_H_
//


#define VFB_DS_IMAGE_MAX_WIDTH      320
#define VFB_DS_IMAGE_MAX_HEIGHT     320

#include <gs_hal_base.h>

namespace NSCamNode{

#define DEFAULT_SMOOTH_LEVEL        5
#define DEFAULT_SKIN_COLOR          5
#define DEFAULT_ENLARGE_EYE_LEVEL   5
#define DEFAULT_SLIM_FACE_LEVEL     5
#define DEFAULT_EXTREME_BEAUTY      true

typedef struct {
    MtkCameraFace    mFaceInfo[15];
    MtkFaceInfo      mPoseInfo[15];
    MtkCameraFaceMetadata mFaceMetadata;
} VFB_NODE_FACE_METADATA;

#define VFB_NODE_VFB_PARAMS_BUFFER_NUM      3
#define VFB_NODE_WARP_MAP_BUFFER_NUM        2

/******************************************************************************
 *
 * @class FeaturePipeVfbNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeVfbNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeVfbNode*      createInstance(MUINT32 featureOnMask, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

    virtual MBOOL                   getDSImgSize(MSize& srcImgSize, MSize& dsImgSize);

    virtual MBOOL                   setTuning(FeaturePipeTuningInfo& tuningInfo);
    virtual MBOOL                   setTouch(MINT32 touchX, MINT32 touchY);
    virtual MBOOL                   set(FeaturePipeSetType setType, const MINTPTR setData);

    virtual MVOID                   setPCA(IImageBuffer* pImageBuffer);
    virtual MVOID                   setLCE(IImageBuffer* pImageBuffer);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeVfbNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            FeaturePipe* pFeaturePipeObj);

    DECLARE_ICAMTHREADNODE_INTERFACES()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    halVFBBase*      mpVfbHal;
    MUINT8*          mpWorkingBuffer;
    queue<FeaturePipeNodeTagImgReqJob_s> mqJobFullImg;
    queue<FeaturePipeNodeImgReqJob_s> mqJobDsImg;
    queue<FeaturePipeNodeImgReqJob_s> mqJobFaceMetadata;
    queue<FeaturePipeNodeImgReqJob_s> mqJobWarpMap;
    MINT32*          mpClearWarpMapX;
    MINT32*          mpClearWarpMapY;
    MINT32           mSmoothLevel;
    MINT32           mSkinColor;
    MINT32           mEnlargeEyeLevel;
    MINT32           mSlimFaceLevel;
    MBOOL            mExtremeBeauty;
    MINT32           mTouchX;
    MINT32           mTouchY;
    MBOOL            mRecordingHint;
};


#define FD_NODE_FACE_INFO_NUM       2

/******************************************************************************
 *
 * @class FeaturePipeFdNode
 * @brief
 * @details
 *
 *
 ******************************************************************************/
class
FeaturePipeFdNode : public FeaturePipeNode
{
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  IPipe Interface.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////                    Instantiation.

    static FeaturePipeFdNode*       createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe* pFeaturePipeObj);
    virtual MVOID                   destroyInstance();

    virtual MBOOL                   setRotation(MINT32 rotation);

private:
    virtual MINT32                  doFaceDetection(IImageBuffer* pImageBuffer, MtkCameraFaceMetadata *faceMetadata);
    virtual MINT32                  doGestureShot(IImageBuffer* pImageBuffer);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:   ////     to be implemented by subclass

    FeaturePipeFdNode(
            const char* name,
            eThreadNodeType type,
            int policy,
            int priority,
            MUINT32 featureOnMask,
            MUINT32 mOpenedSensorIndex,
            FeaturePipe* pFeaturePipeObj);

    DECLARE_ICAMTHREADNODE_INTERFACES()

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    halFDBase*       mpFDHalObj;
    MBOOL            mFDInited;
    halGSBase*       mpGDHalObj;
    MUINT8*          mpFDWorkingBuffer;
    MUINT8*          mFDVGABuffer;
    MUINT8*          mFDYBuffer;
    queue<FeaturePipeNodeImgReqJob_s> mqJobDsImg;
    MINT32           mRotation;
    NS3A::IHal3A*    mp3AHal;
    MUINT32          mFDWidth;
    MUINT32          mFDHeight;
    MtkCameraFace    mGSFaceInfo[15];
    MtkFaceInfo      mGSPoseInfo[15];
    MtkCameraFaceMetadata mGSFaceMetadata;
};
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
#endif  //_MTK_PLATFORM_HARDWARE_CORE_FEATUREIO_PIPE_FEATUREPIPE_VFB_H_

