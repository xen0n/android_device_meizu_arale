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
#define LOG_TAG "MtkCam/EncNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;

#include <mtkcam/v1/config/PriorityDefs.h>

#include <queue>
using namespace std;

#include <utils/Mutex.h>
using namespace android;
//
#include <mtkcam/camnode/JpegEncNode.h>
#include <mtkcam/iopipe/SImager/ISImager.h>
using namespace NSIoPipe::NSSImager;
//
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

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "JpgEnc"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_JPEG)

/*******************************************************************************
*
********************************************************************************/


/*******************************************************************************
*
********************************************************************************/
typedef struct MyJobInfo{
    MUINT32              mSrcDataType;
    MUINT32              mDstDataType;
    IImageBuffer const*  mSrcBuffer;
    IImageBuffer const*  mDstBuffer;
    MBOOL                mIsSOI;
    MUINT32              mQuality;
    MINT32               mTransform;
    //MUINT32              mZoom;
} MyJobInfo_t;

class JpegEncNodeImpl : public JpegEncNode
{
    public: // ctor & dtor
        JpegEncNodeImpl(MBOOL createThread);
        ~JpegEncNodeImpl() {};

    DECLARE_ICAMTHREADNODE_INTERFACES();

    virtual  MVOID setEncParam(MBOOL const isSOI, MUINT32 const quality);

    private:

    MBOOL       queueJob(MyJobInfo const& jobInfo);
    MBOOL       dequeJob(MyJobInfo* jobInfo);
    MBOOL       executeAndWait(MyJobInfo const& jobInfo);

    Mutex                                  mLock;
    MBOOL                                  mIsSOI;
    MUINT32                                mQuality;
    queue< MyJobInfo_t >                   mqJob;
    MUINT32                                muJobDoneCount;
};


/*******************************************************************************
*
********************************************************************************/
JpegEncNode*
JpegEncNode::
createInstance(MBOOL createThread)
{
    return new JpegEncNodeImpl(createThread);
}


