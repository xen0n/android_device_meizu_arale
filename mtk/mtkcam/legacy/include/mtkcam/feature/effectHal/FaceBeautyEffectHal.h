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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V1_ADAPTER_SCENARIO_SHOT_FACEBEAUTY_EFFECTHAL_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V1_ADAPTER_SCENARIO_SHOT_FACEBEAUTY_EFFECTHAL_H_
#include <IEffectHal.h>
#include <EffectHalBase.h>
#include <mtkcam/sdk/hal/IFaceBeautySDKHal.h>
#include "SDKFaceBeauty.h"
//#include <utils/Vector.h>
#include <vector>

namespace NSCam {
class SDK_facebeauty;
};


/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
class FaceBeautyEffectHal : public EffectHalBase
{
public:
                                FaceBeautyEffectHal() {}
                                ~FaceBeautyEffectHal() {}
public: //EffectHalBase
    virtual bool allParameterConfigured();
    
    virtual android::status_t   initImpl();
    virtual android::status_t   uninitImpl();
    //non-blocking
    virtual android::status_t   prepareImpl();
    virtual android::status_t   releaseImpl();

    virtual android::status_t   getNameVersionImpl(EffectHalVersion &nameVersion) const;
    virtual android::status_t   getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const;
    virtual android::status_t   setParameterImpl(android::String8 &key, android::String8 &object);
    virtual android::status_t   setParametersImpl(android::sp<EffectParameter> parameter);
    virtual android::status_t   startImpl(uint64_t *uid=NULL);
    virtual android::status_t   abortImpl(EffectResult &result, EffectParameter const *parameter=NULL);
    virtual android::status_t   doEffect();
    virtual android::status_t   setCallback();
    virtual android::status_t   FBSDKCbFunc();
    virtual android::status_t   getFaceBeautyBeautifiedPosition(Vector<SDK_FACE_BEAUTY_POS>& pos);

    virtual android::status_t   updateEffectRequestImpl(const android::sp<EffectRequest> request);
public: //debug
public: //autotest
private:
    FACE_BEAUTY_SDK_HAL_PARAMS  mpParams;
    const EffectListener*       mpListener;
    sp<EffectRequest>           mpRequest;
    sp<IImageBuffer>            mpInputFrame;
    sp<IImageBuffer>            mpOutputFrame;
    android::sp<EffectParameter> mOutputParameter;
    android::sp<EffectParameter> mInputParameter;
    android::sp<EffectParameter> mParameter;
    SDK_facebeauty* mpSDKFBshot;
    MBOOL mpisabort;
};

}   //namespace NSCam {

#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_V1_ADAPTER_SCENARIO_SHOT_FACEBEAUTY_EFFECTHAL_H_
