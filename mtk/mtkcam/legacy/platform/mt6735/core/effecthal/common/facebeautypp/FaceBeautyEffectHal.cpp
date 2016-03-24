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

#define LOG_TAG "FaceBeautyEffectHal"
//#include <cutils/xlog.h>
#include <cutils/log.h>
#include "FaceBeautyEffectHal.h"
#include <mtkcam/sdk/hal/IFaceBeautySDKHal.h>

/******************************************************************************
 *
 ******************************************************************************/
#define FUNCTION_LOG_START      ALOGD("[%s] - E.", __FUNCTION__)
#define FUNCTION_LOG_END        ALOGD("[%s] - X.", __FUNCTION__)


using namespace NSCam;
using namespace android;

bool
FaceBeautyEffectHal::
allParameterConfigured() 
{
    FUNCTION_LOG_START;
    //@todo implement this    
    FUNCTION_LOG_END;
    return true;
}


status_t
FaceBeautyEffectHal::
initImpl() 
{
    FUNCTION_LOG_START;
    //@todo implement this
    //mpListener = NULL;
    ALOGD("[%s] +", __FUNCTION__);
    mpParams.SmoothLevel     = 0;
    mpParams.SkinColor       = 0;
    mpParams.EnlargeEyeLevel = 0;
    mpParams.SlimFaceLevel   = 0;

    mpInputFrame = NULL;
    mpOutputFrame = NULL;
    mpSDKFBshot  = NULL;

    mpisabort = MFALSE;

    mpSDKFBshot = new SDK_facebeauty();

    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceBeautyEffectHal::
uninitImpl() 
{
    FUNCTION_LOG_START;
    //@todo implement this
    ALOGD("[%s] +", __FUNCTION__);
    delete  mpSDKFBshot;
    mpSDKFBshot = NULL;
    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}


//non-blocking
status_t           
FaceBeautyEffectHal::
prepareImpl() 
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}


status_t
FaceBeautyEffectHal::
releaseImpl() 
{
    FUNCTION_LOG_START;
    //@todo implement this
    FUNCTION_LOG_END;
    return OK;
}

status_t
FaceBeautyEffectHal::
getNameVersionImpl(EffectHalVersion &nameVersion) const
{
    FUNCTION_LOG_START;
    //@todo implement this
    nameVersion.effectName = "FaceBeauty";
    nameVersion.major = 1;
    nameVersion.minor = 0;
    FUNCTION_LOG_END;
    return OK;
}


status_t           
FaceBeautyEffectHal::
getCaptureRequirementImpl(EffectParameter *inputParam, Vector<EffectCaptureRequirement> &requirements) const 
{   
    FUNCTION_LOG_START;
    //@todo implement this
    ALOGD("[%s] +", __FUNCTION__);
    requirements.clear();
    requirements.setCapacity(1);
    EffectCaptureRequirement req;
    char str[32];
    ALOGD("[%s] picture size %dx%d",__FUNCTION__,mParameter->getInt("picture-width"),mParameter->getInt("picture-height") );
    sprintf(str, "%dx%d", mParameter->getInt("picture-height"), mParameter->getInt("picture-width"));
    req.set("picture-size", str);
    req.set("picture-format", eImgFmt_YV12);
    requirements.push_back(req);
    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}


status_t           
FaceBeautyEffectHal::
setParameterImpl(android::String8 &key, android::String8 &object) 
{
    FUNCTION_LOG_START;
    //@todo implement this

    //mpParams.SmoothLevel     = parameter->getInt("fb-smooth-level");
    //mpParams.SkinColor       = parameter->getInt("fb-skin-color");
    //mpParams.EnlargeEyeLevel = parameter->getInt("fb-enlarge-eye");
	//mpParams.SlimFaceLevel   = parameter->getInt("fb-slim-face");

    FUNCTION_LOG_END;
    return OK;
}

status_t           
FaceBeautyEffectHal::
setParametersImpl(android::sp<EffectParameter> parameter) 
{
    FUNCTION_LOG_START;
    ALOGD("[%s] +", __FUNCTION__);
    parameter->dump();
    mParameter = parameter;
    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}

