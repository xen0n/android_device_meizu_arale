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
#define LOG_TAG "MtkCam/DataBufHdl"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
using namespace android;
#include <semaphore.h>
//
#include <vector>
using namespace std;

#include <mtkcam/utils/Format.h>
using namespace NSCam::Utils::Format;

#include "./inc/DataBufHandler.h"
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
DataBufHandler::
DataBufHandler(MUINT32 const type, MUINT32 const data)
    : mType(type)
    , mData(data)
    , mTransform(0)
    , mUsage(0)
    , mFlag(FLAG_BUFFER_REUSE)
{
}


/*******************************************************************************
*
********************************************************************************/
DataBufHandler::
~DataBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
init()
{
    Mutex::Autolock _l(mLock);
    return sem_init(&msemBuf, 0, 0) == 0;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
uninit()
{
    MY_LOGD("data %d", mData);
    //check dequeed buf
    MBOOL ret = MTRUE;
    Mutex::Autolock _l(mLock);
    if( mvDequeBuf.size() )
    {
        MY_LOGE("buffer not returned yet data(%d)", mData);
        vector< sp<IImageBuffer> >::iterator iter = mvDequeBuf.begin();
        while( iter != mvDequeBuf.end() )
        {
            MY_LOGW("dequeed buf 0x%x", iter->get() );
#if 1
            // force release
            mvAvailableBuf.push_back( *iter );
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
        while( mvAvailableBuf.size() )
        {
            sp<IImageBuffer> pImageBuf = mvAvailableBuf.front();

            if( !pImageBuf->unlockBuf( LOG_TAG ) )
            {
                CAM_LOGE("unlock Buffer failed\n");
                ret = MFALSE;
            }
            allocator->free(pImageBuf.get());

            mvAvailableBuf.erase(mvAvailableBuf.begin());
        }
    }

    sem_destroy(&msemBuf);

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
registerBuffer(IImageBuffer const * pImageBuffer)
{
    MY_LOGD("data %d, regisBuffer buf 0x%x", mData, pImageBuffer);

    {
        Mutex::Autolock _l(mLock);
        sp<IImageBuffer> pImgBuf = const_cast<IImageBuffer*>(pImageBuffer);
        mvAvailableBuf.push_back(pImgBuf);
    }

    if( sem_post(&msemBuf) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
unRegisterBuffer(IImageBuffer const * pImageBuffer)
{
    MY_LOGD("data %d, unRegisBuffer buf 0x%x", mData, pImageBuffer);

    {
        Mutex::Autolock _l(mLock);
        vector< sp<IImageBuffer> >::iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            if( iter->get() == pImageBuffer ) {
                iter = mvDequeBuf.erase(iter);
                return MTRUE;
            }
        }
        //
        for( iter = mvAvailableBuf.begin(); iter != mvAvailableBuf.end() ; iter++ )
        {
            if( iter->get() == pImageBuffer ) {
                iter = mvAvailableBuf.erase(iter);
                return MTRUE;
            }
        }
    }

    MY_LOGW("cannot unreg %p", pImageBuffer);
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
isBufferExisted(
    IImageBuffer const * pImageBuffer
) const
{
    for( size_t i = 0; i < mvDequeBuf.size(); i++ )
        if( mvDequeBuf[i].get() == pImageBuffer )
            return MTRUE;
    //
    for( size_t i = 0; i < mvAvailableBuf.size(); i++ )
        if( mvAvailableBuf[i].get() == pImageBuffer )
            return MTRUE;
    //
    MY_LOGD("buf %p does not exist", pImageBuffer);
    //
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
allocBuffer(AllocInfo const & info)
{
    MY_LOGD("%p, data %d, %dx%d, fmt(0x%x), usage(0x%x), gralloc(%d)", this, mData, info.w, info.h, info.fmt, info.usage, info.isGralloc);

    if( !info.usage )
    {
        MY_LOGE("should specify the buf usage");
        return MFALSE;
    }

    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    sp<IImageBuffer> pBuf = NULL;
    //allocate buffer
    if( info.fmt != eImgFmt_JPEG )
    {
        MINT32 bufBoundaryInBytes[] = {0,0,0};
        MUINT32 planeCount = 0, bufStridesInBytes[] = {0,0,0};

        if(info.strideInByte[0] == 0)
        {
            MY_LOGD("query plane count and stride");
            planeCount = queryPlaneCount(info.fmt);
            for (MUINT32 i = 0; i < planeCount; i++)
            {
                bufStridesInBytes[i] =
                    (queryPlaneWidthInPixels(info.fmt,i, info.w)*queryPlaneBitsPerPixel(info.fmt,i))>>3;
            }
        }
        else
        {
            for (planeCount = 0; planeCount < 3; planeCount++)
            {
                if(info.strideInByte[planeCount] > 0)
                {
                    bufStridesInBytes[planeCount] = info.strideInByte[planeCount];
                }
                else
                {
                    break;
                }
            }
        }

        for (MUINT32 i = 0; i < planeCount; i++)
        {
            MY_LOGD("i(%d/%d),bufStridesInBytes(%d)",
                    i,
                    planeCount,
                    bufStridesInBytes[i]);
        }

        IImageBufferAllocator::ImgParam imgParam(
                info.fmt,
                MSize(info.w,info.h),
                bufStridesInBytes,
                bufBoundaryInBytes,
                planeCount);

        if( info.isGralloc )
        {
            IImageBufferAllocator::ExtraParam extParam(info.usage);
            pBuf = allocator->alloc_gb(LOG_TAG, imgParam, extParam);
        }
        else
        {
            pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
        }
    }
    else
    {
        MUINT32 const thumbnailsize = 160 * 128; //FIXME: temp solution
        MUINT32 bufsize = info.w * info.h;

        if( bufsize > thumbnailsize ) // to make sure buffer is large enough for thumbnail
        {
            bufsize = bufsize * 6 / 5; //jpeg compression ratio
        }
        else
        {
            bufsize = bufsize * 2;
        }

        MINT32 bufBoundaryInBytes = 0;
        IImageBufferAllocator::ImgParam imgParam(
                MSize(info.w,info.h),
                bufsize,
                bufBoundaryInBytes);

        pBuf = allocator->alloc_ion(LOG_TAG, imgParam);
    }
    if  ( pBuf.get() == 0 )
    {
        MY_LOGE("NULL Buffer");
        return  MFALSE;
    }

    if ( !pBuf->lockBuf( LOG_TAG, info.usage ) )
    {
        MY_LOGE("lock Buffer failed");
        return  MFALSE;
    }

    if ( !pBuf->syncCache(eCACHECTRL_INVALID) )
    {
        MY_LOGE("syncCache failed");
        return  MFALSE;
    }

    {
        Mutex::Autolock _l(mLock);
        mvAvailableBuf.push_back(pBuf);
    }
    pBuf = NULL;

    if( sem_post(&msemBuf) < 0 )
    {
        MY_LOGE("errno = %d", errno);
        return MFALSE;
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
getBuffer(ImgRequest * pImgReq)
{
    MY_LOGD("data %d trans %d", mData, mTransform);
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

        mvDequeBuf.push_back( mvAvailableBuf.front() );

        pImgReq->mBuffer    = mvAvailableBuf.front().get();
        pImgReq->mTransform = mTransform;
        pImgReq->mUsage = mUsage;

        mvAvailableBuf.erase(mvAvailableBuf.begin());
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
returnBuffer(IImageBuffer const * pImageBuf)
{
    MY_LOGD("data %d buf 0x%x flag 0x%x", mData, pImageBuf, mFlag);
    MBOOL ret = MFALSE;
    {
        Mutex::Autolock _l(mLock);
        vector< sp<IImageBuffer> >::iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            if( iter->get() != pImageBuf )
                continue;

            if( mType == AllocType )
            {
                if( mFlag & FLAG_BUFFER_ONESHOT )
                {
                    // free if not reused
                    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
                    if( !(*iter)->unlockBuf( LOG_TAG ) )
                    {
                        CAM_LOGE("unlock Buffer failed\n");
                        ret = MFALSE;
                    }
                    allocator->free(iter->get());
                }
                else
                {
                    mvAvailableBuf.push_back( *iter );

                    if( sem_post(&msemBuf) < 0 )
                    {
                        MY_LOGE("errno = %d", errno);
                        return MFALSE;
                    }
                }
            }

            iter = mvDequeBuf.erase(iter);
            ret = MTRUE;
            break;
        }
    }

#if 0 //workaround
    if( !ret )
    {
        MY_LOGE("connot find dequeed data %d buf 0x%x", mData, pImageBuf);
        Mutex::Autolock _l(mLock);
        vector< sp<IImageBuffer> >::const_iterator iter;
        for( iter = mvDequeBuf.begin(); iter != mvDequeBuf.end() ; iter++ )
        {
            MY_LOGW("dequeed buf 0x%x", iter->get() );
        }
    }

    return ret;
#else
    return MTRUE;
#endif
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
DataBufHandler::
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

