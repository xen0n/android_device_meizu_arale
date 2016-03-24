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
#define LOG_TAG "MtkCam/ShotCB"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;

#include <mtkcam/v1/config/PriorityDefs.h>

#include <list>
#include <queue>
using namespace std;

#include <utils/Mutex.h>
using namespace android;
//
#include <mtkcam/exif/IDbgInfoContainer.h>
//
#include <mtkcam/camnode/ShotCallbackNode.h>
//
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

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_NAME  MY_LOGD("")
#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")


/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

#define MODULE_NAME        "ShotCB"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_SHOTCB)

/*******************************************************************************
*
********************************************************************************/
#define  USE_MAIN_JPEG   0x1
#define  USE_THUMBNAIL   0x2
#define  USE_DBGINFO     0x4

#define MASK     (0x3)
#define DONE     (0x0)
#define WAITBUF  (0x3)
#define PENDING  (0x2)

#define RAW_SHIFT              (0)
#define YUV_SHIFT              (2)
#define DEBUGINFO_YUV_SHIFT    (4)
#define POSTVIEW_SHIFT         (6)
#define JPEG_SHIFT             (8)
#define THUMBNAIL_SHIFT        (10)
#define DEBUGINFO_JPEG_SHIFT   (12)
#define EIS_SHIFT              (14)
#define JPS_SHIFT              (16)
#define DEBUGINFO_JPS_SHIFT    (18)

#define USE(NAME)                 (MASK<<NAME##_SHIFT)
#define GETST(flag, NAME)         ((flag>>NAME##_SHIFT)&(0x3))
#define isStatus(flag, NAME, st)  (GETST(flag,NAME) == st)
#define setStatus(flag, NAME, st) do{ flag = (flag&(~USE(NAME)))|(st<<NAME##_SHIFT); }while(0)

/*******************************************************************************
*
********************************************************************************/

class ShotCallbackNodeImpl : public ShotCallbackNode
{
    public: // ctor & dtor
        ShotCallbackNodeImpl(MBOOL createThread);
        ~ShotCallbackNodeImpl();

    public: // operations

        virtual MVOID   setCallbacks(
                            NodeNotifyCallback_t notify_cb,
                            NodeDataCallback_t data_cb,
                            MVOID* user
                            );

        MVOID   enableNotifyMsg(MUINT32 const msg);
        MVOID   enableDataMsg(MUINT32 const msg);
        MVOID   disableNofityMsg(MUINT32 const msg);
        MVOID   disableDataMsg(MUINT32 const msg);

        MBOOL   stop();

        DECLARE_ICAMTHREADNODE_INTERFACES();

        MVOID   handleNotifyCallback(
                    MUINT32 const msg,
                    MUINT32 const ext1,
                    MUINT32 const ext2) const;

        MVOID   handleDataCallback(
                    MUINT32 const msg,
                    MUINTPTR const ext1,
                    MUINTPTR const ext2,
                    IImageBuffer* const pImgBuf) const;

    private:

        MBOOL   isNotifyMsgEnabled(MUINT32 const msg) const;
        MBOOL   isDataMsgEnabled(MUINT32 const msg) const;

        MBOOL   keepForCallback(MUINT32 const nodedatatype) const;
        MBOOL   queueData(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext);

        struct callbackdata
        {
            MUINT32 data;
            MUINTPTR buf;
            MUINT32 ext;
        };

        struct pictureCallback
        {
            MINT64              mTimestamp;
            MUINT32             muFlag;
            list<callbackdata>  mlData;
        };

        MBOOL   processCallback();

    private:

        mutable Mutex                          mLock;
        MVOID*                                 mpUser;
        MUINT32                                mNotifyMsgEnabled;
        MUINT32                                mDataMsgEnabled;
        NodeNotifyCallback_t                   mNotifyCb;
        NodeDataCallback_t                     mDataCb;

        MBOOL                                  mbStop;
        MUINT32                                muInternalFlag;
        //
        list<pictureCallback>                  mlPending;

};


