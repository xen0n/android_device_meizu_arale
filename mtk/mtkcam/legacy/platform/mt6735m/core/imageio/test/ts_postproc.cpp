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

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
#include "PipeImp.h"//../pipe/inc/
#include "PostProcPipe.h"//../pipe/inc/

using namespace NSImageio;
using namespace NSIspio;

#include <mtkcam/drv_common/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/drv_common/isp_reg.h>
#include <mtkcam/featureio/tdri_mgr.h>

//thread
#include <semaphore.h>
#include <pthread.h>
#include <utils/threads.h>
#include <utils/Mutex.h>    // For android::Mutex.
#include <mtkcam/v1/config/PriorityDefs.h>

//thread priority
#include <system/thread_defs.h>
#include <sys/resource.h>
#include <utils/ThreadDefs.h>


using namespace android;

/*******************************************************************************
*
********************************************************************************/
#define MY_LOGV(fmt, arg...)    printf("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)    printf("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)    printf("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)    printf("[%s] " fmt, __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)    printf("[%s] " fmt, __FUNCTION__, ##arg)


/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static  bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    MY_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}



int BasicEnqueDequeWOTuning(int testitem);
int BasicEnqueDequeWithTuning(int testitem);
int VSScase(int testitem);
int P2DVTcase4CQswitch(void);
/*******************************************************************************
*  Main Function
********************************************************************************/
int main_PostProc(int argc, char** argv)
{
    int ret = 0;
    int testCase = 0;
    int testItem = 0;
    int loopcount = 0;

    if (argc != 3)
    {
        MY_LOGD("Usage: imageiotest 1 <testCase> <testItem>\n");
        MY_LOGD("<testCase> 0: p2 basic enque/deque without tuning, 1:p2 basic enque/deque with tuning, 2: vss flow \n");
        goto EXIT;
    }
    else
    {
        MY_LOGI("main_PostProc enter\n");
        testCase = atoi(argv[1]);
        testItem = atoi(argv[2]);
        switch(testCase)
        {
            case 0:
                ret=BasicEnqueDequeWOTuning(testItem);
                break;
            case 1:
                ret=BasicEnqueDequeWithTuning(testItem);
                break;
            case 2:
                ret=VSScase(testItem);
                break;
            case 3:
                ret=P2DVTcase4CQswitch();
            default:
                break;
        }
    }

EXIT:
    MY_LOGI("main_PostProc leave\n");
    return ret;
}

/////
#include "p2_pics/imgi_1280x720_bayer10.h"
#include "p2_pics/imgi_2560x1440_bayer10.h" //for vss
#include "p2_pics/imgi_3264x1836_bayer10.h" //for vss
//debug
#include "p2_pics/imgi_320x240_yuyv.h"

#define TEST_RDMA 0

int BasicEnqueDequeWOTuning(int testitem)
{
    int ret=0;
    printf("BasicEnqueDequeWOTuning ENTRY\n");
    NSImageio::NSIspio::IPostProcPipe *mpPostProcPipe;

    IspDrv* mpIspDrv=NULL;
    mpIspDrv=IspDrv::createInstance();
    mpIspDrv->init();

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //memory allocation
    IMEM_BUF_INFO imgiBuf;
    int inputImgW=1280,inputImgH=720;
    if(TEST_RDMA)
    {
        imgiBuf.size=sizeof(g_imgi_320x240_yuyv);
        mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_320x240_yuyv), imgiBuf.size);
        mpImemDrv->mapPhyAddr(&imgiBuf);
        inputImgW=320;
        inputImgH=240;
    }
    else
    {
        imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
        mpImemDrv->allocVirtBuf(&imgiBuf);
        memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
        mpImemDrv->mapPhyAddr(&imgiBuf);
        inputImgW=1280;
        inputImgH=720;
    }

    //basic without crop/resize
    {
        mpPostProcPipe = NSImageio::NSIspio::IPostProcPipe::createInstance(NSImageio::NSIspio::eScenarioID_VSS,NSImageio::NSIspio::eScenarioFmt_RAW);
        ret=mpPostProcPipe->init();
        if (NULL == mpPostProcPipe || !mpPostProcPipe->init())
        {
            MY_LOGE("mpPostProcPipe init fail\n");
            return -1;
        }
        //memory allocation for output dma
        int outImgW_dispo=1280,outImgH_dispo=720;
        int outImgW_vido=1280,outImgH_vido=720;
        switch(testitem)
        {
            case 0: //no crop/ no resize
            default:
                if(TEST_RDMA)
                {
                    outImgW_dispo=320;
                    outImgH_dispo=240;
                    outImgW_vido=320;
                    outImgH_vido=240;
                }
                break;
            case 1: //no crop/ different resize
                outImgW_dispo=640;
                outImgH_dispo=480;
                outImgW_vido=1920;
                outImgH_vido=1080;
                break;
            case 2: //with crop/ different resize
                outImgW_dispo=640;
                outImgH_dispo=480;
                outImgW_vido=1920;
                outImgH_vido=1080;
                break;
            case 3: //no crop/ different resize/wroto rotate 90
                outImgW_dispo=640;
                outImgH_dispo=480;
                outImgW_vido=1920;
                outImgH_vido=1080;
                break;
            case 4: // crop/ different resize/wroto rotate 90
                outImgW_dispo=640;
                outImgH_dispo=480;
                outImgW_vido=1920;
                outImgH_vido=1080;
                break;
        }
        IMEM_BUF_INFO dispoBuf;
        IMEM_BUF_INFO vidoBuf;
        dispoBuf.size=outImgW_dispo*outImgH_dispo*2;
        mpImemDrv->allocVirtBuf(&dispoBuf);
        mpImemDrv->mapPhyAddr(&dispoBuf);
        memset((MUINT8*)dispoBuf.virtAddr, 0x0, dispoBuf.size);
        vidoBuf.size=outImgW_vido*outImgH_vido*2;
        mpImemDrv->allocVirtBuf(&vidoBuf);
        mpImemDrv->mapPhyAddr(&vidoBuf);
        memset((MUINT8*)vidoBuf.virtAddr, 0x0, vidoBuf.size);
        MY_LOGI("[christ] buffer allocation done\n");
        //
        MUINT32 reg_val=0x0;
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
        reg_val = ISP_IOCTL_READ_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_DMA_EN, 0);

        /////////////////////////////////////////////////////
        //set CQ first before pipe config
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,(MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ1,0,0);
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)(MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);
        MY_LOGI("[christ]stage1 done\n");
        //////////////////////////////////////////////////////
        // dma preparation 1 in, 2 out
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
        vPostProcInPorts.resize(1);
        vPostProcOutPorts.resize(2);//vPostProcOutPorts.resize(2);
        NSImageio::NSIspio::PortInfo port_imgi;
        NSImageio::NSIspio::PortInfo port_dispo,port_vido;
        //input
        if(TEST_RDMA)
        {
            port_imgi.eImgFmt = eImgFmt_YUY2;
        }
        else
        {
            port_imgi.eImgFmt = eImgFmt_BAYER10;
        }
        port_imgi.u4ImgWidth = inputImgW;
        port_imgi.u4ImgHeight = inputImgH;
        port_imgi.u4Stride[0] = inputImgW;  //camera 1.0, using pixel domain
        port_imgi.u4Stride[1] = 0;
        port_imgi.u4Stride[2] = 0;
        port_imgi.u4BufSize=imgiBuf.size;
        port_imgi.u4BufVA=imgiBuf.virtAddr;
        port_imgi.u4BufPA=imgiBuf.phyAddr;
        port_imgi.memID=imgiBuf.memID;
        switch(testitem)
        {
            case 0: //no crop/ no resize
            case 1://no crop/ different resize
            case 3: //no crop/ different resize/wroto rotate 90
            default:
                port_imgi.crop.x = 0;
                port_imgi.crop.y = 0;
                port_imgi.crop.w = inputImgW;
                port_imgi.crop.h = inputImgH;
                break;
            case 2://with crop/ different resize
            case 4:// crop/ different resize/wroto rotate 90
                port_imgi.crop.x = 400;
                port_imgi.crop.y = 200;
                port_imgi.crop.w = inputImgW-400;
                port_imgi.crop.h = inputImgH-200;
                break;
        }
        port_imgi.u4IsRunSegment = 0;
        port_imgi.type = NSImageio::NSIspio::EPortType_Memory;
        port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
        port_imgi.inout  = NSImageio::NSIspio::EPortDirection_In;
        port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2;
        vPostProcInPorts.at(0)=&port_imgi;
        //output
        port_dispo.eImgFmt = eImgFmt_YUY2;
        port_dispo.u4ImgWidth = outImgW_dispo;
        port_dispo.u4ImgHeight = outImgH_dispo;
        port_dispo.eImgRot = NSImageio::NSIspio::eImgRot_0;              //dispo NOT support rotation
        port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;          //dispo NOT support flip
        port_dispo.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
        port_dispo.index = NSImageio::NSIspio::EPortIndex_DISPO;
        port_dispo.inout  = NSImageio::NSIspio::EPortDirection_Out;
        port_dispo.u4Stride[0] = outImgW_dispo;
        port_dispo.u4Stride[1] = 0;
        port_dispo.u4Stride[2] = 0;
        port_dispo.u4BufSize=dispoBuf.size;
        port_dispo.u4BufVA=dispoBuf.virtAddr;
        port_dispo.u4BufPA=dispoBuf.phyAddr;
        port_dispo.memID=dispoBuf.memID;
        vPostProcOutPorts.at(0)=&port_dispo;
        //
        port_vido.eImgFmt = eImgFmt_YUY2;
        if((testitem==3)||testitem==4)
        {
            port_vido.u4ImgWidth = outImgH_vido;//outImgW_vido;
            port_vido.u4ImgHeight = outImgW_vido;//outImgH_vido;
            port_vido.eImgRot = eImgRot_90;//NSImageio::NSIspio::eImgRot_0;
            port_vido.u4Stride[0] = outImgH_vido;//outImgW_vido;
            port_vido.u4Stride[1] = 0;
            port_vido.u4Stride[2] = 0;
        }
        else
        {
            port_vido.u4ImgWidth = outImgW_vido;
            port_vido.u4ImgHeight =outImgH_vido;
            port_vido.eImgRot = NSImageio::NSIspio::eImgRot_0;
            port_vido.u4Stride[0] = outImgW_vido;
            port_vido.u4Stride[1] = 0;
            port_vido.u4Stride[2] = 0;
        }
        port_vido.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;
        port_vido.type = NSImageio::NSIspio::EPortType_VID_RDMA;
        port_vido.index = NSImageio::NSIspio::EPortIndex_VIDO;
        port_vido.inout  = NSImageio::NSIspio::EPortDirection_Out;

        port_vido.u4BufSize=vidoBuf.size;
        port_vido.u4BufVA=vidoBuf.virtAddr;
        port_vido.u4BufPA=vidoBuf.phyAddr;
        port_vido.memID=vidoBuf.memID;
        vPostProcOutPorts.at(1)=&port_vido;

        MY_LOGI("[christ]stage2 done\n");
        //config pipe
        ret=mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        if(!ret)
        {
            MY_LOGE("postprocPipe config fail");
            return ret;
        }
        MY_LOGI("[christ]stage3 done\n");
        ///////////////////////////////////////////////////////////
        //enque buffer
        NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory,NSImageio::NSIspio::EPortIndex_IMGI,0);
        NSImageio::NSIspio::QBufInfo    rQBufInfo;
        for(int i=0;i<vPostProcInPorts.size();i++)
        {
            rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].u4BufSize = vPostProcInPorts[i]->u4BufSize; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA = vPostProcInPorts[i]->u4BufVA;
            rQBufInfo.vBufInfo[0].u4BufPA = vPostProcInPorts[i]->u4BufPA;
            rQBufInfo.vBufInfo[0].memID = vPostProcInPorts[i]->memID;
            ret=mpPostProcPipe->enqueInBuf(rPortID, rQBufInfo);
            if(!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                return ret;
            }
        }
       for(int i=0;i<vPostProcOutPorts.size();i++)
        {
            switch(vPostProcOutPorts[i]->index)
            {
                case NSImageio::NSIspio::EPortIndex_VIDO:
                        rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                    break;
                case NSImageio::NSIspio::EPortIndex_DISPO:
                        rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG2O:
                    default:
                        rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O;
                        break;
            }
            rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].u4BufSize = vPostProcOutPorts[i]->u4BufSize; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA = vPostProcOutPorts[i]->u4BufVA;
            rQBufInfo.vBufInfo[0].u4BufPA = vPostProcOutPorts[i]->u4BufPA;
            rQBufInfo.vBufInfo[0].memID = vPostProcOutPorts[i]->memID;
            ret=mpPostProcPipe->enqueOutBuf(rPortID, rQBufInfo);
            if(!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                return ret;
            }
        }
        MY_LOGI("[christ]stage4.2 done\n");
         ///////////////////////////////////////////////////////////
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                      (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

        //set pass2 IN DMA register before pass2 start
        for(int i=0;i<vPostProcInPorts.size();i++)
        {
            mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcInPorts[i]->index),0,0);
        }
        //set pass2 OUT DMA register before pass2 start
        for(int i=0;i<vPostProcOutPorts.size();i++)
        {
            mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcOutPorts[i]->index),0,0);
        }
            //start
        ret=mpPostProcPipe->start();
        if(!ret)
        {
            MY_LOGE("P2 Start Fail!");
            return ret;
        }
       MY_LOGI("[christ]stage5 done.........\n");

        ///////////////////////////////////////////////////////////
        NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
        rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
        ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
           MY_LOGE("deque dispo out buffer fail,whileeeeee");
           return ret;
        }
        MY_LOGI("[christ]deque dispo done\n");
        rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
        ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGI("deque vido out buffer fail,whileeeeee");
            return ret;
        }
        MY_LOGE("[christ]deque vido done\n");
        rPortID.type = NSImageio::NSIspio::EPortType_Memory;
        rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
        ret=mpPostProcPipe->dequeInBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGE("deque imgi buffer fail,whileeeeee");
            return ret;
        }
        MY_LOGI("[christ]deque imgi done\n");
        ret=mpPostProcPipe->stop();
       MY_LOGI("[christ]stop done\n");
        ///////////////////////////////////////////////////////////
        //
        if (NULL != mpPostProcPipe)
        {
            if (MTRUE != mpPostProcPipe->uninit())
            {
                MY_LOGE("mpPostProcPipe uninit fail\n");
                ret = -1;
            }
            mpPostProcPipe->destroyInstance();
            mpPostProcPipe = NULL;
        }

        //save images
        char filename[256];
        sprintf(filename, "/data/P2UT_1_0_%d_wdmao_%dx%d.yuv",testitem, outImgW_dispo,outImgH_dispo);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dispoBuf.virtAddr), outImgW_dispo *outImgH_dispo * 2);
        char filename_2[256];
        if((testitem==3)||(testitem==4))
        {
            sprintf(filename_2, "/data/P2UT_1_0_%d_wroto_%dx%d.yuv",testitem, outImgH_vido,outImgW_vido);
            saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vidoBuf.virtAddr), outImgH_vido *outImgW_vido * 2);
        }
        else
        {
            sprintf(filename_2, "/data/P2UT_1_0_%d_wroto_%dx%d.yuv",testitem, outImgW_vido,outImgH_vido);
            saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vidoBuf.virtAddr), outImgW_vido *outImgH_vido * 2);
        }

        //free memory
        mpImemDrv->unmapPhyAddr(&dispoBuf);
        mpImemDrv->freeVirtBuf(&dispoBuf);
        mpImemDrv->unmapPhyAddr(&vidoBuf);
        mpImemDrv->freeVirtBuf(&vidoBuf);
    }



    //free memory
    mpImemDrv->unmapPhyAddr(&imgiBuf);
    mpImemDrv->freeVirtBuf(&imgiBuf);
    //free object
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit();
    mpIspDrv->destroyInstance();
    MY_LOGI("BasicEnqueDequeWOTuning EXIT\n");
    return ret;
}


