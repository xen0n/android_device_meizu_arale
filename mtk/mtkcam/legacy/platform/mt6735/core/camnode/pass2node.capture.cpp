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
using namespace NSCam;
//
// for debug dump
#include <cutils/properties.h>
#include <mtkcam/utils/common.h>
using namespace NSCam::Utils;
//
#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;
//
#include <mtkcam/featureio/capturenr.h>
#include <mtkcam/iopipe/SImager/IImageTransform.h>
using namespace NSIoPipe::NSSImager;
//
#include "./inc/pass2nodeImpl.h"
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

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define DUMP_PROCESSEDRAW    (0x1)
#define DUMP_HWNR            (0x2)
#define DUMP_SWNR            (0x4)

/*******************************************************************************
*
********************************************************************************/
CapPass2::
CapPass2(Pass2NodeType const type)
    : Pass2NodeImpl(type)
    , muNRType(0)
    , mpInternalNR1Buf(NULL)
    , mpInternalNR2Buf(NULL)
    , mpInternalPurerawBuf(NULL)
    , mpSwNR(NULL)
    , muDumpFlag(0)
{
    addDataSupport( ENDPOINT_SRC , PASS2_CAP_SRC);
    addDataSupport( ENDPOINT_DST , PASS2_CAP_DST_0);
    addDataSupport( ENDPOINT_DST , PASS2_CAP_DST_1);
    addDataSupport( ENDPOINT_DST , PASS2_CAP_DST_2);

    muDequeOrder[0] = PASS2_CAP_DST_1;
    muDequeOrder[1] = PASS2_CAP_DST_0;
    muDequeOrder[2] = PASS2_CAP_DST_2;
    muDequeOrder[3] = 0;

    // debug dump option
    // 0: disable, 1: pure-raw, 2: hwnr, 4: swnr
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.p2node.dump", value, "0");
    muDumpFlag = ::atoi(value);
}


