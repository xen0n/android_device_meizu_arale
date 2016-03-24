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

#ifndef _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_EFFECTREQUEST_H_
#define _MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_EFFECTREQUEST_H_

#include <utils/String8.h>
#include <utils/Vector.h>
#include <utils/RefBase.h>
#include <utils/KeyedVector.h>
#include <utils/Errors.h>
#include <mmsdk/BasicParameters.h>

namespace NSCam
{
using namespace android;

typedef int64_t frame_no;

class EffectFrameInfo: public LightRefBase<EffectFrameInfo>
{
public:
    typedef MVOID           (*PFN_CALLBACK_T)(MVOID* tag, sp<EffectFrameInfo>& frame);

    EffectFrameInfo(
            MUINT32             _reqNo = 0,
            MUINT32             _frameNo = 0,
            PFN_CALLBACK_T      _cb = NULL,
            MVOID*              _tag = NULL
    )
            : mRequestNo(_reqNo)
            , mFrameNo(_frameNo)
            , mpOnFrameProcessed(_cb)
            , mpTag(_tag)
            , mIsFrameReady(0)
            , mFrame(NULL)
            , mpFrameParameter(NULL)
            , mpFrameResult(NULL)

    {
    }


    ///< copy constructor
    EffectFrameInfo(const EffectFrameInfo& other);

    ///< operator constructor
    EffectFrameInfo&    operator=(const EffectFrameInfo& other);

    bool                        isFrameBufferReady();
    status_t                    getFrameBuffer(sp<IImageBuffer>& frame);
    sp<EffectParameter>   getFrameParameter();
    sp<EffectResult>      getFrameResult();

    status_t                    setFrameBuffer(sp<IImageBuffer> frame);
    status_t                    setFrameParameter(sp<EffectParameter> result);
    status_t                    setFrameResult(sp<EffectResult> result);
    status_t                    setFrameReady(bool ready);

    /**
     *  @brief          get Frame number of this EffectFrameInfo object
     *  @return         Frame number
     */
    frame_no                     getFrameNo() const {
                                    return mFrameNo;
                                }
    /**
     *  @brief          get Request number of this EffectFrameInfo object of EffectRequest object
     *  @return         Request number
     */
    MUINT32                     getRequestNo() const {
                                    return mRequestNo;
                                }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //e.g. mpOnRequestProcessed(mpTag, this);
    PFN_CALLBACK_T              mpOnFrameProcessed;
    // callback tag; it shouldn't be modified by the client(user).
    // e.g. the tag may be a pointer to clinet(this).
    MVOID*                      mpTag;

private:
    frame_no                    mFrameNo;
    MUINT32                     mRequestNo;
    bool                        mIsFrameReady;
    sp<IImageBuffer>            mFrame;

    sp<EffectParameter>         mpFrameParameter;
    sp<EffectResult>            mpFrameResult;


};



/**
 *  @brief EffectRequest is the basic class to passing data from client to effectHal.
 *         The client can be a EffectHalClinet or a Pipeline Node.
 *
 */
class EffectRequest : public RefBase
{
public:     //EffectRequest

    typedef MVOID           (*PFN_CALLBACK_T)(MVOID* tag, String8 status, sp<EffectRequest>& request);//completed, failed
    EffectRequest(
            MUINT32             _reqNo = 0,
            PFN_CALLBACK_T      _cb = NULL,
            MVOID*              _tag = NULL
        )
            : mRequestNo(_reqNo)
            , mpOnRequestProcessed(_cb)
            , mpTag(_tag)
            , mpRequestParameter(NULL)
            , mpRequestResult(NULL)

        {
        }


    ///< copy constructor
    EffectRequest(const EffectRequest& other);

    ///< operator constructor
    EffectRequest&              operator=(const EffectRequest& other);

    /**
     *  @brief          get Request number of this EffectRequest object
     *  @return         request number
     */
    MUINT32                     getRequestNo() const {
                                    return mRequestNo;
                                }

    /**
     *  @brief          get the pointer of EffectParameter object that keep all input paramters related to this EffectRequest
     *  @param[in]      parameter pointer to the EffectParameter object
     *  @return         sp<EffectParameter>
     */
    const sp<EffectParameter>   getRequestParameter();
    /**
     *  @brief          get the pointer of EffectResult object that keep all output results related to this EffectRequest
     *  @param[in]      result pointer to the EffectResult object
     *  @return         status_t
     */
    const sp<EffectResult>      getRequestResult();
    /**
     *  @brief          set the pointer of EffectParameter object that keep all input results related to this EffectRequest
     *  @param[in]      result pointer to the EffectResult object
     *  @return         status_t
     */
    status_t                    setRequestParameter(sp<EffectParameter> parameter);
    /**
     *  @brief          set the pointer of EffectResult object that keep all output results related to this EffectRequest
     *  @param[in]      result pointer to the EffectResult object
     *  @return         status_t
     */
    status_t                    setRequestResult(sp<EffectResult> result);


    /*
    MVOID*                      getTag() const { return mpTag; }
    status_t                    setTag(MVOID* tag){ mpTag = tag; }
    */


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Variables.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    DefaultKeyedVector<frame_no, sp<EffectFrameInfo> > vInputFrameInfo;
    DefaultKeyedVector<frame_no, sp<EffectFrameInfo> > vOutputFrameInfo;
    // Vector<EffectFrameInfo> vInputFrameInfo;
    // Vector<EffectFrameInfo> vOutputFrameInfo;

    //e.g. mpOnRequestProcessed(mpTag, "completed", this);
    PFN_CALLBACK_T          mpOnRequestProcessed;
    // callback tag; it shouldn't be modified by the client(user).
    // e.g. the tag may be a pointer to clinet(this of node or effectHalClinet).
    MVOID*                  mpTag;

private:
    MUINT32                 mRequestNo;
    sp<EffectParameter>     mpRequestParameter;
    sp<EffectResult>        mpRequestResult;

};
} //end namespace NSCam


#endif  //_MTK_PLATFORM_HARDWARE_INCLUDE_MTKCAM_SDK_HAL_EFFECTREQUEST_H_