/////////
int BasicEnqueDequeWithTuning(int testitem)
{
    int ret=0;
    printf("BasicEnqueDequeWithTuning ENTRY\n");
    NSImageio::NSIspio::IPostProcPipe *mpPostProcPipe;

    IspDrv* mpIspDrv=NULL;
    mpIspDrv=IspDrv::createInstance();
    mpIspDrv->init();

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //memory allocation
    IMEM_BUF_INFO imgiBuf;
    int inputImgW=1280,inputImgH=720;
    imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
    mpImemDrv->mapPhyAddr(&imgiBuf);
    inputImgW=1280;
    inputImgH=720;

    //basic without crop/resize
    {
        mpPostProcPipe = NSImageio::NSIspio::IPostProcPipe::createInstance(NSImageio::NSIspio::eScenarioID_VSS,NSImageio::NSIspio::eScenarioFmt_RAW);
        ret=mpPostProcPipe->init();
        if (NULL == mpPostProcPipe || !mpPostProcPipe->init())
        {
            MY_LOGE("mpPostProcPipe init fail\n");
            return -1;
        }
        //memory allocation for output dma
        int outImgW_dispo=1280,outImgH_dispo=720;
        int outImgW_vido=1280,outImgH_vido=720;
        IMEM_BUF_INFO dispoBuf;
        IMEM_BUF_INFO vidoBuf;
        dispoBuf.size=outImgW_dispo*outImgH_dispo*2;
        mpImemDrv->allocVirtBuf(&dispoBuf);
        mpImemDrv->mapPhyAddr(&dispoBuf);
        memset((MUINT8*)dispoBuf.virtAddr, 0x0, dispoBuf.size);
        vidoBuf.size=outImgW_vido*outImgH_vido*2;
        mpImemDrv->allocVirtBuf(&vidoBuf);
        mpImemDrv->mapPhyAddr(&vidoBuf);
        memset((MUINT8*)vidoBuf.virtAddr, 0x0, vidoBuf.size);
        MY_LOGI("[christ] buffer allocation done\n");
        //
        MUINT32 reg_val=0x0;
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
        reg_val = ISP_IOCTL_READ_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_DMA_EN, 0);

        /////////////////////////////////////////////////////
        //set CQ first before pipe config
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,(MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ1,0,0);
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)(MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);
        MY_LOGI("[christ]stage1 done\n");
        //////////////////////////////////////////////////////
        // dma preparation 1 in, 2 out
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
        vPostProcInPorts.resize(1);
        vPostProcOutPorts.resize(2);//vPostProcOutPorts.resize(2);
        NSImageio::NSIspio::PortInfo port_imgi;
        NSImageio::NSIspio::PortInfo port_dispo,port_vido;
        //input
        port_imgi.eImgFmt = eImgFmt_BAYER10;
        port_imgi.u4ImgWidth = inputImgW;
        port_imgi.u4ImgHeight = inputImgH;
        port_imgi.u4Stride[0] = inputImgW;  //camera 1.0, using pixel domain
        port_imgi.u4Stride[1] = 0;
        port_imgi.u4Stride[2] = 0;
        port_imgi.u4BufSize=imgiBuf.size;
        port_imgi.u4BufVA=imgiBuf.virtAddr;
        port_imgi.u4BufPA=imgiBuf.phyAddr;
        port_imgi.memID=imgiBuf.memID;
        port_imgi.u4IsRunSegment = 0;
        port_imgi.type = NSImageio::NSIspio::EPortType_Memory;
        port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
        port_imgi.inout  = NSImageio::NSIspio::EPortDirection_In;
        port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2;
        vPostProcInPorts.at(0)=&port_imgi;
        //output
        port_dispo.eImgFmt = eImgFmt_YUY2;
        port_dispo.u4ImgWidth = outImgW_dispo;
        port_dispo.u4ImgHeight = outImgH_dispo;
        port_dispo.eImgRot = NSImageio::NSIspio::eImgRot_0;              //dispo NOT support rotation
        port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;          //dispo NOT support flip
        port_dispo.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
        port_dispo.index = NSImageio::NSIspio::EPortIndex_DISPO;
        port_dispo.inout  = NSImageio::NSIspio::EPortDirection_Out;
        port_dispo.u4Stride[0] = outImgW_dispo;
        port_dispo.u4Stride[1] = 0;
        port_dispo.u4Stride[2] = 0;
        port_dispo.u4BufSize=dispoBuf.size;
        port_dispo.u4BufVA=dispoBuf.virtAddr;
        port_dispo.u4BufPA=dispoBuf.phyAddr;
        port_dispo.memID=dispoBuf.memID;
        vPostProcOutPorts.at(0)=&port_dispo;
        //
        port_vido.eImgFmt = eImgFmt_YUY2;
        port_vido.u4ImgWidth = outImgW_vido;
        port_vido.u4ImgHeight = outImgH_vido;
        port_vido.eImgRot = NSImageio::NSIspio::eImgRot_0;
        port_vido.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;
        port_vido.type = NSImageio::NSIspio::EPortType_VID_RDMA;
        port_vido.index = NSImageio::NSIspio::EPortIndex_VIDO;
        port_vido.inout  = NSImageio::NSIspio::EPortDirection_Out;
        port_vido.u4Stride[0] = outImgW_vido;
        port_vido.u4Stride[1] = 0;
        port_vido.u4Stride[2] = 0;
        port_vido.u4BufSize=vidoBuf.size;
        port_vido.u4BufVA=vidoBuf.virtAddr;
        port_vido.u4BufPA=vidoBuf.phyAddr;
        port_vido.memID=vidoBuf.memID;
        vPostProcOutPorts.at(1)=&port_vido;


        ///////////////////////////////////////////
        //set tuning before config pipe
        IspDrv* mpVirIspDrv=NULL;
        mpVirIspDrv=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ01_SYNC);
        isp_reg_t*  virIspReg=NULL;
        virIspReg=(isp_reg_t*)mpVirIspDrv->getRegAddr();
        TdriMgr::getInstance().init();
        //MY_LOGI("[christ]stage........ init tdri done, (0x%x)\n",(MUINT32)virIspReg);
        TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC,TDRI_MGR_FUNC_G2C);
        mpVirIspDrv->writeReg(0x00004A00, 0x012D0099);    /* 0x15004A00: CAM_G2C_CONV_0A */
        mpVirIspDrv->writeReg(0x00004A04, 0x0000003A);    /* 0x15004A04: CAM_G2C_CONV_0B */
        mpVirIspDrv->writeReg(0x00004A08, 0x075607AA);    /* 0x15004A08: CAM_G2C_CONV_1A */
        mpVirIspDrv->writeReg(0x00004A0C, 0x00000100);    /* 0x15004A0C: CAM_G2C_CONV_1B */
        mpVirIspDrv->writeReg(0x00004A10, 0x072A0100);    /* 0x15004A10: CAM_G2C_CONV_2A */
        mpVirIspDrv->writeReg(0x00004A14, 0x000007D6);    /* 0x15004A14: CAM_G2C_CONV_2B */
        TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC,TDRI_MGR_FUNC_G2C);

        ISP_WRITE_ENABLE_BITS(virIspReg, CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
        ISP_WRITE_ENABLE_BITS(virIspReg, CAM_CTL_EN2_SET, G2C_EN_SET, 1);
        MY_LOGI("[christ]stage........ set tuning done\n");
        ///////////////////////////////////////////


        MY_LOGI("[christ]stage2 done\n");
        //config pipe
        ret=mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        if(!ret)
        {
            MY_LOGE("postprocPipe config fail");
            return ret;
        }
        MY_LOGI("[christ]stage3 done\n");
        ///////////////////////////////////////////////////////////
        //enque buffer
        NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory,NSImageio::NSIspio::EPortIndex_IMGI,0);
        NSImageio::NSIspio::QBufInfo    rQBufInfo;
        for(int i=0;i<vPostProcInPorts.size();i++)
        {
            rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].u4BufSize = vPostProcInPorts[i]->u4BufSize; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA = vPostProcInPorts[i]->u4BufVA;
            rQBufInfo.vBufInfo[0].u4BufPA = vPostProcInPorts[i]->u4BufPA;
            rQBufInfo.vBufInfo[0].memID = vPostProcInPorts[i]->memID;
            ret=mpPostProcPipe->enqueInBuf(rPortID, rQBufInfo);
            if(!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                return ret;
            }
        }
       for(int i=0;i<vPostProcOutPorts.size();i++)
        {
            switch(vPostProcOutPorts[i]->index)
            {
                case NSImageio::NSIspio::EPortIndex_VIDO:
                        rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                    break;
                case NSImageio::NSIspio::EPortIndex_DISPO:
                        rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                        break;
                    case NSImageio::NSIspio::EPortIndex_IMG2O:
                    default:
                        rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                        rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O;
                        break;
            }
            rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].u4BufSize = vPostProcOutPorts[i]->u4BufSize; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA = vPostProcOutPorts[i]->u4BufVA;
            rQBufInfo.vBufInfo[0].u4BufPA = vPostProcOutPorts[i]->u4BufPA;
            rQBufInfo.vBufInfo[0].memID = vPostProcOutPorts[i]->memID;
            ret=mpPostProcPipe->enqueOutBuf(rPortID, rQBufInfo);
            if(!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                return ret;
            }
        }
        MY_LOGI("[christ]stage4.2 done\n");
         ///////////////////////////////////////////////////////////
        ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                      (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

        //set pass2 IN DMA register before pass2 start
        for(int i=0;i<vPostProcInPorts.size();i++)
        {
            mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcInPorts[i]->index),0,0);
        }
        //set pass2 OUT DMA register before pass2 start
        for(int i=0;i<vPostProcOutPorts.size();i++)
        {
            mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcOutPorts[i]->index),0,0);
        }
            //start
        ret=mpPostProcPipe->start();
        if(!ret)
        {
            MY_LOGE("P2 Start Fail!");
            return ret;
        }
       MY_LOGI("[christ]stage5 done.........\n");

        ///////////////////////////////////////////////////////////
        NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
        rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
        ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
           MY_LOGE("deque dispo out buffer fail,whileeeeee");
           return ret;
        }
        MY_LOGI("[christ]deque dispo done\n");
        rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
        ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGI("deque vido out buffer fail,whileeeeee");
            return ret;
        }
        MY_LOGE("[christ]deque vido done\n");
        rPortID.type = NSImageio::NSIspio::EPortType_Memory;
        rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
        ret=mpPostProcPipe->dequeInBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGE("deque imgi buffer fail,whileeeeee");
            return ret;
        }
        MY_LOGI("[christ]deque imgi done\n");
        ret=mpPostProcPipe->stop();
       MY_LOGI("[christ]stop done\n");
        ///////////////////////////////////////////////////////////
        //
        if (NULL != mpPostProcPipe)
        {
            if (MTRUE != mpPostProcPipe->uninit())
            {
                MY_LOGE("mpPostProcPipe uninit fail\n");
                ret = -1;
            }
            mpPostProcPipe->destroyInstance();
            mpPostProcPipe = NULL;
        }

        //save image
        //save images
        char filename[256];
        sprintf(filename, "/data/P2UT_1_1_0_wdmao_%dx%d.yuv", outImgW_dispo,outImgH_dispo);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dispoBuf.virtAddr), outImgW_dispo *outImgH_dispo * 2);
        char filename_2[256];
        sprintf(filename_2, "/data/P2UT_1_1_0_wroto_%dx%d.yuv", outImgW_vido,outImgH_vido);
        saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vidoBuf.virtAddr), outImgW_vido *outImgH_vido * 2);

        //free memory
        mpImemDrv->unmapPhyAddr(&dispoBuf);
        mpImemDrv->freeVirtBuf(&dispoBuf);
        mpImemDrv->unmapPhyAddr(&vidoBuf);
        mpImemDrv->freeVirtBuf(&vidoBuf);
    }



    //free memory
    mpImemDrv->unmapPhyAddr(&imgiBuf);
    mpImemDrv->freeVirtBuf(&imgiBuf);
    //free object
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit();
    mpIspDrv->destroyInstance();
    MY_LOGI("BasicEnqueDequeWithTuning EXIT\n");
    return ret;
}


