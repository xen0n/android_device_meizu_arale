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

///////////////////////////////////////////////////////////////////////////////
// No Warranty
// Except as may be otherwise agreed to in writing, no warranties of any
// kind, whether express or implied, are given by MTK with respect to any MTK
// Deliverables or any use thereof, and MTK Deliverables are provided on an
// "AS IS" basis.  MTK hereby expressly disclaims all such warranties,
// including any implied warranties of merchantability, non-infringement and
// fitness for a particular purpose and any warranties arising out of course
// of performance, course of dealing or usage of trade.  Parties further
// acknowledge that Company may, either presently and/or in the future,
// instruct MTK to assist it in the development and the implementation, in
// accordance with Company's designs, of certain softwares relating to
// Company's product(s) (the "Services").  Except as may be otherwise agreed
// to in writing, no warranties of any kind, whether express or implied, are
// given by MTK with respect to the Services provided, and the Services are
// provided on an "AS IS" basis.  Company further acknowledges that the
// Services may contain errors, that testing is important and Company is
// solely responsible for fully testing the Services and/or derivatives
// thereof before they are used, sublicensed or distributed.  Should there be
// any third party action brought against MTK, arising out of or relating to
// the Services, Company agree to fully indemnify and hold MTK harmless.
// If the parties mutually agree to enter into or continue a business
// relationship or other arrangement, the terms and conditions set forth
// hereunder shall remain effective and, unless explicitly stated otherwise,
// shall prevail in the event of a conflict in the terms in any agreements
// entered into between the parties.
////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2008, MediaTek Inc.
// All rights reserved.
//
// Unauthorized use, practice, perform, copy, distribution, reproduction,
// or disclosure of this information in whole or in part is prohibited.
////////////////////////////////////////////////////////////////////////////////
// AcdkCLITest.cpp  $Revision$
////////////////////////////////////////////////////////////////////////////////

//! \file  AcdkCLITest.cpp
//! \brief

#define LOG_TAG "IOpipeTest_FrmB"

#include <vector>

using namespace std;

//#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/prctl.h>
#include <stdio.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <mtkcam/common/hw/hwstddef.h>

#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/drv_FrmB/isp_drv_FrmB.h>
#include <mtkcam/drv_common/isp_reg.h>


#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>


//thread
#include <utils/threads.h>
#include <mtkcam/v1/config/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>
#include <pthread.h>
#include <semaphore.h>

using namespace android;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc_FrmB;
using namespace NSIspDrv_FrmB;



/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(IOpipeTest_FrmB);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (IOpipeTest_FrmB_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (IOpipeTest_FrmB_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#if 0  //kk test
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#else
#define LOG_INF        printf
#endif

#define LOG_WRN(fmt, arg...)        do { if (IOpipeTest_FrmB_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (IOpipeTest_FrmB_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (IOpipeTest_FrmB_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");


static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGI, 0);
static const PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WDMAO, 1);
static const PortID WROTO( NSIoPipe::EPortType_Memory, NSImageio_FrmB::NSIspio_FrmB::EPortIndex_WROTO, 1);
static const PortID IMG2O( NSIoPipe::EPortType_Memory, NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMG2O, 1);
static const PortID IMGO( NSIoPipe::EPortType_Memory, NSImageio_FrmB::NSIspio_FrmB::EPortIndex_IMGO, 1);
///////////////////////////////////////////////
//test cases
int pass2_BasicTest(int casenum, int type);
int pass2_loopTest(int casenum, int type);
int pass2_multiEnqueTest(int casenum, int type);
int pass2_TuningTest(int casenum);
int applyTuningSetting(int mode, isp_reg_t **ppIspPhyReg);

//////////////////////////////////////////////
//global variables
static sem_t     mSem;
static sem_t     mSem_ThreadEnd;
static pthread_t           mThread;
static bool bTuning=false;
/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    //LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    //LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
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
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}



/******************************************************************************
* another thread for pass2 operation test
*******************************************************************************/
#include "pic/imgi_1280x720_bayer10.h"
#include "pic/imgi_640x480_bayer10.h"
#include "pic/imgi_640x480_yuy2.h"
#include "pic/imgi_2560x1440_bayer10.h"
#include "pic/imgi_960x640_bayer10.h"


static int multiThread_case=0;
static int multiThread_type=0;
#define PR_SET_NAME 15
static MVOID* onThreadLoop(MVOID *arg)
{
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"onThreadLoop",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority    = NICE_CAMERA_PASS2;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    if(policy == SCHED_OTHER)
    {    //    Note: "priority" is nice-value priority.
        sched_p.sched_priority = 0;
        ::sched_setscheduler(0, policy, &sched_p);
        ::setpriority(PRIO_PROCESS, 0, priority);
    }
    else
    {    //    Note: "priority" is real-time priority.
        sched_p.sched_priority = priority;
        ::sched_setscheduler(0, policy, &sched_p);
    }
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //    detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //////////////////////////////////////////////////////////////////
    //
    NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pCCStream;
    NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pIPStream;
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
    mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
    mpIspDrv->init("onThreadLoop");
    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
        enqueParams.mvMagicNo.push_back(0);
    IMEM_BUF_INFO imgiBuf;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBuffer* tempBuffer;
    MBOOL isV3 = MFALSE;    //temp disable tuning path
    if(multiThread_case==6)
    {
        switch(multiThread_type)
        {
            case 0:
                {
                    //object init
                    pCCStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("onThreadLoop", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, isV3);
                    pCCStream->init();
                    //////////////////////////
                    //prepare dma data, imgi
                    MUINT32 bufStridesInBytes[3] = {1280, 0, 0};
                        imgiBuf.size=sizeof(g_imgi_array_640x480_yuy2);
                    mpImemDrv->allocVirtBuf(&imgiBuf);
                        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_yuy2), imgiBuf.size);
                        //imem buffer 2 image heap
                        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                        MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
                        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
                    tempBuffer = pHeap->createImageBuffer();
                        tempBuffer->incStrong(tempBuffer);
                        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                        Input src;
                    src.mPortID=IMGI;
                    src.mBuffer=tempBuffer;
                    enqueParams.mvIn.push_back(src);
                    //default: w/o crop, w/o resize
                    MCrpRsInfo crop;
                    crop.mGroupID=1;
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=640;
                    crop.mCropRect.s.h=480;
                    crop.mResizeDst.w=640;
                    crop.mResizeDst.h=480;
                    enqueParams.mvCropRsInfo.push_back(crop);
                    //////////////////////////
                    //prepare dma data, img2o
                    IMEM_BUF_INFO img2oBuf;
                    img2oBuf.size=640*480*2;
                        mpImemDrv->allocVirtBuf(&img2oBuf);
                    memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                    MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
                    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                        MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                        outBuffer->incStrong(outBuffer);
                        outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst;
                    dst.mPortID=IMG2O;
                    dst.mBuffer=outBuffer;
                    enqueParams.mvOut.push_back(dst);
                    ////////////////////////////
                    ::sem_wait(&mSem);
                    //enque/deque operation
                    mpImemDrv->cacheFlushAll();
                    pCCStream->enque(enqueParams);
                    LOG_INF("onThreadLoop ENQUE DONE");
                    QParams dequeParams;
                    pCCStream->deque(dequeParams);
                    LOG_INF("onThreadLoop DEQUE DONE");

                    ///////////////////////////
                    //save image
                    char filename[256];
                        sprintf(filename, "/sdcard/Pictures/multiThread_onThreadLoop_case0_img2o%dx%d.yuv", 640,480);
                        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
                        //destroy
                        LOG_INF("onThreadLoop destroy normalstream");
                        pCCStream->uninit();
                    pCCStream->destroyInstance("onThreadLoop");
                    }
                break;
            case 1:
                {
                    pIPStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("onThreadLoop", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_IPRaw,0xFFFF, isV3);
                    pIPStream->init();
                    //////////////////////////
                    //prepare dma data, imgi
                    MUINT32 bufStridesInBytes[3] = {1280, 0, 0};
                        imgiBuf.size=sizeof(g_imgi_array_640x480_yuy2);
                    mpImemDrv->allocVirtBuf(&imgiBuf);
                        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_yuy2), imgiBuf.size);
                        //imem buffer 2 image heap
                        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                        MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
                        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
                    tempBuffer = pHeap->createImageBuffer();
                        tempBuffer->incStrong(tempBuffer);
                        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                        Input src;
                    src.mPortID=IMGI;
                    src.mBuffer=tempBuffer;
                    enqueParams.mvIn.push_back(src);
                    //default: w/o crop, w/o resize
                    MCrpRsInfo crop;
                    crop.mGroupID=1;
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=640;
                    crop.mCropRect.s.h=480;
                    crop.mResizeDst.w=640;
                    crop.mResizeDst.h=480;
                    enqueParams.mvCropRsInfo.push_back(crop);
                    //////////////////////////
                    //prepare dma data, imgo
                    IMEM_BUF_INFO imgoBuf;
                    imgoBuf.size=640*480*2;
                        mpImemDrv->allocVirtBuf(&imgoBuf);
                    memset((MUINT8*)imgoBuf.virtAddr, 0xffffffff, imgoBuf.size);
                    MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
                    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( imgoBuf.memID,imgoBuf.virtAddr,0,imgoBuf.bufSecu, imgoBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                        MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                        outBuffer->incStrong(outBuffer);
                        outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst;
                    dst.mPortID=IMGO;
                    dst.mBuffer=outBuffer;
                    enqueParams.mvOut.push_back(dst);
                    ////////////////////////////
                    ::sem_wait(&mSem);
                    LOG_INF("onThreadLoop loop start");
                    for(int i=0;i<10;i++)
                    {
                        memset((MUINT8*)imgoBuf.virtAddr, 0xffffffff, imgoBuf.size);
                        //enque/deque operation
                        mpImemDrv->cacheFlushAll();
                        pIPStream->enque(enqueParams);
                        QParams dequeParams;
                        pIPStream->deque(dequeParams);

                        //LOG_INF("enter while");
                        //test
                            //while(1);

                        ///////////////////////////
                        //save image
                        char filename[256];
                            sprintf(filename, "/sdcard/Pictures/multiThread_onThreadLoop_case1_%d_imgo%dx%d.yuv",i, 640,480);
                            saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);

                    }
                    LOG_INF("onThreadLoop loop end");
                    //destroy
                        pIPStream->uninit();
                    pIPStream->destroyInstance("onThreadLoop");
                }
                break;
            default:
                break;
        }
    }
    else if(multiThread_case==8)
    {    // temp use 960x640, TO DO 2560x1440 later
        //object init
        NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pStreamObj;
        void *pTuningQueBuf;
        isp_reg_t *pIspPhyReg;
        MUINT32 size;
        switch (multiThread_type)
        {
            default:
            case 0:
                pStreamObj= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("onThreadLoop", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, bTuning);
                break;
            case 1:
                pStreamObj= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("onThreadLoop", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Vss,0xFFFF, bTuning);
                break;
        }
        pStreamObj->init();
        //////////////////////////
        //prepare dma data, imgi
        MUINT32 bufStridesInBytes[3] = {1200, 0, 0};
            imgiBuf.size=sizeof(g_imgi_array_960x640_b10);
            //MUINT32 bufStridesInBytes[3] = {3200, 0, 0};
            //imgiBuf.size=sizeof(g_imgi_array_2560x1440_b10);
        mpImemDrv->allocVirtBuf(&imgiBuf);
            memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_960x640_b10), imgiBuf.size);
             //memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_2560x1440_b10), imgiBuf.size);
            //imem buffer 2 image heap
            PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                            MSize(/*2560, 1440*/960, 640), bufStridesInBytes, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( "onThreadLoop", imgParam,portBufInfo,true);
        tempBuffer = pHeap->createImageBuffer();
            tempBuffer->incStrong(tempBuffer);
            tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            //
        MRect p1SrcCrop;
        MSize p1Dst;
        MRect p1DstCrop;

        p1SrcCrop.s.w = 960;
        p1SrcCrop.s.h =  640;
        p1Dst.w =960;
        p1Dst.h =  640;
        p1DstCrop.p.x =  0;
        p1DstCrop.p.y =  0;
        enqueParams.mvP1SrcCrop.push_back(p1SrcCrop);
        enqueParams.mvP1Dst.push_back(p1Dst);
        enqueParams.mvP1DstCrop.push_back(p1DstCrop);
        //
            Input src;
        src.mPortID=IMGI;
        src.mBuffer=tempBuffer;
        src.mPortID.group=0;
        enqueParams.mvIn.push_back(src);
        //default: w/o crop, w/o resize
        int out_w=960;//960;
        int out_h=640;//640;
        MCrpRsInfo crop;
        crop.mGroupID=1;
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=out_w;
        crop.mCropRect.s.h=out_h;
        crop.mResizeDst.w=out_w;
        crop.mResizeDst.h=out_h;
        enqueParams.mvCropRsInfo.push_back(crop);
        MCrpRsInfo crop_2;
        crop_2.mGroupID=2;
        crop_2.mCropRect.p_fractional.x=0;
        crop_2.mCropRect.p_fractional.y=0;
        crop_2.mCropRect.p_integral.x=0;
        crop_2.mCropRect.p_integral.y=0;
        crop_2.mCropRect.s.w=out_w;
        crop_2.mCropRect.s.h=out_h;
        crop_2.mResizeDst.w=out_w;
        crop_2.mResizeDst.h=out_h;
        enqueParams.mvCropRsInfo.push_back(crop_2);
        //////////////////////////
        //////////////////////////
        //prepare dma data, wdmao
        IMEM_BUF_INFO wdmaoBuf;
        wdmaoBuf.size=out_w*out_h*2;
            mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {out_w, out_w/2, out_w/2};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(out_w,out_h),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( "onThreadLoop", imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=WDMAO;
        dst.mBuffer=outBuffer;
        dst.mPortID.group=0;
        enqueParams.mvOut.push_back(dst);

        ////////////////////////////
        ::sem_wait(&mSem);
        LOG_INF("onThreadLoop loop start");
        for(int i=0;i<5;i++)
        {
            memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
            enqueParams.mpTuningData.clear();
            //enque/deque operation
            mpImemDrv->cacheFlushAll();
            if(bTuning)
            {
                pStreamObj->deTuningQue(size, pTuningQueBuf);
                pIspPhyReg = (isp_reg_t *)pTuningQueBuf;
                applyTuningSetting(2, &pIspPhyReg);
                enqueParams.mpTuningData.push_back(pTuningQueBuf);
            }
            pStreamObj->enque(enqueParams);

            if(bTuning)
            {
                // return tuningque buffer
                pStreamObj->enTuningQue(pTuningQueBuf);
            }
            //test
            //while(1);

            QParams dequeParams;
            pStreamObj->deque(dequeParams);

            ///////////////////////////
            //save image
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2_case_%d_type_%d_onThreadLoop_%d_wdmao%dx%d.yuv",multiThread_case,multiThread_type,i, out_w,out_h);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), out_w *out_h * 2);

        }
        LOG_INF("onThreadLoop loop end");
            pStreamObj->uninit();
        pStreamObj->destroyInstance("onThreadLoop");
    }

    LOG_INF("onThreadLoop start to uninit");
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit("onThreadLoop");
    mpIspDrv->destroyInstance();
    LOG_INF("onThreadLoop EXIT");

    ::sem_post(&mSem_ThreadEnd);
    return NULL;
}

