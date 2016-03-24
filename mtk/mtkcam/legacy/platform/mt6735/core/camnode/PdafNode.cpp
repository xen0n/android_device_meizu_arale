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
#define LOG_TAG "MtkCam/PdafNode"

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
#include <mtkcam/camnode/PdafNode.h>
//

#include "aaa_types.h"
#include "debug_exif/dbg_id_param.h"
#include "aaa_error_code.h"
#include "mtkcam/featureio/aaa_hal_common.h"
#include "aaa_hal.h"
#include "af_mgr/af_mgr_if.h"
using namespace NS3A;

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)       CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define MY_LOGD(fmt, arg...)       CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define MY_LOGI(fmt, arg...)       CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define MY_LOGW(fmt, arg...)       CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define MY_LOGE(fmt, arg...)       CAM_LOGE("[%s] " fmt, __func__, ##arg)
#define MY_LOGA(fmt, arg...)       CAM_LOGA("[%s] " fmt, __func__, ##arg)
#define MY_LOGF(fmt, arg...)       CAM_LOGF("[%s] " fmt, __func__, ##arg)

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

#define MODULE_NAME        "PdafNode"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*   utilities
********************************************************************************/


/*******************************************************************************
 *
 ********************************************************************************/
class PdafNodeImpl : public PdafNode
{
    private:

    public: // ctor & dtor
        PdafNodeImpl(MUINT32 const sensorIdx);
        ~PdafNodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    public: // operations

        static PdafNodeImpl*    createInstance(MUINT32 const sensorIdx);
        virtual MVOID               destroyInstance();
        MBOOL           pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext);
        MUINT32 getSensorIdx() const { return muSensorIdx; };


    protected:

        typedef struct
        {
            MUINT32          data;
            IImageBuffer*    buf;
            MUINT32          ext;
        }PostBufInfo;

    private:
        // === mLock protect following ===
        mutable Mutex           mLock;
        MUINT32 const           muSensorIdx;
        MUINT32                 muUserCnt;
        MUINT32                 muPostFrameCnt;
        MUINT32                 muDeqFrameCnt;
        list<PostBufInfo>       mlPostBufData;
};

static Mutex                g_lock;
static list<PdafNodeImpl*>  g_ctrllist;


/*******************************************************************************
 *
 ********************************************************************************/
PdafNode*
PdafNode::
createInstance(MUINT32 const sensorIdx)
{
    return PdafNodeImpl::createInstance(sensorIdx);
}


/*******************************************************************************
 *
 ********************************************************************************/
void
PdafNode::
destroyInstance()
{
    return this->destroyInstance();
}


/*******************************************************************************
 *
 ********************************************************************************/
PdafNode::
    PdafNode()
: ICamThreadNode( MODULE_NAME, SingleTrigger, SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
PdafNode::
~PdafNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
PdafNodeImpl::
    PdafNodeImpl(MUINT32 const sensorIdx)
    : PdafNode()
    , muSensorIdx(sensorIdx)
    , muPostFrameCnt(0)
    , muDeqFrameCnt(0)
    , muUserCnt(0)
{
    //DATA
    addDataSupport( ENDPOINT_SRC, PDAF_SRC );
}


/*******************************************************************************
 *
 ********************************************************************************/
PdafNodeImpl::
~PdafNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
PdafNodeImpl*
PdafNodeImpl::
createInstance(MUINT32 const sensorIdx)
{
    Mutex::Autolock _l(g_lock);

    PdafNodeImpl* pPdafCtrl = NULL;
    list<PdafNodeImpl*>::const_iterator iter;
    for( iter = g_ctrllist.begin() ; iter != g_ctrllist.end(); iter++ )
    {
        if( (*iter)->getSensorIdx() == sensorIdx )
            pPdafCtrl = (*iter);
    }

    if( pPdafCtrl == NULL )
    {
        //create new
        pPdafCtrl = new PdafNodeImpl(sensorIdx);
        g_ctrllist.push_back(pPdafCtrl);
    }

    pPdafCtrl->muUserCnt++;
    MY_LOGD("this(0x%x), userCnt(%d)", pPdafCtrl, pPdafCtrl->muUserCnt );

    return pPdafCtrl;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
PdafNodeImpl::
destroyInstance()
{
    Mutex::Autolock _l(g_lock);
    muUserCnt--;
    MY_LOGD("this(0x%x), userCnt(%d)", this, muUserCnt);
    if( muUserCnt == 0 )
    {
        list<PdafNodeImpl*>::iterator iter;
        for( iter = g_ctrllist.begin() ; iter != g_ctrllist.end(); iter++ )
        {
            if( (*iter) == this )
            {
                g_ctrllist.erase(iter);
                break;
            }
        }
        delete this;
    }
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PdafNodeImpl::
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
PdafNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PdafNodeImpl::
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
PdafNodeImpl::
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
PdafNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PdafNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
PdafNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
PdafNodeImpl::
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
            case PDAF_SRC:
                DUMP_IImageBuffer( PDAF_SRC, buf, "raw", muPostFrameCnt );
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
PdafNodeImpl::
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

        postBufData = mlPostBufData.back();
        mlPostBufData.pop_back();
    }
    IAfMgr *pIAfMgr = &IAfMgr::getInstance();
    IImageBuffer *pImgBuf = postBufData.buf;
    // currently force main sensor (1)
    pIAfMgr->doPDBuffer(1, (MVOID *)pImgBuf->getBufVA(0), pImgBuf->getImgSize().w, pImgBuf->getImgSize().h, pImgBuf->getBufStridesInBytes(0));

    IHal3A* mpHal3A = IHal3A::createInstance(IHal3A::E_Camera_1,muSensorIdx ,"PdafNode");
    if(mpHal3A!=NULL)
    {
        mpHal3A->send3ACtrl(E3ACtrl_PostPDAFtask, 0 ,0);
        mpHal3A->destroyInstance("PdafNode");
    }
    handleReturnBuffer(PDAF_SRC, (MUINTPTR)pImgBuf);
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PdafNodeImpl::
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

    triggerLoop();

    return MTRUE;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