int VSScase(int testitem)
{
    int ret=0;
    printf("VSScase ENTRY\n");

    NSImageio::NSIspio::IPostProcPipe *mpVSSPostProcPipe;


    IspDrv* mpIspDrv=NULL;
    mpIspDrv=IspDrv::createInstance();
    mpIspDrv->init();

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();

    {
        //
        MUINT32 reg_val=0x0;
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
        reg_val = ISP_IOCTL_READ_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_DMA_EN, 0);
        ISP_IOCTL_WRITE_BITS(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SPARE3,INT_MRG, 1);
        //ISP_IOCTL_WRITE_BITS(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SPARE3,INT_MRG, 1);

        IMEM_BUF_INFO vss_imgiBuf;
        int vss_inputImgW=3264,vss_inputImgH=1836;
        vss_imgiBuf.size=sizeof(g_imgi_array_3264x1836_b10);
        mpImemDrv->allocVirtBuf(&vss_imgiBuf);
        memcpy( (MUINT8*)(vss_imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_3264x1836_b10),vss_imgiBuf.size);
        mpImemDrv->mapPhyAddr(&vss_imgiBuf);
        vss_inputImgW=3264;
        vss_inputImgH=1836;
        int vss_outImgW=640,vss_outImgH=480;
        int vss_outImgW2=1840,vss_outImgH2=2448;

        IMEM_BUF_INFO vss_dispoBuf;
        IMEM_BUF_INFO vss_vidoBuf;
        vss_dispoBuf.size=vss_outImgW*vss_outImgH*2;
        mpImemDrv->allocVirtBuf(&vss_dispoBuf);
        mpImemDrv->mapPhyAddr(&vss_dispoBuf);
        memset((MUINT8*)vss_dispoBuf.virtAddr, 0x0, vss_dispoBuf.size);
        vss_vidoBuf.size=vss_outImgW2*vss_outImgH2*2;
        mpImemDrv->allocVirtBuf(&vss_vidoBuf);
        mpImemDrv->mapPhyAddr(&vss_vidoBuf);
        memset((MUINT8*)vss_vidoBuf.virtAddr, 0x0, vss_vidoBuf.size);

        {//VSS
            mpVSSPostProcPipe = NSImageio::NSIspio::IPostProcPipe::createInstance(NSImageio::NSIspio::eScenarioID_VSS,NSImageio::NSIspio::eScenarioFmt_RAW);
            ret=mpVSSPostProcPipe->init();
            if (NULL == mpVSSPostProcPipe || !mpVSSPostProcPipe->init())
            {
                MY_LOGE("mpVSSPostProcPipe init fail\n");
                return -1;
            }

            //set CQ first before pipe config
            ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,(MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ2/*EPIPE_PASS2_CQ2*/,0,0);
            ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
            ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)(MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);
            //
            vector<NSImageio::NSIspio::PortInfo const*> vVssPostProcInPorts;
            vector<NSImageio::NSIspio::PortInfo const*> vVssPostProcOutPorts;
            vVssPostProcInPorts.resize(1);
            vVssPostProcOutPorts.resize(2);//vVssPostProcOutPorts.resize(2);
            NSImageio::NSIspio::PortInfo vss_port_imgi;
            NSImageio::NSIspio::PortInfo vss_port_dispo,vss_port_vido;
            //input
            vss_port_imgi.eImgFmt = eImgFmt_BAYER10;
            vss_port_imgi.u4ImgWidth = vss_inputImgW;
            vss_port_imgi.u4ImgHeight = vss_inputImgH;
            vss_port_imgi.u4Stride[0] = vss_inputImgW;  //camera 1.0, using pixel domain
            vss_port_imgi.u4Stride[1] = 0;
            vss_port_imgi.u4Stride[2] = 0;
            vss_port_imgi.u4BufSize=vss_imgiBuf.size;
            vss_port_imgi.u4BufVA=vss_imgiBuf.virtAddr;
            vss_port_imgi.u4BufPA=vss_imgiBuf.phyAddr;
            vss_port_imgi.memID=vss_imgiBuf.memID;
            vss_port_imgi.u4IsRunSegment = 0;
            vss_port_imgi.type = NSImageio::NSIspio::EPortType_Memory;
            vss_port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
            vss_port_imgi.inout  = NSImageio::NSIspio::EPortDirection_In;
            vss_port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2B;//NSImageio::NSIspio::EPipePass_PASS2B;
            vss_port_imgi.crop.x = 408;
            vss_port_imgi.crop.y = 0;
            vss_port_imgi.crop.w = 2448;
            vss_port_imgi.crop.h = 1836;
            vVssPostProcInPorts.at(0)=&vss_port_imgi;
            //output
            vss_port_dispo.eImgFmt = eImgFmt_YV12;
            vss_port_dispo.u4ImgWidth = vss_outImgW;
            vss_port_dispo.u4ImgHeight = vss_outImgH;
            vss_port_dispo.eImgRot = NSImageio::NSIspio::eImgRot_0;              //dispo NOT support rotation
            vss_port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;          //dispo NOT support flip
            vss_port_dispo.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
            vss_port_dispo.index = NSImageio::NSIspio::EPortIndex_DISPO;
            vss_port_dispo.inout  = NSImageio::NSIspio::EPortDirection_Out;
            vss_port_dispo.u4Stride[0] = vss_outImgW;
            vss_port_dispo.u4Stride[1] = vss_outImgW/2;
            vss_port_dispo.u4Stride[2] = vss_outImgW/2;
            vss_port_dispo.u4BufSize=vss_dispoBuf.size;
            vss_port_dispo.u4BufVA=vss_dispoBuf.virtAddr;
            vss_port_dispo.u4BufPA=vss_dispoBuf.phyAddr;
            vss_port_dispo.memID=vss_dispoBuf.memID;
            vVssPostProcOutPorts.at(0)=&vss_port_dispo;
            //
            vss_port_vido.eImgFmt = eImgFmt_YV12;
            vss_port_vido.u4ImgWidth = vss_outImgW2;
            vss_port_vido.u4ImgHeight = vss_outImgH2;
            vss_port_vido.eImgRot = NSImageio::NSIspio::eImgRot_90;
            vss_port_vido.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;
            vss_port_vido.type = NSImageio::NSIspio::EPortType_VID_RDMA;
            vss_port_vido.index = NSImageio::NSIspio::EPortIndex_VIDO;
            vss_port_vido.inout  = NSImageio::NSIspio::EPortDirection_Out;
            vss_port_vido.u4Stride[0] = vss_outImgW2;
            vss_port_vido.u4Stride[1] =vss_outImgW2/2;
            vss_port_vido.u4Stride[2] = vss_outImgW2/2;
            vss_port_vido.u4BufSize=vss_vidoBuf.size;
            vss_port_vido.u4BufVA=vss_vidoBuf.virtAddr;
            vss_port_vido.u4BufPA=vss_vidoBuf.phyAddr;
            vss_port_vido.memID=vss_vidoBuf.memID;
            vVssPostProcOutPorts.at(1)=&vss_port_vido;

            ///////////////////////////////////////////
            //set tuning before config pipe
            #if 1
            IspDrv* mpVssVirIspDrv=NULL;
            mpVssVirIspDrv=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ02_SYNC);
            isp_reg_t*  vss_virIspReg=NULL;
            vss_virIspReg=(isp_reg_t*)mpVssVirIspDrv->getRegAddr();
            TdriMgr::getInstance().init();
            TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC,TDRI_MGR_FUNC_G2C);
            mpVssVirIspDrv->writeReg(0x00004A00, 0x012D0099);    /* 0x15004A00: CAM_G2C_CONV_0A */
            mpVssVirIspDrv->writeReg(0x00004A04, 0x0000003A);    /* 0x15004A04: CAM_G2C_CONV_0B */
            mpVssVirIspDrv->writeReg(0x00004A08, 0x075607AA);    /* 0x15004A08: CAM_G2C_CONV_1A */
            mpVssVirIspDrv->writeReg(0x00004A0C, 0x00000100);    /* 0x15004A0C: CAM_G2C_CONV_1B */
            mpVssVirIspDrv->writeReg(0x00004A10, 0x072A0100);    /* 0x15004A10: CAM_G2C_CONV_2A */
            mpVssVirIspDrv->writeReg(0x00004A14, 0x000007D6);    /* 0x15004A14: CAM_G2C_CONV_2B */
            TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC,TDRI_MGR_FUNC_G2C);
            ISP_WRITE_ENABLE_BITS(vss_virIspReg, CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(vss_virIspReg, CAM_CTL_EN2_SET, G2C_EN_SET, 1);
            #endif
            ///////////////////////////////////////////

            //config pipe
            ret=mpVSSPostProcPipe->configPipe(vVssPostProcInPorts, vVssPostProcOutPorts);
            if(!ret)
            {
                MY_LOGE("onThreadLoop postprocPipe config fail");
            }
            ///////////////////////////////////////////////////////////
            //enque buffer
            NSImageio::NSIspio::PortID vss_rPortID(NSImageio::NSIspio::EPortType_Memory,NSImageio::NSIspio::EPortIndex_IMGI,0);
            NSImageio::NSIspio::QBufInfo    vss_rQBufInfo;
            for(int i=0;i<vVssPostProcInPorts.size();i++)
            {
                vss_rQBufInfo.vBufInfo.resize(1);
                vss_rQBufInfo.vBufInfo[0].u4BufSize = vVssPostProcInPorts[i]->u4BufSize; //bytes
                vss_rQBufInfo.vBufInfo[0].u4BufVA = vVssPostProcInPorts[i]->u4BufVA;
                vss_rQBufInfo.vBufInfo[0].u4BufPA = vVssPostProcInPorts[i]->u4BufPA;
                vss_rQBufInfo.vBufInfo[0].memID = vVssPostProcInPorts[i]->memID;
                ret=mpVSSPostProcPipe->enqueInBuf(vss_rPortID, vss_rQBufInfo);
                if(!ret)
                {
                    MY_LOGE("onThreadLoop postprocPipe enque in buffer fail");
                }
            }
           for(int i=0;i<vVssPostProcOutPorts.size();i++)
            {
                switch(vVssPostProcOutPorts[i]->index)
                {
                    case NSImageio::NSIspio::EPortIndex_VIDO:
                            vss_rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                            vss_rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                        break;
                    case NSImageio::NSIspio::EPortIndex_DISPO:
                            vss_rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                            vss_rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                            break;
                        case NSImageio::NSIspio::EPortIndex_IMG2O:
                        default:
                            vss_rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                            vss_rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O;
                            break;
                }
                vss_rQBufInfo.vBufInfo.resize(1);
                vss_rQBufInfo.vBufInfo[0].u4BufSize = vVssPostProcOutPorts[i]->u4BufSize; //bytes
                vss_rQBufInfo.vBufInfo[0].u4BufVA = vVssPostProcOutPorts[i]->u4BufVA;
                vss_rQBufInfo.vBufInfo[0].u4BufPA = vVssPostProcOutPorts[i]->u4BufPA;
                vss_rQBufInfo.vBufInfo[0].memID = vVssPostProcOutPorts[i]->memID;
                ret=mpVSSPostProcPipe->enqueOutBuf(vss_rPortID, vss_rQBufInfo);
                if(!ret)
                {
                    MY_LOGE("onThreadLoop postprocPipe enque in buffer fail");
                }
            }
            MY_LOGI("[christ_onThreadLoop]stage4.2 done\n");
             ///////////////////////////////////////////////////////////
            ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                          (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

            //set pass2 IN DMA register before pass2 start
            for(int i=0;i<vVssPostProcInPorts.size();i++)
            {
                mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                      (MINT32)(vVssPostProcInPorts[i]->index),0,0);
            }
            //set pass2 OUT DMA register before pass2 start
            for(int i=0;i<vVssPostProcOutPorts.size();i++)
            {
                mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                      (MINT32)(vVssPostProcOutPorts[i]->index),0,0);
            }

            //start
            ret=mpVSSPostProcPipe->start();
            if(!ret)
            {
                MY_LOGE("onThreadLoop P2 Start Fail!");
            }


            NSImageio::NSIspio::QTimeStampBufInfo vss_rQTSBufInfo;
            bool getVss=false;
            do
            {
                vss_rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                ret=mpVSSPostProcPipe->dequeOutBuf(vss_rPortID,vss_rQTSBufInfo);
                if(!ret)
                {
                   MY_LOGE("[christ_onThreadLoop]deque dispo not yet\n");
                }
                else
                {
                    MY_LOGI("[christ_onThreadLoop]deque dispo done\n");
                    getVss=true;
                }
                vss_rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                ret=mpVSSPostProcPipe->dequeOutBuf(vss_rPortID,vss_rQTSBufInfo);
                if(!ret)
                {
                   MY_LOGE("[christ_onThreadLoop]deque vido not yet\n");
                   getVss=false;
                }
                else
                {
                    MY_LOGI("[christ_onThreadLoop]deque vido done\n");
                    getVss=true;
                }
                MY_LOGI("[christ_onThreadLoop]debuggg getVss(%d)\n",getVss);
                usleep(10000); //wait 10ms to simulate operating one preview frame
            }while(!getVss);

            vss_rPortID.type = NSImageio::NSIspio::EPortType_Memory;
            vss_rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
            ret=mpVSSPostProcPipe->dequeInBuf(vss_rPortID,vss_rQTSBufInfo);
            if(!ret)
            {
                MY_LOGE("onThreadLoop deque imgi buffer fail,whileeeeee");
            }
            ret=mpVSSPostProcPipe->stop();

           //save image
           //save images
            char filename[256];
            sprintf(filename, "/data/P2UT_1_2_0_wdmao_%dx%d.yuv", vss_outImgW,vss_outImgH);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(vss_dispoBuf.virtAddr), vss_outImgW *vss_outImgH * 2);
            char filename_2[256];
            sprintf(filename_2, "/data/P2UT_1_2_0_wroto_%dx%d.yuv", vss_outImgW2,vss_outImgH2);
            saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vss_vidoBuf.virtAddr), vss_outImgW2 *vss_outImgH2 * 2);
            ///////////////////////////////////////////////////////////
            //
            if (NULL != mpVSSPostProcPipe)
            {
                if (MTRUE != mpVSSPostProcPipe->uninit())
                {
                    MY_LOGE("onThreadLoop mpPostProcPipe uninit fail\n");
                    ret = -1;
                }
                mpVSSPostProcPipe->destroyInstance();
                mpVSSPostProcPipe = NULL;
            }

            //free memory
            mpImemDrv->unmapPhyAddr(&vss_dispoBuf);
            mpImemDrv->freeVirtBuf(&vss_dispoBuf);
            mpImemDrv->unmapPhyAddr(&vss_vidoBuf);
            mpImemDrv->freeVirtBuf(&vss_vidoBuf);
        }
    }

    //free memory
    //mpImemDrv->unmapPhyAddr(&imgiBuf);
    //mpImemDrv->freeVirtBuf(&imgiBuf);
    //free object
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit();
    mpIspDrv->destroyInstance();


    MY_LOGI("VSScase EXIT\n");

    return ret;
}

