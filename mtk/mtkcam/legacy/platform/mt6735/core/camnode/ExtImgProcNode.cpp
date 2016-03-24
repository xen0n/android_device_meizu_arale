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
#define LOG_TAG "MtkCam/EIPNode"

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
#include <mtkcam/v1/ExtImgProc/IExtImgProc.h>
#include <mtkcam/v1/ExtImgProcHw/ExtImgProcHw.h>
//
#include <mtkcam/camnode/ExtImgProcNode.h>

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

#define MODULE_NAME        "EIPNode"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class ExtImgProcNodeImpl : public ExtImgProcNode
{
    private:

    public: // ctor & dtor
        ExtImgProcNodeImpl();
        ~ExtImgProcNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations

        MBOOL           doExtImgProc(
                            MUINT32 const data,
                            IImageBuffer* const buf,
                            MUINT32 const ext);

        MBOOL           isReadyToTransfrom() const { return MTRUE; }

        MBOOL           pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);


    protected:

    #define MAX_DST_PORT_NUM        (1)

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

    private:
        // dst deque order, should be overwriten by subclass
        MUINT32                 muDequeOrder[MAX_DST_PORT_NUM];
        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muDeqFrameCnt;
        list<PostBufInfo>       mlPostBufData;
        //
        ExtImgProcHw*           mpExtImgProcHw;
};


/*******************************************************************************
 *
 ********************************************************************************/
ExtImgProcNode*
ExtImgProcNode::
createInstance()
{
    return new ExtImgProcNodeImpl();
}


/*******************************************************************************
 *
 ********************************************************************************/
void
ExtImgProcNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
 *
 ********************************************************************************/
ExtImgProcNode::
    ExtImgProcNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
ExtImgProcNode::
~ExtImgProcNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
ExtImgProcNodeImpl::
    ExtImgProcNodeImpl()
    : ExtImgProcNode()
    , muPostFrameCnt(0)
    , muDeqFrameCnt(0)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, EXTIMGPROC_SRC );
    addDataSupport( ENDPOINT_DST, EXTIMGPROC_DST_0 );
    //
    muDequeOrder[0] = EXTIMGPROC_DST_0;
    //
}


/*******************************************************************************
 *
 ********************************************************************************/
ExtImgProcNodeImpl::
~ExtImgProcNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    mpExtImgProcHw = ExtImgProcHw::createInstance();
    if(!mpExtImgProcHw || !mpExtImgProcHw->init())
    {
        MY_LOGE("IExtImgProc init error");
        return MFALSE;
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
ExtImgProcNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    if(mpExtImgProcHw != NULL)
    {
        mpExtImgProcHw->uninit();
        mpExtImgProcHw->destroyInstance();
        mpExtImgProcHw = NULL;
    }
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
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
ExtImgProcNodeImpl::
onStop()
{
    FUNC_START;
    MBOOL ret = syncWithThread(); //wait for jobs done

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

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    CAM_TRACE_CALL();
    handleReturnBuffer(EXTIMGPROC_SRC, buf);
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
ExtImgProcNodeImpl::
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
            case EXTIMGPROC_SRC:
                DUMP_IImageBuffer( EXTIMGPROC_SRC, buf, "yuv", muPostFrameCnt );
                break;
            case EXTIMGPROC_DST_0:
                DUMP_IImageBuffer( EXTIMGPROC_DST_0, buf, "yuv", muDeqFrameCnt  );
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
ExtImgProcNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
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
    ret = doExtImgProc(postBufData.data, postBufData.buf, postBufData.ext);
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
doExtImgProc(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    FUNC_START;
    if(mpExtImgProcHw != NULL)
    {
        if(mpExtImgProcHw->getImgMask() & ExtImgProcHw::BufType_ISP_P2_TwoRun_In)
        {
            IImageBuffer* tempBuffer = const_cast<IImageBuffer*>(buf);
            mpExtImgProcHw->doImgProc(
                                ExtImgProcHw::BufType_ISP_P2_TwoRun_In,
                                tempBuffer);
        }
    }
    //
    muDeqFrameCnt++;
    handlePostBuffer(EXTIMGPROC_DST_0, (MUINTPTR)buf);
    //
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ExtImgProcNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);

    PostBufInfo postBufData = {data, buf, ext};
    mlPostBufData.push_back(postBufData);

    MY_LOGD("size(%d), data(%d), buf(0x%x)",
            mlPostBufData.size(),
            postBufData.data,
            postBufData.buf);

    muPostFrameCnt++;

    if( isReadyToTransfrom() )
    {
        triggerLoop();
    }

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

