/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */
//#define LOG_TAG "MtkCam/PostProc.PipeWrapper"
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   LOG_TAG    // Decide a Log TAG for current file.
#define  LOG_TAG    "FPipeEis"

#include <mtkcam/Log.h>

// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__

#define LOG_VRB(fmt, arg...)        CAM_LOGV("[%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[%s] " fmt, __func__, ##arg)
#define LOG_INF(fmt, arg...)        CAM_LOGI("[%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[ERROR][%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[%s] " fmt, __func__, ##arg); } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");
//
#define FEATUREPIPE_EIS_NAME "FeaturePipeEis"
//
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <vector>
#include <queue>
#include <cutils/atomic.h>
#include <cutils/properties.h>
#include <ui/GraphicBuffer.h>
#include "FeaturePipe.BufHandler.h"
#include "FeaturePipe.h"
#include "MTKWarp.h"
#include "MTKFeatureSet.h"
#include "DpBlitStream.h"
#include <mtkcam/utils/Format.h>
#include <mtkcam/common.h>
#include <mtkcam/featureio/IHal3A.h>
#include "eis_hal.h"
#include "vhdr_hal.h"
#include "FeaturePipeNode.h"
#include "FeaturePipe.EIS.h"

using namespace NSCam;
using namespace NSCamNode;
using namespace NSCam::Utils::Format;

/******************************************************************************
*
******************************************************************************/

static MINT32 g_debugDump = 0;

#define EIS_GPU_WARP_MAP_NUM 3

/******************************************************************************
*
******************************************************************************/
#ifdef _DEBUG_DUMP
#include <fcntl.h>
#include <sys/stat.h>

static bool saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            LOG_ERR("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    LOG_INF("done writing %d bytes to file [%s]\n", size, fname);
    ::close(fd);
    return true;
}

#endif