/*******************************************************************************
*
********************************************************************************/
CapPass2::
~CapPass2()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
moduleInit()
{
    MBOOL ret = MFALSE;

    switch( mPass2Type )
    {
        case PASS2_CAPTURE:
        case PASS2_PURERAW_CAPTURE:
            mpPostProcPipe = INormalStream::createInstance(getName(), ENormalStreamTag_Cap, getSensorIdx());
            break;
        case PASS2_VSS:
            mpPostProcPipe = INormalStream::createInstance(getName(), ENormalStreamTag_Vss, getSensorIdx());
            break;
        default:
            MY_LOGE("not support type(0x%x)", mPass2Type);
            break;
    }

    if( mpPostProcPipe == NULL )
    {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }

    if( !mpPostProcPipe->init() )
    {
        MY_LOGE("postproc pipe init failed");
        goto lbExit;
    }

    if( muNRType ) {
        MY_LOGD("use NR type %d", muNRType);
    }

    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
moduleStop()
{
    MBOOL ret = MTRUE;
    if( !freeMemory(mpInternalNR1Buf) ) {
        MY_LOGE("free memory failed");
        ret = MFALSE;
    }

    if( !freeMemory(mpInternalNR2Buf) ) {
        MY_LOGE("free memory failed");
        ret = MFALSE;
    }

    if( mpSwNR )
        delete mpSwNR;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
pureRawPass(IImageBuffer* pSrcBuf, IImageBuffer* pDstBuf)
{
    MBOOL ret = MTRUE;
    IFeatureStream* pStream = IFeatureStream::createInstance(
            LOG_TAG,
            EFeatureStreamTag_PRaw_Stream,
            getSensorIdx());

    if( pStream == NULL ) {
        MY_LOGE("create stream failed");
        return MFALSE;
    }

    if( !pStream->init() ) {
        MY_LOGE("pure raw init failed");
        return MFALSE;
    }

    QParams enqueParams;
    MUINT32 magicNum;
    MBOOL   isRrzo;
    MVOID*  pPrivateData;
    MUINT32 privateDataSize;

    enqueParams.mvIn.reserve(1);
    enqueParams.mvOut.reserve(1);

    // query
    mpIspSyncCtrlHw->queryImgBufInfo(
                        pSrcBuf,
                        magicNum,
                        isRrzo,
                        pPrivateData,
                        privateDataSize);

    if( !isPreviewPath() )
        magicNum = SetCap(magicNum);

    enqueParams.mpPrivaData = pPrivateData;
    enqueParams.mFrameNo = magicNum;

    {    //input
        MY_LOGD("pureraw in: buf 0x%x, va 0x%x, # 0x%X, type %d",
                pSrcBuf, pSrcBuf->getBufVA(0), magicNum, mPass2Type);
        Input src;
        src.mPortID = IMGI;
        src.mBuffer = pSrcBuf;
        enqueParams.mvIn.push_back(src);
    }

    {    //output
        MY_LOGD("pureraw out: buf 0x%x, va 0x%x",
                pDstBuf, pDstBuf->getBufVA(0));
        Output dst;
        dst.mPortID    = IMGO;
        dst.mBuffer    = pDstBuf;
        dst.mTransform = 0;
        enqueParams.mvOut.push_back(dst);
    }

    if( !mpIspSyncCtrlHw->lockHw(IspSyncControlHw::HW_PURERAW) )
    {
        MY_LOGE("isp sync lock pass2 failed");
        return MFALSE;
    }

    if( !pStream->enque(enqueParams) )
    {
        MY_LOGE("enque pure-raw pass2 failed");
        return MFALSE;
    }

    {
        QParams dequeParams;
        MY_LOGD("pure-raw deque + ");
        if( !pStream->deque(dequeParams) )
        {
            MY_LOGE("pureraw pass2: deque fail");
            AEE_ASSERT("ISP pure-raw p2 deque fail");
            return MFALSE;
        }
        MY_LOGD("pure-raw deque - ");
    }

    if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PURERAW) )
    {
        MY_LOGE("isp sync unlock pass2 failed");
        return MFALSE;
    }

    if( !pStream->uninit() ) {
        MY_LOGE("pure raw uninit failed");
        return MFALSE;
    }

    pDstBuf->setTimestamp( pSrcBuf->getTimestamp() );

    pStream->destroyInstance(LOG_TAG);
    pStream = NULL;

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
transform(
        IImageBuffer* src,
        IImageBuffer* dst0,
        IImageBuffer* dst1,
        MRect const crop,
        MINT32 const transform0)
{
    MBOOL ret = MTRUE;
    IImageTransform* pImgTransform = NULL;
    //
    if( src == NULL || (dst0==NULL && dst1==NULL) )
    {
        ret = MFALSE;
        goto lbExit;
    }
    //
    pImgTransform = IImageTransform::createInstance();
    if( !pImgTransform )
    {
        MY_LOGE("imageTransform create failed");
        return MFALSE;
    }
    //
    ret = pImgTransform->execute(
            src,
            dst0,
            dst1,
            crop,
            transform0,
            0xFFFFFFFF);
    //
    if( pImgTransform )
    {
        pImgTransform->destroyInstance();
        pImgTransform = NULL;
    }
    //
    if( dst0 )
        dst0->setTimestamp( src->getTimestamp() );
    if( dst1 )
        dst1->setTimestamp( src->getTimestamp() );
    //
lbExit:
    if( !ret )
    {
        if( src )
        {
            MY_LOGE("src: buf 0x%x, %dx%d, fmt 0x%x",
                    src,
                    src->getImgSize().w,
                    src->getImgSize().h,
                    src->getImgFormat());
        }
        if( dst0 )
        {
            MY_LOGE("dst0: buf 0x%x, %dx%d, fmt 0x%x",
                    dst0,
                    dst0->getImgSize().w,
                    dst0->getImgSize().h,
                    dst0->getImgFormat());
        }
        if( dst1 )
        {
            MY_LOGE("dst1: buf 0x%x, %dx%d, fmt 0x%x",
                    dst1,
                    dst1->getImgSize().w,
                    dst1->getImgSize().h,
                    dst1->getImgFormat());
        }
    }
    //
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
threadLoopUpdate()
{
    MBOOL ret = MFALSE;

    // no pure-raw path & nr path
    if( mPass2Type != PASS2_PURERAW_CAPTURE && muNRType == 0)
    {
        return enquePass2(MTRUE);
    }

    if( mPass2Type == PASS2_PURERAW_CAPTURE )
    {
        PostBufInfo postBufData;
        {
            Mutex::Autolock lock(mLock);

            if( mlPostBufData.size() == 0 ) {
                MY_LOGE("no posted buf");
                goto lbExit;
            }

            postBufData = mlPostBufData.front();
            mlPostBufData.pop_front();
        }

        if( mpInternalPurerawBuf.get() == NULL &&
                !allocMemory(postBufData.buf, postBufData.buf->getImgFormat(), mpInternalPurerawBuf) )
        {
            MY_LOGE("allocate memory failed");
            goto lbExit;
        }

        if( !createFakeResultMetadata( postBufData.buf, mpInternalPurerawBuf.get()) )
        {
            MY_LOGE("cannot create fake resultmetadata");
            goto lbExit;
        }

        // pure-raw process
        if( !pureRawPass( postBufData.buf, mpInternalPurerawBuf.get() ) ){
            MY_LOGE("pure raw pass failed");
            goto lbExit;
        }

        if( muDumpFlag & DUMP_PROCESSEDRAW )
        {
            dumpBuffer(mpInternalPurerawBuf.get(), "processed_raw", "raw");
        }

        handleReturnBuffer(PASS2_CAP_SRC, (MUINTPTR)postBufData.buf);

        {
            Mutex::Autolock lock(mLock);

            PostBufInfo postBufData = {PASS2_CAP_SRC, mpInternalPurerawBuf.get(), postBufData.ext};
            mlPostBufData.push_front(postBufData);
        }

        // normal p2 path
        if( !enquePass2(MFALSE) )
        {
            MY_LOGD("enque p2 failed");
            goto lbExit;
        }

        if( muNRType == 1 ) //HW NR
        {
            if( mpInternalNR2Buf.get() == NULL &&
                    !allocMemory(
                        mpInternalNR1Buf.get(),
                        eImgFmt_YUY2,
                        mpInternalNR2Buf) )
            {
                MY_LOGE("failed to allocate memory, nr2");
            }
        }

        if( !dequePass2() )
        {
            MY_LOGD("deque p2 failed");
            goto lbExit;
        }

        if( !destroyFakeResultMetadata(mpInternalPurerawBuf.get()) )
        {
            MY_LOGE("cannot destroy fake resultmetadata");
            goto lbExit;
        }

        if( !freeMemory(mpInternalPurerawBuf) ) {
            MY_LOGE("free memory failed");
            goto lbExit;
        }
    } else {
        // normal p2 path
        if( !enquePass2(MFALSE) )
        {
            MY_LOGE("enque p2 failed");
            goto lbExit;
        }

        if( muNRType == 1 ) //HW NR
        {
            if( mpInternalNR2Buf.get() == NULL &&
                    !allocMemory(
                        mpInternalNR1Buf.get(),
                        eImgFmt_YUY2,
                        mpInternalNR2Buf) )
            {
                MY_LOGE("failed to allocate memory, nr2");
            }
        }

        if( !dequePass2() )
        {
            MY_LOGE("deque p2 failed");
            goto lbExit;
        }
    }

    // try to allocate NR buffer if needed
    if( muNRType != 0 )
    {
        IImageBuffer* out[2] = {NULL, NULL};
        MINT32 trans = 0;
        if( mvOutRequest.size() == 0 || mvOutRequest.size() > 2 ||
                mvOutRequest.size() != mvOutData.size() )
        {
            MY_LOGE("wrong request count %d, %d",
                    mvOutRequest.size(),
                    mvOutData.size());
            goto lbExit;
        }

        trans = mvOutRequest[0].mTransform;
        for( int i = 0 ; i < mvOutRequest.size() ; i++ )
        {
            out[i] = const_cast<IImageBuffer*>(mvOutRequest[i].mBuffer);
        }

        if( muNRType == 1 ) //HW NR
        {
            MY_LOGD("HWNR pass +");

            if( !doHwNR(getSensorIdx(),
                        mpInternalNR1Buf.get(),
                        mpInternalNR2Buf.get(),
                        NULL,
                        MRect(MPoint(0), mpInternalNR1Buf.get()->getImgSize()),
                        0,
                        mPass2Type != PASS2_VSS ?
                        EIspProfile_Capture_MultiPass_ANR_1 : EIspProfile_VSS_Capture_MultiPass_ANR_1 ) ) {
                MY_LOGE("NR pass 1 failed");
                goto lbExit;
            }

            if( muDumpFlag & DUMP_HWNR )
            {
                dumpBuffer(mpInternalNR1Buf.get(),
                        "nr_original", "raw");
                dumpBuffer(mpInternalNR2Buf.get(), "hwnr_nr1", "yuv");
            }

            if( !doHwNR(getSensorIdx(),
                        mpInternalNR2Buf.get(),
                        out[0],
                        out[1],
                        mCapCrop,
                        trans,
                        mPass2Type != PASS2_VSS ?
                        EIspProfile_Capture_MultiPass_ANR_2 : EIspProfile_VSS_Capture_MultiPass_ANR_2 ) ) {
                MY_LOGE("NR pass 2 failed");
                goto lbExit;
            }

            if( muDumpFlag & DUMP_HWNR )
            {
                dumpBuffer(out[0], "hwnr_nr2_main", "yuv");
                dumpBuffer(out[1], "hwnr_nr2_post", "yuv");
            }

            MY_LOGD("HWNR pass -");
        }
        else if ( muNRType == 2 ) //SW NR
        {
            MY_LOGD("SWNR pass +");
            if( !mpSwNR )
                mpSwNR = new SwNR( getSensorIdx() );

            if( muDumpFlag & DUMP_SWNR )
            {
                dumpBuffer(mpInternalNR1Buf.get(), "swnr_original", "yuv");
            }

            if( !mpSwNR->doSwNR(mpInternalNR1Buf.get()) )
            {
                MY_LOGE("SW NR failed");
                goto lbExit;
            }

            if( muDumpFlag & DUMP_SWNR )
            {
                dumpBuffer(mpInternalNR1Buf.get(), "swnr_nr", "yuv");
            }
            // format transform
            if( !transform(mpInternalNR1Buf.get(), out[0], out[1], mCapCrop, trans) )
            {
                MY_LOGE("format transform failed");
                goto lbExit;
            }

            if( muDumpFlag & DUMP_SWNR )
            {
                dumpBuffer(out[0], "swnr_trans", "yuv");
                dumpBuffer(out[1], "hwnr_nr2_post", "yuv");
            }
            //
            MY_LOGD("SWNR pass -");
        }
        //
        if( mpInternalNR1Buf.get() )
        {
            if( out[0] )
                out[0]->setTimestamp( mpInternalNR1Buf->getTimestamp() );
            if( out[1] )
                out[1]->setTimestamp( mpInternalNR1Buf->getTimestamp() );
        }
        //
        for( int i = 0 ; i < mvOutRequest.size() ; i++ )
        {
            handlePostBuffer( mvOutData[i], (MUINTPTR)mvOutRequest[i].mBuffer );
        }
        mvOutRequest.clear();
        mvOutData.clear();
    }

    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
getPass2Buffer(vector<p2data>& vP2data)
{
    MBOOL haveDst = MFALSE;
    p2data one;
    IImageBuffer* pSrcBuf = NULL;
    // src
    {
        Mutex::Autolock lock(mLock);
        //
        if( mlPostBufData.size() == 0 ) {
            MY_LOGE("no posted buf");
            return MFALSE;
        }
        //
        one.src = mlPostBufData.front();
        pSrcBuf = one.src.buf;
        mlPostBufData.pop_front();
    }

    // get output bufffers
    for(MUINT32 i=0; i<MAX_DST_PORT_NUM; i++)
    {
        MBOOL ret;
        ImgRequest outRequest;
        //
        if(muDequeOrder[i] == 0)
            break;

        ret = getDstBuffer(
                muDequeOrder[i],
                &outRequest);
        //
        if(ret)
        {
            haveDst = MTRUE;

            one.vDstReq.push_back(outRequest);
            one.vDstData.push_back(muDequeOrder[i]);
        }
    }

    if( one.vDstReq[0].mTransform & eTransform_ROT_90 ) //90, 270
    {
        MSize temp = one.vDstReq[0].mBuffer->getImgSize();
        one.dstSize.w = temp.h;
        one.dstSize.h = temp.w;
    }
    else
    {
        one.dstSize = one.vDstReq[0].mBuffer->getImgSize();
    }

    if( muNRType == 0 )
    {
        one.doCrop = MTRUE;
    }
    else
    {
        MBOOL alloc_ret = MTRUE;
        ImgRequest outRequest;
        //
        // allocate internal buffers
        if ( muNRType == 1 )
        {
            // allocate internal buffer
            if( mpInternalNR1Buf.get() == NULL &&
                    !allocMemory(
                        pSrcBuf,
                        eImgFmt_YUY2,
                        mpInternalNR1Buf) )
            {
                MY_LOGE("failed to allocate memory, nr1");
                alloc_ret = MFALSE;
            }
#if 0
            if( mpInternalNR2Buf.get() == NULL &&
                    !allocMemory(
                        pSrcBuf,
                        eImgFmt_YUY2,
                        mpInternalNR2Buf) )
            {
                MY_LOGE("failed to allocate memory, nr2");
                alloc_ret = MFALSE;
            }
#endif
        }
        else if ( muNRType == 2 )
        {
            if( mpInternalNR1Buf.get() == NULL &&
                    !allocMemory(
                        pSrcBuf,
                        eImgFmt_I420,
                        mpInternalNR1Buf) )
            {
                MY_LOGE("failed to allocate memory, nr1");
                alloc_ret = MFALSE;
            }
        }

        if( alloc_ret )
        {
            one.doCrop = MFALSE;
            //
            // keep request
            mvOutRequest = one.vDstReq;
            mvOutData = one.vDstData;
            one.vDstReq.clear();
            one.vDstData.clear();

            if( mpInternalNR1Buf.get() )
            {
                outRequest.mBuffer = mpInternalNR1Buf.get();
                outRequest.mTransform = 0;
                outRequest.mUsage = NSIoPipe::EPortCapbility_Cap;
            }
            one.vDstReq.push_back(outRequest);
            one.vDstData.push_back(PASS2_CAP_DST_1);

            {
                MUINT32 magicNum;
                MVOID*  pPrivateData;
                MUINT32 privateDataSize;
                MCropRect p2InCrop;
                if( mpIspSyncCtrlHw->getPass2Info(
                        pSrcBuf,
                        one.dstSize,
                        magicNum,
                        pPrivateData,
                        privateDataSize,
                        p2InCrop) )
                {
                    mCapCrop.p = p2InCrop.p_integral;
                    mCapCrop.s = p2InCrop.s;
                }
                else
                {
                    MY_LOGE("can't get p2 crop info");
                    mCapCrop.p = MPoint(0);
                    mCapCrop.s = pSrcBuf->getImgSize();
                }
            }
        }
        else
        {
            MY_LOGW("cannot alloc memory, no NR path");
            //error handling
            one.doCrop = MTRUE;
            muNRType = 0;

            if( !freeMemory(mpInternalNR1Buf) ) {
                MY_LOGE("free memory failed");
            }

            if( !freeMemory(mpInternalNR2Buf) ) {
                MY_LOGE("free memory failed");
            }
        }
    }
    //
    if( !haveDst )
    {
        MY_LOGW("no dst buffer, skip data %d, buf 0x%x", one.src.data, one.src.buf);
        handleReturnBuffer(one.src.data, (MUINTPTR)one.src.buf);
        return MFALSE; //return false if no buffer
    }
    //
    vP2data.push_back(one);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
handleP2Done(QParams& rParams)
{
    CAM_TRACE_FMT_BEGIN("cap_deqP2:%d", muDeqFrameCnt);
    MBOOL ret = MFALSE;
    Vector<Input>::const_iterator iterIn;
    Vector<Output>::const_iterator iterOut;
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
    MY_LOGD("type %d cnt %d in %d out %d",
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
        if( iterIn->mBuffer == mpInternalPurerawBuf.get() )
        {
            // internal buffer should not be passed to other stage
            continue;
        }

        MUINT32 nodeDataType = mapToNodeDataType( iterIn->mPortID );
        handleReturnBuffer( nodeDataType, (MUINTPTR)iterIn->mBuffer, 0 );
    }
    //
    if( muNRType == 0 )
    {
        for( iterOut = rParams.mvOut.begin() ; iterOut != rParams.mvOut.end() ; iterOut++ )
        {
            MUINT32 nodeDataType = mapToNodeDataType( iterOut->mPortID );
            handlePostBuffer( nodeDataType, (MUINTPTR)iterOut->mBuffer, 0 );
        }
    }
    //
    {
        Mutex::Autolock lock(mLock);
        muDeqFrameCnt += rParams.mvIn.size();
        mCondDeque.broadcast();
    }
    //
    ret = MTRUE;
lbExit:
    CAM_TRACE_FMT_END();
    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
createFakeResultMetadata(IImageBuffer* srcbuf, IImageBuffer* dstbuf)
{
    MUINT32 magicNum;
    MBOOL   isRrzo;
    MVOID*  pPrivateData;
    MUINT32 privateDataSize;

    // query
    if( !mpIspSyncCtrlHw->queryImgBufInfo(
                        srcbuf,
                        magicNum,
                        isRrzo,
                        pPrivateData,
                        privateDataSize) )
    {
        MY_LOGE("cannot find infos");
        return MFALSE;
    }

    {   //fake infos
        ResultMetadata metadata(
                MRect( MPoint(0,0), srcbuf->getImgSize() ),
                0,
                magicNum,
                magicNum,
                0,
                srcbuf->getTimestamp(),
                pPrivateData,
                privateDataSize
                );

        // fake one for p2
        mpIspSyncCtrlHw->addPass1Info(
                magicNum,
                dstbuf,
                metadata,
                MFALSE);
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
destroyFakeResultMetadata(IImageBuffer* dstbuf)
{
    mpIspSyncCtrlHw->dropPass2Info(dstbuf);
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
allocMemory(IImageBuffer* srcbuf, MINT32 const fmt, sp<IImageBuffer>& targetBuf)
{
    MBOOL ret = MTRUE;
    // allocate internal memory
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    if( allocator == NULL ) {
        MY_LOGE("cannot get allocator");
        return MFALSE;
    }

    MSize const size = srcbuf->getImgSize();
    MUINT const planecount = queryPlaneCount(fmt);
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    MUINT32 bufStridesInBytes[3];

    if( fmt == eImgFmt_BAYER8  ||
        fmt == eImgFmt_BAYER10 ||
        fmt == eImgFmt_BAYER12 ||
        fmt == eImgFmt_BAYER14 )
    {
        for (MUINT i = 0; i < planecount; i++)
        {
            bufStridesInBytes[i] = srcbuf->getBufStridesInBytes(i);
        }
    }
    else
    {
        for (MUINT i = 0; i < planecount; i++)
        {
            bufStridesInBytes[i] =
                (queryPlaneWidthInPixels(fmt,i, size.w)*queryPlaneBitsPerPixel(fmt,i))>>3;
        }
    }

    MY_LOGD("alloc %d x %d, fmt 0x%x", size.w, size.h, fmt);
    IImageBufferAllocator::ImgParam imgParam(
            fmt,
            size,
            bufStridesInBytes,
            bufBoundaryInBytes,
            planecount
            );

    targetBuf = allocator->alloc_ion(LOG_TAG, imgParam);

    if( targetBuf.get() == NULL )
    {
        MY_LOGE("failed to allocate memory");
        goto lbExit;
    }

    if ( !targetBuf->lockBuf( LOG_TAG, eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK ) )
    {
        MY_LOGE("lock Buffer failed\n");
        goto lbExit;
    }

    if ( !targetBuf->syncCache( eCACHECTRL_INVALID ) )
    {
        MY_LOGE("syncCache failed");
        goto lbExit;
    }

    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CapPass2::
freeMemory(sp<IImageBuffer>& targetBuf)
{
    MBOOL ret = MFALSE;
    if( targetBuf.get() != NULL ) {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        if( !targetBuf->unlockBuf( LOG_TAG ) )
        {
            MY_LOGE("unlock Buffer failed\n");
            goto lbExit;
        }
        allocator->free(targetBuf.get());
        targetBuf = NULL;
    }

    ret = MTRUE;
lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
CapPass2::
dumpBuffer(IImageBuffer* pBuf, const char* filename, const char* fileext)
{
#define dumppath "/sdcard/cameradump_p2node"
    char fname[256];

    if( pBuf == NULL )
        return;

    if(!makePath(dumppath,0660))
    {
        MY_LOGE("makePath [%s] fail",dumppath);
        return;
    }

    sprintf(fname, "%s/%s_%dx%d_%d.%s",
            dumppath,
            filename,
            pBuf->getImgSize().w,
            pBuf->getImgSize().h,
            muDeqFrameCnt,
            fileext
            );
    pBuf->saveToFile(fname);
#undef dumppath
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

