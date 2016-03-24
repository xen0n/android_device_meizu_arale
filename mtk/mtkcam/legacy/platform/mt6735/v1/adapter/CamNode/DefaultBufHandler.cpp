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
#define LOG_TAG "MtkCam/DBHdl"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
#include <camera/MtkCamera.h>
#include <CamUtils.h>
using namespace android;
using namespace MtkCamUtils;
using namespace NSCam::Utils;
//
#include <cutils/properties.h>
//
#include <list>
#include <vector>
using namespace std;
//
#include <mtkcam/iopipe/Port.h>
//
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/camnode/nodeDataTypes.h>
//
#include <CamNode/DefaultBufHandler.h>
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s:%s]" fmt, getName(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s:%s]" fmt, getName(), __func__, ##arg)
#else
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg); \
                                    printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#endif
//
#define FUNC_START  MY_LOGD("+")
#define FUNC_END    MY_LOGD("-")
#define FUNC_NAME   MY_LOGD("")
//
/******************************************************************************
*
*******************************************************************************/
#define GET_IIMGBUF_IMG_STRIDE_IN_BYTE(pBuf, plane)         (Format::queryPlaneCount(Format::queryImageFormat(pBuf->getImgFormat().string())) >= (plane+1)) ? \
                                                            ((pBuf->getImgWidthStride(plane)*Format::queryPlaneBitsPerPixel(Format::queryImageFormat(pBuf->getImgFormat().string()),plane)))>>3 : 0
#define GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pBuf, plane)    (pBuf->getPlaneCount() >= (plane+1)) ? (pBuf->getBufStridesInBytes(plane)) : 0
#define GET_IIMAGEBUFFER_BUF_SIZE(pBuf, plane)              (pBuf->getPlaneCount() >= (plane+1)) ? pBuf->getBufSizeInBytes(plane) : 0
#define GET_IIMAGEBUFFER_BUF_VA(pBuf, plane)                (pBuf->getPlaneCount() >= (plane+1)) ? pBuf->getBufVA(plane) : 0
#define GET_IIMAGEBUFFER_BUF_PA(pBuf, plane)                (pBuf->getPlaneCount() >= (plane+1)) ? pBuf->getBufPA(plane) : 0


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class DefaultBufHandlerImpl : public DefaultBufHandler
{
    public:
        DefaultBufHandlerImpl(
            const char*                 userName,
            sp<ImgBufProvidersManager>  spImgBufProvidersMgr);
        ~DefaultBufHandlerImpl();

        DECLARE_ICAMBUFHANDLER_INTERFACE();
    //
    virtual MBOOL   setForceRotation(
                        MBOOL   bIsForceRotation,
                        MUINT32 rotationAnagle);
    virtual MBOOL   mapPort(
                        EBufProvider    bufType,
                        NodeDataTypes   nodeType,
                        MUINT32         timeout = 0,
                        MBOOL           bPushFront = MFALSE);
    virtual MBOOL   unmapPort(EBufProvider bufType);
    //
    virtual MINT32  mapNode2Dst(MUINT32 nodeType);
    //
    protected:
        //
        typedef struct
        {
            EBufProvider    bufType;
            NodeDataTypes   nodeType;
            MUINT32         timeout;
        }MAP_PORT_INFO;
        //
        enum EPass2Out
        {
            EPass2Out_PRV_DST_0,
            EPass2Out_PRV_DST_1,
            EPass2Out_PRV_DST_2,
            EPass2Out_PRV_DST_3,
            EPass2Out_CAP_DST_0,
            EPass2Out_CAP_DST_1,
            EPass2Out_DST_AMOUNT
        };
        //
        const char* getName() const {return msName;}
        //
    private:
        mutable Mutex               mLock;
        const char* const           msName;
        MBOOL                       mbEnableIImageBufferLog;
        MBOOL                       mbIsForceRotation;
        MUINT32                     mRotationAnagle;
        sp<ImgBufProvidersManager>  mspImgBufProvidersMgr;
        vector<ImgBufQueNode>       mvBufQueNode[EPass2Out_DST_AMOUNT];
        list<MAP_PORT_INFO>         mlMapPort[EPass2Out_DST_AMOUNT];
};


