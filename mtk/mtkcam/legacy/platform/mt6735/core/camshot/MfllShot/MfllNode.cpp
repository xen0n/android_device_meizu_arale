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
 *     TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "CamNode/MfllNode"
#define MTK_LOG_ENABLE 1
#include <mtkcam/Log.h>
#include <mtkcam/common.h>
using namespace NSCam;

#include <vector>
using namespace std;
//
//
#include <mtkcam/iopipe/PostProc/INormalStream.h>
using namespace NSCam::NSIoPipe::NSPostProc;
//
#include "../inc/MfllNode.h"
#include "../inc/MfllCommon.h"
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#if 1
#define MY_LOGV(fmt, arg...)        CAM_LOGV("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("[%s] " fmt,  __func__, ##arg); printf("[%s] " fmt,  __func__, ##arg)
#else
//#define MY_LOGD(fmt, arg...)    do {ALOGD(LOG_TAG fmt, ##arg);printf(LOG_TAG fmt "\n", ##arg);} while (0)
//#define MY_LOGE(fmt, arg...)    do {ALOGE("error" LOG_TAG fmt, ##arg);printf(LOG_TAG fmt "\n", ##arg);} while (0)
//#define MY_LOGD(fmt, arg...)    ALOGD(LOG_TAG fmt, ##arg)
//#define MY_LOGE(fmt, arg...)    ALOGE("error" LOG_TAG fmt, ##arg)
//#define MY_LOGD                 ALOGD
//#define MY_LOGE                 ALOGE
#endif

#define CHECK_RET( exp ) do{if(!(exp)) { MY_LOGE(#exp); return false; }}while(0)

#define FUNC_START MY_LOGD("+")
#define FUNC_END   MY_LOGD("-")

#define SCHED_POLICY       (SCHED_NORMAL)
#define SCHED_PRIORITY     (0)

#if 0
//******************
#warning [FIXME] should not include these header
//#include <mtkcam/core/iopipe/CamIO/PortMap.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
//using namespace NSCam::NSIoPipe;
//******************
#endif