/*******************************************************************************
*
********************************************************************************/
ShotCallbackNode*
ShotCallbackNode::
createInstance(MBOOL createThread)
{
    return new ShotCallbackNodeImpl(createThread);
}


/*******************************************************************************
*
********************************************************************************/
void
ShotCallbackNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
ShotCallbackNode::
ShotCallbackNode(MBOOL createThread)
    : ICamThreadNode(MODULE_NAME, createThread ? SingleTrigger : NO_THREAD,
            SCHED_POLICY, SCHED_PRIORITY)
{
}


/*******************************************************************************
*
********************************************************************************/
ShotCallbackNodeImpl::
ShotCallbackNodeImpl(MBOOL createThread)
    : ShotCallbackNode(createThread)
    , mNotifyMsgEnabled(0x0)
    , mDataMsgEnabled(0x0)
    , mNotifyCb(NULL)
    , mDataCb(NULL)
    , mbStop(MFALSE)
{
    //addSupport
    addDataSupport( ENDPOINT_SRC, SHOTCB_RAWDATA_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_YUVDATA_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_POSTVIEW_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_JPEG_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_DBGINFO_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_EISDATA_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_THUMBNAIL_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_JPS_SRC );
    addDataSupport( ENDPOINT_SRC, SHOTCB_DBGINFO_1_SRC );
    //bypass src to dst
    addDataSupport( ENDPOINT_DST, SHOTCB_RAWDATA_DST );
    addDataSupport( ENDPOINT_DST, SHOTCB_YUVDATA_DST );
    addDataSupport( ENDPOINT_DST, SHOTCB_POSTVIEW_DST );
}


