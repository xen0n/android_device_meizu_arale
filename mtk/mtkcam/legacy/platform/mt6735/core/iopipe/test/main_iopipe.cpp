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


#define LOG_TAG "iopipetest"

#include <vector>

#include <sys/time.h>
#include <sys/stat.h>
#include <sys/prctl.h>

#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>

//
#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/iopipe/PostProc/IFeatureStream.h>

#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>
//
using namespace std;
using namespace android;
using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;

//
static const PortID IMGI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGI, 0);
static const PortID UFDI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_UFDI, 0);
static const PortID LCEI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_LCEI, 0);
static const PortID VIPI( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIPI, 0);
static const PortID VIP3I( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_VIP3I, 0);
static const PortID WDMAO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WDMAO, 1);
static const PortID WROTO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_WROTO, 1);
static const PortID IMG2O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG2O, 1);
static const PortID IMG3O( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMG3O, 1);
static const PortID IMGO( NSIoPipe::EPortType_Memory, NSImageio::NSIspio::EPortIndex_IMGO, 1);

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
        printf(": failed to create file [%s]: %s \n", fname, ::strerror(errno));
        return false;
    }

    //LOG_INF("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf(": failed to write to file [%s]: %s\n", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    //LOG_INF("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


#include "pic/imgi_1280x720_bayer10.h"
#include "pic/imgi_2560x1440_bayer10.h"
#include "pic/imgi_320x240_yuyv.h"
#include "pic/alpha1_320x240_bayer8.h"
#include "pic/alpha2_320x240_bayer8.h"
#include "pic/g_imgi_5280x960_yuy2.h"
#include "pic/g_vipi_5280x960_yuy2.h"
#include "pic/g_ufdi_5280x960_b8.h"
#include "pic/g_vip3i_5280x960_b8.h"


/*******************************************************************************
*  Main Function
*
*  case 0 0: preview path, 1 in / 2 out (wdma/wroto) with same src crop and same dst size
*  case 0 1: preview path, 1 in / 2 out (wdma/wroto) with different src crop but same dst size
*  case 0 2: preview path, 1 in / 2 out (wdma/wroto) with different src crop and different dst size(rotate 90 for wroto)
*  case 0 3: preview path, 1 in / 3 out (wdma/wroto/img2o) with different src crop and different dst size
*  case 1 0: vfb path,p2a 1 in / 2 out (img3o/wdmao) with different src crop and different dst size
*                            p2b path, 4 in / 2 out (wdma/wroto)
*  case 2 0: mfb mixing path,4 in / 3 out (img3o/wdmao/wroto) with different src crop and different dst size
********************************************************************************/
int case0(int type);
int case1();
int case2();


int test_iopipe(int testNum,int testType)
{
        int ret = 0;

    switch(testNum)
    {
        case 0:
            ret=case0(testType);
            break;
        case 1:
            ret=case1();
            break;
        case 2:
            ret=case2();
            break;
        default:
            break;
    }

    ret = 1;
        return ret;
}