///////////////////////////////////////////////////////////////////////
int P2DVTcase4CQswitch(void)
{
    int ret=0;

    printf("P2DVTcase4CQswitch ENTRY\n");
    NSImageio::NSIspio::IPostProcPipe *mpPostProcPipe;
    NSImageio::NSIspio::IPostProcPipe *mpVSSPostProcPipe;

    IspDrv* mpIspDrv=NULL;
    mpIspDrv=IspDrv::createInstance();
    mpIspDrv->init();

    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
    //memory allocation
    IMEM_BUF_INFO imgiBuf;
    int inputImgW=1280,inputImgH=720;
    imgiBuf.size=sizeof(g_imgi_array_1280x720_b10);
    mpImemDrv->allocVirtBuf(&imgiBuf);
    memcpy( (MUINT8*)(imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_1280x720_b10), imgiBuf.size);
    mpImemDrv->mapPhyAddr(&imgiBuf);
    inputImgW=1280;
    inputImgH=720;

    bool getVss=false;
    //basic without crop/resize
    {
        /***************************************************/
        mpPostProcPipe = NSImageio::NSIspio::IPostProcPipe::createInstance(NSImageio::NSIspio::eScenarioID_VSS,NSImageio::NSIspio::eScenarioFmt_RAW);
        ret=mpPostProcPipe->init();
        if (NULL == mpPostProcPipe || !mpPostProcPipe->init())
        {
            MY_LOGE("mpPostProcPipe init fail\n");
            return -1;
        }
        MY_LOGI("[christ] flag-1\n");
        mpVSSPostProcPipe = NSImageio::NSIspio::IPostProcPipe::createInstance(NSImageio::NSIspio::eScenarioID_VSS,NSImageio::NSIspio::eScenarioFmt_RAW);
        MY_LOGI("[christ] flag-2\n");
        ret=mpVSSPostProcPipe->init();
        MY_LOGI("[christ] flag-3\n");
        if (NULL == mpVSSPostProcPipe || !mpVSSPostProcPipe->init())
        {
            MY_LOGE("mpVSSPostProcPipe init fail\n");
            return -1;
        }
        NSImageio::NSIspio::QTimeStampBufInfo vss_rQTSBufInfo;
        NSImageio::NSIspio::PortID vss_rPortID(NSImageio::NSIspio::EPortType_Memory,NSImageio::NSIspio::EPortIndex_IMGI,0);
        NSImageio::NSIspio::QBufInfo    vss_rQBufInfo;
        MY_LOGI("[christ] flag-4\n");

        //memory allocation for output dma
        int outImgW_dispo=1280,outImgH_dispo=720;
        int outImgW_vido=1280,outImgH_vido=720;
        IMEM_BUF_INFO dispoBuf;
        IMEM_BUF_INFO vidoBuf;
        dispoBuf.size=outImgW_dispo*outImgH_dispo*2;
        mpImemDrv->allocVirtBuf(&dispoBuf);
        mpImemDrv->mapPhyAddr(&dispoBuf);
        memset((MUINT8*)dispoBuf.virtAddr, 0x0, dispoBuf.size);
        vidoBuf.size=outImgW_vido*outImgH_vido*2;
        mpImemDrv->allocVirtBuf(&vidoBuf);
        mpImemDrv->mapPhyAddr(&vidoBuf);
        memset((MUINT8*)vidoBuf.virtAddr, 0x0, vidoBuf.size);
        MY_LOGI("[christ] buffer allocation done\n");
        MY_LOGI("[prv] imgi VA/PA/SIZE(0x%x,0x%x,0x%x),dispo VA/PA/SIZE(0x%x,0x%x,0x%x),vido VA/PA/SIZE(0x%x,0x%x,0x%x)\n",\
                imgiBuf.virtAddr,imgiBuf.phyAddr,imgiBuf.size,dispoBuf.virtAddr,dispoBuf.phyAddr,dispoBuf.size,vidoBuf.virtAddr,\
                vidoBuf.phyAddr,vidoBuf.size);

        /****************************************************/
        IMEM_BUF_INFO vss_imgiBuf;
        int vss_inputImgW=2560,vss_inputImgH=1440;
        vss_imgiBuf.size=sizeof(g_imgi_array_2560x1440_b10);
        mpImemDrv->allocVirtBuf(&vss_imgiBuf);
        memcpy( (MUINT8*)(vss_imgiBuf.virtAddr), (MUINT8*)(g_imgi_array_2560x1440_b10),vss_imgiBuf.size);
        mpImemDrv->mapPhyAddr(&vss_imgiBuf);
        vss_inputImgW=2560;
        vss_inputImgH=1440;
        int vss_outImgW=2560,vss_outImgH=1440;
        IMEM_BUF_INFO vss_dispoBuf;
        IMEM_BUF_INFO vss_vidoBuf;
        vss_dispoBuf.size=vss_outImgW*vss_outImgH*2;
        mpImemDrv->allocVirtBuf(&vss_dispoBuf);
        mpImemDrv->mapPhyAddr(&vss_dispoBuf);
        memset((MUINT8*)vss_dispoBuf.virtAddr, 0x0, vss_dispoBuf.size);
        vss_vidoBuf.size=vss_outImgW*vss_outImgH*2;
        mpImemDrv->allocVirtBuf(&vss_vidoBuf);
        mpImemDrv->mapPhyAddr(&vss_vidoBuf);
        memset((MUINT8*)vss_vidoBuf.virtAddr, 0x0, vss_vidoBuf.size);
        MY_LOGI("[vss] imgi VA/PA/SIZE(0x%x,0x%x,0x%x),dispo VA/PA/SIZE(0x%x,0x%x,0x%x),vido VA/PA/SIZE(0x%x,0x%x,0x%x)\n",\
                vss_imgiBuf.virtAddr,vss_imgiBuf.phyAddr,vss_imgiBuf.size,vss_dispoBuf.virtAddr,vss_dispoBuf.phyAddr,vss_dispoBuf.size,vss_vidoBuf.virtAddr,\
                vss_vidoBuf.phyAddr,vss_vidoBuf.size);
        //
        MUINT32 reg_val=0x0;
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_SEL, 0);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN1, 0x40000000);
        reg_val = ISP_IOCTL_READ_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2);
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_EN2, reg_val&0x04000000);//FMT_EN don't touch
        ISP_IOCTL_WRITE_ENABLE_REG(mpIspDrv,mpIspDrv->getRegAddrMap(),CAM_CTL_DMA_EN, 0);

        /////
        //for debug
        IspDrv* mpVirIspDrv_CQ1=NULL;
        mpVirIspDrv_CQ1=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ01);
        IspDrv* mpVirIspDrv_CQ2=NULL;
        mpVirIspDrv_CQ2=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ02);

        /****************************************************/
        for(int q=0;q<5;q++)
        {
            memset((MUINT8*)dispoBuf.virtAddr, 0x0, dispoBuf.size);
            memset((MUINT8*)vidoBuf.virtAddr, 0x0, vidoBuf.size);
            /////////////////////////////////////////////////////
            //set CQ first before pipe config
            ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,(MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ1,0,0);
            ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
            ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)(MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);
            MY_LOGI("[christ] (prv_%d) stage1 done\n",q);
            //////////////////////////////////////////////////////
            // dma preparation 1 in, 2 out
            vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
            vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
            vPostProcInPorts.resize(1);
            vPostProcOutPorts.resize(2);//vPostProcOutPorts.resize(2);
            NSImageio::NSIspio::PortInfo port_imgi;
            NSImageio::NSIspio::PortInfo port_dispo,port_vido;
            //input
            port_imgi.eImgFmt = eImgFmt_BAYER10;
            port_imgi.u4ImgWidth = inputImgW;
            port_imgi.u4ImgHeight = inputImgH;
            port_imgi.u4Stride[0] = inputImgW;  //camera 1.0, using pixel domain
            port_imgi.u4Stride[1] = 0;
            port_imgi.u4Stride[2] = 0;
            port_imgi.u4BufSize=imgiBuf.size;
            port_imgi.u4BufVA=imgiBuf.virtAddr;
            port_imgi.u4BufPA=imgiBuf.phyAddr;
            port_imgi.memID=imgiBuf.memID;
            port_imgi.u4IsRunSegment = 0;
            port_imgi.type = NSImageio::NSIspio::EPortType_Memory;
            port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
            port_imgi.inout  = NSImageio::NSIspio::EPortDirection_In;
            port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2;
            vPostProcInPorts.at(0)=&port_imgi;
            //output
            port_dispo.eImgFmt = eImgFmt_YUY2;
            port_dispo.u4ImgWidth = outImgW_dispo;
            port_dispo.u4ImgHeight = outImgH_dispo;
            port_dispo.eImgRot = NSImageio::NSIspio::eImgRot_0;              //dispo NOT support rotation
            port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;          //dispo NOT support flip
            port_dispo.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
            port_dispo.index = NSImageio::NSIspio::EPortIndex_DISPO;
            port_dispo.inout  = NSImageio::NSIspio::EPortDirection_Out;
            port_dispo.u4Stride[0] = outImgW_dispo;
            port_dispo.u4Stride[1] = 0;
            port_dispo.u4Stride[2] = 0;
            port_dispo.u4BufSize=dispoBuf.size;
            port_dispo.u4BufVA=dispoBuf.virtAddr;
            port_dispo.u4BufPA=dispoBuf.phyAddr;
            port_dispo.memID=dispoBuf.memID;
            vPostProcOutPorts.at(0)=&port_dispo;
            //
            port_vido.eImgFmt = eImgFmt_YUY2;
            port_vido.u4ImgWidth = outImgW_vido;
            port_vido.u4ImgHeight = outImgH_vido;
            port_vido.eImgRot = NSImageio::NSIspio::eImgRot_0;
            port_vido.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;
            port_vido.type = NSImageio::NSIspio::EPortType_VID_RDMA;
            port_vido.index = NSImageio::NSIspio::EPortIndex_VIDO;
            port_vido.inout  = NSImageio::NSIspio::EPortDirection_Out;
            port_vido.u4Stride[0] = outImgW_vido;
            port_vido.u4Stride[1] = 0;
            port_vido.u4Stride[2] = 0;
            port_vido.u4BufSize=vidoBuf.size;
            port_vido.u4BufVA=vidoBuf.virtAddr;
            port_vido.u4BufPA=vidoBuf.phyAddr;
            port_vido.memID=vidoBuf.memID;
            vPostProcOutPorts.at(1)=&port_vido;


            ///////////////////////////////////////////
            //set tuning before config pipe
            IspDrv* mpVirIspDrv=NULL;
            mpVirIspDrv=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ01_SYNC);
            isp_reg_t*  virIspReg=NULL;
            virIspReg=(isp_reg_t*)mpVirIspDrv->getRegAddr();
            TdriMgr::getInstance().init();
            //MY_LOGI("[christ]stage........ init tdri done, (0x%x)\n",(MUINT32)virIspReg);
            TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ01_SYNC,TDRI_MGR_FUNC_G2C);
            mpVirIspDrv->writeReg(0x00004A00, 0x012D0099);    /* 0x15004A00: CAM_G2C_CONV_0A */
            mpVirIspDrv->writeReg(0x00004A04, 0x0000003A);    /* 0x15004A04: CAM_G2C_CONV_0B */
            mpVirIspDrv->writeReg(0x00004A08, 0x075607AA);    /* 0x15004A08: CAM_G2C_CONV_1A */
            mpVirIspDrv->writeReg(0x00004A0C, 0x00000100);    /* 0x15004A0C: CAM_G2C_CONV_1B */
            mpVirIspDrv->writeReg(0x00004A10, 0x072A0100);    /* 0x15004A10: CAM_G2C_CONV_2A */
            mpVirIspDrv->writeReg(0x00004A14, 0x000007D6);    /* 0x15004A14: CAM_G2C_CONV_2B */
            TdriMgr::getInstance().applySetting(ISP_DRV_CQ01_SYNC,TDRI_MGR_FUNC_G2C);

            ISP_WRITE_ENABLE_BITS(virIspReg, CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
            ISP_WRITE_ENABLE_BITS(virIspReg, CAM_CTL_EN2_SET, G2C_EN_SET, 1);
            //MY_LOGI("[christ]stage........ set tuning done\n");
            ///////////////////////////////////////////


            MY_LOGI("[christ](prv_%d) stage2 done\n",q);
            MY_LOGI("(prv_%d) before config baseAddr imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpIspDrv->readReg(0x4230),mpIspDrv->readReg(0x4204),mpIspDrv->readReg(0x4208));
            MY_LOGI("(prv_%d) before config cq1 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ1->readReg(0x4230),mpVirIspDrv_CQ1->readReg(0x4204),mpVirIspDrv_CQ1->readReg(0x4208));
            MY_LOGI("(prv_%d) before config cq2 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ2->readReg(0x4230),mpVirIspDrv_CQ2->readReg(0x4204),mpVirIspDrv_CQ2->readReg(0x4208));
            //config pipe
            ret=mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
            if(!ret)
            {
                MY_LOGE("(prv_%d) postprocPipe config fail",q);
                return ret;
            }
            MY_LOGI("[christ] (prv_%d) stage3 done\n",q);
            MY_LOGI("(prv_%d) after config baseAddr imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpIspDrv->readReg(0x4230),mpIspDrv->readReg(0x4204),mpIspDrv->readReg(0x4208));
            MY_LOGI("(prv_%d) after config cq1 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ1->readReg(0x4230),mpVirIspDrv_CQ1->readReg(0x4204),mpVirIspDrv_CQ1->readReg(0x4208));
            MY_LOGI("(prv_%d) after config cq2 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ2->readReg(0x4230),mpVirIspDrv_CQ2->readReg(0x4204),mpVirIspDrv_CQ2->readReg(0x4208));
            ///////////////////////////////////////////////////////////
            //enque buffer
            NSImageio::NSIspio::PortID rPortID(NSImageio::NSIspio::EPortType_Memory,NSImageio::NSIspio::EPortIndex_IMGI,0);
            NSImageio::NSIspio::QBufInfo    rQBufInfo;
            for(int i=0;i<vPostProcInPorts.size();i++)
            {
                rQBufInfo.vBufInfo.resize(1);
                rQBufInfo.vBufInfo[0].u4BufSize = vPostProcInPorts[i]->u4BufSize; //bytes
                rQBufInfo.vBufInfo[0].u4BufVA = vPostProcInPorts[i]->u4BufVA;
                rQBufInfo.vBufInfo[0].u4BufPA = vPostProcInPorts[i]->u4BufPA;
                rQBufInfo.vBufInfo[0].memID = vPostProcInPorts[i]->memID;
                ret=mpPostProcPipe->enqueInBuf(rPortID, rQBufInfo);
                if(!ret)
                {
                    MY_LOGE("(prv_%d)postprocPipe enque in buffer fail",q);
                    return ret;
                }
            }
           for(int i=0;i<vPostProcOutPorts.size();i++)
            {
                switch(vPostProcOutPorts[i]->index)
                {
                    case NSImageio::NSIspio::EPortIndex_VIDO:
                            rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                            rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                        break;
                    case NSImageio::NSIspio::EPortIndex_DISPO:
                            rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                            rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                            break;
                        case NSImageio::NSIspio::EPortIndex_IMG2O:
                        default:
                            rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                            rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O;
                            break;
                }
                rQBufInfo.vBufInfo.resize(1);
                rQBufInfo.vBufInfo[0].u4BufSize = vPostProcOutPorts[i]->u4BufSize; //bytes
                rQBufInfo.vBufInfo[0].u4BufVA = vPostProcOutPorts[i]->u4BufVA;
                rQBufInfo.vBufInfo[0].u4BufPA = vPostProcOutPorts[i]->u4BufPA;
                rQBufInfo.vBufInfo[0].memID = vPostProcOutPorts[i]->memID;
                ret=mpPostProcPipe->enqueOutBuf(rPortID, rQBufInfo);
                if(!ret)
                {
                    MY_LOGE("(prv_%d)postprocPipe enque in buffer fail",q);
                    return ret;
                }
            }
            MY_LOGI("[christ](prv_%d) stage4.2 done\n",q);
             ///////////////////////////////////////////////////////////
            ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                          (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

            //set pass2 IN DMA register before pass2 start
            for(int i=0;i<vPostProcInPorts.size();i++)
            {
                mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                      (MINT32)(vPostProcInPorts[i]->index),0,0);
            }
            //set pass2 OUT DMA register before pass2 start
            for(int i=0;i<vPostProcOutPorts.size();i++)
            {
                mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                      (MINT32)(vPostProcOutPorts[i]->index),0,0);
            }
            MY_LOGI("(prv_%d) before start baseAddr imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpIspDrv->readReg(0x4230),mpIspDrv->readReg(0x4204),mpIspDrv->readReg(0x4208));
            MY_LOGI("(prv_%d) before start cq1 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ1->readReg(0x4230),mpVirIspDrv_CQ1->readReg(0x4204),mpVirIspDrv_CQ1->readReg(0x4208));
            MY_LOGI("(prv_%d) before start cq2 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ2->readReg(0x4230),mpVirIspDrv_CQ2->readReg(0x4204),mpVirIspDrv_CQ2->readReg(0x4208));
                //start
            ret=mpPostProcPipe->start();
            if(!ret)
            {
                MY_LOGE("(prv_%d)P2 Start Fail!",q);
                return ret;
            }
           MY_LOGI("[christ](prv_%d)stage5 done.........\n",q);
            MY_LOGI("(prv_%d) after start baseAddr imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpIspDrv->readReg(0x4230),mpIspDrv->readReg(0x4204),mpIspDrv->readReg(0x4208));
            MY_LOGI("(prv_%d) after start cq1 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ1->readReg(0x4230),mpVirIspDrv_CQ1->readReg(0x4204),mpVirIspDrv_CQ1->readReg(0x4208));
            MY_LOGI("(prv_%d) after start cq2 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ2->readReg(0x4230),mpVirIspDrv_CQ2->readReg(0x4204),mpVirIspDrv_CQ2->readReg(0x4208));
            ///////////////////////////////////////////////////////////
            NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
            rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
            rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
            ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
            if(!ret)
            {
               MY_LOGE("(prv_%d)deque dispo out buffer fail,whileeeeee",q);
               return ret;
            }
            MY_LOGI("[christ](prv_%d)deque dispo done\n",q);
            rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
            rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
            ret=mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
            if(!ret)
            {
                MY_LOGI("(prv_%d)deque vido out buffer fail,whileeeeee",q);
                return ret;
            }
            MY_LOGE("[christ](prv_%d)deque vido done\n",q);
            rPortID.type = NSImageio::NSIspio::EPortType_Memory;
            rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
            ret=mpPostProcPipe->dequeInBuf(rPortID,rQTSBufInfo);
            if(!ret)
            {
                MY_LOGE("(prv_%d)deque imgi buffer fail,whileeeeee",q);
                return ret;
            }
            MY_LOGI("[christ](prv_%d)deque imgi done\n",q);
            ret=mpPostProcPipe->stop();
            MY_LOGI("[christ](prv_%d)stop done\n",q);
            MY_LOGI("(prv_%d) after deque done baseAddr imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpIspDrv->readReg(0x4230),mpIspDrv->readReg(0x4204),mpIspDrv->readReg(0x4208));
            MY_LOGI("(prv_%d) after deque done cq1 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ1->readReg(0x4230),mpVirIspDrv_CQ1->readReg(0x4204),mpVirIspDrv_CQ1->readReg(0x4208));
            MY_LOGI("(prv_%d) after deque done cq2 imgi(0x%x), tdri (0x%x/0x%x)\n",q,mpVirIspDrv_CQ2->readReg(0x4230),mpVirIspDrv_CQ2->readReg(0x4204),mpVirIspDrv_CQ2->readReg(0x4208));
            //save image
            //save images
            mpImemDrv->cacheFlushAll();
            char filename[256];
            sprintf(filename, "/data/P2DVT_prv_%d_wdmao_%dx%d.yuv", q,outImgW_dispo,outImgH_dispo);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(dispoBuf.virtAddr), outImgW_dispo *outImgH_dispo * 2);
            char filename_2[256];
            sprintf(filename_2, "/data/P2DVT_prv_%d_wroto_%dx%d.yuv", q,outImgW_vido,outImgH_vido);
            saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vidoBuf.virtAddr), outImgW_vido *outImgH_vido * 2);

            ///////////////////////////////////////////////////////////////
            //do vss operation after each preview

            if(q==0)
            {
                //set CQ first before pipe config
                ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,(MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ2/*EPIPE_PASS2_CQ2*/,0,0);
                ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
                ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE, (MINT32)(MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);

                MY_LOGI("[christ] (vss) stage1 done\n",q);
                //
                vector<NSImageio::NSIspio::PortInfo const*> vVssPostProcInPorts;
                vector<NSImageio::NSIspio::PortInfo const*> vVssPostProcOutPorts;
                vVssPostProcInPorts.resize(1);
                vVssPostProcOutPorts.resize(2);//vVssPostProcOutPorts.resize(2);
                NSImageio::NSIspio::PortInfo vss_port_imgi;
                NSImageio::NSIspio::PortInfo vss_port_dispo,vss_port_vido;
                //input
                vss_port_imgi.eImgFmt = eImgFmt_BAYER10;
                vss_port_imgi.u4ImgWidth = vss_inputImgW;
                vss_port_imgi.u4ImgHeight = vss_inputImgH;
                vss_port_imgi.u4Stride[0] = vss_inputImgW;  //camera 1.0, using pixel domain
                vss_port_imgi.u4Stride[1] = 0;
                vss_port_imgi.u4Stride[2] = 0;
                vss_port_imgi.u4BufSize=vss_imgiBuf.size;
                vss_port_imgi.u4BufVA=vss_imgiBuf.virtAddr;
                vss_port_imgi.u4BufPA=vss_imgiBuf.phyAddr;
                vss_port_imgi.memID=vss_imgiBuf.memID;
                vss_port_imgi.u4IsRunSegment = 0;
                vss_port_imgi.type = NSImageio::NSIspio::EPortType_Memory;
                vss_port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
                vss_port_imgi.inout  = NSImageio::NSIspio::EPortDirection_In;
                vss_port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2B;//NSImageio::NSIspio::EPipePass_PASS2B;
                vVssPostProcInPorts.at(0)=&vss_port_imgi;
                //output
                vss_port_dispo.eImgFmt = eImgFmt_YUY2;
                vss_port_dispo.u4ImgWidth = vss_outImgW;
                vss_port_dispo.u4ImgHeight = vss_outImgH;
                vss_port_dispo.eImgRot = NSImageio::NSIspio::eImgRot_0;              //dispo NOT support rotation
                vss_port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;          //dispo NOT support flip
                vss_port_dispo.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                vss_port_dispo.index = NSImageio::NSIspio::EPortIndex_DISPO;
                vss_port_dispo.inout  = NSImageio::NSIspio::EPortDirection_Out;
                vss_port_dispo.u4Stride[0] = vss_outImgW;
                vss_port_dispo.u4Stride[1] = 0;
                vss_port_dispo.u4Stride[2] = 0;
                vss_port_dispo.u4BufSize=vss_dispoBuf.size;
                vss_port_dispo.u4BufVA=vss_dispoBuf.virtAddr;
                vss_port_dispo.u4BufPA=vss_dispoBuf.phyAddr;
                vss_port_dispo.memID=vss_dispoBuf.memID;
                vVssPostProcOutPorts.at(0)=&vss_port_dispo;
                //
                vss_port_vido.eImgFmt = eImgFmt_YUY2;
                vss_port_vido.u4ImgWidth = vss_outImgW;
                vss_port_vido.u4ImgHeight = vss_outImgH;
                vss_port_vido.eImgRot = NSImageio::NSIspio::eImgRot_0;
                vss_port_vido.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;
                vss_port_vido.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                vss_port_vido.index = NSImageio::NSIspio::EPortIndex_VIDO;
                vss_port_vido.inout  = NSImageio::NSIspio::EPortDirection_Out;
                vss_port_vido.u4Stride[0] = vss_outImgW;
                vss_port_vido.u4Stride[1] = 0;
                vss_port_vido.u4Stride[2] = 0;
                vss_port_vido.u4BufSize=vss_vidoBuf.size;
                vss_port_vido.u4BufVA=vss_vidoBuf.virtAddr;
                vss_port_vido.u4BufPA=vss_vidoBuf.phyAddr;
                vss_port_vido.memID=vss_vidoBuf.memID;
                vVssPostProcOutPorts.at(1)=&vss_port_vido;

                MY_LOGI("[christ](vss_0) stage2 done\n");
                ///////////////////////////////////////////
                //set tuning before config pipe
                IspDrv* mpVssVirIspDrv=NULL;
                mpVssVirIspDrv=mpIspDrv->getCQInstance((ISP_DRV_CQ_ENUM)ISP_DRV_CQ02_SYNC);
                isp_reg_t*  vss_virIspReg=NULL;
                vss_virIspReg=(isp_reg_t*)mpVssVirIspDrv->getRegAddr();
                TdriMgr::getInstance().init();
                TdriMgr::getInstance().setOtherEngin(ISP_DRV_CQ02_SYNC,TDRI_MGR_FUNC_G2C);
                mpVssVirIspDrv->writeReg(0x00004A00, 0x012D0099);    /* 0x15004A00: CAM_G2C_CONV_0A */
                mpVssVirIspDrv->writeReg(0x00004A04, 0x0000003A);    /* 0x15004A04: CAM_G2C_CONV_0B */
                mpVssVirIspDrv->writeReg(0x00004A08, 0x075607AA);    /* 0x15004A08: CAM_G2C_CONV_1A */
                mpVssVirIspDrv->writeReg(0x00004A0C, 0x00000100);    /* 0x15004A0C: CAM_G2C_CONV_1B */
                mpVssVirIspDrv->writeReg(0x00004A10, 0x072A0100);    /* 0x15004A10: CAM_G2C_CONV_2A */
                mpVssVirIspDrv->writeReg(0x00004A14, 0x000007D6);    /* 0x15004A14: CAM_G2C_CONV_2B */
                TdriMgr::getInstance().applySetting(ISP_DRV_CQ02_SYNC,TDRI_MGR_FUNC_G2C);
                ISP_WRITE_ENABLE_BITS(vss_virIspReg, CAM_CTL_EN2_CLR, G2C_EN_CLR, 0);
                ISP_WRITE_ENABLE_BITS(vss_virIspReg, CAM_CTL_EN2_SET, G2C_EN_SET, 1);
                ///////////////////////////////////////////
                MY_LOGI("[christ](vss_0) stage3 done\n");
                //config pipe
                ret=mpVSSPostProcPipe->configPipe(vVssPostProcInPorts, vVssPostProcOutPorts);
                if(!ret)
                {
                    MY_LOGE("vss postprocPipe config fail");
                }
                ///////////////////////////////////////////////////////////
                //enque buffer
                MY_LOGI("[christ](vss_0) stage4 done\n");
                for(int i=0;i<vVssPostProcInPorts.size();i++)
                {
                    vss_rQBufInfo.vBufInfo.resize(1);
                    vss_rQBufInfo.vBufInfo[0].u4BufSize = vVssPostProcInPorts[i]->u4BufSize; //bytes
                    vss_rQBufInfo.vBufInfo[0].u4BufVA = vVssPostProcInPorts[i]->u4BufVA;
                    vss_rQBufInfo.vBufInfo[0].u4BufPA = vVssPostProcInPorts[i]->u4BufPA;
                    vss_rQBufInfo.vBufInfo[0].memID = vVssPostProcInPorts[i]->memID;
                    ret=mpVSSPostProcPipe->enqueInBuf(vss_rPortID, vss_rQBufInfo);
                    if(!ret)
                    {
                        MY_LOGE("vss postprocPipe enque in buffer fail");
                    }
                }
               for(int i=0;i<vVssPostProcOutPorts.size();i++)
                {
                    switch(vVssPostProcOutPorts[i]->index)
                    {
                        case NSImageio::NSIspio::EPortIndex_VIDO:
                                vss_rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                            break;
                        case NSImageio::NSIspio::EPortIndex_DISPO:
                                vss_rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                                break;
                            case NSImageio::NSIspio::EPortIndex_IMG2O:
                            default:
                                vss_rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_IMG2O;
                                break;
                    }
                    vss_rQBufInfo.vBufInfo.resize(1);
                    vss_rQBufInfo.vBufInfo[0].u4BufSize = vVssPostProcOutPorts[i]->u4BufSize; //bytes
                    vss_rQBufInfo.vBufInfo[0].u4BufVA = vVssPostProcOutPorts[i]->u4BufVA;
                    vss_rQBufInfo.vBufInfo[0].u4BufPA = vVssPostProcOutPorts[i]->u4BufPA;
                    vss_rQBufInfo.vBufInfo[0].memID = vVssPostProcOutPorts[i]->memID;
                    ret=mpVSSPostProcPipe->enqueOutBuf(vss_rPortID, vss_rQBufInfo);
                    if(!ret)
                    {
                        MY_LOGE("vss postprocPipe enque in buffer fail");
                    }
                }
                MY_LOGI("[christ](vss_0) stage4.2 done\n");
                 ///////////////////////////////////////////////////////////
                ret = mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                              (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

                //set pass2 IN DMA register before pass2 start
                for(int i=0;i<vVssPostProcInPorts.size();i++)
                {
                    mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                          (MINT32)(vVssPostProcInPorts[i]->index),0,0);
                }
                //set pass2 OUT DMA register before pass2 start
                for(int i=0;i<vVssPostProcOutPorts.size();i++)
                {
                    mpVSSPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                          (MINT32)(vVssPostProcOutPorts[i]->index),0,0);
                }

                //start
                ret=mpVSSPostProcPipe->start();
                if(!ret)
                {
                    MY_LOGE("vss P2 Start Fail!");
                }


            }

            //check vss buffer is ready or not after each preview
            if(!getVss)
            {
                vss_rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
                ret=mpVSSPostProcPipe->dequeOutBuf(vss_rPortID,vss_rQTSBufInfo);
                if(!ret)
                {
                   MY_LOGE("[christ](vss_%d) deque dispo not yet\n",q);
                }
                else
                {
                    MY_LOGI("[christ](vss_%d) deque dispo done\n",q);
                    getVss=true;
                }
                vss_rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
                vss_rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
                ret=mpVSSPostProcPipe->dequeOutBuf(vss_rPortID,vss_rQTSBufInfo);
                if(!ret)
                {
                   MY_LOGE("[christ](vss_%d) deque vido not yet\n",q);
                   getVss=false;
                }
                else
                {
                    MY_LOGI("[christ](vss_%d) deque vido done\n",q);
                    getVss=true;
                }
                MY_LOGI("[christ](vss_%d) debuggg getVss(%d)\n",q,getVss);

                if(getVss)
                {
                    vss_rPortID.type = NSImageio::NSIspio::EPortType_Memory;
                    vss_rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
                    ret=mpVSSPostProcPipe->dequeInBuf(vss_rPortID,vss_rQTSBufInfo);
                    if(!ret)
                    {
                        MY_LOGE("vss deque imgi buffer fail,whileeeeee");
                    }
                    ret=mpVSSPostProcPipe->stop();

                    mpImemDrv->cacheFlushAll();
                    char filename[256];
                    sprintf(filename, "/data/P2DVT_vss_wdmao_%dx%d.yuv", vss_outImgW,vss_outImgH);
                    saveBufToFile(filename, reinterpret_cast<MUINT8*>(vss_dispoBuf.virtAddr), vss_outImgW *vss_outImgH * 2);
                    char filename_2[256];
                    sprintf(filename_2, "/data/P2DVT_vss_wroto_%dx%d.yuv", vss_outImgW,vss_outImgH);
                    saveBufToFile(filename_2, reinterpret_cast<MUINT8*>(vss_vidoBuf.virtAddr), vss_outImgW *vss_outImgH * 2);
                }
            }


        }
        ///////////////////////////////////////////////////////////
        //
        if (NULL != mpPostProcPipe)
        {
            if (MTRUE != mpPostProcPipe->uninit())
            {
                MY_LOGE("mpPostProcPipe uninit fail\n");
                ret = -1;
            }
            mpPostProcPipe->destroyInstance();
            mpPostProcPipe = NULL;
        }
        //free memory
        mpImemDrv->unmapPhyAddr(&dispoBuf);
        mpImemDrv->freeVirtBuf(&dispoBuf);
        mpImemDrv->unmapPhyAddr(&vidoBuf);
        mpImemDrv->freeVirtBuf(&vidoBuf);

        if (NULL != mpVSSPostProcPipe)
        {
            if (MTRUE != mpVSSPostProcPipe->uninit())
            {
                MY_LOGE("onThreadLoop mpPostProcPipe uninit fail\n");
                ret = -1;
            }
            mpVSSPostProcPipe->destroyInstance();
            mpVSSPostProcPipe = NULL;
        }

        //free memory
        mpImemDrv->unmapPhyAddr(&vss_dispoBuf);
        mpImemDrv->freeVirtBuf(&vss_dispoBuf);
        mpImemDrv->unmapPhyAddr(&vss_vidoBuf);
        mpImemDrv->freeVirtBuf(&vss_vidoBuf);
    }


    //free memory
    mpImemDrv->unmapPhyAddr(&imgiBuf);
    mpImemDrv->freeVirtBuf(&imgiBuf);
    //free object
    mpImemDrv->uninit();
    mpImemDrv->destroyInstance();
    mpIspDrv->uninit();
    mpIspDrv->destroyInstance();
    MY_LOGI("P2DVTcase4CQswitch EXIT\n");


    return ret;
}
