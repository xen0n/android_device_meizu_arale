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
#define  LOG_TAG    "MtkCam/FeaturePipe"

#include <mtkcam/Log.h>
#include <aee.h>
#include "camera_custom_3dnr.h"

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

#define FUNCTION_LOG_START      LOG_DBG("+");
#define FUNCTION_LOG_END        LOG_DBG("-");
#define ERROR_LOG               LOG_ERR("Error");

// Added by Marx's request.
#define AEE_ASSERT(String)    \
    do {                      \
        aee_system_exception( \
            LOG_TAG,          \
            NULL,             \
            DB_OPT_DEFAULT | DB_OPT_FTRACE,   \
            String);          \
    } while(0)
//

#include <sys/time.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>
#include <vector>
#include <queue>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <ui/GraphicBuffer.h>
#include "FeaturePipe.BufHandler.h"
#include "FeaturePipe.h"
#include "MTKWarp.h"
#include "MTKFeatureSet.h"
#include "DpBlitStream.h"
#include "DpIspStream.h"
#include <mtkcam/hwutils/HwMisc.h>
#include <mtkcam/utils/Format.h>
#include <mtkcam/common.h>
#include <mtkcam/common/faces.h>
#include "vfb_hal_base.h"
#include "eis_hal.h"    // For EisHal in FeaturePipeNode.h.
#include "eis_type.h"   // For EIS_P1_RESULT struct.
#include "vhdr_hal.h"
#include <fd_hal_base.h>
#include <mtkcam/featureio/IHal3A.h>
#include "mtkcam/hal/IHalSensor.h"
#include "FeaturePipeNode.h"
#include "FeaturePipe.vFB.h"
#include "FeaturePipe.EIS.h"
//#include "aaa_hal_common.h"
#include "PortMap.h"
#include <cutils/properties.h>  // For property_get().

#ifdef USING_MTK_PERFSERVICE
//#include "PerfServiceNative.h"
#include <dlfcn.h>
#include <mtkcam/drv/res_mgr_drv.h>
#endif //USING_MTK_PERFSERVICE


using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
using namespace NSCamNode;
using namespace NSCam::Utils::Format;

//#define _CPUWARP
//#define _WORKAROUND_P2B
//#define _WORKAROUND_GPU_FLASH
#define NR3D_FORCE_GMV_ZERO     0
#define NR3D_NO_HW_POWER_OFF    0
#define ROUND_TO_2X(x) ((x) & (~0x1))

// mkdbg
static char sgmkDebugFlag[PROPERTY_VALUE_MAX] = {'\0'};

static inline MUINT32 getMagicNo(QParams const& rParams)
{
    if (rParams.mvMagicNo.size())
        return rParams.mvMagicNo[0];
    else
        return rParams.mFrameNo;
}

static MINT32
convertRGBA8888toYV12(MINTPTR srcBufAddr, int srcBufWidth, int srcBufHeight, IImageBuffer* dstIImageBuffer)
{
    bool ret = true;
    FUNCTION_LOG_START;
    DpBlitStream dpBlit;

    //***************************src RGBA8888****************************//
    MINTPTR src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    int plane_num = 1;
    src_addr_list[0] = srcBufAddr;
    src_size_list[0] = FULL_IMAGE_MAX_WIDTH * srcBufHeight * 4;
    dpBlit.setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    dpBlit.setSrcConfig(srcBufWidth, srcBufHeight, FULL_IMAGE_MAX_WIDTH * 4, 0, DP_COLOR_RGBA8888, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);

    //***************************dst YV12********************************//
    MINTPTR dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    plane_num = 3;
    dst_addr_list[0] = dstIImageBuffer->getBufVA(0);
    dst_addr_list[1] = dstIImageBuffer->getBufVA(1);
    dst_addr_list[2] = dstIImageBuffer->getBufVA(2);

    dst_size_list[0] = dstIImageBuffer->getBufSizeInBytes(0);
    dst_size_list[1] = dstIImageBuffer->getBufSizeInBytes(1);
    dst_size_list[2] = dstIImageBuffer->getBufSizeInBytes(2);
    dpBlit.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    dpBlit.setDstConfig(dstIImageBuffer->getImgSize().w, dstIImageBuffer->getImgSize().h, dstIImageBuffer->getBufStridesInBytes(0), dstIImageBuffer->getBufStridesInBytes(1), DP_COLOR_YV12, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);
    dpBlit.setRotate(0);

    // set & add pipe to stream
    if (dpBlit.invalidate())  //trigger HW
    {
          LOG_ERR("[convertRGBA8888toYV12] FDstream invalidate failed");
          return false;
    }
    FUNCTION_LOG_END;
    return ret;
}

#if 1//def _DEBUG_DUMP
#include <fcntl.h>
#include <sys/stat.h>

static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_DBG("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_DBG("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    //LOG_DBG("writing %d bytes to file [%s]\n", size, fname);
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
    LOG_DBG("done writing %d bytes to file [%s]\n", size, fname);
    ::close(fd);
    return true;
}
#endif
#ifdef _UNITTEST
/******************************************************************************
*   read the file to the buffer
*******************************************************************************/
static uint32_t
loadFileToBuf(char const*const fname, uint8_t*const buf, uint32_t size)
{
    int nr, cnt = 0;
    uint32_t readCnt = 0;

    LOG_DBG("opening file [%s] adr 0x%x\n", fname,buf);
    int fd = ::open(fname, O_RDONLY);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, strerror(errno));
        return readCnt;
    }
    //
    if (size == 0) {
        size = ::lseek(fd, 0, SEEK_END);
        ::lseek(fd, 0, SEEK_SET);
    }
    //
    LOG_DBG("read %d bytes from file [%s]\n", size, fname);
    while (readCnt < size) {
        nr = ::read(fd,
                    buf + readCnt,
                    size - readCnt);
        if (nr < 0) {
            LOG_ERR("failed to read from file [%s]: %s",
                        fname, strerror(errno));
            break;
        }
        readCnt += nr;
        cnt++;
    }
    LOG_DBG("done reading %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);

    return readCnt;
}
#endif

#ifdef USING_MTK_PERFSERVICE
static MBOOL   initPerf();
static MBOOL   uninitPerf();

MBOOL enablePerfService(MUINT32 scen);
MBOOL disablePerfService(MUINT32 scen);

#define PERF_LIB_FULLNAME        "libperfservicenative.so"
#define STR_FUNC_REG             "PerfServiceNative_userRegBigLittle"
#define STR_FUNC_ENABLETIMEOUT   "PerfServiceNative_userEnable"
#define STR_FUNC_DISABLE         "PerfServiceNative_userDisable"
#define STR_FUNC_UNREG           "PerfServiceNative_userUnreg"
typedef int (*funcPerfRegBL)(int, int, int, int);
typedef void (*funcEnableTimeout)(int, int);
typedef void (*funcDisable)(int);
typedef int (*funcPerfUnreg)(int);
static Mutex               gPerfLock;
static void*               gPerfLib = NULL;
static funcEnableTimeout   gPerfEnable = NULL;
static funcDisable         gPerfDisable = NULL;
static int                 gPerfEnabledIdx = -1;
//
struct camera_scenario
{
    MUINT32 scen;
    int bigcore;
    int bigfreq;
    int littlecore;
    int littlefreq;
    int timeout;
};

#define SCENARIO_SIZE   (2)
static camera_scenario gScenTable[SCENARIO_SIZE] =
{
#if 1
#if 0
    { ResMgrDrv::SCEN_SW_CAM_PRV, 2, 2002000, 0, 0, 1 },
    { ResMgrDrv::SCEN_SW_CAM_CAP, 2, 2002000, 0, 0, 1 },
#endif
#if 1
    { ResMgrDrv::SCEN_SW_CAM_PRV, 0, 0, 4, 1300000, 1 },
    { ResMgrDrv::SCEN_SW_CAM_CAP, 0, 0, 4, 1300000, 1 },
#endif
#else
    { ResMgrDrv::SCEN_SW_CAM_PRV, 0, 0, 4, 2002000, 1 },
    { ResMgrDrv::SCEN_SW_CAM_CAP, 0, 0, 4, 2002000, 1 },
#endif
};

static int gScenHandle[SCENARIO_SIZE] =
{ -1, -1 };

//-----------------------------------------------------------------------------
MBOOL
FeaturePipe::
initPerf()
{
    LOG_ERR("");
    Mutex::Autolock _l(gPerfLock);
    if( !gPerfLib )
    {
        gPerfLib = dlopen(PERF_LIB_FULLNAME, RTLD_NOW);
        if  ( ! gPerfLib )
        {
            char const *err_str = dlerror();
            LOG_ERR("dlopen: %s error=%s", PERF_LIB_FULLNAME, (err_str ? err_str : "unknown"));
            goto lbExit;
        }
    }

    gPerfEnable = reinterpret_cast<funcEnableTimeout>(dlsym(gPerfLib, STR_FUNC_ENABLETIMEOUT));
    if( gPerfEnable == NULL )
        LOG_ERR("cannot get %s", STR_FUNC_ENABLETIMEOUT);
    //
    gPerfDisable = reinterpret_cast<funcDisable>(dlsym(gPerfLib, STR_FUNC_DISABLE));
    if( gPerfDisable == NULL )
        LOG_ERR("cannot get %s", STR_FUNC_DISABLE);
    //
    {
        funcPerfRegBL pReg = NULL;
        pReg = reinterpret_cast<funcPerfRegBL>(dlsym(gPerfLib, STR_FUNC_REG));
        if( pReg == NULL )
        {
            LOG_ERR("cannot get %s", STR_FUNC_REG);
            goto lbExit;
        }

        for( MUINT32 i = 0 ; i < SCENARIO_SIZE; i++ )
        {
            if( gScenHandle[i] == - 1 )
            {
                gScenHandle[i] = pReg(
                        gScenTable[i].bigcore,
                        gScenTable[i].bigfreq,
                        gScenTable[i].littlecore,
                        gScenTable[i].littlefreq
                        );
                if( gScenHandle[i] == -1 )
                {
                    LOG_ERR("register scenario failed");
                    goto lbExit;
                }
            }
        }

    }
    //
lbExit:
    return MTRUE;
}
//-----------------------------------------------------------------------------
MBOOL
FeaturePipe::
uninitPerf()
{
    LOG_ERR("");
    Mutex::Autolock _l(gPerfLock);
    funcPerfUnreg pUnreg = NULL;
    //
    if( !gPerfLib )
    {
        LOG_ERR("no lib");
        return MFALSE;
    }
    //
    if( gPerfEnabledIdx != -1 )
    {
        if( gPerfDisable )
        {
            LOG_ERR("disable previous scen idx %d", gPerfEnabledIdx);
            gPerfDisable(gScenHandle[gPerfEnabledIdx]);
            gPerfEnabledIdx = -1;
        }
        else
        {
            LOG_ERR("cannot disable idx %d", gPerfEnabledIdx);
        }
    }
    //
    pUnreg = reinterpret_cast<funcPerfUnreg>(dlsym(gPerfLib, STR_FUNC_UNREG));
    if( pUnreg != NULL )
    {
        for( MUINT32 i = 0 ; i < SCENARIO_SIZE; i++ )
        {
            if( gScenHandle[i] != - 1 )
            {
                pUnreg(gScenHandle[i]);
                gScenHandle[i] = -1;
            }
        }
    }
    else
    {
        LOG_ERR("cannot get %s", STR_FUNC_UNREG);
    }
    //
    gPerfEnable = NULL;
    gPerfDisable = NULL;
    //
    dlclose(gPerfLib);
    gPerfLib = NULL;
    //
    return MTRUE;
}


MBOOL
FeaturePipe::
enablePerfService(MUINT32 scen)
{
    if( !gPerfEnable )
    {
        LOG_ERR("no func");
        return MFALSE;
    }
    //
    if( gPerfEnabledIdx != -1 )
    {
        LOG_ERR("disable previous scen idx %d", gPerfEnabledIdx);
        disablePerfService(gScenTable[gPerfEnabledIdx].scen);
    }
    //
    for( MUINT32 i = 0 ; i < SCENARIO_SIZE; i++ )
    {
        if( gScenTable[i].scen == scen )
        {
            if( gScenHandle[i] != -1 )
            {
                LOG_ERR("enable PerfService, scen %d, idx %d", scen, i);
                gPerfEnable(gScenHandle[i], gScenTable[i].timeout);
                gPerfEnabledIdx = i;
            }
            break;
        }
    }
    //
    if( gPerfEnabledIdx == -1 )
    {
        LOG_ERR("cannot find rule for scen %d", scen);
    }
    //
    return MTRUE;

}
//-----------------------------------------------------------------------------
MBOOL
FeaturePipe::
disablePerfService(MUINT32 scen)
{

    if( !gPerfDisable )
    {
        LOG_ERR("no func");
        return MFALSE;
    }
    //
    if( gPerfEnabledIdx == -1 )
    {
        return MTRUE;
    }
    //
    for( MUINT32 i = 0 ; i < SCENARIO_SIZE; i++ )
    {
        if( gScenTable[i].scen == scen )
        {
            LOG_ERR("disable PerfService, scen %d, idx %d", scen, i);
            if( gPerfEnabledIdx != i )
            {
                LOG_ERR("idx not matched enabled %d vs. %d", gPerfEnabledIdx, i );
            }
            gPerfDisable(gScenHandle[gPerfEnabledIdx]);
            gPerfEnabledIdx = -1;
            break;
        }
    }
    return MTRUE;
}
#endif //USING_MTK_PERFSERVICE



/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::initFeatureGraph()
{
    FUNCTION_LOG_START;

    bool ret = MTRUE;

    CPTLog(Event_FeaturePipe, CPTFlagStart);
    CPTLog(Event_FeaturePipe_init, CPTFlagStart);

    // mkdbg
    property_get("debug.fpipe.p2a.mk", sgmkDebugFlag, "0");

    /* Create nodes */
    //mpControlNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_CONTROL, mFeatureOnMask, mOpenedSensorIndex);
    mpP2aNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_P2A, mFeatureOnMask, mOpenedSensorIndex, this);
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
            FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpGpuNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_GPU, mFeatureOnMask, mOpenedSensorIndex, this);
        mpMdpNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_MDP, mFeatureOnMask, mOpenedSensorIndex, this);
    }
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        mpVfbSwNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_VFBSW, mFeatureOnMask, mOpenedSensorIndex, this);
        mpFdNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_FD, mFeatureOnMask, mOpenedSensorIndex, this);
        mpP2bNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_P2B, mFeatureOnMask, mOpenedSensorIndex, this);
        mpSwitcherNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_SWITCHER, mFeatureOnMask, mOpenedSensorIndex, this);
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpEisSwNode = FeaturePipeNode::createInstance(FEATURE_STREAM_NODE_EISSW, mFeatureOnMask, mOpenedSensorIndex, this);
    }

    mpAllocBufHdl = FeaturePipeBufHandler::createInstance();
    mpAllocBufHdl->init();

    /* Connect nodes */
    //mpCamGraph->connect(0x1, mpControlNode, mpP2aNode);
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
#ifdef USING_MTK_PERFSERVICE
        initPerf();
        enablePerfService(gScenTable[0].scen);
        misPerfService = MTRUE;
        LOG_DBG("enable PerfService");
#endif

        mpCamGraph->setBufferHandler(P2A_TO_VFBSW_DSIMG,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(P2A_TO_SWITCHER,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(P2A_TO_FD_DSIMG,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(FD_TO_VFBSW,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(VFBSW_TO_GPU,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(VFBSW_TO_P2B_DSIMG,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(VFBSW_TO_P2B_ALPCL,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(VFBSW_TO_P2B_ALPNR,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(VFBSW_TO_P2B_PCA,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(GPU_TO_MDP,  mpAllocBufHdl);
        mpCamGraph->setBufferHandler(MDP_TO_SWITCHER,  mpAllocBufHdl);

        mpCamGraph->connectData(P2A_TO_VFBSW_DSIMG, P2A_TO_VFBSW_DSIMG, mpP2aNode, mpVfbSwNode);
        mpCamGraph->connectData(P2A_TO_SWITCHER, P2A_TO_SWITCHER, mpP2aNode, mpSwitcherNode);
        mpCamGraph->connectData(P2A_TO_FD_DSIMG, P2A_TO_FD_DSIMG, mpP2aNode, mpFdNode);
        mpCamGraph->connectData(FD_TO_VFBSW, FD_TO_VFBSW, mpFdNode, mpVfbSwNode);
        mpCamGraph->connectData(VFBSW_TO_GPU, VFBSW_TO_GPU, mpVfbSwNode, mpGpuNode);
        mpCamGraph->connectData(VFBSW_TO_P2B_DSIMG, VFBSW_TO_P2B_DSIMG, mpVfbSwNode, mpP2bNode);
        mpCamGraph->connectData(VFBSW_TO_P2B_ALPCL, VFBSW_TO_P2B_ALPCL, mpVfbSwNode, mpP2bNode);
        mpCamGraph->connectData(VFBSW_TO_P2B_ALPNR, VFBSW_TO_P2B_ALPNR, mpVfbSwNode, mpP2bNode);
        mpCamGraph->connectData(VFBSW_TO_P2B_PCA, VFBSW_TO_P2B_PCA, mpVfbSwNode, mpP2bNode);
        mpCamGraph->connectData(GPU_TO_MDP, GPU_TO_MDP, mpGpuNode, mpMdpNode);
        mpCamGraph->connectData(MDP_TO_SWITCHER, MDP_TO_SWITCHER, mpMdpNode, mpSwitcherNode);
        mpCamGraph->connectData(P2A_TO_P2B, P2A_TO_P2B, mpP2aNode, mpP2bNode);
        mpCamGraph->connectData(SWITCHER_TO_GPU, SWITCHER_TO_GPU, mpSwitcherNode, mpGpuNode);
        mpCamGraph->connectData(SWITCHER_TO_VFBSW, SWITCHER_TO_VFBSW, mpSwitcherNode, mpVfbSwNode);
        mpCamGraph->connectData(SWITCHER_TO_P2B, SWITCHER_TO_P2B, mpSwitcherNode, mpP2bNode);

        mpCamGraph->connectNotify(FEATURE_MASK_CHANGE, mpP2aNode, mpFdNode);
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpCamGraph->setBufferHandler(P2A_TO_EISSW,  mpAllocBufHdl);
        mpCamGraph->connectData(P2A_TO_EISSW, P2A_TO_EISSW, mpP2aNode, mpEisSwNode);
        if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            mpCamGraph->setBufferHandler(EISSW_TO_VFBSW,  mpAllocBufHdl);
            mpCamGraph->connectData(EISSW_TO_VFBSW, EISSW_TO_VFBSW, mpEisSwNode, mpVfbSwNode);
        }
        else
        {
            mpCamGraph->setBufferHandler(P2A_TO_GPU,  mpAllocBufHdl);
            mpCamGraph->setBufferHandler(EISSW_TO_GPU,  mpAllocBufHdl);
            mpCamGraph->setBufferHandler(GPU_TO_MDP,  mpAllocBufHdl);

            mpCamGraph->connectData(P2A_TO_GPU, P2A_TO_GPU, mpP2aNode, mpGpuNode);
            mpCamGraph->connectData(P2A_TO_MDP_QPARAMS, P2A_TO_MDP_QPARAMS, mpP2aNode, mpMdpNode);
            mpCamGraph->connectData(EISSW_TO_GPU, EISSW_TO_GPU, mpEisSwNode, mpGpuNode);
            mpCamGraph->connectData(GPU_TO_MDP, GPU_TO_MDP, mpGpuNode, mpMdpNode);

            char gpuDebug[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.featurepipe.dumpgpu", gpuDebug, "0");
            if(gpuDebug[0] == '2' || gpuDebug[0] == '3')
            {
                mpCamGraph->setBufferHandler(8001,mpAllocBufHdl);
            }

            char mdpDebug[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.featurepipe.dumpmdp", mdpDebug, "0");
            if(mdpDebug[0] == '1' || mdpDebug[0] == '3')
            {
                mpCamGraph->setBufferHandler(8001,mpAllocBufHdl);
            }

        }
    }

    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        mpCamGraph->setBufferHandler(P2A_TO_GPU,    mpAllocBufHdl);
    }

    if (!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
            !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpCamGraph->connectData(0, 0, mpP2aNode, NULL);
    }

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
        reinterpret_cast<FeaturePipeVfbNode*>(mpVfbSwNode)->set(RECORDING_HINT, (MINTPTR) mRecordingHint);
#endif
    }

    /* Init graph */
    mpCamGraph->init();
    mpCamGraph->start();

    CPTLog(Event_FeaturePipe_init, CPTFlagEnd);

    FUNCTION_LOG_END;
    return ret;
}


/******************************************************************************
 *
 ******************************************************************************/
MBOOL
FeaturePipe::uninitFeatureGraph()
{
    FUNCTION_LOG_START;
    bool ret = true;

    CPTLog(Event_FeaturePipe_uninit, CPTFlagStart);

    /*Uninit PerfService */
#ifdef USING_MTK_PERFSERVICE
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) && (misPerfService == MTRUE))
    {
        disablePerfService(gScenTable[0].scen);
        uninitPerf();
        misPerfService = MFALSE;
        LOG_DBG("Uninit PerfService");
    }
#endif

    /* Uninit graph */
    mpCamGraph->stop();
    mpCamGraph->uninit();

    /* Disconnect nodes */
    mpCamGraph->disconnect();

    /* Destroy nodes */
    mpP2aNode->destroyInstance();
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
            FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        //mpControlNode->destroyInstance();
        mpGpuNode->destroyInstance();
        mpMdpNode->destroyInstance();
    }
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        mpVfbSwNode->destroyInstance();
        mpP2bNode->destroyInstance();
        mpSwitcherNode->destroyInstance();
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpEisSwNode->destroyInstance();
    }
    mpAllocBufHdl->uninit();
    mpAllocBufHdl->destroyInstance();

    CPTLog(Event_FeaturePipe_uninit, CPTFlagEnd);
    CPTLog(Event_FeaturePipe, CPTFlagEnd);

    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipe::startVideoRecord(MINT32 wd,MINT32 ht,MINT32 fps)
{
    FUNCTION_LOG_START;
    MBOOL ret = reinterpret_cast<FeaturePipeP2aNode*>(mpP2aNode)->startVideoRecord(wd, ht,fps);
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipe::stopVideoRecord()
{
    FUNCTION_LOG_START;
    MBOOL ret = reinterpret_cast<FeaturePipeP2aNode*>(mpP2aNode)->stopVideoRecord();
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipe::enqueFeatureGraph(QParams const& rParams)
{
    //FUNCTION_LOG_START;
    CAM_TRACE_FMT_BEGIN("FPipeEnq:%d", rParams.mFrameNo);
    Mutex::Autolock autoLock(mReconfigMtx); //Lock to make sure no more is enqued into FeatureGraph
    bool ret = true;
    ::android_atomic_inc(&mEnqueCount);

#ifdef FORCE_P2A_MKDBG
    sgmkDebugFlag[0] = '1';
#endif
    if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fp-enqueFG_001: frm: %d", getMagicNo(rParams)); }

    LOG_DBG("FrameNo(%d)", getMagicNo(rParams));
    ret = reinterpret_cast<FeaturePipeP2aNode*>(mpP2aNode)->enqueWrapper(0, (MUINTPTR) &rParams, getMagicNo(rParams));

    if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fp-enqueFG_002: frm: %d", getMagicNo(rParams)); }

    CAM_TRACE_FMT_END();
    //FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipe::dequeFeatureGraph(QParams& rParams, MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    bool ret = true;

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        ret = reinterpret_cast<FeaturePipeP2bNode*>(mpP2bNode)->dequeWrapper(rParams, i8TimeoutNs);
    }
    else if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        ret = reinterpret_cast<FeaturePipeMdpNode*>(mpMdpNode)->dequeWrapper(rParams, i8TimeoutNs);
    }
    else
    {
        ret = reinterpret_cast<FeaturePipeP2aNode*>(mpP2aNode)->dequeWrapper(rParams, i8TimeoutNs);
    }

    LOG_DBG("FrameNo(%d)", getMagicNo(rParams));
    ::android_atomic_dec(&mEnqueCount);

    FUNCTION_LOG_END;
    return ret;
}

void
FeaturePipe::performCallback(QParams& rParams)
{
    //FUNCTION_LOG_START;
    CAM_TRACE_FMT_BEGIN("FPipeCb:%d", rParams.mFrameNo);
#ifdef USING_MTK_PERFSERVICE
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) && (misPerfService == MTRUE))
    {
        disablePerfService(gScenTable[0].scen);
        uninitPerf();
        misPerfService = MFALSE;
        LOG_DBG("Uninit PerfService");
    }
#endif

#ifdef FORCE_P2A_MKDBG
    sgmkDebugFlag[0] = '1';
#endif
    if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fp-pcb_001: frm: %d", getMagicNo(rParams)); }

    LOG_DBG("FeatureGraph callback FrameNo(%d)", getMagicNo(rParams));
    ::android_atomic_dec(&mEnqueCount);
    rParams.mpfnCallback(rParams);
    CAM_TRACE_FMT_END();
    //FUNCTION_LOG_END;
}

MBOOL
FeaturePipe::setFeature(MUINT32 featureMask)
{
    bool ret = MTRUE;
    if (featureMask == mFeatureOnMask)
    {
        return ret;
    }

    FUNCTION_LOG_START;
    LOG_DBG("SetFeature: %x->%x", mFeatureOnMask, featureMask);

    MUINT32 featureDiff = featureMask ^ mFeatureOnMask;
    if (FEATURE_MASK_IS_3DNR_ENABLED(featureDiff) ||
        FEATURE_MASK_IS_VFB_ENABLED(featureDiff) ||
        FEATURE_MASK_IS_EIS_ENABLED(featureDiff) ||
        FEATURE_MASK_IS_VHDR_ENABLED(featureDiff) ||
        FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(featureDiff))
    {
        /* Change feature(s) that affect the node graph */
        Mutex::Autolock autoLock(mReconfigMtx); //Lock to make sure no more is enqued into FeatureGraph
        /* Wait till enqued are all dequed */
        while (mEnqueCount)
        {
            usleep(50000);
        }

        uninitFeatureGraph();

        mFeatureOnMask = featureMask;
        ret = initFeatureGraph();
    }
    else if(FEATURE_MASK_IS_GESTURE_SHOT_ENABLED(featureDiff) || FEATURE_MASK_IS_VHDR_ENABLED(featureDiff))
    {
        /* Change feature(s) that do not affect the node graph */
        mFeatureOnMask = featureMask;
        if (mpP2aNode != NULL)
        {
            ret = reinterpret_cast<FeaturePipeP2aNode*>(mpP2aNode)->notifyGraphNodes(FEATURE_MASK_CHANGE, mFeatureOnMask, 0);
        }
    }

    LOG_AST(ret, "Set features (0x%x) failed!", featureMask);

    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipe::setTuning(FeaturePipeTuningInfo& tuningInfo)
{
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
        reinterpret_cast<FeaturePipeVfbNode*>(mpVfbSwNode)->setTuning(tuningInfo);
        reinterpret_cast<FeaturePipeFdNode*>(mpFdNode)->setRotation(tuningInfo.Rotation);
        reinterpret_cast<FeaturePipeSwitcherNode*>(mpSwitcherNode)->setVfbExtreme(tuningInfo.ExtremeBeauty);
#endif
    }
    return MTRUE;
}

MBOOL
FeaturePipe::setTouch(MINT32 touchX, MINT32 touchY)
{
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
        return reinterpret_cast<FeaturePipeVfbNode*>(mpVfbSwNode)->setTouch(touchX, touchY);
#endif
    }
    return MTRUE;
}