/*******************************************************************************
*
********************************************************************************/
DefaultBufHandler*
DefaultBufHandler::
createInstance(
    const char*                 userName,
    sp<ImgBufProvidersManager>  spImgBufProvidersMgr)
{
    return new DefaultBufHandlerImpl(userName, spImgBufProvidersMgr);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
DefaultBufHandler::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
DefaultBufHandler::
DefaultBufHandler()
    : ICamBufHandler(LOG_TAG)
{
}


/*******************************************************************************
*
********************************************************************************/
DefaultBufHandler::
~DefaultBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
DefaultBufHandlerImpl::
DefaultBufHandlerImpl(
    const char*                 userName,
    sp<ImgBufProvidersManager>  spImgBufProvidersMgr)
    : DefaultBufHandler()
    , msName(userName)
    , mbEnableIImageBufferLog(MFALSE)
    , mbIsForceRotation(MFALSE)
    , mRotationAnagle(0)
    , mspImgBufProvidersMgr(spImgBufProvidersMgr)
{
    FUNC_NAME;
}


/*******************************************************************************
*
********************************************************************************/
DefaultBufHandlerImpl::
~DefaultBufHandlerImpl()
{
    FUNC_NAME;
    mspImgBufProvidersMgr = NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
init()
{
    FUNC_START;
    //
    for(MUINT32 i=0; i<EPass2Out_DST_AMOUNT; i++)
    {
        mvBufQueNode[i].clear();
        mlMapPort[i].clear();
    }
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get( "debug.enable.imagebuffer.log", value, "0");
    mbEnableIImageBufferLog = atoi(value);
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
uninit()
{
    FUNC_START;
    //
    for(MUINT32 i=0; i<EPass2Out_DST_AMOUNT; i++)
    {
        mvBufQueNode[i].clear();
    }
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
requestBuffer(MUINT32 const data, AllocInfo const & info)
{
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
dequeBuffer(MUINT32 const data, ImgRequest * pImgReq)
{
    Mutex::Autolock _l(mLock);
    //
    MBOOL isDequeProvider = MFALSE;
    MBOOL doCacheInvalid = MFALSE;
    MINT32 bufQueIdx = mapNode2Dst(data);
    ImgBufQueNode node;
    list<MAP_PORT_INFO>::iterator iterMapPort;
    //
    #if 0
    MY_LOGD("data(0x%08X)",data);
    #endif
    //
    if(bufQueIdx == -1)
    {
        return MFALSE;
    }
    //
    for(iterMapPort = mlMapPort[bufQueIdx].begin(); iterMapPort != mlMapPort[bufQueIdx].end(); iterMapPort++)
    {
        #if 0
        MY_LOGD("bufQueIdx(%d),data(%d,%d),bufType(%d)",
                bufQueIdx,data,
                (*iterMapPort).nodeType,
                (*iterMapPort).bufType);
        #endif
        if(data == (*iterMapPort).nodeType)
        {
            sp<IImgBufProvider> bufProvider = NULL;
            pImgReq->mUsage = NSIoPipe::EPortCapbility_None;
            switch((*iterMapPort).bufType)
            {
                case eBuf_Disp:
                {
                    bufProvider =  mspImgBufProvidersMgr->getDisplayPvdr();
                    pImgReq->mUsage = NSIoPipe::EPortCapbility_Disp;
                    break;
                }
                case eBuf_Rec:
                {
                    bufProvider =  mspImgBufProvidersMgr->getRecCBPvdr();
                    pImgReq->mUsage = NSIoPipe::EPortCapbility_Rcrd;
                    break;
                }
                case eBuf_AP:
                {
                    bufProvider =  mspImgBufProvidersMgr->getPrvCBPvdr();
                    doCacheInvalid = MTRUE;
                    break;
                }
                case eBuf_Generic:
                {
                    bufProvider =  mspImgBufProvidersMgr->getGenericBufPvdr();
                    break;
                }
                case eBuf_FD:
                {
                    bufProvider =  mspImgBufProvidersMgr->getFDBufPvdr();
                    break;
                }
                case eBuf_OT:
                {
                    bufProvider =  mspImgBufProvidersMgr->getOTBufPvdr();
                    break;
                }
                default:
                {
                    MY_LOGE("un-supported bufType(%d)",(*iterMapPort).bufType);
                    return MFALSE;
                }
            }
            //
            if(bufProvider != 0)
            {
                MUINT32 timeout = (*iterMapPort).timeout;
                //
                while(1)
                {
                    if(bufProvider->dequeProvider(node))
                    {
                        node.setCookieDE((*iterMapPort).bufType);
                        mvBufQueNode[bufQueIdx].push_back(node);
                        isDequeProvider = MTRUE;
                        break;
                    }
                    else
                    {
                        if(timeout == 0)
                        {
                            if((*iterMapPort).timeout != 0)
                            {
                                MY_LOGW("Wait bufType(%d) buffer timeout in %d ms",
                                        (*iterMapPort).bufType,
                                        (*iterMapPort).timeout);
                            }
                            break;
                        }
                        else
                        {
                            MY_LOGW("Wait bufType(%d) buffer, timeout(%d/%d) count down",
                                    (*iterMapPort).bufType,
                                    timeout,
                                    (*iterMapPort).timeout);
                            timeout--;
                            usleep(10*1000);
                        }
                    }
                };
                //
                if(isDequeProvider)
                {
                    break;
                }
            }
        }
    }
    //
    if(isDequeProvider)
    {
        size_t bufStridesInBytes[] = { GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 0),
                                        GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 1),
                                        GET_IIMGBUF_IMG_STRIDE_IN_BYTE(node.getImgBuf(), 2)};
        size_t bufBoundaryInBytes[] = {0,0,0};
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam(
                                                        Format::queryImageFormat(node.getImgBuf()->getImgFormat().string()),
                                                        MSize(
                                                            node.getImgBuf()->getImgWidth(),
                                                            node.getImgBuf()->getImgHeight()),
                                                        bufStridesInBytes,
                                                        bufBoundaryInBytes,
                                                        Format::queryPlaneCount(Format::queryImageFormat(node.getImgBuf()->getImgFormat().string())));
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                        node.getImgBuf()->getIonFd(),
                                        (MUINTPTR)node.getImgBuf()->getVirAddr(),
                                        0,
                                        node.getImgBuf()->getBufSecu(),
                                        node.getImgBuf()->getBufCohe());
        //
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                        LOG_TAG,
                                                        imgParam,
                                                        portBufInfo,
                                                        mbEnableIImageBufferLog);
        if(pHeap == 0)
        {
            MY_LOGE("pHeap is NULL");
            return MFALSE;
        }
        //
        IImageBuffer* tempBuffer = pHeap->createImageBuffer();
        tempBuffer->incStrong(tempBuffer);
        tempBuffer->lockBuf(
                        LOG_TAG,
                        eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);

        if( doCacheInvalid )
        {
            if( !tempBuffer->syncCache(eCACHECTRL_INVALID) )
                MY_LOGE("invalid cache failed imgbuf 0x%x", tempBuffer);
        }

        pImgReq->mBuffer = tempBuffer;
        if(mbIsForceRotation)
        {
            pImgReq->mTransform  = mRotationAnagle == 0 ? 0
                                 : mRotationAnagle == 90 ? eTransform_ROT_90
                                 : mRotationAnagle == 180 ? eTransform_ROT_180 : eTransform_ROT_270;
        }
        else
        {
            pImgReq->mTransform  = node.getRotation() == 0 ? 0
                                 : node.getRotation() == 90 ? eTransform_ROT_90
                                 : node.getRotation() == 180 ? eTransform_ROT_180 : eTransform_ROT_270;
        }
        //
        #if 0
        MY_LOGD("Node:DE(%d),VA(%p),S(%d),Id(%d)",
                node.getCookieDE(),
                (MUINTPTR)node.getImgBuf()->getVirAddr(),
                node.getImgBuf()->getBufSize(),
                node.getImgBuf()->getIonFd());
        MY_LOGD("Node:F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
                node.getImgBuf()->getImgFormat().string(),
                node.getImgBuf()->getImgWidth(),
                node.getImgBuf()->getImgHeight(),
                GET_IIMGBUF_IMG_W_STRIDE(node.getImgBuf(), 0),
                GET_IIMGBUF_IMG_W_STRIDE(node.getImgBuf(), 1),
                GET_IIMGBUF_IMG_W_STRIDE(node.getImgBuf(), 2),
                node.getRotation());
        #endif
        //
        #if 1
        /*
        MY_LOGD("buf(%p),heap(%p),VA(%p/%p/%p),PA(%p/%p/%p),BS(%d=%d+%d+%d),Id(%d),F(0x%08X),S(%dx%d),Str(%d,%d,%d),R(%d),U(%d)",
                pImgReq->mBuffer,
                pImgReq->mBuffer->getImageBufferHeap(),
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 2),
                GET_IIMAGEBUFFER_BUF_PA(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_PA(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_PA(pImgReq->mBuffer, 2),
                (   GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 0)+
                    GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 1)+
                    GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 2)),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 2),
                pImgReq->mBuffer->getFD(),
                pImgReq->mBuffer->getImgFormat(),
                pImgReq->mBuffer->getImgSize().w,
                pImgReq->mBuffer->getImgSize().h,
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 2),
                pImgReq->mTransform,
                pImgReq->mUsage);
        */
        MY_LOGD("buf(%p),VA(%p/%p/%p),BS(%d=%d+%d+%d),Id(%d),F(0x%08X),S(%dx%d),Str(%d,%d,%d),R(%d),U(%d)",
                pImgReq->mBuffer,
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_VA(pImgReq->mBuffer, 2),
                (   GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 0)+
                    GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 1)+
                    GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 2)),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_SIZE(pImgReq->mBuffer, 2),
                pImgReq->mBuffer->getFD(),
                pImgReq->mBuffer->getImgFormat(),
                pImgReq->mBuffer->getImgSize().w,
                pImgReq->mBuffer->getImgSize().h,
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 0),
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 1),
                GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImgReq->mBuffer, 2),
                pImgReq->mTransform,
                pImgReq->mUsage);

        #endif
        //
        return MTRUE;
    }
    //
    #if 0
    MY_LOGD("empty data(0x%08X)",data);
    #endif
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
enqueBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    Mutex::Autolock _l(mLock);
    //
    MINT32 i, bufQueIdx = mapNode2Dst(data);
    ImgBufQueNode keepImgBufQueNode;
    //
    #if 0
    MY_LOGD("data(0x%08X)",data);
    #endif
    //
    if(bufQueIdx == -1)
    {
        return MFALSE;
    }
    //
    if(bufQueIdx >= 0)
    {
        if(mvBufQueNode[bufQueIdx].size() > 0)
        {
            keepImgBufQueNode = mvBufQueNode[bufQueIdx][0];
        }
        else
        {
            MY_LOGE("mvBufQueNode[%d] size(%d) = 0",
                    bufQueIdx,
                    mvBufQueNode[bufQueIdx].size());
            return MFALSE;
        }
    }
    else
    {
        MY_LOGE("bufQueIdx(%d) < 0",bufQueIdx);
    }
    //
    MUINTPTR keepAddr = (MUINTPTR)(mvBufQueNode[bufQueIdx][0].getImgBuf()->getVirAddr());
    MUINTPTR enqueAddr = (MUINTPTR)(GET_IIMAGEBUFFER_BUF_VA(pImageBuffer, 0));
    //
    #if 0
    MY_LOGD("Addr(0x%08X/0x%08X),Port(%d)",
            keepAddr,
            enqueAddr,
            mvBufQueNode[bufQueIdx][0].getCookieDE());
    #endif
    //
    if(keepAddr == enqueAddr)
    {
        MBOOL isAPClientFromFD = MFALSE;
        sp<IImgBufProvider> bufProvider = NULL;
        IImageBuffer* tempBuffer = const_cast<IImageBuffer*>(pImageBuffer);
        //
        switch(keepImgBufQueNode.getCookieDE())
        {
            case eBuf_Disp:
            {
                bufProvider = mspImgBufProvidersMgr->getDisplayPvdr();
                break;
            }
            case eBuf_Rec:
            {
                bufProvider = mspImgBufProvidersMgr->getRecCBPvdr();
                break;
            }
            case eBuf_AP:
            {
                bufProvider = mspImgBufProvidersMgr->getPrvCBPvdr();
                const_cast<ImgBufQueNode*>(&keepImgBufQueNode)->setCookieDE(0); // 0 for preview
                break;
            }
            case eBuf_Generic:
            {
                bufProvider = mspImgBufProvidersMgr->getGenericBufPvdr();
                break;
            }
            case eBuf_FD:
            {
                bufProvider = mspImgBufProvidersMgr->getFDBufPvdr();
                isAPClientFromFD = MTRUE;
                break;
            }
            case eBuf_OT:
            {
                bufProvider = mspImgBufProvidersMgr->getOTBufPvdr();
                break;
            }
            default:
            {
                MY_LOGE("unknown port(%d)",keepImgBufQueNode.getCookieDE());
                return MFALSE;
            }
        }
        //
        if (bufProvider == NULL)
        {
            MY_LOGW("bufProvider(%d) is not available, drop it!", keepImgBufQueNode.getCookieDE());
            tempBuffer->unlockBuf(LOG_TAG);
            tempBuffer->decStrong(tempBuffer);
            mvBufQueNode[bufQueIdx].erase(mvBufQueNode[bufQueIdx].begin());
            return MTRUE;
        }
        else
        {
            #if 0
            MY_LOGD("Node:VA(%p),S(%d),Id(%d)",
                    (MUINPTR)keepImgBufQueNode.getImgBuf()->getVirAddr(),
                    keepImgBufQueNode.getImgBuf()->getBufSize(),
                    keepImgBufQueNode.getImgBuf()->getIonFd());
            MY_LOGD("Node:F(%s),W(%d),H(%d),Str(%d,%d,%d),Rot(%d)",
                    keepImgBufQueNode.getImgBuf()->getImgFormat().string(),
                    keepImgBufQueNode.getImgBuf()->getImgWidth(),
                    keepImgBufQueNode.getImgBuf()->getImgHeight(),
                    GET_IIMGBUF_IMG_W_STRIDE(keepImgBufQueNode.getImgBuf(), 0),
                    GET_IIMGBUF_IMG_W_STRIDE(keepImgBufQueNode.getImgBuf(), 1),
                    GET_IIMGBUF_IMG_W_STRIDE(keepImgBufQueNode.getImgBuf(), 2),
                    keepImgBufQueNode.getRotation());
            #endif
            const_cast<ImgBufQueNode*>(&(keepImgBufQueNode))->setStatus(ImgBufQueNode::eSTATUS_DONE);
            const_cast<ImgBufQueNode*>(&(keepImgBufQueNode))->getImgBuf()->setTimestamp(pImageBuffer->getTimestamp());
            //
            char debugMsg[150];
            #if 1
            /*
            sprintf(debugMsg, "buf(%p),heap(%p),VA(%p),BS(%d),Id(%d),TS(%d.%06d),F(0x%08X),S(%dx%d),Str(%d,%d,%d)",
                    pImageBuffer,
                    pImageBuffer->getImageBufferHeap(),
                    GET_IIMAGEBUFFER_BUF_VA(pImageBuffer, 0),
                    (   GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 0)+
                        GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 1)+
                        GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 2)),
                    pImageBuffer->getFD(0),
                    (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000),
                    (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000),
                    pImageBuffer->getImgFormat(),
                    pImageBuffer->getImgSize().w,
                    pImageBuffer->getImgSize().h,
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 0),
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 1),
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 2));
            */
            sprintf(debugMsg, "buf(%p),VA(%p),BS(%d),Id(%d),TS(%d.%06d),F(0x%08X),S(%dx%d),Str(%d,%d,%d)",
                    pImageBuffer,
                    GET_IIMAGEBUFFER_BUF_VA(pImageBuffer, 0),
                    (   GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 0)+
                        GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 1)+
                        GET_IIMAGEBUFFER_BUF_SIZE(pImageBuffer, 2)),
                    pImageBuffer->getFD(0),
                    (MUINT32)((pImageBuffer->getTimestamp()/1000)/1000000),
                    (MUINT32)((pImageBuffer->getTimestamp()/1000)%1000000),
                    pImageBuffer->getImgFormat(),
                    pImageBuffer->getImgSize().w,
                    pImageBuffer->getImgSize().h,
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 0),
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 1),
                    GET_IIMAGEBUFFER_BUF_STRIDE_IN_BYTE(pImageBuffer, 2));
            #endif
            //
            tempBuffer->unlockBuf(LOG_TAG);
            tempBuffer->decStrong(tempBuffer);
            //
            bufProvider->enqueProvider(keepImgBufQueNode);
            // only dequeue is not from APClient
            if(isAPClientFromFD)
            {
                // If APClient exists, copy to it
                sp<IImgBufProvider> pBufProvider;
                pBufProvider = mspImgBufProvidersMgr->getAPClientBufPvdr();
                ImgBufQueNode APClientnode;
                if (pBufProvider != 0 && pBufProvider->dequeProvider(APClientnode))
                {
                    MY_LOGD("APClient size:%d, fdClient size:%d", APClientnode.getImgBuf()->getBufSize() ,  keepImgBufQueNode.getImgBuf()->getBufSize());
                    //if ( APClientnode.getImgBuf()->getBufSize() >= keepImgBufQueNode.getImgBuf()->getBufSize())
                    if (1)
                    {
                           MY_LOGD("APClient addr:0x%x, FDCLient addr:0x%x", APClientnode.getImgBuf()->getVirAddr(), keepImgBufQueNode.getImgBuf()->getVirAddr());
                           memcpy(APClientnode.getImgBuf()->getVirAddr(),
                           keepImgBufQueNode.getImgBuf()->getVirAddr(),
                           APClientnode.getImgBuf()->getBufSize());
                           //keepImgBufQueNode.getImgBuf()->getBufSize());
                           const_cast<ImgBufQueNode*>(&APClientnode)->setStatus(ImgBufQueNode::eSTATUS_DONE);
                    }
                    else
                    {
                        MY_LOGE("APClient buffer size < FD buffer size");
                        const_cast<ImgBufQueNode*>(&APClientnode)->setStatus(ImgBufQueNode::eSTATUS_CANCEL);
                    }
                    //
                    pBufProvider->enqueProvider(APClientnode);
                }
            }
            //
            mvBufQueNode[bufQueIdx].erase(mvBufQueNode[bufQueIdx].begin());
            //
            #if 1
            MY_LOGD("%s",debugMsg);
            #endif
            //
            return MTRUE;
        }
    }
    else
    {
        MY_LOGE("Addr(0x%X != 0x%X),Port(%d), drop it!",
                keepAddr,
                enqueAddr,
                mvBufQueNode[bufQueIdx][0].getCookieDE());
        return MFALSE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
updateRequest(MUINT32 const data, MINT32 const transform, MINT32 const usage)
{
    return MFALSE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
setForceRotation(
    MBOOL   bIsForceRotation,
    MUINT32 rotationAnagle)
{
    MY_LOGD("FR(%d,%d)",
            bIsForceRotation,
            rotationAnagle);
    mbIsForceRotation = bIsForceRotation;
    mRotationAnagle = rotationAnagle;
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
mapPort(
    EBufProvider    bufType,
    NodeDataTypes   nodeType,
    MUINT32         timeout,
    MBOOL           bPushFront)
{
    Mutex::Autolock _l(mLock);
    //
    MINT32 bufQueIdx = mapNode2Dst((MUINT32)nodeType);
    list<MAP_PORT_INFO>::iterator iterMapPort;
    //
    if(bufQueIdx == -1)
    {
        return MFALSE;
    }
    //
#if 0
    for(iterMapPort = mlMapPort[bufQueIdx].begin(); iterMapPort != mlMapPort[bufQueIdx].end(); iterMapPort++)
    {
        if((*iterMapPort).bufType == bufType)
        {
            MY_LOGE("Exist, bufType(%d),nodeType(%d)",
                    (*iterMapPort).bufType,
                    (*iterMapPort).nodeType);
            return MFALSE;
        }
    }
#endif
    //
    MY_LOGD("bufType(%d),nodeType(%d),timeout(%d),bPushFront(%d)",
            bufType,
            nodeType,
            timeout,
            bPushFront);
    MAP_PORT_INFO mapInfo;
    mapInfo.bufType = bufType;
    mapInfo.nodeType = nodeType;
    mapInfo.timeout = timeout;
    if(bPushFront)
    {
        mlMapPort[bufQueIdx].push_front(mapInfo);
    }
    else
    {
        mlMapPort[bufQueIdx].push_back(mapInfo);
    }
    //
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DefaultBufHandlerImpl::
unmapPort(EBufProvider bufType)
{
    Mutex::Autolock _l(mLock);
    //
    MBOOL isFind= MFALSE;
    MUINT32 i;
    list<MAP_PORT_INFO>::iterator iterMapPort;
    //
    for(i=0; i<EPass2Out_DST_AMOUNT; i++)
    {
        for(iterMapPort = mlMapPort[i].begin(); iterMapPort != mlMapPort[i].end();)
        {
            if((*iterMapPort).bufType == bufType)
            {
                MY_LOGD("bufType(%d),nodeType(%d)",
                        (*iterMapPort).bufType,
                        (*iterMapPort).nodeType);
                iterMapPort = mlMapPort[i].erase(iterMapPort);
                isFind = MTRUE;
                break;
            }
            else
            {
                iterMapPort++;
            }
        }
    }
    //
    if(!isFind)
    {
        MY_LOGW("Can't fin bufType(%d)",bufType);
    }
    //
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MINT32
DefaultBufHandlerImpl::
mapNode2Dst(MUINT32 nodeType)
{
    switch(nodeType)
    {
        case PASS2_PRV_DST_0:
        case TRANSFORM_DST_0:
        {
            return EPass2Out_PRV_DST_0;
        }
        case PASS2_PRV_DST_1:
        case TRANSFORM_DST_1:
        {
            return EPass2Out_PRV_DST_1;
        }
        case PASS2_PRV_DST_2:
        case STEREO_DST:
        {
            return EPass2Out_PRV_DST_2;
        }
        case PASS2_PRV_DST_3:
        {
            return EPass2Out_PRV_DST_3;
        }
        case PASS2_CAP_DST_0:
        {
            return EPass2Out_CAP_DST_0;
        }
        case PASS2_CAP_DST_1:
        {
            return EPass2Out_CAP_DST_1;
        }
        default:
        {
            MY_LOGE("un-supported nodeType(%d)",nodeType);
            return -1;
        }
    }
}



}; // namespace NSCamNode

