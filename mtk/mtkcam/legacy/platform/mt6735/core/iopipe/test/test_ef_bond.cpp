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

#define LOG_TAG "EFTest"

#include <vector>

using namespace std;

#include <linux/cache.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <stdio.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>

#include <mtkcam/iopipe/PostProc/INormalStream.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_scenario.h>
#include <mtkcam/drv/imem_drv.h>
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/imageio/ispio_utility.h>

#include <mtkcam/IImageBuffer.h>
#include <utils/StrongPointer.h>
#include <mtkcam/utils/common.h>
#include <mtkcam/utils/ImageBufferHeap.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>


using namespace android;

using namespace NSCam;
using namespace NSIoPipe;
using namespace NSPostProc;
using namespace NSImageio;
using namespace NSIspio;

/*************************************************************************************
* Log Utility
*************************************************************************************/
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

#include <imageio_log.h>    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(EFTest);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (EFTest_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (EFTest_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (EFTest_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (EFTest_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (EFTest_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (EFTest_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

#define FUNCTION_LOG_START      LOG_INF("+");
#define FUNCTION_LOG_END        LOG_INF("-");
#define ERROR_LOG               LOG_ERR("Error");




#define BD_SEG_NUM (4+1) //5M, 8M, 13M, 16M, no limitration
#define TG_grabH 40
struct efMappingTable
{
    int                TG_grabW;
    const char*     resultStr;
};


efMappingTable efTable[BD_SEG_NUM]
{
    {2048,   "Bonding at 5M"},    //temp width
    {3364,   "Bonding at 8M"},
    {4324,   "Bonding at 13M"},
    {4708,   "Bonding at 16M"},
    {5120,   "No limitation"}//{5264,   "No limitation"}
};

static vector<PortInfo const*> g_vCamIOInPorts(1);
static vector<PortInfo const*> g_vCamIOIOutPorts(1);


