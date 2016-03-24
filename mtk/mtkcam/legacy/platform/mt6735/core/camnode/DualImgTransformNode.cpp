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
#define LOG_TAG "MtkCam/DualITNode"

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
#include <mtkcam/camnode/DualImgTransformNode.h>

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
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)


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

#define MODULE_NAME        "DualITNode"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class DualITNodeImpl : public DualImgTransformNode
{
    private:

    public: // ctor & dtor
        DualITNodeImpl(MUINT32 const pos);
        ~DualITNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations

        MBOOL           doImageTransform(
                            MUINT32 const data,
                            IImageBuffer* const buf,
                            MRect const crop,
                            MBOOL const isRightBuf);

        MBOOL           isReadyToTransfrom() const { return (mlPostBuf_0.size() != 0 && mlPostBuf_1.size() != 0); }

        MBOOL           pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);


    protected:

    #define MAX_DST_PORT_NUM        (3)

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MRect            crop;
        }PostBufInfo;

    private:
        // define:
        //    0 is main-main2 (main in L)
        //    1 is main2-main (main in R)
        MUINT32                 muSensorPosition;

        // dst deque order, should be overwriten by subclass
        MUINT32                 muDequeOrder[MAX_DST_PORT_NUM];
        IImageTransform*        mpImgTransform;
        ImgRequest              mImgRequest_0;
        ImgRequest              mImgRequest_1;
        ImgRequest              mImgRequest_SBS;

        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muDeqFrameCnt;
        list<PostBufInfo>       mlPostBuf_0;
        list<PostBufInfo>       mlPostBuf_1;

};


/*******************************************************************************
 *
 ********************************************************************************/
DualImgTransformNode*
DualImgTransformNode::
createInstance(MUINT32 const pos)
{
    return new DualITNodeImpl(pos);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
DualImgTransformNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
DualImgTransformNode::
    DualImgTransformNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
DualImgTransformNode::
~DualImgTransformNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
DualITNodeImpl::
    DualITNodeImpl(MUINT32 const pos)
    : DualImgTransformNode()
    , muSensorPosition(pos)
    , mpImgTransform(NULL)
    , mImgRequest_0()
    , mImgRequest_1()
    , mImgRequest_SBS()
    , muPostFrameCnt(0)
    , muDeqFrameCnt(0)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, DUALIT_SRC_0 );
    addDataSupport( ENDPOINT_SRC, DUALIT_SRC_1 );
    addDataSupport( ENDPOINT_DST, DUALIT_DST_0 );
    addDataSupport( ENDPOINT_DST, DUALIT_DST_1 );
    addDataSupport( ENDPOINT_DST, DUALIT_DST_2 );
    //
    muDequeOrder[0] = DUALIT_DST_0;
    muDequeOrder[1] = DUALIT_DST_1;
    muDequeOrder[2] = DUALIT_DST_2;
    //
}


/*******************************************************************************
 *
 ********************************************************************************/
