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
#define LOG_TAG "MtkCam/ITNode"

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
#include <mtkcam/iopipe/SImager/IImageTransform.h>
using namespace NSCam::NSIoPipe::NSSImager;
//
#include <aee.h>
//
#include <mtkcam/camnode/ImgTransformNode.h>

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

#define ENABLE_CAMIOCONTROL_LOG (0)
#define ENABLE_BUFCONTROL_LOG   (1)
#define BUFFER_RETURN_CHECK     (1)

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

#define MODULE_NAME        "ITNode"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class ImgTransformNodeImpl : public ImgTransformNode
{
    private:

    public: // ctor & dtor
        ImgTransformNodeImpl();
        ~ImgTransformNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations

        MBOOL           doImageTransform(
                            MUINT32 const data,
                            IImageBuffer* const buf,
                            MRect const crop);

        MBOOL           isReadyToTransfrom() const { return MTRUE; }

        MBOOL           pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);


    protected:

    #define MAX_DST_PORT_NUM        (2)

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MRect            crop;
        }PostBufInfo;

    private:
        // dst deque order, should be overwriten by subclass
        MUINT32                 muDequeOrder[MAX_DST_PORT_NUM];
        IImageTransform*        mpImgTransform;

        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muDeqFrameCnt;
        list<PostBufInfo>       mlPostBufData;

};


/*******************************************************************************
 *
 ********************************************************************************/
ImgTransformNode*
ImgTransformNode::
createInstance()
{
    return new ImgTransformNodeImpl();
}


/*******************************************************************************
 *
 ********************************************************************************/
void
ImgTransformNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
ImgTransformNode::
    ImgTransformNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
ImgTransformNode::
~ImgTransformNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
ImgTransformNodeImpl::
    ImgTransformNodeImpl()
    : ImgTransformNode()
    , mpImgTransform(NULL)
    , muPostFrameCnt(0)
    , muDeqFrameCnt(0)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, TRANSFORM_SRC );
    addDataSupport( ENDPOINT_DST, TRANSFORM_DST_0 );
    addDataSupport( ENDPOINT_DST, TRANSFORM_DST_1 );
    //
    muDequeOrder[0] = TRANSFORM_DST_1;
    muDequeOrder[1] = TRANSFORM_DST_0;
    //
}


/*******************************************************************************
 *
 ********************************************************************************/