/*******************************************************************************
*  Postproc 1 IN / 2 Out
********************************************************************************/
int main_testNormalStream(int testcaseType,int testcaseNum, int testtype)
{
    LOG_INF("+");
    int ret=0;
    switch(testcaseType)
    {
        case 1:
            ret=pass2_BasicTest(testcaseNum,testtype);
            break;
        case 2:
            ret=pass2_TuningTest(testcaseNum);
            break;
        default:
            break;
    }

    return ret;
}

/*******************************************************************************
* [image width limitation w/o tpipe is 768]
* case0 : [preview path][path test] imgi in/img2o out, test isp only mode (bypass mdp), frame mode(disable tpipe), 640x480 (input format: bayer 10)
* case1 : [preview path][path test] imgi in/img2o out, test isp only mode (bypass mdp), frame mode(disable tpipe), 640x480 (input format: yuy2)
* case2 : [ip-raw path][path test] imgi in/imgo out, test isp only mode(bypass mdp), frame mode, 640x480 (input format: bayer 10)
* case3 : [ip-raw path][path test] imgi in/imgo out, test isp only mode (bypass mdp), frame mode(disable tpipe), 640x480 (input format: yuy2)
* case4 : [preview path][zoom test] imgi in/img2o out, isp only&frame mode, enable cdrz, 640x480 in/320x240 out (input format: bayer 10)
* case5 : [preview path][zoom test] imgi in/img2o out, isp only&frame mode, enable cdrz, 640x480 in,crop 400x200, /320x240 out (input format: bayer 10)
* case6 : [dup cq&buffer control test]
   - type 0: thread A and thread B do enque / deque
   - type 1: thread A and thread B do enque / deque and change different scenario in each run
   - type 2: multi-frame enque test (single thread),,, may need pass1 update burst queue num api
   - kernel buffer control mechanism

above cases are isp only mode,,,

test cases below are direct link mode,,,
* case7: [preview path]
   - type 0: imgi in(1280x720 bayer10) / two out(1280x720 wmdao[yv12], 1280x720 wroto[yuy2])
   - type 1: [loop test] imgi in(1280x720 bayer10)_with different crop / two out(1280x720 wmdao[yv12], 1280x720 wroto[yuy2])_with different resized value and rotate
* case 8: [preview path+vss]
   - multi-thread + duplicated CQ + pass2 buffer control test
   - (1) -> imgi in (1280x720 bayer 10 / two out(1280x720 wmdao[yv12], 1280x720 wroto[yv12])
   - (2) -> imgi in (2560x1440 bayer 10 / wdmao out(2560x1440 yv12)
   - type 0: [preview path] using (1) + [preview path] using (2)
   - type 1: [preview path] using (1) + [vss path] using (2)
   - type 2: [preview path] using (1) + [preview path] using (2) , with different tuning setting

///////////// reserved test case /////////////
* case9: [preview path][zoom test]
   - type 0: imgi in(640x480 bayer10) / two out(img2o[yuy2,resized], wdmao[yv12])
   - type 1: imgi in(640x480 bayer10) / two out(img2o[yuy2,resized], wdmao[yv12,resized])
   - type 2: imgi in(640x480 bayer10) / three out(img2o[yuy2,resized], wdmao[yv12],wroto[yv12,resized])
   - type 3: imgi in(640x480 bayer10) / three out(img2o[yuy2,resized], wdmao[yv12,resized],wroto[yv12,resized])

********************************************************************************/
int pass2_BasicTest(int casenum, int type)
{
    int ret=0;
        MBOOL isV3 = MFALSE;    //temp disable tuning path
    sem_init(&mSem, 0, 0);
    sem_init(&mSem_ThreadEnd, 0, 0);
    LOG_INF("casenum(%d),type(%d)",casenum,type);

    if(casenum==6)
    {
        switch(type)
        {
            case 0:
            case 1:
                {
                    multiThread_case=casenum;
                    multiThread_type=type;
                    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS2};
                    pthread_create(&mThread, &attr, onThreadLoop, NULL);
                    break;
                }
            case 2:
            default:
                break;
        }

        if(type == 1)
        {    //sub-function
            int subRet=0;
            subRet=pass2_loopTest(casenum,type);
            LOG_INF("subRet...(%d)",subRet);
            goto EXIT;
        }
        else if(type==2)
        {    //sub-fucntion
            int subRet=0;
            subRet=pass2_multiEnqueTest(casenum,type);
            LOG_INF("subRet...(%d)",subRet);
            goto EXIT;

        }
    }
    else if(casenum==7)
    {
        if(type==1)
        {
            int subRet=0;
            subRet=pass2_loopTest(casenum,type);
            LOG_INF("subRet...(%d)",subRet);
            goto EXIT;
        }
    }
    else if(casenum==8)
    {
        {
            multiThread_case=casenum;
            multiThread_type=type;
            pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS2};
            pthread_create(&mThread, &attr, onThreadLoop, NULL);
        }
        if(type==2)
        {
            bTuning=true;
        }
        int subRet=0;
        subRet=pass2_loopTest(casenum,type);
        LOG_INF("subRet...(%d)",subRet);
        goto EXIT;
    }


    {
        //objects
        NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pStream;

        switch(casenum)
        {
            case 2:
            case 3:
                pStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_IPRaw,0xFFFF, isV3);
                break;
            default:
                pStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, isV3);
                break;
        }
        pStream->init();
        IMemDrv* mpImemDrv=NULL;
        mpImemDrv=IMemDrv::createInstance();
        mpImemDrv->init();
        NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
        mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
        mpIspDrv->init(LOG_TAG);

        QParams enqueParams;
        //enqueParams.mpPrivaData = NULL;
        enqueParams.mvPrivaData.push_back(NULL);
            enqueParams.mvMagicNo.push_back(0);

        ///////
        //create input
        IMEM_BUF_INFO imgiBuf;

        MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        IImageBuffer* tempBuffer;

            if((casenum==0) ||(casenum==2) ||(casenum==4) ||(casenum==5)||(casenum==6))
            {    //imgi in/img2o out, test isp only mode (bypass mdp)
                MUINT32 bufStridesInBytes[3] = {800, 0, 0};
                imgiBuf.size=sizeof(g_imgi_array_640x480_b10);
            mpImemDrv->allocVirtBuf(&imgiBuf);
                memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), imgiBuf.size);
                //imem buffer 2 image heap
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
            tempBuffer = pHeap->createImageBuffer();
                tempBuffer->incStrong(tempBuffer);
                tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            }
            else if((casenum==1) ||(casenum==3))
            {    //imgi in/img2o out, test isp only mode (bypass mdp)
                MUINT32 bufStridesInBytes[3] = {1280, 0, 0};
                imgiBuf.size=sizeof(g_imgi_array_640x480_yuy2);
            mpImemDrv->allocVirtBuf(&imgiBuf);
                memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_yuy2), imgiBuf.size);
                //imem buffer 2 image heap
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
            tempBuffer = pHeap->createImageBuffer();
                tempBuffer->incStrong(tempBuffer);
                tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            }
            else
            {
                MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
                imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
            mpImemDrv->allocVirtBuf(&imgiBuf);
                memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
                 //imem buffer 2 image heap
                PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
            tempBuffer = pHeap->createImageBuffer();
                tempBuffer->incStrong(tempBuffer);
                tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        }

        //
        Input src;
        src.mPortID=IMGI;
        src.mBuffer=tempBuffer;
        enqueParams.mvIn.push_back(src);
        //crop
        MCrpRsInfo crop;
        crop.mGroupID=1;
        MCrpRsInfo crop2;
        crop2.mGroupID=2;
        switch (casenum)
        {
            case 0:
            case 1:
            case 2:
            case 3:
                crop.mCropRect.p_fractional.x=0;
                crop.mCropRect.p_fractional.y=0;
                crop.mCropRect.p_integral.x=0;
                crop.mCropRect.p_integral.y=0;
                crop.mCropRect.s.w=640;
                crop.mCropRect.s.h=480;
                crop.mResizeDst.w=640;
                crop.mResizeDst.h=480;
                break;
            case 4:
                crop.mCropRect.p_fractional.x=0;
                crop.mCropRect.p_fractional.y=0;
                crop.mCropRect.p_integral.x=0;
                crop.mCropRect.p_integral.y=0;
                crop.mCropRect.s.w=640;
                crop.mCropRect.s.h=480;
                crop.mResizeDst.w=320;
                crop.mResizeDst.h=240;
                break;
            case 5:
                crop.mCropRect.p_fractional.x=0;
                crop.mCropRect.p_fractional.y=0;
                crop.mCropRect.p_integral.x=120;
                crop.mCropRect.p_integral.y=140;
                crop.mCropRect.s.w=400;
                crop.mCropRect.s.h=200;
                crop.mResizeDst.w=320;
                crop.mResizeDst.h=240;
                break;
            case 6:
                if(type==0)
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=640;
                    crop.mCropRect.s.h=380;
                    crop.mResizeDst.w=640;
                    crop.mResizeDst.h=480;
                }
                else if(type==1)
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=120;
                    crop.mCropRect.p_integral.y=140;
                    crop.mCropRect.s.w=400;
                    crop.mCropRect.s.h=200;
                    crop.mResizeDst.w=320;
                    crop.mResizeDst.h=240;
                }
                break;
            case 7:
                if(type==0)
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=1280;
                    crop.mCropRect.s.h=720;
                    crop.mResizeDst.w=1280;
                    crop.mResizeDst.h=720;

                    crop2.mCropRect.p_fractional.x=0;
                    crop2.mCropRect.p_fractional.y=0;
                    crop2.mCropRect.p_integral.x=0;
                    crop2.mCropRect.p_integral.y=0;
                    crop2.mCropRect.s.w=1280;
                    crop2.mCropRect.s.h=720;
                    crop2.mResizeDst.w=1280;
                    crop2.mResizeDst.h=720;
                }
                else if (type==1)
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=1280;
                    crop.mCropRect.s.h=720;
                    crop.mResizeDst.w=1280;
                    crop.mResizeDst.h=720;
                }
                break;
            case 9:
                crop.mCropRect.p_fractional.x=0;
                crop.mCropRect.p_fractional.y=0;
                crop.mCropRect.p_integral.x=0;
                crop.mCropRect.p_integral.y=0;
                crop.mCropRect.s.w=640;
                crop.mCropRect.s.h=480;
                crop.mResizeDst.w=640;
                crop.mResizeDst.h=480;

                crop2.mCropRect.p_fractional.x=0;
                crop2.mCropRect.p_fractional.y=0;
                crop2.mCropRect.p_integral.x=0;
                crop2.mCropRect.p_integral.y=0;
                crop2.mCropRect.s.w=640;
                crop2.mCropRect.s.h=480;
                crop2.mResizeDst.w=640;
                crop2.mResizeDst.h=480;
                break;
            default:
                break;
        }
        enqueParams.mvCropRsInfo.push_back(crop);
        if((casenum==7) || (casenum==9))
        {
            enqueParams.mvCropRsInfo.push_back(crop2);
        }

        /////////
        //output buffer
        IMEM_BUF_INFO wdmaoBuf;
        IMEM_BUF_INFO wrotoBuf;
        IMEM_BUF_INFO img2oBuf;
        IMEM_BUF_INFO imgoBuf;
        if(casenum==0)
        {    //imgi in/img2o out, test isp only mode (bypass mdp)
            img2oBuf.size=640*480*2;
                mpImemDrv->allocVirtBuf(&img2oBuf);
            memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=IMG2O;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
        }
        else if(casenum==1)
        {    //imgi in/img2o out, test isp only mode (bypass mdp)
            img2oBuf.size=640*480*2;
                mpImemDrv->allocVirtBuf(&img2oBuf);
            memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=IMG2O;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
        }
        else if((casenum==2) ||(casenum==3))
        {
            #if 0
            imgoBuf.size=640*480*10/8; //output pack 10
                mpImemDrv->allocVirtBuf(&imgoBuf);
            memset((MUINT8*)imgoBuf.virtAddr, 0x0, imgoBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {800, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( imgoBuf.memID,imgoBuf.virtAddr,0,imgoBuf.bufSecu, imgoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                #else
                imgoBuf.size=640*480*2;
                mpImemDrv->allocVirtBuf(&imgoBuf);
            memset((MUINT8*)imgoBuf.virtAddr, 0xffffffff, imgoBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( imgoBuf.memID,imgoBuf.virtAddr,0,imgoBuf.bufSecu, imgoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                #endif
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=IMGO;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
        }
        else if((casenum==4) ||(casenum==5))
        {
            img2oBuf.size=320*240*2;
                mpImemDrv->allocVirtBuf(&img2oBuf);
            memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=IMG2O;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
        }
        else if(casenum==6)
        {
            if(type==0)
            {
                img2oBuf.size=640*480*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
            }
            else if(type==1)
            {
                img2oBuf.size=320*240*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
            }
        }
        else if(casenum==7)
        {
            if(type==0)
            {
                wdmaoBuf.size=1280*720*2;
                    mpImemDrv->allocVirtBuf(&wdmaoBuf);
                memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
                MUINT32 bufStridesInBytes_3[3] = {1280, 640, 640};
                PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(1280, 720),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
                IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                    outBuffer2->incStrong(outBuffer2);
                    outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst2;
                dst2.mPortID=WDMAO;
                dst2.mBuffer=outBuffer2;
                enqueParams.mvOut.push_back(dst2);
                //
                wrotoBuf.size=1280*720*2;
                    mpImemDrv->allocVirtBuf(&wrotoBuf);
                memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
                MUINT32 bufStridesInBytes_4[3] = {2560,0,0};
                PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(1280, 720),  bufStridesInBytes_4, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
                IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
                    outBuffer3->incStrong(outBuffer3);
                    outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst3;
                dst3.mPortID=WROTO;
                dst3.mBuffer=outBuffer3;
                enqueParams.mvOut.push_back(dst3);
            }
            else if (type==1)
            {
            }
        }
        else if(casenum==9)
        {
            if(type==0)
            {
                img2oBuf.size=320*240*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
                //
                wdmaoBuf.size=640*480*2;
                    mpImemDrv->allocVirtBuf(&wdmaoBuf);
                memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
                MUINT32 bufStridesInBytes_3[3] = {640, 320, 320};
                PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(640, 480),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
                IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                    outBuffer2->incStrong(outBuffer2);
                    outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst2;
                dst2.mPortID=WDMAO;
                dst2.mBuffer=outBuffer2;
                enqueParams.mvOut.push_back(dst2);
            }
            else if(type==1)
            {
                img2oBuf.size=320*240*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
                //
                wdmaoBuf.size=640*480*2;
                    mpImemDrv->allocVirtBuf(&wdmaoBuf);
                memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
                MUINT32 bufStridesInBytes_3[3] = {640, 320, 320};
                PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(640, 480),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
                IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                    outBuffer2->incStrong(outBuffer2);
                    outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst2;
                dst2.mPortID=WDMAO;
                dst2.mBuffer=outBuffer2;
                enqueParams.mvOut.push_back(dst2);

            }
            else if(type==2)
            {
                img2oBuf.size=320*240*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
                //
                wdmaoBuf.size=640*480*2;
                    mpImemDrv->allocVirtBuf(&wdmaoBuf);
                memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
                MUINT32 bufStridesInBytes_3[3] = {640, 320, 320};
                PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(640, 480),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
                IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                    outBuffer2->incStrong(outBuffer2);
                    outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst2;
                dst2.mPortID=WDMAO;
                dst2.mBuffer=outBuffer2;
                enqueParams.mvOut.push_back(dst2);
                //
                wrotoBuf.size=1280*720*2;
                    mpImemDrv->allocVirtBuf(&wrotoBuf);
                memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
                MUINT32 bufStridesInBytes_4[3] = {1280, 640, 640};
                PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(1280, 720),  bufStridesInBytes_4, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
                IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
                    outBuffer3->incStrong(outBuffer3);
                    outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst3;
                dst3.mPortID=WROTO;
                dst3.mBuffer=outBuffer3;
                enqueParams.mvOut.push_back(dst3);
            }
            else if(type==3)
            {
                img2oBuf.size=320*240*2;
                    mpImemDrv->allocVirtBuf(&img2oBuf);
                memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                MUINT32 bufStridesInBytes_2[3] = {640, 0, 0};
                PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                    MSize(320, 240),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
                IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                    outBuffer->incStrong(outBuffer);
                    outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst;
                dst.mPortID=IMG2O;
                dst.mBuffer=outBuffer;
                enqueParams.mvOut.push_back(dst);
                //
                wdmaoBuf.size=480*360*2;
                    mpImemDrv->allocVirtBuf(&wdmaoBuf);
                memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
                MUINT32 bufStridesInBytes_3[3] = {480, 240, 240};
                PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(480, 360),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
                IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                    outBuffer2->incStrong(outBuffer2);
                    outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst2;
                dst2.mPortID=WDMAO;
                dst2.mBuffer=outBuffer2;
                enqueParams.mvOut.push_back(dst2);
                //
                wrotoBuf.size=1280*720*2;
                    mpImemDrv->allocVirtBuf(&wrotoBuf);
                memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
                MUINT32 bufStridesInBytes_4[3] = {1280, 640, 640};
                PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                    MSize(1280, 720),  bufStridesInBytes_4, bufBoundaryInBytes, 3);
                sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
                IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
                    outBuffer3->incStrong(outBuffer3);
                    outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                Output dst3;
                dst3.mPortID=WROTO;
                dst3.mBuffer=outBuffer3;
                enqueParams.mvOut.push_back(dst3);
            }
        }
        #if 0
        else if(casenum==9)
        {
            wdmaoBuf.size=640*480*2;
                mpImemDrv->allocVirtBuf(&wdmaoBuf);
            memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=WDMAO;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
            //
            wrotoBuf.size=1280*720*2;
                mpImemDrv->allocVirtBuf(&wrotoBuf);
            memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
            MUINT32 bufStridesInBytes_3[3] = {1280, 640, 640};
            PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                MSize(1280, 720),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
            sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
            IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                outBuffer2->incStrong(outBuffer2);
                outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst2;
            dst2.mPortID=WROTO;
            dst2.mBuffer=outBuffer2;
            enqueParams.mvOut.push_back(dst2);
        }
        else if(casenum==10)
        {
            wdmaoBuf.size=640*480*2;
                mpImemDrv->allocVirtBuf(&wdmaoBuf);
            memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
            MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
            PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
            IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=WDMAO;
            dst.mBuffer=outBuffer;
            enqueParams.mvOut.push_back(dst);
            //
            wrotoBuf.size=960*640*2;
                mpImemDrv->allocVirtBuf(&wrotoBuf);
            memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
            MUINT32 bufStridesInBytes_3[3] = {640, 320, 320};
            PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                MSize(640, 960),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
            sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
            IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
                outBuffer2->incStrong(outBuffer2);
                outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst2;
            dst2.mPortID=WROTO;
            dst2.mBuffer=outBuffer2;
            dst2.mTransform=eTransform_ROT_90;
            enqueParams.mvOut.push_back(dst2);
        }
        #endif


        /////
        //temp to add, (original done in pass1)
        MUINT32 reg_val=0x0;
        ISP_WRITE_REG(mpIspDrv,CAM_CTL_SEL, 0, ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(mpIspDrv,CAM_CTL_EN1, 0x40000000, ISP_DRV_USER_ISPF);
            reg_val = ISP_READ_REG_NOPROTECT(mpIspDrv,CAM_CTL_EN2);
            ISP_WRITE_REG(mpIspDrv,CAM_CTL_EN2, reg_val&0x04000000, ISP_DRV_USER_ISPF);//FMT_EN don't touch
            ISP_WRITE_REG(mpIspDrv,CAM_CTL_DMA_EN, 0, ISP_DRV_USER_ISPF);
            //disable GGM
            ISP_WRITE_REG(mpIspDrv,CAM_GGM_CTRL, 0, ISP_DRV_USER_ISPF);
            //reset GDMA relative setting
            ISP_WRITE_REG(mpIspDrv,CAM_CTL_SEL, 0, ISP_DRV_USER_ISPF);
            //interrupt merge
            ISP_WRITE_BITS(mpIspDrv,CAM_CTL_SPARE3,INT_MRG, 1, ISP_DRV_USER_ISPF);
            /////


            LOG_INF("enque");
        //flush first
        mpImemDrv->cacheFlushAll();
        LOG_INF("flush done");

        if(casenum==6)
        {
            switch(type)
            {
                case 0:
                case 1:
                    ::sem_post(&mSem);
                    break;
                case 3:
                default:
                    break;
            }
        }
        //enque
        pStream->enque(enqueParams);
        LOG_INF("ENQUE DONE");

        ////
        //deque
        QParams dequeParams;
        pStream->deque(dequeParams);
        LOG_INF("DEQUE DONE");

        ////
        //dump pic to watch
        if(casenum==0)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2UT_basicCC_rawin_img2o%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
        }
        else if(casenum==1)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2UT_basicCC_yuvin_img2o%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
        }
        else if(casenum==2)
        {
            char filename[256];
            #if 0
                sprintf(filename, "/sdcard/Pictures/basicP2_IPRAW_imgo%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 10/8);
                #else
                sprintf(filename, "/sdcard/Pictures/P2_basicIPRAW_rawin_imgo%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
                #endif
        }
        else if(casenum==3)
        {
            char filename[256];
            #if 0
                sprintf(filename, "/sdcard/Pictures/basicP2_IPRAW_imgo%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 10/8);
                #else
                sprintf(filename, "/sdcard/Pictures/P2UT_basicIPRAW_yuvin_imgo%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
                #endif
        }
        else if(casenum==4)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2UT_basicCC_img2o_resized_%dx%d.yuv", 320,240);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 320 *240 * 2);
        }
        else if(casenum==5)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2UT_basicCC_img2o_crop_resized_%dx%d.yuv", 320,240);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 320 *240 * 2);
        }
        else if(casenum==6)
        {
            if(type==0)
            {
                char filename[256];
                    sprintf(filename, "/sdcard/Pictures/P2UT_multiThread_CC_img2o%dx%d.yuv", 640,480);
                    saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
            }
        }
        else if(casenum==7)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/P2UT_directLink_7_0_wdmao%dx%d.yuv", 1280,720);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 1280 *720 * 2);
                char filename2[256];
                sprintf(filename2, "/sdcard/Pictures/P2UT_directLink_7_0_wroto%dx%d.yuv", 1280,720);
                saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), 1280 *720 * 2);
        }
        #if 0
        else if(casenum==8)
        {
            char filename[256];
                sprintf(filename, "/sdcard/Pictures/basicP2_4_disp%dx%d.yuv", 640,480);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 2);
                char filename2[256];
                sprintf(filename2, "/sdcard/Pictures/basicP2_4_vido%dx%d.yuv", 640,960);
                saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), 640 *960 * 2);
        }
        #endif
        ////
        //temp enter while to attach code viser
        //LOG_INF("ENTER WHILE");
        //while(1);
        //
        pStream->uninit();
        pStream->destroyInstance(LOG_TAG);
        mpImemDrv->uninit();
        mpImemDrv->destroyInstance();
        mpIspDrv->uninit(LOG_TAG);
        mpIspDrv->destroyInstance();

        if(casenum==6)
        {
            switch(type)
            {
                case 0:
                case 1:
                    ::sem_wait(&mSem_ThreadEnd);
                    break;
                case 2:
                default:
                    break;
            }
        }
    }