status_t           
FaceBeautyEffectHal::
getFaceBeautyBeautifiedPosition(Vector<SDK_FACE_BEAUTY_POS>& pos)
{
    ALOGD("[%s] +", __FUNCTION__);

    mParameter->dump();
    char* p = (char*) mParameter->get("fb-face-position");
    if (p == NULL)
    {
        ALOGD("getFaceBeautyBeautifiedPosition fail");
        return MFALSE;
    }
	
    char* end = p + strlen(p);
    char* q;
    SDK_FACE_BEAUTY_POS xy;
    while(p < end) {
        xy.left= strtol(p,&q,10);
        p = q+1;
        xy.top= strtol(p,&q,10);
        p = q+1;
        xy.right= strtol(p,&q,10);
        p = q+1;
        xy.button= strtol(p,&q,10);
        p = q+1;
        pos.push_back(xy);
    }

    ALOGD("[%s] -", __FUNCTION__);

    return OK;

}

status_t           
FaceBeautyEffectHal::
startImpl(uint64_t *uid) 
{
    FUNCTION_LOG_START;
    //@todo implement this
    ALOGD("[%s] +", __FUNCTION__);
    mpisabort = MFALSE;
    mpParams.SmoothLevel     = mParameter->getInt("fb-smooth-level");
    mpParams.SkinColor       = mParameter->getInt("fb-skin-color");
    mpParams.SlimFaceLevel   = mParameter->getInt("fb-slim-face");
    mpParams.Rotation        = mParameter->getInt("rotation");
	mpParams.JPGquality      = mParameter->getInt("jpeq_quality");
	
    mpSDKFBshot->init();
    setCallback();	
    //setCallback();	
    //CaptureFaceBeautySDK_init();

    //EffectResult result;
    //EffectParameter parameter;
    //result.set("onCompleted", 1);
    //startDone(result, parameter, OK);   //@todo call this after completed
    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}


status_t           
FaceBeautyEffectHal::
abortImpl(EffectResult &result, EffectParameter const *parameter) 
{
    FUNCTION_LOG_START;
    //@todo implement this
    ALOGD("[%s] +", __FUNCTION__);
    mpisabort = MTRUE;
    mpSDKFBshot->uninit();

    if(mpInputFrame != NULL)
    {
      ALOGD("[%s] release mpInputFrame", __FUNCTION__);
      mpInputFrame->unlockBuf( "addInputFrame");
      mpInputFrame = NULL;
    }

    if(mpOutputFrame != NULL)
    {
      ALOGD("[%s] release mpOutputFrame", __FUNCTION__);
      mpOutputFrame->unlockBuf( "addInputFrame");
      mpOutputFrame = NULL;
    }

    if(mOutputParameter != NULL)
    {
      ALOGD("[%s] release mOutputParameter", __FUNCTION__);
      mOutputParameter = NULL;
    }

    if(mInputParameter != NULL)
    {
      ALOGD("[%s] release mInputParameter", __FUNCTION__);
      mInputParameter = NULL;
    }

    if(mpRequest != NULL)
    {
      ALOGD("[%s] release mpRequest", __FUNCTION__);
      mpRequest = NULL;
    }

    //mpInputFrame->unlockBuf( "addInputFrame");
    //mpOutputFrame->unlockBuf( "addOutputFrame");
    //mpOutputFrame = NULL;
    //mpInputFrame = NULL; 
    //mOutputParameter = NULL;
    //mInputParameter = NULL;
    //CaptureFaceBeautySDK_uninit();
    result.set("onAborted", 1);
    ALOGD("[%s] -", __FUNCTION__);
    FUNCTION_LOG_END;
    return OK;
}

status_t           
FaceBeautyEffectHal::
doEffect()
{
    ALOGD("[%s] +", __FUNCTION__);
    Vector<SDK_FACE_BEAUTY_POS> mSDKFacePos;
    mSDKFacePos.clear();
    getFaceBeautyBeautifiedPosition(mSDKFacePos);

    if (!mpSDKFBshot->CaptureFaceBeauty_apply(mpInputFrame.get(), mpOutputFrame.get(), &mpParams, mSDKFacePos, (EImageFormat) mpInputFrame->getImgFormat()))
        return UNKNOWN_ERROR;
    ALOGD("[%s] -", __FUNCTION__);
    return OK;
}

