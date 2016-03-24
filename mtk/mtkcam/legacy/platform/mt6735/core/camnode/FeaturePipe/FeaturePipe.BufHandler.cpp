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
#define LOG_TAG "MtkCam/FeaturePipeBufHdl"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;
//
#include <utils/Mutex.h>
using namespace android;
//
#include <vector>
using namespace std;

//
#include "FeaturePipe.BufHandler.h"
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include "FeaturePipe.DataHandler.h"
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
#define FUNC_NAME  MY_LOGD("")


#define MODULE_NAME "FeaturePipeBufHdl"

/******************************************************************************
*
*******************************************************************************/


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class FeaturePipeBufHandlerImpl : public FeaturePipeBufHandler
{
    public:
        FeaturePipeBufHandlerImpl();
        ~FeaturePipeBufHandlerImpl();

        DECLARE_ICAMBUFHANDLER_INTERFACE();
        virtual MBOOL   registerBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer);
        virtual MBOOL   resizeBuffer(MUINT32 const data, IImageBuffer const ** pImageBuffer, MUINT32 width, MUINT32 height);
        virtual MBOOL   requestBuffer_GB(MUINT32 const data, AllocInfo const & info, sp<GraphicBuffer>* pGraphicBuffer);
        virtual MBOOL   getGraphicBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer, sp<GraphicBuffer>** pGraphicBuffer);

    private:
        mutable Mutex                       mLock;

        MBOOL   findData(MUINT32 const data, FeaturePipeDataHandler** pData); //with lock protecting mvDataQue

        vector< FeaturePipeDataHandler* > mvDataQue;

};


/*******************************************************************************
*
********************************************************************************/
FeaturePipeBufHandler*
FeaturePipeBufHandler::
createInstance()
{
    return new FeaturePipeBufHandlerImpl();
}


/*******************************************************************************
*
********************************************************************************/
MVOID
FeaturePipeBufHandler::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipeBufHandler::
FeaturePipeBufHandler()
    : ICamBufHandler(MODULE_NAME)
{
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipeBufHandler::
~FeaturePipeBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipeBufHandlerImpl::
FeaturePipeBufHandlerImpl()
    : FeaturePipeBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
FeaturePipeBufHandlerImpl::
~FeaturePipeBufHandlerImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
init()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
uninit()
{
    Mutex::Autolock _l(mLock);
    vector< FeaturePipeDataHandler* >::iterator iter = mvDataQue.begin();
    while( iter != mvDataQue.end() )
    {
        if( (*iter)->uninit() )
        {
            delete (*iter);
            iter = mvDataQue.erase(iter);
        }
        else
        {
            MY_LOGE("uninit failed");
            //skip current one
            iter++;
        }
    }

    return (mvDataQue.size() == 0);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
requestBuffer(MUINT32 const data, AllocInfo const & info)
{
    MBOOL ret = MFALSE;

    // try to find data
    FeaturePipeDataHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        Mutex::Autolock _l(mLock);

        pData = new FeaturePipeDataHandler(FeaturePipeDataHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete pData;
            return MFALSE;
        }

        mvDataQue.push_back( pData );
    }

    if( pData->getType() == FeaturePipeDataHandler::AllocType )
    {
        ret = pData->allocBuffer(info);
    }
    else
    {
        //TODO: check size
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
requestBuffer_GB(MUINT32 const data, AllocInfo const & info, sp<GraphicBuffer>* pGraphicBuffer)
{
    MBOOL ret = MFALSE;

    // try to find data
    FeaturePipeDataHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        Mutex::Autolock _l(mLock);

        pData = new FeaturePipeDataHandler(FeaturePipeDataHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete pData;
            return MFALSE;
        }

        mvDataQue.push_back( pData );
    }

    if( pData->getType() == FeaturePipeDataHandler::AllocType )
    {
        ret = pData->allocBuffer_GB(info, pGraphicBuffer);
    }
    else
    {
        //TODO: check size
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
registerBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    MBOOL ret = MFALSE;

    // try to find data
    FeaturePipeDataHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        Mutex::Autolock _l(mLock);

        pData = new FeaturePipeDataHandler(FeaturePipeDataHandler::RegisterType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete pData;
            return MFALSE;
        }

        mvDataQue.push_back( pData );
    }

    if( pData )
    {
        ret = pData->registerBuffer(pImageBuffer);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
//MBOOL
//FeaturePipeBufHandlerImpl::
//registerBuffer(MUINT32 data, IImageBuffer const * pImageBuffer)
//{
//    MY_LOGE("not support register buffer");
//    return MFALSE;
//}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
dequeBuffer(MUINT32 const data, ImgRequest * pImgReq)
{
    // try to find data
    FeaturePipeDataHandler* pData;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    return pData->getBuffer(pImgReq);

}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
enqueBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    // try to find data
    FeaturePipeDataHandler* pData;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    return pData->returnBuffer(pImageBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
updateRequest(MUINT32 const data, MINT32 const transform, MINT32 const usage)
{
    // try to find data
    FeaturePipeDataHandler* pData;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    return pData->updateRequest(transform, usage);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
resizeBuffer(MUINT32 const data, IImageBuffer const ** pImageBuffer, MUINT32 width, MUINT32 height)
{
    // try to find data
    FeaturePipeDataHandler* pData;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    return pData->resizeBuffer(pImageBuffer, width, height);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
getGraphicBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer, sp<GraphicBuffer>** pGraphicBuffer)
{
    // try to find data
    FeaturePipeDataHandler* pData;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    return pData->getGraphicBuffer(pImageBuffer, pGraphicBuffer);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
FeaturePipeBufHandlerImpl::
findData(MUINT32 const data, FeaturePipeDataHandler** pData)
{
    Mutex::Autolock _l(mLock);
    vector< FeaturePipeDataHandler* >::const_iterator iter;
    for( iter = mvDataQue.begin() ; iter != mvDataQue.end() ; iter++ )
    {
        if ((*iter)->mData == data)
        {
            *pData = *iter;
            return MTRUE;
        }
    }
    return MFALSE;
}
/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