MBOOL
FeaturePipe::set(FeaturePipeSetType setType, const MINTPTR setData)
{
    //LOG_DBG("setType(%d), setData(0x%08x)", setType, (MUINT32) setData);
    switch (setType)
    {
        case MAX_INPUT_IMAGE_SIZE:
        case RECORDING_HINT:
            mRecordingHint = (MBOOL) setData;
            break;
    }
    return MTRUE;
}

/******************************************************************************
 *
 ******************************************************************************/
FeaturePipeNode*
FeaturePipeNode::createInstance(FeaturePipeNodeObject_e eobject, MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe *pFeaturePipeObj)
{
    if (eobject == FEATURE_STREAM_NODE_CONTROL) {
         return  NULL;//TODO:FeaturePipeControlNode::createInstance(featureOnMask);
    }
    else if (eobject == FEATURE_STREAM_NODE_P2A) {
         return  FeaturePipeP2aNode::createInstance(featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
    }
    else if (eobject == FEATURE_STREAM_NODE_P2B) {
         return  FeaturePipeP2bNode::createInstance(featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
    }
    else if (eobject == FEATURE_STREAM_NODE_EISSW) {
         return  FeaturePipeEisNode::createInstance(featureOnMask,mOpenedSensorIndex);
    }
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
    else if (eobject == FEATURE_STREAM_NODE_VFBSW) {
         return  FeaturePipeVfbNode::createInstance(featureOnMask, pFeaturePipeObj);
    }
#endif
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
    else if (eobject == FEATURE_STREAM_NODE_FD) {
         return  FeaturePipeFdNode::createInstance(featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
    }
#endif
    else if (eobject == FEATURE_STREAM_NODE_GPU) {
         return  FeaturePipeGpuNode::createInstance(featureOnMask, pFeaturePipeObj);
    }
    else if (eobject == FEATURE_STREAM_NODE_MDP) {
         return  FeaturePipeMdpNode::createInstance(featureOnMask, pFeaturePipeObj);
    }
    else if (eobject == FEATURE_STREAM_NODE_SWITCHER) {
         return  FeaturePipeSwitcherNode::createInstance(featureOnMask, pFeaturePipeObj);
    }
    else
        return  0;
}

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[P2A:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[P2A:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[P2A:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[P2A:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[P2A:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[P2A:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeP2aNode*
FeaturePipeP2aNode::createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe *pFeaturePipeObj)
{
    return new FeaturePipeP2aNode("FeaturePipeP2aNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
}
MVOID
FeaturePipeP2aNode::destroyInstance()
{
    delete this;
}
FeaturePipeP2aNode::FeaturePipeP2aNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        MUINT32 mOpenedSensorIndex,
        FeaturePipe *pFeaturePipeObj
        )
        : FeaturePipeNode(name, type, policy, priority, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj),
          mpIFeatureStream(NULL),
          mpINormalStream(NULL)
{
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_DST, P2A_TO_SWITCHER);
        addDataSupport(ENDPOINT_DST, P2A_TO_VFBSW_DSIMG);
        addDataSupport(ENDPOINT_DST, P2A_TO_P2B);
        addDataSupport(ENDPOINT_DST, P2A_TO_FD_DSIMG);

        addNotifySupport(FEATURE_MASK_CHANGE);
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_DST, P2A_TO_EISSW);
        if (!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            addDataSupport(ENDPOINT_DST, P2A_TO_GPU);
            addDataSupport(ENDPOINT_DST, P2A_TO_MDP_QPARAMS);
        }
    }

    mpVHdrObj = NULL;
}

MBOOL
FeaturePipeP2aNode::onInit()
{
    FUNCTION_LOG_START;
    MRESULT mret;
    MUINT8 imemBufInfoIdx = 0;


    LOG_DBG("mFeatureOnMask 0x%08x", mFeatureOnMask);

    mpNr3dParam = new NSCam::NSIoPipe::NSPostProc::NR3D;
    mpFeoParam  = new NSCam::NSIoPipe::NSPostProc::StaData;

    if(FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
    {
        mpLceiParam = new NSCam::NSIoPipe::NSPostProc::StaData;
    }

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;

    if(
        FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)  &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
    )   // For 3DNR only case.
    {
        /* Create IFeatureStream */
        mpIFeatureStream = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("FeaturePipe_FeatureP2", EFeatureStreamTag_Stream, mOpenedSensorIndex);
        if (mpIFeatureStream == NULL)
        {
            LOG_ERR("IFeatureStream create instance for FeaturePipe_FeatureP2 failed!");
            goto _Exit;
        }
        mpIFeatureStream->init();
    }
    else if(FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask))
        // For Stereo Camera case
    {
        /* Create IFeatureStream */
        IHalSensorList* const pHalSensorList = IHalSensorList::get();
        EFeatureStreamTag eTag = ( SENSOR_DEV_MAIN_2 == pHalSensorList->querySensorDevIdx(mOpenedSensorIndex) )
                                ? EFeatureStreamTag_N3D_Stream_M2 : EFeatureStreamTag_vFB_Stream;

        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("debug.stereovfb.enable", EnableOption, "0");
        if(EnableOption[0] == '1')
        {
            eTag = ( SENSOR_DEV_SUB == pHalSensorList->querySensorDevIdx(mOpenedSensorIndex) )
                                ? EFeatureStreamTag_N3D_Stream_M2 : eTag;
        }

        if (eTag == EFeatureStreamTag_N3D_Stream_M2)
            LOG_DBG("Creating IFeatureStream stereo sub-sensor instance");
        mpIFeatureStream = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("FeaturePipe_FeatureP2", eTag, mOpenedSensorIndex);
        if (mpIFeatureStream == NULL)
        {
            LOG_ERR("IFeatureStream create instance for FeaturePipe_FeatureP2 failed!");
            goto _Exit;
        }
        mpIFeatureStream->init();
    }
    else if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
            FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
    )
    {
        /* Create IFeatureStream */
        mpIFeatureStream = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("FeaturePipe_VfbP2A", EFeatureStreamTag_vFB_Stream, mOpenedSensorIndex);
        if (mpIFeatureStream == NULL)
        {
            LOG_ERR("IFeatureStream create instance failed!");
            goto _Exit;
        }
        mpIFeatureStream->init();
    }
    else
    {
        /* Create INormalStream */
        mpINormalStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance("FeaturePipe_NormalP2", ENormalStreamTag_Prv, mOpenedSensorIndex);
        mpINormalStream->init();
    }

    mpEisHalFor3DNR = EisHal::CreateInstance("FeaturePipe_3DNR", mOpenedSensorIndex);
    if (mpEisHalFor3DNR == NULL)
    {
        LOG_ERR("mpEisHalFor3DNR CreateInstance failed");
        goto _Exit;
    }

    // Can't unmark, will cause hang. Because under current code architecture,
    // when pass 2 thread is up and this line is executed, Pass 1 thread might
    // not be ready yet, which will cause hang because not correct TG/sensor
    // information for EIS.
    //if(EIS_RETURN_NO_ERROR != mpEisHalFor3DNR->Init())
    //{
    //    LOG_ERR("mpEisHalFor3DNR init failed");
    //    goto _Exit;
    //}

    //if(FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))  // Workaround for EIS GMV Queue can't unregister user => Pass 2 always register user (in FeaturePipeP2aNode::onInit()), Pass 1 EIS always set GMV, Pass 2 always get GMV.
    {

        // Can't unmark, will cause hang. Because under current code architecture,
        // when pass 2 thread is up and this line is executed, Pass 1 thread might
        // not be ready yet, which will cause hang because not correct TG/sensor
        // information for EIS.
        //if (mpEisHalFor3DNR->GetEisSupportInfo(mOpenedSensorIndex))    // This sensor support EIS.
        {
            mpEisHalFor3DNR->SetEisP2User("FeaturePipe_3DNR");
        }

    }

#ifdef MTK_CAM_VHDR_SUPPORT

    if(FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
    {
        mpVHdrObj = VHdrHal::CreateInstance("FPipeVHdr",mOpenedSensorIndex);

        if(mpVHdrObj != NULL)
        {
            mpVHdrObj->Init(SENSOR_VHDR_MODE_IVHDR);
        }
    }

#endif

    mp3AHal = NS3A::IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mOpenedSensorIndex, "FeaturePipe_FeatureP2");
    if (!mp3AHal)
    {
        LOG_ERR("IHal3A::createInstance failed!");
        goto _Exit;
    }

    sem_init(&mModuleSem, 0, 0);

    /* Allocate buffer for next node */

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
        FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        NodeDataTypes dType = FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)? P2A_TO_SWITCHER: P2A_TO_GPU;
        pBufHandler = getBufferHandler(dType);
        if (pBufHandler == NULL)
        {
            LOG_ERR("ICamBufHandler getBufferHandler(P2A_TO_GPU) failed!");
            goto _Exit;
        }

        LOG_DBG("Allocate P2A buffer %dx%d", NR3D_WORKING_BUFF_WIDTH, NR3D_WORKING_BUFF_HEIGHT);
        AllocInfo allocinfo(NR3D_WORKING_BUFF_WIDTH,
                            NR3D_WORKING_BUFF_HEIGHT,
                            eImgFmt_YV12,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);

        for (MUINT8 i = 0; i < P2A_FULLIMG_BUFFER_NUM; i++)
        {
            mFullImgGB[i] = new GraphicBuffer(NR3D_WORKING_BUFF_WIDTH, NR3D_WORKING_BUFF_HEIGHT, HAL_PIXEL_FORMAT_YV12, GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN);
            if (mFullImgGB[i] == NULL)
            {
                LOG_ERR("P2A GraphicBuffer allocation for YV12 failed!");
                goto _Exit;
            }

            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->requestBuffer_GB(dType, allocinfo, &(mFullImgGB[i])) )
            {
                LOG_ERR("Request P2A_TO_GPU full image buffer failed");
                goto _Exit;
            }
        }
    }
    else if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)) //3DNR only
    {
        pBufHandler = getBufferHandler(P2A_TO_GPU);
        if (pBufHandler == NULL)
        {
            LOG_ERR("ICamBufHandler getBufferHandler(P2A_TO_GPU) failed!");
            goto _Exit;
        }

        LOG_DBG("Allocate P2A buffer %dx%d", NR3D_WORKING_BUFF_WIDTH, NR3D_WORKING_BUFF_HEIGHT);
        AllocInfo allocinfo(NR3D_WORKING_BUFF_WIDTH,
                            NR3D_WORKING_BUFF_HEIGHT,
                            eImgFmt_YV12,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);

        for (MUINT8 i = 0; i < P2A_FULLIMG_BUFFER_NUM; i++)
        {
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->requestBuffer(P2A_TO_GPU, allocinfo) )
            {
                LOG_ERR("Request P2A_TO_GPU full image buffer failed");
                goto _Exit;
            }
        }
    }

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Allocate DS image buffer %dx%d", VFB_DS_IMAGE_MAX_WIDTH, VFB_DS_IMAGE_MAX_HEIGHT);
        if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            AllocInfo allocinfo(VFB_DS_IMAGE_MAX_WIDTH,
                                VFB_DS_IMAGE_MAX_HEIGHT,
                                eImgFmt_YUY2,
                                eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
            for(MUINT32 i = 0; i < P2A_DSIMG_BUFFER_NUM; i++)
            {
                if( !pBufHandler->requestBuffer(P2A_TO_VFBSW_DSIMG, allocinfo) )
                    goto _Exit;
            }
        }
    }

    sem_init(&mCallbackSem, 0, 0);

    FUNCTION_LOG_END;
    return true;

_Exit:
    if (mpIFeatureStream != NULL)
    {
        mpIFeatureStream->destroyInstance("FeaturePipe_VfbP2A");
    }
    if (mpINormalStream != NULL)
    {
        mpINormalStream->destroyInstance("FeaturePipe_NormalP2");
    }

    return false;
}

MBOOL
FeaturePipeP2aNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;


    // Return (enque) all buffers before leaving.
    if(
       FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) &&  //Vent@20140228: For 3DNR only case.
       !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
    )
    {
        FeaturePipeNodeImgReqJob_s jobFullImg;

        // enqueBuffer.
        while (!m3dnrPrvFrmQueue.empty())
        {
            jobFullImg = m3dnrPrvFrmQueue.front();
            pBufHandler->enqueBuffer(P2A_TO_GPU, jobFullImg.imgReq.mBuffer);
            LOG_DBG("m3dnrPrvFrmQueue[size %d]: mem<va(0x%x)/pa(0x%x)>", m3dnrPrvFrmQueue.size(), jobFullImg.imgReq.mBuffer->getBufVA(0), jobFullImg.imgReq.mBuffer->getBufPA(0));
            m3dnrPrvFrmQueue.pop();
        }
    }

    delete mpNr3dParam; // Return allocated memory.
    delete mpFeoParam;

    if(FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
    {
        delete mpLceiParam;
    }

    mPrevFrameWidth         = 0;
    mPrevFrameHeight        = 0;
    m3dnrGainZeroCount      = 0;
    m3dnrErrorStatus        = NR3D_ERROR_NONE;
    m3dnrStateMachine       = NR3D_STATE_PREPARING;
    mNmvX = 0;
    mNmvY = 0;
    mCmvX = 0;
    mCmvY = 0;
    mPrevCmvX = 0;
    mPrevCmvY = 0;


    if (mpEisHalFor3DNR != NULL)
    {
//        mpEisHalFor3DNR->Uninit();
        mpEisHalFor3DNR->DestroyInstance("FeaturePipe_3DNR");
        mpEisHalFor3DNR = NULL;
    }

#ifdef MTK_CAM_VHDR_SUPPORT

    if(mpVHdrObj != NULL)
    {
        mpVHdrObj->Uninit();
        mpVHdrObj->DestroyInstance("FPipeVHdr");
    }

#endif

    if (mp3AHal != NULL)
    {
        // Restore AE ISO limit. (Leaving feature pipe, so don't need 3DNR raise ISO limit.)
        // Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 100: it means don't need to increase. 100: it means don't need to increase.
        mp3AHal->modifyPlineTableLimitation(MTRUE, MTRUE, 100, 100);
        LOG_DBG("Restore 3DNR AE ISO limit.");
        mp3AHal->destroyInstance("FeaturePipe_FeatureP2");
        mp3AHal = NULL;
    }

    if (mpIFeatureStream != NULL)
    {
        mpIFeatureStream->uninit();
        mpIFeatureStream->destroyInstance("FeaturePipe_VfbP2A");
    }
    if (mpINormalStream != NULL)
    {
        mpINormalStream->uninit();
        mpINormalStream->destroyInstance("FeaturePipe_NormalP2");
        sem_destroy(&mModuleSem);
    }

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
        FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Freeing GraphicBuffer");
        for (MUINT8 i = 0; i < P2A_FULLIMG_BUFFER_NUM; i++)
        {
            mFullImgGB[i]->unlock();
            mFullImgGB[i] = NULL;
        }
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeP2aNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2aNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2aNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2aNode::
enqueWrapper(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#if 0
    return handlePostBuffer(data, buf, ext);
}
MBOOL
FeaturePipeP2aNode::
onPostBuffer(MUINT32 const data, MUINT32 const buf, MUINT32 const ext)
{
#endif
    //FUNCTION_LOG_START;
    //LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MBOOL ret = MTRUE;

#ifdef FORCE_P2A_MKDBG
    sgmkDebugFlag[0] = '1';
#endif
    if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-enwrap_001: frm: %d", ext); }

    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) )    //Vent@20140228: Add for 3DNR.
    {
        QParams* pQParams = reinterpret_cast<QParams*>(buf);
        if (pQParams->mpfnCallback != NULL)
        {
            Mutex::Autolock lock(mLock);

            /* Change to P2A callback func */
            mqCookie.push(pQParams->mpCookie);
            mqfnCallback.push(pQParams->mpfnCallback);
            pQParams->mpCookie = (void*) this;
            pQParams->mpfnCallback = p2aCbFunc;

            if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-enwrap_002: frm: %d: cb changed.", ext); }
        }
        else
        {
            if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-enwrap_003: frm: %d: cb NO changed.", ext); }
        }

        CAM_TRACE_FMT_BEGIN("P2ANStrm:%d", getMagicNo((*pQParams)));

#ifdef MTK_CAM_VHDR_SUPPORT

        if(FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
        {
            LCEI_DMA_INFO aLceiDma;
            ModuleInfo vhdrLceiModuleinfo;

            //> get LCEI info

            mpVHdrObj->GetLceiDmaInfo(&aLceiDma,pQParams->mvIn[0].mBuffer->getTimestamp());

            if(aLceiDma.memID == VHDR_LCSO_SYNC_FAIL)
            {
                LOG_ERR("[FPipe_N] VHDR_LCSO_SYNC_FAIL");
            }

            //> prepare info and push to QParam

            mpLceiParam->bufInfo.size        = aLceiDma.size;
            mpLceiParam->bufInfo.memID       = aLceiDma.memID;
            mpLceiParam->bufInfo.virtAddr    = aLceiDma.va;
            mpLceiParam->bufInfo.phyAddr     = aLceiDma.pa;
            mpLceiParam->bufInfo.bufSecu     = aLceiDma.bufSecu;
            mpLceiParam->bufInfo.bufCohe     = aLceiDma.bufCohe;
            mpLceiParam->bufInfo.useNoncache = aLceiDma.useNoncache;

            mpLceiParam->w          = aLceiDma.xSize;
            mpLceiParam->h          = aLceiDma.ySize;
            mpLceiParam->stride     = aLceiDma.stride;
            mpLceiParam->port_idx   = NSImageio::NSIspio::EPortIndex_LCEI;
            mpLceiParam->port_type  = NSIoPipe::EPortType_Memory;
            mpLceiParam->port_inout = 1;

            vhdrLceiModuleinfo.moduleTag    = EFeatureModule_STA_LCEI;
            vhdrLceiModuleinfo.moduleStruct = reinterpret_cast<MVOID*>(mpLceiParam);

            LOG_DBG("[FPipe_N] LCEI_PA(0x%08x)",mpLceiParam->bufInfo.phyAddr);

            pQParams->mvModuleData.push_back(vhdrLceiModuleinfo);
        }

#endif
        if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-enwrap_004: frm: %d", ext); }

        ret = mpINormalStream->enque(*pQParams);
        CAM_TRACE_FMT_END();
    }
    else
    {
        Mutex::Autolock lock(mLock);
        mqJob.push(*(reinterpret_cast<QParams*>(buf)));

        if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-enwrap_005: frm: %d", ext); }
        triggerLoop();
    }


    //FUNCTION_LOG_END;
    return ret;
}
MBOOL
FeaturePipeP2aNode::
notifyGraphNodes(MUINT32 const msg, MUINT32 const ext1, MUINT32 const ext2)
{
    LOG_DBG("msg(%d), ext1(0x%x), ext2(0x%x)", msg, ext1, ext2);
    handleNotify(msg, ext1, ext2);
    return MTRUE;
}
void
FeaturePipeP2aNode::
getGraphicBuffer(vector< sp<GraphicBuffer>* >& vpGB)
{
    for (int i=0; i<P2A_FULLIMG_BUFFER_NUM; i++)
        vpGB.push_back(&(mFullImgGB[i]));
}
MBOOL
FeaturePipeP2aNode::
set(FeaturePipeSetType setType, const MINTPTR setData)
{
    switch (setType)
    {
        case MAX_INPUT_IMAGE_SIZE:
            mMaxInputImageSize = *((MSize*) setData);
            break;
    }
    return MTRUE;
}
MBOOL
FeaturePipeP2aNode::startVideoRecord(MINT32 wd,MINT32 ht,MINT32 fps)
{
    FUNCTION_LOG_START;
    MBOOL ret = MFALSE;
    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) )
    {
        ret = mpINormalStream->startVideoRecord(wd, ht, fps);
    }
    else
    {
        ret = mpIFeatureStream->startVideoRecord(wd, ht, fps);
    }
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipeP2aNode::stopVideoRecord()
{
    FUNCTION_LOG_START;
    MBOOL ret = MFALSE;
    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) )
    {
        ret = mpINormalStream->stopVideoRecord();
    }
    else
    {
        ret = mpIFeatureStream->stopVideoRecord();
    }
    FUNCTION_LOG_END;
    return ret;
}
MBOOL
FeaturePipeP2aNode::
dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    MRESULT mret = MTRUE;

    if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask) &&
       !FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) )    //Vent@20140228: Add for 3DNR.
    {
        mret = mpINormalStream->deque(rParams, i8TimeoutNs);
        if (!mret)
        {
            LOG_ERR("INormalStream deque failed! (frame %d)", getMagicNo(rParams));
        }
        else
        {
            LOG_DBG("INormalStream deque succeeds! (frame %d)", getMagicNo(rParams));
        }
    }
    else
    {
        sem_wait(&mModuleSem);

        Mutex::Autolock lock(mLock);
        rParams = qCallerQParams.front();
        qCallerQParams.pop();
    }

    FUNCTION_LOG_END;
    return mret;
}
MVOID
FeaturePipeP2aNode::
p2aCbFunc(QParams& rParams)
{
#ifdef FORCE_P2A_MKDBG
		sgmkDebugFlag[0] = '1';
#endif
	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-p2aCbF_001: frm: %d", getMagicNo(rParams)); }

    //FUNCTION_LOG_START;
    CAM_TRACE_FMT_BEGIN("P2ACb:%d", getMagicNo(rParams));

    CPTLog(Event_FeaturePipe_P2A_FeatureStream, CPTFlagEnd);
    FeaturePipeP2aNode* pP2ANodeObj = (FeaturePipeP2aNode*)(rParams.mpCookie);
    if(!FEATURE_MASK_IS_VFB_ENABLED(pP2ANodeObj->mFeatureOnMask) &&
       !FEATURE_MASK_IS_EIS_ENABLED(pP2ANodeObj->mFeatureOnMask) &&
       !FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(pP2ANodeObj->mFeatureOnMask) &&
       !FEATURE_MASK_IS_3DNR_ENABLED(pP2ANodeObj->mFeatureOnMask))
    {
        Mutex::Autolock lock(pP2ANodeObj->mLock);

        /* Normal Stream: callback by enqued QParams */
        rParams.mpCookie = pP2ANodeObj->mqCookie.front();
        pP2ANodeObj->mqCookie.pop();
        rParams.mpfnCallback = pP2ANodeObj->mqfnCallback.front();
        pP2ANodeObj->mqfnCallback.pop();

        pP2ANodeObj->mpFeaturePipeObj->performCallback(rParams);
    }
    else if (
        (FEATURE_MASK_IS_3DNR_ENABLED(pP2ANodeObj->mFeatureOnMask)   &&
         !FEATURE_MASK_IS_VFB_ENABLED(pP2ANodeObj->mFeatureOnMask)    &&
         !FEATURE_MASK_IS_EIS_ENABLED(pP2ANodeObj->mFeatureOnMask) ) ||
        (FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(pP2ANodeObj->mFeatureOnMask)   &&
         !FEATURE_MASK_IS_VFB_ENABLED(pP2ANodeObj->mFeatureOnMask)    &&
         !FEATURE_MASK_IS_EIS_ENABLED(pP2ANodeObj->mFeatureOnMask) )
    )
    {
        Mutex::Autolock lock(pP2ANodeObj->mLock);

        /* P2A-only Feature Stream: callback by enqued QParams */
        MBOOL dequeSuccess = rParams.mDequeSuccess;
        rParams = pP2ANodeObj->mqCallbackQParams.front();
        pP2ANodeObj->mqCallbackQParams.pop();
        rParams.mDequeSuccess = dequeSuccess;
        pP2ANodeObj->handleP2Done(rParams);

        pP2ANodeObj->mpFeaturePipeObj->performCallback(rParams);
    }
    else
    {
        LOG_DBG("Callback of frame %d", getMagicNo(rParams));
        /* Feature Stream: callback from P2B or GPU node */
        LOG_AST(rParams.mDequeSuccess, "P2A deque failed! (frame %d)", getMagicNo(rParams));

        if(FEATURE_MASK_IS_EIS_ENABLED(pP2ANodeObj->mFeatureOnMask))
        {
            reinterpret_cast<FeaturePipeEisNode*>(pP2ANodeObj->mpFeaturePipeObj->mpEisSwNode)->FlushMemory(EIS_DMA_FEO,EIS_FLUSH_SW);
        }

        pP2ANodeObj->handleP2Done(rParams);
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-p2aCbF_002: frm: %d", getMagicNo(rParams)); }


    CAM_TRACE_FMT_END();
    //FUNCTION_LOG_END;
}
MBOOL
FeaturePipeP2aNode::
handleP2Done(QParams& rParams)
{

#ifdef FORCE_P2A_MKDBG
		sgmkDebugFlag[0] = '1';
#endif
	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-hP2Done_001: frm: %d", getMagicNo(rParams)); }

    IImageBuffer* pFullImgBuffer = NULL;
    IImageBuffer* pDsImgBuffer = NULL;
    for(MUINT32 i = 0; i < rParams.mvOut.size(); i++)
    {
        if (rParams.mvOut[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMG3O)
        {
            pFullImgBuffer = rParams.mvOut[i].mBuffer;
        }
        else if (rParams.mvOut[i].mPortID.index == NSImageio::NSIspio::EPortIndex_WDMAO)
        {
            pDsImgBuffer = rParams.mvOut[i].mBuffer;
        }
    }

    // Flush for CPU read later
    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        pDsImgBuffer->syncCache(eCACHECTRL_INVALID);
    }

#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpp2a", EnableOption, "0");
    if(EnableOption[0] == '2' || EnableOption[0] == '3')
#endif
    {
        char szFileName[100];
        if(pFullImgBuffer &&
           (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)||
           FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)))
        {
            ::sprintf(szFileName, "/sdcard/p2ao_%dx%d_%04d.yuv", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT, getMagicNo(rParams));
            saveBufToFile(szFileName, (MUINT8*) pFullImgBuffer->getBufVA(0), FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 3 / 2);
        }
        if(pDsImgBuffer && FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            ::sprintf(szFileName, "/sdcard/p2ao_%dx%d_%04d.yuv", pDsImgBuffer->getImgSize().w, pDsImgBuffer->getImgSize().h, getMagicNo(rParams));
            pDsImgBuffer->saveToFile(szFileName);
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-hP2Done_002: frm: %d", getMagicNo(rParams)); }

    ImgRequest fullImgReq(pFullImgBuffer);
    ImgRequest dsImgReq(pDsImgBuffer);

    /* Post buffer to next node */
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Posting P2A->Switcher node full image buffer of frame %d", getMagicNo(rParams));
        handlePostBuffer(P2A_TO_SWITCHER, (MUINTPTR) &fullImgReq, getMagicNo(rParams));
        LOG_DBG("Posting P2A->VFB node DS image of frame %d", getMagicNo(rParams));
        handlePostBuffer(P2A_TO_VFBSW_DSIMG, (MUINTPTR) &dsImgReq, getMagicNo(rParams));
        LOG_DBG("Posting P2A->FD node DS image of frame %d", getMagicNo(rParams));
        handlePostBuffer(P2A_TO_FD_DSIMG, (MUINTPTR) &dsImgReq, getMagicNo(rParams));
    }

    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("Posting P2A->EIS node full image buffer of frame %d", getMagicNo(rParams));
        handlePostBuffer(P2A_TO_EISSW, (MUINTPTR) &fullImgReq, getMagicNo(rParams));
        if (!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            LOG_DBG("Posting P2A->GPU node full image buffer of frame %d", getMagicNo(rParams));
            handlePostBuffer(P2A_TO_GPU, (MUINTPTR) &fullImgReq, getMagicNo(rParams));
        }
    }

    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        /* 3DNR must wait for P2 H/W done so that its output can be input of next frame */
        sem_post(&mCallbackSem);
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_fpp2a-hP2Done_003: frm: %d", getMagicNo(rParams)); }

    return MTRUE;
}
MBOOL
FeaturePipeP2aNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    LOG_AST(false, "Never call P2A onPostBuffer!", 0);
    return false;
}