/*********************************************************************************/
int case0(int type)
{
    int ret=0;
    MBOOL isV3 = MFALSE;    //temp disable tuning path cuz tuning function not ready

    NSCam::NSIoPipe::NSPostProc::INormalStream* pCCStream;
    pCCStream= NSCam::NSIoPipe::NSPostProc::INormalStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc::ENormalStreamTag_Prv,0xFFFF, isV3);
    pCCStream->init();
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    IspDrv* mpIspDrv=NULL;
        mpIspDrv=IspDrv::createInstance();
        mpIspDrv->init(LOG_TAG);
    //
    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvMagicNo.push_back(0);
    //input image
    IMEM_BUF_INFO imgiBuf;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBuffer* srcBuffer;
    MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
        imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    srcBuffer = pHeap->createImageBuffer();
        srcBuffer->incStrong(srcBuffer);
        srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=srcBuffer;
    enqueParams.mvIn.push_back(src);

    //crop information
    MCrpRsInfo crop;
    crop.mGroupID=1;
    MCrpRsInfo crop2;
    crop2.mGroupID=2;
    MCrpRsInfo crop3;
    crop3.mGroupID=3;
    IImageBuffer* outBuffer=NULL;

    for(int i=0;i<5;i++)
    {
        enqueParams.mvOut.clear();
        enqueParams.mvCropRsInfo.clear();

        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=1280;
        crop.mCropRect.s.h=720;
        crop.mResizeDst.w=1280;
        crop.mResizeDst.h=720;
        int wdma_out_w=0,wdma_out_h=0;
        int wrot_out_w=0,wrot_out_h=0;
        switch(type)
        {
            case 0:
            default:
                //same crop for mdp 2 output
                crop2.mCropRect.p_fractional.x=0;
                crop2.mCropRect.p_fractional.y=0;
                crop2.mCropRect.p_integral.x=0;
                crop2.mCropRect.p_integral.y=0;
                crop2.mCropRect.s.w=1280;
                crop2.mCropRect.s.h=720;
                crop2.mResizeDst.w=1280;
                crop2.mResizeDst.h=720;
                crop3.mCropRect.p_fractional.x=0;
                crop3.mCropRect.p_fractional.y=0;
                crop3.mCropRect.p_integral.x=0;
                crop3.mCropRect.p_integral.y=0;
                crop3.mCropRect.s.w=1280;
                crop3.mCropRect.s.h=720;
                crop3.mResizeDst.w=1280;
                crop3.mResizeDst.h=720;
                wdma_out_w=1280;
                wdma_out_h=720;
                wrot_out_w=1280;
                wrot_out_h=720;
                break;
            case 1:
                //different crop/same dst size
                crop2.mCropRect.p_fractional.x=0;
                crop2.mCropRect.p_fractional.y=0;
                crop2.mCropRect.p_integral.x=0;
                crop2.mCropRect.p_integral.y=0;
                crop2.mCropRect.s.w=1000;
                crop2.mCropRect.s.h=600;
                crop2.mResizeDst.w=1280;
                crop2.mResizeDst.h=720;
                crop3.mCropRect.p_fractional.x=0;
                crop3.mCropRect.p_fractional.y=0;
                crop3.mCropRect.p_integral.x=280;
                crop3.mCropRect.p_integral.y=120;
                crop3.mCropRect.s.w=1000;
                crop3.mCropRect.s.h=600;
                crop3.mResizeDst.w=1280;
                crop3.mResizeDst.h=720;
                wdma_out_w=1280;
                wdma_out_h=720;
                wrot_out_w=1280;
                wrot_out_h=720;
                break;
            case 2:
                //different crop/different dst size (wroto rotate)
                crop2.mCropRect.p_fractional.x=0;
                crop2.mCropRect.p_fractional.y=0;
                crop2.mCropRect.p_integral.x=0;
                crop2.mCropRect.p_integral.y=0;
                crop2.mCropRect.s.w=700;
                crop2.mCropRect.s.h=400;
                crop2.mResizeDst.w=640;
                crop2.mResizeDst.h=480;
                crop3.mCropRect.p_fractional.x=0;
                crop3.mCropRect.p_fractional.y=0;
                crop3.mCropRect.p_integral.x=280;
                crop3.mCropRect.p_integral.y=120;
                crop3.mCropRect.s.w=1000;
                crop3.mCropRect.s.h=600;
                crop3.mResizeDst.w=960;
                crop3.mResizeDst.h=640;
                wdma_out_w=640;
                wdma_out_h=480;
                wrot_out_w=640;
                wrot_out_h=960;
                break;
            case 3:
                if(i%2==0)
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=100;
                    crop.mCropRect.p_integral.y=100;
                    crop.mCropRect.s.w=700;
                    crop.mCropRect.s.h=350;
                    crop.mResizeDst.w=320;
                    crop.mResizeDst.h=240;
                    crop2.mCropRect.p_fractional.x=0;
                    crop2.mCropRect.p_fractional.y=0;
                    crop2.mCropRect.p_integral.x=0;
                    crop2.mCropRect.p_integral.y=0;
                    crop2.mCropRect.s.w=700;
                    crop2.mCropRect.s.h=400;
                    crop2.mResizeDst.w=640;
                    crop2.mResizeDst.h=480;
                    crop3.mCropRect.p_fractional.x=0;
                    crop3.mCropRect.p_fractional.y=0;
                    crop3.mCropRect.p_integral.x=280;
                    crop3.mCropRect.p_integral.y=120;
                    crop3.mCropRect.s.w=1000;
                    crop3.mCropRect.s.h=600;
                    crop3.mResizeDst.w=960;
                    crop3.mResizeDst.h=640;
                    wdma_out_w=640;
                    wdma_out_h=480;
                    wrot_out_w=640;
                    wrot_out_h=960;
                }
                else
                {
                    crop.mCropRect.p_fractional.x=0;
                    crop.mCropRect.p_fractional.y=0;
                    crop.mCropRect.p_integral.x=300;
                    crop.mCropRect.p_integral.y=200;
                    crop.mCropRect.s.w=500;
                    crop.mCropRect.s.h=250;
                    crop.mResizeDst.w=320;
                    crop.mResizeDst.h=240;
                    crop2.mCropRect.p_fractional.x=0;
                    crop2.mCropRect.p_fractional.y=0;
                    crop2.mCropRect.p_integral.x=250;
                    crop2.mCropRect.p_integral.y=200;
                    crop2.mCropRect.s.w=700;
                    crop2.mCropRect.s.h=400;
                    crop2.mResizeDst.w=640;
                    crop2.mResizeDst.h=480;
                    crop3.mCropRect.p_fractional.x=0;
                    crop3.mCropRect.p_fractional.y=0;
                    crop3.mCropRect.p_integral.x=0;
                    crop3.mCropRect.p_integral.y=0;
                    crop3.mCropRect.s.w=1000;
                    crop3.mCropRect.s.h=600;
                    crop3.mResizeDst.w=960;
                    crop3.mResizeDst.h=640;
                    wdma_out_w=640;
                    wdma_out_h=480;
                    wrot_out_w=640;
                    wrot_out_h=960;
                }
                break;
        }
        enqueParams.mvCropRsInfo.push_back(crop);
        enqueParams.mvCropRsInfo.push_back(crop2);
        enqueParams.mvCropRsInfo.push_back(crop3);

        //output buffer
        IMEM_BUF_INFO img2oBuf;
        if(type==3)
        {    //simulate fd buffer from img2o
            img2oBuf.size=320*240*2;
                mpImemDrv->allocVirtBuf(&img2oBuf);
            memset((MUINT8*)img2oBuf.virtAddr, 0x0, img2oBuf.size);
            MUINT32 bufStridesInBytes_1[3] = {320*2,0,0};
            PortBufInfo_v1 portBufInfo_1 = PortBufInfo_v1( img2oBuf.memID,img2oBuf.virtAddr,0,img2oBuf.bufSecu, img2oBuf.bufCohe);
                IImageBufferAllocator::ImgParam imgParam_1 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                                MSize(320,240),  bufStridesInBytes_1, bufBoundaryInBytes, 1);
            sp<ImageBufferHeap> pHeap_1 = ImageBufferHeap::create( LOG_TAG, imgParam_1,portBufInfo_1,true);
            outBuffer = pHeap_1->createImageBuffer();
                outBuffer->incStrong(outBuffer);
                outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
            Output dst;
            dst.mPortID=IMG2O;
            dst.mBuffer=outBuffer;
            dst.mPortID.group=0;
            enqueParams.mvOut.push_back(dst);
        }

        IMEM_BUF_INFO wdmaoBuf;
        IMEM_BUF_INFO wrotoBuf;
        wdmaoBuf.size=wdma_out_w*wdma_out_h*2;
            mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0x0, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_3[3] = {wdma_out_w, wdma_out_w/2, wdma_out_w/2};
        PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdma_out_w, wdma_out_h),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
        IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
            outBuffer2->incStrong(outBuffer2);
            outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst2;
        dst2.mPortID=WDMAO;
        dst2.mBuffer=outBuffer2;
        dst2.mPortID.group=0;
        enqueParams.mvOut.push_back(dst2);
        //
        wrotoBuf.size=wrot_out_w*wrot_out_h*2;
            mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0x0, wrotoBuf.size);
        MUINT32 bufStridesInBytes_4[3] = {wrot_out_w*2,0,0};
        PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(wrot_out_w, wrot_out_h),  bufStridesInBytes_4, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
        IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
            outBuffer3->incStrong(outBuffer3);
            outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst3;
        dst3.mPortID=WROTO;
        dst3.mBuffer=outBuffer3;
        dst3.mPortID.group=0;
        if(type==2 || type==3)
        {
            dst3.mTransform=eTransform_ROT_90;
        }
        enqueParams.mvOut.push_back(dst3);

        //buffer operation
        mpImemDrv->cacheFlushAll();
        printf(": enque\n");
        ret=pCCStream->enque(enqueParams);
        if(!ret)
        {
            printf(" : ERR enque fail\n");
        }
        else
        {
            printf(": enque done\n");
        }
        QParams dequeParams;
        ret=pCCStream->deque(dequeParams);
        if(!ret)
        {
            printf(" : ERR deque fail\n");
        }
        else
        {
            printf(": deque done\n");
        }
        printf("(CAM_CTL_TILE %d):",ISP_READ_REG_NOPROTECT(mpIspDrv,CAM_CTL_TILE));

        //dump image
        char filename[256];
        if(type==3)
        {
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case0_%d_%d_img2o_%dx%d.yuv",type,i, 320,240);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(img2oBuf.virtAddr), 320 *240 * 2);
        }
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case0_%d_%d_wdmao_%dx%d.yuv",type,i, wdma_out_w,wdma_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(wdmaoBuf.virtAddr), wdma_out_w *wdma_out_h * 2);
            char filename2[256];
            sprintf(filename2, "/sdcard/Pictures/P2UT_DiffViewAngle_case0_%d_%d_wroto_%dx%d.yuv",type,i, wrot_out_w,wrot_out_h);
            saveBufToFile(filename2, reinterpret_cast<MUINT8*>(wrotoBuf.virtAddr), wrot_out_w *wrot_out_h * 2);

            //
            if(type==3)
            {
                outBuffer->unlockBuf(LOG_TAG);
                mpImemDrv->freeVirtBuf(&img2oBuf);
            }
                outBuffer2->unlockBuf(LOG_TAG);
                outBuffer3->unlockBuf(LOG_TAG);
                mpImemDrv->freeVirtBuf(&wdmaoBuf);
                mpImemDrv->freeVirtBuf(&wrotoBuf);
            //
    }
    //
    pCCStream->uninit();
    pCCStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit(LOG_TAG);
        mpIspDrv->destroyInstance();
    return ret;
}