ImgTransformNodeImpl::
~ImgTransformNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;

    mpImgTransform = IImageTransform::createInstance();
    if( !mpImgTransform )
    {
        MY_LOGE("imageTransform create failed");
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
ImgTransformNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;

    if( mpImgTransform )
    {
        mpImgTransform->destroyInstance();
        mpImgTransform = NULL;
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;


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
ImgTransformNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done

    Mutex::Autolock lock(mLock);
    {
        list<PostBufInfo>::iterator iter;
        for(iter = mlPostBufData.begin(); iter != mlPostBufData.end(); iter++)
        {
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)",
                    (*iter).data,
                    (*iter).buf);
            handleReturnBuffer(
                (*iter).data,
                (MUINTPTR)((*iter).buf),
                0);
        }
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;
    CAM_TRACE_CALL();
    MY_LOGD("data %d, buf 0x%x", data, buf);
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    if( !pBufHdl )
    {
        MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
        return MFALSE;
    }
    ret = pBufHdl->enqueBuffer(data, (IImageBuffer*)buf);
    if( !ret )
    {
        MY_LOGE("enque fail: data %d, buf 0x%x", data, buf);
    }
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
ImgTransformNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
        char dumppath[256];
        sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)               \
        do{                                                        \
            IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
            char filename[256];                                    \
            sprintf(filename, "%s%s_%d_%dx%d_%d.%s",               \
                    dumppath,                                      \
                    #type,                                         \
                    getSensorIdx(),                                \
                    buffer->getImgSize().w,buffer->getImgSize().h, \
                    cnt,                                           \
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
            case TRANSFORM_SRC:
                DUMP_IImageBuffer( TRANSFORM_SRC, buf, "yuv", muPostFrameCnt );
                break;
            case TRANSFORM_DST_0:
                DUMP_IImageBuffer( TRANSFORM_DST_0, buf, "yuv", muDeqFrameCnt  );
                break;
            case TRANSFORM_DST_1:
                DUMP_IImageBuffer( TRANSFORM_DST_1, buf, "yuv", muDeqFrameCnt  );
                break;
            default:
                MY_LOGE("not handle this yet: data %d", data);
                break;
        }
#undef DUMP_IImageBuffer
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ImgTransformNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    PostBufInfo postBufData;
    {
        Mutex::Autolock lock(mLock);

        if( mlPostBufData.size() == 0 ) {
            MY_LOGE("no posted buf");
            return MFALSE;
        }

        postBufData = mlPostBufData.front();
        mlPostBufData.pop_front();
    }
    ret = doImageTransform(postBufData.data, postBufData.buf, postBufData.crop);
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImgTransformNodeImpl::
doImageTransform(MUINT32 const data, IImageBuffer* const buf, MRect const crop)
{
    MBOOL ret = MTRUE;
    //
    MUINT32 dstCount = 0, dstDataType[MAX_DST_PORT_NUM];
    MUINT32 transform = 0;
    IImageBuffer* pDstBuf[MAX_DST_PORT_NUM] = {NULL, NULL};

    for(MUINT32 i = 0; i < MAX_DST_PORT_NUM; i++)
    {
        if(muDequeOrder[i] == 0)
            break;

        MUINT32 const dstData = muDequeOrder[i];
        ICamBufHandler* pBufHdl = getBufferHandler(dstData);
        ImgRequest outRequest;
        if( pBufHdl && pBufHdl->dequeBuffer( dstData, &outRequest) )
        {
            switch (dstData)
            {
                case TRANSFORM_DST_0:
                    // not support transform
                    if( outRequest.mTransform )
                        MY_LOGW("dst 0 does not support transform");
                    break;
                case TRANSFORM_DST_1:
                    transform = outRequest.mTransform;
                    break;
                default:
                    MY_LOGE("unsupport deque port(%d)", dstData);
                    break;
            }
            pDstBuf[dstCount] = const_cast<IImageBuffer*>(outRequest.mBuffer);
            dstDataType[dstCount] = dstData;
//            MY_LOGD("dstDataType(%d)",dstDataType[dstCount]);
            dstCount++;
        }
    }
    if( dstCount == 0 )
    {
        MY_LOGW("no dst buffer, skip data(%d), buf(0x%x)", data, buf);
        handleReturnBuffer(data, (MUINTPTR)buf);
        return MTRUE;
    }
    // further crop due to different aspect ratio
    MRect newCrop = crop;
    MSize const srcSize = crop.s;
    MSize const dstSize =
        transform & eTransform_ROT_90 ?
        MSize( pDstBuf[0]->getImgSize().h, pDstBuf[0]->getImgSize().w ) :
        pDstBuf[0]->getImgSize();
#define align2(x)   (((x) + 1) & (~1))
    if( srcSize.w * dstSize.h > srcSize.h * dstSize.w ) {
        newCrop.s.w = align2(dstSize.w * srcSize.h / dstSize.h);
        newCrop.s.h = align2(srcSize.h);
        newCrop.p.x = crop.p.x + (srcSize.w - newCrop.s.w) / 2;
        newCrop.p.y = crop.p.y;
    } else if( srcSize.w * dstSize.h < srcSize.h * dstSize.w ) {
        newCrop.s.w = align2(srcSize.w);
        newCrop.s.h = align2(dstSize.h * srcSize.w / dstSize.w);
        newCrop.p.x = crop.p.x;
        newCrop.p.y = crop.p.y + (srcSize.h - newCrop.s.h) / 2;
    }
#undef align2
    //
    MY_LOGD(" src:(%p),S(%dx%d),F(%d),TS(%lld)", buf, buf->getImgSize().w, buf->getImgSize().h, buf->getImgFormat(), buf->getTimestamp() );
    for ( MUINT32 i = 0; i < dstCount; ++i) {
        IImageBuffer* pBuf = pDstBuf[i];
        pBuf->setTimestamp(buf->getTimestamp());
        MY_LOGD(" dst:(%p),type(%d),S(%dx%d),F(%d),TS(%lld) trans: %d", pBuf, dstDataType[i],
            pBuf->getImgSize().w, pBuf->getImgSize().h, pBuf->getImgFormat(), pBuf->getTimestamp(), transform );
    }
    CAM_TRACE_FMT_BEGIN("execP2:%d", muDeqFrameCnt);
    ret = ret && mpImgTransform->execute(buf, pDstBuf[0], pDstBuf[1], newCrop, transform, 0xFFFFFFFF);
    CAM_TRACE_FMT_END();
    muDeqFrameCnt++;
    //
    for ( MUINT32 i = 0; i < dstCount; ++i) {
        IImageBuffer* postBuf = pDstBuf[i];
        handlePostBuffer( dstDataType[i], (MUINTPTR)postBuf, 0 );
    }
    handleReturnBuffer( data, (MUINTPTR)buf);
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ImgTransformNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);

    #if 0 //AARON FIXME
    MRect crop = ( ext != 0 ) ? *(MRect*)ext : MRect(MPoint(), buf->getImgSize());
    #else
    MRect crop = MRect(MPoint(), buf->getImgSize());
    #endif
    PostBufInfo postBufData = {data, buf, crop};
    mlPostBufData.push_back(postBufData);

    MY_LOGD("size(%d), data(%d), buf(0x%x), ext(0x%x), crop(%d,%d,%d,%d)",
            mlPostBufData.size(),
            postBufData.data,
            postBufData.buf,
            ext,
            postBufData.crop.p.x,
            postBufData.crop.p.y,
            postBufData.crop.s.w,
            postBufData.crop.s.h);

    muPostFrameCnt++;

    if( isReadyToTransfrom() )
    {
        triggerLoop();
    }

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