MBOOL
FeaturePipeP2aNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
    //FUNCTION_LOG_START;
    MRESULT mret = MFALSE;

    CPTLog(Event_FeaturePipe_P2AThread, CPTFlagStart);

    char aInputValue[PROPERTY_VALUE_MAX] = {'\0'};
    MINT32  i4TempInputValue = 0;
    MUINT32 u4Img3oOffset_X  = 0;
    MUINT32 u4Img3oOffset_Y  = 0;
    MUINT32 eisFeBlock      = 0;
    MUINT32 u4Img3oWidth    = 0;
    MUINT32 u4Img3oHeight   = 0;
    MUINT32 u4VipiWidth     = 0;
    MUINT32 u4VipiHeight    = 0;
    MUINT32 u4VipiStartAddrOffset = 0;
    MSize imgSize, TempImgSize;


    /************************************************************************/
    /*                          Preprocessing                               */
    /************************************************************************/
    //////////////////////////////////////////////////////////////////////////
    // Process special request (e.g. adb command)                           //
    //////////////////////////////////////////////////////////////////////////
    char p2aDebugFlag[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("debug.fpipe.p2a", p2aDebugFlag, "0");

    //////////////////////////////////////////////////////////////////////////
    // Get settings from MW                                                 //
    //////////////////////////////////////////////////////////////////////////
    QParams InQParams;
    {
        Mutex::Autolock lock(mLock);
        InQParams = mqJob.front();
        mqJob.pop();
    }

#ifdef FORCE_P2A_MKDBG
		sgmkDebugFlag[0] = '1';
#endif
	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_001: frm: %d", getMagicNo(InQParams)); }

    CAM_TRACE_FMT_BEGIN("P2AThread:%d", getMagicNo(InQParams));

    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        //////////////////////////////////////////////////////////////////////////
        // 3DNR State Machine operation                                         //
        //////////////////////////////////////////////////////////////////////////
        if (m3dnrStateMachine == NR3D_STATE_PREPARING) // Last frame is NR3D_STATE_PREPARING.
        {
            m3dnrStateMachine = NR3D_STATE_WORKING;   // NR3D, IMG3O, VIPI all enabled.
        }
        // Reset m3dnrErrorStatus.
        m3dnrErrorStatus = NR3D_ERROR_NONE;

        //////////////////////////////////////////////////////////////////////////
        //  3DNR HW module on/off according Nr3dGain                            //
        //////////////////////////////////////////////////////////////////////////
        MUINT32 u43dnrHwPowerOffThreshold   = get_3dnr_hw_power_off_threshold();
        MUINT32 u43dnrHwPowerReopenDelay    = get_3dnr_hw_power_reopen_delay();
        MINT32 i4Nr3dGain = 0;
        mpIFeatureStream->sendCommand(EPIPECmd_GET_NR3D_GAIN, getMagicNo(InQParams), (MINTPTR)&i4Nr3dGain);
        if (i4Nr3dGain)
        {
            if (m3dnrGainZeroCount > 0)       // limit at 0.
                m3dnrGainZeroCount--;
        }
        else    // i4Nr3dGain0 is 0.
        {
            m3dnrGainZeroCount++;
            if (m3dnrGainZeroCount > u43dnrHwPowerOffThreshold)       // Cap at u43dnrHwPowerOffThreshold.
                m3dnrGainZeroCount = u43dnrHwPowerOffThreshold;
        }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_002: frm: %d", getMagicNo(InQParams)); }

        #if NR3D_NO_HW_POWER_OFF
        m3dnrGainZeroCount = 0; // For m3dnrGainZeroCount to be 0, so it won't change m3dnrStateMachine.
        #endif  // NR3D_NO_HW_POWER_OFF

        if (m3dnrGainZeroCount >= u43dnrHwPowerOffThreshold)
        {
            m3dnrStateMachine = NR3D_STATE_STOP;
        }
        else if (m3dnrGainZeroCount > (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay))
        {
            // StateMachine stays the same.
        }
        else    // (u43dnrHwPowerOffThreshold - u43dnrHwPowerReopenDelay) > m3dnrGainZeroCount > 0
        {
            if (m3dnrStateMachine == NR3D_STATE_STOP)
            {
                m3dnrStateMachine = NR3D_STATE_PREPARING;
                m3dnrGainZeroCount = 0; // Reset m3dnrGainZeroCount.
            }
        }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_003: frm: %d", getMagicNo(InQParams)); }

        // LOG_DBG("i4Nr3dGain(%d), 3dnrGnZoCnt(%d), StateMachine(%d S0P1W2)", i4Nr3dGain, m3dnrGainZeroCount, m3dnrStateMachine);
    }

    //////////////////////////////////////////////////////////////////////////
    //  3DNR GMV Calculation                                                //
    //////////////////////////////////////////////////////////////////////////
    // Get GMV from EIS HAL.