EXIT:

    LOG_INF("TEST CASE DONE");
    return ret;
}
int pass2_loopTest(int casenum, int type)
{
    int ret=0;
    MBOOL isV3 = MFALSE;    //temp disable tuning path
    LOG_INF("enter pass2_loopTest");

    switch (casenum)
    {
        case 6:
            if(type==1)
            {
                IMemDrv* mpImemDrv=NULL;
                mpImemDrv=IMemDrv::createInstance();
                mpImemDrv->init();
                NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
                mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
                mpIspDrv->init("pass2_loopTest");
                QParams enqueParams;
                enqueParams.mvPrivaData.push_back(NULL);
                    enqueParams.mvMagicNo.push_back(0);
                ////
                ISP_WRITE_BITS(mpIspDrv,CAM_CTL_SPARE3,INT_MRG, 1, ISP_DRV_USER_ISPF);

                NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pCCStream;
                //object init
                pCCStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("pass2_loopTest", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, isV3);
                pCCStream->init();
                //////////////////////////
                //prepare dma data, imgi
                IMEM_BUF_INFO imgiBuf;
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                IImageBuffer* tempBuffer;
                MUINT32 bufStridesInBytes[3] = {800, 0, 0};
                    imgiBuf.size=sizeof(g_imgi_array_640x480_b10);
                mpImemDrv->allocVirtBuf(&imgiBuf);
                    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), imgiBuf.size);
                    //imem buffer 2 image heap
                    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                    MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( "pass2_loopTest", imgParam,portBufInfo,true);
                tempBuffer = pHeap->createImageBuffer();
                    tempBuffer->incStrong(tempBuffer);
                    tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    //
                Input src;
                src.mPortID=IMGI;
                src.mBuffer=tempBuffer;
                enqueParams.mvIn.push_back(src);
                    //////////////////////////
                    ::sem_post(&mSem);

                    LOG_INF("pass2_loopTest loop start");
                    //prepare different setting for ten runs
                for(int i=0;i<10;i++)
                {
                    //clear
                    enqueParams.mvCropRsInfo.clear();
                    enqueParams.mvOut.clear();
                    //setting
                    int out_w=0,out_h=0;
                    if(i==0)
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=0;
                        crop.mCropRect.p_integral.y=0;
                        crop.mCropRect.s.w=640;
                        crop.mCropRect.s.h=480;
                        crop.mResizeDst.w=640;
                        crop.mResizeDst.h=480;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        out_w=640;
                        out_h=480;
                    }
                    else if((i>0) && (i<5))
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=100+10*i;
                        crop.mCropRect.p_integral.y=50+10*i;
                        crop.mCropRect.s.w=400;
                        crop.mCropRect.s.h=300;
                        crop.mResizeDst.w=320;
                        crop.mResizeDst.h=240;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        out_w=320;
                        out_h=240;
                    }
                    else
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=100+10*(i-5);
                        crop.mCropRect.p_integral.y=50+10*(i-5);
                        crop.mCropRect.s.w=200;
                        crop.mCropRect.s.h=200;
                        crop.mResizeDst.w=480;
                        crop.mResizeDst.h=360;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        out_w=480;
                        out_h=360;
                    }
                    //////////////////////////
                    //prepare dma data, img2o
                    IMEM_BUF_INFO img2oBuf;
                    img2oBuf.size=out_w*out_h*2;
                        mpImemDrv->allocVirtBuf(&img2oBuf);
                    memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
                    MUINT32 bufStridesInBytes_2[3] = {out_w*2, 0, 0};
                    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                        MSize(out_w, out_h),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( "pass2_loopTest", imgParam_2,portBufInfo_2,true);
                    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                        outBuffer->incStrong(outBuffer);
                        outBuffer->lockBuf("pass2_loopTest",eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst;
                    dst.mPortID=IMG2O;
                    dst.mBuffer=outBuffer;
                    enqueParams.mvOut.push_back(dst);
                    ////////////////////////////
                    mpImemDrv->cacheFlushAll();
                    pCCStream->enque(enqueParams);
                    QParams dequeParams;
                    pCCStream->deque(dequeParams);
                    ///////////////////////////
                    //save image
                    char filename[256];
                        sprintf(filename, "/sdcard/Pictures/P2UT_multiThread_pass2_loopTest_case6_1_%d_img2o%dx%d.yuv",i, out_w,out_h);
                        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), out_w *out_h * 2);
                        //unmap and free buffer
                        outBuffer->unlockBuf("pass2_loopTest");
                        mpImemDrv->freeVirtBuf(&img2oBuf);
                }
                    LOG_INF("pass2_loopTest loop end");
                //destroy
                    pCCStream->uninit();
                pCCStream->destroyInstance("pass2_loopTest");
                mpImemDrv->uninit();
                mpImemDrv->destroyInstance();
                mpIspDrv->uninit("pass2_loopTest");
                mpIspDrv->destroyInstance();

                ::sem_wait(&mSem_ThreadEnd);
            }
            break;
        case 7:
            if(type==1)
            {
                IMemDrv* mpImemDrv=NULL;
                mpImemDrv=IMemDrv::createInstance();
                mpImemDrv->init();
                NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
                mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
                mpIspDrv->init("pass2_loopTest");
                QParams enqueParams;
                enqueParams.mvPrivaData.push_back(NULL);
                    enqueParams.mvMagicNo.push_back(0);
                ////
                ISP_WRITE_BITS(mpIspDrv,CAM_CTL_SPARE3,INT_MRG, 1, ISP_DRV_USER_ISPF);

                NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pCCStream;
                //object init
                pCCStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("pass2_loopTest", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, isV3);
                pCCStream->init();
                //////////////////////////
                //prepare dma data, imgi
                IMEM_BUF_INFO imgiBuf;
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                IImageBuffer* tempBuffer;
                MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
                    imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
                mpImemDrv->allocVirtBuf(&imgiBuf);
                    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
                    //imem buffer 2 image heap
                    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                    MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( "pass2_loopTest", imgParam,portBufInfo,true);
                tempBuffer = pHeap->createImageBuffer();
                    tempBuffer->incStrong(tempBuffer);
                    tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    //
                Input src;
                src.mPortID=IMGI;
                src.mBuffer=tempBuffer;
                src.mPortID.group=0;
                enqueParams.mvIn.push_back(src);
                    //////////////////////////
                    LOG_INF("pass2_loopTest loop start");
                    //prepare different setting for ten runs
                for(int i=0;i<10;i++)
                {

                    //clear
                    enqueParams.mvCropRsInfo.clear();
                    enqueParams.mvOut.clear();
                    //setting
                    int out_w=0,out_h=0;
                    #if 1
                    MCrpRsInfo crop;
                    crop.mGroupID=1;
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=1280;
                    crop.mCropRect.s.h=720;
                    crop.mResizeDst.w=1280;
                    crop.mResizeDst.h=720;
                    enqueParams.mvCropRsInfo.push_back(crop);
                    //
                    MCrpRsInfo crop_2;
                    crop_2.mGroupID=2;
                    crop_2.mCropRect.p_fractional.x=0;
                    crop_2.mCropRect.p_fractional.y=0;
                    crop_2.mCropRect.p_integral.x=0;
                    crop_2.mCropRect.p_integral.y=0;
                    crop_2.mCropRect.s.w=1280;
                    crop_2.mCropRect.s.h=720;
                    crop_2.mResizeDst.w=1280;
                    crop_2.mResizeDst.h=720;
                    enqueParams.mvCropRsInfo.push_back(crop_2);
                    //
                    out_w=1280;
                    out_h=720;
                    #else
                    if(i==0)
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=0;
                        crop.mCropRect.p_integral.y=0;
                        crop.mCropRect.s.w=1280;
                        crop.mCropRect.s.h=720;
                        crop.mResizeDst.w=1280;
                        crop.mResizeDst.h=720;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        MCrpRsInfo crop_2;
                        crop_2.mGroupID=2;
                        crop_2.mCropRect.p_fractional.x=0;
                        crop_2.mCropRect.p_fractional.y=0;
                        crop_2.mCropRect.p_integral.x=0;
                        crop_2.mCropRect.p_integral.y=0;
                        crop_2.mCropRect.s.w=1280;
                        crop_2.mCropRect.s.h=720;
                        crop_2.mResizeDst.w=1280;
                        crop_2.mResizeDst.h=720;
                        enqueParams.mvCropRsInfo.push_back(crop_2);
                        //
                        out_w=1280;
                        out_h=720;
                    }
                    else if((i>0) && (i<5))
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=0;
                        crop.mCropRect.p_integral.y=0;
                        crop.mCropRect.s.w=1280;
                        crop.mCropRect.s.h=720;
                        crop.mResizeDst.w=1280;
                        crop.mResizeDst.h=720;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        MCrpRsInfo crop_2;
                        crop_2.mGroupID=2;
                        crop_2.mCropRect.p_fractional.x=0;
                        crop_2.mCropRect.p_fractional.y=0;
                        crop_2.mCropRect.p_integral.x=200+50*i;
                        crop_2.mCropRect.p_integral.y=100+50*i;
                        crop_2.mCropRect.s.w=700;
                        crop_2.mCropRect.s.h=300;
                        crop_2.mResizeDst.w=640;
                        crop_2.mResizeDst.h=480;
                        enqueParams.mvCropRsInfo.push_back(crop_2);
                        //
                        out_w=640;
                        out_h=480;
                    }
                    else
                    {
                        MCrpRsInfo crop;
                        crop.mGroupID=1;
                        crop.mCropRect.p_fractional.x=0;
                        crop.mCropRect.p_fractional.y=0;
                        crop.mCropRect.p_integral.x=0;
                        crop.mCropRect.p_integral.y=0;
                        crop.mCropRect.s.w=1280;
                        crop.mCropRect.s.h=720;
                        crop.mResizeDst.w=1280;
                        crop.mResizeDst.h=720;
                        enqueParams.mvCropRsInfo.push_back(crop);
                        //
                        MCrpRsInfo crop_2;
                        crop_2.mGroupID=2;
                        crop_2.mCropRect.p_fractional.x=0;
                        crop_2.mCropRect.p_fractional.y=0;
                        crop_2.mCropRect.p_integral.x=100+10*(i-5);
                        crop_2.mCropRect.p_integral.y=50+10*(i-5);
                        crop_2.mCropRect.s.w=600;
                        crop_2.mCropRect.s.h=600;
                        crop_2.mResizeDst.w=1280;
                        crop_2.mResizeDst.h=720;
                        enqueParams.mvCropRsInfo.push_back(crop_2);
                        //
                        out_w=1280;
                        out_h=720;
                    }
                    #endif
                    LOG_INF("pass2_loopTest (%d),w/h(%d/%d)",i,out_w,out_h);
                    //////////////////////////
                    //prepare dma data, wdmao
                    IMEM_BUF_INFO wdmaoBuf;
                    wdmaoBuf.size=out_w*out_h*2;
                        mpImemDrv->allocVirtBuf(&wdmaoBuf);
                    memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
                    MUINT32 bufStridesInBytes_2[3] = {out_w, out_w/2, out_w/2};
                    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                        MSize(out_w, out_h),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
                    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( "pass2_loopTest", imgParam_2,portBufInfo_2,true);
                    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                        outBuffer->incStrong(outBuffer);
                        outBuffer->lockBuf("pass2_loopTest",eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst;
                    dst.mPortID=WDMAO;
                    dst.mBuffer=outBuffer;
                    dst.mPortID.group=0;
                    enqueParams.mvOut.push_back(dst);


                    ////////////////////////////
                    int out_w_2=0,out_h_2=0;
                    if(( i % 5 == 1) || ( i % 5 == 3))
                    {    //rotate 90 and rotate 270
                        out_w_2=out_h;
                        out_h_2=out_w;
                    }
                    else
                    {
                        //no rotate and rotate 180
                        out_w_2=out_w;
                        out_h_2=out_h;
                    }
                    //prepare dma data, wroto
                    IMEM_BUF_INFO wrotoBuf;
                    wrotoBuf.size=out_w_2*out_h_2*2;
                        mpImemDrv->allocVirtBuf(&wrotoBuf);
                    memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
                    MUINT32 bufStridesInBytes_4[3] = {out_w_2,out_w_2/2,out_w_2/2};
                    PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                        MSize(out_w_2, out_h_2),  bufStridesInBytes_4, bufBoundaryInBytes, 3);
                    sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( "pass2_loopTest", imgParam_4,portBufInfo_4,true);
                    IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
                        outBuffer3->incStrong(outBuffer3);
                        outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst3;
                    dst3.mPortID=WROTO;
                    dst3.mBuffer=outBuffer3;
                    dst3.mPortID.group=0;
                    switch(( i % 5))
                    {
                        case 1:    //rotate 90
                            dst3.mTransform=eTransform_ROT_90;
                            break;
                        case 2:    //rotate 180
                            dst3.mTransform=eTransform_ROT_180;
                            break;
                        case 3:    //rotate 270
                            dst3.mTransform=eTransform_ROT_270;
                            break;
                        default:
                            break;
                    }
                    enqueParams.mvOut.push_back(dst3);
                    //
                    mpImemDrv->cacheFlushAll();
                    pCCStream->enque(enqueParams);
                    QParams dequeParams;
                    pCCStream->deque(dequeParams);
                    ///////////////////////////
                    LOG_INF("dequemvOutsize(%d)",dequeParams.mvOut.size());
                    //save image
                    char filename[256];
                        sprintf(filename, "/sdcard/Pictures/P2UT_directLink_loopTest_case7_1_%d_wdmao%dx%d.yuv",i, out_w,out_h);
                        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), out_w *out_h * 2);
                        char filename_2[256];
                        sprintf(filename_2, "/sdcard/Pictures/P2UT_directLink_loopTest_case7_1_%d_wroto%dx%d.yuv",i, out_w_2,out_h_2);
                        saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(dequeParams.mvOut[1].mBuffer->getBufVA(0)), out_w_2 *out_h_2 * 2);
                        //unmap and free buffer
                        outBuffer->unlockBuf("pass2_loopTest");
                        outBuffer3->unlockBuf("pass2_loopTest");
                        mpImemDrv->freeVirtBuf(&wdmaoBuf);
                        mpImemDrv->freeVirtBuf(&wrotoBuf);
                }
                    LOG_INF("pass2_loopTest loop end");
                //destroy
                    pCCStream->uninit();
                pCCStream->destroyInstance("pass2_loopTest");
                mpImemDrv->uninit();
                mpImemDrv->destroyInstance();
                mpIspDrv->uninit("pass2_loopTest");
                mpIspDrv->destroyInstance();
            }
            break;
        case 8:
            {
                IMemDrv* mpImemDrv=NULL;
                mpImemDrv=IMemDrv::createInstance();
                mpImemDrv->init();
                NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
                mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
                mpIspDrv->init("pass2_loopTest");
                QParams enqueParams;
                enqueParams.mvPrivaData.push_back(NULL);
                    enqueParams.mvMagicNo.push_back(0);
                    void *pTuningQueBuf;
                isp_reg_t *pIspPhyReg;
                MUINT32 size;
                MRect p1SrcCrop;
                MSize p1Dst;
                MRect p1DstCrop;
                ////
                ISP_WRITE_BITS(mpIspDrv,CAM_CTL_SPARE3,INT_MRG, 1, ISP_DRV_USER_ISPF);

                NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pCCStream;
                //object init
                pCCStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("pass2_loopTest", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, bTuning);
                pCCStream->init();
                //////////////////////////
                //prepare dma data, imgi
                IMEM_BUF_INFO imgiBuf;
                MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
                IImageBuffer* tempBuffer;
                MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
                    imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
                mpImemDrv->allocVirtBuf(&imgiBuf);
                    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
                    //imem buffer 2 image heap
                    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
                    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                                    MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
                    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( "pass2_loopTest", imgParam,portBufInfo,true);
                tempBuffer = pHeap->createImageBuffer();
                    tempBuffer->incStrong(tempBuffer);
                    tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    //
                p1SrcCrop.s.w = 1280;
                p1SrcCrop.s.h =  720;
                p1Dst.w =1290;
                p1Dst.h =  720;
                p1DstCrop.p.x =  0;
                p1DstCrop.p.y =  0;
                enqueParams.mvP1SrcCrop.push_back(p1SrcCrop);
                enqueParams.mvP1Dst.push_back(p1Dst);
                enqueParams.mvP1DstCrop.push_back(p1DstCrop);
                    //
                Input src;
                src.mPortID=IMGI;
                src.mBuffer=tempBuffer;
                src.mPortID.group=0;
                enqueParams.mvIn.push_back(src);
                //////////////////////////
                ::sem_post(&mSem);
                for(int i=0;i<10;i++)
                {
                    //clear
                    enqueParams.mvCropRsInfo.clear();
                    enqueParams.mvOut.clear();
                    enqueParams.mpTuningData.clear();
                    //setting
                    int out_w=0,out_h=0;
                    MCrpRsInfo crop;
                    crop.mGroupID=1;
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=0;
                    crop.mCropRect.p_integral.y=0;
                    crop.mCropRect.s.w=1280;
                    crop.mCropRect.s.h=720;
                    crop.mResizeDst.w=1280;
                    crop.mResizeDst.h=720;
                    enqueParams.mvCropRsInfo.push_back(crop);
                    //
                    MCrpRsInfo crop_2;
                    crop_2.mGroupID=2;
                    crop_2.mCropRect.p_fractional.x=0;
                    crop_2.mCropRect.p_fractional.y=0;
                    crop_2.mCropRect.p_integral.x=0;
                    crop_2.mCropRect.p_integral.y=0;
                    crop_2.mCropRect.s.w=1280;
                    crop_2.mCropRect.s.h=720;
                    crop_2.mResizeDst.w=1280;
                    crop_2.mResizeDst.h=720;
                    enqueParams.mvCropRsInfo.push_back(crop_2);
                    //
                    out_w=1280;
                    out_h=720;

                    //////////////////////////
                    //prepare dma data, wdmao
                    IMEM_BUF_INFO wdmaoBuf;
                    wdmaoBuf.size=out_w*out_h*2;
                        mpImemDrv->allocVirtBuf(&wdmaoBuf);
                    memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
                    MUINT32 bufStridesInBytes_2[3] = {out_w, out_w/2, out_w/2};
                    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                        MSize(out_w, out_h),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
                    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( "pass2_loopTest", imgParam_2,portBufInfo_2,true);
                    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
                        outBuffer->incStrong(outBuffer);
                        outBuffer->lockBuf("pass2_loopTest",eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst;
                    dst.mPortID=WDMAO;
                    dst.mBuffer=outBuffer;
                    dst.mPortID.group=0;
                    enqueParams.mvOut.push_back(dst);
                    //prepare dma data, wroto
                    IMEM_BUF_INFO wrotoBuf;
                    wrotoBuf.size=out_w*out_h*2;
                        mpImemDrv->allocVirtBuf(&wrotoBuf);
                    memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
                    MUINT32 bufStridesInBytes_4[3] = {out_w,out_w/2,out_w/2};
                    PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
                        IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                                        MSize(out_w, out_h),  bufStridesInBytes_4, bufBoundaryInBytes, 3);
                    sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( "pass2_loopTest", imgParam_4,portBufInfo_4,true);
                    IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
                        outBuffer3->incStrong(outBuffer3);
                        outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
                    Output dst3;
                    dst3.mPortID=WROTO;
                    dst3.mBuffer=outBuffer3;
                    dst3.mPortID.group=0;

                    enqueParams.mvOut.push_back(dst3);
                    //
                    if(bTuning)
                    {
                        pCCStream->deTuningQue(size, pTuningQueBuf);
                        pIspPhyReg = (isp_reg_t *)pTuningQueBuf;
                        applyTuningSetting(1, &pIspPhyReg);
                        enqueParams.mpTuningData.push_back(pTuningQueBuf);
                    }
                    //
                    mpImemDrv->cacheFlushAll();
                    pCCStream->enque(enqueParams);

                    if(bTuning)
                    {
                        // return tuningque buffer
                            pCCStream->enTuningQue(pTuningQueBuf);
                        }

                    QParams dequeParams;
                    pCCStream->deque(dequeParams);
                    ///////////////////////////
                    printf("dequemvOutsize(%d)\n",dequeParams.mvOut.size());
                    //save image
                    char filename[256];
                        sprintf(filename, "/sdcard/Pictures/P2_case_%d_type_%d_pass2loop_%d_wdmao%dx%d.yuv",casenum,type,i, out_w,out_h);
                        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), out_w *out_h * 2);
                        char filename_2[256];
                        sprintf(filename_2, "/sdcard/Pictures/P2_case_%d_type_%d_pass2loop_%d_wroto%dx%d.yuv",casenum,type,i, out_w,out_h);
                        saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(dequeParams.mvOut[1].mBuffer->getBufVA(0)), out_w *out_h * 2);
                        //unmap and free buffer
                        outBuffer->unlockBuf("pass2_loopTest");
                        outBuffer3->unlockBuf("pass2_loopTest");
                        mpImemDrv->freeVirtBuf(&wdmaoBuf);
                        mpImemDrv->freeVirtBuf(&wrotoBuf);
                }
                    LOG_INF("pass2_loopTest loop end");
                //destroy
                    pCCStream->uninit();
                pCCStream->destroyInstance("pass2_loopTest");
                mpImemDrv->uninit();
                mpImemDrv->destroyInstance();
                mpIspDrv->uninit("pass2_loopTest");
                mpIspDrv->destroyInstance();
                ::sem_wait(&mSem_ThreadEnd);
            }
            break;
        default:
            break;
    }
    return ret;
}

