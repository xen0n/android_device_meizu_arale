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

#define LOG_TAG "TestNode"

#include <mtkcam/Log.h>
#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
using namespace NSCam;
using namespace NSCam::Utils;
#include <mtkcam/camnode/test/testNodes.h>
#if BUILD_IN_TESTNODE

#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <utils/Mutex.h>
using namespace android;

#include <queue>
using namespace std;

#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%d:%s:%s] " fmt, getSensorIdx(), getName(), __func__, ##arg)

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {


/*******************************************************************************
*
********************************************************************************/
class TestPass1NodeImpl : public TestPass1Node
{
    public: // ctor & dtor
        TestPass1NodeImpl(Pass1NodeInitCfg const initcfg);
        ~TestPass1NodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    private: // operations

        MBOOL pushToRingBuffer( IImageBuffer const* pImgBuf, vector< IImageBuffer* > & dequelist);

        Pass1NodeInitCfg const                 mInitCfg;

        MUINT32     muFrameCnt;

        Mutex                                  mLock;
        //fake ring buffer
        MUINT32                              muCurIdx;
        vector< IImageBuffer* >              mqRingFullBuf;
        vector< IImageBuffer* >              mqRingResizedBuf;
        vector< IImageBuffer* >              mqDequeFullBuf;
        vector< IImageBuffer* >              mqDequeResizedBuf;
};


/*******************************************************************************
*
********************************************************************************/
Pass1Node*
TestPass1Node::
createInstance(Pass1NodeInitCfg const initcfg)
{
    return new TestPass1NodeImpl(initcfg);
}


/*******************************************************************************
*
********************************************************************************/
void
TestPass1Node::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
TestPass1Node::
TestPass1Node()
    : Pass1Node()
{
}


/*******************************************************************************
*
********************************************************************************/
TestPass1Node::
~TestPass1Node()
{
}


/*******************************************************************************
*
********************************************************************************/
TestPass1NodeImpl::
TestPass1NodeImpl(Pass1NodeInitCfg const initcfg)
    : TestPass1Node()
    , mInitCfg(initcfg)
{
    //DATA
    addDataSupport( ENDPOINT_DST, PASS1_FULLRAW );
    addDataSupport( ENDPOINT_DST, PASS1_RESIZEDRAW );
    //NODECAT_BASIC_NOTIFY
    addNotifySupport( PASS1_START_ISP | PASS1_SOF | PASS1_EOF);
}


/*******************************************************************************
*
********************************************************************************/
TestPass1NodeImpl::
~TestPass1NodeImpl()
{
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MTRUE;

    if( isDataConnected(PASS1_FULLRAW) )
    {
        AllocInfo allocinfo(3200, 2400, eImgFmt_BAYER10,
                eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);
        ICamBufHandler* pBufHdl = getBufferHandler(PASS1_FULLRAW);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", PASS1_FULLRAW);
            ret = MFALSE;
            goto lbExit;
        }

        for(MUINT32 i = 0 ; i < mInitCfg.muRingBufCnt ; i++ )
        {
            ImgRequest ImgRequest;
            IImageBuffer * pImageBuffer;
            ret = ret && pBufHdl->requestBuffer(PASS1_FULLRAW, allocinfo);
            ret = ret && pBufHdl->dequeBuffer(PASS1_FULLRAW, &ImgRequest);
            pImageBuffer = const_cast<IImageBuffer*>(ImgRequest.mBuffer);
            //TODO: load image here
            mqRingFullBuf.push_back(pImageBuffer);
            MY_LOGD("add ring buffer data(%d), imgbuf(0x%x)", PASS1_FULLRAW, ImgRequest.mBuffer);
            printf("add ring buffer data(%d), imgbuf(0x%x)\n", PASS1_FULLRAW, ImgRequest.mBuffer);
        }
    }

    if( isDataConnected(PASS1_RESIZEDRAW) )
    {
        AllocInfo allocinfo(1600, 1200, eImgFmt_FG_BAYER10,
                eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);
        ICamBufHandler* pBufHdl = getBufferHandler(PASS1_RESIZEDRAW);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", PASS1_RESIZEDRAW);
            ret = MFALSE;
            goto lbExit;
        }

        for(MUINT32 i = 0 ; i < mInitCfg.muRingBufCnt ; i++ )
        {
            ImgRequest ImgRequest;
            IImageBuffer * pImageBuffer;
            ret = ret && pBufHdl->requestBuffer(PASS1_RESIZEDRAW, allocinfo);
            ret = ret && pBufHdl->dequeBuffer(PASS1_RESIZEDRAW, &ImgRequest);
            pImageBuffer = const_cast<IImageBuffer*>(ImgRequest.mBuffer);
            //TODO: load image here
            mqRingResizedBuf.push_back(pImageBuffer);
            MY_LOGD("add ring buffer data(%d), imgbuf(0x%x)", PASS1_RESIZEDRAW, ImgRequest.mBuffer);
            printf("add ring buffer data(%d), imgbuf(0x%x)\n", PASS1_RESIZEDRAW, ImgRequest.mBuffer);
        }
    }