//    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)) // Workaround for EIS GMV Queue can't unregister user => Pass 2 always register user (in FeaturePipeP2aNode::onInit()), Pass 1 EIS always set GMV, Pass 2 always get GMV.
    {
        MINT32 i4TempNmvXFromQueue = 0, i4TempNmvYFromQueue = 0;
        MINT32 i4TempCmvXFromQueue = 0, i4TempCmvYFromQueue = 0;
        MINT32 i4TempX = 0, i4TempY = 0;
        EIS_P1_RESULT_INFO rEisResult;

        MBOOL fIsSupportEis = mpEisHalFor3DNR->GetEisSupportInfo(mOpenedSensorIndex);
        if (fIsSupportEis)    // This sensor support EIS.
        {
            mpEisHalFor3DNR->GetEisInfoAtP2("FeaturePipe_3DNR", &rEisResult, InQParams.mvIn[0].mBuffer->getTimestamp(), MTRUE);
            //LOG_DBG("3DNR TS[%3d]: DoEisCount(%d), %s, EisTS(%lld), ImgiTS(%lld)", getMagicNo(InQParams), rEisResult.DoEisCount, ((rEisResult.timeStamp == InQParams.mvIn[0].mBuffer->getTimestamp()) ? ("==") : ("!=")), rEisResult.timeStamp, ((InQParams.mvIn.size()) ? (InQParams.mvIn[0].mBuffer->getTimestamp()) : (0)) );   // Check whether InQParams.mvIn.size() is not 0, if not 0, print IMGI TS, else print 0.

            if (rEisResult.timeStamp != InQParams.mvIn[0].mBuffer->getTimestamp())   // GMV is invalid, set GMV to 0.
            {
                m3dnrErrorStatus |= NR3D_ERROR_INVALID_GMV;
                if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
                i4TempNmvXFromQueue = 0;
                i4TempNmvYFromQueue = 0;
                i4TempCmvXFromQueue = 0;
                i4TempCmvYFromQueue = 0;
            }
            else
            {
                //i4TempNmvXFromQueue = (rEisResult.gmvX / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_w must be even, so make mNmvX even. Discussed with James Liang.
                //i4TempNmvYFromQueue = (rEisResult.gmvY / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_h must be even when image format is 420, so make mNmvX even. Discussed with TC & Christopher.
                i4TempNmvXFromQueue = (-(rEisResult.gmvX) / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_w must be even, so make mNmvX even. Discussed with James Liang.
                i4TempNmvYFromQueue = (-(rEisResult.gmvY) / 256);  // The unit of Gmv is 256x 'pixel', so /256 to change unit to 'pixel'.  >> 1 << 1: nr3d_on_h must be even when image format is 420, so make mNmvX even. Discussed with TC & Christopher.
                i4TempCmvXFromQueue = rEisResult.cmvX_Int;  // The unit of Cmv is 256x 'pixel', so /256 to change unit to 'pixel'.
                i4TempCmvYFromQueue = rEisResult.cmvY_Int;  // The unit of Cmv is 256x 'pixel', so /256 to change unit to 'pixel'.
            }
        }
        else    // This sensor not support EIS.
        {
            i4TempNmvXFromQueue = 0;
            i4TempNmvYFromQueue = 0;
            i4TempCmvXFromQueue = 0;
            i4TempCmvYFromQueue = 0;
        }
		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_004: frm: %d", getMagicNo(InQParams)); }

        property_get("camera.3dnr.forcegmv.enable", aInputValue, "0");
        if (aInputValue[0] == '1')
        {
            property_get("camera.3dnr.forcegmv.x", aInputValue, "0");
            i4TempNmvXFromQueue = atoi(aInputValue);

            property_get("camera.3dnr.forcegmv.y", aInputValue, "0");
            i4TempNmvYFromQueue = atoi(aInputValue);

            LOG_DBG("Force GMV X/Y (%d, %d)", i4TempNmvXFromQueue, i4TempNmvYFromQueue);
        }

        #if (NR3D_FORCE_GMV_ZERO)   // Force GMV to be 0.
        mNmvX = 0;
        mNmvY = 0;
        #else   // Normal flow.
        property_get("camera.3dnr.usecmv.enable", aInputValue, "1");
        if (aInputValue[0] == '1')   // For EIS 1.2 (use CMV). gmv_crp (t) = gmv(t) - ( cmv(t) - cmv(t-1) )
        {
            // Use GMV and CMV
            mCmvX = i4TempCmvXFromQueue;    // Curr frame CMV X. Make it even.
            mCmvY = i4TempCmvYFromQueue;    // Curr frame CMV Y. Make it even.
            mNmvX = (i4TempNmvXFromQueue - (mCmvX - mPrevCmvX)) & ~1;    // Make it even.
            mNmvY = (i4TempNmvYFromQueue - (mCmvY - mPrevCmvY)) & ~1;    // Make it even.
            i4TempX = mCmvX - mPrevCmvX;
            i4TempY = mCmvY - mPrevCmvY;

            mPrevCmvX = mCmvX;                   // Recore last frame CMV X.
            mPrevCmvY = mCmvY;                   // Recore last frame CMV Y.
        }
        else   // For EIS 2.0 (use GMV)
        {
            // Use GMV only.
            mNmvX = i4TempNmvXFromQueue & ~1;    // Make it even.
            mNmvY = i4TempNmvYFromQueue & ~1;    // Make it even.

            mCmvX = 0;
            mCmvY = 0;
            mPrevCmvX = 0;                   // Recore last frame CMV X.
            mPrevCmvY = 0;                   // Recore last frame CMV Y.
        }
        #endif  // NR3D_FORCE_GMV_ZERO

        if ((abs(mNmvX) >= 28) || (abs(mNmvY) >= 28))
        {
            m3dnrErrorStatus |= NR3D_ERROR_GMV_TOO_LARGE;
            if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
        }

        if(p2aDebugFlag[0] != '0') {LOG_DBG("mFeatureOnMask 0x%08x", mFeatureOnMask);}

#if 0   // Special request for James.
        LOG_DBG("gmv_ana,frm[%3d],NmvX/Y(%3d,%3d),ConfidX/y(%3d,%3d)",
            getMagicNo(InQParams),
            mNmvX, mNmvY,
            rEisResult.gmvConfidX, rEisResult.gmvConfidY
        );
#else   // Original

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_005: frm: %d", getMagicNo(InQParams)); }

        LOG_DBG("gmv_ana,frm[%3d],gmv(x,y)=[%5d,%5d],IsSupportEis(%d),SenorIdx(%d),NmvX/Y(%5d,%5d)(ori %5d,%5d),CmvX/Y(%5d,%5d)(diff %5d,%5d)(ori %5d,%5d),x/yInt(%5d,%5d),x/yFlt(%9d,%9d),DoEisCount(%d),ConfidX/y(%d,%d),EisTS(%lld),ImgiTS(%lld),GnZoCnt(%d)",
            getMagicNo(InQParams),
            rEisResult.gmvX, rEisResult.gmvY,
            fIsSupportEis,
            mOpenedSensorIndex,
            mNmvX, mNmvY,
            i4TempNmvXFromQueue, i4TempNmvYFromQueue,
            mCmvX, mCmvY,
            i4TempX, i4TempY,
            i4TempCmvXFromQueue, i4TempCmvYFromQueue,
            rEisResult.cmvX_Int, rEisResult.cmvY_Int,
            rEisResult.cmvX_Flt, rEisResult.cmvY_Flt,
            rEisResult.DoEisCount,
            rEisResult.gmvConfidX, rEisResult.gmvConfidY,
            rEisResult.timeStamp,
            ((InQParams.mvIn.size()) ? (InQParams.mvIn[0].mBuffer->getTimestamp()) : (0)),   // Check whether InQParams.mvIn.size() is not 0, if not 0, print IMGI TS, else print 0.
            m3dnrGainZeroCount
        );
#endif

    }


    /************************************************************************/
    /*  Config Input DMA (mvIn) according to infomation passed down by MW   */
    /************************************************************************/

#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpp2a", EnableOption, "0");
    if (EnableOption[0] == '1' || EnableOption[0] == '3')
#endif
    {
        char szFileName[100];
        ::sprintf(szFileName, "/sdcard/p2ai_%dx%d_%04d.yuv", InQParams.mvIn[0].mBuffer->getImgSize().w, InQParams.mvIn[0].mBuffer->getImgSize().h, getMagicNo(InQParams));
        LOG_DBG("DUMP p2ai(%s)",szFileName);
        InQParams.mvIn[0].mBuffer->saveToFile(szFileName);
    }

    MUINT32 eisCrop2Idx = 0;
    for (int i=0; i < InQParams.mvCropRsInfo.size(); i++)
    {
        if(p2aDebugFlag[0] != '0')
        {
            LOG_DBG("[Given mCropRsInfo] groupid(%d),crop w(%d),h(%d),p_int(%d,%d),p_fra(%d,%d), resize w(%d),h(%d)",
                    InQParams.mvCropRsInfo[i].mGroupID,InQParams.mvCropRsInfo[i].mCropRect.s.w,InQParams.mvCropRsInfo[i].mCropRect.s.h,InQParams.mvCropRsInfo[i].mCropRect.p_integral.x,InQParams.mvCropRsInfo[i].mCropRect.p_integral.y,
                    InQParams.mvCropRsInfo[i].mCropRect.p_fractional.x,InQParams.mvCropRsInfo[i].mCropRect.p_fractional.y,InQParams.mvCropRsInfo[i].mResizeDst.w,InQParams.mvCropRsInfo[i].mResizeDst.h);
        }

        if (InQParams.mvCropRsInfo[i].mGroupID == 2)
        {
            eisCrop2Idx = i;
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_006: frm: %d", getMagicNo(InQParams)); }

    EIS_HAL_CONFIG_DATA eisConfigData;

    // Config mQParams.
    QParams mQParams;
    mQParams.mvIn.clear();
    mQParams.mvIn.reserve(InQParams.mvIn.size());
    mQParams.mvOut.clear();
    mQParams.mFrameNo = getMagicNo(InQParams);
    if (InQParams.mvMagicNo.size())
        mQParams.mvMagicNo.push_back(InQParams.mvMagicNo[0]);
    mQParams.mFrameNo = InQParams.mFrameNo;
    mQParams.mpPrivaData = InQParams.mpPrivaData;
    if (InQParams.mvPrivaData.size())
        mQParams.mvPrivaData.push_back(InQParams.mvPrivaData[0]);

    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        mQParams.mvOut.reserve(2);
    }
    else
    {
        mQParams.mvOut.reserve(1);
    }

    for (int i=0; i < InQParams.mvIn.size(); i++)
    {
        if(p2aDebugFlag[0] != '0')
        {
            LOG_DBG("[P2A sets mvIn (%d)] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d),magicnum(%d),stride(%d)",i,
                    InQParams.mvIn[i].mBuffer->getImgFormat(),InQParams.mvIn[i].mBuffer->getImgSize().w,InQParams.mvIn[i].mBuffer->getImgSize().h,
                    InQParams.mvIn[i].mPortID.type,InQParams.mvIn[i].mPortID.index,InQParams.mvIn[i].mPortID.inout,
                    InQParams.mvIn[i].mBuffer->getBufVA(0),InQParams.mvIn[i].mBuffer->getBufPA(0),
                    InQParams.mvIn[i].mBuffer->getBufSizeInBytes(0),InQParams.mvIn[i].mExtraParams.p2pxlID,getMagicNo(InQParams),
                    InQParams.mvIn[0].mBuffer->getBufStridesInBytes(0));
        }

        mQParams.mvIn.push_back(InQParams.mvIn[i]);
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_007: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*                        Config Output DMAs                            */
    /************************************************************************/
    //////////////////////////////////////////////////////////////////////////
    //  Calculate target width/height to set IMG3O                          //
    //////////////////////////////////////////////////////////////////////////
    // Set IMG3O info.
    ImgRequest fullImgReq;

    /* vFB and EIS have to use node buffer; otherwise, just use middleware buffer */
    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
       FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) ||
       FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)
    )
    {
        /* Calculate P2A output width and height */
        MUINT32 PostProcOutWidth = 0, PostProcOutHeight = 0;

        // Decide W/H of the buffer for IMG3O use.
        for (int i=0; i < InQParams.mvOut.size(); i++)
        {
            if(p2aDebugFlag[0] != '0')
            {
                LOG_DBG("[P2A gets mvOut (%d)] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d),magicnum(%d)",i,
                        InQParams.mvOut[i].mBuffer->getImgFormat(),InQParams.mvOut[i].mBuffer->getImgSize().w,InQParams.mvOut[i].mBuffer->getImgSize().h,
                        InQParams.mvOut[i].mPortID.type,InQParams.mvOut[i].mPortID.index,InQParams.mvOut[i].mPortID.inout,
                        InQParams.mvOut[i].mBuffer->getBufVA(0),InQParams.mvOut[i].mBuffer->getBufPA(0),
                        InQParams.mvOut[i].mBuffer->getBufSizeInBytes(0),InQParams.mvOut[i].mExtraParams.p2pxlID,getMagicNo(InQParams));
            }

            // Use largest size of pass 2 output.
            if (PostProcOutWidth < InQParams.mvOut[i].mBuffer->getImgSize().w || PostProcOutHeight < InQParams.mvOut[i].mBuffer->getImgSize().h)
            {
                PostProcOutWidth = InQParams.mvOut[i].mBuffer->getImgSize().w;
                PostProcOutHeight = InQParams.mvOut[i].mBuffer->getImgSize().h;
            }
        }

        // EIS has special rules for deciding W/H of the buffer for IMG3O use.
        NSCamHW::Rect eisCropRect;

        if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        {
            NSCamHW::Rect srcRect(InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.p_integral.x,
                                  InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.p_integral.y,
                                  InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.s.w,
                                  InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.s.h);

            NSCamHW::Rect dstRect(0, 0,InQParams.mvOut[0].mBuffer->getImgSize().w,InQParams.mvOut[0].mBuffer->getImgSize().h);  // only use display ratio

            eisCropRect = android::MtkCamUtils::calCrop(srcRect,dstRect,100);
            eisCropRect.x += srcRect.x;
            eisCropRect.y += srcRect.y;

            LOG_DBG("[EIS]Idx(%u),ROI(%u,%u,%u,%u),Dst(0,0,%u,%u),Crop(%u,%u,%u,%u)",eisCrop2Idx,
                                                                                     InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.p_integral.x,
                                                                                     InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.p_integral.y,
                                                                                     InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.s.w,
                                                                                     InQParams.mvCropRsInfo[eisCrop2Idx].mCropRect.s.h,
                                                                                     InQParams.mvOut[0].mBuffer->getImgSize().w,
                                                                                     InQParams.mvOut[0].mBuffer->getImgSize().h,
                                                                                     eisCropRect.x,
                                                                                     eisCropRect.y,
                                                                                     eisCropRect.w,
                                                                                     eisCropRect.h);

            // if ROI is bigger than EIS limitation
            if((eisCropRect.w*eisCropRect.h) > (FULL_IMAGE_MAX_WIDTH*FULL_IMAGE_MAX_HEIGHT) ||
                eisCropRect.w > FULL_IMAGE_MAX_WIDTH ||
                eisCropRect.h > FULL_IMAGE_MAX_HEIGHT)
            {
                NSCamHW::Rect tempEisDstRect(0, 0,FULL_IMAGE_MAX_WIDTH,FULL_IMAGE_MAX_HEIGHT);

                NSCamHW::Rect tempEisSrcRect  = android::MtkCamUtils::calCrop(srcRect,tempEisDstRect,100);
                NSCamHW::Rect tempEisCropRect = android::MtkCamUtils::calCrop(tempEisSrcRect,dstRect,100);

                tempEisSrcRect.x += srcRect.x;
                tempEisSrcRect.y += srcRect.y;

                LOG_DBG("[EIS]p2a out:(%u,%u,%u,%u)->(%u,%u,%u,%u)",eisCropRect.x,eisCropRect.y,eisCropRect.w,eisCropRect.h,
                                                                    tempEisCropRect.x,tempEisCropRect.y,tempEisCropRect.w,tempEisCropRect.h);
                eisCropRect.x = tempEisCropRect.x;
                eisCropRect.y = tempEisCropRect.y;
                eisCropRect.w = tempEisCropRect.w;
                eisCropRect.h = tempEisCropRect.h;
            }

            if(eisCropRect.w <= D1_WIDTH && eisCropRect.h <= D1_HEIGHT)
            {
                eisFeBlock = 8;
            }
            else if(eisCropRect.w <= EIS_FE_MAX_INPUT_W && eisCropRect.h <= EIS_FE_MAX_INPUT_H)
            {
                eisFeBlock = 16;
            }
            else
            {
                eisFeBlock = 32;
                LOG_WRN("FE should not use 32");
            }

            eisCropRect.w = ALIGN_SIZE(eisCropRect.w,eisFeBlock);
            eisCropRect.h = ALIGN_SIZE(eisCropRect.h,eisFeBlock);

            PostProcOutWidth  = eisCropRect.w;
            PostProcOutHeight = eisCropRect.h;

            eisConfigData.gpuTargetW=PostProcOutWidth/(reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->GetEisPlusCropRatio()/100.0);
            eisConfigData.gpuTargetH=PostProcOutHeight/(reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->GetEisPlusCropRatio()/100.0);

            eisConfigData.gpuTargetW = ROUND_TO_2X(eisConfigData.gpuTargetW);
            eisConfigData.gpuTargetH = ROUND_TO_2X(eisConfigData.gpuTargetH);

            // TODO-Need to modified for N3D/stereo
            eisConfigData.feTargetW = eisConfigData.gpuTargetW;
            eisConfigData.feTargetH = eisConfigData.gpuTargetH;

            LOG_DBG("[EIS]p2a final out(%u,%u),feBlock(%u)",PostProcOutWidth,PostProcOutHeight,eisFeBlock);
        }

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_008: frm: %d", getMagicNo(InQParams)); }

        // If 3DNR+vFB/EIS2.0, follow vFB/EIS2.0 size. If 3DNR only, overwrite PostProcOutWidth/PostProcOutHeight.
        if (
             FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) &&
            !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)  &&
            !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
        )   // When 3DNR only.
        {
            // Acquire IMG3O buffer size from MDP input crop size.
            MCrpRsInfo crop = InQParams.mvCropRsInfo[1]; // copy given group 2 (align mdp crop)
            PostProcOutWidth  = crop.mCropRect.s.w;
            PostProcOutHeight = crop.mCropRect.s.h;
        }

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_009: frm: %d", getMagicNo(InQParams)); }

        // W/H of buffer (i.e. Current frame size) is decided, so deque a buffer and resize it to calculated W/H.
        /* Prepare buffer for next node */
        NodeDataTypes dType = FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)? P2A_TO_SWITCHER: P2A_TO_GPU;
        if(!pBufHandler->dequeBuffer(dType, &fullImgReq))
        {
            LOG_AST(false, "dequeBuffer fail. Out of P2A_TO_GPU buffer", 0);
            return MFALSE;
        }
		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_010: frm: %d", getMagicNo(InQParams)); }

        // W/H of buffer (i.e. Current frame size) is determined, so check previous vs. current frame size for 3DNR.
        if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
        {
            if (
                (mPrevFrameWidth  != PostProcOutWidth) ||
                (mPrevFrameHeight != PostProcOutHeight)
            )
            {
                m3dnrErrorStatus |= NR3D_ERROR_FRAME_SIZE_CHANGED;
                if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
            }
        }

        if((fullImgReq.mBuffer->getImgSize().w != PostProcOutWidth) ||
           (fullImgReq.mBuffer->getImgSize().h != PostProcOutHeight))
        {
            LOG_DBG("Resize full image buffer from %dx%d to %dx%d", fullImgReq.mBuffer->getImgSize().w, fullImgReq.mBuffer->getImgSize().h, PostProcOutWidth, PostProcOutHeight);
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(dType, &fullImgReq.mBuffer, PostProcOutWidth, PostProcOutHeight) )
            {
                LOG_ERR("Resize P2A_TO_GPU full image buffer failed");
                return MFALSE;
            }
        }

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_011: frm: %d", getMagicNo(InQParams)); }


        if (
             FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) &&
            !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)  &&
            !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
        )   // When 3DNR only.
        {

            /* Output */
            for (int i=0; i < InQParams.mvOut.size(); i++)
            {
                if(p2aDebugFlag[0] != '0')
                {
					if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_012: frm: %d", getMagicNo(InQParams)); }

                    LOG_DBG("[P2A 3DNR sets mvOut (%d)] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d),magicnum(%d)",i,
                        InQParams.mvOut[i].mBuffer->getImgFormat(),
                        InQParams.mvOut[i].mBuffer->getImgSize().w,
                        InQParams.mvOut[i].mBuffer->getImgSize().h,
                        InQParams.mvOut[i].mPortID.type,
                        InQParams.mvOut[i].mPortID.index,
                        InQParams.mvOut[i].mPortID.inout,
                        InQParams.mvOut[i].mBuffer->getBufVA(0),
                        InQParams.mvOut[i].mBuffer->getBufPA(0),
                        InQParams.mvOut[i].mBuffer->getBufSizeInBytes(0),
                        InQParams.mvOut[i].mExtraParams.p2pxlID,
                        getMagicNo(InQParams)
                    );
                }

                mQParams.mvOut.push_back(InQParams.mvOut[i]);
            }
			if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_013: frm: %d", getMagicNo(InQParams)); }

            for (int i=0; i < InQParams.mvCropRsInfo.size(); i++)
            {
                MCrpRsInfo crop = InQParams.mvCropRsInfo[i];

                if(p2aDebugFlag[0] != '0')
                {
					if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_014: frm: %d", getMagicNo(InQParams)); }

                    LOG_DBG("[P2A 3DNR sets mCropRsInfo] groupid(%d),crop w(%d),h(%d),p_int(%d,%d),p_fra(%d,%d), resize w(%d),h(%d)",
                            crop.mGroupID,crop.mCropRect.s.w,crop.mCropRect.s.h,crop.mCropRect.p_integral.x,crop.mCropRect.p_integral.y,
                            crop.mCropRect.p_fractional.x,crop.mCropRect.p_fractional.y,crop.mResizeDst.w,crop.mResizeDst.h);
                }

                mQParams.mvCropRsInfo.push_back(crop);
            }
        }

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_015: frm: %d", getMagicNo(InQParams)); }

        // Set mCropRsInfo for cropGroup 1.
        if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        {
            MCrpRsInfo eisCrop = InQParams.mvCropRsInfo[eisCrop2Idx];

            eisCrop.mGroupID = 1;
            eisCrop.mCropRect.p_integral.x = eisCropRect.x;
            eisCrop.mCropRect.p_integral.y = eisCropRect.y;
            eisCrop.mCropRect.s.w = PostProcOutWidth;
            eisCrop.mCropRect.s.h = PostProcOutHeight;
            eisCrop.mResizeDst.w  = PostProcOutWidth;
            eisCrop.mResizeDst.h  = PostProcOutHeight;

            eisConfigData.crzOutW = eisCrop.mCropRect.s.w;
            eisConfigData.crzOutH = eisCrop.mCropRect.s.h;
            eisConfigData.srzOutW = eisCrop.mCropRect.s.w; //TODO : need to modified for stereo
            eisConfigData.srzOutH = eisCrop.mCropRect.s.h; //TODO : need to modified for stereo
            eisConfigData.cropX   = eisCrop.mCropRect.p_integral.x;
            eisConfigData.cropY   = eisCrop.mCropRect.p_integral.y;

            mQParams.mvCropRsInfo.push_back(eisCrop);

            LOG_DBG("[EIS crop]ID(%d),crop(%d,%d),p_int(%d,%d),p_fra(%d,%d),resize(%d,%d)",
                    eisCrop.mGroupID,eisCrop.mCropRect.s.w,eisCrop.mCropRect.s.h,eisCrop.mCropRect.p_integral.x,eisCrop.mCropRect.p_integral.y,
                    eisCrop.mCropRect.p_fractional.x,eisCrop.mCropRect.p_fractional.y,eisCrop.mResizeDst.w,eisCrop.mResizeDst.h);
        }
        else if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))  // For non-EIS case, mCropRsInfo will not be changed, so use the mCropRsInfo MW passed down (InQParams.mvCropRsInfo[1]).
        {
            LOG_AST(InQParams.mvCropRsInfo.size() > 0, "Not given any crop info", 0);
            MCrpRsInfo crop = InQParams.mvCropRsInfo[1]; //copy given group 2
            crop.mGroupID    = 1;
            crop.mResizeDst  = fullImgReq.mBuffer->getImgSize();

            mQParams.mvCropRsInfo.push_back(crop);
            LOG_DBG("[P2A vFB sets mCropRsInfo] groupid(%d),crop w(%d),h(%d),p_int(%d,%d),p_fra(%d,%d), resize w(%d),h(%d)",
                    crop.mGroupID,crop.mCropRect.s.w,crop.mCropRect.s.h,crop.mCropRect.p_integral.x,crop.mCropRect.p_integral.y,
                    crop.mCropRect.p_fractional.x,crop.mCropRect.p_fractional.y,crop.mResizeDst.w,crop.mResizeDst.h);
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_016: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*                         Config WDMAO                                 */
    /************************************************************************/
    ImgRequest dsImgReq;
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
static const NSCam::NSIoPipe::PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);

        if(!pBufHandler->dequeBuffer(P2A_TO_VFBSW_DSIMG, &dsImgReq))
        {
            LOG_AST(false, "Out of P2A_TO_VFBSW_DSIMG buffer", 0);
            return MFALSE;
        }
        MSize fullImgSize = fullImgReq.mBuffer->getImgSize();
        MSize dsImgSize;
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
        reinterpret_cast<FeaturePipeVfbNode*>(mpFeaturePipeObj->mpVfbSwNode)->getDSImgSize(fullImgSize, dsImgSize);