/*******************************************************************************
*
********************************************************************************/
namespace NSCamNode {

/*******************************************************************************
*
********************************************************************************/
MfllNode::
MfllNode()
    : ICamNode("MfllNode")
    , mUser(NULL)
    , mCallback(NULL)
    /*
    : ICamThreadNode(MODULE_NAME
                    , ContinuousTrigger
                    , SCHED_POLICY
                    , SCHED_PRIORITY
                    )
    */
    , mpEisHal(NULL)
{
    ALOGD("kidd 4");
    addDataSupport(ENDPOINT_SRC, CONTROL_FULLRAW);
    addDataSupport(ENDPOINT_SRC, CONTROL_RESIZEDRAW);

    addDataSupport(ENDPOINT_DST, CONTROL_PRV_SRC);
    addDataSupport(ENDPOINT_DST, CONTROL_CAP_SRC);

    //workaround for pass1node::requestStop()
    addNotifySupport(CONTROL_STOP_PASS1);
}


/*******************************************************************************
*
********************************************************************************/
MfllNode::
~MfllNode()
{
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllNode::
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
MfllNode::
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
MfllNode::
onStart()
{
    FUNC_START;
    mPass1Cnt = 0;
    mPass2Cnt = 0;
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllNode::
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
MfllNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);

    switch(datamsg) {
        case PASS1_START_ISP:
            MY_LOGD("PASS1_START_ISP");
            // init eis
            mpEisHal = EisHal::CreateInstance("mfll_eis", 0);   //@TODO use correct index
            if (mpEisHal == NULL) {
                MY_LOGE("mpEisHal CreateInstance failed");
                //goto _Exit;
            }
            break;

        case PASS1_EOF:
            MY_LOGD("PASS1_EOF");

#if 0
            if( muCapParamCount > 0 && mpCapParams )
                handleEOF_CapParams(msg, ext1, ext2);
            else
                handleEOF_CapFixed(msg, ext1, ext2);
#endif

            //if(ext2 =! MAGIC_NUM_NO_VALID_BUF) {
            if(ext2 == 1) {
                mPass1Cnt++;
                MY_LOGD("pass1 %d done", mPass1Cnt);
                if(mPass1Cnt == MAX_REFERENCE_FRAMES+1) {
                    MY_LOGD("handleNotify(CONTROL_STOP_PASS1, 0, 0)");
                    handleNotify(CONTROL_STOP_PASS1, 0, 0);
                }

                if(mpEisHal) {
                    MINT32 GMV_X, GMV_Y;
                    // do eis
                    mpEisHal->DoEis(EIS_PASS_1);
                    //mpEisHal->GetEisPlusResult(&mEisPlueResult);
                    mpEisHal->GetEisGmv(GMV_X, GMV_Y);
                    MY_LOGD("MFLL EIS GMV(%d, %d)", GMV_X, GMV_Y);
                }
            }
            break;

        //case PASS1_STOP_ISP:
        case CONTROL_STOP_PASS1:    //@TODO change this
            MY_LOGD("CONTROL_STOP_PASS1");
            // destory eis
            if (mpEisHal != NULL) {
                mpEisHal->DestroyInstance("mfll_eis");
            }
            break;

        default:
            break;
    }
    FUNC_END;
    return MTRUE;
}


#if 0
/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllNode::
handleEOF_CapFixed(MUINT32 const msg, MUINT32 const newMagicNum, MUINT32 const magicDeque)
{
    mbDoCap = MFALSE;
    if(mpHal3A && newMagicNum != MAGIC_NUM_INVALID ) //check if 3A udpate ready
    {
        if( magicDeque != MAGIC_NUM_INVALID ) // wait for sensor dealy
        {
            MY_LOGD("cur shot count %d", muCurShotCount);
            muCurShotCount++;
            if( muCurShotCount == muShotCount ) {
                MY_LOGD("stop p1");
                // stop pass1
                handleNotify( CONTROL_STOP_PASS1, 0, 0 );
            }

            if(mpHal3A)
            {
                // no need to update anymore
                mbDo3AUpdate = MFALSE;
                // calculate statistics
                mpHal3A->sendCommand(ECmd_CaptureEnd);

                if(mbCapConnected) {
                    EIspProfile_T capProfile = EIspProfile_Capture;
                    MY_LOGD("3A setIspProfile %d, 0x%x",
                            capProfile,
                            SetCap(magicDeque) );
                    ParamIspProfile_T _3A_profile(
                            capProfile,
                            SetCap(magicDeque),
                            MFALSE,
                            ParamIspProfile_T::EParamValidate_P2Only);
                    mpHal3A->setIspProfile(_3A_profile);
                }

                if(mbPrvConnected) {
                    MY_LOGD("3A setIspProfile %d, 0x%x",
                            EIspProfile_Preview,
                            magicDeque );
                    ParamIspProfile_T _3A_profile(
                            EIspProfile_Preview,
                            magicDeque,
                            MFALSE,
                            ParamIspProfile_T::EParamValidate_P2Only);
                    mpHal3A->setIspProfile(_3A_profile);
                }
            }
            mbDoCap = MTRUE;
        }

        // if not finished
        if(mpHal3A && mbDo3AUpdate) {
            // do 3A update
            mCurMagicNum = newMagicNum;
            ParamIspProfile_T _3A_profile(
                    EIspProfile_Preview,
                    mCurMagicNum,
                    MFALSE,
                    ParamIspProfile_T::EParamValidate_All);
            mpHal3A->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
        }
    }
    return MTRUE;
}
#endif


/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MY_LOGD("data(0x%x), buf(%p), ext(0x%x)", data, (void*)buf, ext);
    IImageBuffer* pSrc = static_cast<IImageBuffer*>((MVOID*)buf);
    switch(data)
    {
#if 0
        case PASS1_FULLRAW:
            handlePostBuffer(PASS2_CAP_SRC, buf);
            break;
        case PASS1_RESIZEDRAW:
            handlePostBuffer(PASS2_PRV_SRC, buf);
            break;
        default:
            MY_LOGE("not support yet: 0x%x", data);
            break;
#else
        case CONTROL_FULLRAW:
            handlePostBuffer(CONTROL_CAP_SRC, buf);
            break;
        case CONTROL_RESIZEDRAW:
            handlePostBuffer(CONTROL_PRV_SRC, buf);
            break;
        default:
            MY_LOGE("not support yet: 0x%x", data);
            break;
#endif
    }
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
MfllNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MY_LOGD("data(0x%x), buf(%p), ext(0x%x)", data, (void*)buf, ext);
    switch(data)
    {
#if 0
        case PASS2_CAP_SRC:
            handleReturnBuffer(PASS1_FULLRAW, buf);
            mFrameCnt++;
            break;
        case PASS2_PRV_SRC:
            handleReturnBuffer(PASS1_RESIZEDRAW, buf);
            mFrameCnt++;
            break;
        default:
            MY_LOGE("not support yet: 0x%x", data);
            break;
#else
        case CONTROL_CAP_SRC:
            handleReturnBuffer(CONTROL_FULLRAW, buf);
            mPass2Cnt++;
            break;
        case CONTROL_PRV_SRC:
            mPass2Cnt++;
            handleReturnBuffer(CONTROL_RESIZEDRAW, buf);
            break;
        default:
            MY_LOGE("not support yet: 0x%x", data);
            break;
#endif
    }

