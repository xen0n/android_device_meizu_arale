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
#define LOG_TAG "MtkCam/AllocBufHdl"

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
#include <mtkcam/camnode/AllocBufHandler.h>
#include "./inc/DataBufHandler.h"
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


#define MODULE_NAME "AllocHdl"

/******************************************************************************
*
*******************************************************************************/


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

class AllocBufHandlerImpl : public AllocBufHandler
{
    public:
        AllocBufHandlerImpl();
        ~AllocBufHandlerImpl();

        DECLARE_ICAMBUFHANDLER_INTERFACE();
        virtual MBOOL   setDataToAlloc(MUINT32 const data);
        virtual MBOOL   registerBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer);
        virtual MBOOL   unRegisterBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer);
        virtual MBOOL   updateFlag(MUINT32 const data, MUINT32 const flag);
        virtual MBOOL   isRegistered(MUINT32 const data);
        virtual MBOOL   isBufferExisted(MUINT32 const data, IImageBuffer const * pImageBuffer);

    protected:

        MBOOL   findData(MUINT32 const data, DataBufHandler** pData); //with lock protecting mvDataQue

    private:

        mutable Mutex                       mLock;
        vector< DataBufHandler* >           mvDataQue;
};


/*******************************************************************************
*
********************************************************************************/
AllocBufHandler*
AllocBufHandler::
createInstance()
{
    return new AllocBufHandlerImpl();
}


/*******************************************************************************
*
********************************************************************************/
MVOID
AllocBufHandler::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
AllocBufHandler::
AllocBufHandler()
    : ICamBufHandler(MODULE_NAME)
{
}


/*******************************************************************************
*
********************************************************************************/
AllocBufHandler::
~AllocBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
AllocBufHandlerImpl::
AllocBufHandlerImpl()
    : AllocBufHandler()
{
}


/*******************************************************************************
*
********************************************************************************/
AllocBufHandlerImpl::
~AllocBufHandlerImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
init()
{
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
uninit()
{
    Mutex::Autolock _l(mLock);
    vector< DataBufHandler* >::iterator iter = mvDataQue.begin();
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
AllocBufHandlerImpl::
requestBuffer(MUINT32 const data, AllocInfo const & info)
{
    MBOOL ret = MFALSE;

    // try to find data
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        pData = new DataBufHandler(DataBufHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete(pData);
            return MFALSE;
        }

        {
            Mutex::Autolock _l(mLock);
            mvDataQue.push_back( pData );
        }
    }

    if( pData->getType() == DataBufHandler::AllocType )
    {
        ret = pData->allocBuffer(info);
    }
    else
    {
        //TODO: check size
        ret = MTRUE;
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
setDataToAlloc(MUINT32 const data)
{
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        pData = new DataBufHandler(DataBufHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete(pData);
            return MFALSE;
        }

        {
            Mutex::Autolock _l(mLock);
            mvDataQue.push_back( pData );
        }
    }

    return pData != NULL;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
registerBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    MBOOL ret = MFALSE;

    // try to find data
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        pData = new DataBufHandler(DataBufHandler::RegisterType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete(pData);
            return MFALSE;
        }

        {
            Mutex::Autolock _l(mLock);
            mvDataQue.push_back( pData );
        }
    }

    if( pData && pImageBuffer )
    {
        ret = pData->registerBuffer(pImageBuffer);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
unRegisterBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    MBOOL ret = MFALSE;

    // try to find data
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        MY_LOGE("cannot find data 0x%x", data);
        return MFALSE;
    }

    if( pData && pImageBuffer )
    {
        ret = pData->unRegisterBuffer(pImageBuffer);
    }

    return ret;
}


/*******************************************************************************
*
********************************************************************************/
//MBOOL
//AllocBufHandlerImpl::
//registerBuffer(MUINT32 data, IImageBuffer const * pImageBuffer)
//{
//    MY_LOGE("not support register buffer");
//    return MFALSE;
//}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
dequeBuffer(MUINT32 const data, ImgRequest * pImgReq)
{
    // try to find data
    DataBufHandler* pData;
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
AllocBufHandlerImpl::
enqueBuffer(MUINT32 const data, IImageBuffer const * pImageBuffer)
{
    // try to find data
    DataBufHandler* pData;
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
AllocBufHandlerImpl::
updateRequest(MUINT32 const data, MINT32 const transform, MINT32 const usage)
{
    // try to find data
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        pData = new DataBufHandler(DataBufHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete(pData);
            return MFALSE;
        }

        {
            Mutex::Autolock _l(mLock);
            mvDataQue.push_back( pData );
        }
    }

    return pData->updateRequest(transform, usage);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
findData(MUINT32 const data, DataBufHandler** pData)
{
    Mutex::Autolock _l(mLock);
    vector< DataBufHandler* >::const_iterator iter;
    for( iter = mvDataQue.begin() ; iter != mvDataQue.end() ; iter++ )
    {
        if ((*iter)->getData() == data)
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
MBOOL
AllocBufHandlerImpl::
updateFlag(MUINT32 const data, MUINT32 const flag)
{
    // try to find data
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        pData = new DataBufHandler(DataBufHandler::AllocType, data);
        if( !pData->init() )
        {
            MY_LOGE("init failed");
            delete(pData);
            return MFALSE;
        }

        {
            Mutex::Autolock _l(mLock);
            mvDataQue.push_back( pData );
        }
    }
    return pData->updateFlag(flag);
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
isRegistered(
    MUINT32 const data
)
{
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        MY_LOGW("cannot find data %d", data);
        return MFALSE;
    }
    //
    return pData->getType() == DataBufHandler::RegisterType;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
AllocBufHandlerImpl::
isBufferExisted(
    MUINT32 const data,
    IImageBuffer const * pImageBuffer
)
{
    DataBufHandler* pData = NULL;
    if( !findData(data, &pData) )
    {
        MY_LOGW("cannot find data %d", data);
        return MFALSE;
    }
    //
    return pData->isBufferExisted(pImageBuffer);
}

/*******************************************************************************
*
********************************************************************************/

}; // namespace NSCamNode