#endif
        if ((dsImgReq.mBuffer->getImgSize().w != dsImgSize.w) ||
                (dsImgReq.mBuffer->getImgSize().h != dsImgSize.h))
        {
            LOG_DBG("Resize DS image buffer to %dx%d", dsImgSize.w, dsImgSize.h);
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(P2A_TO_VFBSW_DSIMG, &dsImgReq.mBuffer, dsImgSize.w, dsImgSize.h) )
            {
                LOG_ERR("Resize full image buffer failed");
                return MFALSE;
            }
        }
        NSCam::NSIoPipe::NSPostProc::Output moutput;
        /* PRZO */
        moutput.mPortID = WDMAO;
        moutput.mBuffer = const_cast<IImageBuffer*>(dsImgReq.mBuffer);
        mQParams.mvOut.push_back(moutput);
        LOG_DBG("[P2A sets WDMAO mvOut] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d)",
                moutput.mBuffer->getImgFormat(),moutput.mBuffer->getImgSize().w,moutput.mBuffer->getImgSize().h,
                moutput.mPortID.type,moutput.mPortID.index,moutput.mPortID.inout,
                moutput.mBuffer->getBufVA(0),moutput.mBuffer->getBufPA(0),
                moutput.mBuffer->getBufSizeInBytes(0),moutput.mExtraParams.p2pxlID);
        //
        MCrpRsInfo crop;
        crop.mGroupID    = 2;
        crop.mCropRect.s = fullImgReq.mBuffer->getImgSize();
        crop.mCropRect.p_integral.x   = 0;
        crop.mCropRect.p_integral.y   = 0;
        crop.mCropRect.p_fractional.x = 0;
        crop.mCropRect.p_fractional.y = 0;
        crop.mResizeDst  = {0,0};

        mQParams.mvCropRsInfo.push_back(crop);
        LOG_DBG("[P2A sets mCropRsInfo] groupid(%d),crop w(%d),h(%d),p_int(%d,%d),p_fra(%d,%d), resize w(%d),h(%d)",
                crop.mGroupID,crop.mCropRect.s.w,crop.mCropRect.s.h,crop.mCropRect.p_integral.x,crop.mCropRect.p_integral.y,
                crop.mCropRect.p_fractional.x,crop.mCropRect.p_fractional.y,crop.mResizeDst.w,crop.mResizeDst.h);
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_017: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*                         Config IMG3O                                 */
    /************************************************************************/
    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
       FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) ||
       FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)
    )
    {
        // Config P2A output DMA(IMG3O) for 3DNR. IMG3O uses 3DNR output current frame setting. Other output DMAs use original setting pass down from MiddleWare.
        static const NSCam::NSIoPipe::PortID IMG3O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1);
        NSCam::NSIoPipe::NSPostProc::Output moutput;
        moutput.mPortID = IMG3O;
        moutput.mBuffer = const_cast<IImageBuffer*>(fullImgReq.mBuffer);

        // IMG3O Offset
        #if 0
        MCrpRsInfo MdpCrop;
        MdpCrop = mQParams.mvCropRsInfo[1]; // copy given group 2 (align mdp crop)
        #else   // Modified.
        MCrpRsInfo MdpCrop;
        if (
            !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) &&
             FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) &&
             FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)
        )   // For 3DNR+EIS2.0
        {
            //MdpCrop = mQParams.mvCropRsInfo[0]; // copy given group 1 (align crz)
            u4Img3oOffset_X = 0;   // MDP crop offset (unit of crop.mCropRect.p_integral.x is pixel)
            u4Img3oOffset_Y = 0;   // MDP crop offset (unit of crop.mCropRect.p_integral.y is pixel)
        }
        else    // For 3DNR only, vFB only, EIS2.0 only, 3DNR+vFB, or 3DNR+vFB+EIS2.0
        {
            MdpCrop = mQParams.mvCropRsInfo[1]; // copy given group 2 (align mdp crop)
            u4Img3oOffset_X = MdpCrop.mCropRect.p_integral.x & ~1;   // MDP crop offset (unit of crop.mCropRect.p_integral.x is pixel)
            u4Img3oOffset_Y = MdpCrop.mCropRect.p_integral.y & ~1;   // MDP crop offset (unit of crop.mCropRect.p_integral.y is pixel)
        }
        #endif

        // Assign IMG3O buffer with current frame time stamp.
        moutput.mBuffer->setTimestamp(InQParams.mvIn[0].mBuffer->getTimestamp());

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_018: frm: %d", getMagicNo(InQParams)); }

        if (
            !FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) ||
             FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)  ||
             FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)  ||
             m3dnrStateMachine != NR3D_STATE_STOP
        )   // If not 3DNR only or 3DNR State Machine is not NR3D_STATE_STOP.
        {
            mQParams.mvOut.push_back(moutput);    // Only don't enable IMG3O when 3DNR only and 3DNR state machine is NR3D_STATE_STOP.

            LOG_DBG("[P2A sets IMG3O mvOut] 3dnrSM0(%d S0P1W2),ES(0x%02X FsSzDfLrIn),WH(%d,%d),OfstXY(%d,%d),mem<va(0x%x)/pa(0x%x)/TS(%lld)/size(0x%x)>",
                    m3dnrStateMachine,
                    m3dnrErrorStatus,
                    moutput.mBuffer->getImgSize().w, moutput.mBuffer->getImgSize().h,
                    u4Img3oOffset_X, u4Img3oOffset_Y,
                    moutput.mBuffer->getBufVA(0), moutput.mBuffer->getBufPA(0),
                    moutput.mBuffer->getTimestamp(),
                    moutput.mBuffer->getBufSizeInBytes(0)
                    //moutput.mBuffer->getImgFormat(),
                    //moutput.mPortID.type, moutput.mPortID.index, moutput.mPortID.inout,
                    //moutput.mExtraParams.p2pxlID,
                    //moutput.mBuffer->getBufStridesInBytes(0)
            );

            // Record IMG3O W/H for later use.
            u4Img3oWidth  = moutput.mBuffer->getImgSize().w;
            u4Img3oHeight = moutput.mBuffer->getImgSize().h;
        }
        else
        {
            LOG_DBG("[P2A not sets IMG3O mvOut] 3dnrSM0(%d S0P1W2),ES(0x%02X FsSzDfLrIn),WH(%d,%d),OfstXY(%d,%d),mem<va(0x%x)/pa(0x%x)/TS(%lld)/size(0x%x)>",
                    m3dnrStateMachine,
                    m3dnrErrorStatus,
                    moutput.mBuffer->getImgSize().w, moutput.mBuffer->getImgSize().h,
                    u4Img3oOffset_X, u4Img3oOffset_Y,
                    moutput.mBuffer->getBufVA(0), moutput.mBuffer->getBufPA(0),
                    moutput.mBuffer->getTimestamp(),
                    moutput.mBuffer->getBufSizeInBytes(0)
                    //moutput.mBuffer->getImgFormat(),
                    //moutput.mPortID.type, moutput.mPortID.index, moutput.mPortID.inout,
                    //moutput.mExtraParams.p2pxlID,
                    //moutput.mBuffer->getBufStridesInBytes(0)
            );

            // Record IMG3O W/H for later use.
            u4Img3oWidth  = moutput.mBuffer->getImgSize().w;
            u4Img3oHeight = moutput.mBuffer->getImgSize().h;
        }

    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_019: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*                                 ???                                  */
    /************************************************************************/
    //>> EIS
    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        eisConfigData.imgiW = InQParams.mvIn[0].mBuffer->getImgSize().w;
        eisConfigData.imgiH = InQParams.mvIn[0].mBuffer->getImgSize().h;
        eisConfigData.configSce = EIS_SCE_EIS_PLUS;

        LOG_DBG("[FPipe ConfigEis] imgi(%u,%u),crz(%u,%u),srz(%u,%u),feT(%u,%u),gpuT(%u,%u),crop(%u,%u),sce(%d)",eisConfigData.imgiW,
                                                                                                                 eisConfigData.imgiH,
                                                                                                                 eisConfigData.crzOutW,
                                                                                                                 eisConfigData.crzOutH,
                                                                                                                 eisConfigData.srzOutW,
                                                                                                                 eisConfigData.srzOutH,
                                                                                                                 eisConfigData.feTargetW,
                                                                                                                 eisConfigData.feTargetH,
                                                                                                                 eisConfigData.gpuTargetW,
                                                                                                                 eisConfigData.gpuTargetH,
                                                                                                                 eisConfigData.cropX,
                                                                                                                 eisConfigData.cropY,
                                                                                                                 eisConfigData.configSce);

        if(MTRUE != reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->ConfigEis(EIS_PASS_2,eisConfigData))
        {
            LOG_ERR("FPipe ConfigEis fail");
            return MFALSE;
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_020: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*                         Config VIPI                                  */
    /************************************************************************/
    // Config VIPI for 3DNR previous frame input.
    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        MUINT32 u4VipiOffset_X = 0;
        MUINT32 u4VipiOffset_Y = 0;
        MUINT32 u4PixelToBytes = 0;

        /* VIPI */
        static const NSCam::NSIoPipe::PortID VIPI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIPI, 0/*in*/);
        NSCam::NSIoPipe::NSPostProc::Input minput;
        minput.mPortID = VIPI;

        if (!m3dnrPrvFrmQueue.empty())
        {
            FeaturePipeNodeImgReqJob_s jobFullImg;

            // Get previous frame from mqJobFullImg.
            jobFullImg = m3dnrPrvFrmQueue.front();

			if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_021: frm: %d", getMagicNo(InQParams)); }

            minput.mBuffer = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer);

            // Calculate VIPI Start Address = nmv_x + nmv_y * vipi_stride. Unit: bytes.
            //     Calculate Offset X/Y according to NMV X/Y.
            u4VipiOffset_X = (mNmvX >= 0) ? (mNmvX): (0);
            u4VipiOffset_Y = (mNmvY >= 0) ? (mNmvY): (0);
            //     Calculate u4PixelToBytes.
            if (minput.mBuffer->getImgFormat() == eImgFmt_YUY2) u4PixelToBytes = 2;
            if (minput.mBuffer->getImgFormat() == eImgFmt_YV12) u4PixelToBytes = 1;
            //     Calculate VIPI start addr offset.
            u4VipiStartAddrOffset = u4VipiOffset_Y * minput.mBuffer->getBufStridesInBytes(0) + u4VipiOffset_X * u4PixelToBytes;
            //     Calculate VIPI valid region w/h.
            TempImgSize.w = minput.mBuffer->getImgSize().w - abs(mNmvX);  // valid region w
            TempImgSize.h = minput.mBuffer->getImgSize().h - abs(mNmvY);  // valid region h
            imgSize.w = TempImgSize.w & ~1;  // valid region w
            imgSize.h = TempImgSize.h & ~1;  // valid region h
            //     Set VIPI start addr offset and region w/h.
            //minput.mBuffer->setExtParam(imgSize, u4VipiStartAddrOffset);


            // Check whether current frame size is equal to last frame size.
            property_get("camera.3dnr.forceskip", aInputValue, "0");
            if (aInputValue[0] == '1')
            {
                m3dnrErrorStatus |= NR3D_ERROR_FORCE_SKIP;
                if (m3dnrStateMachine == NR3D_STATE_WORKING) m3dnrStateMachine = NR3D_STATE_PREPARING;  // Current frame don't do 3DNR, but IMG3O still needs to output current frame for next run use.
            }

            if (m3dnrStateMachine == NR3D_STATE_WORKING)
            {
                mQParams.mvIn.push_back(minput);    // Only push_back VIPI when N3RD state machine is NR3D_STATE_WORKING.
                //LOG_DBG("push_back VIPI buffer.");

                LOG_DBG("[P2A sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn),WH(%d,%d),OfstXY(%d,%d),mem<va(0x%x)/pa(0x%x)/TS(%lld)/size(0x%x)>,mPrvQueSz(%d),imgSizeWH(%d,%d),PrvWH(%d,%d),ExtOfstByte(%d)",
                        m3dnrStateMachine,
                        m3dnrErrorStatus,
                        minput.mBuffer->getImgSize().w, minput.mBuffer->getImgSize().h,
                        u4VipiOffset_X, u4VipiOffset_Y,
                        minput.mBuffer->getBufVA(0), minput.mBuffer->getBufPA(0),
                        minput.mBuffer->getTimestamp(),
                        minput.mBuffer->getBufSizeInBytes(0),
                        (m3dnrPrvFrmQueue.empty()) ? (0) : (m3dnrPrvFrmQueue.size()),
                        imgSize.w,imgSize.h,
                        mPrevFrameWidth, mPrevFrameHeight,
                        minput.mBuffer->getExtOffsetInBytes(0)
                );

                // Record VIPI W/H for later use.
                u4VipiWidth   = imgSize.w;//minput.mBuffer->getImgSize().w;
                u4VipiHeight  = imgSize.h;//minput.mBuffer->getImgSize().h;
            }
            else    // Not NR3D_STATE_WORKING.
            {
                LOG_DBG("[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn),WH(%d,%d),OfstXY(%d,%d),mem<va(0x%x)/pa(0x%x)/TS(%lld)/size(0x%x)>,mPrvQueSz(%d),imgSizeWH(%d,%d),PrvWH(%d,%d),ExtOfstByte(%d)",
                        m3dnrStateMachine,
                        m3dnrErrorStatus,
                        minput.mBuffer->getImgSize().w, minput.mBuffer->getImgSize().h,
                        u4VipiOffset_X, u4VipiOffset_Y,
                        minput.mBuffer->getBufVA(0), minput.mBuffer->getBufPA(0),
                        minput.mBuffer->getTimestamp(),
                        minput.mBuffer->getBufSizeInBytes(0),
                        (m3dnrPrvFrmQueue.empty()) ? (0) : (m3dnrPrvFrmQueue.size()),
                        imgSize.w,imgSize.h,
                        mPrevFrameWidth, mPrevFrameHeight,
                        minput.mBuffer->getExtOffsetInBytes(0)
                );

                // Record VIPI W/H for later use.
                u4VipiWidth   = 0;
                u4VipiHeight  = 0;
            }
        }
        else
        {
            LOG_DBG("[P2A not sets VIPI mvIn  ] 3dnrSM1(%d S0P1W2),ES(0x%02X FsSzDfLrIn). m3dnrPrvFrmQueue is empty", m3dnrStateMachine, m3dnrErrorStatus); // m3dnrPrvFrmQueue is empty => maybe first run.

            // Record VIPI W/H for later use.
            u4VipiWidth   = 0;
            u4VipiHeight  = 0;
        }

    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_022: frm: %d", getMagicNo(InQParams)); }

    /************************************************************************/
    /*               Config enque parameters for 3DNR                       */
    /************************************************************************/
    ModuleInfo moduleinfo;             //set for nr3d module
    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        if (m3dnrStateMachine == NR3D_STATE_WORKING)    // Only set NR3D register when N3RD state machine is NR3D_STATE_WORKING.
        {
            mpNr3dParam->ctrl_onEn = 1;
            mpNr3dParam->onOff_onOfStX = ((mNmvX >= 0) ? (0) : (-mNmvX)) + u4Img3oOffset_X;   // Must be even.
            mpNr3dParam->onOff_onOfStY = ((mNmvY >= 0) ? (0) : (-mNmvY)) + u4Img3oOffset_Y;
            mpNr3dParam->onSiz_onWd = u4VipiWidth  & ~1;   // Must be even.
            mpNr3dParam->onSiz_onHt = u4VipiHeight & ~1;
            mpNr3dParam->vipi_offst = u4VipiStartAddrOffset;    //in byte
            mpNr3dParam->vipi_readW = imgSize.w;    //in pixel
            mpNr3dParam->vipi_readH = imgSize.h;    //in pixel

            moduleinfo.moduleTag = EFeatureModule_NR3D;
            moduleinfo.moduleStruct   = reinterpret_cast<MVOID*> (mpNr3dParam);
            mQParams.mvModuleData.push_back(moduleinfo);    // Only push_back moduleinfo (i.e. driver will enable NR3D HW module) when N3RD status is normal.
            // Raise AE ISO limit to 130%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 130: Raise Increase ISO Limit to 130% (increase 30%). 100: it means don't need to increase.
            mp3AHal->modifyPlineTableLimitation(MTRUE, MTRUE, get_3dnr_max_iso_increase_percentage(), 100);
        }
        else    // 3DNR not in working state, don't do 3DNR.
        {
            mpNr3dParam->ctrl_onEn      = 1;
            mpNr3dParam->onOff_onOfStX  = 0;   // Must be even.
            mpNr3dParam->onOff_onOfStY  = 0;
            mpNr3dParam->onSiz_onWd     = 0;   // Must be even.
            mpNr3dParam->onSiz_onHt     = 0;
            mpNr3dParam->vipi_offst     = 0;    //in byte
            mpNr3dParam->vipi_readW     = 0;    //in pixel
            mpNr3dParam->vipi_readH     = 0;    //in pixel
            // Restore AE ISO limit to 100%. Parameter meaning: MTRUE: Enable the function. MTRUE: Need to equivalent for orginal BV range. 100: it means don't need to increase. 100: it means don't need to increase.
            mp3AHal->modifyPlineTableLimitation(MTRUE, MTRUE, 100, 100);
        }

        LOG_DBG("3dnrSM2(%d S0P1W2),ES(0x%02X FsSzDfLrIn),NmvX/Y(%d, %d).Img3oOfX/Y(%d, %d),onOfX/Y(%d, %d).onW/H(%d, %d).VipiOff/W/H(%d, %d, %d).MaxIsoInc(%d)",
            m3dnrStateMachine,
            m3dnrErrorStatus,
            mNmvX, mNmvY,
            u4Img3oOffset_X, u4Img3oOffset_Y,
            mpNr3dParam->onOff_onOfStX, mpNr3dParam->onOff_onOfStY,
            mpNr3dParam->onSiz_onWd, mpNr3dParam->onSiz_onHt,
            mpNr3dParam->vipi_offst, mpNr3dParam->vipi_readW, mpNr3dParam->vipi_readH,
            get_3dnr_max_iso_increase_percentage()
        );

        // Recordng mPrevFrameWidth/mPrevFrameHeight for next frame.
        mPrevFrameWidth  = u4Img3oWidth;
        mPrevFrameHeight = u4Img3oHeight;
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_023: frm: %d", getMagicNo(InQParams)); }

    // Config enque parameters for EIS.
    ModuleInfo eisFeoModuleinfo;
    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        //> get feo info

        FEO_REG_INFO feoRegInfo;
        reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->GetFeoRegInfo(&feoRegInfo);

        mpFeoParam->bufInfo.size        = feoRegInfo.size;
        mpFeoParam->bufInfo.memID       = feoRegInfo.memID;
        mpFeoParam->bufInfo.virtAddr    = feoRegInfo.va;
        mpFeoParam->bufInfo.phyAddr     = feoRegInfo.pa;
        mpFeoParam->bufInfo.bufSecu     = feoRegInfo.bufSecu;
        mpFeoParam->bufInfo.bufCohe     = feoRegInfo.bufCohe;
        mpFeoParam->bufInfo.useNoncache = feoRegInfo.useNoncache;

        for(MUINT32 i = 0; i < mQParams.mvOut.size(); i++)
        {
            if(mQParams.mvOut[i].mPortID.index == NSImageio::NSIspio::EPortIndex_IMG3O)
            {
                mpFeoParam->w = (mQParams.mvOut[i].mBuffer->getImgSize().w / eisFeBlock) * 56;
                mpFeoParam->h = mQParams.mvOut[i].mBuffer->getImgSize().h / eisFeBlock;
                LOG_DBG("[FPipeEis](%d,%d),block(%u)",mQParams.mvOut[i].mBuffer->getImgSize().w,mQParams.mvOut[i].mBuffer->getImgSize().h,eisFeBlock);
                break;
            }
        }

        mpFeoParam->stride = feoRegInfo.stride;

        mpFeoParam->port_idx   = NSImageio::NSIspio::EPortIndex_FEO;
        mpFeoParam->port_type  = NSIoPipe::EPortType_Memory;
        mpFeoParam->port_inout = 1;

        eisFeoModuleinfo.moduleTag = EFeatureModule_STA_FEO;
        eisFeoModuleinfo.moduleStruct   = reinterpret_cast<MVOID*>(mpFeoParam);
        mQParams.mvModuleData.push_back(eisFeoModuleinfo);
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_024: frm: %d", getMagicNo(InQParams)); }

#ifdef MTK_CAM_VHDR_SUPPORT

    if(FEATURE_MASK_IS_VHDR_ENABLED(mFeatureOnMask))
    {
        LCEI_DMA_INFO aLceiDma;
        ModuleInfo vhdrLceiModuleinfo;

        //> get LCEI info

        mpVHdrObj->GetLceiDmaInfo(&aLceiDma,InQParams.mvIn[0].mBuffer->getTimestamp());

        if(aLceiDma.memID == VHDR_LCSO_SYNC_FAIL)
        {
            LOG_ERR("[FPipe] VHDR_LCSO_SYNC_FAIL");
        }

        //> prepare info and push to QParam

        mpLceiParam->bufInfo.size        = aLceiDma.size;
        mpLceiParam->bufInfo.memID       = aLceiDma.memID;
        mpLceiParam->bufInfo.virtAddr    = aLceiDma.va;
        mpLceiParam->bufInfo.phyAddr     = aLceiDma.pa;
        mpLceiParam->bufInfo.bufSecu     = aLceiDma.bufSecu;
        mpLceiParam->bufInfo.bufCohe     = aLceiDma.bufCohe;
        mpLceiParam->bufInfo.useNoncache = aLceiDma.useNoncache;

        mpLceiParam->w          = aLceiDma.xSize;
        mpLceiParam->h          = aLceiDma.ySize;
        mpLceiParam->stride     = aLceiDma.stride;
        mpLceiParam->port_idx   = NSImageio::NSIspio::EPortIndex_LCEI;
        mpLceiParam->port_type  = NSIoPipe::EPortType_Memory;
        mpLceiParam->port_inout = 1;

        vhdrLceiModuleinfo.moduleTag    = EFeatureModule_STA_LCEI;
        vhdrLceiModuleinfo.moduleStruct = reinterpret_cast<MVOID*>(mpLceiParam);

        LOG_DBG("[FPipe] LCEI_PA(0x%08x)",mpLceiParam->bufInfo.phyAddr);

        mQParams.mvModuleData.push_back(vhdrLceiModuleinfo);
    }