/*******************************************************************************
********************************************************************************/
int pass2_multiEnqueTest(int casenum, int type)
{
    int ret=0;
    #if 0
    MBOOL isV3 = MFALSE;    //temp disable tuning path
    LOG_INF("enter pass2_loopTest");

    //
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
    mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
    mpIspDrv->init("pass2_loopTest");
    ////
    ISP_WRITE_BITS(mpIspDrv,CAM_CTL_SPARE3,INT_MRG, 1, ISP_DRV_USER_ISPF);

    NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pCCStream;
    //object init
    pCCStream= NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance("pass2_multiEnqueTest", NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0, isV3);
    pCCStream->init();

    //////////////////////////////////////////
    //update burstQ number to 2
    ret=pCCStream->sendCommand(0x1115,2,0);
    if(ret==false)
    {
        LOG_ERR("update burstQ number fail");
    }
    //////////////////////////////////////////

    #if 0 //do not support
    //
    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvPrivaData.push_back(NULL);
        enqueParams.mvMagicNo.push_back(0);
        enqueParams.mvMagicNo.push_back(1);

    //////////////////////////
    //prepare dma data, imgi_1
    IMEM_BUF_INFO imgiBuf;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBuffer* tempBuffer;
    MUINT32 bufStridesInBytes[3] = {800, 0, 0};
        imgiBuf.size=sizeof(g_imgi_array_640x480_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_640x480_b10), imgiBuf.size);
        //imem buffer 2 image heap
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                        MSize(640, 480), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( "pass2_multiEnqueTest", imgParam,portBufInfo,true);
    tempBuffer = pHeap->createImageBuffer();
        tempBuffer->incStrong(tempBuffer);
        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        //
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=tempBuffer;
    src.mPortID.group=0;
    enqueParams.mvIn.push_back(src);

    //imgi_2
    IMEM_BUF_INFO imgiBuf_2;
    MINT32 bufBoundaryInBytes_2[3] = {0, 0, 0};
    IImageBuffer* tempBuffer_2;
    MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
        imgiBuf_2.size=sizeof(g_imgi_array_640x480_yuy2);
    mpImemDrv->allocVirtBuf(&imgiBuf_2);
        memcpy( (MUINT8*)(imgiBuf_2.virtAddr), (MUINT8*)(g_imgi_array_640x480_yuy2), imgiBuf_2.size);
        //imem buffer 2 image heap
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( imgiBuf_2.memID,imgiBuf.virtAddr,0,imgiBuf_2.bufSecu, imgiBuf_2.bufCohe);
        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                        MSize(640, 480), bufStridesInBytes_2, bufBoundaryInBytes_2, 1);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( "pass2_multiEnqueTest", imgParam_2,portBufInfo_2,true);
    tempBuffer_2 = pHeap->createImageBuffer();
        tempBuffer_2->incStrong(tempBuffer_2);
        tempBuffer_2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Input src_2;
    src_2.mPortID=IMGI;
    src_2.mBuffer=tempBuffer_2;
    src_2.mPortID.group=1;
    enqueParams.mvIn.push_back(src_2);
        //////////////////////////
    LOG_INF("pass2_multiEnque start");
        for(int i=0;i<1;i++)
        {
            //clear
        enqueParams.mvCropRsInfo.clear();
        enqueParams.mvOut.clear();

        //crop information
        MCrpRsInfo crop;
        crop.mGroupID=1;
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=640;
        crop.mCropRect.s.h=480;
        crop.mResizeDst.w=640;
        crop.mResizeDst.h=480;
        enqueParams.mvCropRsInfo.push_back(crop);
        MCrpRsInfo crop_2;
        crop_2.mGroupID=1;
        crop_2.mCropRect.p_fractional.x=0;
        crop_2.mCropRect.p_fractional.y=0;
        crop_2.mCropRect.p_integral.x=100;
        crop_2.mCropRect.p_integral.y=100;
        crop_2.mCropRect.s.w=400;
        crop_2.mCropRect.s.h=300;
        crop_2.mResizeDst.w=640;
        crop_2.mResizeDst.h=480;
        enqueParams.mvCropRsInfo.push_back(crop_2);

        //output dma preparation
        IMEM_BUF_INFO img2oBuf;
        img2oBuf.size=640*480*2;
            mpImemDrv->allocVirtBuf(&img2oBuf);
        memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
        MUINT32 bufStridesInBytes[3] = {1280*2, 0, 0};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create("pass2_multiEnqueTest", imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf("pass2_loopTest",eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=IMG2O;
        dst.mBuffer=outBuffer;
        dst.mPortID.group=0;
        enqueParams.mvOut.push_back(dst);
        //
        IMEM_BUF_INFO img2oBuf_2;
        img2oBuf_2.size=640*480*2;
            mpImemDrv->allocVirtBuf(&img2oBuf_2);
        memset((MUINT8*)img2oBuf_2.virtAddr, 0xffffffff, img2oBuf_2.size);
        MUINT32 bufStridesInBytes_3[3] = {1280*2, 0, 0};
        PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( img2oBuf_2.memID,img2oBuf_2.virtAddr,0,img2oBuf_2.bufSecu, img2oBuf_2.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(640, 480),  bufStridesInBytes_3, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create("pass2_multiEnqueTest", imgParam_3,portBufInfo_3,true);
        IImageBuffer* outBuffer_2 = pHeap_3->createImageBuffer();
            outBuffer_2->incStrong(outBuffer_2);
            outBuffer_2->lockBuf("pass2_loopTest",eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst_2;
        dst_2.mPortID=IMG2O;
        dst_2.mBuffer=outBuffer;
        dst_2.mPortID.group=1;
        enqueParams.mvOut.push_back(dst_2);
        ////////////////////////////
        mpImemDrv->cacheFlushAll();
        pCCStream->enque(enqueParams);
        QParams dequeParams;
        pCCStream->deque(dequeParams);
        ///////////////////////////
        //save image
        char filename[256];
            sprintf(filename, "/sdcard/Pictures/multiEnque_pass2_%d_0_img2o%dx%d.yuv",i, 640,480);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 640 *480 * 2);
            char filename_2[256];
            sprintf(filename_2, "/sdcard/Pictures/multiEnque_pass2_%d_1_img2o%dx%d.yuv",i, 640,480);
            saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(dequeParams.mvOut[1].mBuffer->getBufVA(0)), 640 *480 * 2);
            //unmap and free buffer
            outBuffer->unlockBuf("pass2_loopTest");
            mpImemDrv->freeVirtBuf(&img2oBuf);

    }
        //revert burstQ number
        pCCStream->sendCommand(0x1115,1,0);
    LOG_INF("pass2_multiEnque end");
    #endif
    //destroy
        pCCStream->uninit();
    pCCStream->destroyInstance("pass2_loopTest");
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit("pass2_loopTest");
    mpIspDrv->destroyInstance();
    #endif
    return ret;
}

/*******************************************************************************
********************************************************************************/
void *Vss_Thread(void *data)
{
    int threadData;
    MBOOL isV3 = MTRUE;
    threadData = *((int*)data);
    int wrotoW, wrotoH;
    int cropW, cropH;
    int imageW, imageH;

    MRect p1SrcCrop;
    MSize p1Dst;
    MRect p1DstCrop;
    //p1 private data
    p1SrcCrop.s.w = 2560;
    p1SrcCrop.s.h = 1440;
    p1Dst.w = 2560;
    p1Dst.h = 1440;
    p1DstCrop.p.x = 0;
    p1DstCrop.p.y = 0;
    //
    LOG_INF("VSS thread threadData(%d)\n",threadData);
    //
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //
    NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;
    mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
    mpIspDrv->init(LOG_TAG);
    //
    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvMagicNo.push_back(0);
    enqueParams.mvP1SrcCrop.push_back(p1SrcCrop);
    enqueParams.mvP1Dst.push_back(p1Dst);
    enqueParams.mvP1DstCrop.push_back(p1DstCrop);
    //
    //objects
    NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pStream = NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance(LOG_TAG,
        NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Vss,0xFFFF, isV3);

    pStream->init();

    ///////
    //create input
    IMEM_BUF_INFO imgiBuf;
    imgiBuf.size=sizeof(g_imgi_array_2560x1440_b10);

    LOG_INF("size(0x%x)\n",imgiBuf.size);
    //
    imageW = 2560;
    imageH = 1440;
    cropW = 2560;
    cropH = 1440;
    wrotoW = 2560;
    wrotoH = 1440;
    mpImemDrv->allocVirtBuf(&imgiBuf);
    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_2560x1440_b10), imgiBuf.size);
    // crop
    MCrpRsInfo crop;
    crop.mGroupID=2;
    crop.mCropRect.p_fractional.x=0;
    crop.mCropRect.p_fractional.y=0;
    crop.mCropRect.p_integral.x=0;
    crop.mCropRect.p_integral.y=0;
    crop.mCropRect.s.w=cropW;
    crop.mCropRect.s.h=cropH;
    crop.mResizeDst.w=wrotoW;
    crop.mResizeDst.w=wrotoH;
    enqueParams.mvCropRsInfo.push_back(crop);
    //
    void *pTuningQueBuf;
    isp_reg_t *pIspPhyReg;
    MUINT32 size;

    pStream->deTuningQue(size, pTuningQueBuf);
    pIspPhyReg = (isp_reg_t *)pTuningQueBuf;
    applyTuningSetting(0, &pIspPhyReg);
    LOG_INF("CAM_CTL_EN1(0x%08x),CAM_CTL_EN2(0x%08x)",pIspPhyReg->CAM_CTL_EN1.Raw,pIspPhyReg->CAM_CTL_EN2.Raw);
    enqueParams.mpTuningData.push_back(pTuningQueBuf);
    // in/out port
    //imem buffer 2 image heap
    MUINT32 bufStridesInBytes[3] = {imageW*10/8, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                    MSize(imageW, imageH), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    IImageBuffer* tempBuffer = pHeap->createImageBuffer();
    tempBuffer->incStrong(tempBuffer);
    tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    //
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=tempBuffer;
    enqueParams.mvIn.push_back(src);
    //
    IMEM_BUF_INFO wrotoBuf;
    wrotoBuf.size=wrotoW*wrotoH*2;
    mpImemDrv->allocVirtBuf(&wrotoBuf);
    memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
    MUINT32 bufStridesInBytes_2[3] = {wrotoW, wrotoW>>1,wrotoW>>1};
    PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                        MSize(wrotoW, wrotoH),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
    sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
    IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
        outBuffer->incStrong(outBuffer);
        outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Output dst;
    dst.mPortID=WROTO;
    dst.mBuffer=outBuffer;
    enqueParams.mvOut.push_back(dst);
    //
    mpImemDrv->cacheFlushAll();
    //
    //enque
    pStream->enque(enqueParams);
    LOG_INF("VSS ENQUE DONE");

    // return tuningque buffer
    pStream->enTuningQue(pTuningQueBuf);

    ////
    //deque
    QParams dequeParams;
    pStream->deque(enqueParams);
    LOG_INF("VSS DEQUE DONE");
    // dump image
    char filename[256];
    sprintf(filename, "/sdcard/Pictures/VSS_P2_%d_wroto%dx%d.yuv", threadData, wrotoW, wrotoH);
    saveBufToFile(filename, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), wrotoW *wrotoH * 2);
    //
    pStream->uninit();
    pStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit(LOG_TAG);
    mpIspDrv->destroyInstance();

    return NULL;
}