/******************************************************************************
*
******************************************************************************/
FeaturePipeEisNode *FeaturePipeEisNode::createInstance(MUINT32 featureOnMask,MUINT32 mOpenedSensorIndex)
{
    LOG_AST(FEATURE_MASK_IS_EIS_ENABLED(featureOnMask), "Create EIS node while EIS is off", 0);
    return new FeaturePipeEisNode("FeaturePipeEisNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY,featureOnMask,mOpenedSensorIndex);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::destroyInstance()
{
    delete this;
}

/******************************************************************************
*
******************************************************************************/
FeaturePipeEisNode::FeaturePipeEisNode( const char* name,
                                              eThreadNodeType type,
                                              int policy,
                                              int priority,
                                              MUINT32 featureOnMask,
                                              MUINT32 mOpenedSensorIndex)
                                              : FeaturePipeNode(name, type, policy, priority, featureOnMask, mOpenedSensorIndex)
{
    if(mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)
    {
        LOG_DBG("EisPlus+VFB");
        addDataSupport(ENDPOINT_SRC, P2A_TO_EISSW);
        addDataSupport(ENDPOINT_DST, EISSW_TO_VFBSW);
    }
    else
    {
        LOG_DBG("EisPlus");
        addDataSupport(ENDPOINT_SRC, P2A_TO_EISSW);
        addDataSupport(ENDPOINT_DST, EISSW_TO_GPU);
    }

    mpEisHal = NULL;

    while(!mFrameNum.empty())
    {
        mFrameNum.pop();
    }

    while(!mEisHalCfgData.empty())
    {
        mEisHalCfgData.pop();
    }

    while(!mGpuTargetW.empty())
    {
        mGpuTargetW.pop();
    }

    while(!mGpuTargetH.empty())
    {
        mGpuTargetH.pop();
    }
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onInit()
{
    FUNCTION_LOG_START;

    MRESULT mret;
    MUINT8 imemBufInfoIdx = 0;
    NodeDataTypes allocDataType = (mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)? EISSW_TO_VFBSW: EISSW_TO_GPU;

    //====== Dynamic Debug ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.eis.dump", value, "0");
    g_debugDump = atoi(value);

    //====== Create EIS HAL instance ======

    LOG_DBG("mOpenedSensorIndex(%u)",mOpenedSensorIndex);
    mpEisHal = EisHal::CreateInstance(FEATUREPIPE_EIS_NAME,mOpenedSensorIndex);
    if (mpEisHal == NULL)
    {
        LOG_ERR("mpEisHal CreateInstance failed");
        goto _Exit;
    }

    if(EIS_RETURN_NO_ERROR != mpEisHal->Init())
    {
        LOG_ERR("mpEisHal init failed");
        goto _Exit;
    }

    //====== Allocate buffer for next node ======

    pBufHandler = getBufferHandler(allocDataType);
    if (pBufHandler == NULL)
    {
        LOG_ERR("ICamBufHandler failed!");
        goto _Exit;
    }

    //> get EIS Plus GPU grid size

    mpEisHal->GetEisPlusGpuInfo(&mEisPlusGpuInfo);

    LOG_DBG("GPU grid (W/H)=(%u/%u)",mEisPlusGpuInfo.gridW,mEisPlusGpuInfo.gridH);

    // GPU warp map

    {
        AllocInfo allocinfo(mEisPlusGpuInfo.gridW * 4 , //Warp Map X + Y (both X and Y need gridW * gridH * 4 bytes)
                            mEisPlusGpuInfo.gridH * 2,
                            eImgFmt_BAYER8,
                            eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_OFTEN);

        for(MUINT32 i = 0; i < EIS_GPU_WARP_MAP_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(allocDataType, allocinfo) )
            {
                LOG_ERR("requestBuffer failed!");
                goto _Exit;
            }
        }
    }

    //====== Register EIS Pass2 User ======

    //RegisterEisP2User();

    FUNCTION_LOG_END;
    return MTRUE;

_Exit:

    if (mpEisHal != NULL)
    {
        mpEisHal->DestroyInstance(FEATUREPIPE_EIS_NAME);
    }
    return MFALSE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);

    FUNCTION_LOG_START;

    if (mpEisHal != NULL)
    {
        mpEisHal->Uninit();
        mpEisHal->DestroyInstance(FEATUREPIPE_EIS_NAME);
        mpEisHal = NULL;
    }

    while(!mFrameNum.empty())
    {
        mFrameNum.pop();
    }

    while(!mEisHalCfgData.empty())
    {
        mEisHalCfgData.pop();
    }

    while(!mGpuTargetW.empty())
    {
        mGpuTargetW.pop();
    }

    while(!mGpuTargetH.empty())
    {
        mGpuTargetH.pop();
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onNotify(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    LOG_DBG("msg(0x%x), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);

    Mutex::Autolock lock(mLock);
#if 0   // EISPlus does not need input image

    FeaturePipeNodeJob_s job;

    job.pBufList = &((reinterpret_cast<FeaturePipeNodeBuf_s*>(buf))->bufList);
    job.mFrameNo = (reinterpret_cast<FeaturePipeNodeBuf_s*>(buf))->mFrameNo;
    job.buf = buf;
    job.ext = ext;
    mqJobInput.push(job);
#endif
    mFrameNum.push(ext);
    triggerLoop();

    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);

    MRESULT mret;

#if 0   // EISPlus does not need input image

    //====== Get P2A Image ======

    LOG_AST(mqJobInput.front().pBufList->begin()->etype == FEATURE_STREAM_BUF_FULL_IMAGE, "Wrong type of input buffer", 0);

    FeaturePipeNodeJob_s jobP2aImg;
    jobP2aImg = mqJobInput.front();
    mqJobInput.pop();
    MUINT32 frameNo = mqJobInput.mFrameNo;
#endif

    //====== EISPlus Process ======

    ImgRequest warpImgReq;
    NodeDataTypes allocDataType = (mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)? EISSW_TO_VFBSW: EISSW_TO_GPU;
    if(!pBufHandler->dequeBuffer(allocDataType, &warpImgReq))
    {
        LOG_AST(false, "Out of buffer", 0);
        return MFALSE;
    }

    // because the return type of getBufVA is MINT32, therefore need to mutiply (mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH) by 4
    mpEisHal->SetEisPlusGpuInfo((MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0)),
                                (MINT32 *) (const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0) + mEisPlusGpuInfo.gridW*mEisPlusGpuInfo.gridH*4));

    //> prepare config data

    EIS_HAL_CONFIG_DATA eisHalCfgData;

    {
        Mutex::Autolock lock(mLock);

        if(mEisHalCfgData.empty())
        {
            LOG_ERR("empty mEisHalCfgData queue");
        }
        else
        {
            eisHalCfgData.imgiW      = mEisHalCfgData.front().imgiW;
            eisHalCfgData.imgiH      = mEisHalCfgData.front().imgiH;
            eisHalCfgData.crzOutW    = mEisHalCfgData.front().crzOutW;
            eisHalCfgData.crzOutH    = mEisHalCfgData.front().crzOutH;
            eisHalCfgData.srzOutW    = mEisHalCfgData.front().srzOutW;
            eisHalCfgData.srzOutH    = mEisHalCfgData.front().srzOutH;
            eisHalCfgData.feTargetW  = mEisHalCfgData.front().feTargetW;
            eisHalCfgData.feTargetH  = mEisHalCfgData.front().feTargetH;
            eisHalCfgData.gpuTargetW = mEisHalCfgData.front().gpuTargetW;
            eisHalCfgData.gpuTargetH = mEisHalCfgData.front().gpuTargetH;
            eisHalCfgData.cropX      = mEisHalCfgData.front().cropX;
            eisHalCfgData.cropY      = mEisHalCfgData.front().cropY;

            mEisHalCfgData.pop();
        }
    }

    //> do EIS pass2

    mpEisHal->DoEis(EIS_PASS_2,&eisHalCfgData);

    if(g_debugDump >= 1)
    {
        MUINT32 *ptr  = (MUINT32 *) const_cast<IImageBuffer*>(warpImgReq.mBuffer)->getBufVA(0);
        for(MUINT32 i = 0; i < 8; i++)
        {
            LOG_INF("EIS WarpMap(%p,%d)",(ptr+i),*(ptr+i));
        }
    }

#if 0   // EISPlus does not need input image

    //====== Return Buffer to Previous Node ======

    handleReturnBuffer(P2A_TO_EISSW, jobP2aImg.buf, jobP2aImg.ext);

#endif

    //====== Post Buffer to Next Node ======

    if(mFeatureOnMask & FEATURE_ON_MASK_BIT_VFB)    //vFB on
    {
        LOG_DBG("EIS->vFB warp map(%u)", mFrameNum.front());
        handlePostBuffer(EISSW_TO_VFBSW, (MUINTPTR) &warpImgReq, mFrameNum.front());
    }
    else    //vFB off
    {
        LOG_DBG("EIS->GPU warp map(%u)", mFrameNum.front());
        handlePostBuffer(EISSW_TO_GPU, (MUINTPTR) &warpImgReq, mFrameNum.front());
    }

    {
        Mutex::Autolock lock(mLock);
        mFrameNum.pop();
    }

    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    LOG_DBG("data(0x%x),buf(0x%x),ext(0x%x)", data, buf, ext);

    pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

/******************************************************************************
*
******************************************************************************/
MBOOL FeaturePipeEisNode::ConfigEis(const EIS_PASS_ENUM &aEisPass,const EIS_HAL_CONFIG_DATA &aEisConfig)
{
    Mutex::Autolock lock(mLock);

    mEisHalCfgData.push(aEisConfig);
    mGpuTargetW.push(aEisConfig.gpuTargetW);
    mGpuTargetH.push(aEisConfig.gpuTargetH);

    if(EIS_RETURN_NO_ERROR != mpEisHal->ConfigEis(aEisPass,aEisConfig))
    {
        LOG_ERR("mpEisHal->ConfigEis fail");
        return MFALSE;
    }

    return MTRUE;
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisResult( MUINT32 &a_CMV_X_Int,
                                           MUINT32 &a_CMV_X_Flt,
                                           MUINT32 &a_CMV_Y_Int,
                                           MUINT32 &a_CMV_Y_Flt,
                                           MUINT32 &a_TarWidth,
                                           MUINT32 &a_TarHeight)
{
    mpEisHal->GetEisResult(a_CMV_X_Int,a_CMV_X_Flt,a_CMV_Y_Int,a_CMV_Y_Flt,a_TarWidth,a_TarHeight);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisGmv(MINT32 &aGMV_X,MINT32 &aGMV_Y,MUINT32 *aEisInW,MUINT32 *aEisInH)
{
    mpEisHal->GetEisGmv(aGMV_X,aGMV_Y,aEisInW,aEisInH);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetFeoRegInfo(FEO_REG_INFO *aFeoRegInfo)
{
    mpEisHal->GetFeoRegInfo(aFeoRegInfo);
}

/******************************************************************************
*
******************************************************************************/
MUINT32 FeaturePipeEisNode::GetEisPlusCropRatio()
{
    return mpEisHal->GetEisPlusCropRatio();
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisPlusTargetSize(MUINT32 &aWidth,MUINT32 &aHeight)
{
    Mutex::Autolock lock(mLock);

    if(mGpuTargetW.empty() || mGpuTargetH.empty())
    {
        LOG_ERR("empty queue(%d,%d)",mGpuTargetW.size(),mGpuTargetH.size());
        return;
    }

    aWidth  = mGpuTargetW.front();
    aHeight = mGpuTargetH.front();

    mGpuTargetW.pop();
    mGpuTargetH.pop();
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::GetEisInfoAtP2(char const *userName,EIS_P1_RESULT_INFO *apEisResult,const MINT64 &aTimeStamp,MBOOL lastUser)
{
    mpEisHal->GetEisInfoAtP2(userName,apEisResult,aTimeStamp,lastUser);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::FlushMemory(const EIS_DMA_ENUM &aDma,const EIS_FLUSH_ENUM &aFlush)
{
    mpEisHal->FlushMemory(aDma,aFlush);
}

/******************************************************************************
*
******************************************************************************/
MVOID FeaturePipeEisNode::RegisterEisP2User()
{
    //mpEisHal->SetEisP2User("3DNR");
}