#endif

    /************************************************************************/
    /*               Config enque parameters for Stereo Camera              */
    /************************************************************************/
    if (FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask))
    {
        if (!FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) &&
            !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)  &&
            !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)
           )   // When Stereo Camera only
        {
            for (int i=0; i < InQParams.mvOut.size(); i++)
            {
                mQParams.mvOut.push_back(InQParams.mvOut[i]);
            }
            for (int i=0; i < InQParams.mvCropRsInfo.size(); i++)
            {
                MCrpRsInfo crop = InQParams.mvCropRsInfo[i];
                mQParams.mvCropRsInfo.push_back(crop);
            }
        }

        for(int i=0; i<InQParams.mvModuleData.size(); i++)
        {
            if(InQParams.mvModuleData[i].moduleTag == (MINT32)(EFeatureModule_STA_FEO) ||
               InQParams.mvModuleData[i].moduleTag == (MINT32)(EFeatureModule_SRZ1) ||
               InQParams.mvModuleData[i].moduleTag == (MINT32)(EFeatureModule_FE))
            {
                LOG_DBG("[FPipe] Stereo pushing %s data", (InQParams.mvModuleData[i].moduleTag == (MINT32)(EFeatureModule_STA_FEO))? "FEO": ((InQParams.mvModuleData[i].moduleTag == (MINT32)(EFeatureModule_SRZ1))?"SRZ":"FE"));
                mQParams.mvModuleData.push_back(InQParams.mvModuleData[i]);
            }
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_025: frm: %d", getMagicNo(InQParams)); }

    /* Enque */
    CPTLog(Event_FeaturePipe_P2A_FeatureStream, CPTFlagStart);

    if (InQParams.mpfnCallback != NULL)
    {
        mQParams.mpCookie = this;
        mQParams.mpfnCallback = p2aCbFunc;
        if (
            ( FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask)    &&
             !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)     &&
             !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) )   ||
            ( FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask)    &&
             !FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)     &&
             !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) )
        )
        {
            Mutex::Autolock lock(mLock);
            mqCallbackQParams.push(InQParams);
        }
    }


	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_026: frm: %d", getMagicNo(InQParams)); }

    // mpIFeatureStream enque for 3DNR/EIS.
    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask) ||
        FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) ||
        FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) ||
        FEATURE_MASK_IS_STEREO_CAMERA_ENABLED(mFeatureOnMask))
    {
        LOG_DBG("FStrm enq (frame %d)", getMagicNo(mQParams));

        if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
        {
            reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->FlushMemory(EIS_DMA_FEO,EIS_FLUSH_HW);
        }

        CAM_TRACE_FMT_BEGIN("P2AFStrm:%d", getMagicNo(InQParams));
        MBOOL ret = mpIFeatureStream->enque(mQParams);
        CAM_TRACE_FMT_END();
        if (!ret)
        {
            LOG_ERR("IFeatureStream enque failed!");
            //AEE_ASSERT("IFeatureStream enque failed");
            return MFALSE;
        }
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_027: frm: %d", getMagicNo(InQParams)); }

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        handlePostBuffer(P2A_TO_P2B, (MUINTPTR) &InQParams, getMagicNo(InQParams));
    }
    else if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        handlePostBuffer(P2A_TO_MDP_QPARAMS, (MUINTPTR) &InQParams, getMagicNo(InQParams));
    }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_028: frm: %d", getMagicNo(InQParams)); }

    /* Deque */
    if (InQParams.mpfnCallback == NULL)
    {
        QParams outParams;
        if(FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
        {
            //MBOOL ret = mpIFeatureStream->deque(outParams, -1/*TODO*/, mQFeatureData);
            MBOOL ret = mpIFeatureStream->deque(outParams, -1/*TODO*/);
            if (!ret)
            {
                LOG_ERR("IFeatureStream deque failed(frame %d)", getMagicNo(outParams));
                return MFALSE;
            }
            LOG_DBG("IFeatureStream deque succeeds(frame %d)", getMagicNo(outParams));

            if (InQParams.mpfnCallback == NULL)
            {
                Mutex::Autolock lock(mLock);
                InQParams.mDequeSuccess = outParams.mDequeSuccess;
                qCallerQParams.push(InQParams);
                sem_post(&mModuleSem);
            }

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_029: frm: %d", getMagicNo(InQParams)); }

        }
        else
        {
            if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
            {
                MBOOL ret = mpIFeatureStream->deque(outParams, -1/*TODO*/);

                if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
                {
                    reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->FlushMemory(EIS_DMA_FEO,EIS_FLUSH_SW);
                }

                if (!ret)
                {
                    LOG_ERR("IFeatureStream EisPlus deque failed(frame %d)", getMagicNo(outParams));
                    return MFALSE;
                }
                LOG_DBG("IFeatureStream EisPlus deque succeeds(frame %d)", getMagicNo(outParams));
            }
            else //vFB
            {
                MBOOL ret = mpIFeatureStream->deque(outParams, -1/*TODO*/);
                if (!ret)
                {
                    LOG_ERR("IFeatureStream deque failed! (frame %d)", getMagicNo(outParams));
                    return MFALSE;
                }
                LOG_DBG("IFeatureStream deque succeeds! (frame %d)", getMagicNo(outParams));
            }
			if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_030: frm: %d", getMagicNo(InQParams)); }

        }
        CPTLog(Event_FeaturePipe_P2A_FeatureStream, CPTFlagEnd);

        handleP2Done(outParams);

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_031: frm: %d", getMagicNo(InQParams)); }

    }

    // When 3DNR only, return (i.e. enque) previous frame buffer because it won't be used anymore.
    if (FEATURE_MASK_IS_3DNR_ENABLED(mFeatureOnMask))
    {
        sem_wait(&mCallbackSem);

        FeaturePipeNodeImgReqJob_s jobFullImg;
        if (!m3dnrPrvFrmQueue.empty())
        {
            jobFullImg = m3dnrPrvFrmQueue.front();
            m3dnrPrvFrmQueue.pop();

            if (!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) && !FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
            {
                pBufHandler->enqueBuffer(P2A_TO_GPU, jobFullImg.imgReq.mBuffer);
            }

        }

		if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_032: frm: %d", getMagicNo(InQParams)); }


        // Push current frame for next run use as previous frame.
        FeaturePipeNodeImgReqJob_s job;
        job.imgReq = fullImgReq;
        job.mFrameNo = getMagicNo(mQParams);
        m3dnrPrvFrmQueue.push(job);
    }

    CPTLog(Event_FeaturePipe_P2AThread, CPTFlagEnd);

	if (sgmkDebugFlag[0] != '0') { LOG_DBG("mkdbg_p2athr_033: frm: %d", getMagicNo(InQParams)); }

    CAM_TRACE_FMT_END();

    //FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeP2aNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    switch (data)
    {
        case P2A_TO_VFBSW_DSIMG:
        case P2A_TO_SWITCHER:
        case P2A_TO_GPU:
            LOG_DBG("Recycling P2A node %s image of frame %d", (data == P2A_TO_VFBSW_DSIMG)? "DS": "full", ext);
            pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
            break;
        case P2A_TO_EISSW:
        case P2A_TO_FD_DSIMG:
            /* Do nothing. Wait until VFB returns DS image buffer */
            break;
    }
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeP2aNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[SWT:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[SWT:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[SWT:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[SWT:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[SWT:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[SWT:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeSwitcherNode*
FeaturePipeSwitcherNode::createInstance(MUINT32 featureOnMask, FeaturePipe *pFeaturePipeObj)
{
    return new FeaturePipeSwitcherNode("FeaturePipeSwitcherNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, pFeaturePipeObj);
}
MVOID
FeaturePipeSwitcherNode::destroyInstance()
{
    delete this;
}
FeaturePipeSwitcherNode::FeaturePipeSwitcherNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        FeaturePipe *pFeaturePipeObj
        )
        : FeaturePipeNode(name, type, policy, priority, featureOnMask, 0, pFeaturePipeObj),
          mVfbExtreme(true),
          mGpuProcCount(0)
{
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_SRC, P2A_TO_SWITCHER);
        addDataSupport(ENDPOINT_SRC, MDP_TO_SWITCHER);
        addDataSupport(ENDPOINT_DST, SWITCHER_TO_GPU);
        addDataSupport(ENDPOINT_DST, SWITCHER_TO_VFBSW);
        addDataSupport(ENDPOINT_DST, SWITCHER_TO_P2B);

        addNotifySupport(FEATURE_MASK_CHANGE);
    }
}

MBOOL
FeaturePipeSwitcherNode::onInit()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeSwitcherNode::
onUninit()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeSwitcherNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeSwitcherNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeSwitcherNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeSwitcherNode::
setVfbExtreme(MBOOL mode)
{
    Mutex::Autolock lock(mVfbExtremeLock);
    mVfbExtreme = mode;
}
MBOOL
FeaturePipeSwitcherNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    FeaturePipeNodeImgReqJob_s job;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case P2A_TO_SWITCHER:
            mqJobFromP2a.push(job);
            triggerLoop();
            break;

        case MDP_TO_SWITCHER:
            mqJobFromGpu.push(job);
            triggerLoop();
            break;
    }

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeSwitcherNode::
threadLoopUpdate()
{
    FUNCTION_LOG_START;
    if (mqJobFromGpu.empty() && mqJobFromP2a.empty())
        return MTRUE;

    if (!mqJobFromGpu.empty())
    {
        FeaturePipeNodeImgReqJob_s job;

        {
            Mutex::Autolock lock(mLock);
            /* Get full image job */
            job = mqJobFromGpu.front();
            mqJobFromGpu.pop();

            mqGpuToP2b.push(MTRUE);
        }

        /* Post buffer to next node */
        LOG_DBG("Posting SWITCHER(GPU)->P2B node full image of frame %d", job.mFrameNo);
        handlePostBuffer(SWITCHER_TO_P2B, (MUINTPTR) &job.imgReq, job.mFrameNo);

        LOG_AST(mGpuProcCount>0, "Switcher GPU-processing counter is wrong", 0);
        mGpuProcCount--;
    }

    {
        Mutex::Autolock lock(mVfbExtremeLock);
        if (mVfbExtreme ||
            FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask) ||
            (mGpuProcCount == 0))
        {
            while(!mqJobFromP2a.empty())
            {
                FeaturePipeNodeImgReqJob_s job;

                {
                    Mutex::Autolock lock(mLock);
                    /* Get full image job */
                    job = mqJobFromP2a.front();
                    mqJobFromP2a.pop();
                }

                LOG_DBG("vFB %s mode", mVfbExtreme? "Extreme": "Normal");
                if (mVfbExtreme ||
                    FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
                {
                    LOG_DBG("Posting SWITCHER->GPU node full image of frame %d", job.mFrameNo);
                    handlePostBuffer(SWITCHER_TO_GPU, (MUINTPTR) &job.imgReq, job.mFrameNo);
                    mGpuProcCount++;
                }
                else
                {
                    {
                        Mutex::Autolock lock(mLock);
                        mqGpuToP2b.push(MFALSE);
                    }

                    LOG_DBG("Posting SWITCHER(P2A)->P2B node full image of frame %d", job.mFrameNo);
                    handlePostBuffer(SWITCHER_TO_P2B, (MUINTPTR) &job.imgReq, job.mFrameNo);
                }

                TagImgRequest tagImgReq;
                tagImgReq.imgReq = job.imgReq;
                tagImgReq.vfbExtreme = mVfbExtreme;
                LOG_DBG("Posting SWITCHER->VFB node full image of frame %d", job.mFrameNo);
                handlePostBuffer(SWITCHER_TO_VFBSW, (MUINTPTR) &tagImgReq, job.mFrameNo);
            }
        }
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeSwitcherNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    MBOOL bGpuTo_P2b;
    switch (data)
    {
        case SWITCHER_TO_P2B:
            {
                Mutex::Autolock lock(mLock);
                bGpuTo_P2b = mqGpuToP2b.front();
                mqGpuToP2b.pop();
            }
            if (bGpuTo_P2b)
            {
                handleReturnBuffer(MDP_TO_SWITCHER, buf, ext);
                break;
            }
            /* Else, then it's from P2A; continue to next case... */
        case SWITCHER_TO_GPU:
            handleReturnBuffer(P2A_TO_SWITCHER, buf, ext);
            break;
        case SWITCHER_TO_VFBSW:
            /* Do nothing. Wait until GPU or P2B returns full image buffer */
            break;
    }
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeSwitcherNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[GPU:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[GPU:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[GPU:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[GPU:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[GPU:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[GPU:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeGpuNode*
FeaturePipeGpuNode::createInstance(MUINT32 featureOnMask, FeaturePipe *pFeaturePipeObj)
{
    return new FeaturePipeGpuNode("FeaturePipeGpuNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, pFeaturePipeObj);
}
MVOID
FeaturePipeGpuNode::destroyInstance()
{
    delete this;
}
FeaturePipeGpuNode::FeaturePipeGpuNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        FeaturePipe *pFeaturePipeObj)
    : FeaturePipeNode(name, type, policy, priority, featureOnMask, 0, pFeaturePipeObj),
      mWarpWorkBufSize(0),
      mpWorkingBuffer(NULL),
      mpGpuWarp(NULL),
      mInputImageWidth(0),
      mInputImageHeight(0),
      mOutputImageWidth(0),
      mOutputImageHeight(0)
{
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_SRC, SWITCHER_TO_GPU);
        addDataSupport(ENDPOINT_SRC, VFBSW_TO_GPU);
        addDataSupport(ENDPOINT_DST, GPU_TO_MDP);
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_SRC, EISSW_TO_GPU);
        if(!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
        {
            addDataSupport(ENDPOINT_SRC, P2A_TO_GPU);
        }
        addDataSupport(ENDPOINT_DST, GPU_TO_MDP);
    }
}
MBOOL
FeaturePipeGpuNode::onInit()
{
    FUNCTION_LOG_START;

    pBufHandler = getBufferHandler(P2A_TO_GPU);

    /* Allocate buffer for next node */
    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpgpu", EnableOption, "0");
        if(EnableOption[0] == '2' || EnableOption[0] == '3')
        {
            pBufHandler = getBufferHandler(8001);
            if (pBufHandler == NULL)
            {
                LOG_ERR("EIS gfxo pBufHandler failed!");
                goto _Exit;
            }

            LOG_DBG("EIS gfxo buffer(%dx%d)", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT);
            AllocInfo allocinfo(FULL_IMAGE_MAX_WIDTH,
                                FULL_IMAGE_MAX_HEIGHT,
                                eImgFmt_YV12,
                                eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
            if( !pBufHandler->requestBuffer(8001, allocinfo) )
            {
                LOG_ERR("EIS gfxo buffer fail");
                return MFALSE;
            }
        }
    }

    sem_init(&mModuleSem, 0, 0);

    FUNCTION_LOG_END;
    return true;

_Exit:
    return false;
}

MBOOL
FeaturePipeGpuNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;

    sem_destroy(&mModuleSem);

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeGpuNode::
threadInit()
{
    FUNCTION_LOG_START;
    MRESULT mret;
    vector< sp<GraphicBuffer>* > vpGB;

#ifndef _WORKAROUND_GPU_FLASH
    reinterpret_cast<FeaturePipeP2aNode*>(mpFeaturePipeObj->mpP2aNode)->getGraphicBuffer(vpGB);
    LOG_AST(vpGB.size(), "P2A does not provide any GB", 0);
    sp<GraphicBuffer>* srcGBArray[vpGB.size()];
#else
    sp<GraphicBuffer>* srcGBArray[1];
    /* Allocate Graphic buffer */
    mGfxYuvBuffer = new GraphicBuffer(FULL_IMAGE_MAX_WIDTH,
                                      FULL_IMAGE_MAX_HEIGHT,
                                      PIXEL_FORMAT_RGBA_8888,
                                      GraphicBuffer::USAGE_HW_TEXTURE | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN);
    if (mGfxYuvBuffer == NULL)
    {
        LOG_ERR("GraphicBuffer allocation for YV12 failed!");
        goto _Exit;
    }
#endif //_WORKAROUND_GPU_FLASH

    /* Create MTKWarp instance */
    mpGpuWarp = MTKWarp::createInstance(
#ifndef _CPUWARP
            DRV_WARP_OBJ_GLES
#else
            DRV_WARP_OBJ_CPU
#endif
            );
    if (mpGpuWarp == NULL)
    {
        LOG_ERR("MTKWarp create instance failed!");
        goto _Exit;
    }

    {
        pBufHandler = getBufferHandler(GPU_TO_MDP);
        if (pBufHandler == NULL)
        {
            LOG_ERR("ICamBufHandler getBufferHandler(GPU_TO_MDP) failed!");
            goto _Exit;
        }
        for (MUINT8 i = 0; i < GPU_NODE_BUFFER_NUM; i++)
        {
            AllocInfo allocinfo(FULL_IMAGE_MAX_WIDTH,
                                FULL_IMAGE_MAX_HEIGHT,
                                eImgFmt_RGBA8888,
                                eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
            mGfxRgbaBuffer[i] = new GraphicBuffer(FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT, PIXEL_FORMAT_RGBA_8888, GraphicBuffer::USAGE_HW_RENDER | GraphicBuffer::USAGE_SW_READ_OFTEN | GraphicBuffer::USAGE_SW_WRITE_OFTEN);
            if (mGfxRgbaBuffer[i] == NULL)
            {
                LOG_ERR("GraphicBuffer allocation for RGBA8888 failed!");
                goto _Exit;
            }

            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->requestBuffer_GB(GPU_TO_MDP, allocinfo, &(mGfxRgbaBuffer[i])) )
            {
                LOG_ERR("Request GPU_TO_MDP full image buffer failed");
                goto _Exit;
            }
        }
    }

#ifdef _CPUWARP
    mpCpuWarpInputBuffer = malloc(FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 3 / 2);
    mpCpuWarpOutputBuffer = malloc(FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 4);
#endif

    /* Create GPU context and shader program */
    struct WarpImageExtInfo warp_info;
    GpuTuning gpu_tuning;
    sp<GraphicBuffer>* dstGBArray[GPU_NODE_BUFFER_NUM];
    gpu_tuning.GLESVersion = 2;
    gpu_tuning.Demo = 0;
    warp_info.Features = 0;
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        ADD_FEATURE(warp_info.Features, MTK_VFB_BIT);
    }
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        ADD_FEATURE(warp_info.Features, MTK_EIS2_BIT);
    }
#ifndef _WORKAROUND_GPU_FLASH
    warp_info.ImgFmt = WARP_IMAGE_YV12;
    {
        int i = 0;
        {
            for (vector< sp<GraphicBuffer>* >::iterator it=vpGB.begin(); it!=vpGB.end(); it++)
                srcGBArray[i++] = *it;
        }
    }
#else
    warp_info.ImgFmt = WARP_IMAGE_RGBA8888;
    srcGBArray[0] = &mGfxYuvBuffer;
#endif //_WORKAROUND_GPU_FLASH
    warp_info.OutImgFmt = WARP_IMAGE_RGBA8888;
#ifndef _CPUWARP
    for (MUINT8 i = 0; i < GPU_NODE_BUFFER_NUM; i++)
        dstGBArray[i] = &(mGfxRgbaBuffer[i]);
    warp_info.SrcGraphicBuffer = (void*) &srcGBArray;
    warp_info.DstGraphicBuffer = (void*) &dstGBArray;
#ifndef _WORKAROUND_GPU_FLASH
    warp_info.InputGBNum = vpGB.size();
#else
    warp_info.InputGBNum = 1;
#endif //_WORKAROUND_GPU_FLASH
    warp_info.OutputGBNum = GPU_NODE_BUFFER_NUM;
#else
    warp_info.SrcGraphicBuffer = mpCpuWarpInputBuffer;
    warp_info.DstGraphicBuffer = mpCpuWarpOutputBuffer;
#endif
//    warp_info.Width = FULL_IMAGE_MAX_WIDTH;
//    warp_info.Height = FULL_IMAGE_MAX_HEIGHT;
    warp_info.Width = NR3D_WORKING_BUFF_WIDTH;  // For 3DNR/EIS/VFB co-exist.
    warp_info.Height = NR3D_WORKING_BUFF_HEIGHT;  // For 3DNR/EIS/VFB co-exist

    warp_info.pTuningPara = &gpu_tuning;
    warp_info.WarpMapNum = 1;
    warp_info.WarpMatrixNum = 0;
    warp_info.MaxWarpMapSize[0] = VFB_DS_IMAGE_MAX_WIDTH;
    warp_info.MaxWarpMapSize[1] = VFB_DS_IMAGE_MAX_HEIGHT;
    //warp_info.demo = 0;
    LOG_DBG("WarpInit features=0x%x, src buffer=0x%p, dst buffer=0x%p, width=%d, height=%d", warp_info.Features, warp_info.SrcGraphicBuffer, warp_info.DstGraphicBuffer, warp_info.Width, warp_info.Height);
    mret = mpGpuWarp->WarpInit((MUINT32*) &warp_info, (MUINT32*) NULL);
    if (mret != S_WARP_OK)
    {
        LOG_ERR("MTKWarp init failed! (%d)", mret);
        goto _Exit;
    }

    /* Allocate working memory */
    unsigned int warpWorkBufSize;
    mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_GET_WORKBUF_SIZE, NULL, &warpWorkBufSize);
    if (mret != S_WARP_OK)
    {
        LOG_ERR("MTKWarp WARP_FEATURE_GET_WORKBUF_ADDR failed! (%d)", mret);
        goto _Exit;
    }
    if ((mpWorkingBuffer != NULL) && (mWarpWorkBufSize != warpWorkBufSize))
    {
        free(mpWorkingBuffer);
        mpWorkingBuffer = NULL;
    }
    if (mpWorkingBuffer == NULL)
    {
        mpWorkingBuffer = (MUINT8*) malloc(warpWorkBufSize);
        if (mpWorkingBuffer == NULL)
        {
            LOG_ERR("Warp working buffer allocation failed!");
            goto _Exit;
        }
    }
    mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_SET_WORKBUF_ADDR, &mpWorkingBuffer, NULL);
    if (mret != S_WARP_OK)
    {
        LOG_ERR("MTKWarp WARP_FEATURE_SET_WORKBUF_ADDR failed! (%d)", mret);
        goto _Exit;
    }
    mWarpWorkBufSize = warpWorkBufSize;

    FUNCTION_LOG_END;
    return MTRUE;

_Exit:
    if (mpGpuWarp != NULL)
    {
        mpGpuWarp->WarpReset();
        mpGpuWarp->destroyInstance(mpGpuWarp);
    }
#ifdef _CPUWARP
    free(mpCpuWarpInputBuffer);
    free(mpCpuWarpOutputBuffer);
#endif
    for (MUINT8 i = 0; i < GPU_NODE_BUFFER_NUM; i++)
    {
        LOG_DBG("Freeing GPU Node output GraphicBuffer");
        mGfxRgbaBuffer[i]->unlock();
        mGfxRgbaBuffer[i] = NULL;
    }
    if (mpWorkingBuffer != NULL)
    {
        free(mpWorkingBuffer);
    }
    return MFALSE;
}

MBOOL
FeaturePipeGpuNode::
threadUninit()
{
    FUNCTION_LOG_START;

    if (mpGpuWarp != NULL)
    {
        mpGpuWarp->WarpReset();
        mpGpuWarp->destroyInstance(mpGpuWarp);
    }
#ifdef _CPUWARP
    free(mpCpuWarpInputBuffer);
    free(mpCpuWarpOutputBuffer);
#endif
    for (MUINT8 i = 0; i < GPU_NODE_BUFFER_NUM; i++)
    {
        LOG_DBG("Freeing GPU Node output GraphicBuffer");
        mGfxRgbaBuffer[i]->unlock();
        mGfxRgbaBuffer[i] = NULL;
    }
    if (mpWorkingBuffer != NULL)
    {
        free(mpWorkingBuffer);
    }

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeGpuNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeGpuNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeGpuNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeGpuNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    MUINT32 numReadyJobs = MIN(mqJobFullImg.size(),mqJobWarpMap.size());

    FeaturePipeNodeImgReqJob_s job;
    job.dataType = (NSCamNode::NodeDataTypes) data;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case P2A_TO_GPU:
        case SWITCHER_TO_GPU:
            mqJobFullImg.push(job);
            break;

        case VFBSW_TO_GPU:
        case EISSW_TO_GPU:
            mqJobWarpMap.push(job);
            break;
        default:
            LOG_AST(0, "Wrong data (%d)!", data);
    }

    MUINT32 newReadyJobs = MIN(mqJobFullImg.size(),mqJobWarpMap.size());
    if (newReadyJobs > numReadyJobs)
        triggerLoop();

    //FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeGpuNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;
    MRESULT mret;
    struct WarpImageExtInfo warp_info;
    GpuTuning gpu_tuning;
    warp_info.pTuningPara = &gpu_tuning;

    CPTLog(Event_FeaturePipe_GPUThread, CPTFlagStart);

    FeaturePipeNodeImgReqJob_s jobWarpMap;
    FeaturePipeNodeImgReqJob_s jobFullImg;
    {
        Mutex::Autolock lock(mLock);

        /* Get warp job */
        jobWarpMap = mqJobWarpMap.front();
        mqJobWarpMap.pop();

        /* Get full image job */
        jobFullImg = mqJobFullImg.front();
        mqJobFullImg.pop();
    }

    MUINT32 frameNo = jobWarpMap.mFrameNo;
    LOG_AST(frameNo == jobFullImg.mFrameNo, "Frame number does not match", 0);
    CAM_TRACE_FMT_BEGIN("GPUThread:%d", frameNo);

    /* Calculate GPU input/output width and height */
    mInputImageWidth = jobFullImg.imgReq.mBuffer->getImgSize().w;
    mInputImageHeight = jobFullImg.imgReq.mBuffer->getImgSize().h;
    if (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        reinterpret_cast<FeaturePipeEisNode*>(mpFeaturePipeObj->mpEisSwNode)->GetEisPlusTargetSize(mOutputImageWidth,mOutputImageHeight);
        LOG_DBG("FPipe_G:FNO(%u,%u),Eis(%u,%u,%u,%u)",frameNo,jobFullImg.mFrameNo,mInputImageWidth,mInputImageHeight,mOutputImageWidth,mOutputImageHeight);
    }
    else
    {
        mOutputImageWidth = mInputImageWidth;
        mOutputImageHeight = mInputImageHeight;
    }


    /* Copy full image into Graphic buffer */
#ifndef _CPUWARP
    CPTLog(Event_FeaturePipe_GPU_Memcpy, CPTFlagStart);
    CAM_TRACE_FMT_BEGIN("GPUMemcpy:%d", frameNo);

    char* gfxYuvBuf = NULL;
    char* gfxRgbaBuf = NULL;
#ifdef _WORKAROUND_GPU_FLASH
    mGfxYuvBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&gfxYuvBuf));
    //***************************src YV12********************************//
    DpBlitStream dpBlit;
    unsigned int src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    int plane_num = 3;
    src_addr_list[0] = (unsigned int)jobFullImg.imgReq.mBuffer->getBufVA(0);
    src_addr_list[1] = (unsigned int)jobFullImg.imgReq.mBuffer->getBufVA(1);
    src_addr_list[2] = (unsigned int)jobFullImg.imgReq.mBuffer->getBufVA(2);
    src_size_list[0] = jobFullImg.imgReq.mBuffer->getBufSizeInBytes(0);
    src_size_list[1] = jobFullImg.imgReq.mBuffer->getBufSizeInBytes(1);
    src_size_list[2] = jobFullImg.imgReq.mBuffer->getBufSizeInBytes(2);
    dpBlit.setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    dpBlit.setSrcConfig(jobFullImg.imgReq.mBuffer->getImgSize().w, jobFullImg.imgReq.mBuffer->getImgSize().h, FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_WIDTH / 2, DP_COLOR_YV12);
    //***************************dst RGBA8888****************************//
    unsigned int dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    plane_num = 1;
    dst_addr_list[0] = (unsigned int)gfxYuvBuf;
    dst_size_list[0] = FULL_IMAGE_MAX_WIDTH * jobFullImg.imgReq.mBuffer->getImgSize().h * 4;
    dpBlit.setDstBuffer((void **)dst_addr_list, dst_size_list, plane_num);
    dpBlit.setDstConfig(jobFullImg.imgReq.mBuffer->getImgSize().w, jobFullImg.imgReq.mBuffer->getImgSize().h, FULL_IMAGE_MAX_WIDTH * 4, 0, DP_COLOR_RGBA8888);
    dpBlit.setRotate(0);

    // set & add pipe to stream
    if (dpBlit.invalidate())  //trigger HW
    {
          LOG_ERR("[convertRGBA8888toYV12] FDstream invalidate failed");
          return false;
    }
    mGfxYuvBuffer->unlock();
#endif //_WORKAROUND_GPU_FLASH

    CAM_TRACE_FMT_END();
    CPTLog(Event_FeaturePipe_GPU_Memcpy, CPTFlagEnd);
#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpgpu", EnableOption, "0");
    if (EnableOption[0] == '1' || EnableOption[0] == '3')
#endif
    {
        if( gfxYuvBuf )
        {
            char szFileName[100];
            ::sprintf(szFileName, "/sdcard/gfxi_%dx%d_%04d.yuv", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT, frameNo);
            LOG_DBG("DUMP gfxi:%s",szFileName);
            saveBufToFile(szFileName, (MUINT8*) gfxYuvBuf, FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 3 / 2);
        }
    }
#else
    memcpy(mpCpuWarpInputBuffer, (void*) jobFullImg.imgReq.mBuffer->getBufVA(0), mInputImageWidth * mInputImageHeight * 3 / 2);
    {
        char szFileName[100];
        ::sprintf(szFileName, "/sdcard/gfxi_%dx%d_%04d.yuv", mInputImageWidth, mInputImageHeight, frameNo);
        saveBufToFile(szFileName, (MUINT8*) mpCpuWarpInputBuffer, mInputImageWidth * mInputImageHeight * 4);
    }
#endif

    /* Set per frame process info */
    warp_info.ClipWidth = mOutputImageWidth;
    warp_info.ClipHeight = mOutputImageHeight;
    warp_info.Width = mInputImageWidth;
    warp_info.Height = mInputImageHeight;
    warp_info.WarpMapSize[0][0] = const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getImgSize().w / 4;
    warp_info.WarpMapSize[0][1] = const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getImgSize().h / 2;
    warp_info.WarpMapAddr[0][0] = (MUINT32*) const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufVA(0);
    warp_info.WarpMapAddr[0][1] = (MUINT32*) (const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufVA(0) + const_cast<IImageBuffer*>(jobWarpMap.imgReq.mBuffer)->getBufSizeInBytes(0) / 2);

#ifdef _CPUWARP
    warp_info.SrcGraphicBuffer = (void*) mpCpuWarpInputBuffer;
    warp_info.DstGraphicBuffer = (void*) mpCpuWarpOutputBuffer;
#else

#ifndef _WORKAROUND_GPU_FLASH
    if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->getGraphicBuffer((jobFullImg.dataType == SWITCHER_TO_GPU)? P2A_TO_SWITCHER: jobFullImg.dataType, const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer), (sp<GraphicBuffer>**) &(warp_info.SrcGraphicBuffer)) )
    {
        LOG_ERR("Failed to get GB from ImageBuffer(%x)", const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer));
        return MFALSE;
    }
#else
    warp_info.SrcGraphicBuffer = (void*)&mGfxYuvBuffer;
#endif //_WORKAROUND_GPU_FLASH

    ImgRequest fullImgReq;
    if(!pBufHandler->dequeBuffer(GPU_TO_MDP, &fullImgReq))
    {
        LOG_AST(false, "dequeBuffer fail. Out of GPU_TO_MDP buffer", 0);
        return MFALSE;
    }
    if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->getGraphicBuffer(GPU_TO_MDP, const_cast<IImageBuffer*>(fullImgReq.mBuffer), (sp<GraphicBuffer>**) &(warp_info.DstGraphicBuffer)) )
    {
        LOG_ERR("Failed to get GB from ImageBuffer(%x)", const_cast<IImageBuffer*>(fullImgReq.mBuffer));
        return MFALSE;
    }
    if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(GPU_TO_MDP, &fullImgReq.mBuffer, mOutputImageWidth, mOutputImageHeight) )
    {
        LOG_ERR("EIS Resize full image buffer failed");
        return MFALSE;
    }
#endif  //_CPUWARP

    mret = mpGpuWarp->WarpFeatureCtrl(WARP_FEATURE_ADD_IMAGE, &warp_info, NULL);
    if (mret != S_WARP_OK)
    {
        LOG_ERR("MTKWarp WARP_FEATURE_ADD_IMAGE failed! (%d)", mret);
    }

    /* GPU Processing */
    LOG_DBG("WarpMain: feature(0x%08x),input(%d,%d),output(%d,%d),warp map(%d,%d),addr(0x%x,0x%x)",warp_info.Features,warp_info.Width,warp_info.Height, warp_info.ClipWidth, warp_info.ClipHeight, warp_info.WarpMapSize[0][0], warp_info.WarpMapSize[0][1], warp_info.WarpMapAddr[0][0], warp_info.WarpMapAddr[0][1]);

    CPTLog(Event_FeaturePipe_GPU_Warp, CPTFlagStart);
    CAM_TRACE_FMT_BEGIN("GPUWarp:%d", frameNo);

    (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->unlock();
    mret = mpGpuWarp->WarpMain();
    (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&gfxRgbaBuf));

    CAM_TRACE_FMT_END();
    CPTLog(Event_FeaturePipe_GPU_Warp, CPTFlagEnd);

    if (mret != S_WARP_OK)
    {
        LOG_ERR("MTKWarp WarpMain failed! (%d)", mret);
    }

#ifndef _DEBUG_DUMP
    property_get("camera.featurepipe.dumpgpu", EnableOption, "0");
    if (EnableOption[0] == '1')