/*******************************************************************************
*  only support 1 pixel mode, so we should revise halsensor.control.cpp
********************************************************************************/
int ef_test()
{
    LOG_INF("+");
    int ret=0;
        int resultIdx=0;
    //
    //1. create sensor object
    int i=0;
    SensorStaticInfo  sensorStaticInfo[3];
    SensorDynamicInfo sensorDynamicInfo[3];
    MUINT32 sensorArray[1] = {0}, sensorArray2[1] = {1};
    IHalSensor::ConfigParam configParam[2];
    char const*const szCallerName = "ef_test";
    MUINT32 sensorDevId     = SENSOR_DEV_MAIN;
    IHalSensor* pHalSensor  = NULL;
    IHalSensorList*const pHalSensorList = IHalSensorList::get();

    //search sensor
    pHalSensorList->searchSensors();
    MUINT const sensorNum = pHalSensorList->queryNumberOfSensors();
    LOG_INF("sensorNum(%d)", sensorNum);//debug
    for (i = 0; i < sensorNum; i++)
    {
        LOG_INF(" i(%d),name:%s type:%d", i,pHalSensorList->queryDriverName(i), pHalSensorList->queryType(i));//debug
        LOG_INF(" SensorDevIdx:%d", pHalSensorList->querySensorDevIdx(i));//debug
    }

    //power on
    pHalSensor = pHalSensorList->createSensor(szCallerName, 1, &sensorArray[0]);//modify
    if  ( ! pHalSensor ) {
        LOG_ERR("createSensor fail");
    }
    pHalSensor->powerOn(szCallerName, 1, &sensorArray[0]);
    usleep(5000);
    pHalSensorList->querySensorStaticInfo(SENSOR_DEV_MAIN,&sensorStaticInfo[0]);
    LOG_INF(" Main preview width = %d, height = %d\n",sensorStaticInfo[0].previewWidth,sensorStaticInfo[0].previewHeight);
    LOG_INF(" Main capture width = %d, height = %d\n",sensorStaticInfo[0].captureWidth,sensorStaticInfo[0].captureHeight);
    pHalSensor->querySensorDynamicInfo(SENSOR_DEV_MAIN, &sensorDynamicInfo[0]);
    LOG_INF("TgInfo[0] = %d\n", sensorDynamicInfo[0].TgInfo, sensorDynamicInfo[1].TgInfo, sensorDynamicInfo[2].TgInfo);
    LOG_INF("pixelMode[0] = %d\n", sensorDynamicInfo[0].pixelMode, sensorDynamicInfo[1].pixelMode, sensorDynamicInfo[2].pixelMode);

    MUINT32 u4BitDepth = sensorStaticInfo[0].rawSensorBit;
        MUINT32 u4Order = sensorStaticInfo[0].sensorFormatOrder;
        MUINT32 sensorFmt;
    switch(u4BitDepth)
    {
        case 0 :
            sensorFmt  = eImgFmt_BAYER8;
                break;
        case 1 :
            sensorFmt  = eImgFmt_BAYER10;
                break;
        case 2 :
            sensorFmt  = eImgFmt_BAYER12;
                break;
        case 3 :
            sensorFmt  = eImgFmt_BAYER12; //To do : raw 14 bit
                break;
        default :
            sensorFmt = eImgFmt_UNKNOWN;
                LOG_ERR("unknown raw image bit depth(%u)",u4BitDepth);
    }

    //2. pass1 object and port information
    ICamIOPipe *m_pICamIOPipe;
    m_pICamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, eScenarioFmt_RAW, (MINT8 const*)szCallerName);
    m_pICamIOPipe->init();
    PortInfo m_tgi;
    PortInfo m_imgo;
           PortInfo m_rrzo;
    NSImageio::NSIspio::PortID mPortID;
    QBufInfo rQBufInfo;
    IMemDrv* mpImemDrv=NULL;
    mpImemDrv=IMemDrv::createInstance();
    mpImemDrv->init();
            IspDrv* mpIspDrv=NULL;
            mpIspDrv=IspDrv::createInstance();
            mpIspDrv->init("eftest");
    IMEM_BUF_INFO imgo_buf[3];
           IMEM_BUF_INFO rrzo_buf[3];
    QTimeStampBufInfo rQTSBufInfo;

        //3. run each cases
        int stride=0;
        for(int q=0;q<BD_SEG_NUM;q++)
        {

         LOG_INF("i(%d)",q);

                    //reset
                    mpIspDrv->reset(ISP_DRV_RST_CAM_P1);
                    mpIspDrv->reset(ISP_DRV_RST_CAM_P1);
                    LOG_INF("reset done");
                    //


         resultIdx=q;
         //config sensor, TG grab window
        configParam[0] =
        {
            index               : 0, //modify
            crop                : MSize(efTable[q].TG_grabW,TG_grabH), //FIXME
            scenarioId          : SENSOR_SCENARIO_ID_NORMAL_CAPTURE,
            isBypassScenario    : 0,
            isContinuous        : 1,
            HDRMode             : 0,
            framerate           : 0, //default frame rate
            twopixelOn          : 0,
            debugMode           : 1,//test model
        };
        pHalSensor->configure(1, &configParam[0]);

                    #if 1
                      MINT32 u32Enable = 1;
                      MINT32 err = pHalSensor->sendCommand(SENSOR_DEV_MAIN,
                                                                 SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                                                 (MUINTPTR)&u32Enable,
                                                                 0,
                                                                 0);
                      #endif
        //port TG
        m_tgi.ePxlMode    = ePxlMode_One_Single; //only support one pixel mode in test pattern, need update codes in halsensor.control.cpp
        m_tgi.index       = NSImageio::NSIspio::EPortIndex_TG1I; //main sensor
        m_tgi.eImgFmt     = (EImageFormat)sensorFmt; //  Image Pixel Format
        m_tgi.eRawPxlID   = (ERawPxlID)u4Order;
        m_tgi.u4ImgWidth  = efTable[q].TG_grabW;  // Image Width
        m_tgi.u4ImgHeight = TG_grabH; // Image Height
        m_tgi.u4PureRaw   = 1;
        m_tgi.u4PureRawPak = 1;
        m_tgi.type        = NSImageio::NSIspio::EPortType_Sensor;               // EPortType
        m_tgi.inout       = NSImageio::NSIspio::EPortDirection_In;              // 0:in/1:out

        //imgo
        m_imgo.index = NSImageio::NSIspio::EPortIndex_IMGO;
        m_imgo.eImgFmt      = m_tgi.eImgFmt;     // Image Pixel Format
        m_imgo.u4PureRaw    = 0;
        m_imgo.u4ImgWidth   = m_tgi.u4ImgWidth;  // Image Width
        m_imgo.u4ImgHeight  = m_tgi.u4ImgHeight; // Image Height
        m_imgo.crop1.y      = 0;
        m_imgo.crop1.floatY = 0;
                     m_imgo.crop1.w   = m_imgo.u4ImgWidth;  // Image Width
        m_imgo.crop1.h = m_imgo.u4ImgHeight;
        m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = efTable[q].TG_grabW*10/8;  //queryRawStride((MUINT32)m_imgo.eImgFmt , m_imgo.u4ImgWidth);//efTable[i].TG_grabW*10/8;
        m_imgo.u4Stride[ESTRIDE_2ND_PLANE] = 0;
        m_imgo.u4Stride[ESTRIDE_3RD_PLANE] = 0;
        m_imgo.type  = NSImageio::NSIspio::EPortType_Memory;        // EPortType
        m_imgo.index = NSImageio::NSIspio::EPortIndex_IMGO;         // port index
        m_imgo.inout = NSImageio::NSIspio::EPortDirection_Out;      // 0:in/1:out
                    m_rrzo.eImgFmt        = eImgFmt_FG_BAYER10;  //  Image Pixel Format
                    m_rrzo.u4ImgWidth     = m_tgi.u4ImgWidth;//3072; //3072;        //  Image Width
                    m_rrzo.u4ImgHeight    = m_tgi.u4ImgHeight; //1920;        //  Image Height
                    m_rrzo.crop1.x         = 0;
                    m_rrzo.crop1.y         = 0;
                    m_rrzo.crop1.floatX    = 0;
                    m_rrzo.crop1.floatY    = 0;
                    if(m_rrzo.u4ImgWidth > 3072)
                    {
                        MINT32 rrzowidth,rrzoheight;
                        rrzowidth = 2048;
                        rrzoheight = (((m_rrzo.u4ImgHeight*2048)/m_rrzo.u4ImgWidth)>>2)<<2;//rrzoheight* ((MFLOAT)m_imgo.u4ImgWidth/3072);
                        m_rrzo.u4ImgWidth = rrzowidth;
                        m_rrzo.u4ImgHeight = rrzoheight;
                    }
                    m_rrzo.crop1.w         = m_rrzo.u4ImgWidth;
                    m_rrzo.crop1.h         = m_rrzo.u4ImgHeight;
                    m_rrzo.u4Stride[ESTRIDE_1ST_PLANE] = m_rrzo.crop1.w*10/8*1.5;//  unit:PIXEL
                    m_rrzo.type   = NSImageio::NSIspio::EPortType_Memory;     //  EPortType
                    m_rrzo.index  = NSImageio::NSIspio::EPortIndex_RRZO;     //  port index
                    m_rrzo.inout  = NSImageio::NSIspio::EPortDirection_Out; //  0:in/1:out

        //set CQ first before pipe config
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS1_CQ0,0,0);
        //single trigger & config
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
        //config pipe
        g_vCamIOInPorts.at(0)   = &m_tgi;
                    #if 0
        g_vCamIOIOutPorts.resize(1); //revise for 6795
           g_vCamIOIOutPorts.at(0) = &m_imgo;
                    #else
                    g_vCamIOIOutPorts.resize(2);
                    g_vCamIOIOutPorts.at(0) = &m_imgo;
                    g_vCamIOIOutPorts.at(1) = &m_rrzo;
                    #endif
                    LOG_INF("start m_pICamIOPipe->configPipe()");
           if(MTRUE != m_pICamIOPipe->configPipe(g_vCamIOInPorts, g_vCamIOIOutPorts))
        {
            LOG_ERR("m_pICamIOPipe->configPipe() fail");
            return -1;
        }

        //enque pass1 output buffer
        rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].header   = NULL;
        for(int j=0;j<3;j++)
        {
            //allocate memory
            imgo_buf[j].size=m_imgo.u4Stride[ESTRIDE_1ST_PLANE]*m_imgo.u4ImgHeight;
            LOG_INF("w/h/stride/size (%d/%d/%d/%d)",m_imgo.u4ImgWidth,m_imgo.u4ImgHeight,m_imgo.u4Stride[ESTRIDE_1ST_PLANE],imgo_buf[j].size);
            mpImemDrv->allocVirtBuf(&(imgo_buf[j]));
            mpImemDrv->mapPhyAddr(&(imgo_buf[j]));

            mPortID.index = EPortIndex_IMGO;
            rQBufInfo.vBufInfo[0].memID[0]   = imgo_buf[j].memID;
            rQBufInfo.vBufInfo[0].u4BufSize[0] = imgo_buf[j].size;  //bytes
            rQBufInfo.vBufInfo[0].u4BufVA[0] = imgo_buf[j].virtAddr;
            rQBufInfo.vBufInfo[0].u4BufPA[0] = imgo_buf[j].phyAddr;
                                rQBufInfo.vBufInfo[0].mBufIdx=j;
            LOG_INF("IMGO m_pICamIOPipe->enqueOutBuf[%d].u4BufPA(%x).size(0x%x)",j,rQBufInfo.vBufInfo[0].u4BufVA[0],rQBufInfo.vBufInfo[0].u4BufSize[0]);
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
                                rrzo_buf[j].size=m_rrzo.u4Stride[ESTRIDE_1ST_PLANE]*m_rrzo.u4ImgHeight;
            LOG_INF("w/h/stride/size (%d/%d/%d/%d)",m_rrzo.u4ImgWidth,m_rrzo.u4ImgHeight,m_rrzo.u4Stride[ESTRIDE_1ST_PLANE],rrzo_buf[j].size);
            mpImemDrv->allocVirtBuf(&(rrzo_buf[j]));
            mpImemDrv->mapPhyAddr(&(rrzo_buf[j]));

                                mPortID.index = EPortIndex_RRZO;
                                rQBufInfo.vBufInfo[0].memID[0]   = rrzo_buf[j].memID;
                                rQBufInfo.vBufInfo[0].u4BufSize[0] = rrzo_buf[j].size;  //bytes
                                rQBufInfo.vBufInfo[0].u4BufVA[0] = rrzo_buf[j].virtAddr;
                                rQBufInfo.vBufInfo[0].u4BufPA[0] = rrzo_buf[j].phyAddr;
                                rQBufInfo.vBufInfo[0].mBufIdx=j;
                                m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
                                LOG_INF("RRZO m_pICamIOPipe->enqueOutBuf[%d].u4BufPA(%x).size(0x%x)",j,rQBufInfo.vBufInfo[0].u4BufVA[0],rQBufInfo.vBufInfo[0].u4BufSize[0]);
        }

                    //force to enable TM_EN in seninf
                    MUINT32 valuee=mpIspDrv->readReg(0x8208);
                    LOG_INF("DAMN valuee(0x%x)\n",valuee);
                    printf("DAMN valuee(0x%x)\n",valuee);
                    mpIspDrv->writeReg(0x8208,(valuee|0x1),ISP_DRV_USER_ISPF);
                    valuee=mpIspDrv->readReg(0x8208);
                    LOG_INF("DAMN valuee_2(0x%x)",valuee);
                    printf("DAMN valuee_2(0x%x)\n",valuee);


                     LOG_INF("startCQ0()");
        m_pICamIOPipe->startCQ0();
        //continuous mode
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
        //wait VD to align

             valuee=mpIspDrv->readReg(0x8208);
                    LOG_INF("DAMN valuee3(0x%x)\n",valuee);
                    printf("DAMN valuee3(0x%x)\n",valuee);
                    mpIspDrv->writeReg(0x8208,(valuee|0x1),ISP_DRV_USER_ISPF);
                    valuee=mpIspDrv->readReg(0x8208);
                    LOG_INF("DAMN valuee_4(0x%x)",valuee);
                    printf("DAMN valuee_4(0x%x)\n",valuee);
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_VSYNC,ISP_DRV_IRQ_USER_ACDK);

        //check HW reg setting
        LOG_INF("DAMN valuee3(0x%x)\n",mpIspDrv->readReg(0x4418));
        printf("ISP_BOND = 0x%x\n", mpIspDrv->readReg(0x4418));

        //start
        m_pICamIOPipe->start();

            //dequeue pass1 OUT buffer
                LOG_INF("start to Deque()");
            mPortID.index = NSImageio::NSIspio::EPortIndex_IMGO;
            ret=m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo,eDrvScenario_CC,6,0);
            if(!ret)
            {
               printf("(%d), %s_ISP FAIL!!!!!!!!!!!!!\n",resultIdx,efTable[resultIdx].resultStr);
                LOG_ERR("(%d), %s_ISP FAIL!!!!!!!!!!",resultIdx,efTable[resultIdx].resultStr);
                resultIdx=resultIdx-1;    //real pass segment is the previous one
                printf("get failllllllllllllllllllllllll, enter while\n");
                LOG_INF("resultIdx, ret(%d/%d)",resultIdx,ret);
                    LOG_INF("BONDING RESULT: %s",efTable[resultIdx].resultStr);
                    LOG_INF("enter while for debuggggg");
                    while(1);
                goto EXIT;
            }

                     LOG_INF("stop()");
            //stop
            if(MTRUE != m_pICamIOPipe->stop())
        {
            LOG_ERR("m_pICamIOPipe->stop() fail");
        }

        //
        LOG_INF("%s pass",efTable[q].resultStr);

        //free memory
        for(int j=0;j<3;j++)
        {
            mpImemDrv->unmapPhyAddr(&(imgo_buf[j]));
            mpImemDrv->freeVirtBuf(&(imgo_buf[j]));
        }
    }




EXIT:
        LOG_INF("resultIdx, ret(%d/%d)",resultIdx,ret);
    LOG_INF("BONDING RESULT: %s",efTable[resultIdx].resultStr);
        if(!ret)
        {
            //debug
            printf("get fail, enter while\n");
             LOG_INF("enter while for debug");
            while(1);
        }

    pHalSensor->powerOff(szCallerName, 1, &sensorArray[0]);
    if(m_pICamIOPipe != NULL)
    {
        m_pICamIOPipe->uninit();
        m_pICamIOPipe->destroyInstance();
        m_pICamIOPipe = NULL;
    }
    if(mpImemDrv!=NULL)
    {
        mpImemDrv->uninit();
        mpImemDrv->destroyInstance();
        mpImemDrv=NULL;
    }
            if(mpIspDrv!=NULL)
           {
                mpIspDrv->uninit("eftest");
                mpIspDrv->destroyInstance();
         mpIspDrv=NULL;
           }

    LOG_INF("TEST DONE!!!!");
    return ret;
}