/*********************************************************************************/
int case1()
{
    int ret=0;
    MBOOL isV3 = MFALSE;    //temp disable tuning path cuz tuning function not ready
    NSCam::NSIoPipe::NSPostProc::IFeatureStream* pP2AStream;
    pP2AStream= NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc::EFeatureStreamTag_vFB_Stream,0xFFFF, isV3);
    pP2AStream->init();
    NSCam::NSIoPipe::NSPostProc::IFeatureStream* pP2BStream;
    pP2BStream= NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc::EFeatureStreamTag_vFB_FB,0xFFFF, isV3);
    pP2BStream->init();
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    IspDrv* mpIspDrv=NULL;
        mpIspDrv=IspDrv::createInstance();
        mpIspDrv->init(LOG_TAG);
    //

    //p2a data
    QParams p2aenqueParams;
    p2aenqueParams.mvPrivaData.push_back(NULL);
    p2aenqueParams.mvMagicNo.push_back(0);
    //input image
    IMEM_BUF_INFO imgiBuf;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBuffer* srcBuffer;
    MUINT32 bufStridesInBytes[3] = {3200, 0, 0};
        imgiBuf.size=sizeof(g_imgi_array_2560x1440_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_2560x1440_b10), imgiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),MSize(2560, 1440), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    srcBuffer = pHeap->createImageBuffer();
        srcBuffer->incStrong(srcBuffer);
        srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=srcBuffer;
    p2aenqueParams.mvIn.push_back(src);
    //crop information
    MCrpRsInfo crop;
    crop.mGroupID=1;
    MCrpRsInfo crop2;
    crop2.mGroupID=2;
    MCrpRsInfo crop3;
    crop3.mGroupID=3;

    //p2b data
    QParams p2benqueParams;
    p2benqueParams.mvPrivaData.push_back(NULL);
    p2benqueParams.mvMagicNo.push_back(0);
    //input image
    IMEM_BUF_INFO p2b_imgiBuf;
    IImageBuffer* p2b_srcBuffer;
    MUINT32 p2b_bufStridesInBytes[3] = {640, 0, 0};
        p2b_imgiBuf.size=sizeof(g_imgi_320x240_yuyv);
    mpImemDrv->allocVirtBuf(&p2b_imgiBuf);
        memcpy( (MUINT8*)(p2b_imgiBuf.virtAddr), (MUINT8*)(g_imgi_320x240_yuyv), p2b_imgiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 p2b_portBufInfo = PortBufInfo_v1( p2b_imgiBuf.memID,p2b_imgiBuf.virtAddr,0,p2b_imgiBuf.bufSecu, p2b_imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam p2b_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),MSize(320, 240), p2b_bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> p2b_pHeap = ImageBufferHeap::create( LOG_TAG, p2b_imgParam,p2b_portBufInfo,true);
    p2b_srcBuffer = p2b_pHeap->createImageBuffer();
        p2b_srcBuffer->incStrong(p2b_srcBuffer);
        p2b_srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input p2b_src;
    p2b_src.mPortID=IMGI;
    p2b_src.mBuffer=p2b_srcBuffer;
    p2benqueParams.mvIn.push_back(p2b_src);
    //alpha maps
    IMEM_BUF_INFO p2b_ufdiBuf;
    IImageBuffer* p2b_alpha1Buffer;
    MUINT32 p2b_alphabufStridesInBytes[3] = {320, 0, 0};
        p2b_ufdiBuf.size=sizeof(g_alpha1_320x240_b8);
    mpImemDrv->allocVirtBuf(&p2b_ufdiBuf);
        memcpy( (MUINT8*)(p2b_ufdiBuf.virtAddr), (MUINT8*)(g_alpha1_320x240_b8), p2b_ufdiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 p2b_alpha1_portBufInfo = PortBufInfo_v1( p2b_ufdiBuf.memID,p2b_ufdiBuf.virtAddr,0,p2b_ufdiBuf.bufSecu, p2b_ufdiBuf.bufCohe);
        IImageBufferAllocator::ImgParam p2b_alpha1_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER8),MSize(320, 240), p2b_alphabufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> p2b_alpha1_pHeap = ImageBufferHeap::create( LOG_TAG, p2b_alpha1_imgParam,p2b_alpha1_portBufInfo,true);
    p2b_alpha1Buffer = p2b_alpha1_pHeap->createImageBuffer();
        p2b_alpha1Buffer->incStrong(p2b_alpha1Buffer);
        p2b_alpha1Buffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input p2b_alpha1;
    p2b_alpha1.mPortID=UFDI;
    p2b_alpha1.mBuffer=p2b_alpha1Buffer;
    p2benqueParams.mvIn.push_back(p2b_alpha1);
    //
    //alpha maps
    IMEM_BUF_INFO p2b_lceiBuf;
    IImageBuffer* p2b_alpha2Buffer;
        p2b_lceiBuf.size=sizeof(g_alpha2_320x240_b8);
    mpImemDrv->allocVirtBuf(&p2b_lceiBuf);
        memcpy( (MUINT8*)(p2b_lceiBuf.virtAddr), (MUINT8*)(g_alpha2_320x240_b8), p2b_lceiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 p2b_alpha2_portBufInfo = PortBufInfo_v1( p2b_lceiBuf.memID,p2b_lceiBuf.virtAddr,0,p2b_lceiBuf.bufSecu, p2b_lceiBuf.bufCohe);
        IImageBufferAllocator::ImgParam p2b_alpha2_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER8),MSize(320, 240), p2b_alphabufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> p2b_alpha2_pHeap = ImageBufferHeap::create( LOG_TAG, p2b_alpha2_imgParam,p2b_alpha2_portBufInfo,true);
    p2b_alpha2Buffer = p2b_alpha2_pHeap->createImageBuffer();
        p2b_alpha2Buffer->incStrong(p2b_alpha2Buffer);
        p2b_alpha2Buffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input p2b_alpha2;
    p2b_alpha2.mPortID=LCEI;
    p2b_alpha2.mBuffer=p2b_alpha2Buffer;
    p2benqueParams.mvIn.push_back(p2b_alpha2);
    //crop information
    MCrpRsInfo p2b_crop;
    p2b_crop.mGroupID=1;
    MCrpRsInfo p2b_crop2;
    p2b_crop2.mGroupID=2;
    MCrpRsInfo p2b_crop3;
    p2b_crop3.mGroupID=3;

    //output buffer
    for(int i=0;i<5;i++)
    {
        //clear
        p2aenqueParams.mvOut.clear();
        p2aenqueParams.mvCropRsInfo.clear();

        ////////////////////////////////////////////////////////
        //p2a
        ////////////////////////////////////////////////////////
        //crop
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=2560;
        crop.mCropRect.s.h=1440;
        crop.mResizeDst.w=1920;
        crop.mResizeDst.h=1080;
        int wdma_out_w=0,wdma_out_h=0;
        int wrot_out_w=0,wrot_out_h=0;
        //different crop/same dst size
        if(i%2==0)
        {
            crop2.mCropRect.p_fractional.x=0;
            crop2.mCropRect.p_fractional.y=0;
            crop2.mCropRect.p_integral.x=0;
            crop2.mCropRect.p_integral.y=0;
            crop2.mCropRect.s.w=1800;
            crop2.mCropRect.s.h=800;
            crop2.mResizeDst.w=1280;
            crop2.mResizeDst.h=720;
            crop3.mCropRect.p_fractional.x=0;
            crop3.mCropRect.p_fractional.y=0;
            crop3.mCropRect.p_integral.x=120;
            crop3.mCropRect.p_integral.y=280;
            crop3.mCropRect.s.w=1800;
            crop3.mCropRect.s.h=800;
            crop3.mResizeDst.w=1920;
            crop3.mResizeDst.h=1080;
        }
        else
        {
            crop2.mCropRect.p_fractional.x=0;
            crop2.mCropRect.p_fractional.y=0;
            crop2.mCropRect.p_integral.x=120;
            crop2.mCropRect.p_integral.y=280;
            crop2.mCropRect.s.w=1800;
            crop2.mCropRect.s.h=800;
            crop2.mResizeDst.w=1280;
            crop2.mResizeDst.h=720;
            crop3.mCropRect.p_fractional.x=0;
            crop3.mCropRect.p_fractional.y=0;
            crop3.mCropRect.p_integral.x=0;
            crop3.mCropRect.p_integral.y=0;
            crop3.mCropRect.s.w=1800;
            crop3.mCropRect.s.h=800;
            crop3.mResizeDst.w=1920;
            crop3.mResizeDst.h=1080;
        }
        wdma_out_w=1280;
        wdma_out_h=720;
        wrot_out_w=1920;
        wrot_out_h=1080;
        p2aenqueParams.mvCropRsInfo.push_back(crop);
        p2aenqueParams.mvCropRsInfo.push_back(crop2);
        p2aenqueParams.mvCropRsInfo.push_back(crop3);

        //full size img3o
        IMEM_BUF_INFO img3oBuf;
        img3oBuf.size=1920*1080*2;
            mpImemDrv->allocVirtBuf(&img3oBuf);
        memset((MUINT8*)img3oBuf.virtAddr, 0xffffffff, img3oBuf.size);
        MUINT32 bufStridesInBytes_1[3] = {1920,960,960};
        PortBufInfo_v1 portBufInfo_1 = PortBufInfo_v1( img3oBuf.memID,img3oBuf.virtAddr,0,img3oBuf.bufSecu, img3oBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_1 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(1920,1080),  bufStridesInBytes_1, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_1 = ImageBufferHeap::create( LOG_TAG, imgParam_1,portBufInfo_1,true);
        IImageBuffer* outBuffer = pHeap_1->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=IMG3O;
        dst.mBuffer=outBuffer;
        dst.mPortID.group=0;
        p2aenqueParams.mvOut.push_back(dst);
        //
        IMEM_BUF_INFO wdmaoBuf;
        IMEM_BUF_INFO wrotoBuf;
        wdmaoBuf.size=wdma_out_w*wdma_out_h*2;
            mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_3[3] = {wdma_out_w, wdma_out_w/2, wdma_out_w/2};
        PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdma_out_w, wdma_out_h),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
        IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
            outBuffer2->incStrong(outBuffer2);
            outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst2;
        dst2.mPortID=WDMAO;
        dst2.mBuffer=outBuffer2;
        dst2.mPortID.group=0;
        p2aenqueParams.mvOut.push_back(dst2);
        //
        wrotoBuf.size=wrot_out_w*wrot_out_h*2;
            mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0xffffffff, wrotoBuf.size);
        MUINT32 bufStridesInBytes_4[3] = {wrot_out_w*2,0,0};
        PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(wrot_out_w, wrot_out_h),  bufStridesInBytes_4, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
        IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
            outBuffer3->incStrong(outBuffer3);
            outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst3;
        dst3.mPortID=WROTO;
        dst3.mBuffer=outBuffer3;
        dst3.mPortID.group=0;
        p2aenqueParams.mvOut.push_back(dst3);

        //buffer operation
        mpImemDrv->cacheFlushAll();
        printf("(%d) : p2a enque\n",i);
        ret=pP2AStream->enque(p2aenqueParams);
        if(!ret)
        {
            printf("(%d) : p2a ERR enque fail\n",i);
        }
        else
        {
            printf("(%d) : p2a enque done\n",i);
        }
        QParams p2adequeParams;
        ret=pP2AStream->deque(p2adequeParams);
        if(!ret)
        {
            printf("(%d) : p2a ERR deque fail\n",i);
        }
        else
        {
            printf("(%d) : p2a deque done\n",i);
        }
        printf("(%d) : (CAM_CTL_TILE  0x%x) \n",i,ISP_READ_REG_NOPROTECT(mpIspDrv,CAM_CTL_TILE));

        //dump image
        char filename[256];
        sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case1_p2a_%d_img3o_%dx%d.yuv",i, 1920,1080);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(p2adequeParams.mvOut[0].mBuffer->getBufVA(0)), 1920 *1080 * 2);
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case1_p2a_%d_wdmao_%dx%d.yuv",i, wdma_out_w,wdma_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(p2adequeParams.mvOut[1].mBuffer->getBufVA(0)), wdma_out_w *wdma_out_h * 2);
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case1_p2a_%d_wroto_%dx%d.yuv",i, wrot_out_w,wrot_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(p2adequeParams.mvOut[2].mBuffer->getBufVA(0)), wrot_out_w *wrot_out_h * 2);

        printf("(%d) : --- p2a done ---\n", i);
        ////////////////////////////////////////////////////////
        //p2b
        //vipi: 1920x1080
        //imgi: 320x240, ufdi: 320x240, lcei: 320x240
        ////////////////////////////////////////////////////////

        //clear
        p2benqueParams.mvOut.clear();
        p2benqueParams.mvCropRsInfo.clear();

        //img3o from p2a as vipi for p2b
        Input vipisrc;
                vipisrc.mPortID=VIPI;
                vipisrc.mBuffer=p2adequeParams.mvOut[0].mBuffer;
            vipisrc.mPortID.group=0;
                p2benqueParams.mvIn.push_back(vipisrc);

        //crop
        p2b_crop.mCropRect.p_fractional.x=0;
        p2b_crop.mCropRect.p_fractional.y=0;
        p2b_crop.mCropRect.p_integral.x=0;
        p2b_crop.mCropRect.p_integral.y=0;
        p2b_crop.mCropRect.s.w=320;
        p2b_crop.mCropRect.s.h=240;
        p2b_crop.mResizeDst.w=1920;
        p2b_crop.mResizeDst.h=1080;
        wdma_out_w=0;
        wdma_out_h=0;
        wrot_out_w=0;
        wrot_out_h=0;
        if(i%2==0)
        {
            //different crop/same dst size
            p2b_crop2.mCropRect.p_fractional.x=0;
            p2b_crop2.mCropRect.p_fractional.y=0;
            p2b_crop2.mCropRect.p_integral.x=0;
            p2b_crop2.mCropRect.p_integral.y=0;
            p2b_crop2.mCropRect.s.w=1800;
            p2b_crop2.mCropRect.s.h=800;
            p2b_crop2.mResizeDst.w=1280;
            p2b_crop2.mResizeDst.h=720;
            p2b_crop3.mCropRect.p_fractional.x=0;
            p2b_crop3.mCropRect.p_fractional.y=0;
            p2b_crop3.mCropRect.p_integral.x=120;
            p2b_crop3.mCropRect.p_integral.y=280;
            p2b_crop3.mCropRect.s.w=1800;
            p2b_crop3.mCropRect.s.h=800;
            p2b_crop3.mResizeDst.w=1920;
            p2b_crop3.mResizeDst.h=1080;
            wdma_out_w=1280;
            wdma_out_h=720;
            wrot_out_w=1920;
            wrot_out_h=1080;
        }
        else
        {
            //different crop/same dst size
            p2b_crop2.mCropRect.p_fractional.x=0;
            p2b_crop2.mCropRect.p_fractional.y=0;
            p2b_crop2.mCropRect.p_integral.x=120;
            p2b_crop2.mCropRect.p_integral.y=280;
            p2b_crop2.mCropRect.s.w=1800;
            p2b_crop2.mCropRect.s.h=800;
            p2b_crop2.mResizeDst.w=1280;
            p2b_crop2.mResizeDst.h=720;
            p2b_crop3.mCropRect.p_fractional.x=0;
            p2b_crop3.mCropRect.p_fractional.y=0;
            p2b_crop3.mCropRect.p_integral.x=0;
            p2b_crop3.mCropRect.p_integral.y=0;
            p2b_crop3.mCropRect.s.w=1800;
            p2b_crop3.mCropRect.s.h=800;
            p2b_crop3.mResizeDst.w=960;
            p2b_crop3.mResizeDst.h=640;
            wdma_out_w=1280;
            wdma_out_h=720;
            wrot_out_w=960;
            wrot_out_h=640;
        }
        p2benqueParams.mvCropRsInfo.push_back(p2b_crop);
        p2benqueParams.mvCropRsInfo.push_back(p2b_crop2);
        p2benqueParams.mvCropRsInfo.push_back(p2b_crop3);

        //output
        IMEM_BUF_INFO p2b_wdmaoBuf;
        IMEM_BUF_INFO p2b_wrotoBuf;
        p2b_wdmaoBuf.size=wdma_out_w*wdma_out_h*2;
            mpImemDrv->allocVirtBuf(&p2b_wdmaoBuf);
        memset((MUINT8*)p2b_wdmaoBuf.virtAddr, 0xffffffff, p2b_wdmaoBuf.size);
        MUINT32 p2b_bufStridesInBytes_3[3] = {wdma_out_w, wdma_out_w/2, wdma_out_w/2};
        PortBufInfo_v1 p2b_portBufInfo_3 = PortBufInfo_v1( p2b_wdmaoBuf.memID,p2b_wdmaoBuf.virtAddr,0,p2b_wdmaoBuf.bufSecu, p2b_wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam p2b_imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdma_out_w, wdma_out_h),  p2b_bufStridesInBytes_3, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> p2b_pHeap_3 = ImageBufferHeap::create( LOG_TAG, p2b_imgParam_3,p2b_portBufInfo_3,true);
        IImageBuffer* p2b_outBuffer2 = p2b_pHeap_3->createImageBuffer();
            p2b_outBuffer2->incStrong(p2b_outBuffer2);
            p2b_outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output p2b_dst;
        p2b_dst.mPortID=WDMAO;
        p2b_dst.mBuffer=p2b_outBuffer2;
        p2b_dst.mPortID.group=0;
        p2benqueParams.mvOut.push_back(p2b_dst);
        //
        p2b_wrotoBuf.size=wrot_out_w*wrot_out_h*2;
            mpImemDrv->allocVirtBuf(&p2b_wrotoBuf);
        memset((MUINT8*)p2b_wrotoBuf.virtAddr, 0xffffffff, p2b_wrotoBuf.size);
        MUINT32 p2b_bufStridesInBytes_4[3] = {wrot_out_w*2,0,0};
        PortBufInfo_v1 p2b_portBufInfo_4 = PortBufInfo_v1( p2b_wrotoBuf.memID,p2b_wrotoBuf.virtAddr,0,p2b_wrotoBuf.bufSecu, p2b_wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam p2b_imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(wrot_out_w, wrot_out_h),  p2b_bufStridesInBytes_4, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> p2b_pHeap_4 = ImageBufferHeap::create( LOG_TAG, p2b_imgParam_4,p2b_portBufInfo_4,true);
        IImageBuffer* p2b_outBuffer3 = p2b_pHeap_4->createImageBuffer();
            p2b_outBuffer3->incStrong(p2b_outBuffer3);
            p2b_outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output p2b_dst2;
        p2b_dst2.mPortID=WROTO;
        p2b_dst2.mBuffer=p2b_outBuffer3;
        p2b_dst2.mPortID.group=0;
        p2benqueParams.mvOut.push_back(p2b_dst2);

        //buffer operation
        mpImemDrv->cacheFlushAll();
        printf("(%d) : p2b enque\n",i);
        ret=pP2BStream->enque(p2benqueParams);
        if(!ret)
        {
            printf("(%d) : p2b ERR enque fail\n",i);
        }
        else
        {
            printf("(%d) : p2b enque done\n",i);
        }
        QParams p2bdequeParams;
        ret=pP2BStream->deque(p2bdequeParams);
        if(!ret)
        {
            printf("(%d) : p2b ERR deque fail\n",i);
        }
        else
        {
            printf("(%d) : p2b deque done\n",i);
        }
        printf("(%d) : (CAM_CTL_TILE 0x%x) \n",i,ISP_READ_REG_NOPROTECT(mpIspDrv,CAM_CTL_TILE));
        //dump image
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case1_p2b_%d_wdmao_%dx%d.yuv",i, wdma_out_w,wdma_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(p2bdequeParams.mvOut[0].mBuffer->getBufVA(0)), wdma_out_w *wdma_out_h * 2);
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case1_p2b_%d_wroto_%dx%d.yuv",i, wrot_out_w,wrot_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(p2bdequeParams.mvOut[1].mBuffer->getBufVA(0)), wrot_out_w *wrot_out_h * 2);

        printf("(%d) : --- p2b done ---\n", i);

        //unmap and free buffer
        //p2a
                outBuffer->unlockBuf(LOG_TAG);
                outBuffer2->unlockBuf(LOG_TAG);
                outBuffer3->unlockBuf(LOG_TAG);
                mpImemDrv->freeVirtBuf(&img3oBuf);
                mpImemDrv->freeVirtBuf(&wdmaoBuf);
                mpImemDrv->freeVirtBuf(&wrotoBuf);
                //p2b
                p2b_outBuffer2->unlockBuf(LOG_TAG);
                p2b_outBuffer3->unlockBuf(LOG_TAG);
                mpImemDrv->freeVirtBuf(&p2b_wdmaoBuf);
                mpImemDrv->freeVirtBuf(&p2b_wrotoBuf);
    }
    //
    pP2AStream->uninit();
    pP2AStream->destroyInstance(LOG_TAG);
    pP2BStream->uninit();
    pP2BStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit(LOG_TAG);
        mpIspDrv->destroyInstance();
    return ret;
}