    if(mPass2Cnt >= MAX_REFERENCE_FRAMES+1) {
        if(mCallback && mUser) {
            MY_LOGD("exec mCallback");
            mCallback(mUser);
        }
        else
            MY_LOGE("didn't setCallback()");
    }
    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
MfllNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MY_LOGD("data(0x%x), buf(%p), ext(0x%x)", data, (void*)buf, ext);
    FUNC_END;
    //return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MVOID
MfllNode::
setCallback(MVOID* user, NodeCallback_t callback) {
    mUser = user;
    mCallback = callback;
}


//
//  TestDump : @TODO - remove this class
//
MBOOL
DumpNode::
onInit()
{
    FUNC_START;
    FUNC_END;
    return true;
}
MBOOL
DumpNode::
onUninit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MBOOL
DumpNode::
onStart()
{
    FUNC_START;
    count = 0;
    FUNC_END;
    return MTRUE;
}
MBOOL
DumpNode::
onStop()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MBOOL
DumpNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}

#define DUMP_PATH "/sdcard/"

#if 0
#define DUMP_IImageBuffer( type, pbuf, fileExt )               \
    do{                                                        \
        IImageBuffer* buffer = (IImageBuffer*)pbuf;            \
        char filename[256];                                    \
        sprintf(filename, "%s/%s_%dx%d_%d.%s",                 \
                DUMP_PATH,                                     \
                #type,                                         \
                buffer->getImgSize().w,buffer->getImgSize().h, \
                count,                                         \
                fileExt                                        \
               );                                              \
        buffer->saveToFile(filename);                          \
    }while(0)
#else
#define DUMP_IImageBuffer( type, pbuf, fileExt )
#endif

MBOOL
DumpNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    printf("data(0x%x), buf(%p), ext(0x%x)\n", data, (void*)buf, ext);
    MY_LOGD("data(0x%x), buf(%p), ext(0x%x)", data, (void*)buf, ext);
    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        case PASS2_PRV_DST_0:
            DUMP_IImageBuffer( PASS2_PRV_DST_0, buf, "yuv" );
            break;
        case PASS2_CAP_DST_0:
            DUMP_IImageBuffer( PASS2_CAP_DST_0, buf, "yuv" );
            break;
        default:
            MY_LOGE("not handle this yet data(0x%x)", data);
            break;
    }
    handleReturnBuffer(data, buf, ext);
    count++;
    FUNC_END;
    return MTRUE;
}
MBOOL
DumpNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MVOID
DumpNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#if 0
    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        default:
            MY_LOGE("not handle this yet data(%p)", data);
            break;
    }
#endif
}


////////////////////////////////////////////////////////////////////////////////
MBOOL
TestPass1Node::
onInit()
{
    FUNC_START;

    //test
    // get bufhandler for port PASS1_FULLRAW
    ICamBufHandler* bufhdl = getBufferHandler(mPort);
    MY_LOGD("bufhdl(%p)", (void*)bufhdl);
    if( !bufhdl )
        return MFALSE;
    AllocInfo info( mSize.w, mSize.h, eImgFmt_BAYER10,
                    eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
    // allocate 2 buffer for data PASS1_FULLRAW
    bufhdl->requestBuffer( mPort, info );
    bufhdl->requestBuffer( mPort, info );

    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onUninit()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onStart()
{
    FUNC_START;
    count = 0;
    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onStop()
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNC_START;
    MY_LOGD("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    FUNC_END;
    return MTRUE;
}
MBOOL
TestPass1Node::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNC_START;
    MY_LOGD("data(0x%x), buf(%p)", data, (void*)buf);
    // return buffer
    getBufferHandler(data)->enqueBuffer( data, (IImageBuffer*)buf );
    FUNC_END;
    return MTRUE;
}
MVOID
TestPass1Node::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    switch( data )
    {
        case PASS1_FULLRAW:
            DUMP_IImageBuffer( PASS1_FULLRAW, buf, "raw" );
            break;
        case PASS1_RESIZEDRAW:
            DUMP_IImageBuffer( PASS1_RESIZEDRAW, buf, "raw" );
            break;
        default:
            MY_LOGE("not handle this yet data(0x%x)", data);
            break;
    }
}


MBOOL
TestPass1Node::
threadLoopUpdate()
{
    FUNC_START;
    ::usleep(33000);
    // get buffer
    ImgRequest imgRequest;
    if( !getBufferHandler(mPort)->dequeBuffer( mPort, &imgRequest ) )
    {
        MY_LOGE("deque buffer from handler failed");
    }
    MY_LOGD("imagebuffer(%p)", (void*)imgRequest.mBuffer);

    IImageBuffer const * pImageBuffer = imgRequest.mBuffer;
    //MUINT8* addr = (MUINT8*)pImageBuffer->getBufVA(0);
    //size_t size = pImageBuffer->getBufSizeInBytes(0);

    //MY_LOGD("addr 0x%x, size 0x%x", addr, size);
    //memset( addr             , 0xFF , size >> 1 );
    //memset( addr + (size>>1) , 0x00 , size >> 1 );

    //send to next node
    handlePostBuffer(mPort, (MUINTPTR)pImageBuffer, count);
    count++;
    FUNC_END;
    return MTRUE;
}
};  //namespace NSCamNode