#endif
    {
        (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&gfxRgbaBuf));
        char szFileName[100];
        ::sprintf(szFileName, "/sdcard/gfxo_%dx%d_%04d.rgb", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT, frameNo);
        saveBufToFile(szFileName, (MUINT8*) gfxRgbaBuf, FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 4);
        (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->unlock();
    }

    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        /* Return buffer to previous node */
        LOG_DBG("Returning VFB node warp map buffer %d", jobWarpMap.mFrameNo);
        handleReturnBuffer(VFBSW_TO_GPU, (MUINTPTR) &jobWarpMap.imgReq, jobWarpMap.mFrameNo);
    }
    else if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        ImgRequest fullImgReq;

        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpgpu", EnableOption, "0");
        if(EnableOption[0] == '2' || EnableOption[0] == '3')
        {
            (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->lock(GRALLOC_USAGE_SW_WRITE_OFTEN | GRALLOC_USAGE_SW_READ_OFTEN, (void**)(&gfxRgbaBuf));

            if(!pBufHandler->dequeBuffer(8001, &fullImgReq))
            {
                LOG_ERR("Out of EIS gfxo buffer");
                return MFALSE;
            }

            LOG_DBG("EIS Resize full image buffer to %dx%d", mOutputImageWidth, mOutputImageHeight);
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(8001, &fullImgReq.mBuffer, mOutputImageWidth, mOutputImageHeight) )
            {
                LOG_ERR("EIS Resize full image buffer failed");
                return MFALSE;
            }

            char szFileName[100];
            //::sprintf(szFileName, "/sdcard/gfxo_%dx%d_%04d.rgb", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT, frameNo);
            //saveBufToFile(szFileName, (MUINT8*) gfxRgbaBuf, FULL_IMAGE_MAX_WIDTH * FULL_IMAGE_MAX_HEIGHT * 4);
            convertRGBA8888toYV12((MINTPTR)gfxRgbaBuf, mOutputImageWidth, mOutputImageHeight, const_cast<IImageBuffer*>(fullImgReq.mBuffer));
            ::sprintf(szFileName, "/sdcard/gfxo_%dx%d_%04d.yuv", mOutputImageWidth, mOutputImageHeight, frameNo);
            LOG_DBG("DUMP+EIS gfxo:%s",szFileName);
            const_cast<IImageBuffer*>(fullImgReq.mBuffer)->syncCache(eCACHECTRL_INVALID);
            const_cast<IImageBuffer*>(fullImgReq.mBuffer)->saveToFile(szFileName);
            pBufHandler->enqueBuffer(8001,fullImgReq.mBuffer);

            (*((sp<GraphicBuffer>*) warp_info.DstGraphicBuffer))->unlock();
        }

        /* Return buffer to previous node */
        LOG_DBG("Returning EIS node warp map buffer %d", jobWarpMap.mFrameNo);
        handleReturnBuffer(EISSW_TO_GPU, (MUINTPTR) &jobWarpMap.imgReq, jobWarpMap.mFrameNo);
    }

#ifndef _CPUWARP
    //mGfxRgbaBuffer->unlock();
#endif

    LOG_DBG("Returning P2A node full image buffer %d", jobFullImg.mFrameNo);
    handleReturnBuffer(jobFullImg.dataType, (MUINTPTR) &jobFullImg.imgReq, jobFullImg.mFrameNo);

    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask) || FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        /* Post buffer to next node */
        LOG_DBG("Posting GPU->MDP node GB of frame %d", frameNo);
        handlePostBuffer(GPU_TO_MDP, (MUINTPTR) &fullImgReq, frameNo);
    }

    CPTLog(Event_FeaturePipe_GPUThread, CPTFlagEnd);
    CAM_TRACE_FMT_END();

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeGpuNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    LOG_DBG("Recycling GPU GB node buffer %d", ext);
    pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeGpuNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[MDP:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[MDP:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[MDP:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[MDP:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[MDP:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[MDP:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeMdpNode*
FeaturePipeMdpNode::createInstance(MUINT32 featureOnMask, FeaturePipe *pFeaturePipeObj)
{
    return new FeaturePipeMdpNode("FeaturePipeMdpNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, pFeaturePipeObj);
}
MVOID
FeaturePipeMdpNode::destroyInstance()
{
    delete this;
}
FeaturePipeMdpNode::FeaturePipeMdpNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        FeaturePipe *pFeaturePipeObj)
    : FeaturePipeNode(name, type, policy, priority, featureOnMask, 0, pFeaturePipeObj),
      mImageWidth(0),
      mImageHeight(0)
{
    if ((FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)) ||
        (FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask)))
    {
        addDataSupport(ENDPOINT_SRC, GPU_TO_MDP);
    }
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_DST, MDP_TO_SWITCHER);
    }
    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        addDataSupport(ENDPOINT_SRC, P2A_TO_MDP_QPARAMS);
    }
}
MBOOL
FeaturePipeMdpNode::onInit()
{
    FUNCTION_LOG_START;

    /* Allocate buffer for next node */
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        pBufHandler = getBufferHandler(MDP_TO_SWITCHER);
        if (pBufHandler == NULL)
        {
            LOG_ERR("ICamBufHandler failed!");
            goto _Exit;
        }

        LOG_DBG("Allocate full image buffer %dx%d", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT);
        AllocInfo allocinfo(FULL_IMAGE_MAX_WIDTH,
                            FULL_IMAGE_MAX_HEIGHT,
                            eImgFmt_YV12,
                            eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
        for (MUINT8 i = 0; i < MDP_NODE_BUFFER_NUM; i++)
        {
            if( !pBufHandler->requestBuffer(MDP_TO_SWITCHER, allocinfo) )
                return MFALSE;
        }
    }

    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        mpStream = new DpIspStream(DpIspStream::ISP_ZSD_STREAM);
        if(mpStream == NULL)
        {
            LOG_ERR("New DpIspStream failed");
            goto _Exit;
        }

        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpmdp", EnableOption, "0");
        if(EnableOption[0] == '1' || EnableOption[0] == '3')
        {
            pBufHandler = getBufferHandler(8001);
            if (pBufHandler == NULL)
            {
                LOG_ERR("EIS mdpi pBufHandler failed!");
                goto _Exit;
            }

            LOG_DBG("EIS mdpi buffer(%dx%d)", FULL_IMAGE_MAX_WIDTH, FULL_IMAGE_MAX_HEIGHT);
            AllocInfo allocinfo(FULL_IMAGE_MAX_WIDTH,
                                FULL_IMAGE_MAX_HEIGHT,
                                eImgFmt_YV12,
                                eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN | eBUFFER_USAGE_SW_WRITE_RARELY);
            if( !pBufHandler->requestBuffer(8001, allocinfo) )
            {
                LOG_ERR("EIS mdpi buffer fail");
                return MFALSE;
            }
        }
    }

    sem_init(&mModuleSem, 0, 0);

    FUNCTION_LOG_END;
    return true;

_Exit:
    return false;
}

MBOOL
FeaturePipeMdpNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;

    if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        delete mpStream;
    }
    sem_destroy(&mModuleSem);

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeMdpNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeMdpNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeMdpNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeMdpNode::
dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    sem_wait(&mModuleSem);
    rParams = mqPostProcQParams.front();
    mqPostProcQParams.pop();
    FUNCTION_LOG_END;
    return ret;
}

MBOOL
FeaturePipeMdpNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    MUINT32 numReadyJobs = MIN(mqFullImgJob.size(),mqQParamsJob.size());

    FeaturePipeNodeImgReqJob_s job;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case P2A_TO_MDP_QPARAMS:
            mqQParamsJob.push(*(reinterpret_cast<QParams*>(buf)));
            break;
        case GPU_TO_MDP:
            mqFullImgJob.push(job);
            break;

        default:
            LOG_AST(0, "Wrong data (%d)!", data);
    }

    if (!FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask)
        && FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        //Must wait for QParams when EIS-only
        MUINT32 newReadyJobs = MIN(mqFullImgJob.size(),mqQParamsJob.size());
        if (newReadyJobs > numReadyJobs)
            triggerLoop();
    }
    else{
        triggerLoop();
    }

    //FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeMdpNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;
    MRESULT mret;

    /* Get job */
    FeaturePipeNodeImgReqJob_s jobFullImg;
    {
        Mutex::Autolock lock(mLock);
        jobFullImg = mqFullImgJob.front();
        mqFullImgJob.pop();
    }


    MUINT32 frameNo = jobFullImg.mFrameNo;
    CAM_TRACE_FMT_BEGIN("MDPThread:%d", frameNo);

    ImgRequest fullImgReq;
    if (FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        IImageBuffer *pImageBuffer = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer);

        /* Prepare buffer for next node */
        if(!pBufHandler->dequeBuffer(MDP_TO_SWITCHER, &fullImgReq))
        {
            LOG_AST(false, "Out of MDP_TO_SWITCHER buffer", 0);
            return MFALSE;
        }
        if ((fullImgReq.mBuffer->getImgSize().w != pImageBuffer->getImgSize().w) ||
                (fullImgReq.mBuffer->getImgSize().h != pImageBuffer->getImgSize().h))
        {
            LOG_DBG("Resize full image buffer to %dx%d", pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h);
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(MDP_TO_SWITCHER, &fullImgReq.mBuffer, pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h) )
            {
                LOG_ERR("Resize full image buffer failed");
                return MFALSE;
            }
        }

        /* MDP format convertion from RGBA8888 to YV12 */
        convertRGBA8888(pImageBuffer, const_cast<IImageBuffer*>(fullImgReq.mBuffer));
#ifndef _DEBUG_DUMP
        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpgpu", EnableOption, "0");
        if (EnableOption[0] == '1')
#endif
        {
            char szFileName[100];
            ::sprintf(szFileName, "/sdcard/mdpo_%dx%d_%04d.yuv", pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h, frameNo);
            const_cast<IImageBuffer*>(fullImgReq.mBuffer)->saveToFile(szFileName);
        }
    }
    else if(FEATURE_MASK_IS_EIS_ENABLED(mFeatureOnMask))
    {
        QParams OutQParams;

        {
            Mutex::Autolock lock(mLock);

            /*Get output QParams*/
            OutQParams = mqQParamsJob.front();
            mqQParamsJob.pop();
        }

        IImageBuffer *pImageBuffer = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer);

        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpmdp", EnableOption, "0");
        if(EnableOption[0] == '1' || EnableOption[0] == '3')
        {
            ImgRequest fullImgReq;

            if(!pBufHandler->dequeBuffer(8001, &fullImgReq))
            {
                LOG_ERR("Out of EIS mdpi buffer");
                return MFALSE;
            }

            LOG_DBG("EIS Resize full image buffer to %dx%d",pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h);
            if( !reinterpret_cast<FeaturePipeBufHandler*>(pBufHandler)->resizeBuffer(8001, &fullImgReq.mBuffer,pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h))
            {
                LOG_ERR("EIS Resize full image buffer failed");
                return MFALSE;
            }

            char szFileName[100];
            convertRGBA8888toYV12(pImageBuffer->getBufVA(0), pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h, const_cast<IImageBuffer*>(fullImgReq.mBuffer));
            ::sprintf(szFileName, "/sdcard/mdpi_%dx%d_%04d.yuv", pImageBuffer->getImgSize().w, pImageBuffer->getImgSize().h, frameNo);
            LOG_DBG("DUMP+EIS mdpi:%s",szFileName);
            const_cast<IImageBuffer*>(fullImgReq.mBuffer)->syncCache(eCACHECTRL_INVALID);
            const_cast<IImageBuffer*>(fullImgReq.mBuffer)->saveToFile(szFileName);
            pBufHandler->enqueBuffer(8001,fullImgReq.mBuffer);
        }

        /* Copy result to PostProc output buffer */
        if(mpStream->setSrcConfig(DP_COLOR_RGBA8888,
                                  pImageBuffer->getImgSize().w,
                                  pImageBuffer->getImgSize().h,
                                  FULL_IMAGE_MAX_WIDTH * 4,
                                  false) < 0)
        {
            LOG_ERR("DpIspStream->setSrcConfig failed");
            return MFALSE;
        }

        MUINT32 va[3] = {(MUINT32) pImageBuffer->getBufVA(0),0,0};
        MUINT32 mva[3] = {(MUINT32) pImageBuffer->getBufPA(0),0,0};
        MUINT32 size[3] = {FULL_IMAGE_MAX_WIDTH * pImageBuffer->getImgSize().h * 4,0,0};
        if (mpStream->queueSrcBuffer((void**)va,
                                     mva,
                                     size,
                                     1) < 0)
        {
            LOG_ERR("DpIspStream->queueSrcBuffer failed");
            return MFALSE;
        }

        if(mpStream->setSrcCrop(0,0,0,0,pImageBuffer->getImgSize().w,pImageBuffer->getImgSize().h) < 0)
        {
             LOG_ERR("DpIspStream->setSrcCrop failed");
             return MFALSE;
        }

        for(int i=0; i < OutQParams.mvOut.size(); i++)
        {
            DpColorFormat dp_fmt;
            switch( NSCam::EImageFormat fmt = (NSCam::EImageFormat) OutQParams.mvOut[i].mBuffer->getImgFormat() )
            {
        #define FMT_SUPPORT( fmt, dpfmt ) \
                case fmt: \
                dp_fmt = dpfmt; \
                break;
                FMT_SUPPORT(eImgFmt_YUY2   , DP_COLOR_YUYV)
                FMT_SUPPORT(eImgFmt_UYVY   , DP_COLOR_UYVY)
                FMT_SUPPORT(eImgFmt_YVYU   , DP_COLOR_YVYU)
                FMT_SUPPORT(eImgFmt_VYUY   , DP_COLOR_VYUY)
                FMT_SUPPORT(eImgFmt_NV16   , DP_COLOR_NV16)
                FMT_SUPPORT(eImgFmt_NV61   , DP_COLOR_NV61)
                FMT_SUPPORT(eImgFmt_NV21   , DP_COLOR_NV21)
                FMT_SUPPORT(eImgFmt_NV12   , DP_COLOR_NV12)
                FMT_SUPPORT(eImgFmt_YV16   , DP_COLOR_YV16)
                FMT_SUPPORT(eImgFmt_I422   , DP_COLOR_I422)
                FMT_SUPPORT(eImgFmt_YV12   , DP_COLOR_YV12)
                FMT_SUPPORT(eImgFmt_I420   , DP_COLOR_I420)
                FMT_SUPPORT(eImgFmt_Y800   , DP_COLOR_GREY)
                FMT_SUPPORT(eImgFmt_RGB565 , DP_COLOR_RGB565)
                FMT_SUPPORT(eImgFmt_RGB888 , DP_COLOR_RGB888)
                FMT_SUPPORT(eImgFmt_ARGB888, DP_COLOR_ARGB8888)
                // not supported
                default:
                    LOG_ERR(" fmt(0x%x) not support in DP", fmt);
                    return MFALSE;
                    break;
          #undef FMT_SUPPORT
            }

            //LOG_DBG("mvOut:idx(%d),SZ(%u,%u),stride(%u,%u)",i,OutQParams.mvOut[i].mBuffer->getImgSize().w,
            //                                                OutQParams.mvOut[i].mBuffer->getImgSize().h,
            //                                                OutQParams.mvOut[i].mBuffer->getBufStridesInBytes(0),
            //                                                OutQParams.mvOut[i].mBuffer->getBufStridesInBytes(1));

            if(mpStream->setDstConfig(i, //port
                                      OutQParams.mvOut[i].mBuffer->getImgSize().w,
                                      OutQParams.mvOut[i].mBuffer->getImgSize().h,
                                      OutQParams.mvOut[i].mBuffer->getBufStridesInBytes(0),
                                      OutQParams.mvOut[i].mBuffer->getBufStridesInBytes(1),
                                      dp_fmt,
                                      DP_PROFILE_FULL_BT601,
                                      eInterlace_None,  //default
                                      NULL, //default
                                      false) < 0)
            {
                LOG_ERR("DpIspStream->setDstConfig failed");
                return MFALSE;
            }

            MUINT32 va[3]   = {0,0,0};
            MUINT32 size[3] = {0,0,0};
            MUINT32 pa[3]   = {0,0,0};

            for(int j=0; j < OutQParams.mvOut[i].mBuffer->getPlaneCount(); j++)
            {
                va[j]   = OutQParams.mvOut[i].mBuffer->getBufVA(j);
                pa[j]   = OutQParams.mvOut[i].mBuffer->getBufPA(j);
                size[j] = OutQParams.mvOut[i].mBuffer->getBufSizeInBytes(j);
            }

            if(mpStream->queueDstBuffer(i, //port
                                        (void**)va,
                                        pa,
                                        size,
                                        OutQParams.mvOut[i].mBuffer->getPlaneCount()) < 0)
            {
                LOG_ERR("queueDstBuffer failed");
                return MFALSE;
            }
        }

        if(mpStream->startStream() < 0)
        {
            LOG_ERR("startStream failed");
            return MFALSE;
        }

        if(mpStream->dequeueSrcBuffer() < 0)
        {
            LOG_ERR("dequeueSrcBuffer failed");
            return MFALSE;
        }

        for(int i=0; i < OutQParams.mvOut.size(); i++)
        {
            MUINT32 va[3] = {0,0,0};

            if(mpStream->dequeueDstBuffer(i,(void**)va) < 0)
            {
                LOG_ERR("dequeueDstBuffer failed");
                return MFALSE;
            }
        }

        if(mpStream->stopStream() < 0)
        {
            LOG_ERR("stopStream failed");
            return MFALSE;
        }

        if(mpStream->dequeueFrameEnd() < 0)
        {
            LOG_ERR("dequeueFrameEnd failed");
            return MFALSE;
        }

        // FLUSH
        //for(int i=0; i < OutQParams.mvOut.size(); i++)
        //{
        //    OutQParams.mvOut[i].mBuffer->syncCache(eCACHECTRL_INVALID);
        //}

        // dump
        {
            char debugFlag[PROPERTY_VALUE_MAX] = {'\0'};
            property_get("camera.featurepipe.dumpmdp", debugFlag, "0");
            if(debugFlag[0] == '2' || debugFlag[0] == '3')
            {
                for(int i=0; i < OutQParams.mvOut.size(); i++)
                {
                    char szFileName[100];
                    ::sprintf(szFileName, "/sdcard/mdpo_%dx%d_%04d.yuv",OutQParams.mvOut[i].mBuffer->getImgSize().w,OutQParams.mvOut[i].mBuffer->getImgSize().h,frameNo);
                    LOG_DBG("MDP+EIS mdpo:%s",szFileName);
                    OutQParams.mvOut[i].mBuffer->saveToFile(szFileName);
                }
            }
        }

        OutQParams.mDequeSuccess = MTRUE;
        if(OutQParams.mpfnCallback != NULL)
        {
            LOG_DBG("Callback of frame %d", getMagicNo(OutQParams));
            mpFeaturePipeObj->performCallback(OutQParams);
        }
        else
        {
            Mutex::Autolock lock(mLock);
            mqPostProcQParams.push(OutQParams);
            sem_post(&mModuleSem);
        }
    }

    LOG_DBG("Returning GPU node GB %d", jobFullImg.mFrameNo);
    handleReturnBuffer(GPU_TO_MDP, (MUINTPTR) &jobFullImg.imgReq, jobFullImg.mFrameNo);

    if(FEATURE_MASK_IS_VFB_ENABLED(mFeatureOnMask))
    {
        /* Post buffer to next node */
        LOG_DBG("Posting MDP->SWITCHER node full image of frame %d", frameNo);
        handlePostBuffer(MDP_TO_SWITCHER, (MUINTPTR) &fullImgReq, frameNo);
    }

    CAM_TRACE_FMT_END();

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeMdpNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    LOG_DBG("Recycling MDP full image node buffer %d", ext);
    pBufHandler->enqueBuffer(data, ((ImgRequest*)buf)->mBuffer);
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeMdpNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}
MINT32
FeaturePipeMdpNode::
convertRGBA8888(IImageBuffer* srcIImageBuffer, IImageBuffer* dstIImageBuffer)
{
    bool ret = true;
    FUNCTION_LOG_START;

    //***************************src RGBA8888****************************//
    unsigned int src_addr_list[3] = {0, 0, 0};
    unsigned int src_mva_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    int plane_num = 1;
    src_addr_list[0] = (unsigned int)srcIImageBuffer->getBufVA(0);
    src_mva_addr_list[0] = (unsigned int)srcIImageBuffer->getBufPA(0);
    src_size_list[0] = FULL_IMAGE_MAX_WIDTH * srcIImageBuffer->getImgSize().h * 4;

    mDpBlit.setSrcBuffer((void **)src_addr_list, (void **)src_mva_addr_list, src_size_list, plane_num);
    mDpBlit.setSrcConfig(srcIImageBuffer->getImgSize().w, srcIImageBuffer->getImgSize().h, FULL_IMAGE_MAX_WIDTH * 4, 0, DP_COLOR_RGBA8888, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);

    //***************************dst YV12********************************//
    unsigned int dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_mva_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    plane_num = 3;
    dst_addr_list[0] = (unsigned int)dstIImageBuffer->getBufVA(0);
    dst_addr_list[1] = (unsigned int)dstIImageBuffer->getBufVA(1);
    dst_addr_list[2] = (unsigned int)dstIImageBuffer->getBufVA(2);
    dst_mva_addr_list[0] = (unsigned int)dstIImageBuffer->getBufPA(0);
    dst_mva_addr_list[1] = (unsigned int)dstIImageBuffer->getBufPA(1);
    dst_mva_addr_list[2] = (unsigned int)dstIImageBuffer->getBufPA(2);

    dst_size_list[0] = dstIImageBuffer->getBufSizeInBytes(0);
    dst_size_list[1] = dstIImageBuffer->getBufSizeInBytes(1);
    dst_size_list[2] = dstIImageBuffer->getBufSizeInBytes(2);
    mDpBlit.setDstBuffer((void**)dst_addr_list, (void**)dst_mva_addr_list, dst_size_list, plane_num);
    mDpBlit.setDstConfig(dstIImageBuffer->getImgSize().w, dstIImageBuffer->getImgSize().h, dstIImageBuffer->getBufStridesInBytes(0), dstIImageBuffer->getBufStridesInBytes(1), DP_COLOR_YV12, DP_PROFILE_FULL_BT601, eInterlace_None, 0, DP_SECURE_NONE, false);
    mDpBlit.setRotate(0);

    // set & add pipe to stream
    if (mDpBlit.invalidate())  //trigger HW
    {
          LOG_ERR("[convertRGBA8888toYV12] FDstream invalidate failed");
          return false;
    }
    FUNCTION_LOG_END;
    return ret;
}