/*******************************************************************************
*
********************************************************************************/
void
JpegEncNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
JpegEncNode::
JpegEncNode(MBOOL createThread)
    : ICamThreadNode(MODULE_NAME, createThread ? SingleTrigger : NO_THREAD,
                     SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
*
********************************************************************************/
JpegEncNodeImpl::
JpegEncNodeImpl(MBOOL createThread)
    : JpegEncNode(createThread)
    , mIsSOI(MTRUE)
    , mQuality(90)
{
    //addSupport
    addDataSupport( ENDPOINT_SRC, JPEG_ENC_SW_SRC );
    addDataSupport( ENDPOINT_SRC, JPEG_ENC_HW_SRC );
    addDataSupport( ENDPOINT_DST, JPEG_ENC_SW_DST );
    addDataSupport( ENDPOINT_DST, JPEG_ENC_HW_DST );
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onInit()
{
    FUNC_START;
    //query infos

    FUNC_END;
    return true;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onUninit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onStart()
{
    FUNC_START;
    muJobDoneCount = 0;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;

#define PREPARE_DST_BUFFER(data, dstdata, imgRequest)         \
    do{                                                       \
        if( isDataConnected(data) )                           \
        {                                                     \
            ICamBufHandler* pBufHdl = getBufferHandler(data); \
            if( !pBufHdl)                                     \
            {                                                 \
                break;                                        \
            }                                                 \
            if( pBufHdl->dequeBuffer(data, &imgRequest) )     \
            {                                                 \
                dstdata = data;                               \
            }                                                 \
        }                                                     \
    }while(0)

    MUINT32 dstDataType = 0;
    ImgRequest imgRequest;
    switch( data )
    {
        case JPEG_ENC_SW_SRC:
            PREPARE_DST_BUFFER(JPEG_ENC_SW_DST, dstDataType, imgRequest);
            break;
        case JPEG_ENC_HW_SRC:
            PREPARE_DST_BUFFER(JPEG_ENC_HW_DST, dstDataType, imgRequest);
            break;
        default:
            MY_LOGE("not support: %d", data);
            break;
    }
#undef PREPARE_DST_BUFFER
    if( dstDataType )
    {
        MyJobInfo job = {
            data,
            dstDataType,
            static_cast<IImageBuffer const*>((MVOID*)buf),
            imgRequest.mBuffer,
            mIsSOI,
            mQuality,
            imgRequest.mTransform
        //    0
        };

        if( getThreadNodeType() == NO_THREAD )
        {
            ret = ret && executeAndWait(job);

            if( ret )
            {
                handleReturnBuffer( job.mSrcDataType, (MUINTPTR)job.mSrcBuffer );
                handlePostBuffer( job.mDstDataType, (MUINTPTR)job.mDstBuffer );
            }
            else
            {
                //todo: error handling
                MY_LOGE("error");
            }
        }
        else
        {
            queueJob(job);
            triggerLoop();
        }
    }
    else
    {
        MY_LOGW("no dst buffer, skip jpeg src buf 0x%x", buf);
        handleReturnBuffer( data, buf, ext);
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGV("data(%d), buf(0x%x)", data, buf);
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    if( !pBufHdl )
    {
        MY_LOGE("no buffer hdl for data(%d), buf(0x%x)", data, buf);
        return MFALSE;
    }

    MBOOL ret = pBufHdl->enqueBuffer(data, (IImageBuffer*)buf);
    if( !ret )
    {
        MY_LOGE("enque fail: data(%d), buf(0x%x)", data, buf);
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
JpegEncNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    MyJobInfo job;

    dequeJob(&job);
    ret = executeAndWait(job);

    if( ret )
    {
        handleReturnBuffer( job.mSrcDataType, (MUINTPTR)job.mSrcBuffer );
        handlePostBuffer( job.mDstDataType, (MUINTPTR)job.mDstBuffer );
    }
    else
    {
        //todo: error handling
        MY_LOGE("exe failed");
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
JpegEncNodeImpl::
setEncParam(MBOOL const isSOI, MUINT32 const quality)
{
    mIsSOI = isSOI;
    mQuality = quality;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
queueJob(MyJobInfo const& jobInfo)
{
    Mutex::Autolock _l(mLock);
    mqJob.push(jobInfo);

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
dequeJob(MyJobInfo* jobInfo)
{
    Mutex::Autolock _l(mLock);
    *jobInfo = mqJob.front();
    mqJob.pop();

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
JpegEncNodeImpl::
executeAndWait(MyJobInfo const& jobInfo)
{
    MBOOL ret = MTRUE;

    ISImager *pISImager = ISImager::createInstance(jobInfo.mSrcBuffer);
    if( !pISImager )
    {
        return MFALSE;
    }
    //
    MY_LOGD("jpeg%s: %d, %d src 0x%x, dst 0x%x, cnt %d",
            jobInfo.mSrcDataType == JPEG_ENC_HW_SRC ? "HW" : "SW",
            jobInfo.mSrcDataType,
            jobInfo.mDstDataType,
            jobInfo.mSrcBuffer,
            jobInfo.mDstBuffer,
            muJobDoneCount);

    CAM_TRACE_FMT_BEGIN("jpeg(%s)",
                        jobInfo.mSrcDataType == JPEG_ENC_HW_SRC ? "HW" : "SW"
                        );
    // if different aspect ratio
    MRect crop = MRect(MPoint(0, 0), jobInfo.mSrcBuffer->getImgSize());
    MSize const srcSize = jobInfo.mSrcBuffer->getImgSize();
    MSize const dstSize =
        (jobInfo.mTransform & eTransform_ROT_90) ?
        MSize(jobInfo.mDstBuffer->getImgSize().h, jobInfo.mDstBuffer->getImgSize().w) :
        jobInfo.mDstBuffer->getImgSize();
    //
#define align2(x)   (((x) + 1) & (~1))
    if( srcSize.w * dstSize.h > srcSize.h * dstSize.w ) {
        crop.s.w = align2(dstSize.w * srcSize.h / dstSize.h);
        crop.s.h = align2(srcSize.h);
        crop.p.x = (srcSize.w - crop.s.w) / 2;
        crop.p.y = 0;
    } else if( srcSize.w * dstSize.h < srcSize.h * dstSize.w ) {
        crop.s.w = align2(srcSize.w);
        crop.s.h = align2(dstSize.h * srcSize.w / dstSize.w);
        crop.p.x = 0;
        crop.p.y = (srcSize.h - crop.s.h) / 2;
    } else {
        crop = MRect(MPoint(0, 0), jobInfo.mSrcBuffer->getImgSize());
    }
#undef align2
    //
#if 0
    MY_LOGD("crop %d, %d, %dx%d",
            crop.p.x, crop.p.y, crop.s.w, crop.s.h);
#endif
    //
    ret = ret
        && pISImager->setTargetImgBuffer(jobInfo.mDstBuffer)
    //
        && pISImager->setTransform(jobInfo.mTransform)
    //
        && pISImager->setEncodeParam(jobInfo.mIsSOI, jobInfo.mQuality,
                                     jobInfo.mSrcDataType == JPEG_ENC_SW_SRC ?
                                     JPEGENC_SW: JPEGENC_HW_FIRST)
    //
        && pISImager->setCropROI(crop)
    //
        && pISImager->execute();
    //
    pISImager->destroyInstance();
    //
    IImageBuffer* pDstBuf = const_cast<IImageBuffer*>(jobInfo.mDstBuffer);
    pDstBuf->setTimestamp( jobInfo.mSrcBuffer->getTimestamp() );
    //
    if( !ret )
    {
        MY_LOGE("enc failed");
    }
    //
    muJobDoneCount++;
    //
    CAM_TRACE_FMT_END();
    //
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

