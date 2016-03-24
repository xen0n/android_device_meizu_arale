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
#define LOG_TAG "MtkCam/FeaturePipeDataHdl"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
using namespace android;
#include <semaphore.h>
//
#include <vector>
#include <queue>
using namespace std;

#include <mtkcam/utils/Format.h>
#include <mtkcam/drv/imem_drv.h>
#include <gralloc_extra/include/ui/gralloc_extra.h>
#include <mtkcam/utils/ImageBufferHeap.h>
using namespace NSCam::Utils::Format;

#include "FeaturePipe.DataHandler.h"
//
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

//
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")

#define MODULE_NAME "DataHdl"

/******************************************************************************
*
*******************************************************************************/


namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
FeaturePipeDataHandler::
FeaturePipeDataHandler(MUINT32 const type, MUINT32 const data)
    : mType(type)
    , mData(data)
    , mTransform(0)
    , mpIMemDrv(NULL)
{
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipeDataHandler::
~FeaturePipeDataHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
init()
{
    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        CAM_LOGE("g_pIMemDrv is NULL \n");
        return MFALSE;
    }
    if( !mpIMemDrv->init() )
    {
        CAM_LOGE("g_pIMemDrv->init() error");
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
        return MFALSE;
    }

    return sem_init(&msemBuf, 0, 0) == 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
uninit()
{
    MY_LOGD("data(%d)", mData);
    //check dequeed buf
    MBOOL ret = MTRUE;
    Mutex::Autolock _l(mLock);
    if( mvDequeBuf.size() )
    {
        MY_LOGE("buffer not returned yet data(%d)", mData);
        vector<ImageBufferMap>::iterator iter = mvDequeBuf.begin();
        while( iter != mvDequeBuf.end() )
        {
            MY_LOGW("dequeed buf 0x%x", iter->pImgBuf );
#if 1
            // force release
            mqAvailableBuf.push( *iter );
            iter = mvDequeBuf.erase(iter);
#else
            iter++
#endif
        }
        ret = MFALSE;
    }

    if( mType == AllocType )
    {
        IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
        //release
        while( mqAvailableBuf.size() )
        {
            ImageBufferMap bufMap = mqAvailableBuf.front();
            if( !bufMap.pImgBuf->unlockBuf( LOG_TAG ) )
            {
                CAM_LOGE("unlock Buffer failed\n");
                ret = MFALSE;
            }
            bufMap.pImgBuf->decStrong(bufMap.pImgBuf);
            if (bufMap.type == IMEM_TYPE)
            {
                if ((mpIMemDrv->unmapPhyAddr(&bufMap.memBuf) != 0) ||
                    (mpIMemDrv->freeVirtBuf(&bufMap.memBuf) != 0))
                {
                    CAM_LOGE("free Buffer failed\n");
                    ret = MFALSE;
                }
            }

            mqAvailableBuf.pop();
        }
    }

    if( mpIMemDrv )
    {
        mpIMemDrv->uninit();
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }

    sem_destroy(&msemBuf);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
registerBuffer(IImageBuffer const * pImageBuffer)
{
    MY_LOGE("cannot support!");
#if 0
    MY_LOGD("regisBuffer imgbuf(0x%x)", pImageBuffer);

    {
        Mutex::Autolock _l(mLock);
        sp<IImageBuffer> pImgBuf = const_cast<IImageBuffer*>(pImageBuffer);
        mqAvailableBuf.push(pImgBuf);
    }

    if( sem_post(&msemBuf) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    return MTRUE;
#endif
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
allocBuffer(AllocInfo const & info)
{
    MY_LOGD("data(%d), %dx%d, fmt(0x%x)", mData, info.w, info.h, info.fmt);

    if( !info.usage )
    {
        MY_LOGE("should specify the buf usage");
        return MFALSE;
    }

    ImageBufferMap bufMap;
    /* Allocate from IMEM */
    MUINT32 plane = queryPlaneCount(info.fmt);
    bufMap.memBuf.size = 0;
    for (int i=0; i < plane; i++)
    {
        bufMap.memBuf.size += (queryPlaneWidthInPixels(info.fmt,i, info.w) * queryPlaneBitsPerPixel(info.fmt,i) / 8) * queryPlaneHeightInPixels(info.fmt, i, info.h);
    }
    if (mpIMemDrv->allocVirtBuf(&bufMap.memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        return NULL;
    }
    if (mpIMemDrv->mapPhyAddr(&bufMap.memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error \n");
        return NULL;
    }

    /* Map to ImageBuffer */
    size_t bufStridesInBytes[3] = {0};

    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] = queryPlaneWidthInPixels(info.fmt,i, info.w) * queryPlaneBitsPerPixel(info.fmt,i) / 8;
    }

    //
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParam(
            info.fmt,
            MSize(info.w,info.h),
            bufStridesInBytes,
            bufBoundaryInBytes,
            plane
            );

    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                    bufMap.memBuf.memID,
                                    bufMap.memBuf.virtAddr,
                                    bufMap.memBuf.useNoncache,
                                    bufMap.memBuf.bufSecu,
                                    bufMap.memBuf.bufCohe);

    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                    LOG_TAG,
                                                    imgParam,
                                                    portBufInfo);
    if(pHeap == 0)
    {
        MY_LOGE("pHeap is NULL");
        return NULL;
    }
    //
    bufMap.type = IMEM_TYPE;
    bufMap.pImgBuf = pHeap->createImageBuffer();
    bufMap.usage = info.usage;
    if  ( bufMap.pImgBuf == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return  MFALSE;
    }
    bufMap.pImgBuf->incStrong(bufMap.pImgBuf);

    if ( !bufMap.pImgBuf->lockBuf( LOG_TAG, info.usage ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return  MFALSE;
    }

    bufMap.pImgBuf->syncCache(eCACHECTRL_INVALID);

    {
        Mutex::Autolock _l(mLock);
        mqAvailableBuf.push(bufMap);
    }

    if( sem_post(&msemBuf) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    /*MY_LOGD("Allocated size(%d) and mapped to imgfmt(%d),w(%d),h(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )", bufMap.memBuf.size,
                bufMap.pImgBuf->getImgFormat(),bufMap.pImgBuf->getImgSize().w,bufMap.pImgBuf->getImgSize().h,
                bufMap.pImgBuf->getBufVA(0),bufMap.pImgBuf->getBufVA(1),bufMap.pImgBuf->getBufVA(2),
                bufMap.pImgBuf->getBufPA(0),bufMap.pImgBuf->getBufPA(1),bufMap.pImgBuf->getBufPA(2),
                bufMap.pImgBuf->getBufSizeInBytes(0),bufMap.pImgBuf->getBufSizeInBytes(1),bufMap.pImgBuf->getBufSizeInBytes(2));*/

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
allocBuffer_GB(AllocInfo const & info, sp<GraphicBuffer>* pGraphicBuffer)
{
    MY_LOGD("data(%d), %dx%d, fmt(0x%x)", mData, info.w, info.h, info.fmt);

    if( !info.usage )
    {
        MY_LOGE("should specify the buf usage");
        return MFALSE;
    }

    ImageBufferMap bufMap;
    MUINT32 plane = queryPlaneCount(info.fmt);
    size_t bufStridesInBytes[3] = {0};

    /* Get GraphicBuffer info */
    buffer_handle_t handle = (*pGraphicBuffer)->handle;
    MUINT32 virtAddr;
    int fdIon, idx = 0, num = 0;

    GRALLOC_EXTRA_RESULT result = GRALLOC_EXTRA_OK;
    result = gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &fdIon);
    if  (GRALLOC_EXTRA_OK != result )
    {
        MY_LOGE("gralloc_extra_query(handle, GRALLOC_EXTRA_GET_ION_FD, &fdIon) failed: ret: %d, fdIon: %d", result, fdIon);
        return MFALSE;
    }

    (*pGraphicBuffer)->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&virtAddr));

    bufMap.memBuf.memID = fdIon;
    bufMap.memBuf.virtAddr = virtAddr;
    bufMap.memBuf.useNoncache = false;
    bufMap.memBuf.bufSecu = false;
    bufMap.memBuf.bufCohe = false;
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                bufMap.memBuf.memID,
                                bufMap.memBuf.virtAddr,
                                bufMap.memBuf.useNoncache,
                                bufMap.memBuf.bufSecu,
                                bufMap.memBuf.bufCohe);
    bufMap.memBuf.size = 0;
    for (int i=0; i < plane; i++)
    {
        bufMap.memBuf.size += (queryPlaneWidthInPixels(info.fmt,i, info.w) * queryPlaneBitsPerPixel(info.fmt,i) / 8) * queryPlaneHeightInPixels(info.fmt, i, info.h);
    }

    /* Map to ImageBuffer */
    for (MUINT32 i = 0; i < plane; i++)
    {
        bufStridesInBytes[i] = queryPlaneWidthInPixels(info.fmt,i, info.w) * queryPlaneBitsPerPixel(info.fmt,i) / 8;
    }

    //
    size_t bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBufferAllocator::ImgParam imgParam(
            info.fmt,
            MSize(info.w,info.h),
            bufStridesInBytes,
            bufBoundaryInBytes,
            plane
            );

    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                    LOG_TAG,
                                                    imgParam,
                                                    portBufInfo);
    if(pHeap == 0)
    {
        MY_LOGE("pHeap is NULL");
        return NULL;
    }
    //
    bufMap.type = GRAPHICBUFFER_TYPE;
    bufMap.pGB = pGraphicBuffer;
    bufMap.pImgBuf = pHeap->createImageBuffer();
    bufMap.usage = info.usage;
    if  ( bufMap.pImgBuf == 0 )
    {
        MY_LOGE("NULL Buffer\n");
        return  MFALSE;
    }
    bufMap.pImgBuf->incStrong(bufMap.pImgBuf);

    if ( !bufMap.pImgBuf->lockBuf( LOG_TAG, info.usage ) )
    {
        MY_LOGE("lock Buffer failed\n");
        return  MFALSE;
    }

    bufMap.pImgBuf->syncCache(eCACHECTRL_INVALID);

    {
        Mutex::Autolock _l(mLock);
        mqAvailableBuf.push(bufMap);
    }

    if( sem_post(&msemBuf) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    /*MY_LOGD("Allocated size(%d) and mapped to imgfmt(%d),w(%d),h(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )", bufMap.memBuf.size,
                bufMap.pImgBuf->getImgFormat(),bufMap.pImgBuf->getImgSize().w,bufMap.pImgBuf->getImgSize().h,
                bufMap.pImgBuf->getBufVA(0),bufMap.pImgBuf->getBufVA(1),bufMap.pImgBuf->getBufVA(2),
                bufMap.pImgBuf->getBufPA(0),bufMap.pImgBuf->getBufPA(1),bufMap.pImgBuf->getBufPA(2),
                bufMap.pImgBuf->getBufSizeInBytes(0),bufMap.pImgBuf->getBufSizeInBytes(1),bufMap.pImgBuf->getBufSizeInBytes(2));*/

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
getBuffer(ImgRequest * pImgReq)
{
    MY_LOGD("data(%d), trans(%d)", mData, mTransform);
    {   //wait for buffer
        int ret;
        while( 0 !=( ret = sem_wait( &msemBuf ) ) && errno == EINTR );
        if( ret < 0 )
        {
            MY_LOGE("errno = %d", errno);
            return MFALSE;
        }
    }

    {
        Mutex::Autolock _l(mLock);

        mvDequeBuf.push_back( mqAvailableBuf.front() );

        pImgReq->mBuffer    = mqAvailableBuf.front().pImgBuf;
        pImgReq->mTransform = mTransform;

        mqAvailableBuf.pop();
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
returnBuffer(IImageBuffer const * pImageBuffer)
{
    MY_LOGD("data(%d), buf(0x%x)", mData, pImageBuffer);
    MBOOL ret = MFALSE;
    {
        Mutex::Autolock _l(mLock);
        vector<ImageBufferMap>::iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            if( iter->pImgBuf == pImageBuffer )
            {
                if( mType == AllocType )
                {
                    mqAvailableBuf.push( *iter );

                    if( sem_post(&msemBuf) < 0 )
                    {
                        MY_LOGE("errno = %d", errno);
                        return MFALSE;
                    }
                }

                iter = mvDequeBuf.erase(iter);
                ret = MTRUE;
                break;
            }
        }
    }

    if( !ret )
    {
        MY_LOGE("connot find dequeed data(%d) buf(0x%x)", mData, pImageBuffer);
        Mutex::Autolock _l(mLock);
        vector<ImageBufferMap>::const_iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            MY_LOGW("dequeed buf 0x%x", iter->pImgBuf );
        }
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
resizeBuffer(IImageBuffer const ** pImageBuffer, MUINT32 width, MUINT32 height)
{
    MY_LOGD("data(%d), buf(0x%x) resize %dx%d", mData, *pImageBuffer, width, height);
    MBOOL ret = MFALSE;
    {
        Mutex::Autolock _l(mLock);
        vector<ImageBufferMap>::iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            if( iter->pImgBuf == *pImageBuffer )
            {
                if( mType == AllocType )
                {
                    MUINT32 fmt = (*pImageBuffer)->getImgFormat();
                    MUINT32 plane = (*pImageBuffer)->getPlaneCount();

                    MUINT32 size = 0;
                    for (int i=0; i < plane; i++)
                    {
                        size += (queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i) / 8) * queryPlaneHeightInPixels(fmt, i, height);
                    }
                    if (size > iter->memBuf.size)
                    {
                        MY_LOGE("Resizing to a size (%dx%d) larger than originally allocated (%d bytes)\n", width, height, iter->memBuf.size);
                        break;
                    }

                    //
                    size_t bufStridesInBytes[3] = {0};
                    if (iter->type == IMEM_TYPE)
                    {
                        for (MUINT32 i = 0; i < plane; i++)
                        {
                            bufStridesInBytes[i] = queryPlaneWidthInPixels(fmt,i, width) * queryPlaneBitsPerPixel(fmt,i) / 8;
                        }
                    }
                    else if (iter->type == GRAPHICBUFFER_TYPE)
                    {
                        ret = iter->pImgBuf->setExtParam(MSize(width, height));
                        break;
                    }

                    /* Delete original mapping */
                    if( !iter->pImgBuf->unlockBuf( LOG_TAG ) )
                    {
                        MY_LOGE("unlock Buffer failed\n");
                        break;
                    }
                    iter->pImgBuf->decStrong(iter->pImgBuf);

                    /* Map to new ImageBuffer */
                    size_t bufBoundaryInBytes[3] = {0, 0, 0};
                    IImageBufferAllocator::ImgParam imgParam(
                            fmt,
                            MSize(width,height),
                            bufStridesInBytes,
                            bufBoundaryInBytes,
                            plane
                            );

                    PortBufInfo_v1 portBufInfo = PortBufInfo_v1(
                                                    iter->memBuf.memID,
                                                    iter->memBuf.virtAddr,
                                                    iter->memBuf.useNoncache,
                                                    iter->memBuf.bufSecu,
                                                    iter->memBuf.bufCohe);

                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create(
                                                                    LOG_TAG,
                                                                    imgParam,
                                                                    portBufInfo);
                    if(pHeap == 0)
                    {
                        MY_LOGE("pHeap is NULL");
                        break;
                    }
                    //
                    iter->pImgBuf = pHeap->createImageBuffer();
                    if  ( iter->pImgBuf == 0 )
                    {
                        MY_LOGE("NULL Buffer\n");
                        break;
                    }
                    iter->pImgBuf->incStrong(iter->pImgBuf);

                    if ( !iter->pImgBuf->lockBuf( LOG_TAG, iter->usage ) )
                    {
                        MY_LOGE("lock Buffer failed\n");
                        break;
                    }

                    /* All done */
                    *pImageBuffer = iter->pImgBuf;

                    /*MY_LOGD("Remapped size(%d) to imgfmt(%d),w(%d),h(%d),mem( va(0x%x,0x%x,0x%x)/pa(0x%x,0x%x,0x%x)/size(0x%x,0x%x,0x%x) )", iter->memBuf.size,
                                iter->pImgBuf->getImgFormat(),iter->pImgBuf->getImgSize().w,iter->pImgBuf->getImgSize().h,
                                iter->pImgBuf->getBufVA(0),iter->pImgBuf->getBufVA(1),iter->pImgBuf->getBufVA(2),
                                iter->pImgBuf->getBufPA(0),iter->pImgBuf->getBufPA(1),iter->pImgBuf->getBufPA(2),
                                iter->pImgBuf->getBufSizeInBytes(0),iter->pImgBuf->getBufSizeInBytes(1),iter->pImgBuf->getBufSizeInBytes(2));*/
                }

                ret = MTRUE;
                break;
            }
        }
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
getGraphicBuffer(IImageBuffer const * pImageBuffer, sp<GraphicBuffer>** pGraphicBuffer)
{
    MY_LOGD("data(%d), buf(0x%x)", mData, pImageBuffer);
    MBOOL ret = MFALSE;
    {
        Mutex::Autolock _l(mLock);
        vector<ImageBufferMap>::iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            if( iter->pImgBuf == pImageBuffer )
            {
                if( (mType == AllocType) && (iter->type == GRAPHICBUFFER_TYPE))
                {
                    *pGraphicBuffer = iter->pGB;
                    ret = MTRUE;
                }

                break;
            }
        }
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeDataHandler::
updateRequest(MINT32 const transform, MINT32 const usage)
{
    Mutex::Autolock _l(mLock);
    mTransform = transform;
    mUsage = usage;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

