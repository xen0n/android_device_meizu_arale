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
#define LOG_TAG "MtkCam/SyncNode"

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
#include <aee.h>
//
#include <mtkcam/camnode/SyncNode.h>

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

#define MODULE_NAME        "Sync"
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class SyncNodeImpl : public SyncNode
{
    private:

    public: // ctor & dtor
        SyncNodeImpl();
        ~SyncNodeImpl();

        DECLARE_ICAMNODE_INTERFACES();

    public: // operations

        MBOOL           isReadyToPostBuf() const { return (mlPostBuf_0.size() != 0 && mlPostBuf_1.size() != 0); }

        MBOOL           pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);


    protected:

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

    private:
        //     counter
        MUINT32                                 muFrameCnt;
        //
        // === mLock protect following ===
        mutable Mutex                           mLock;
        list<PostBufInfo>                       mlPostBuf_0;
        list<PostBufInfo>                       mlPostBuf_1;

};


/*******************************************************************************
 *
 ********************************************************************************/
SyncNode*
SyncNode::
createInstance()
{
    return new SyncNodeImpl();
}


/*******************************************************************************
 *
 ********************************************************************************/
void
SyncNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNode::
    SyncNode()
: ICamNode(MODULE_NAME)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNode::
~SyncNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNodeImpl::
    SyncNodeImpl()
    : SyncNode()
    , muFrameCnt(0)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, SYNC_SRC_0 );
    addDataSupport( ENDPOINT_SRC, SYNC_SRC_1 );
    addDataSupport( ENDPOINT_DST, SYNC_DST_0 );
    addDataSupport( ENDPOINT_DST, SYNC_DST_1 );
    //
}


/*******************************************************************************
 *
 ********************************************************************************/
SyncNodeImpl::
~SyncNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onInit()
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
SyncNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;


    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
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
SyncNodeImpl::
onStop()
{
    FUNC_START;
    Mutex::Autolock lock(mLock);
    {
        list<PostBufInfo>::iterator iter;
        for(iter = mlPostBuf_0.begin(); iter != mlPostBuf_0.end(); iter++)
        {
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", (*iter).data, (*iter).buf);
            handleReturnBuffer( (*iter).data, (MUINTPTR)((*iter).buf));
        }
        for(iter = mlPostBuf_1.begin(); iter != mlPostBuf_1.end(); iter++)
        {
            MY_LOGD("ReturnBuffer:data(%d), buf(0x%x)", (*iter).data, (*iter).buf);
            handleReturnBuffer( (*iter).data, (MUINTPTR)((*iter).buf));
        }
    }
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
SyncNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    CAM_TRACE_CALL();
    MY_LOGD("data %d, buf 0x%x ext 0x%08X", data, buf, ext);
    switch(data)
    {
        case SYNC_DST_0:
            handleReturnBuffer(SYNC_SRC_0, buf);
            break;
        case SYNC_DST_1:
            handleReturnBuffer(SYNC_SRC_1, buf);
            break;
        default:
            MY_LOGE("not support data: %d", data);
            break;
    }
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
SyncNodeImpl::
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
SyncNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    MY_LOGD("data(%d), buf(0x%x)", data, buf);
    MBOOL ret = MTRUE;
    PostBufInfo postBufData = {data, buf, ext};
    switch (data)
    {
        case SYNC_SRC_0:
            mlPostBuf_0.push_back(postBufData);
            break;
        case SYNC_SRC_1:
            mlPostBuf_1.push_back(postBufData);
            break;
        default:
            MY_LOGE("Unsupported data(%d)", data);
            handleReturnBuffer(data, (MUINTPTR)buf);
            ret = MFALSE;
            break;
    }
    //
    if( isReadyToPostBuf() )
    {
        muFrameCnt++;
        //
        PostBufInfo postBuf_0   = mlPostBuf_0.front();
        PostBufInfo postBuf_1   = mlPostBuf_1.front();
        mlPostBuf_0.pop_front();
        mlPostBuf_1.pop_front();
        //
        MY_LOGD("muFrameCnt(%d), buf(ID:%d/0x%x; ID:%d/0x%x)", muFrameCnt,
                postBuf_0.buf->getFD(), postBuf_0.buf,
                postBuf_1.buf->getFD(), postBuf_1.buf);
        handlePostBuffer(SYNC_DST_0, (MUINTPTR)postBuf_0.buf);
        handlePostBuffer(SYNC_DST_1, (MUINTPTR)postBuf_1.buf);
    }
    else
    {
        PostBufInfo retBufData;
        while( mlPostBuf_0.size() > 1 ) {
            retBufData = mlPostBuf_0.front();
            mlPostBuf_0.pop_front();
            handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf);
        }
        while( mlPostBuf_1.size() > 1 ) {
            retBufData = mlPostBuf_1.front();
            mlPostBuf_1.pop_front();
            handleReturnBuffer(retBufData.data, (MUINTPTR)retBufData.buf);
        }
    }
    //
    return ret;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