/*******************************************************************************
casenum(1) : in:imgi(1280x720)(raw10)  / out:wdmao(1280x720)
casenum(2) : in:imgi(1280x720)(raw10)  / out:img2o(1280x720) ==> ISP only (Not Ready Yet)
casenum(3) : in:imgi(1280x720)(raw10)  / out:crop(1000,562)==>wroto(1000x562) + wroto(1000x562)
casenum(4) :(thread1) in:imgi(1280x720) (raw10) / out:wroto(1280x720)
            (thread2) in:imgi(2560x1440)(raw10) / out:(VSS)wroto(2560x1440)
casenum(5) : in:imgi(2560x1440)(raw10)  / out:wroto(2560x1440)



********************************************************************************/
int pass2_TuningTest(int casenum)
{
#define P2_TUNINGQUE_NUM    10
    int ret=0;
    MBOOL isV3 = MTRUE;
    MUINT32 size;
    void *pTuningQueBuf[P2_TUNINGQUE_NUM];
    isp_reg_t *pIspPhyReg[P2_TUNINGQUE_NUM];
    pthread_t pThread[2];
    int threadId;
    int threadData;
    int threadStatus;
    int wrotoW, wrotoH, wdmaoW, wdmaoH;
    int cropW, cropH;
    int imageW, imageH;
    MRect p1SrcCrop;
    MSize p1Dst;
    MRect p1DstCrop;

    //crop
    MCrpRsInfo crop;
    crop.mGroupID=1;
    MCrpRsInfo crop2;
    crop2.mGroupID=2;

    //p1 private data
    p1SrcCrop.s.w = 1280;
    p1SrcCrop.s.h = 720;
    p1Dst.w = 1260;
    p1Dst.h = 708;
    p1DstCrop.p.x = 10;
    p1DstCrop.p.y = 6;

    //objects
    NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream* pStream = NSCam::NSIoPipe::NSPostProc_FrmB::INormalStream::createInstance(LOG_TAG,
        NSCam::NSIoPipe::NSPostProc_FrmB::ENormalStreamTag_Stream,0xFFFF, isV3);

    LOG_INF("casenum(%d)\n",casenum);

    pStream->init();

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    NSIspDrv_FrmB::IspDrv* mpIspDrv=NULL;

    mpIspDrv=NSIspDrv_FrmB::IspDrv::createInstance();
    mpIspDrv->init(LOG_TAG);

    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvMagicNo.push_back(0);
    enqueParams.mvP1SrcCrop.push_back(p1SrcCrop);
    enqueParams.mvP1Dst.push_back(p1Dst);
    enqueParams.mvP1DstCrop.push_back(p1DstCrop);

    ///////


    if(casenum==1) {
        imageW = 1280;
        imageH = 720;
        //
        wrotoW = 1280;
        wrotoH = 720;
        wdmaoW = 1280;
        wdmaoH = 720;
        cropW = 1280;
        cropH = 720;
    } else if(casenum==2) {
        imageW = 1280;
        imageH = 720;
        //
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=1000;
        crop.mCropRect.s.h=600;
        crop.mResizeDst.w=1000;
        crop.mResizeDst.w=600;
    } else if(casenum==3) {
        imageW = 1280;
        imageH = 720;
        //
        cropW = 1000;
        cropH = 562;
        //
        wrotoW = 1000;
        wrotoH = 562;
        //
        wdmaoW = 1000;
        wdmaoH = 562;
        //
    } else if(casenum==4) {
        imageW = 1280;
        imageH = 720;
        //
        cropW = 1280;
        cropH = 720;
        //
        wrotoW = 1280;
        wrotoH = 720;
        //
    } else if(casenum==5) {
        imageW = 2560;
        imageH = 1440;
        //
        cropW = 2560;
        cropH = 1440;
        //
        wrotoW = 2560;
        wrotoH = 1440;
        //
    }

    //create input
    IMEM_BUF_INFO imgiBuf;

    if(imageW==1280 && imageH==720){
        imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
        mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
    } else if(imageW==2560 && imageH==1440){
        imgiBuf.size=sizeof(g_imgi_array_2560x1440_b10);
        mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_2560x1440_b10), imgiBuf.size);
    } else {
        LOG_ERR("[Error]Not set the input image size");
        return -1;
    }
    LOG_INF("size(0x%x)\n",imgiBuf.size);

    //imem buffer 2 image heap
    MUINT32 bufStridesInBytes[3] = {imageW*10/8, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
    IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                    MSize(imageW, imageH), bufStridesInBytes, bufBoundaryInBytes, 1);
    sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    IImageBuffer* tempBuffer = pHeap->createImageBuffer();
    tempBuffer->incStrong(tempBuffer);
    tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    //
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=tempBuffer;
    enqueParams.mvIn.push_back(src);


    crop2.mCropRect.p_fractional.x=0;
    crop2.mCropRect.p_fractional.y=0;
    crop2.mCropRect.p_integral.x=(imageW-wrotoW)>>1;
    crop2.mCropRect.p_integral.y=(imageH-wrotoH)>>1;
    crop2.mCropRect.s.w=cropW;
    crop2.mCropRect.s.h=cropH;
    crop2.mResizeDst.w=wrotoW;
    crop2.mResizeDst.w=wrotoH;

    enqueParams.mvCropRsInfo.push_back(crop);
    enqueParams.mvCropRsInfo.push_back(crop2);

    /////////
    //output buffer
    IMEM_BUF_INFO wdmaoBuf;
    IMEM_BUF_INFO wrotoBuf;
    IMEM_BUF_INFO img2oBuf;

    if(casenum==1){
        //wdmao
        wdmaoBuf.size=wdmaoW*wdmaoH*2;
        mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {wdmaoW, wdmaoW>>1, wdmaoW>>1};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdmaoW, wdmaoH),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=WDMAO;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);
    } else if (casenum==2){
        img2oBuf.size=640*480*2;
            mpImemDrv->allocVirtBuf(&img2oBuf);
        memset((MUINT8*)img2oBuf.virtAddr, 0xffffffff, img2oBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {1280, 0, 0};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(640, 480),  bufStridesInBytes_2, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=IMG2O;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);
    } else if (casenum==3){
        //wdmao(1000x562)
        wdmaoBuf.size=wdmaoW*wdmaoH*2;
        mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_1[3] = {wdmaoW, wdmaoW>>1, wdmaoW>>1};
        PortBufInfo_v1 portBufInfo_1 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_1 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdmaoW, wdmaoH),  bufStridesInBytes_1, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_1 = ImageBufferHeap::create( LOG_TAG, imgParam_1,portBufInfo_1,true);
        IImageBuffer* outBuffer = pHeap_1->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=WDMAO;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);

        //wroto(1000x562)
        wrotoBuf.size=wrotoW*wrotoH*2;
        mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {wrotoW, wrotoW>>1, wrotoW>>1};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wrotoW, wrotoH),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer2 = pHeap_2->createImageBuffer();
            outBuffer2->incStrong(outBuffer2);
            outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst2;
        dst2.mPortID=WROTO;
        dst2.mBuffer=outBuffer2;
        enqueParams.mvOut.push_back(dst2);
    } else if (casenum==4){
        //wroto
        wrotoBuf.size=wrotoW*wrotoH*2;
        mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {wrotoW, wrotoW>>1, wrotoW>>1};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wrotoW, wrotoH),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=WROTO;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);
        //
        // create another thread for VSS
        threadData = casenum;
        threadId = pthread_create(&pThread[0], NULL, Vss_Thread, (void *)&threadData);
        if (threadId < 0)
        {
            LOG_ERR("thread create error(%d)",threadId);
            return -1;
        }
    } else if (casenum==5){
        //wroto
        wrotoBuf.size=wrotoW*wrotoH*2;
        mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {wrotoW, wrotoW>>1, wrotoW>>1};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wrotoW, wrotoH),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=WROTO;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);
    }


    // test detuningQue and enTuningQue
    for(MUINT32 i=0;i<P2_TUNINGQUE_NUM;i++){
        pStream->deTuningQue(size, pTuningQueBuf[i]);
        LOG_INF("i(%d),size(0x%x),pTuningQueBuf(0x%x)\n",i,size,pTuningQueBuf[i]);
    }

    pIspPhyReg[0] = (isp_reg_t *)pTuningQueBuf[0];

    applyTuningSetting(0, &pIspPhyReg[0]);
    LOG_INF("CAM_CTL_EN1(0x%08x),CAM_CTL_EN2(0x%08x)",pIspPhyReg[0]->CAM_CTL_EN1.Raw,pIspPhyReg[0]->CAM_CTL_EN2.Raw);

    enqueParams.mpTuningData.push_back(pTuningQueBuf[0]);

    mpImemDrv->cacheFlushAll();

    LOG_INF("enque");

    //enque
    pStream->enque(enqueParams);
    LOG_INF("ENQUE DONE");


    for(MUINT32 i=0;i<P2_TUNINGQUE_NUM;i++){
        LOG_INF("i(%d),pTuningQueBuf(0x%x)\n",i,pTuningQueBuf[i]);
        pStream->enTuningQue(pTuningQueBuf[i]);
    }

    ////
    //deque
    QParams dequeParams;
    pStream->deque(enqueParams);
    LOG_INF("DEQUE DONE");


    ////
    //dump pic to watch
    if(casenum==1)
    {
        char filename[256];
            sprintf(filename, "/sdcard/Pictures/basicP2_%d_wdmao%dx%d.yuv", casenum, wdmaoW,wdmaoH);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), wdmaoW *wdmaoH * 2);
    } else if(casenum==2) {
        char filename[256];
            sprintf(filename, "/sdcard/Pictures/basicP2_%d_disp%dx%d.yuv", casenum, 640,480);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 2);
            char filename2[256];
            sprintf(filename2, "/sdcard/Pictures/basicP2_%d_vido%dx%d.yuv", casenum, 1280,720);
            saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), 1280 *720 * 2);
    } else if(casenum==3) {
        char filename[256];
        sprintf(filename, "/sdcard/Pictures/basicP2_%d_wdmao%dx%d.yuv", casenum, wdmaoW,wdmaoH);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), wdmaoW *wdmaoH * 2);
        //
        char filename2[256];
        sprintf(filename2, "/sdcard/Pictures/basicP2_%d_wroto%dx%d.yuv", casenum, wrotoW,wrotoH);
        saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), wrotoW *wrotoH * 2);
    } else if(casenum==4) {
        char filename[256];
            sprintf(filename, "/sdcard/Pictures/basicP2_%d_wroto%dx%d.yuv", casenum, wrotoW,wrotoH);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), wrotoW *wrotoH * 2);
        //
        #if 1  //kk test default:1
        pthread_join(pThread[0], (void **)&threadStatus);  // wait for pThread[0] termination
        #endif
    } else if(casenum==5) {
        char filename[256];
            sprintf(filename, "/sdcard/Pictures/basicP2_%d_wroto%dx%d.yuv", casenum, wrotoW,wrotoH);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), wrotoW *wrotoH * 2);
    }

    pStream->uninit();
    pStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit(LOG_TAG);
    mpIspDrv->destroyInstance();

    LOG_INF("-,pass2_TuningTest\n");

    return ret;
}

