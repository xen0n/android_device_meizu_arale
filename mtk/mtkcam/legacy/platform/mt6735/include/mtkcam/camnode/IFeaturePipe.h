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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_FEATUREPIPE_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_FEATUREPIPE_H_
//
#include "mtkcam/iopipe/PostProc/IHalPostProcPipe.h"
#warning "[FIXME] daniel.wu: v1-only interface?"
//#include <v1/hal/adapter/inc/CamUtils.h>
#include <mtkcam/common.h>
#include <mtkcam/v1/camutils/CamInfo.h>


using namespace android;

/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSPostProc {

#define FEATURE_MASK_CLEAR(x)           (x)=0
// Affecting node graph
#define FEATURE_MASK_ENABLE_VFB(x)      (x)|=(1<<0)
#define FEATURE_MASK_IS_VFB_ENABLED(x)  ((x&(1<<0))?true:false)
#define FEATURE_MASK_ENABLE_EIS(x)      (x)|=(1<<1)
#define FEATURE_MASK_IS_EIS_ENABLED(x)  ((x&(1<<1))?true:false)
#define FEATURE_MASK_ENABLE_3DNR(x)     (x)|=(1<<2)
#define FEATURE_MASK_IS_3DNR_ENABLED(x) ((x&(1<<2))?true:false)
#define FEATURE_MASK_ENABLE_STEREO_CAMERA(x)        (x)|=(1<<3)
#define FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(x)    ((x&(1<<3))?true:false)
// Not affecting node graph
#define FEATURE_MASK_ENABLE_GESTURE_SHOT(x)     (x)|=(1<<16)
#define FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(x) ((x&(1<<16))?true:false)
#define FEATURE_MASK_ENABLE_VHDR(x)             (x)|=(1<<17)
#define FEATURE_MASK_IS_VHDR_ENABLED(x)         ((x&(1<<17))?true:false)


typedef struct
{
    MINT32 SmoothLevel;
    MINT32 SkinColor;
    MINT32 EnlargeEyeLevel;
    MINT32 SlimFaceLevel;
    MBOOL  ExtremeBeauty; //1:enable warp, 1 face only; 0:no warp, multiple face
    MINT32 Rotation;
} FeaturePipeTuningInfo;

typedef enum
{
    MAX_INPUT_IMAGE_SIZE,
    RECORDING_HINT,
} FeaturePipeSetType;


/******************************************************************************
 *
 * @class IFeaturePipe
 * @brief Post-Processing Pipe Interface for Feature Stream.
 * @details
 * The data path will be Mem --> ISP--XDP --> Mem.
 *
 ******************************************************************************/
class IFeaturePipe : public virtual IHalPostProcPipe
{
public:     ////                    Instantiation.

    /**
     * @brief Create the instance
     *
     * @details
     *
     * @note
     *
     * @return
     * An instance to this class.
     *
     */
    static  IFeaturePipe*        createInstance(char const* szCallerName,MUINT32 openedSensorIndex);

public:     ////                    Attributes.

    /**
     * @brief Query pipe capability.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    static  MBOOL                   queryCapability(IMetadata& rCapability);

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    static  char const*             pipeName() { return "PostProc::FeaturePipe"; }

    /**
     * @brief get the pipe name
     *
     * @details
     *
     * @note
     *
     * @return
     * A null-terminated string indicating the name of the pipe.
     *
     */
    virtual char const*             getPipeName() const { return pipeName(); }

public:

    /**
     * @brief Configure features.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setFeature(MUINT32 featureMask) = 0;

    /**
     * @brief Tuning per frame.
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setTuning(FeaturePipeTuningInfo& tuningInfo) = 0;

    /**
     * @brief User touch X-Y coordination
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   setTouch(MINT32 touchX, MINT32 touchY) = 0;

    /**
     * @brief Set parameters
     *
     * @details
     *
     * @note
     *
     * @return
     * - MTRUE indicates success; MFALSE indicates failure.
     */
    virtual MBOOL                   set(FeaturePipeSetType setType, const MINTPTR setData) = 0;

    /**
     * @brief Set camera message-callback information.
     *
     * @details
     *
     * @note
     *
     * @return
     */
    virtual void                    setCallbacks(sp<CamMsgCbInfo> const& rpCamMsgCbInfo) = 0;

    virtual void                    enableMsgType(int32_t msgType) = 0;

    virtual void                    disableMsgType(int32_t msgType) = 0;

};


/******************************************************************************
 *
 ******************************************************************************/
};  //namespace NSPostProc
};  //namespace NSIoPipe
};  //namespace NSCam
#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_FEATUREIO_FEATUREPIPE_H_