DualITNodeImpl::
~DualITNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualITNodeImpl::
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
DualITNodeImpl::
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
DualITNodeImpl::
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
DualITNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done

    Mutex::Autolock lock(mLock);
    {
        list<PostBufInfo>::iterator iter;
        for(iter = mlPostBuf_0.begin(); iter != mlPostBuf_0.end(); iter++)
        {
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)",
                    (*iter).data,
                    (*iter).buf);
            handleReturnBuffer(
                (*iter).data,
                (MUINTPTR)((*iter).buf),
                0);
        }
        for(iter = mlPostBuf_1.begin(); iter != mlPostBuf_1.end(); iter++)
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
DualITNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualITNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
DualITNodeImpl::
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
DualITNodeImpl::
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
            case DUALIT_SRC_0:
                DUMP_IImageBuffer( DUALIT_SRC_0, buf, "yuv", muPostFrameCnt );
                break;
            case DUALIT_SRC_1:
                DUMP_IImageBuffer( DUALIT_SRC_1, buf, "yuv", muPostFrameCnt );
                break;
            case DUALIT_DST_0:
                DUMP_IImageBuffer( DUALIT_DST_0, buf, "yuv", muDeqFrameCnt  );
                break;
            case DUALIT_DST_1:
                DUMP_IImageBuffer( DUALIT_DST_1, buf, "yuv", muDeqFrameCnt  );
                break;
            case DUALIT_DST_2:
                DUMP_IImageBuffer( DUALIT_DST_2, buf, "yuv", muDeqFrameCnt  );
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
DualITNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    MUINT32 dstCount = 0;
    IImageBuffer* pDstBuf_SBS = NULL;
    PostBufInfo postBuf_0, postBuf_1;
    {
        Mutex::Autolock lock(mLock);

        if( mlPostBuf_0.size() == 0 || mlPostBuf_1.size() == 0 ) {
            MY_LOGE("no posted buf(%d/%d)", mlPostBuf_0.size(), mlPostBuf_1.size());
            return MFALSE;
        }

        postBuf_0 = mlPostBuf_0.front();
        postBuf_1 = mlPostBuf_1.front();
        mlPostBuf_0.pop_front();
        mlPostBuf_1.pop_front();
    }
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
                case DUALIT_DST_0:
                    mImgRequest_0 = outRequest;
                    break;
                case DUALIT_DST_1:
                    mImgRequest_1 = outRequest;
                    break;
                case DUALIT_DST_2:
                    mImgRequest_SBS = outRequest;
                    pDstBuf_SBS = const_cast<IImageBuffer*>(outRequest.mBuffer);
                    pDstBuf_SBS->setTimestamp(postBuf_0.buf->getTimestamp());
                    pDstBuf_SBS->unlockBuf(LOG_TAG);
                    // not support transform
                    if( outRequest.mTransform )
                        MY_LOGW("dst 2 does not support transform");
                    break;
                default:
                    MY_LOGE("unsupport deque port(%d)", dstData);
                    break;
            }
            MY_LOGD("dstDataType(%d)", dstData);
            dstCount++;
        }
    }
    if( dstCount == 0 )
    {
        MY_LOGW("no dst buffer, skip data(%d/%d), buf(0x%x/0x%x)", postBuf_0.data, postBuf_1.data, postBuf_0.buf, postBuf_1.buf);
        handleReturnBuffer(postBuf_0.data, (MUINTPTR)postBuf_0.buf);
        handleReturnBuffer(postBuf_1.data, (MUINTPTR)postBuf_1.buf);
        return MTRUE;
    }
    // muSensorPosition:
    //    0 is main-main2 (main in L)
    //    1 is main2-main (main in R)
    ret =   doImageTransform(postBuf_0.data, postBuf_0.buf, postBuf_0.crop, (muSensorPosition == 1))
        &&  doImageTransform(postBuf_1.data, postBuf_1.buf, postBuf_1.crop, (muSensorPosition != 1));
    //
    if ( pDstBuf_SBS != NULL )
    {
        pDstBuf_SBS->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
        handlePostBuffer( DUALIT_DST_2, (MUINTPTR)pDstBuf_SBS);
    }
    handleReturnBuffer(postBuf_0.data, (MUINTPTR)postBuf_0.buf);
    handleReturnBuffer(postBuf_1.data, (MUINTPTR)postBuf_1.buf);
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DualITNodeImpl::
doImageTransform(
    MUINT32 const data,
    IImageBuffer* const buf,
    MRect const crop,
    MBOOL const isRightBuf
)
{
    MBOOL ret = MTRUE;
    //
    MUINT32 const dstDataType   = ( data == DUALIT_SRC_0 ) ? DUALIT_DST_0 : DUALIT_DST_1;
    ImgRequest const dstRequest = ( data == DUALIT_SRC_0 ) ? mImgRequest_0 : mImgRequest_1;
    ImgRequest const dstRequest_SBS = mImgRequest_SBS;
    MUINT32 const trans = dstRequest.mTransform;
    IImageBuffer* pDstBuf = NULL;
    IImageBuffer* pDstBuf_SBS = NULL;   // L or R of SBS image buffer

    if ( dstRequest.mBuffer != NULL )
    {
        pDstBuf = const_cast<IImageBuffer*>(dstRequest.mBuffer);
        pDstBuf->setTimestamp(buf->getTimestamp());
    }
    if ( dstRequest_SBS.mBuffer != NULL )
    {
        IImageBuffer* pBuf = const_cast<IImageBuffer*>(dstRequest_SBS.mBuffer);
        pDstBuf_SBS = pBuf->getImageBufferHeap()->createImageBuffer_SideBySide(isRightBuf);
        if ( pDstBuf_SBS != NULL )
        {
            if ( !pDstBuf_SBS->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
            {
                MY_LOGE("lock Buffer failed");
                ret = MFALSE;
            }
            pDstBuf_SBS->setTimestamp(buf->getTimestamp());
        }
        else
        {
            MY_LOGE("pDstBuf_SBS is NULL");
            ret = MFALSE;
        }
    }
    if( pDstBuf == NULL && pDstBuf_SBS == NULL)
    {
        MY_LOGW("no dst buffer");
        return MTRUE;
    }
    //
    MY_LOGD("src:(%p),S(%dx%d),stride(%d),F(%d),TS(%lld)", buf, buf->getImgSize().w, buf->getImgSize().h,
                buf->getBufStridesInBytes(0), buf->getImgFormat(), buf->getTimestamp() );
    MY_LOGD_IF( pDstBuf != NULL, "dst:(%p),S(%dx%d),stride(%d),F(%d),TS(%lld), trans(%d)", pDstBuf, pDstBuf->getImgSize().w, pDstBuf->getImgSize().h,
            pDstBuf->getBufStridesInBytes(0), pDstBuf->getImgFormat(), pDstBuf->getTimestamp(), trans );
    MY_LOGD_IF( pDstBuf_SBS != NULL, "SBS:(%p),isR(%d),S(%dx%d),stride(%d),F(%d),TS(%lld)", pDstBuf_SBS, isRightBuf,
            pDstBuf_SBS->getImgSize().w, pDstBuf_SBS->getImgSize().h,
            pDstBuf_SBS->getBufStridesInBytes(0), pDstBuf_SBS->getImgFormat(), pDstBuf_SBS->getTimestamp() );
    //
    CAM_TRACE_FMT_BEGIN("execP2:%d", muDeqFrameCnt);
    ret = ret && mpImgTransform->execute(buf, pDstBuf, pDstBuf_SBS, crop, trans, 0xFFFFFFFF);
    CAM_TRACE_FMT_END();
    muDeqFrameCnt++;
    //
    if ( pDstBuf != NULL )
    {
        handlePostBuffer( dstDataType, (MUINTPTR)pDstBuf );
    }
    //
    if ( pDstBuf_SBS != NULL && !pDstBuf_SBS->unlockBuf( LOG_TAG ) )
    {
        MY_LOGE("unlock Buffer failed");
        ret = MFALSE;
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DualITNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    MBOOL ret = MTRUE;
    #if 0 //AARON FIXME
    MRect crop = ( ext != 0 ) ? *(MRect*)ext : MRect(MPoint(), buf->getImgSize());
    #else
    MRect crop = MRect(MPoint(), buf->getImgSize());
    #endif
    PostBufInfo postBufData = {data, buf, crop};
    switch (data)
    {
        case DUALIT_SRC_0:
            mlPostBuf_0.push_back(postBufData);
            break;
        case DUALIT_SRC_1:
            mlPostBuf_1.push_back(postBufData);
            break;
        default:
            MY_LOGE("Unsupported data(%d)", data);
            handleReturnBuffer(data, (MUINTPTR)buf);
            ret = MFALSE;
            break;
    }

    MY_LOGD("size(%d), data(%d), buf(0x%x), ext(0x%x), crop(%d,%d,%d,%d)",
            mlPostBuf_0.size(),
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

    return ret;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