/*********************************************************************************/
int case2()
{
    int ret=0;
    MBOOL isV3 = MFALSE;    //temp disable tuning path cuz tuning function not ready
    NSCam::NSIoPipe::NSPostProc::IFeatureStream* pMFBMixStream;
    pMFBMixStream= NSCam::NSIoPipe::NSPostProc::IFeatureStream::createInstance(LOG_TAG, NSCam::NSIoPipe::NSPostProc::EFeatureStreamTag_MFB_Mix,0xFFFF, isV3);
    pMFBMixStream->init();
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    IspDrv* mpIspDrv=NULL;
        mpIspDrv=IspDrv::createInstance();
        mpIspDrv->init(LOG_TAG);
    //

    //data
    QParams enqueParams;
    enqueParams.mvPrivaData.push_back(NULL);
    enqueParams.mvMagicNo.push_back(0);
    //input image
    IMEM_BUF_INFO imgiBuf;
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    IImageBuffer* srcBuffer;
    MUINT32 bufStridesInBytes[3] = {10560, 0, 0};
        imgiBuf.size=sizeof(g_imgi_5280x960_yuy2);
    mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_5280x960_yuy2), imgiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( imgiBuf.memID,imgiBuf.virtAddr,0,imgiBuf.bufSecu, imgiBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),MSize(5280, 960), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,true);
    srcBuffer = pHeap->createImageBuffer();
        srcBuffer->incStrong(srcBuffer);
        srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input src;
    src.mPortID=IMGI;
    src.mBuffer=srcBuffer;
    enqueParams.mvIn.push_back(src);

    IMEM_BUF_INFO vipiBuf;
    IImageBuffer* vipi_srcBuffer;
        vipiBuf.size=sizeof(g_vipi_5280x960_yuy2);
    mpImemDrv->allocVirtBuf(&vipiBuf);
        memcpy( (MUINT8*)(vipiBuf.virtAddr), (MUINT8*)(g_vipi_5280x960_yuy2), vipiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 vipi_portBufInfo = PortBufInfo_v1( vipiBuf.memID,vipiBuf.virtAddr,0,vipiBuf.bufSecu, vipiBuf.bufCohe);
        IImageBufferAllocator::ImgParam vipi_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),MSize(5280, 960), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> vipi_pHeap = ImageBufferHeap::create( LOG_TAG, vipi_imgParam,vipi_portBufInfo,true);
    vipi_srcBuffer = vipi_pHeap->createImageBuffer();
        vipi_srcBuffer->incStrong(vipi_srcBuffer);
        vipi_srcBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input vipi_src;
    vipi_src.mPortID=VIPI;
    vipi_src.mBuffer=vipi_srcBuffer;
    enqueParams.mvIn.push_back(vipi_src);
    //alpha maps
    IMEM_BUF_INFO vip3iBuf;
    IImageBuffer* vip3i_alpha1Buffer;
    MUINT32 vip3i_alphabufStridesInBytes[3] = {5280, 0, 0};
        vip3iBuf.size=sizeof(g_vip3i_5280x960_b8);
    mpImemDrv->allocVirtBuf(&vip3iBuf);
        memcpy( (MUINT8*)(vip3iBuf.virtAddr), (MUINT8*)(g_vip3i_5280x960_b8), vip3iBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 vip3i_alpha1_portBufInfo = PortBufInfo_v1( vip3iBuf.memID,vip3iBuf.virtAddr,0,vip3iBuf.bufSecu, vip3iBuf.bufCohe);
        IImageBufferAllocator::ImgParam vip3i_alpha1_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER8),MSize(5280, 960), vip3i_alphabufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> vip3i_alpha1_pHeap = ImageBufferHeap::create( LOG_TAG, vip3i_alpha1_imgParam,vip3i_alpha1_portBufInfo,true);
    vip3i_alpha1Buffer = vip3i_alpha1_pHeap->createImageBuffer();
        vip3i_alpha1Buffer->incStrong(vip3i_alpha1Buffer);
        vip3i_alpha1Buffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input vip3i_alpha1;
    vip3i_alpha1.mPortID=VIP3I;
    vip3i_alpha1.mBuffer=vip3i_alpha1Buffer;
    enqueParams.mvIn.push_back(vip3i_alpha1);

    IMEM_BUF_INFO ufdiBuf;
    IImageBuffer* ufdi_alpha1Buffer;
        ufdiBuf.size=sizeof(g_ufdi_5280x960_b8);
    mpImemDrv->allocVirtBuf(&ufdiBuf);
        memcpy( (MUINT8*)(ufdiBuf.virtAddr), (MUINT8*)(g_ufdi_5280x960_b8), ufdiBuf.size);
         //imem buffer 2 image heap
        PortBufInfo_v1 ufdi_alpha1_portBufInfo = PortBufInfo_v1( ufdiBuf.memID,ufdiBuf.virtAddr,0,ufdiBuf.bufSecu, ufdiBuf.bufCohe);
        IImageBufferAllocator::ImgParam ufdi_alpha1_imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER8),MSize(5280, 960), vip3i_alphabufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> ufdi_alpha1_pHeap = ImageBufferHeap::create( LOG_TAG, ufdi_alpha1_imgParam,ufdi_alpha1_portBufInfo,true);
    ufdi_alpha1Buffer = ufdi_alpha1_pHeap->createImageBuffer();
        ufdi_alpha1Buffer->incStrong(ufdi_alpha1Buffer);
        ufdi_alpha1Buffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
    Input ufdi_alpha1;
    ufdi_alpha1.mPortID=UFDI;
    ufdi_alpha1.mBuffer=ufdi_alpha1Buffer;
    enqueParams.mvIn.push_back(ufdi_alpha1);

    //crop information
    MCrpRsInfo crop;
    crop.mGroupID=1;
    MCrpRsInfo crop2;
    crop2.mGroupID=2;
    MCrpRsInfo crop3;
    crop3.mGroupID=3;


    //output buffer
    for(int i=0;i<5;i++)
    {
        //clear
        enqueParams.mvOut.clear();
        enqueParams.mvCropRsInfo.clear();

        //crop
        crop.mCropRect.p_fractional.x=0;
        crop.mCropRect.p_fractional.y=0;
        crop.mCropRect.p_integral.x=0;
        crop.mCropRect.p_integral.y=0;
        crop.mCropRect.s.w=5280;
        crop.mCropRect.s.h=960;
        crop.mResizeDst.w=5280;
        crop.mResizeDst.h=960;
        int wdma_out_w=0,wdma_out_h=0;
        int wrot_out_w=0,wrot_out_h=0;
        //different crop/same dst size
        if(i%2==0)
        {
            crop2.mCropRect.p_fractional.x=0;
            crop2.mCropRect.p_fractional.y=0;
            crop2.mCropRect.p_integral.x=0;
            crop2.mCropRect.p_integral.y=0;
            crop2.mCropRect.s.w=5280;
            crop2.mCropRect.s.h=960;
            crop2.mResizeDst.w=1280;
            crop2.mResizeDst.h=720;
            crop3.mCropRect.p_fractional.x=0;
            crop3.mCropRect.p_fractional.y=0;
            crop3.mCropRect.p_integral.x=280;
            crop3.mCropRect.p_integral.y=160;
            crop3.mCropRect.s.w=5000;
            crop3.mCropRect.s.h=800;
            crop3.mResizeDst.w=1920;
            crop3.mResizeDst.h=1080;
            wdma_out_w=1280;
            wdma_out_h=720;
            wrot_out_w=1920;
            wrot_out_h=1080;
        }
        else
        {
            crop2.mCropRect.p_fractional.x=0;
            crop2.mCropRect.p_fractional.y=0;
            crop2.mCropRect.p_integral.x=0;
            crop2.mCropRect.p_integral.y=0;
            crop2.mCropRect.s.w=5000;
            crop2.mCropRect.s.h=800;
            crop2.mResizeDst.w=1280;
            crop2.mResizeDst.h=720;
            crop3.mCropRect.p_fractional.x=0;
            crop3.mCropRect.p_fractional.y=0;
            crop3.mCropRect.p_integral.x=280;
            crop3.mCropRect.p_integral.y=160;
            crop3.mCropRect.s.w=5000;
            crop3.mCropRect.s.h=800;
            crop3.mResizeDst.w=960;
            crop3.mResizeDst.h=640;
            wdma_out_w=1280;
            wdma_out_h=720;
            wrot_out_w=960;
            wrot_out_h=640;
        }

        enqueParams.mvCropRsInfo.push_back(crop);
        enqueParams.mvCropRsInfo.push_back(crop2);
        enqueParams.mvCropRsInfo.push_back(crop3);

        //full size img3o
        IMEM_BUF_INFO img3oBuf;
        img3oBuf.size=5280*960*2;
            mpImemDrv->allocVirtBuf(&img3oBuf);
        memset((MUINT8*)img3oBuf.virtAddr, 0xffffffff, img3oBuf.size);
        MUINT32 bufStridesInBytes_1[3] = {5280,2640,2640};
        PortBufInfo_v1 portBufInfo_1 = PortBufInfo_v1( img3oBuf.memID,img3oBuf.virtAddr,0,img3oBuf.bufSecu, img3oBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_1 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(5280,960),  bufStridesInBytes_1, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_1 = ImageBufferHeap::create( LOG_TAG, imgParam_1,portBufInfo_1,true);
        IImageBuffer* outBuffer = pHeap_1->createImageBuffer();
            outBuffer->incStrong(outBuffer);
            outBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst;
        dst.mPortID=IMG3O;
        dst.mBuffer=outBuffer;
        dst.mPortID.group=0;
        enqueParams.mvOut.push_back(dst);
        //
        IMEM_BUF_INFO wdmaoBuf;
        IMEM_BUF_INFO wrotoBuf;
        wdmaoBuf.size=wdma_out_w*wdma_out_h*2;
            mpImemDrv->allocVirtBuf(&wdmaoBuf);
        memset((MUINT8*)wdmaoBuf.virtAddr, 0xffffffff, wdmaoBuf.size);
        MUINT32 bufStridesInBytes_3[3] = {wdma_out_w, wdma_out_w/2, wdma_out_w/2};
        PortBufInfo_v1 portBufInfo_3 = PortBufInfo_v1( wdmaoBuf.memID,wdmaoBuf.virtAddr,0,wdmaoBuf.bufSecu, wdmaoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_3 = IImageBufferAllocator::ImgParam((eImgFmt_YV12),
                                                            MSize(wdma_out_w, wdma_out_h),  bufStridesInBytes_3, bufBoundaryInBytes, 3);
        sp<ImageBufferHeap> pHeap_3 = ImageBufferHeap::create( LOG_TAG, imgParam_3,portBufInfo_3,true);
        IImageBuffer* outBuffer2 = pHeap_3->createImageBuffer();
            outBuffer2->incStrong(outBuffer2);
            outBuffer2->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst2;
        dst2.mPortID=WDMAO;
        dst2.mBuffer=outBuffer2;
        dst2.mPortID.group=0;
        enqueParams.mvOut.push_back(dst2);
        //
        wrotoBuf.size=wrot_out_w*wrot_out_h*2;
            mpImemDrv->allocVirtBuf(&wrotoBuf);
        memset((MUINT8*)wrotoBuf.virtAddr, 0xffffffff, wrotoBuf.size);
        MUINT32 bufStridesInBytes_4[3] = {wrot_out_w*2,0,0};
        PortBufInfo_v1 portBufInfo_4 = PortBufInfo_v1( wrotoBuf.memID,wrotoBuf.virtAddr,0,wrotoBuf.bufSecu, wrotoBuf.bufCohe);
            IImageBufferAllocator::ImgParam imgParam_4 = IImageBufferAllocator::ImgParam((eImgFmt_YUY2),
                                                            MSize(wrot_out_w, wrot_out_h),  bufStridesInBytes_4, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap_4 = ImageBufferHeap::create( LOG_TAG, imgParam_4,portBufInfo_4,true);
        IImageBuffer* outBuffer3 = pHeap_4->createImageBuffer();
            outBuffer3->incStrong(outBuffer3);
            outBuffer3->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        Output dst3;
        dst3.mPortID=WROTO;
        dst3.mBuffer=outBuffer3;
        dst3.mPortID.group=0;
        enqueParams.mvOut.push_back(dst3);

        //buffer operation
        mpImemDrv->cacheFlushAll();
        printf("(%d) : enque\n",i);
        ret=pMFBMixStream->enque(enqueParams);
        if(!ret)
        {
            printf("(%d) : ERR enque fail\n",i);
        }
        else
        {
            printf("(%d) : enque done\n",i);
        }
        QParams dequeParams;
        ret=pMFBMixStream->deque(dequeParams);
        if(!ret)
        {
            printf("(%d) :  ERR deque fail\n",i);
        }
        else
        {
            printf("(%d) :  deque done\n",i);
        }
        printf("(%d) : (CAM_CTL_TILE  0x%x) \n",i,ISP_READ_REG_NOPROTECT(mpIspDrv,CAM_CTL_TILE));

        //dump image
        char filename[256];
        sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case2_%d_img3o_%dx%d.yuv",i, 5280,960);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[0].mBuffer->getBufVA(0)), 5280 *960 * 2);
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case2_%d_wdmao_%dx%d.yuv",i, wdma_out_w,wdma_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[1].mBuffer->getBufVA(0)), wdma_out_w *wdma_out_h * 2);
            sprintf(filename, "/sdcard/Pictures/P2UT_DiffViewAngle_case2_%d_wroto_%dx%d.yuv",i, wrot_out_w,wrot_out_h);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(dequeParams.mvOut[2].mBuffer->getBufVA(0)), wrot_out_w *wrot_out_h * 2);

        printf("(%d) : --- p2a done ---\n", i);


        //unmap and free buffer
                outBuffer->unlockBuf(LOG_TAG);
                outBuffer2->unlockBuf(LOG_TAG);
                outBuffer3->unlockBuf(LOG_TAG);
                mpImemDrv->freeVirtBuf(&img3oBuf);
                mpImemDrv->freeVirtBuf(&wdmaoBuf);
                mpImemDrv->freeVirtBuf(&wrotoBuf);
    }
    //
    pMFBMixStream->uninit();
    pMFBMixStream->destroyInstance(LOG_TAG);
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit(LOG_TAG);
        mpIspDrv->destroyInstance();
    return ret;
}