/*******************************************************************************
*
********************************************************************************/
ShotCallbackNodeImpl::
~ShotCallbackNodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onInit()
{
    //FUNC_START;
    //query infos
    muInternalFlag = 0;
    if( isDataConnected(SHOTCB_RAWDATA_SRC) && isDataMsgEnabled(ENode_DATA_MSG_RAW) )
        muInternalFlag |= USE(RAW);
    if( isDataConnected(SHOTCB_YUVDATA_SRC) && isDataMsgEnabled(ENode_DATA_MSG_YUV) )
        muInternalFlag |= USE(YUV);
    if( isDataConnected(SHOTCB_POSTVIEW_SRC) && isDataMsgEnabled(ENode_DATA_MSG_POSTVIEW) )
        muInternalFlag |= USE(POSTVIEW);
    if( isDataConnected(SHOTCB_JPEG_SRC) && isDataMsgEnabled(ENode_DATA_MSG_JPEG) )
        muInternalFlag |= USE(JPEG);
    if( isDataConnected(SHOTCB_THUMBNAIL_SRC) && isDataMsgEnabled(ENode_DATA_MSG_JPEG) )
        muInternalFlag |= USE(THUMBNAIL);
    if( isDataConnected(SHOTCB_DBGINFO_SRC) ){
        if( isDataMsgEnabled(ENode_DATA_MSG_YUV) )
            muInternalFlag |= USE(DEBUGINFO_YUV);
        else if ( isDataMsgEnabled(ENode_DATA_MSG_JPEG) )
            muInternalFlag |= USE(DEBUGINFO_JPEG);
    }
    if( isDataConnected(SHOTCB_EISDATA_SRC) && isDataMsgEnabled(ENode_DATA_MSG_EISDATA) )
        muInternalFlag |= USE(EIS);
    if( isDataConnected(SHOTCB_JPS_SRC) && isDataMsgEnabled(ENode_DATA_MSG_JPS) )
        muInternalFlag |= USE(JPS);
    if( isDataConnected(SHOTCB_DBGINFO_1_SRC) ){
        if ( isDataMsgEnabled(ENode_DATA_MSG_JPS) )
            muInternalFlag |= USE(DEBUGINFO_JPS);
    }
    MY_LOGD("internal flag 0x%x", muInternalFlag);
    //FUNC_END;
    return true;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onUninit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onStart()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onStop()
{
    FUNC_START;

    mLock.lock();
    mbStop = MTRUE;
    mLock.unlock();

    list<pictureCallback>::iterator pPicCB = mlPending.begin();
    while( pPicCB != mlPending.end() )
    {
        list<callbackdata>::iterator pdata = pPicCB->mlData.begin();
        while( pdata != pPicCB->mlData.end() )
        {
            handleReturnBuffer(pdata->data,pdata->buf,pdata->ext);
            pdata++;
        }
        pPicCB->mlData.clear();
        pPicCB++;
    }
    mlPending.clear();

    MBOOL ret = syncWithThread(); //wait for jobs done
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    switch(msg)
    {
        case CONTROL_SHUTTER:
            handleNotifyCallback(ENode_NOTIFY_MSG_SHUTTER, 0, 0);
            break;
        default:
            break;
    }
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MBOOL ret = MTRUE;

    switch( data )
    {
        //bypass data first, do callback when buffer is returned
        case SHOTCB_RAWDATA_SRC:
            handlePostBuffer(SHOTCB_RAWDATA_DST, buf, ext);
            goto lbExit;
            break;
        case SHOTCB_YUVDATA_SRC:
            handlePostBuffer(SHOTCB_YUVDATA_DST, buf, ext);
            goto lbExit;
            break;
        case SHOTCB_POSTVIEW_SRC:
            handlePostBuffer(SHOTCB_POSTVIEW_DST, buf, ext);
            goto lbExit;
            break;
        //check if callback is needed, otherwise return data directly
        case SHOTCB_JPEG_SRC:
        case SHOTCB_JPS_SRC:
        case SHOTCB_DBGINFO_SRC:
        case SHOTCB_DBGINFO_1_SRC:
        case SHOTCB_EISDATA_SRC:
        case SHOTCB_THUMBNAIL_SRC:
            break;
        default:
            MY_LOGE("should not happen data %d buf 0x%x", data, buf);
            ret = MFALSE;
            break;
    }

    if( ret )
    {
        if( !keepForCallback(data) )
        {
            handleReturnBuffer(data, buf, ext);
            goto lbExit;
        }

        // try to queue the data: may fail due to going stop
        if( !queueData(data, buf, ext) )
        {
            handleReturnBuffer(data, buf, ext);
            goto lbExit;
        }

        if( getThreadNodeType() == NO_THREAD )
        {
            while( processCallback() )
            {
                MY_LOGD("process another");
            };
        }
        else
        {
            triggerLoop();
        }
    }

lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MY_LOGD("data %d, buf 0x%x, ext 0x%x", data, buf, ext);
    MBOOL ret = MTRUE;

    MUINT32 srcdata = 0;
    switch( data )
    {
        case SHOTCB_RAWDATA_DST:
            srcdata = SHOTCB_RAWDATA_SRC;
            break;
        case SHOTCB_YUVDATA_DST:
            srcdata = SHOTCB_YUVDATA_SRC;
            break;
        case SHOTCB_POSTVIEW_DST:
            srcdata = SHOTCB_POSTVIEW_SRC;
            break;
        default:
            MY_LOGE("should not happen, data %d, buf 0x%x", data, buf);
            ret = MFALSE;
            break;
    }

    if( ret )
    {
        if( !keepForCallback(srcdata) )
        {
            handleReturnBuffer(srcdata, buf, ext);
            goto lbExit;
        }

        // try to queue the data: may fail due to going stop
        if( !queueData(srcdata, buf, ext) )
        {
            handleReturnBuffer(srcdata, buf, ext);
            goto lbExit;
        }

        if( getThreadNodeType() == NO_THREAD )
        {
            while( processCallback() )
            {
                MY_LOGD("process another");
            };
        }
        else
        {
            triggerLoop();
        }
    }

lbExit:
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    while( processCallback() )
    {
        MY_LOGD("process another");
    };
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
setCallbacks(NodeNotifyCallback_t notify_cb, NodeDataCallback_t data_cb, MVOID* user)
{
    Mutex::Autolock _l(mLock);
    mpUser    = user;
    mNotifyCb = notify_cb;
    mDataCb   = data_cb;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
enableNotifyMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mNotifyMsgEnabled |= msg;
    MY_LOGD("enabled notify(0x%x)", msg);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
enableDataMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mDataMsgEnabled |= msg;
    MY_LOGD("enabled data(0x%x)", msg);
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
disableNofityMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mNotifyMsgEnabled &= ~msg;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
disableDataMsg(MUINT32 const msg)
{
    Mutex::Autolock _l(mLock);
    mDataMsgEnabled &= ~msg;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
isNotifyMsgEnabled(MUINT32 const msg) const
{
    MBOOL ret;
    Mutex::Autolock _l(mLock);
    ret = mNotifyMsgEnabled & msg;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
isDataMsgEnabled(MUINT32 const msg) const
{
    MBOOL ret;
    Mutex::Autolock _l(mLock);
    ret = mDataMsgEnabled & msg;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
keepForCallback(MUINT32 const nodedatatype) const
{
    MBOOL ret = MTRUE;
    switch(nodedatatype)
    {
        case SHOTCB_RAWDATA_SRC:
            ret = (muInternalFlag & USE(RAW)) != 0;
            break;
        case SHOTCB_YUVDATA_SRC:
            ret = (muInternalFlag & USE(YUV)) != 0;
            break;
        case SHOTCB_POSTVIEW_SRC:
            ret = (muInternalFlag & USE(POSTVIEW)) != 0;
            break;
        case SHOTCB_JPEG_SRC:
            ret = (muInternalFlag & USE(JPEG)) != 0;
            break;
        case SHOTCB_THUMBNAIL_SRC:
            ret = (muInternalFlag & USE(THUMBNAIL)) != 0;
            break;
        case SHOTCB_DBGINFO_SRC:
            ret = (muInternalFlag & (USE(DEBUGINFO_YUV) | USE(DEBUGINFO_JPEG) )) != 0;
            break;
        case SHOTCB_JPS_SRC:
            ret = (muInternalFlag & USE(JPS)) != 0;
            break;
        case SHOTCB_DBGINFO_1_SRC:
            ret = (muInternalFlag & USE(DEBUGINFO_JPS)) != 0;
            break;
        case SHOTCB_EISDATA_SRC:
            ret = (muInternalFlag & USE(EIS)) != 0;
            break;
        default:
            MY_LOGE("not support data(%d)", nodedatatype);
            ret = MFALSE;
            break;
    }

    //MY_LOGD("test keep %d, flag 0x%x", ret, muInternalFlag);
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
queueData(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MINT64 timestamp;
    if( data != SHOTCB_DBGINFO_SRC && data != SHOTCB_EISDATA_SRC && data != SHOTCB_DBGINFO_1_SRC )
    {
        timestamp = reinterpret_cast<IImageBuffer* const>((MVOID*)buf)->getTimestamp();
    }
    else
    {
        timestamp = -1;
    }

    {
        Mutex::Autolock _l(mLock);

        if( mbStop ) {
            MY_LOGD("going stop, skip queue data");
            return MFALSE;
        }

        callbackdata cbdata = {data, buf, ext};
        MY_LOGD("data %d, buf 0x%x, time %lld", cbdata.data, cbdata.buf, timestamp);

        //check for timestamp
        MBOOL find = MFALSE;
        list<pictureCallback>::iterator pPicCB = mlPending.begin();
        if( data != SHOTCB_DBGINFO_SRC && data != SHOTCB_EISDATA_SRC && data != SHOTCB_DBGINFO_1_SRC )
        {
            while( pPicCB != mlPending.end() )
            {
                if( pPicCB->mTimestamp == -1 || pPicCB->mTimestamp == timestamp )
                {
                    pPicCB->mlData.push_back(cbdata);
                    pPicCB->mTimestamp = timestamp; //update timestamp
                    find = MTRUE;
                    break;
                }
                pPicCB++;
            }
        }
        else
        {
            //debug info: special case
            while( pPicCB != mlPending.end() )
            {
                if( data == SHOTCB_DBGINFO_SRC &&
                        ( isStatus(pPicCB->muFlag, DEBUGINFO_YUV, WAITBUF)  ||
                          isStatus(pPicCB->muFlag, DEBUGINFO_JPEG, WAITBUF) )
                  )
                {
                    pPicCB->mlData.push_back(cbdata);
                    find = MTRUE;
                    break;
                }

                if( data == SHOTCB_DBGINFO_1_SRC &&
                        isStatus(pPicCB->muFlag, DEBUGINFO_JPS, WAITBUF)
                  )
                {
                    pPicCB->mlData.push_back(cbdata);
                    find = MTRUE;
                    break;
                }

                if( data == SHOTCB_EISDATA_SRC &&
                        isStatus(pPicCB->muFlag, EIS, WAITBUF)
                  )
                {
                    pPicCB->mlData.push_back(cbdata);
                    find = MTRUE;
                    break;
                }
                pPicCB++;
            }
        }

        if( !find )
        {
            pictureCallback piccb;
            pPicCB = mlPending.insert( pPicCB, piccb);
            pPicCB->mTimestamp = timestamp;
            pPicCB->muFlag     = muInternalFlag;
            pPicCB->mlData.push_back(cbdata);
            MY_LOGD("new callback t %lld, total %d", timestamp, mlPending.size());
        }

        // update status
        switch( data )
        {
            case SHOTCB_RAWDATA_SRC:
                setStatus(pPicCB->muFlag, RAW, PENDING);
                break;
            case SHOTCB_YUVDATA_SRC:
                setStatus(pPicCB->muFlag, YUV, PENDING);
                break;
            case SHOTCB_POSTVIEW_SRC:
                setStatus(pPicCB->muFlag, POSTVIEW, PENDING);
                break;
            case SHOTCB_JPEG_SRC:
                setStatus(pPicCB->muFlag, JPEG, PENDING);
                break;
            case SHOTCB_DBGINFO_SRC:
                if( muInternalFlag & USE(DEBUGINFO_YUV) )
                    setStatus(pPicCB->muFlag, DEBUGINFO_YUV, PENDING);
                else if( muInternalFlag & USE(DEBUGINFO_JPEG) )
                    setStatus(pPicCB->muFlag, DEBUGINFO_JPEG, PENDING);
                break;
            case SHOTCB_THUMBNAIL_SRC:
                setStatus(pPicCB->muFlag, THUMBNAIL, PENDING);
                break;
            case SHOTCB_EISDATA_SRC:
                setStatus(pPicCB->muFlag, EIS, PENDING);
                break;
            case SHOTCB_JPS_SRC:
                setStatus(pPicCB->muFlag, JPS, PENDING);
                break;
            case SHOTCB_DBGINFO_1_SRC:
                if( muInternalFlag & USE(DEBUGINFO_JPS) )
                    setStatus(pPicCB->muFlag, DEBUGINFO_JPS, PENDING);
                break;
            default:
                break;
        }
    }

    //FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
handleNotifyCallback(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2) const
{
    if( mNotifyCb == NULL )
    {
        MY_LOGE("notifyCallback is not set");
        return;
    }

    if( isNotifyMsgEnabled(msg) )
    {
        NodeNotifyInfo notifyinfo(msg, ext1, ext2);
        mNotifyCb(mpUser, notifyinfo);
    }
}


/*******************************************************************************
*
********************************************************************************/
MVOID
ShotCallbackNodeImpl::
handleDataCallback(MUINT32 const msg, MUINTPTR const ext1, MUINTPTR const ext2, IImageBuffer* const pImgBuf) const
{
    if( mDataCb == NULL )
    {
        MY_LOGE("dataCallback is not set");
        return;
    }

    if( isDataMsgEnabled(msg) )
    {
        NodeDataInfo datainfo(msg, ext1, ext2, pImgBuf);
        mDataCb(mpUser, datainfo);
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
processCallback()
{
    //FUNC_START;

    MBOOL bProcessNext = MFALSE;
    mLock.lock();

    if( mbStop )
    {
        MY_LOGD("going stop, skip process cb");
        mLock.unlock();
        return MFALSE;
    }

    list<pictureCallback>::iterator pPicCB = mlPending.begin();

    if( pPicCB == mlPending.end() )
    {
        MY_LOGD("no pending cb");
        bProcessNext = MFALSE;
        goto lbExit;
    }

#define GET_DATA(NODENAME, DATA, piccb, getdata)                     \
    do{                                                              \
        MBOOL find = MFALSE;                                         \
        list<callbackdata>::iterator pdata = piccb->mlData.begin();  \
        while(  pdata != piccb->mlData.end() )                       \
        {                                                            \
            if( pdata->data == SHOTCB_##NODENAME##_SRC ) {           \
                find = MTRUE;                                        \
                getdata = *pdata;                                    \
                piccb->mlData.erase(pdata);                          \
                setStatus(piccb->muFlag, DATA, DONE);                \
                break;                                               \
            }                                                        \
            pdata++;                                                 \
        }                                                            \
        if( !find ) {                                                \
            MY_LOGE("not sync with data %s, %s", #NODENAME, #DATA ); \
            goto lbExit;                                             \
        }                                                            \
    }while(0)

#define ERASE_IF_DONE(cblist, item) \
    do{                             \
        if(item->muFlag == 0){      \
            cblist.erase(item);     \
        }                           \
    }while(0)

    //if raw cb is needed
    if( muInternalFlag & USE(RAW) )
    {
        if( isStatus(pPicCB->muFlag, RAW, PENDING) )
        {
            callbackdata todo;
            //get data
            GET_DATA(RAWDATA, RAW, pPicCB, todo);
            ERASE_IF_DONE(mlPending, pPicCB);

            mLock.unlock();
            MY_LOGD("do cb raw data: buf 0x%x", todo.buf);
            handleDataCallback(
                    ENode_DATA_MSG_RAW,
                    0,
                    0,
                    (IImageBuffer*)(todo.buf)
                    );
            handleReturnBuffer(todo.data, todo.buf, todo.ext);
            return MTRUE;
        }
        else if( isStatus(pPicCB->muFlag, RAW, WAITBUF) )
        {
            MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                    pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
            goto lbExit;
        }
    }

    //if eis cb is needed
    if( muInternalFlag & USE(EIS) )
    {
        if( isStatus(pPicCB->muFlag, EIS, PENDING) )
        {
            callbackdata todo;
            //get data
            GET_DATA(EISDATA, EIS, pPicCB, todo);
            ERASE_IF_DONE(mlPending, pPicCB);

            mLock.unlock();
            MY_LOGD("do cb eis data: buf 0x%x", todo.buf);
            handleDataCallback(
                    ENode_DATA_MSG_EISDATA,
                    todo.buf,
                    0,
                    0
                    );
            handleReturnBuffer(todo.data, todo.buf, todo.ext);
            return MTRUE;
        }
        else if( isStatus(pPicCB->muFlag, EIS, WAITBUF) )
        {
            MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                    pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
            goto lbExit;
        }
    }

    //if yuv cb: w/wo dbginfo
    if( muInternalFlag & USE(YUV) )
    {
        if( (muInternalFlag & USE(DEBUGINFO_YUV) ) )
        {
            // yuv with debuginfo
            if( isStatus(pPicCB->muFlag, YUV, PENDING) &&
                    isStatus(pPicCB->muFlag, DEBUGINFO_YUV, PENDING) )
            {
                callbackdata todo;
                callbackdata dbg;
                //get data
                GET_DATA(YUVDATA, YUV, pPicCB, todo);
                GET_DATA(DBGINFO, DEBUGINFO_YUV, pPicCB, dbg);
                ERASE_IF_DONE(mlPending, pPicCB);

                mLock.unlock();
                MY_LOGD("do cb yuv data: buf 0x%x, dbg 0x%x", todo.buf, dbg.buf);
                handleDataCallback(
                        ENode_DATA_MSG_YUV,
                        0,
                        dbg.buf,
                        (IImageBuffer*)(todo.buf)
                        );
                handleReturnBuffer(todo.data, todo.buf, todo.ext);
                handleReturnBuffer(dbg.data, dbg.buf, dbg.ext);
                return MTRUE;
            }
            else if( isStatus(pPicCB->muFlag, YUV, WAITBUF) ||
                    isStatus(pPicCB->muFlag, DEBUGINFO_YUV, WAITBUF) )
            {
                MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                        pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
                goto lbExit;
            }
        }
        else // yuv only
        {
            if( isStatus(pPicCB->muFlag, YUV, PENDING) )
            {
                callbackdata todo;
                //get data
                GET_DATA(YUVDATA, YUV, pPicCB, todo);
                ERASE_IF_DONE(mlPending, pPicCB);

                mLock.unlock();
                MY_LOGD("do cb yuv data: buf 0x%x", todo.buf);
                handleDataCallback(
                        ENode_DATA_MSG_YUV,
                        0,
                        0,
                        (IImageBuffer*)(todo.buf)
                        );
                handleReturnBuffer(todo.data, todo.buf, todo.ext);
                return MTRUE;
            }
            else if( isStatus(pPicCB->muFlag, YUV, WAITBUF) )
            {
                MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                        pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
                goto lbExit;
            }
        }
    }

    //if postview cb
    if( muInternalFlag & USE(POSTVIEW) )
    {
        if( isStatus(pPicCB->muFlag, POSTVIEW, PENDING) )
        {
            callbackdata todo;
            //get data
            GET_DATA(POSTVIEW, POSTVIEW, pPicCB, todo);
            ERASE_IF_DONE(mlPending, pPicCB);

            mLock.unlock();
            MY_LOGD("do cb postview data: buf 0x%x", todo.buf);
            handleDataCallback(
                    ENode_DATA_MSG_POSTVIEW,
                    0,
                    0,
                    (IImageBuffer*)(todo.buf)
                    );
            handleReturnBuffer(todo.data, todo.buf, todo.ext);
            return MTRUE;
        }
        else if( isStatus(pPicCB->muFlag, POSTVIEW, WAITBUF) )
        {
            MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                    pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
            goto lbExit;
        }
    }

    //if jps cb: w/wo dbginfo
    if( muInternalFlag & USE(JPS) )
    {
        if( (muInternalFlag & USE(DEBUGINFO_JPS) ) )
        {
            // jps with debuginfo
            if( isStatus(pPicCB->muFlag, JPS, PENDING) &&
                    isStatus(pPicCB->muFlag, DEBUGINFO_JPS, PENDING) )
            {
                callbackdata todo;
                callbackdata dbg;
                //get data
                GET_DATA(JPS, JPS, pPicCB, todo);
                GET_DATA(DBGINFO_1, DEBUGINFO_JPS, pPicCB, dbg);
                ERASE_IF_DONE(mlPending, pPicCB);

                mLock.unlock();
                MY_LOGD("do cb jps data: buf 0x%x, dbg 0x%x", todo.buf, dbg.buf);
                handleDataCallback(
                        ENode_DATA_MSG_JPS,
                        0,
                        dbg.buf,
                        (IImageBuffer*)(todo.buf)
                        );
                handleReturnBuffer(todo.data, todo.buf, todo.ext);
                handleReturnBuffer(dbg.data, dbg.buf, dbg.ext);
                return MTRUE;
            }
            else if( isStatus(pPicCB->muFlag, JPS, WAITBUF) ||
                    isStatus(pPicCB->muFlag, DEBUGINFO_JPS, WAITBUF) )
            {
                MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                        pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
                goto lbExit;
            }
        }
        else // jps only
        {
            if( isStatus(pPicCB->muFlag, JPS, PENDING) )
            {
                callbackdata todo;
                //get data
                GET_DATA(JPS, JPS, pPicCB, todo);
                ERASE_IF_DONE(mlPending, pPicCB);

                mLock.unlock();
                MY_LOGD("do cb jps data: buf 0x%x", todo.buf);
                handleDataCallback(
                        ENode_DATA_MSG_JPS,
                        0,
                        0,
                        (IImageBuffer*)(todo.buf)
                        );
                handleReturnBuffer(todo.data, todo.buf, todo.ext);
                return MTRUE;
            }
            else if( isStatus(pPicCB->muFlag, JPS, WAITBUF) )
            {
                MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                        pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
                goto lbExit;
            }
        }
    }

    //if jpeg cb: w/wo dbginfo, thumbnail
    if( muInternalFlag & USE(JPEG) )
    {
        MBOOL ready = MTRUE;
        if( isStatus(pPicCB->muFlag, JPEG, WAITBUF) )
            ready = MFALSE;
        if( muInternalFlag & USE(THUMBNAIL) && isStatus(pPicCB->muFlag, THUMBNAIL, WAITBUF) )
            ready = MFALSE;
        if( muInternalFlag & USE(DEBUGINFO_JPEG) && isStatus(pPicCB->muFlag, DEBUGINFO_JPEG, WAITBUF) )
            ready = MFALSE;

        if( ready )
        {
            MUINT32 flag = USE_MAIN_JPEG;
            callbackdata jpeg = {0, 0, 0};
            callbackdata thumbnail = {0, 0, 0};
            callbackdata dbginfo = {0, 0, 0};
            // get data
            GET_DATA(JPEG, JPEG, pPicCB, jpeg);

            if( muInternalFlag & USE(THUMBNAIL) )
            {
                flag |= USE_THUMBNAIL;
                GET_DATA(THUMBNAIL, THUMBNAIL, pPicCB, thumbnail);
            }

            if( muInternalFlag & USE(DEBUGINFO_JPEG) )
            {
                flag |= USE_DBGINFO;
                GET_DATA(DBGINFO, DEBUGINFO_JPEG, pPicCB, dbginfo);
            }
            ERASE_IF_DONE(mlPending, pPicCB);

            mLock.unlock();

            {
                //do callback
                IImageBuffer* jpegbuf  = NULL;
                IImageBuffer* thumbbuf = NULL;
                IDbgInfoContainer* pDbg = NULL;
                if( flag & USE_MAIN_JPEG )
                    jpegbuf = reinterpret_cast<IImageBuffer*>(jpeg.buf);

                if( flag & USE_THUMBNAIL )
                    thumbbuf = reinterpret_cast<IImageBuffer*>(thumbnail.buf);

                if( flag & USE_DBGINFO )
                    pDbg = reinterpret_cast<IDbgInfoContainer*>(dbginfo.buf);

                MY_LOGD("do cb jpeg data: buf 0x%x, thumb 0x%x, dbg 0x%x", jpegbuf, thumbbuf, pDbg);
                handleDataCallback(
                        ENode_DATA_MSG_JPEG,
                        (MUINTPTR)thumbbuf,
                        (MUINTPTR)pDbg,
                        jpegbuf
                        );

                if( flag & USE_MAIN_JPEG )
                    handleReturnBuffer(jpeg.data, jpeg.buf, jpeg.ext);
                if( flag & USE_THUMBNAIL )
                    handleReturnBuffer(thumbnail.data, thumbnail.buf, thumbnail.ext);
                if( flag & USE_DBGINFO )
                    handleReturnBuffer(dbginfo.data, dbginfo.buf, dbginfo.ext);

                return MTRUE;
            }
        }
        else
        {
            MY_LOGD("callback %lld not ready, flag 0x%x, size %d",
                    pPicCB->mTimestamp, pPicCB->muFlag, pPicCB->mlData.size() );
            goto lbExit;
        }
    }
#undef ERASE_IF_DONE
#undef GET_DATA

lbExit:
    mLock.unlock();
    return bProcessNext;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
ShotCallbackNodeImpl::
stop()
{
    FUNC_NAME;
    return onStop();
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