status_t           
FaceBeautyEffectHal::
setCallback()
{
    mpSDKFBshot->SetCallBack(this);
    return OK;
}

status_t           
FaceBeautyEffectHal::
FBSDKCbFunc()
{
    ALOGD("[%s] +", __FUNCTION__);

    mpInputFrame->unlockBuf( "addInputFrame");
    mpOutputFrame->unlockBuf( "addOutputFrame");
    mpOutputFrame = NULL;
    mpInputFrame = NULL;
    mOutputParameter = NULL;
    mInputParameter = NULL;

    sp<EffectResult> iFrameResult = new EffectResult();
    sp<EffectResult> oFrameResult = new EffectResult();
    sp<EffectResult> requestResult = new EffectResult();
    android::sp<EffectFrameInfo> iFrameInfo = mpRequest->vInputFrameInfo.valueAt(0);
    android::sp<EffectFrameInfo> oFrameInfo = mpRequest->vOutputFrameInfo.valueAt(0);
    // sp<EffectResult> iFrameResult = iFrameInfo->getFrameResult();
    // sp<EffectResult> oFrameResult = oFrameInfo->getFrameResult();
    // sp<EffectResult> requestResult = mpRequest->getFrameResult();

    iFrameResult->set("addInputFrameImpl", "1");
    iFrameInfo->setFrameResult(iFrameResult);
    iFrameInfo->mpOnFrameProcessed(iFrameInfo->mpTag, iFrameInfo);


    oFrameResult->set("addOutputFrameImpl", "1");
    oFrameInfo->setFrameResult(oFrameResult);
    oFrameInfo->mpOnFrameProcessed(oFrameInfo->mpTag, oFrameInfo);


    if(mpisabort)
    {
        ALOGD("[%s] AP will call abort before doing CFB", __FUNCTION__);
        mpisabort = MFALSE;

    } else {
        ALOGD("[%s] call start done after doing CFB", __FUNCTION__);

        requestResult->set("onCompleted", 1);
        mpRequest->setRequestResult(requestResult);
        mpRequest->mpOnRequestProcessed(mpRequest->mpTag, String8("onCompleted"), mpRequest);
    }

    ALOGD("[%s] -", __FUNCTION__);

    return OK;
}



status_t
FaceBeautyEffectHal::
updateEffectRequestImpl(const android::sp<EffectRequest> request)
{
    FUNCTION_LOG_START;
    status_t ret = OK;
    mpRequest = request;
    if (request->vOutputFrameInfo.size() == 0 || request->vInputFrameInfo.size() == 0)
    {
        return OK;
    }
    android::sp<EffectFrameInfo> oFrameInfo = request->vOutputFrameInfo.valueAt(0);
    android::sp<EffectFrameInfo> iFrameInfo = request->vInputFrameInfo.valueAt(0);
    if (oFrameInfo != NULL && iFrameInfo != NULL)
    {
        ALOGD("[%s] oFrameInfo != NULL && iFrameInfo != NULL", __FUNCTION__);
        ret = iFrameInfo->getFrameBuffer(mpInputFrame);
        ret = oFrameInfo->getFrameBuffer(mpOutputFrame);
        if (mpOutputFrame != NULL && mpInputFrame != NULL)
        {

            if ((mpInputFrame->getImgFormat() != eImgFmt_I422) && 
                (mpInputFrame->getImgFormat() != eImgFmt_YV12) && 
                (mpInputFrame->getImgFormat() != eImgFmt_BLOB))
            {
                ALOGE("[%s] Bad value", __FUNCTION__);
                return BAD_VALUE;
            }
            if ((mpOutputFrame->getImgFormat() != eImgFmt_I422) && 
                (mpOutputFrame->getImgFormat() != eImgFmt_YV12) && 
                (mpOutputFrame->getImgFormat() != eImgFmt_BLOB))
            {
                ALOGE("[%s] Bad value", __FUNCTION__);
                return BAD_VALUE;
            }


            mOutputParameter = oFrameInfo->getFrameParameter();
            mInputParameter = iFrameInfo->getFrameParameter();
            ALOGD("[%s] mpOutputFrame != NULL && mpInputFrame != NULL", __FUNCTION__);
            FUNCTION_LOG_END;
            return doEffect();
        }
    }


    FUNCTION_LOG_END;
    return OK;
}