/******************************************************************************
 *
 ******************************************************************************/
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_DBG
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        CAM_LOGV("[P2B:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGD("[P2B:%s] " fmt, __func__, ##arg)
#define LOG_DBG(fmt, arg...)        CAM_LOGI("[P2B:%s] " fmt, __func__, ##arg)
#define LOG_WRN(fmt, arg...)        CAM_LOGW("[P2B:%s] " fmt, __func__, ##arg)
#define LOG_ERR(fmt, arg...)        CAM_LOGE("[P2B:%s] " fmt, __func__, ##arg)
#define LOG_AST(cond, fmt, arg...)  do{ if(!cond) CAM_LOGA("[P2B:%s] " fmt, __func__, ##arg); } while(0)
FeaturePipeP2bNode*
FeaturePipeP2bNode::createInstance(MUINT32 featureOnMask, MUINT32 mOpenedSensorIndex, FeaturePipe *pFeaturePipeObj)
{
    LOG_AST(FEATURE_MASK_IS_VFB_ENABLED(featureOnMask), "Create P2B node while vFB is off", 0);
    return new FeaturePipeP2bNode("FeaturePipeP2bNode", SingleTrigger, SCHED_POLICY, SCHED_PRIORITY, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj);
}
MVOID
FeaturePipeP2bNode::destroyInstance()
{
    delete this;
}
FeaturePipeP2bNode::FeaturePipeP2bNode(
        const char* name,
        eThreadNodeType type,
        int policy,
        int priority,
        MUINT32 featureOnMask,
        MUINT32 mOpenedSensorIndex,
        FeaturePipe *pFeaturePipeObj
        )
        : FeaturePipeNode(name, type, policy, priority, featureOnMask, mOpenedSensorIndex, pFeaturePipeObj)
{
    addDataSupport(ENDPOINT_SRC, P2A_TO_P2B);
    addDataSupport(ENDPOINT_SRC, VFBSW_TO_P2B_DSIMG);
    addDataSupport(ENDPOINT_SRC, VFBSW_TO_P2B_ALPCL);
    addDataSupport(ENDPOINT_SRC, VFBSW_TO_P2B_ALPNR);
    addDataSupport(ENDPOINT_SRC, VFBSW_TO_P2B_PCA);
    addDataSupport(ENDPOINT_SRC, SWITCHER_TO_P2B);
}
MBOOL
FeaturePipeP2bNode::onInit()
{
    FUNCTION_LOG_START;
    MRESULT mret;

    /* Create IFeatureStream */
    mpIFeatureStream = NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance("FeaturePipe_VfbP2B", EFeatureStreamTag_vFB_FB, mOpenedSensorIndex);
    if (mpIFeatureStream == NULL)
    {
        LOG_ERR("IFeatureStream create instance failed!");
        goto _Exit;
    }
    mpIFeatureStream->init();

    mp3AHal = NS3A::IHal3A::createInstance(NS3A::IHal3A::E_Camera_1, mOpenedSensorIndex, "FeaturePipe_VfbP2B");
    if (!mp3AHal)
    {
        LOG_ERR("IHal3A::createInstance failed!");
        goto _Exit;
    }

    sem_init(&mModuleSem, 0, 0);

    FUNCTION_LOG_END;
    return true;

_Exit:
    if (mpIFeatureStream != NULL)
    {
        mpIFeatureStream->destroyInstance("FeaturePipe_VfbP2B");
    }
    if (mp3AHal != NULL)
    {
        mp3AHal->destroyInstance("FeaturePipe_VfbP2B");
    }
    return false;
}

MBOOL
FeaturePipeP2bNode::
onUninit()
{
    Mutex::Autolock autoLock(mUninitMtx);
    FUNCTION_LOG_START;

    mpIFeatureStream->uninit();
    mpIFeatureStream->destroyInstance("FeaturePipe_VfbP2B");
    mp3AHal->destroyInstance("FeaturePipe_VfbP2B");
    sem_destroy(&mModuleSem);

    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeP2bNode::
onStart()
{
    FUNCTION_LOG_START;
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2bNode::
onStop()
{
    FUNCTION_LOG_START;
    MBOOL ret = syncWithThread(); // wait for jobs done
    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2bNode::
onNotify(MUINT32 const datamsg, MUINT32 const ext1, MUINT32 const ext2)
{
    FUNCTION_LOG_START;
    LOG_DBG("datamsg(0x%x), ext1(0x%x), ext2(0x%x)", datamsg, ext1, ext2);
    FUNCTION_LOG_END;
    return MTRUE;
}

MBOOL
FeaturePipeP2bNode::
dequeWrapper(QParams& rParams, MINT64 i8TimeoutNs)
{
    FUNCTION_LOG_START;
    MBOOL ret = MTRUE;

    sem_wait(&mModuleSem);

    Mutex::Autolock lock(mLock);
    rParams = qCallerQParams.front();
    qCallerQParams.pop();

    FUNCTION_LOG_END;
    return ret;
}
MVOID
FeaturePipeP2bNode::
p2bCbFunc(QParams& rParams)
{
    FUNCTION_LOG_START;
    CPTLog(Event_FeaturePipe_P2B_FeatureStream, CPTFlagEnd);
    CAM_TRACE_FMT_BEGIN("P2BCb:%d", getMagicNo(rParams));

    LOG_DBG("Callback of frame %d", getMagicNo(rParams));
    FeaturePipeP2bNode* pP2BNodeObj = (FeaturePipeP2bNode*)(rParams.mpCookie);

    pP2BNodeObj->handleP2Done(rParams);

    Mutex::Autolock lock(pP2BNodeObj->mLock);

    MBOOL dequeSuccess = rParams.mDequeSuccess;
    rParams = pP2BNodeObj->mqCallbackQParams.front();
    pP2BNodeObj->mqCallbackQParams.pop();
    rParams.mDequeSuccess = dequeSuccess;

#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpp2b", EnableOption, "0");
    if (EnableOption[0] == '1')
#endif
    {
        for (int i=0; i < rParams.mvOut.size(); i++)
        {
            char szFileName[100];
            ::sprintf(szFileName, "/sdcard/p2bo_%dx%d_%04d.yuv", rParams.mvOut[i].mBuffer->getImgSize().w, rParams.mvOut[i].mBuffer->getImgSize().h, getMagicNo(rParams));
            rParams.mvOut[i].mBuffer->saveToFile(szFileName);
        }
    }

    pP2BNodeObj->mpFeaturePipeObj->performCallback(rParams);

    CAM_TRACE_FMT_END();
    FUNCTION_LOG_END;
}
MBOOL
FeaturePipeP2bNode::
handleP2Done(QParams& rParams)
{
    for(MUINT32 i = 0; i < rParams.mvIn.size(); i++)
    {
        ImgRequest imgReq(rParams.mvIn[i].mBuffer);
        NSCamNode::NodeDataTypes dType;
        switch (rParams.mvIn[i].mPortID.index)
        {
            case NSImageio::NSIspio::EPortIndex_VIPI:
                dType = SWITCHER_TO_P2B;
                break;
            case NSImageio::NSIspio::EPortIndex_IMGI:
                dType = VFBSW_TO_P2B_DSIMG;
                break;
            case NSImageio::NSIspio::EPortIndex_UFDI:
                dType = VFBSW_TO_P2B_ALPNR;
                break;
            case NSImageio::NSIspio::EPortIndex_LCEI:
                dType = VFBSW_TO_P2B_ALPCL;
                break;

            default:
                LOG_AST(0, "Wrong port (%d)!", rParams.mvIn[i].mPortID.index);
        }

        handleReturnBuffer(dType, (MUINTPTR) &imgReq, getMagicNo(rParams));
    }

    return MTRUE;
}
MBOOL
FeaturePipeP2bNode::
onPostBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
#define MIN(a,b) (((a) < (b))? (a): (b))
    //FUNCTION_LOG_START;
    LOG_DBG("data(%d), buf(0x%x), ext(0x%x)", data, buf, ext);
    Mutex::Autolock lock(mLock);

    MUINT32 numReadyJobs = MIN(MIN(mqFullImgJob.size(),mqDsImgJob.size()),mqQParamsJob.size());

    FeaturePipeNodeImgReqJob_s job;
    job.dataType = (NSCamNode::NodeDataTypes) data;
    job.imgReq = *(ImgRequest*) buf;
    job.mFrameNo = ext;
    switch (data)
    {
        case P2A_TO_P2B:
            mqQParamsJob.push(*(reinterpret_cast<QParams*>(buf)));
            break;

        case VFBSW_TO_P2B_DSIMG:
            mqDsImgJob.push(job);
            break;
        case VFBSW_TO_P2B_ALPCL:
            mqAlphaCLJob.push(job);
            break;
        case VFBSW_TO_P2B_ALPNR:
            mqAlphaNRJob.push(job);
            break;
        case VFBSW_TO_P2B_PCA:
            mqPCAJob.push(job);
            break;
        case SWITCHER_TO_P2B:
            mqFullImgJob.push(job);
            break;

        default:
            LOG_AST(0, "Wrong data (%d)!", data);
    }

    MUINT32 newReadyJobs = MIN(MIN(mqFullImgJob.size(),mqDsImgJob.size()),mqQParamsJob.size());
    if (newReadyJobs > numReadyJobs)
        triggerLoop();

    //FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2bNode::
threadLoopUpdate()
{
    Mutex::Autolock autoLock(mUninitMtx);
static const NSCam::NSIoPipe::PortID VIPI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIPI, 0);
static const NSCam::NSIoPipe::PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const NSCam::NSIoPipe::PortID UFDI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_UFDI, 0);
static const NSCam::NSIoPipe::PortID LCEI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_LCEI, 0);
    FUNCTION_LOG_START;
    MRESULT mret;

    CPTLog(Event_FeaturePipe_P2BThread, CPTFlagStart);

    QParams OutQParams;
    FeaturePipeNodeImgReqJob_s jobFullImg;
    FeaturePipeNodeImgReqJob_s jobDsImg;
    FeaturePipeNodeImgReqJob_s jobAlphaCL;
    FeaturePipeNodeImgReqJob_s jobAlphaNR;
    FeaturePipeNodeImgReqJob_s jobPCA;
    {
        /* All jobs have to be de-queued in one place!! */
        Mutex::Autolock lock(mLock);
        if(!(mqQParamsJob.size() && mqFullImgJob.size() && mqDsImgJob.size() && mqAlphaNRJob.size() && mqAlphaCLJob.size() && mqPCAJob.size())){
                LOG_ERR("One or more of job queues are empty\n");
                return false;
        }

        /* Get output QParams */
        OutQParams = mqQParamsJob.front();
        mqQParamsJob.pop();

        /* Get full image */
        jobFullImg = mqFullImgJob.front();
        mqFullImgJob.pop();

        jobDsImg = mqDsImgJob.front();
        mqDsImgJob.pop();
        jobAlphaCL = mqAlphaCLJob.front();
        mqAlphaCLJob.pop();
        jobAlphaNR = mqAlphaNRJob.front();
        mqAlphaNRJob.pop();
        jobPCA = mqPCAJob.front();
        mqPCAJob.pop();
    }

    LOG_AST(jobFullImg.mFrameNo == getMagicNo(OutQParams), "Frame number does not match", 0);
    LOG_AST(jobDsImg.mFrameNo == getMagicNo(OutQParams), "Frame number does not match", 0);
    LOG_AST(jobAlphaCL.mFrameNo == getMagicNo(OutQParams), "Frame number does not match", 0);
    LOG_AST(jobAlphaNR.mFrameNo == getMagicNo(OutQParams), "Frame number does not match", 0);
    LOG_AST(jobPCA.mFrameNo == getMagicNo(OutQParams), "Frame number does not match", 0);
    CAM_TRACE_FMT_BEGIN("P2BThread:%d", getMagicNo(OutQParams));

    QParams mQParams;
    mQParams.mvIn.clear();
    mQParams.mvIn.reserve(4);
    mQParams.mvOut.clear();
    mQParams.mvOut.reserve(OutQParams.mvOut.size());
    mQParams.mFrameNo = getMagicNo(OutQParams);
    if (OutQParams.mvMagicNo.size())
        mQParams.mvMagicNo.push_back(OutQParams.mvMagicNo[0]);
    mQParams.mpPrivaData = OutQParams.mpPrivaData;
    if (OutQParams.mvPrivaData.size())
        mQParams.mvPrivaData.push_back(OutQParams.mvPrivaData[0]);
    /* PASS2 Processing */
    NSCam::NSIoPipe::NSPostProc::Input minput;
    /* VIPI */
    minput.mPortID = VIPI;
    minput.mBuffer = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer);
    minput.mCropRect.p_integral.x = 0;
    minput.mCropRect.p_integral.y = 0;
    minput.mCropRect.p_fractional.x = 0;
    minput.mCropRect.p_fractional.y = 0;
    minput.mCropRect.s.w = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer)->getImgSize().w;
    minput.mCropRect.s.h = const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer)->getImgSize().h;
    mQParams.mvIn.push_back(minput);
    {
#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpp2b", EnableOption, "0");
    if (EnableOption[0] == '1')
#endif
    {
            char szFileName[100];
            ::sprintf(szFileName, "/sdcard/p2bi_%dx%d_%04d.yuv", const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer)->getImgSize().w, const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer)->getImgSize().h, getMagicNo(OutQParams));
            const_cast<IImageBuffer*>(jobFullImg.imgReq.mBuffer)->saveToFile(szFileName);
    }
    }
    /* IMGI */
    LOG_DBG("Received VFB->P2B ds req(%x) mbuffer(%x)", &jobDsImg.imgReq, jobDsImg.imgReq.mBuffer);
    minput.mPortID = IMGI;
    minput.mBuffer = const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer);
    minput.mCropRect.p_integral.x = 0;
    minput.mCropRect.p_integral.y = 0;
    minput.mCropRect.p_fractional.x = 0;
    minput.mCropRect.p_fractional.y = 0;
    minput.mCropRect.s.w = const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->getImgSize().w;
    minput.mCropRect.s.h = const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->getImgSize().h;
    mQParams.mvIn.push_back(minput);
    {
#ifndef _DEBUG_DUMP
    char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.featurepipe.dumpp2b", EnableOption, "0");
    if (EnableOption[0] == '1')
#endif
    {
            char szFileName[100];
            ::sprintf(szFileName, "/sdcard/p2bi_%dx%d_%04d.yuv", const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->getImgSize().w, const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->getImgSize().h, getMagicNo(OutQParams));
            const_cast<IImageBuffer*>(jobDsImg.imgReq.mBuffer)->saveToFile(szFileName);
    }
    }
    /* LCEI */
    minput.mPortID = LCEI;
    minput.mBuffer = const_cast<IImageBuffer*>(jobAlphaCL.imgReq.mBuffer);
    minput.mCropRect.p_integral.x = 0;
    minput.mCropRect.p_integral.y = 0;
    minput.mCropRect.p_fractional.x = 0;
    minput.mCropRect.p_fractional.y = 0;
    minput.mCropRect.s.w = const_cast<IImageBuffer*>(jobAlphaCL.imgReq.mBuffer)->getImgSize().w;
    minput.mCropRect.s.h = const_cast<IImageBuffer*>(jobAlphaCL.imgReq.mBuffer)->getImgSize().h;
    mQParams.mvIn.push_back(minput);
    /* UFDI */
    //reinterpret_cast<FeaturePipeVfbNode*>(mpFeaturePipeObj->mpVfbSwNode)->setLCE(const_cast<IImageBuffer*>(jobAlphaNR.imgReq.mBuffer));
    minput.mPortID = UFDI;
    minput.mBuffer = const_cast<IImageBuffer*>(jobAlphaNR.imgReq.mBuffer);
    minput.mCropRect.p_integral.x = 0;
    minput.mCropRect.p_integral.y = 0;
    minput.mCropRect.p_fractional.x = 0;
    minput.mCropRect.p_fractional.y = 0;
    minput.mCropRect.s.w = const_cast<IImageBuffer*>(jobAlphaNR.imgReq.mBuffer)->getImgSize().w;
    minput.mCropRect.s.h = const_cast<IImageBuffer*>(jobAlphaNR.imgReq.mBuffer)->getImgSize().h;
    mQParams.mvIn.push_back(minput);
    /* PCA */
#ifdef MTK_CAM_VIDEO_FACEBEAUTY_SUPPORT
    reinterpret_cast<FeaturePipeVfbNode*>(mpFeaturePipeObj->mpVfbSwNode)->setPCA(const_cast<IImageBuffer*>(jobPCA.imgReq.mBuffer));
#endif
    for (int i=0; i < mQParams.mvIn.size(); i++)
    {
        LOG_DBG("[P2B sets mvIn (%d)] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d),magicnum(%d)",i,
                mQParams.mvIn[i].mBuffer->getImgFormat(),mQParams.mvIn[i].mBuffer->getImgSize().w,mQParams.mvIn[i].mBuffer->getImgSize().h,
                mQParams.mvIn[i].mPortID.type,mQParams.mvIn[i].mPortID.index,mQParams.mvIn[i].mPortID.inout,
                mQParams.mvIn[i].mBuffer->getBufVA(0),mQParams.mvIn[i].mBuffer->getBufPA(0),
                mQParams.mvIn[i].mBuffer->getBufSizeInBytes(0),mQParams.mvIn[i].mExtraParams.p2pxlID,getMagicNo(mQParams));
    }
    /* Output */
    for (int i=0; i < OutQParams.mvOut.size(); i++)
    {
        LOG_DBG("[P2B gets mvOut (%d)] imgfmt(%d),w(%d),h(%d),type(%d),index(%d),inout(%d),mem( va(0x%x)/pa(0x%x)/size(0x%x) ),p2pxlid(%d),magicnum(%d)",i,
                OutQParams.mvOut[i].mBuffer->getImgFormat(),OutQParams.mvOut[i].mBuffer->getImgSize().w,OutQParams.mvOut[i].mBuffer->getImgSize().h,
                OutQParams.mvOut[i].mPortID.type,OutQParams.mvOut[i].mPortID.index,OutQParams.mvOut[i].mPortID.inout,
                OutQParams.mvOut[i].mBuffer->getBufVA(0),OutQParams.mvOut[i].mBuffer->getBufPA(0),
                OutQParams.mvOut[i].mBuffer->getBufSizeInBytes(0),OutQParams.mvOut[i].mExtraParams.p2pxlID,getMagicNo(OutQParams));

        mQParams.mvOut.push_back(OutQParams.mvOut[i]);
    }
    for (int i=0; i < OutQParams.mvCropRsInfo.size(); i++)
    {
        MCrpRsInfo crop;
        crop.mGroupID = i + 1;
        if (i == 0)
        {
            /* Scale up IMGI to match VIPI */
            crop.mCropRect.s.w = jobDsImg.imgReq.mBuffer->getImgSize().w;
            crop.mCropRect.s.h = jobDsImg.imgReq.mBuffer->getImgSize().h;
        }
        else
        {
            crop.mCropRect.s.w = jobFullImg.imgReq.mBuffer->getImgSize().w;
            crop.mCropRect.s.h = jobFullImg.imgReq.mBuffer->getImgSize().h;
        }

        crop.mCropRect.p_integral.x = 0;
        crop.mCropRect.p_integral.y = 0;
        crop.mCropRect.p_fractional.x = 0;
        crop.mCropRect.p_fractional.y = 0;
        crop.mResizeDst.w = jobFullImg.imgReq.mBuffer->getImgSize().w;
        crop.mResizeDst.h = jobFullImg.imgReq.mBuffer->getImgSize().h;

        LOG_DBG("[P2B sets mCropRsInfo] groupid(%d),crop w(%d),h(%d),p_int(%d,%d),p_fra(%d,%d), resize w(%d),h(%d)",
                crop.mGroupID,crop.mCropRect.s.w,crop.mCropRect.s.h,crop.mCropRect.p_integral.x,crop.mCropRect.p_integral.y,
                crop.mCropRect.p_fractional.x,crop.mCropRect.p_fractional.y,crop.mResizeDst.w,crop.mResizeDst.h);

        mQParams.mvCropRsInfo.push_back(crop);
    }

#ifndef _WORKAROUND_P2B
    /* Invoke ISP tuning to load PCA and LCE */
    NS3A::ParamIspProfile_T _3A_profile(
            NS3A::EIspProfile_VFB_PostProc,
            getMagicNo(mQParams),
            MFALSE,
            NS3A::ParamIspProfile_T::EParamValidate_P2Only
            );
    mp3AHal->setIspProfile(_3A_profile);

    /* Enque */
    CPTLog(Event_FeaturePipe_P2B_FeatureStream, CPTFlagStart);

    if (OutQParams.mpfnCallback != NULL)
    {
        Mutex::Autolock lock(mLock);
        mqCallbackQParams.push(OutQParams);
        mQParams.mpCookie = this;
        mQParams.mpfnCallback = p2bCbFunc;
    }

    LOG_DBG("IFeatureStream enque (frame %d)", getMagicNo(mQParams));
    CAM_TRACE_FMT_BEGIN("P2BFStrm:%d", getMagicNo(mQParams));
    mpIFeatureStream->enque(mQParams);
    CAM_TRACE_FMT_END();

    if (OutQParams.mpfnCallback == NULL)
    {
        /* Deque */
        QParams outParams;
        mret = mpIFeatureStream->deque(outParams, -1);
        if (!mret)
        {
            LOG_ERR("IFeatureStream deque failed! (frame %d)", getMagicNo(mQParams));
            return MFALSE;
        }
        LOG_DBG("IFeatureStream deque succeeds! (frame %d)", getMagicNo(outParams));

        OutQParams.mDequeSuccess = outParams.mDequeSuccess;

        CPTLog(Event_FeaturePipe_P2B_FeatureStream, CPTFlagEnd);

#ifndef _DEBUG_DUMP
        char EnableOption[PROPERTY_VALUE_MAX] = {'\0'};
        property_get("camera.featurepipe.dumpp2b", EnableOption, "0");
        if (EnableOption[0] == '1')
#endif
        {
            for (int i=0; i < OutQParams.mvOut.size(); i++)
            {
                char szFileName[100];
                ::sprintf(szFileName, "/sdcard/p2bo_%dx%d_%04d.yuv", OutQParams.mvOut[i].mBuffer->getImgSize().w, OutQParams.mvOut[i].mBuffer->getImgSize().h, getMagicNo(OutQParams));
                OutQParams.mvOut[i].mBuffer->saveToFile(szFileName);
            }
        }
    }

#else
    DpBlitStream dpBlit;
    MUINT8* dstbufadr = (MUINT8*) mQParams.mvOut[0].mBuffer->getBufVA(0);

    //***************************src YV12****************************//
    unsigned int src_addr_list[3] = {0, 0, 0};
    unsigned int src_size_list[3] = {0, 0, 0};
    int plane_num = 3;
    src_addr_list[0] = (unsigned int) mQParams.mvIn[0].mBuffer->getBufVA(0),
    src_addr_list[1] = (unsigned int) mQParams.mvIn[0].mBuffer->getBufVA(1),
    src_addr_list[2] = (unsigned int) mQParams.mvIn[0].mBuffer->getBufVA(2),
    src_size_list[0] = mQParams.mvIn[0].mBuffer->getBufSizeInBytes(0);
    src_size_list[1] = mQParams.mvIn[0].mBuffer->getBufSizeInBytes(1);
    src_size_list[2] = mQParams.mvIn[0].mBuffer->getBufSizeInBytes(2);
    dpBlit.setSrcBuffer((void **)src_addr_list, src_size_list, plane_num);
    dpBlit.setSrcConfig(mQParams.mvIn[0].mBuffer->getImgSize().w, mQParams.mvIn[0].mBuffer->getImgSize().h, DP_COLOR_YV12);

    //***************************dst YV12********************************//
    int dst_ysize = mQParams.mvOut[0].mBuffer->getImgSize().w * mQParams.mvOut[0].mBuffer->getImgSize().h;
    int dst_uvsize;
    dst_uvsize = dst_ysize / 4;
    unsigned int dst_addr_list[3] = {0, 0, 0};
    unsigned int dst_size_list[3] = {0, 0, 0};
    plane_num = 3;
    dst_addr_list[0] = (unsigned int) mQParams.mvOut[0].mBuffer->getBufVA(0);
    dst_addr_list[1] = (unsigned int) mQParams.mvOut[0].mBuffer->getBufVA(1);
    dst_addr_list[2] = (unsigned int) mQParams.mvOut[0].mBuffer->getBufVA(2);
    dst_size_list[0] = mQParams.mvOut[0].mBuffer->getBufSizeInBytes(0);
    dst_size_list[1] = mQParams.mvOut[0].mBuffer->getBufSizeInBytes(1);
    dst_size_list[2] = mQParams.mvOut[0].mBuffer->getBufSizeInBytes(2);
    dpBlit.setDstBuffer((void**)dst_addr_list, dst_size_list, plane_num);
    dpBlit.setDstConfig(mQParams.mvOut[0].mBuffer->getImgSize().w, mQParams.mvOut[0].mBuffer->getImgSize().h, DP_COLOR_YV12);
    dpBlit.setRotate(0);

    // set & add pipe to stream
    if (dpBlit.invalidate())  //trigger HW
    {
          LOG_ERR("FDstream invalidate failed");
          return MFALSE;
    }
    //saveBufToFile("fdi.yuv", dstbufadr, dst_ysize + dst_uvsize * 2);
#endif

    if (OutQParams.mpfnCallback == NULL)
    {
        Mutex::Autolock lock(mLock);
        qCallerQParams.push(OutQParams);
        sem_post(&mModuleSem);
    }

    handleReturnBuffer(VFBSW_TO_P2B_PCA, (MUINTPTR) &jobPCA.imgReq, jobPCA.mFrameNo);

    CPTLog(Event_FeaturePipe_P2BThread, CPTFlagEnd);
    CAM_TRACE_FMT_END();

    FUNCTION_LOG_END;
    return MTRUE;
}
MBOOL
FeaturePipeP2bNode::
onReturnBuffer(MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
    //FUNCTION_LOG_START;
    //FUNCTION_LOG_END;
    return MTRUE;
}
MVOID
FeaturePipeP2bNode::
onDumpBuffer(const char* usr, MUINT32 const data, MUINTPTR const buf, MUINT32 const ext)
{
}