int applyTuningSetting(int mode, isp_reg_t **ppIspPhyReg)
{
    int ret = 0;
    isp_reg_t *pIspPhyReg = (*ppIspPhyReg);
    MUINT32* pIspSetting = (MUINT32*)pIspPhyReg;

    if(mode==0){
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.OB_EN=0;

        #if 1
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.LSC_EN=0;
        #else
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.LSC_EN=1;

        ppIspPhyReg[0]->CAM_CTL_DMA_EN.Bits.LSCI_EN=1;

        pIspSetting[0x4274>>2] = 0x00000100;
        pIspSetting[0x4278>>2] = 0x00000200;
        pIspSetting[0x427c>>2] = 0x00000300;
        //
        pIspSetting[0x4530>>2] = 0x00000400;
        pIspSetting[0x4534>>2] = 0x00000500;
        pIspSetting[0x454C>>2] = 0x00000600;
        #endif


        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.BNR_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.SL2_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.PGN_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.CCL_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.GGM_EN=0;
        //
        ppIspPhyReg[0]->CAM_CTL_EN2.Bits.NBC_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN2.Bits.PCA_EN=0;
        ppIspPhyReg[0]->CAM_CTL_EN2.Bits.SEEE_EN=0;
        //
        //G2G
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.G2G_EN=1;
        pIspSetting[0x4920>>2] = 0x00000200;
        pIspSetting[0x4924>>2] = 0x00000000;
        pIspSetting[0x4928>>2] = 0x02000000;
        pIspSetting[0x492C>>2] = 0x00000000;
        pIspSetting[0x4930>>2] = 0x00000000;
        pIspSetting[0x4934>>2] = 0x00000200;
        pIspSetting[0x4938>>2] = 0x00000009;
        //
        //CFA
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.CFA_EN=1;
        pIspSetting[0x48A0>>2] = 0x00000000;
        pIspSetting[0x48A4>>2] = 0x00000C03;
        pIspSetting[0x48A8>>2] = 0x01082006;
        pIspSetting[0x48AC>>2] = 0x80081008;
        pIspSetting[0x48B0>>2] = 0x37084208;
        pIspSetting[0x48B4>>2] = 0x1806200A;
        pIspSetting[0x48B8>>2] = 0x0010A020;
        pIspSetting[0x48BC>>2] = 0x0015C020;
        pIspSetting[0x48C0>>2] = 0x0015C040;
        pIspSetting[0x48C4>>2] = 0x00350050;
        pIspSetting[0x48C8>>2] = 0x00A41440;
        pIspSetting[0x48CC>>2] = 0x00421084;
        pIspSetting[0x48D0>>2] = 0x01484185;
        pIspSetting[0x48D4>>2] = 0x00410417;
        pIspSetting[0x48D8>>2] = 0x000203FF;
        pIspSetting[0x48DC>>2] = 0x00000008;
        pIspSetting[0x48E0>>2] = 0xE0088888;
        pIspSetting[0x48E4>>2] = 0x00000010;
        pIspSetting[0x48E8>>2] = 0x0052A5FF;
        pIspSetting[0x48F0>>2] = 0x781F55D4;
        pIspSetting[0x48F4>>2] = 0x00018006;
        //
        //G2C
        ppIspPhyReg[0]->CAM_CTL_EN2.Bits.G2C_EN=1;
        pIspSetting[0x4A00>>2] = 0x012D0099;
        pIspSetting[0x4A04>>2] = 0x0000003A;
        pIspSetting[0x4A08>>2] = 0x075607AA;
        pIspSetting[0x4A0C>>2] = 0x00000100;
        pIspSetting[0x4A10>>2] = 0x072A0100;
        pIspSetting[0x4A14>>2] = 0x000007D6;
    }
    else if(mode==1){
    ppIspPhyReg[0]->CAM_CTL_EN1.Bits.GGM_EN=1;
    pIspSetting[0x5600>>2]= 0x00000001;   /* 0x15005600: CAM_GGM_CTRL */
    pIspSetting[0x5000>>2]= 0x3000F400;   /* 0x15005000: CAM_GGM_RB_GMT[0] */
        pIspSetting[0x5004>>2]=  0x240C5C3D;   /* 0x15005004: CAM_GGM_RB_GMT[1] */
        pIspSetting[0x5008>>2]=  0x1C154454;   /* 0x15005008: CAM_GGM_RB_GMT[2] */
        pIspSetting[0x500C>>2]=  0x181C3865;   /* 0x1500500C: CAM_GGM_RB_GMT[3] */
        pIspSetting[0x5010>>2]=  0x18223073;   /* 0x15005010: CAM_GGM_RB_GMT[4] */
        pIspSetting[0x5014>>2]=  0x14282C7F;   /* 0x15005014: CAM_GGM_RB_GMT[5] */
        pIspSetting[0x5018>>2]=  0x142D288A;   /* 0x15005018: CAM_GGM_RB_GMT[6] */
        pIspSetting[0x501C>>2]=  0x14322494;   /* 0x1500501C: CAM_GGM_RB_GMT[7] */
        pIspSetting[0x5020>>2]=  0x1437249D;   /* 0x15005020: CAM_GGM_RB_GMT[8] */
        pIspSetting[0x5024>>2]=  0x143C20A6;   /* 0x15005024: CAM_GGM_RB_GMT[9] */
        pIspSetting[0x5028>>2]=  0x10411CAE;   /* 0x15005028: CAM_GGM_RB_GMT[10] */
        pIspSetting[0x502C>>2]=  0x144520B5;   /* 0x1500502C: CAM_GGM_RB_GMT[11] */
        pIspSetting[0x5030>>2]=  0x104A1CBD;   /* 0x15005030: CAM_GGM_RB_GMT[12] */
        pIspSetting[0x5034>>2]=  0x104E18C4;   /* 0x15005034: CAM_GGM_RB_GMT[13] */
        pIspSetting[0x5038>>2]=  0x10521CCA;   /* 0x15005038: CAM_GGM_RB_GMT[14] */
        pIspSetting[0x503C>>2]=  0x105618D1;   /* 0x1500503C: CAM_GGM_RB_GMT[15] */
        pIspSetting[0x5040>>2]=  0x105A18D7;   /* 0x15005040: CAM_GGM_RB_GMT[16] */
        pIspSetting[0x5044>>2]=  0x105E14DD;   /* 0x15005044: CAM_GGM_RB_GMT[17] */
        pIspSetting[0x5048>>2]=  0x106218E2;   /* 0x15005048: CAM_GGM_RB_GMT[18] */
        pIspSetting[0x504C>>2]=  0x0C6618E8;   /* 0x1500504C: CAM_GGM_RB_GMT[19] */

        pIspSetting[0x5050>>2]=  0x0;   /* 0x15005050: CAM_GGM_RB_GMT[20] */
        pIspSetting[0x5054>>2]=  0x0;   /* 0x15005054: CAM_GGM_RB_GMT[21] */
        pIspSetting[0x5058>>2]=  0x0;   /* 0x15005058: CAM_GGM_RB_GMT[22] */
        pIspSetting[0x505C>>2]=  0x0;   /* 0x1500505C: CAM_GGM_RB_GMT[23] */
        pIspSetting[0x5060>>2]=  0x0;   /* 0x15005060: CAM_GGM_RB_GMT[24] */
    pIspSetting[0x5064>>2]=0x0 ;   /* 0x15005064: CAM_GGM_RB_GMT[25] */
        pIspSetting[0x5068>>2]=0x0 ;   /* 0x15005068: CAM_GGM_RB_GMT[26] */
        pIspSetting[0x506C>>2]=0x0 ;   /* 0x1500506C: CAM_GGM_RB_GMT[27] */
        pIspSetting[0x5070>>2]=0x0 ;   /* 0x15005070: CAM_GGM_RB_GMT[28] */
        pIspSetting[0x5074>>2]=0x0 ;   /* 0x15005074: CAM_GGM_RB_GMT[29] */
        pIspSetting[0x5078>>2]=0x0 ;   /* 0x15005078: CAM_GGM_RB_GMT[30] */
        pIspSetting[0x507C>>2]=0x0 ;   /* 0x1500507C: CAM_GGM_RB_GMT[31] */
        pIspSetting[0x5080>>2]=0x0 ;   /* 0x15005080: CAM_GGM_RB_GMT[32] */
        pIspSetting[0x5084>>2]=0x0 ;   /* 0x15005084: CAM_GGM_RB_GMT[33] */
        pIspSetting[0x5088>>2]=0x0 ;   /* 0x15005088: CAM_GGM_RB_GMT[34] */
        pIspSetting[0x508C>>2]=0x0 ;   /* 0x1500508C: CAM_GGM_RB_GMT[35] */

    }
    else if(mode==2){
        ppIspPhyReg[0]->CAM_CTL_EN1.Bits.GGM_EN=1;
        pIspSetting[0x5600>>2]= 0x00000001;   /* 0x15005600: CAM_GGM_CTRL */

        pIspSetting[0x5000>>2]= 0x0;   /* 0x15005000: CAM_GGM_RB_GMT[0] */
        pIspSetting[0x5004>>2]=  0x0;   /* 0x15005004: CAM_GGM_RB_GMT[1] */
        pIspSetting[0x5008>>2]=  0x0;   /* 0x15005008: CAM_GGM_RB_GMT[2] */
        pIspSetting[0x500C>>2]=  0x0;   /* 0x1500500C: CAM_GGM_RB_GMT[3] */
        pIspSetting[0x5010>>2]=  0x0;   /* 0x15005010: CAM_GGM_RB_GMT[4] */
        pIspSetting[0x5014>>2]=  0x0;   /* 0x15005014: CAM_GGM_RB_GMT[5] */
        pIspSetting[0x5018>>2]=  0x0;   /* 0x15005018: CAM_GGM_RB_GMT[6] */
        pIspSetting[0x501C>>2]=  0x0;   /* 0x1500501C: CAM_GGM_RB_GMT[7] */
        pIspSetting[0x5020>>2]=  0x0;   /* 0x15005020: CAM_GGM_RB_GMT[8] */
        pIspSetting[0x5024>>2]=  0x0;   /* 0x15005024: CAM_GGM_RB_GMT[9] */
        pIspSetting[0x5028>>2]=  0x0;   /* 0x15005028: CAM_GGM_RB_GMT[10] */
        pIspSetting[0x502C>>2]=  0x0;   /* 0x1500502C: CAM_GGM_RB_GMT[11] */
        pIspSetting[0x5030>>2]=  0x0;   /* 0x15005030: CAM_GGM_RB_GMT[12] */
        pIspSetting[0x5034>>2]=  0x0;   /* 0x15005034: CAM_GGM_RB_GMT[13] */
        pIspSetting[0x5038>>2]=  0x0;   /* 0x15005038: CAM_GGM_RB_GMT[14] */
        pIspSetting[0x503C>>2]=  0x0;   /* 0x1500503C: CAM_GGM_RB_GMT[15] */
        pIspSetting[0x5040>>2]=  0x0;   /* 0x15005040: CAM_GGM_RB_GMT[16] */
        pIspSetting[0x5044>>2]=  0x0;   /* 0x15005044: CAM_GGM_RB_GMT[17] */
        pIspSetting[0x5048>>2]=  0x0;   /* 0x15005048: CAM_GGM_RB_GMT[18] */
        pIspSetting[0x504C>>2]=  0x0;   /* 0x1500504C: CAM_GGM_RB_GMT[19] */

        pIspSetting[0x5050>>2]=  0x106914EE;   /* 0x15005050: CAM_GGM_RB_GMT[20] */
        pIspSetting[0x5054>>2]=  0x106D14F3;   /* 0x15005054: CAM_GGM_RB_GMT[21] */
        pIspSetting[0x5058>>2]=  0x0C7114F8;   /* 0x15005058: CAM_GGM_RB_GMT[22] */
        pIspSetting[0x505C>>2]=  0x107414FD;   /* 0x1500505C: CAM_GGM_RB_GMT[23] */
        pIspSetting[0x5060>>2]=  0x0C781502;   /* 0x15005060: CAM_GGM_RB_GMT[24] */
    pIspSetting[0x5064>>2]=0x107B1107 ;   /* 0x15005064: CAM_GGM_RB_GMT[25] */
        pIspSetting[0x5068>>2]=0x0C7F150B ;   /* 0x15005068: CAM_GGM_RB_GMT[26] */
        pIspSetting[0x506C>>2]=0x0C821110 ;   /* 0x1500506C: CAM_GGM_RB_GMT[27] */
        pIspSetting[0x5070>>2]=0x10851514 ;   /* 0x15005070: CAM_GGM_RB_GMT[28] */
        pIspSetting[0x5074>>2]=0x0C891119 ;   /* 0x15005074: CAM_GGM_RB_GMT[29] */
        pIspSetting[0x5078>>2]=0x0C8C111D ;   /* 0x15005078: CAM_GGM_RB_GMT[30] */
        pIspSetting[0x507C>>2]=0x108F1521 ;   /* 0x1500507C: CAM_GGM_RB_GMT[31] */
        pIspSetting[0x5080>>2]=0x0C931126 ;   /* 0x15005080: CAM_GGM_RB_GMT[32] */
        pIspSetting[0x5084>>2]=0x0C96112A ;   /* 0x15005084: CAM_GGM_RB_GMT[33] */
        pIspSetting[0x5088>>2]=0x0C99112E ;   /* 0x15005088: CAM_GGM_RB_GMT[34] */
        pIspSetting[0x508C>>2]=0x0C9C1132 ;   /* 0x1500508C: CAM_GGM_RB_GMT[35] */
    }



    return ret;
}



