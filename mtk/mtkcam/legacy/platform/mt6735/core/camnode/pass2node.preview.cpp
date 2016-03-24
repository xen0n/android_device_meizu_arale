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
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/v1/config/PriorityDefs.h>
using namespace NSCam;
//
#include <sys/resource.h>
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

#define MY_LOGD_IF(cond, fmt, arg...) CAM_LOGD_IF(cond, "[%d:%s] " fmt, getSensorIdx(), __func__, ##arg)
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
#define MY_LOGD_IF(cond, fmt, arg...) CAM_LOGD("[%d:%s] " fmt, getSensorIdx(), __func__, ##arg); \
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


/*******************************************************************************
*
********************************************************************************/
PrvPass2*
PrvPass2::
createInstance(Pass2NodeType const type)
{
    switch(type)
    {
        case PASS2_PREVIEW:
            return new PrvPass2(type);
        case PASS2_FEATURE:
            return new FeaturePass2(type);
        default:
            break;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
PrvPass2::
PrvPass2(Pass2NodeType const type)
    : Pass2NodeImpl(type)
    , mbRecording(MFALSE)
    , mbPreview(MFALSE)
    , mbRecBufYuv(MTRUE)
    , mVdoSize()
{
    addDataSupport( ENDPOINT_SRC , PASS2_PRV_SRC);
    addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_0);
    addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_1);
    addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_2);
    addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_3);

    muDequeOrder[0] = PASS2_PRV_DST_0;
    muDequeOrder[1] = PASS2_PRV_DST_1;
    muDequeOrder[2] = PASS2_PRV_DST_2;
    muDequeOrder[3] = PASS2_PRV_DST_3;
}