lbExit:
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    if( isDataConnected(PASS1_FULLRAW) )
    {
        ICamBufHandler* pBufHdl = getBufferHandler(PASS1_FULLRAW);
        vector< IImageBuffer* >::const_iterator iter;
        for( iter = mqRingFullBuf.begin() ; iter != mqRingFullBuf.end() ; iter++ )
        {
            ret = ret && pBufHdl->enqueBuffer(PASS1_FULLRAW, *iter);
        }
    }

    if( isDataConnected(PASS1_RESIZEDRAW) )
    {
        ICamBufHandler* pBufHdl = getBufferHandler(PASS1_RESIZEDRAW);
        vector< IImageBuffer* >::const_iterator iter;
        for( iter = mqRingResizedBuf.begin() ; iter != mqRingResizedBuf.end() ; iter++ )
        {
            ret = ret && pBufHdl->enqueBuffer(PASS1_RESIZEDRAW, *iter);
        }
    }
    FUNC_END;
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onStart()
{
    FUNC_START;
    muCurIdx = 0;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onStop()
{
    FUNC_START;
    syncWithThread(); //wait for jobs done
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    switch( msg )
    {
        case CONTROL_STOP_PASS1:
            printf("pass1 receive stop notify\n");
            syncWithThread();
            printf("pass1 stoped\n");
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
TestPass1NodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    //should not happen
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    IImageBuffer const*  pBuf = static_cast<IImageBuffer const*>((MVOID*)buf);
    MBOOL ret = MTRUE;
    {
        Mutex::Autolock _l(mLock);
        if( data == PASS1_FULLRAW )
        {
            if( !pushToRingBuffer(pBuf, mqDequeFullBuf) )
            {
                ret = MFALSE;
            }
        }
        else if( data == PASS1_RESIZEDRAW )
        {
            if( !pushToRingBuffer(pBuf, mqDequeResizedBuf) )
            {
                ret = MFALSE;
            }
        }
    }

    if( !ret )
    {
        MY_LOGE("wrong order: data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
        printf("wrong order: data(%d), buf(0x%x), ext(0x%x)\n", data, buf, ext);
    }
    else
    {
        MY_LOGD("enque: data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
        printf("enque: data(%d), buf(0x%x), ext(0x%x)\n", data, buf, ext);
    }

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
TestPass1NodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PATH "/sdcard/camnode/"
#define DUMP_IImageBuffer( type, pbuf, fileExt )               \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%dx%d_%d.%s",                 \
                DUMP_PATH,                                     \
                #type,                                         \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                muFrameCnt,                                    \
                fileExt                                        \
               );                                              \
        printf("dump %s", filename);                           \
        buffer->saveToFile(filename);                          \
    }while(0)

    if(!makePath(DUMP_PATH,0660))
    {
        MY_LOGE("makePath [%s] fail",DUMP_PATH);
        return;
    }

    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        default:
            MY_LOGE("not handle this yet data(%d)", data);
            break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    handleNotify( PASS1_SOF, 0, 0 );

    usleep(FAKE_PASS1_INTERMAL);

    MUINT32 imgo = 0;
    MUINT32 rrzo = 0;
    {
        Mutex::Autolock _l(mLock);
        MBOOL available = MTRUE;
        if( isDataConnected(PASS1_FULLRAW) )
        {
            vector< IImageBuffer* >::const_iterator iter;
            for( iter = mqDequeFullBuf.begin() ; iter != mqDequeFullBuf.end() ; iter++ )
            {
                if( *iter == mqRingFullBuf[muCurIdx] )
                {
                    available = MFALSE;
                    break;
                }
            }
        }

        if( isDataConnected(PASS1_RESIZEDRAW) )
        {
            vector< IImageBuffer* >::const_iterator iter;
            for( iter = mqDequeResizedBuf.begin() ; iter != mqDequeResizedBuf.end() ; iter++ )
            {
                if( *iter == mqRingResizedBuf[muCurIdx] )
                {
                    available = MFALSE;
                    break;
                }
            }
        }

        if( available )
        {
            if( isDataConnected(PASS1_FULLRAW) )
            {
                mqDequeFullBuf.push_back(mqRingFullBuf[muCurIdx]);
                imgo = (MUINT32)mqRingFullBuf[muCurIdx];
            }
            if( isDataConnected(PASS1_RESIZEDRAW) )
            {
                mqDequeResizedBuf.push_back(mqRingResizedBuf[muCurIdx]);
                rrzo = (MUINT32)mqRingResizedBuf[muCurIdx];
            }
            muCurIdx++;
            if( muCurIdx == (mqRingFullBuf.size() > mqRingResizedBuf.size() ?
                            mqRingFullBuf.size() : mqRingResizedBuf.size() ) )
            {
                muCurIdx = 0;
            }
        }
        else
        {
            MY_LOGW("no buffer available: drop frame");
            printf("no buffer available: drop frame\n");
        }
    }

    if( imgo || rrzo ) {
        MY_LOGD("deque imgo(0x%x), rrzo(0x%x)", imgo, rrzo);
        printf("deque imgo(0x%x), rrzo(0x%x)\n", imgo, rrzo);
        handleNotify( PASS1_EOF, 0, 0 );
    }

    if( imgo ) {
        handlePostBuffer(PASS1_FULLRAW, imgo, 0);
    }
    if( rrzo ) {
        handlePostBuffer(PASS1_RESIZEDRAW, rrzo, 0);
    }
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass1NodeImpl::
pushToRingBuffer( IImageBuffer const* pImgBuf, vector< IImageBuffer* > & dequelist)
{
    if( dequelist.size() && pImgBuf == *(dequelist.begin()) )
    {
        dequelist.erase( dequelist.begin() );
        return MTRUE;
    }
    return MFALSE;
}

/*******************************************************************************
*
********************************************************************************/


class TestPass2NodeImpl : public TestPass2Node
{
    public: // ctor & dtor
        TestPass2NodeImpl(Pass2NodeType const type);
        ~TestPass2NodeImpl();

        DECLARE_ICAMTHREADNODE_INTERFACES();

    private:
        Pass2NodeType const                    mPass2Type;
        MUINT32                                muFrameCnt;

        Mutex                                  mLock;
        struct job{
            MUINT32 mDataType;
            IImageBuffer const* mBuffer;
        };
        queue<job>                             mqJob;
};


/*******************************************************************************
*
********************************************************************************/
Pass2Node*
TestPass2Node::
createInstance(Pass2NodeType type)
{
    return new TestPass2NodeImpl(type);
}


/*******************************************************************************
*
********************************************************************************/
void
TestPass2Node::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
TestPass2Node::
TestPass2Node()
    : Pass2Node()
{
}


/*******************************************************************************
*
********************************************************************************/
TestPass2Node::
~TestPass2Node()
{
}


/*******************************************************************************
*
********************************************************************************/
TestPass2NodeImpl::
TestPass2NodeImpl(Pass2NodeType const type)
    : mPass2Type(type)
{
    //addSupport
    switch( type )
    {
        case PASS2_PREVIEW:
            addDataSupport( ENDPOINT_SRC , PASS2_PRV_SRC);
            addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_0);
            addDataSupport( ENDPOINT_DST , PASS2_PRV_DST_1);
            break;
        case PASS2_CAPTURE:
        case PASS2_VSS:
            addDataSupport( ENDPOINT_SRC , PASS2_CAP_SRC);
            addDataSupport( ENDPOINT_DST , PASS2_CAP_DST_0);
            addDataSupport( ENDPOINT_DST , PASS2_CAP_DST_1);
            break;
        default:
            MY_LOGE("not support type(0x%x)", type);
            break;
    }
}


/*******************************************************************************
*
********************************************************************************/
TestPass2NodeImpl::
~TestPass2NodeImpl()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
onInit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
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
TestPass2NodeImpl::
onStart()
{
    FUNC_START;
    muFrameCnt = 0;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
onStop()
{
    FUNC_START;
    syncWithThread(); //wait for jobs done
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    {
        Mutex::Autolock _l(mLock);
        job newjob = {
            data,
            static_cast<IImageBuffer const*>((MVOID*)buf),
        };
        mqJob.push(newjob);
    }
    triggerLoop();
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    ICamBufHandler* pBufHdl = getBufferHandler(data);
    if( !pBufHdl )
    {
        MY_LOGE("no buffer hdl for data(%d), buf(0x%x)", data, buf);
        return MFALSE;
    }

    MBOOL ret = pBufHdl->enqueBuffer(data, (IImageBuffer*)buf);
    if( !ret )
    {
        MY_LOGE("enque fail: data(%d), buf(0x%x)", data, buf);
    }
    FUNC_END;
    return MTRUE; //return status?
}


/*******************************************************************************
*
********************************************************************************/
MVOID
TestPass2NodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define DUMP_PATH "/sdcard/camnode/"
#define DUMP_IImageBuffer( type, pbuf, fileExt )               \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s%s_%dx%d_%d.%s",                 \
                DUMP_PATH,                                     \
                #type,                                         \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                muFrameCnt,                                    \
                fileExt                                        \
               );                                              \
        printf("dump %s", filename);                           \
        buffer->saveToFile(filename);                          \
    }while(0)

    if(!makePath(DUMP_PATH,0660))
    {
        MY_LOGE("makePath [%s] fail",DUMP_PATH);
        return;
    }

    switch( data )
    {
        case PASS2_PRV_DST_0:
            DUMP_IImageBuffer( PASS2_PRV_DST_0, buf, "yuv" );
            break;
        case PASS2_PRV_DST_1:
            DUMP_IImageBuffer( PASS2_PRV_DST_1, buf, "yuv" );
            break;
        case PASS2_CAP_DST_0:
            DUMP_IImageBuffer( PASS2_CAP_DST_0, buf, "yuv" );
            break;
        case PASS2_CAP_DST_1:
            DUMP_IImageBuffer( PASS2_CAP_DST_1, buf, "yuv" );
            break;
        default:
            MY_LOGE("not handle this yet data(%d)", data);
            break;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestPass2NodeImpl::
threadLoopUpdate()
{
    FUNC_START;
    usleep(FAKE_PASS2_INTERMAL);

    job jobinfo;
    {
        Mutex::Autolock _l(mLock);
        jobinfo = mqJob.front();
        mqJob.pop();
    }


    MINT64 timestamp = jobinfo.mBuffer->getTimestamp();
    // return source buffer
    handleReturnBuffer( jobinfo.mDataType, (MUINTPTR)jobinfo.mBuffer, 0 );

#define PREPARE_DST_BUFFER( nodedata, pImgRequest, flag, count)   \
    do{                                                           \
        ICamBufHandler* pBufHdl = getBufferHandler(nodedata);     \
        if( !pBufHdl)                                             \
            break;                                                \
        if( pBufHdl->dequeBuffer(nodedata, (pImgRequest+count)) ) \
        {                                                         \
            flag[count] = nodedata;                               \
            count++;                                              \
        }                                                         \
    }while(0)

    MUINT32 dstDataType[2];
    ImgRequest outRequest[2];
    MUINT32 dstCount = 0;
    switch( jobinfo.mDataType )
    {
        case PASS2_PRV_SRC:
            PREPARE_DST_BUFFER(PASS2_PRV_DST_0, outRequest, dstDataType, dstCount);
            PREPARE_DST_BUFFER(PASS2_PRV_DST_1, outRequest, dstDataType, dstCount);
            break;
        case PASS2_CAP_SRC:
            PREPARE_DST_BUFFER(PASS2_CAP_DST_0, outRequest, dstDataType, dstCount);
            PREPARE_DST_BUFFER(PASS2_CAP_DST_1, outRequest, dstDataType, dstCount);
            break;
        default:
            MY_LOGE("not support: %d", jobinfo.mDataType);
            break;
    }
#undef PREPARE_DST_BUFFER

    if( dstCount == 0 )
    {
        MY_LOGW("no dst buffer, skip data(%d), buf(0x%x)", jobinfo.mDataType, jobinfo.mBuffer);
        return MTRUE;
    }

    for( MUINT32 i = 0 ; i < dstCount ; i++ )
    {
        IImageBuffer const* pDstBuf = outRequest[i].mBuffer;
        IImageBuffer* pBuf = const_cast<IImageBuffer*>(pDstBuf);

        MY_LOGD("data(%d), buf(0x%x), va/pa(0x%x/0x%x), tans(%d)",
                dstDataType[i], pDstBuf,
                pDstBuf->getBufVA(0), pDstBuf->getBufPA(0),
                outRequest[i].mTransform);

        pBuf->setTimestamp(timestamp);
        //
        handlePostBuffer(dstDataType[i], (MUINTPTR)pBuf, 0);//FIXME
    }
#if 0
    switch( jobinfo.mDataType )
    {
        case PASS2_PRV_SRC:
            handlePostBuffer(PASS2_PRV_DST_0, 0, 0);//FIXME
            handlePostBuffer(PASS2_PRV_DST_1, 0, 0);//FIXME
            break;
        case PASS2_CAP_SRC:
            handlePostBuffer(PASS2_CAP_DST_0, 0, 0);//FIXME
            handlePostBuffer(PASS2_CAP_DST_1, 0, 0);//FIXME
            break;
        default:
            MY_LOGE("not support yet 0x%x", jobinfo.mDataType);
            break;
    }
#endif

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/


class TestDumpNodeImpl : public TestDumpNode
{
    public: // ctor & dtor
        TestDumpNodeImpl() {};
        ~TestDumpNodeImpl() {};

        DECLARE_ICAMNODE_INTERFACES();

    private:
        MUINT32  muFrameCnt;
};


/*******************************************************************************
*
********************************************************************************/
TestDumpNode*
TestDumpNode::
createInstance()
{
    return new TestDumpNodeImpl();
}


/*******************************************************************************
*
********************************************************************************/
void
TestDumpNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
TestDumpNode::
TestDumpNode()
    : ICamNode("TestDumpNode")
{
    addDataSupport(ENDPOINT_SRC,PASS1_FULLRAW);
    addDataSupport(ENDPOINT_SRC,PASS1_RESIZEDRAW);

    addDataSupport(ENDPOINT_SRC,PASS2_PRV_DST_0);
    addDataSupport(ENDPOINT_SRC,PASS2_PRV_DST_1);
    addDataSupport(ENDPOINT_SRC,PASS2_CAP_DST_0);
    addDataSupport(ENDPOINT_SRC,PASS2_CAP_DST_1);
}


/*******************************************************************************
*
********************************************************************************/
TestDumpNode::
~TestDumpNode()
{
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
onInit()
{
    FUNC_START;
    FUNC_END;
    return true;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
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
TestDumpNodeImpl::
onStart()
{
    FUNC_START;
    muFrameCnt = 0;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
onStop()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    printf("TestDumpNode onNotify msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MY_LOGD("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    printf("TestDumpNode: data(%d), buf(0x%x), ext(0x%x)\n", data, buf, ext);
    handleReturnBuffer(data, buf, ext);
    muFrameCnt++;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
TestDumpNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
TestDumpNodeImpl::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

#endif
