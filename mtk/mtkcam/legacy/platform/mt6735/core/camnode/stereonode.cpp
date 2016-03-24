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
#define LOG_TAG "MtkCam/SNode"

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
#include <mtkcam/iopipe/PostProc/IHalPostProcPipe.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h> // TODO: should be removed
using namespace NSCam::NSIoPipe::NSPostProc;
#include <core/iopipe/CamIO/PortMap.h>
using namespace NSCam::NSIoPipe;
//
#include "mtkcam/drv/imem_drv.h"
//
#include <mtkcam/imageio/ispio_utility.h>
//
#include <aee.h>
//
#include <mtkcam/featureio/stereo_hal_base.h>
//
#include "./inc/stereonodeImpl.h"
#include "./inc/IspSyncControlHw.h"
#include <mtkcam/camnode/stereonode.h>
//
#include <cutils/properties.h>  // for debug

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

#define MODULE_PRV_NAME    "PrvStereo"
#define SCHED_POLICY       (SCHED_OTHER)
#define SCHED_PRIORITY     (NICE_CAMERA_PASS2)
/*******************************************************************************
*
********************************************************************************/
StereoNode*
StereoNode::
createInstance(StereoNodeType const type)
{
    switch(type)
    {
        case STEREO_PASS2_PREVIEW:
        case STEREO_PASS2_FEATURE:  // TODO:
            return PrvStereo::createInstance(type);
        default:
            break;
    }
    return NULL;
}


/*******************************************************************************
*
********************************************************************************/
void
StereoNode::
destroyInstance()
{
    delete this;
}


/*******************************************************************************
*
********************************************************************************/
StereoNode::
StereoNode(StereoNodeType const type)
    : ICamThreadNode(
            MODULE_PRV_NAME,
            SingleTrigger,
            SCHED_POLICY,
            SCHED_PRIORITY
            )
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoNode::
~StereoNode()
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoNodeImpl::
    StereoNodeImpl(StereoNodeType const type)
    : StereoNode(type)
    , muFrameCnt(0)
    , mbCfgImgo(MFALSE)
    , mbCfgFeo(MFALSE)
    , mbCfgRgb(MFALSE)
    , mAlgoImgSize()
    , mRgbSize()
    , mFeImgSize()
    , mHwFeSize()
    , mHwFeBlock(0)
    , mpStereoHal(NULL)
    , mpPostProcPipe(NULL)
    , mpIspSyncCtrlHw(NULL)
    , mDebugDumpGB(0)
{
}


/*******************************************************************************
 *
 ********************************************************************************/
StereoNodeImpl::
~StereoNodeImpl()
{
}