/*******************************************************************************
*
********************************************************************************/
PrvPass2::
~PrvPass2()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
startPreview(
    MINT32  width,
    MINT32  height,
    MINT32  fps)
{
    FUNC_START;
    //
    MY_LOGD("%dx%d",
            width,
            height);
    mbPreview = MTRUE;
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
stopPreview()
{
    FUNC_START;
    //
    mbPreview = MFALSE;
    //
    FUNC_END;
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
startRecording(
    MUINT32 width,
    MUINT32 height,
    MUINT32 fps,
    MBOOL   isYuv)
{
    FUNC_START;
    //
    MY_LOGD("Size(%dx%d),FPS(%d),isYUV(%d)",
            width,
            height,
            fps,
            isYuv);
    //
    MBOOL ret = MFALSE;
    mVdoSize.w = width;
    mVdoSize.h = height;
    mbRecBufYuv = isYuv;
    //
    if(mpPostProcPipe)
    {
        Mutex::Autolock lock(mRecordingLock);
        mbRecording = MTRUE;
        if(mbRecBufYuv == MFALSE)
        {
            ret = mpPostProcPipe->startVideoRecord(
                                    width,
                                    height,
                                    fps);
        }
    }
    else
    {
        MY_LOGE("mpPostProcPipe is NULL");
    }
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
stopRecording()
{
    FUNC_START;
    //
    MBOOL ret = MFALSE;
    //
    if(mpPostProcPipe)
    {
        Mutex::Autolock lock(mRecordingLock);
        if(mbRecBufYuv == MFALSE)
        {
            mVdoSize.w = 0;
            mVdoSize.h = 0;
            ret = mpPostProcPipe->stopVideoRecord();
        }
        mbRecording = MFALSE;
    }
    else
    {
        MY_LOGE("mpPostProcPipe is NULL");
    }
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
threadInit()
{
    if( muMultiFrameNum > 1 )
    {
        // normal, for slow-motion
        ::setpriority(PRIO_PROCESS, 0, NICE_CAMERA_SM_PASS2);
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
threadUninit()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
threadLoopUpdate()
{
    MY_LOGV("+");

    MBOOL ret = MTRUE;

    // featurepipe init optimization
    if( doInitialization() )
    {
        goto lbExit;
    }

#if MULTI_FRAME_ENABLE
    ret = enquePass2(MTRUE);
#else
#if PASS2_CALLBACL_ENABLE
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
    ret = enquePass2(postBufData.data, postBufData.buf, postBufData.ext);
#else
    // use p2 thread to deque
    ret = dequePass2();
#endif
#endif

lbExit:
    MY_LOGV("-");
    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
moduleInit()
{
    MBOOL ret = MFALSE;

    mpPostProcPipe = createPipe();
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

    muMultiFrameNum = mpIspSyncCtrlHw->getIspEnquePeriod();

    ret = MTRUE;

lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
isReadyToEnque() const
{
#if MULTI_FRAME_ENABLE
    MY_LOGD_IF(muMultiFrameNum > 1, "%d/%d", muPostFrameCnt, muMultiFrameNum);
    return (muPostFrameCnt % muMultiFrameNum) == 0;
#else
    return MTRUE;
#endif
}


/*******************************************************************************
*
********************************************************************************/
IHalPostProcPipe*
PrvPass2::
createPipe()
{
    if( mPass2Type == PASS2_PREVIEW ) {
        return INormalStream::createInstance(getName(), ENormalStreamTag_Prv, getSensorIdx());
    }

    MY_LOGE("not support type(0x%x)", mPass2Type);
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PrvPass2::
getPass2Buffer(vector<p2data>& vP2data)
{
    MBOOL haveDst = MFALSE;
    // src
    {
        Mutex::Autolock lock(mLock);
        p2data one;
        MUINT32 count = 0;
        //
        if( mlPostBufData.size() < muMultiFrameNum )
        {
            MY_LOGE("posted buf not enough %d < %d",
                    mlPostBufData.size(),
                    muMultiFrameNum);
            return MFALSE;
        }
        //
        list<PostBufInfo>::iterator iter = mlPostBufData.begin();
        while( iter != mlPostBufData.end() )
        {
            one.src = *iter;
            iter = mlPostBufData.erase(iter);
            //
            vP2data.push_back(one);
            count++;
            //
            if(count == muMultiFrameNum)
            {
                break;
            }
        }
    }
    // dst
    {
        MBOOL bDequeDisplay = MTRUE;

        vector<p2data>::iterator pData = vP2data.begin();
        while( pData != vP2data.end() )
        {
            for(MUINT32 i = 0; i < MAX_DST_PORT_NUM; i++)
            {
                MBOOL ret;
                ImgRequest outRequest;
                //
                if(muDequeOrder[i] == 0)
                    break;
                //
                if( bDequeDisplay == MFALSE &&
                        muDequeOrder[i] == PASS2_PRV_DST_0)
                {
                    continue;
                }
                //
                if( muDequeOrder[i] == PASS2_PRV_DST_3 &&
                    ( mbRecording == MFALSE || mbRecBufYuv == MTRUE ) )
                {
                    continue;
                }
                //
                ret = getDstBuffer(
                        muDequeOrder[i],
                        &outRequest);
                //
                if(ret)
                {
                    haveDst = MTRUE;

                    if(muDequeOrder[i] == PASS2_PRV_DST_0)
                    {
                        bDequeDisplay = MFALSE;
                    }
                    //
                    pData->vDstReq.push_back(outRequest);
                    pData->vDstData.push_back(muDequeOrder[i]);
                }
            }
            //
            if(mbRecording)
            {
                pData->dstSize = mVdoSize;
            }
            else if( pData->vDstReq.size() > 0 )
            {
                if( pData->vDstReq[0].mTransform & eTransform_ROT_90 ) //90, 270
                {
                    MSize temp = pData->vDstReq[0].mBuffer->getImgSize();
                    pData->dstSize.w = temp.h;
                    pData->dstSize.h = temp.w;
                }
                else
                {
                    pData->dstSize = pData->vDstReq[0].mBuffer->getImgSize();
                }
            }
            //
            pData->doCrop = MTRUE;
            //
            pData++;
        }
    }
    //
    if( !haveDst )
    {
        vector<p2data>::iterator pData = vP2data.begin();
        while( pData != vP2data.end() )
        {
            MY_LOGW("no dst buffer, skip data %d, buf 0x%x",
                    pData->src.data, pData->src.buf);
            handleReturnBuffer(pData->src.data, (MUINTPTR)pData->src.buf);
            pData = vP2data.erase(pData);
        }
        return MFALSE;
    }
    //
    // make each src have at least one dst
    MUINT32 pivot = muMultiFrameNum + 1;
    for(MUINT32 i = 0; i < muMultiFrameNum; i++)
    {
        if( vP2data[i].vDstData.size() != 0 )
        {
            if( pivot == muMultiFrameNum + 1 )
            {
                //first pivot
                for(MUINT32 j = 0 ; j < i ; j++ )
                {
                    vP2data[j].vDstReq  = vP2data[i].vDstReq;
                    vP2data[j].vDstData = vP2data[i].vDstData;
                    vP2data[j].dstSize = vP2data[i].dstSize;
                }
            }
            pivot = i;
        }
        else
        {
            if( pivot != muMultiFrameNum + 1 )
            {
                vP2data[i].vDstReq  = vP2data[pivot].vDstReq;
                vP2data[i].vDstData = vP2data[pivot].vDstData;
                vP2data[i].dstSize = vP2data[pivot].dstSize;
            }
        }
    }

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode
