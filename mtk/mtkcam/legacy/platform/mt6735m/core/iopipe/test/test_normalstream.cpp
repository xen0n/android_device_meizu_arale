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

#define LOG_TAG "CamIOpipeTest"

#include <vector>

using namespace std;

//#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
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
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv_common/isp_reg.h>


#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>

using namespace android;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(P2PipeWrapper);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (P2PipeWrapper_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (P2PipeWrapper_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");


static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const PortID DISPO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_DISPO, 1);
static const PortID VIDO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIDO, 1);

///////////////////////////////////////////////
//test cases
int pass2_BasicTest(int casenum);

/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    LOG_INF("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    LOG_INF("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        LOG_ERR("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    LOG_INF("writing %d bytes to file [%s]\n", size, fname);
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
    LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


/*******************************************************************************
*  Postproc 1 IN / 2 Out
********************************************************************************/
int main_testNormalStream(int testcaseType,int testcaseNum)
{
    LOG_INF("+");
    int ret=0;
    switch(testcaseType)
    {
        case 1:
            ret=pass2_BasicTest(testcaseNum);
            break;
        case 2:
            break;
        default:
            break;
    }

    return ret;
}


#include "pic/imgi_1280x720_bayer10.h"

/*******************************************************************************
* case1: single in(640x480 bayer10) / single out(dispo_640x480, yv12)
* case2: single in(640x480 bayer10) / two out(dispo_480x360, yuy2/vido_640x480, yv12)

********************************************************************************/
int pass2_BasicTest(int casenum)
{
    int ret=0;

    //objects
    NSCam::NSIoPipe::NSPostProc::INormalStream* pStream = NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(LOG_TAG,
        NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Stream,0,NSCam::NSIoPipe::NSPostProc::eScenarioFormat_RAW);
    pStream->init();
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    IspDrv* mpIspDrv=NULL;
    mpIspDrv=IspDrv::createInstance();
    mpIspDrv->init();

    QParams enqueParams;
    enqueParams.mpPrivaData = NULL;
        enqueParams.mFrameNo = 0;

    ///////
    //create input
    IMEM_BUF_INFO imgiBuf;
    imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
        //imem buffer 2 image heap
        MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                        MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    IImageBuffer* tempBuffer = pHeap->createImageBuffer();
        tempBuffer->incStrong(tempBuffer);
        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        //
        Input src;
    src.mPortID=IMGI;
    src.mBuffer=tempBuffer;
    enqueParams.mvIn.push_back(src);

    //crop
    MCrpRsInfo crop;
    crop.mGroupID=1;
    crop.mCropRect.p_fractional.x=0;
    crop.mCropRect.p_fractional.y=0;
    crop.mCropRect.p_integral.x=0;
    crop.mCropRect.p_integral.y=0;
    crop.mCropRect.s.w=1280;
    crop.mCropRect.s.h=720;
    crop.mResizeDst.w=1280;
    crop.mResizeDst.w=720;
    if(casenum==1)
    {}
    else if(casenum==2)
    {
        crop.mCropRect.s.w=1000;
        crop.mCropRect.s.h=600;
        crop.mResizeDst.w=1000;
        crop.mResizeDst.w=600;
    }
    enqueParams.mvCropRsInfo.push_back(crop);

    /////////
    //output buffer
    IMEM_BUF_INFO wdmaoBuf;
    IMEM_BUF_INFO wrotoBuf;
    if(casenum==1)
    {
        wdmaoBuf.size=1280*720*2;
            mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_2[3] = {1280, 640, 640};
        PortBufInfo_v1 portBufInfo_2 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_2 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(1280, 720),  bufStridesInBytes_2, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_2 = ImageBufferHeap::create( LOG_TAG, imgParam_2,portBufInfo_2,true);
        IImageBuffer* outBuffer = pHeap_2->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=DISPO;
        dst.mBuffer=outBuffer;
        enqueParams.mvOut.push_back(dst);
    }
    else if(casenum==2)
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
        dst.mPortID=DISPO;
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
        dst2.mPortID=VIDO;
        dst2.mBuffer=outBuffer2;
        enqueParams.mvOut.push_back(dst2);
    }
    else if(casenum==3)
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
        dst.mPortID=DISPO;
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
        dst2.mPortID=VIDO;
        dst2.mBuffer=outBuffer2;
        dst2.mTransform=eTransform_ROT_90;
        enqueParams.mvOut.push_back(dst2);
    }


    /////
    //temp to add, (original done in pass1)
    MUINT32 reg_val=0x0;
    ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
    ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
        reg_val = ISP_IOCTL_READ_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_DMA_EN, 0);
        //disable GGM
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_GGM_CTRL, 0);
        //reset GDMA relative setting
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
        /////

    //enque
    pStream->enque(enqueParams);
    LOG_INF("ENQUE DONE");

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
            sprintf(filename, "/data/basicP2_1_disp%dx%d.yuv", 1280,720);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 1280 *720 * 2);
    }
    else if(casenum==2)
    {
        char filename[256];
            sprintf(filename, "/data/basicP2_2_disp%dx%d.yuv", 640,480);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 2);
            char filename2[256];
            sprintf(filename2, "/data/basicP2_2_vido%dx%d.yuv", 1280,720);
            saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), 1280 *720 * 2);
    }
    else if(casenum==3)
    {
        char filename[256];
            sprintf(filename, "/data/basicP2_3_disp%dx%d.yuv", 640,480);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), 640 *480 * 2);
            char filename2[256];
            sprintf(filename2, "/data/basicP2_3_vido%dx%d.yuv", 640,960);
            saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), 640 *960 * 2);
    }
    ////
    //
    pStream->uninit();
    pStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit();
    mpIspDrv->destroyInstance();

    return ret;
}

