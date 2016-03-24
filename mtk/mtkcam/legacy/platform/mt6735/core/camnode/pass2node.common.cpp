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
#define LOG_TAG "MtkCam/P2Node"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
//
#include <semaphore.h>
using namespace std;
//
#include <utils/StrongPointer.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <aee.h>
//
#include <mtkcam/camnode/pass2node.h>
#include "./inc/pass2nodeImpl.h"
#include "./inc/IspSyncControlHw.h"
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                   printf("[%s::%s] " fmt"\n", getName(),__func__, ##arg)
#endif

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")
#define FUNC_NAME  MY_LOGD("")

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


/*******************************************************************************
*
********************************************************************************/
#define MODULE_PRV_NAME    "pass2"
#define MODULE_CAP_NAME    "CapP2"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
#define SCHED_VSS_PRIORITY (NICE_CAMERA_VSS_PASS2)
#define PASS2_TIMEOUT      ((MINT64)5000000000) //ns


/*******************************************************************************
*
********************************************************************************/
Pass2Node*
Pass2Node::
createInstance(Pass2NodeType type)
{
    switch(type)
    {
        case PASS2_PREVIEW:
            return PrvPass2::createInstance(type);
        case PASS2_CAPTURE:
        case PASS2_VSS:
        case PASS2_PURERAW_CAPTURE:
            return new CapPass2(type);
        case PASS2_FEATURE:
            return PrvPass2::createInstance(type);
        default:
            break;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
Pass2Node::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
Pass2Node::
Pass2Node(Pass2NodeType const type)
    : ICamThreadNode(
            (type == PASS2_PREVIEW || type == PASS2_FEATURE) ?  MODULE_PRV_NAME : MODULE_CAP_NAME ,
            SingleTrigger,
            SCHED_POLICY,
            type == PASS2_VSS ? SCHED_VSS_PRIORITY : SCHED_PRIORITY
            )
{
}


/*******************************************************************************
*
********************************************************************************/
Pass2Node::
~Pass2Node()
{
}




/*******************************************************************************
*
********************************************************************************/
Pass2NodeImpl::
Pass2NodeImpl(Pass2NodeType const type)
    : Pass2Node(type)
    , mPass2Type(type)
    , mpPostProcPipe(NULL)
    , mpIspSyncCtrlHw(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
Pass2NodeImpl::
~Pass2NodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;

    mpIspSyncCtrlHw = IspSyncControlHw::createInstance(getSensorIdx());
    if(!mpIspSyncCtrlHw)
    {
        MY_LOGE("create IspSyncControlHw failed");
        goto lbExit;
    }

    mlPostBufData.clear();

    if( !moduleInit() )
    {
        MY_LOGE("moduleInit() failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onUninit()
{
    FUNC_START;
    if(mpIspSyncCtrlHw)
    {
        mpIspSyncCtrlHw->destroyInstance();
        mpIspSyncCtrlHw = NULL;
    }

    MBOOL ret = mpPostProcPipe->uninit();

    mpPostProcPipe->destroyInstance(getName());
    mpPostProcPipe = NULL;
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onStart()
{
    FUNC_START;
    muPostFrameCnt = 0;
    muEnqFrameCnt = 0;
    muDeqFrameCnt = 0;

    if( !moduleStart() )
    {
        MY_LOGE("moduleStart() failed");
        return MFALSE;
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = MTRUE;

    if( !syncWithThread() ) //wait for jobs done
    {
        MY_LOGE("sync with thread failed");
        ret = MFALSE;
    }

    #if PASS2_CALLBACL_ENABLE
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

    while(muEnqFrameCnt > muDeqFrameCnt)
    {
        MY_LOGD("wait lock enq %d > deq %d", muEnqFrameCnt, muDeqFrameCnt);
        mCondDeque.wait(mLock);
        MY_LOGD("wait done");
    }
    #endif

    if( !moduleStop() )
    {
        MY_LOGE("moduleStop failed");
        ret = MFALSE;
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    //FUNC_START;
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#if PASS2_CALLBACL_ENABLE
    if( pushBuf(data, (IImageBuffer*)buf, ext) )
    {
        // no thing
    }
#else
    if( enquePass2(data,buf,ext) )
    {
        triggerLoop();
    }
#endif

    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGV("data %d, buf 0x%x", data, buf);
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    if( !pBufHdl )
    {
        MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
        return MFALSE;
    }

    MBOOL ret = pBufHdl->enqueBuffer(data, (IImageBuffer*)buf);
    if( !ret )
    {
        MY_LOGE("enque fail: data %d, buf 0x%x", data, buf);
    }
    //FUNC_END;
    return MTRUE; //return status?
}


/*******************************************************************************
*
********************************************************************************/
MVOID
Pass2NodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PREFIX "/sdcard/cameradump_"
    char dumppath[256];
    sprintf( dumppath, "%s%s/", DUMP_PREFIX, usr );
#define DUMP_IImageBuffer( type, pbuf, fileExt, cnt)           \
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

#define DUMP_IImageBufferStride( type, pbuf, fileExt, cnt)     \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%d_%dx%d_%d_%d.%s",            \
                dumppath,                                      \
                #type,                                         \
                getSensorIdx(),                                \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                buffer->getBufStridesInBytes(0),               \
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
        case PASS2_PRV_SRC:
            DUMP_IImageBufferStride( PASS2_PRV_SRC, buf, "raw", muPostFrameCnt );
            break;
        case PASS2_CAP_SRC:
            DUMP_IImageBufferStride( PASS2_CAP_SRC, buf, "raw", muPostFrameCnt );
            break;
        case PASS2_PRV_DST_0:
            DUMP_IImageBuffer( PASS2_PRV_DST_0, buf, "yuv", muDeqFrameCnt  );
            break;
        case PASS2_PRV_DST_1:
            DUMP_IImageBuffer( PASS2_PRV_DST_1, buf, "yuv", muDeqFrameCnt  );
            break;
        case PASS2_PRV_DST_2:
            DUMP_IImageBuffer( PASS2_PRV_DST_2, buf, "yuv", muDeqFrameCnt  );
            break;
        case PASS2_PRV_DST_3:
            DUMP_IImageBuffer( PASS2_PRV_DST_3, buf, "vdo", muDeqFrameCnt  );
            break;
        case PASS2_CAP_DST_0:
            DUMP_IImageBuffer( PASS2_CAP_DST_0, buf, "yuv", muDeqFrameCnt  );
            break;
        case PASS2_CAP_DST_1:
            DUMP_IImageBuffer( PASS2_CAP_DST_1, buf, "yuv", muDeqFrameCnt  );
            break;
        case PASS2_CAP_DST_2:
            DUMP_IImageBuffer( PASS2_CAP_DST_2, buf, "yuv", muDeqFrameCnt  );
            break;
        default:
            MY_LOGE("not handle this yet: data %d", data);
            break;
    }
#undef DUMP_IImageBufferStride
#undef DUMP_IImageBuffer
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
threadLoopUpdate()
{
    MY_LOGV("+");

    MBOOL ret = MTRUE;

#if PASS2_CALLBACL_ENABLE
    ret = enquePass2(MTRUE);
#else
    // use p2 thread to deque
    ret = dequePass2();
#endif

    MY_LOGV("-");
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
getDstBuffer(
    MUINT32         nodeData,
    ImgRequest*     pImgReq)
{
    MBOOL ret = MFALSE;
    ICamBufHandler* pBufHdl = getBufferHandler(nodeData);
    if(pBufHdl && pBufHdl->dequeBuffer(nodeData, pImgReq))
    {
        //MY_LOGD("dstDataType(%d)",dstDataType);
        ret = MTRUE;
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
dequePass2()
{
    CAM_TRACE_CALL();
    MBOOL ret = MFALSE;
    QParams dequeParams;
    Vector<Input>::const_iterator iterIn;
    Vector<Output>::const_iterator iterOut;
    //
    MY_LOGV("type(%d) cnt %d: deque", mPass2Type, muDeqFrameCnt);
    if( !mpPostProcPipe->deque(dequeParams, PASS2_TIMEOUT) )
    {
        MY_LOGE("type(%d) pass2 cnt %d: deque fail", mPass2Type, muDeqFrameCnt);
        aee_system_exception(
            LOG_TAG,
            NULL,
            DB_OPT_DEFAULT,
            "\nCRDISPATCH_KEY:MtkCam/P1Node:ISP pass2 deque fail");
        goto lbExit;
    }

    if( !handleP2Done(dequeParams) )
    {
        MY_LOGE("handle p2 callback failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    CAM_TRACE_FMT_END();
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
enquePass2(MBOOL const doCallback)
{
    //workaround for start/stop recording
    Mutex::Autolock lock(mRecordingLock);

    CAM_TRACE_CALL();
    QParams enqueParams;
    vector<p2data> vP2data;
    MUINT32 trace_begin = muEnqFrameCnt;

    if( !getPass2Buffer(vP2data) )
    {
        // no dst buffers
        return MTRUE;
    }

    if( vP2data.size() == 0 )
    {
        MY_LOGE("no src/dst buf");
        return MFALSE;
    }

    enqueParams.mvIn.reserve( vP2data.size() );
    //enqueParams.mvOut.reserve(dstCount);

    MUINT32 index = 0;
    vector<p2data>::const_iterator pData = vP2data.begin();
    while( pData != vP2data.end() )
    {
        MUINT32 magicNum;
        MVOID*  pPrivateData;
        MUINT32 privateDataSize;
        MCropRect p2InCrop;
        MSize srcSize;
        MINT64 timestamp;

        // input
        Input src;
        //
        src.mPortID = mapToPortID(pData->src.data);
        src.mPortID.group = index;
        src.mBuffer = pData->src.buf;
        //
        if( pData->doCrop )
        {
            mpIspSyncCtrlHw->getPass2Info(
                    src.mBuffer,
                    pData->dstSize,
                    magicNum,
                    pPrivateData,
                    privateDataSize,
                    p2InCrop);
        }
        else
        {
            MBOOL isRrzo;
            mpIspSyncCtrlHw->queryImgBufInfo(
                    src.mBuffer,
                    magicNum,
                    isRrzo,
                    pPrivateData,
                    privateDataSize);
            p2InCrop.p_integral = MPoint(0, 0);
            p2InCrop.p_fractional = MPoint(0, 0);
            p2InCrop.s = src.mBuffer->getImgSize();
        }
        //
        if( !isPreviewPath() )
            magicNum = SetCap(magicNum);
        //
        enqueParams.mvPrivaData.push_back(pPrivateData);
        enqueParams.mvMagicNo.push_back(magicNum);
        enqueParams.mvIn.push_back(src);
        //
        srcSize = src.mBuffer->getImgSize();
        timestamp = src.mBuffer->getTimestamp();
        //
        MY_LOGD("i %d data %d, buf 0x%x, va 0x%x, # 0x%X, type %d cnt %d",
                index,
                pData->src.data,
                src.mBuffer,
                src.mBuffer->getBufVA(0),
                magicNum,
                mPass2Type,
                muEnqFrameCnt);
        //output
        //
        MCrpRsInfo crop1;
        crop1.mGroupID    = 1;
        crop1.mCropRect.s = srcSize;
        crop1.mResizeDst  = srcSize;
        //
        MCrpRsInfo crop2;
        crop2.mGroupID    = 2;
        crop2.mCropRect   = p2InCrop;
        //crop2.mResizeDst = MSize(0,0);
        // output
        MUINT32 i = 0;
        for( vector<ImgRequest>::const_iterator pOutReq = pData->vDstReq.begin();
                pOutReq != pData->vDstReq.end(); pOutReq++, i++ )
        {
            const MUINT32 dstDataType = pData->vDstData[i];
            Output dst;
            IImageBuffer* pDstBuf = const_cast<IImageBuffer*>(pOutReq->mBuffer);
            //
            MY_LOGD("data %d, buf 0x%x, va 0x%x, tans %d usg %d",
                    dstDataType,
                    pDstBuf,
                    pDstBuf->getBufVA(0),
                    pOutReq->mTransform,
                    pOutReq->mUsage);
            //
            dst.mPortID             = mapToPortID(dstDataType);
            dst.mPortID.group       = index;
            dst.mPortID.capbility   = (NSIoPipe::EPortCapbility)(pOutReq->mUsage);
            //
            dst.mBuffer     = pDstBuf;
            dst.mTransform  = pOutReq->mTransform;
            //
            dst.mBuffer->setTimestamp(timestamp);
            //
            enqueParams.mvOut.push_back(dst);
            //
            if(mapToPortID(dstDataType) == IMG2O)
            {
                crop1.mCropRect = p2InCrop;
                crop1.mResizeDst = dst.mBuffer->getImgSize();
                MY_LOGD("IMG2O:B(%p),S(%dx%d),C(%d,%d,%dx%d)",
                        dst.mBuffer,
                        crop1.mResizeDst.w,
                        crop1.mResizeDst.h,
                        crop1.mCropRect.p_integral.x,
                        crop1.mCropRect.p_integral.y,
                        crop1.mCropRect.s.w,
                        crop1.mCropRect.s.h);
            }
        }
        //
        enqueParams.mvCropRsInfo.push_back( crop1 );
        enqueParams.mvCropRsInfo.push_back( crop2 );
        //
        // next src/dst pair
        pData++;
        muEnqFrameCnt++;
        index++;
    }
    //
    configFeature();

    //
#if PASS2_CALLBACL_ENABLE
    if( doCallback )
    {
        enqueParams.mpfnCallback = pass2CbFunc;
        enqueParams.mpCookie = this;
    }
    else
#endif
    {
        enqueParams.mpfnCallback = NULL;
        enqueParams.mpCookie = NULL;
    }
    //
    if( !mpIspSyncCtrlHw->lockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync lock pass2 failed");
        return MFALSE;
    }
    //
    CAM_TRACE_FMT_BEGIN("enqP2:%d", trace_begin);
    if( !mpPostProcPipe->enque(enqueParams) )
    {
        CAM_TRACE_FMT_END();
        MY_LOGE("enque pass2 failed");
        AEE_ASSERT("ISP pass2 enque fail");
        return MFALSE;
    }
    CAM_TRACE_FMT_END();
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);

    PostBufInfo postBufData = {data, buf, ext};
    mlPostBufData.push_back(postBufData);

    MY_LOGD("size %d, data %d, buf 0x%x",
            mlPostBufData.size(),
            postBufData.data,
            postBufData.buf);

    CAM_TRACE_INT("P2BufCnt", mlPostBufData.size());

    muPostFrameCnt++;

    if( isReadyToEnque() )
    {
        triggerLoop();
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Pass2NodeImpl::
handleP2Done(QParams& rParams)
{
    CAM_TRACE_FMT_BEGIN("deqP2:%d", muDeqFrameCnt);
    MBOOL ret = MFALSE;
    Vector<Input>::const_iterator iterIn;
    Vector<Output>::const_iterator iterOut;
    vector<IImageBuffer*> vpDstBufAddr;
    //
    if(rParams.mDequeSuccess == MFALSE)
    {
        MY_LOGE("type %d pass2 cnt %d: deque fail",
                mPass2Type,
                muDeqFrameCnt);
        aee_system_exception(
            LOG_TAG,
            NULL,
            DB_OPT_DEFAULT,
            "\nCRDISPATCH_KEY:MtkCam/P1Node:ISP pass2 deque fail");
    }
    //
    MY_LOGD("type %d: cnt %d in %d out %d",
            mPass2Type,
            muDeqFrameCnt,
            rParams.mvIn.size(),
            rParams.mvOut.size());
    //
    if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync unlock pass2 failed");
        goto lbExit;
    }
    //
    for( iterIn = rParams.mvIn.begin() ; iterIn != rParams.mvIn.end() ; iterIn++ )
    {
        //MY_LOGD("In PortID(0x%08X)",portId);
        MUINT32 nodeDataType = mapToNodeDataType( iterIn->mPortID );
        handleReturnBuffer( nodeDataType, (MUINTPTR)iterIn->mBuffer, 0 );
        //
    }
    //
    vpDstBufAddr.clear();
    for( iterOut = rParams.mvOut.begin() ; iterOut != rParams.mvOut.end() ; iterOut++ )
    {
        MBOOL bFind = MFALSE;
        for(MUINT32 i=0; i<vpDstBufAddr.size(); i++)
        {
            if(vpDstBufAddr[i] == iterOut->mBuffer)
            {
                MY_LOGD("Buf(0x%X) has been posted",(MUINTPTR)iterOut->mBuffer);
                bFind = MTRUE;
                break;
            }
        }
        if(!bFind)
        {
            //MY_LOGD("Out PortID(0x%08X)",portId);
            MUINT32 nodeDataType = mapToNodeDataType( iterOut->mPortID );
            handlePostBuffer( nodeDataType, (MUINTPTR)iterOut->mBuffer, 0 );
            vpDstBufAddr.push_back(iterOut->mBuffer);
        }
    }

    {
        Mutex::Autolock lock(mLock);
        muDeqFrameCnt += rParams.mvIn.size();
        mCondDeque.broadcast();
    }

    ret = MTRUE;
lbExit:
    CAM_TRACE_FMT_END();
    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MVOID
Pass2NodeImpl::
pass2CbFunc(QParams& rParams)
{
     Pass2NodeImpl* pPass2NodeImpl = (Pass2NodeImpl*)(rParams.mpCookie);
     pPass2NodeImpl->handleP2Done(rParams);
}


/*******************************************************************************
*
********************************************************************************/
PortID
Pass2NodeImpl::
mapToPortID(MUINT32 const nodeDataType)
{
    //hard-coded mapping
    switch(nodeDataType)
    {
        case PASS2_PRV_SRC:
        case PASS2_CAP_SRC:
            return IMGI;
            break;
        case PASS2_PRV_DST_0:
        case PASS2_CAP_DST_0:
            return WDMAO;
            break;
        case PASS2_PRV_DST_1:
        case PASS2_CAP_DST_1:
            return WROTO;
            break;
        case PASS2_PRV_DST_2:
            return IMG2O;
            break;
        case PASS2_PRV_DST_3:
            return VENC;
            break;
        case PASS2_CAP_DST_2:
            return IMG3O;
        default:
            MY_LOGE("wrong data type %d", nodeDataType);
            break;
    }
    return PortID();
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
Pass2NodeImpl::
mapToNodeDataType(PortID const portId)
{
    PortID _portId = portId;
    _portId.group = 0;
    _portId.capbility = (NSIoPipe::EPortCapbility)0;
    //hard-coded mapping
    if( _portId == IMGI )
        return isPreviewPath() ? PASS2_PRV_SRC : PASS2_CAP_SRC;
    else
    if( _portId == WDMAO )
        return isPreviewPath() ? PASS2_PRV_DST_0 : PASS2_CAP_DST_0;
    else
    if( _portId == WROTO )
        return isPreviewPath() ? PASS2_PRV_DST_1 : PASS2_CAP_DST_1;
    else
    if( _portId == IMG2O )
        return PASS2_PRV_DST_2;
    else
    if( _portId == VENC)
        return PASS2_PRV_DST_3;
    else
    if( _portId == IMG3O )
        return PASS2_CAP_DST_2;
    MY_LOGE("wrong portid");
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