/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onInit()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get( "debug.dump.gb.enable", value, "0");
    mDebugDumpGB = atoi(value);
    //
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    EFeatureStreamTag eTag = ( SENSOR_DEV_MAIN_2 == pHalSensorList->querySensorDevIdx(getSensorIdx()) )
                            ? EFeatureStreamTag_N3D_Stream_M2 : EFeatureStreamTag_vFB_Stream;
    //
    HW_DATA_STEREO_T hwData;
    mpStereoHal = StereoHalBase::createInstance();
    if( !mpStereoHal || !mpStereoHal->STEREOGetInfo(hwData) )
    {
        MY_LOGE("STEREOGetInfo fail");
        goto lbExit;
    }
    mAlgoImgSize    = mpStereoHal->getRrzSize( (SENSOR_DEV_MAIN == pHalSensorList->querySensorDevIdx(getSensorIdx())) ? 0 : 1 );
    mFeImgSize      = mpStereoHal->getFEImgSize();
    mHwFeBlock      = hwData.hwfe_block_size;
    mHwFeSize.w     = hwData.fefm_image_width;
    mHwFeSize.h     = hwData.fefm_imgae_height;
    mRgbSize.w      = hwData.rgba_image_width;
    mRgbSize.h      = hwData.rgba_image_height;
    MY_LOGD("FE(%dx%d):block(%d)Size(%dx%d);RGB(%dx%d);ALGOImg(%dx%d)",
        getHWFESize().w, getHWFESize().h, getHWFEBlock(),
        getFEImgSize().w, getFEImgSize().h,
        getRgbImgSize().w, getRgbImgSize().h,
        getAlgoImgSize().w, getAlgoImgSize().h);
    //
    mpIspSyncCtrlHw = IspSyncControlHw::createInstance(getSensorIdx());
    if( !mpIspSyncCtrlHw )
    {
        MY_LOGE("create IspSyncControlHw failed");
        goto lbExit;
    }
    //
    mlPostBufData.clear();
    //
    mpPostProcPipe = IFeatureStream::createInstance(getName(), eTag, getSensorIdx());
    if( mpPostProcPipe == NULL )
    {
        MY_LOGE("create pipe failed");
        goto lbExit;
    }
    if( !mpPostProcPipe->init() )
    {
        MY_LOGE("postproc pipe init failed");
        goto lbExit;
    }
    //
    for (MUINT32 i = 0; i < BUF_COUNT; ++i)
    {
        mpStaParam[i] = new StaData;
        mpSrzParam[i] = new SrzSize;
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
StereoNodeImpl::
onUninit()
{
    FUNC_START;
    MBOOL ret = MTRUE;
    if( mpIspSyncCtrlHw )
    {
        mpIspSyncCtrlHw->destroyInstance();
        mpIspSyncCtrlHw = NULL;
    }

    if ( mpPostProcPipe )
    {
        ret = mpPostProcPipe->uninit();
        mpPostProcPipe->destroyInstance(getName());
        mpPostProcPipe = NULL;
    }

    if ( mpStereoHal )
    {
        mpStereoHal->destroyInstance();
        mpStereoHal = NULL;
    }

    for (MUINT32 i = 0; i < BUF_COUNT; ++i) {
        if ( mpStaParam[i] )
            delete mpStaParam[i];
        if ( mpSrzParam[i] )
            delete mpSrzParam[i];
    }

    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onStart()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    MSize rgbSize = getRgbImgSize();
    vector<HwPortConfig_t> lHwPortCfg;
    HwPortConfig_t cfgImg = {
        mapToPortID(STEREO_IMG),
        eImgFmt_YV12,
        getAlgoImgSize(),
        MRect( MPoint(0, 0), getAlgoImgSize() )
    };
    HwPortConfig_t cfgRgb = {
        mapToPortID(STEREO_RGB),
        eImgFmt_ARGB8888,
        rgbSize,
        MRect( MPoint(0, 0), rgbSize )
    };
    //
    if( !setupPort(mbCfgImgo, mbCfgFeo, mbCfgRgb ) )
        goto lbExit;
    //
    if ( mbCfgImgo )
        lHwPortCfg.push_back(cfgImg);
    //
    if ( mbCfgRgb )
        lHwPortCfg.push_back(cfgRgb);
    //
    CAM_TRACE_BEGIN("alloc");
    if( !allocBuffers(lHwPortCfg) )
    {
        MY_LOGE("alloc buffers failed");
        goto lbExit;
    }
    CAM_TRACE_END();
    //
    muPostFrameCnt = 0;
    muEnqFrameCnt = 0;
    muDeqFrameCnt = 0;
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
StereoNodeImpl::
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
    while(muEnqFrameCnt > muDeqFrameCnt)
    {
        MY_LOGD("wait lock enq %d > deq %d", muEnqFrameCnt, muDeqFrameCnt);
        mCondDeque.wait(mLock);
        MY_LOGD("wait done");
    }
    //
    freeBuffers();
    //
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    MY_LOGD("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);


    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    return pushBuf(data, (IImageBuffer*)buf, ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    MBOOL ret = MTRUE;
    CAM_TRACE_CALL();
    if ( data != STEREO_FEO )
    {
        ICamBufHandler* pBufHdl = getBufferHandler(data);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data %d, buf 0x%x", data, buf);
            return MFALSE;
        }
        ret = pBufHdl->enqueBuffer(data, (IImageBuffer*)buf);
        if( !ret )
        {
            MY_LOGE("enque fail: data %d, buf 0x%x", data, buf);
        }
        MY_LOGD("data %d, buf 0x%x", data, buf);
    }
    else
    {
        Mutex::Autolock lock(mLock);
        IMEM_BUF_INFO feo = *(IMEM_BUF_INFO*)buf;
        mlFeBufQueue.push_back(feo);
        MY_LOGD("size(%d) data %d, buf(0x%x) ID(%d) VA(0x%x)",
                mlFeBufQueue.size(), data, buf, feo.memID, feo.virtAddr);
    }
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MVOID
StereoNodeImpl::
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
            case STEREO_SRC:
                DUMP_IImageBuffer( STEREO_SRC, buf, "raw", muPostFrameCnt );
                break;
            case STEREO_IMG:
                {
                    if ( !mDebugDumpGB )
                    {
                        IImageBuffer* buffer = (IImageBuffer*)buf;
                        buffer->unlockBuf("DUMP");
                        buffer->lockBuf("DUMP", eBUFFER_USAGE_SW_READ_RARELY);
                        DUMP_IImageBuffer( STEREO_IMG, buf, "yuv", muDeqFrameCnt  );
                        buffer->unlockBuf("DUMP");
                        buffer->lockBuf("DUMP", eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_WRITE_RARELY);
                    }
                    else
                    {
                        DUMP_IImageBuffer( STEREO_IMG, buf, "yuv", muDeqFrameCnt  );
                    }
                }
                break;
            case STEREO_RGB:
                DUMP_IImageBuffer( STEREO_RGB, buf, "rgb", muDeqFrameCnt  );
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
StereoNodeImpl::
threadLoopUpdate()
{
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
    //
    return enquePass2(postBufData.data, postBufData.buf, postBufData.ext);
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
setupPort(MBOOL& cfgImgo, MBOOL& cfgFeo, MBOOL& cfgRgb)
{
    MBOOL dst   = isDataConnected(STEREO_DST);
    cfgImgo     = isDataConnected(STEREO_IMG);
    cfgFeo      = isDataConnected(STEREO_FEO);
    cfgRgb      = isDataConnected(STEREO_RGB);

    MY_LOGD("imgo %d, feo %d, rgb %d, dst %d", cfgImgo, cfgFeo, cfgRgb, dst);
    return MTRUE;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
allocBuffers(vector<HwPortConfig_t> & lPortCfg)
{
    FUNC_START;
    MBOOL ret = MFALSE;
    //
    vector< HwPortConfig_t >::const_iterator pConfig = lPortCfg.begin();
    IMemDrv* pIMemDrv =  IMemDrv::createInstance();
    if ( !pIMemDrv || !pIMemDrv->init() ) {
        MY_LOGE("pIMemDrv->init() error");
        goto lbExit;
    }
    for(MUINT32 i = 0; i < BUF_COUNT; ++i)
    {
        IMEM_BUF_INFO bufInfo;
        bufInfo.size = getHWFESize().size();
        if(pIMemDrv->allocVirtBuf(&bufInfo) < 0)
        {
            MY_LOGE("pIMemDrv->allocVirtBuf() error, i(%d)",i);
            goto lbExit;
        }
        if(pIMemDrv->mapPhyAddr(&bufInfo) < 0)
        {
            MY_LOGE("pIMemDrv->mapPhyAddr() error, i(%d)",i);
            goto lbExit;
        }
        mlFeBufQueue.push_back(bufInfo);
    }
    //
    while( pConfig != lPortCfg.end() )
    {
        MUINT32 nodedatatype = mapToNodeDataType(pConfig->mPortID);
        ICamBufHandler* pBufHdl = getBufferHandler(nodedatatype);
        MY_LOGD("handle(%p) data(%d) S(%dx%d)F(0x%x)", pBufHdl, nodedatatype, pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt);
        if( !pBufHdl )
        {
            MY_LOGE("no buffer hdl for data(%d)", nodedatatype);
            goto lbExit;
        }
        //alloc buffer
        AllocInfo allocinfo(pConfig->mSize.w, pConfig->mSize.h, pConfig->mFmt,
                eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK);

        if( nodedatatype == STEREO_IMG )
        {
            if ( !mDebugDumpGB )
                allocinfo.usage     = eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_WRITE_RARELY;
            else
                allocinfo.usage     = eBUFFER_USAGE_HW_RENDER|eBUFFER_USAGE_HW_TEXTURE|eBUFFER_USAGE_SW_WRITE_RARELY|eBUFFER_USAGE_SW_READ_RARELY;
            allocinfo.isGralloc = MTRUE;
        }

        for(MUINT32 i = 0; i < BUF_COUNT ; i++ )
        {
            if( !pBufHdl->requestBuffer(nodedatatype, allocinfo) )
            {
                MY_LOGE("request buffer failed: data %d", nodedatatype);
                goto lbExit;
            }
        }
        //
        pConfig++;
    }
    //
    ret = MTRUE;
lbExit:
    if ( !pIMemDrv ) {
        pIMemDrv->uninit();
        pIMemDrv->destroyInstance();
    }
    if( !ret ) {
        MY_LOGE("allocBuffers failed");
    }
    FUNC_END;
    return ret;
}


/*******************************************************************************
 *
 ********************************************************************************/
MBOOL
StereoNodeImpl::
freeBuffers()
{
    FUNC_START;
    MBOOL ret = MFALSE;
    list<IMEM_BUF_INFO>::iterator iter;
    IMEM_BUF_INFO feo;
    IMemDrv* pIMemDrv =  IMemDrv::createInstance();
    if ( !pIMemDrv || !pIMemDrv->init() ) {
        MY_LOGE("pIMemDrv->init() error");
        goto lbExit;
    }
    MY_LOGD("check buffer size(%d)", mlFeBufQueue.size());
    for(iter = mlFeBufQueue.begin(); iter != mlFeBufQueue.end(); iter++)
    {
        MY_LOGD("Buffer addr(0x%x)", (*iter).virtAddr);
        if(0 == (*iter).virtAddr)
        {
            MY_LOGE("Buffer doesn't exist");
            continue;
        }
        if(pIMemDrv->unmapPhyAddr(&(*iter)) < 0)
        {
            MY_LOGE("pIMemDrv->unmapPhyAddr() error");
            goto lbExit;
        }
        if (pIMemDrv->freeVirtBuf(&(*iter)) < 0)
        {
            MY_LOGE("pIMemDrv->freeVirtBuf() error");
            goto lbExit;
        }
    }
    //
    ret = MTRUE;
lbExit:
    if ( !pIMemDrv ) {
        pIMemDrv->uninit();
        pIMemDrv->destroyInstance();
    }
    return ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
enquePass2(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    CAM_TRACE_CALL();
    // get output bufffers
    MUINT32 dstCount = 0, dstDataType[MAX_DST_PORT_NUM];
    ImgRequest outRequest[MAX_DST_PORT_NUM];
    IMEM_BUF_INFO feoRequest;
    vector<MUINT32> vDataDst = getDataConnected();
    MSize const srcImgSize = buf->getImgSize();
    MSize const dstImgSize = getAlgoImgSize();
    //
    for(MUINT32 i = 0; i < vDataDst.size(); i++)
    {
        if ( STEREO_FEO != vDataDst[i] )
        {
            ICamBufHandler* pBufHdl = getBufferHandler( vDataDst[i] );
            if( pBufHdl && pBufHdl->dequeBuffer( vDataDst[i], &outRequest[dstCount]) )
            {
                dstDataType[dstCount] = vDataDst[i];
                dstCount++;
            }
            else
            {
                MY_LOGE("no dst buf (0x%x)", vDataDst[i]);
            }
        }
        else
        {
            Mutex::Autolock lock(mLock);
            if( mlFeBufQueue.size() == 0 )
            {
                MY_LOGE("no feo buf");
            }
            else
            {
                feoRequest = mlFeBufQueue.front();
                mlFeBufQueue.pop_front();
                //
                dstDataType[dstCount] = vDataDst[i];
                dstCount++;
            }
        }
    }
    if( dstCount == 0 )
    {
        MY_LOGW("no dst buffer, skip data(%d), buf(0x%x)", data, buf);
        handleReturnBuffer(data, (MUINTPTR)buf);
        return MTRUE;
    }

    QParams enqueParams;
    StaData feoData;

    MUINT32 magicNum = 0;
    MVOID*  pPrivateData = NULL;
    MUINT32 privateDataSize = 0;
    MCropRect p2InCrop;
    MSize feSize = getFEImgSize();

    MINT64 timestamp = 0;
    //input
    {
        IImageBuffer const*  pSrcBuf = buf;
        Input src;
        //
        src.mPortID = mapToPortID(data);
        src.mBuffer = const_cast<IImageBuffer*>(pSrcBuf);
        //
        mpIspSyncCtrlHw->getPass2Info(
                            src.mBuffer,
                            srcImgSize,
                            magicNum,
                            pPrivateData,
                            privateDataSize,
                            p2InCrop);
        //
        enqueParams.mpPrivaData = pPrivateData;
        enqueParams.mFrameNo = magicNum;
        //
        timestamp = pSrcBuf->getTimestamp();

        enqueParams.mvIn.push_back(src);

    }
    //
    MCrpRsInfo crop1;
    crop1.mGroupID      = 1;
    crop1.mCropRect     = p2InCrop;
    crop1.mResizeDst    = dstImgSize;
    //
    MCrpRsInfo crop2;
    crop2.mGroupID      = 2;
    crop2.mCropRect.s   = dstImgSize;

    MY_LOGD("data %d, buf 0x%x,va/pa(0x%x/0x%x),size(%dx%d),crop(%d,%d,%d,%d) # 0x%X, cnt %d; crop1(%d,%d,%d,%d)(%dx%d), crop2(%d,%d,%d,%d)(%dx%d)",
            data, buf, buf->getBufVA(0), buf->getBufPA(0), buf->getImgSize().w, buf->getImgSize().h,
            p2InCrop.p_integral.x, p2InCrop.p_integral.y, p2InCrop.s.w, p2InCrop.s.h, magicNum, muEnqFrameCnt,
            crop1.mCropRect.p_integral.x, crop1.mCropRect.p_integral.y, crop1.mCropRect.s.w, crop1.mCropRect.s.h, crop1.mResizeDst.w, crop1.mResizeDst.h,
            crop2.mCropRect.p_integral.x, crop2.mCropRect.p_integral.y, crop2.mCropRect.s.w, crop2.mCropRect.s.h, crop2.mResizeDst.w, crop2.mResizeDst.h);
    //
    // output
    for( MUINT32 i = 0 ; i < dstCount ; i++ )
    {
        if(dstDataType[i] != STEREO_FEO)
        {
            IImageBuffer const* pDstBuf = outRequest[i].mBuffer;
            MY_LOGD("data %d, buf 0x%x, va/pa(0x%x/0x%x), size(%dx%d), tans %d",
                    dstDataType[i], pDstBuf,
                    pDstBuf->getBufVA(0), pDstBuf->getBufPA(0),
                    pDstBuf->getImgSize().w, pDstBuf->getImgSize().h,
                    outRequest[i].mTransform);

            Output dst;
            dst.mPortID     = mapToPortID(dstDataType[i]);
            dst.mBuffer     = const_cast<IImageBuffer*>(pDstBuf);
            dst.mTransform  = outRequest[i].mTransform;
            dst.mBuffer->setTimestamp(timestamp);
            //
            enqueParams.mvOut.push_back(dst);
        }
        else
        {
            MINT32 idx = muEnqFrameCnt%BUF_COUNT;
            mpStaParam[idx]->bufInfo.size        = feoRequest.size;
            mpStaParam[idx]->bufInfo.memID       = feoRequest.memID;
            mpStaParam[idx]->bufInfo.virtAddr    = feoRequest.virtAddr;
            mpStaParam[idx]->bufInfo.phyAddr     = feoRequest.phyAddr;
            mpStaParam[idx]->bufInfo.bufSecu     = 0;
            mpStaParam[idx]->bufInfo.bufCohe     = 0;
            mpStaParam[idx]->bufInfo.useNoncache = 0;
            mpStaParam[idx]->w                   = feSize.w;
            mpStaParam[idx]->h                   = feSize.h;
            mpStaParam[idx]->stride              = feSize.w;
            mpStaParam[idx]->port_idx            = FEO.index;
            mpStaParam[idx]->port_type           = FEO.type;
            mpStaParam[idx]->port_inout          = FEO.inout;

/*
            feoData.bufInfo.size        = feoRequest.size;
            feoData.bufInfo.memID       = feoRequest.memID;
            feoData.bufInfo.virtAddr    = feoRequest.virtAddr;
            feoData.bufInfo.phyAddr     = feoRequest.phyAddr;
            feoData.bufInfo.bufSecu     = 0;
            feoData.bufInfo.bufCohe     = 0;
            feoData.bufInfo.useNoncache = 0;
            feoData.w                   = FE_W;
            feoData.h                   = FE_H;
            feoData.stride              = FE_W;
            feoData.port_idx            = FEO.index;
            feoData.port_type           = FEO.type;
            feoData.port_inout          = FEO.inout;

            MY_LOGD("(0x%x)FEO:Size(%d),ID(%d),S(%dx%d,%d),VA(%p)PA(%p),Port(%d/%d/%d)",
                    &feoData,
                    feoData.bufInfo.size,
                    feoData.bufInfo.memID,
                    feoData.w,
                    feoData.h,
                    feoData.stride,
                    feoData.bufInfo.virtAddr,
                    feoData.bufInfo.phyAddr,
                    feoData.port_idx,
                    feoData.port_type,
                    feoData.port_inout);*/

            ModuleInfo feoInfo;
            feoInfo.moduleTag    = (MINT32)(EFeatureModule_STA_FEO);
            feoInfo.moduleStruct = reinterpret_cast<MVOID*>(mpStaParam[idx]);
            enqueParams.mvModuleData.push_back(feoInfo);

            mpSrzParam[idx]->in_w    = crop1.mResizeDst.w;
            mpSrzParam[idx]->in_h    = crop1.mResizeDst.h;
            mpSrzParam[idx]->out_w   = getFEImgSize().w;
            mpSrzParam[idx]->out_h   = getFEImgSize().h;

            ModuleInfo srzInfo;
            srzInfo.moduleTag = (MINT32)(EFeatureModule_SRZ1);
            srzInfo.moduleStruct = reinterpret_cast<MVOID*>(mpSrzParam[idx]);
            enqueParams.mvModuleData.push_back(srzInfo);

            MY_LOGD("[0x%x]FEO:Size(%d),ID(%d),S(%dx%d,%d),VA(%p)PA(%p),Port(%d/%d/%d);[0x%x]SRZ:in(%dx%d),out(%dx%d)",
                    mpStaParam[idx],
                    mpStaParam[idx]->bufInfo.size, mpStaParam[idx]->bufInfo.memID,
                    mpStaParam[idx]->w, mpStaParam[idx]->h, mpStaParam[idx]->stride,
                    mpStaParam[idx]->bufInfo.virtAddr, mpStaParam[idx]->bufInfo.phyAddr,
                    mpStaParam[idx]->port_idx, mpStaParam[idx]->port_type, mpStaParam[idx]->port_inout,
                    mpSrzParam[idx],
                    mpSrzParam[idx]->in_w, mpSrzParam[idx]->in_h, mpSrzParam[idx]->out_w, mpSrzParam[idx]->out_h);

        }
    }
    //
    enqueParams.mvCropRsInfo.push_back( crop1 );
    enqueParams.mvCropRsInfo.push_back( crop2 );
    //
    enqueParams.mpfnCallback = pass2CbFunc;
    enqueParams.mpCookie = this;
    //
    if( !mpIspSyncCtrlHw->lockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync lock pass2 failed");
        return MFALSE;
    }
    //
    CAM_TRACE_FMT_BEGIN("enqP2:%d", muEnqFrameCnt);
    if( !mpPostProcPipe->enque(enqueParams) )
    {
        CAM_TRACE_FMT_END();
        MY_LOGE("enque pass2 failed");
        AEE_ASSERT("ISP pass2 enque fail");

        if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS2) )
        {
            MY_LOGE("isp sync unlock pass2 failed");
        }
        return MFALSE;
    }
    CAM_TRACE_FMT_END();

    muEnqFrameCnt++;

    FUNC_END;
    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
pushBuf(MUINT32 const data, IImageBuffer* const buf, MUINT32 const ext)
{
    Mutex::Autolock lock(mLock);
    PostBufInfo postBufData = {data, buf, ext};
    mlPostBufData.push_back(postBufData);

    muPostFrameCnt++;

    if( isReadyToEnque() )
    {
        triggerLoop();
    }

    return MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
StereoNodeImpl::
handleP2Done(QParams& rParams)
{
    CAM_TRACE_CALL();
    CAM_TRACE_FMT_BEGIN("deqP2:%d", muDeqFrameCnt);
    MBOOL ret = MFALSE;
    MBOOL isZoom = (rParams.mvCropRsInfo[0].mCropRect.p_integral.x != 0) ? MTRUE : MFALSE;
    Vector<Input>::const_iterator iterIn;
    Vector<Output>::const_iterator iterOut;
    Vector<ModuleInfo>::const_iterator iterMInfo;
    //
    MY_LOGD("cnt %d in(%d) out(%d) moduleData(%d) isZoom(%d)",
            muDeqFrameCnt,
            rParams.mvIn.size(),
            rParams.mvOut.size(),
            rParams.mvModuleData.size(),
            isZoom);
    //
    if( !mpIspSyncCtrlHw->unlockHw(IspSyncControlHw::HW_PASS2) )
    {
        MY_LOGE("isp sync unlock pass2 failed");
        goto lbExit;
    }
    //
    for( iterIn = rParams.mvIn.begin(); iterIn != rParams.mvIn.end(); iterIn++ )
    {
        PortID portId = iterIn->mPortID;
        portId.group = 0;
        MUINT32 nodeDataType = mapToNodeDataType( portId );
        MY_LOGD("In PortID(0x%08X), nodeData(%d)",portId, nodeDataType);
        handleReturnBuffer( nodeDataType, (MUINTPTR)iterIn->mBuffer);
    }
    //
    for( iterOut = rParams.mvOut.begin(); iterOut != rParams.mvOut.end(); iterOut++ )
    {
        PortID portId = iterOut->mPortID;
        portId.group = 0;
        MUINT32 nodeDataType = mapToNodeDataType( portId );
        MY_LOGD("Out PortID(0x%08X), nodeData(%d), bufInfo(0x%x), va/pa(0x%x/0x%x), ID(%d)",
                portId, nodeDataType, iterOut->mBuffer, iterOut->mBuffer->getBufVA(0), iterOut->mBuffer->getBufPA(0), iterOut->mBuffer->getFD());
        handlePostBuffer( nodeDataType, (MUINTPTR)iterOut->mBuffer, (MUINT32)isZoom );
    }
    for( iterMInfo = rParams.mvModuleData.begin(); iterMInfo != rParams.mvModuleData.end(); iterMInfo++ )
    {
        if ( iterMInfo->moduleTag == EFeatureModule_STA_FEO )
        {
            StaData* pModuleData = reinterpret_cast<StaData*>(iterMInfo->moduleStruct);
            PortID portId = PortID((EPortType)pModuleData->port_type, pModuleData->port_idx, pModuleData->port_inout);
            portId.group = 0;
            MUINT32 nodeDataType = mapToNodeDataType( portId );
            MY_LOGD("ModuleInfo PortID(0x%08X), nodeData(%d), bufInfo(0x%x), va/pa(0x%x/0x%x), ID(%d)",
                    portId, nodeDataType, &(pModuleData->bufInfo), pModuleData->bufInfo.virtAddr, pModuleData->bufInfo.phyAddr, pModuleData->bufInfo.memID);
            handlePostBuffer( nodeDataType, (MUINTPTR)&pModuleData->bufInfo, (MUINT32)isZoom );
        }
    }
    //
    {
        Mutex::Autolock lock(mLock);
        muDeqFrameCnt++;
        mCondDeque.broadcast();
    }
    //
    ret = MTRUE;
lbExit:
    CAM_TRACE_FMT_END();
    return ret;

}


/*******************************************************************************
*
********************************************************************************/
MVOID
StereoNodeImpl::
pass2CbFunc(QParams& rParams)
{
     StereoNodeImpl* pNodeImpl = (StereoNodeImpl*)(rParams.mpCookie);
     pNodeImpl->handleP2Done(rParams);
}


/*******************************************************************************
*
********************************************************************************/
PortID
StereoNodeImpl::
mapToPortID(MUINT32 const nodeDataType)
{
    //hard-coded mapping
    switch(nodeDataType)
    {
        case STEREO_SRC:
            return IMGI;
            break;
        case STEREO_IMG:
            return IMG3O;
            break;
        case STEREO_FEO:
            return FEO;
            break;
        case STEREO_RGB:
            return WDMAO;
        default:
            break;
    }
    return PortID();
}


/*******************************************************************************
*
********************************************************************************/
MUINT32
StereoNodeImpl::
mapToNodeDataType(PortID const portId)
{
    //hard-coded mapping
    if( portId == IMGI )
        return STEREO_SRC;
    else
    if( portId == IMG3O )
        return STEREO_IMG;
    else
    if( portId == FEO )
        return STEREO_FEO;
    else
    if ( portId == WDMAO )
        return STEREO_RGB;
    return 0;
}


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSCamNode

