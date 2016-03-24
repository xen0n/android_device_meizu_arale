#if 1
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
#include "ts_it_imageio.h"

#define BUF_NUM             3

#define VIDO_WIDTH          (1280)
#define VIDO_HEIGHT         (720)
#define DISPO_WIDTH         (640) //(2088)
#define DISPO_HEIGHT        (480) //(1544)


static int m_TestCase = 0;  //0: test pattern, 1: preview
static MUINT32 m_CurDev = SENSOR_DEV_MAIN;

namespace NSImageio {
namespace NSIspio{

Ts_IT TestIT;
Ts_IT *pTestIT = &TestIT;


/******************************************************************************
* Memory
*******************************************************************************/

static void allocMem(IMEM_BUF_INFO &memBuf)
{
    MY_LOGD("size(%u)", memBuf.size );
    if (pTestIT->mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (pTestIT->mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
}

static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (pTestIT->mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if (pTestIT->mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
    }
}

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

/*******************************************************************************
*
********************************************************************************/
Ts_IT::
Ts_IT()
    : mSensorDev(0)
    , mpSensorHal(NULL)
    , mSensorCallerName("Ts_IT")
    , mfgIsYUVPortON(MFALSE)
    , continuous(1)
    , is_yuv_sensor(0)
    , u4SensorFullWidth(0)
    , u4SensorFullHeight(0)
    , u4SensorHalfWidth(0)
    , u4SensorHalfHeight(0)
    , u4SensorWidth(0)
    , u4SensorHeight(0)
{
    //sensorDevId = is_yuv_sensor ? SENSOR_DEV_SUB : SENSOR_DEV_MAIN;
}

Ts_IT::
~Ts_IT()
{

}

/*******************************************************************************
*  Sensor init
********************************************************************************/
int Ts_IT::sensorInit()
{
#if FAKE_SENSOR == 1
    TS_FakeSensorList       *pHalSensorList = 0;
#else
    IHalSensorList          *pHalSensorList = 0;
#endif
    MUINT32                 sensorNum = 0;
    MINT32 supportedSensorDev = 0;
    MINT32 ret;

    MY_LOGD("E\n");

    mSensorDev = m_CurDev;

    MUINT32 tg_grab_w = 0;
    MUINT32 tg_grab_h = 0;
    MUINT32 max_prv_w = 0;
    MUINT32 max_prv_h = 0;

#if FAKE_SENSOR == 1
    pHalSensorList = TS_FakeSensorList::get();
#else
    pHalSensorList = IHalSensorList::get();
#endif
    pHalSensorList->searchSensors();

    sensorNum = pHalSensorList->queryNumberOfSensors();

    if (sensorNum == 0)
    {
        MY_LOGE("Sensor Not found");
        return -1;
    }

    //Get sensor information
    for (MUINT i = 0; i < sensorNum; i++){
        MY_LOGD("name:%s type:%d", pHalSensorList->queryDriverName(i), 0/*pHalSensorList->queryType(i)*/);//debug
        MY_LOGD("index:%d, SensorDevIdx:%d",i ,pHalSensorList->querySensorDevIdx(i));//debug
        supportedSensorDev |= pHalSensorList->querySensorDevIdx(i);
    }

    //======================================================
    // Open sensor device
    //======================================================

    for (MUINT i = 0; i < sensorNum; i++){
        if (pHalSensorList->querySensorDevIdx(i) == mSensorDev)
        {
            mSensorIndex = i;
            break;
        }
    }
    MY_LOGD("mSensorIndex = %d",mSensorIndex);

    //====== Sensor Init======
    //set current sensor device
    mpSensorHal = pHalSensorList->createSensor(mSensorCallerName, 1, &mSensorIndex);//modify
    if (mpSensorHal == NULL)
    {
        MY_LOGE("mpSensorHal is NULL");
        return -1;
    }

    //mSensorInit = MTRUE;
    //======================================================
    // Get sensor size
    //======================================================
    pHalSensorList->querySensorStaticInfo(mSensorDev, &sensorStaticInfo[0]);

    u4SensorHalfWidth   = sensorStaticInfo[0].previewWidth;
    u4SensorHalfHeight  = sensorStaticInfo[0].previewHeight;
    u4SensorFullWidth   = sensorStaticInfo[0].captureWidth;
    u4SensorFullHeight  = sensorStaticInfo[0].captureHeight;

    MY_LOGD("Sensor resolution, Prv(%d,%d),Full(%d,%d)\n",u4SensorHalfWidth,
            u4SensorHalfHeight, u4SensorFullWidth, u4SensorFullHeight);

    //mSensorModeNum = sensorStaticInfo[0].SensorModeNum;

    //======================================================
    // Sensor power on
    //======================================================
    mpSensorHal->powerOn(mSensorCallerName, 1, &mSensorIndex);

    //====== Get Sensor Size ======

    if (mode == 0)
    {
        u4SensorWidth = u4SensorHalfWidth;
        u4SensorHeight = u4SensorHalfHeight;
    }
    else
    {
        u4SensorWidth = u4SensorFullWidth;
        u4SensorHeight = u4SensorFullHeight;
    }

    IHalSensor::ConfigParam configParam[2];

    configParam[0].index        = mSensorIndex;
    configParam[0].isBypassScenario = 0;
    configParam[0].isContinuous = 1;
    configParam[0].HDRMode      = 0;
    configParam[0].framerate    = 0;
    configParam[0].twopixelOn   = 0;
    configParam[0].debugMode    = 0; //1 for output test model from seninf
    if (mode == 0)
    {
        configParam[0].scenarioId = SENSOR_SCENARIO_ID_NORMAL_PREVIEW;
        configParam[0].crop = MSize(sensorStaticInfo[0].previewWidth, sensorStaticInfo[0].previewHeight);
    }
    else
    {   // sensor full size
        configParam[0].scenarioId = SENSOR_SCENARIO_ID_NORMAL_CAPTURE;
        configParam[0].crop = MSize(sensorStaticInfo[0].captureWidth, sensorStaticInfo[0].captureHeight);
    }

    mpSensorHal->configure(1, &configParam[0]);

    mpSensorHal->querySensorDynamicInfo(mSensorDev, &sensorDynamicInfo[0]);

    if (m_TestCase == 0)
    {
        MINT32 u32Enable = 1;
        mpSensorHal->sendCommand(mSensorDev,
                                   SENSOR_CMD_SET_TEST_PATTERN_OUTPUT,
                                   (MUINTPTR)&u32Enable,
                                   0,
                                   0);
        MY_LOGD("Sensor Test Pattern");
    }

    MY_LOGD("X\n");

    return 0;
}

/*******************************************************************************
*  Sensor init /floria
********************************************************************************/
int Ts_IT::sensorUninit()
{
    MY_LOGD("[SetSensor_uninit] E\n");

    if (mpSensorHal)
    {
        mpSensorHal->powerOff(mSensorCallerName, 1, &mSensorIndex);
        mpSensorHal->destroyInstance(mSensorCallerName);
        mpSensorHal= NULL;
    }

    MY_LOGD("[SetSensor_uninit] X\n");

    return 0;
}

/******************************************************************************
* ConfigPipe & ConfigSensor
*******************************************************************************/
MBOOL
Ts_IT::
setConfigPortInfo()
{
    NSCam::EImageFormat eSensorFmt = NSCam::eImgFmt_BAYER10;
    MUINT32 u4SensorStride = 0;
    MBOOL ret = MTRUE;

    MY_LOGD("E");

    u4SensorStride = ((u4SensorWidth + 7) & ~7); //align8 for RAW10

    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    ret = ret
            && mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)EPIPE_PASS1_CQ0,
                                   0,
                                   0
                                   )
            && mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)EPIPE_PASS1_CQ0,
                                  (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                                  (MINT32)EPIPECQ_TRIG_BY_START
                                  )
            && mpCamIOPipe->sendCommand(EPIPECmd_SET_CONFIG_STAGE, //TBD
                                  (MINT32)eConfigSettingStage_Init,
                                   0,
                                   0
                                  );
    if (!ret)
    {
        MY_LOGE("Cammand queue config fail:%d", mpCamIOPipe->getLastErrorCode());
        return ret;
    }

    vector<PortInfo const*> vCamIOInPorts;

    MY_LOGD("Sensor: (devID)(%d) (w,h,fmt,bits,pixID)(%d,%d,0x%x,%d,%d)",
            mSensorDev,
            u4SensorWidth,
            u4SensorHeight,
            eSensorFmt,
            sensorStaticInfo[0].rawSensorBit,
            sensorStaticInfo[0].sensorFormatOrder);

    PortInfo tgi;
    tgi.eImgFmt = eSensorFmt;
    tgi.eRawPxlID = (ERawPxlID)sensorStaticInfo[0].sensorFormatOrder;
    tgi.u4ImgWidth = u4SensorWidth;
    tgi.u4ImgHeight = u4SensorHeight;
    tgi.u4Stride[0] = u4SensorStride;
    tgi.u4Stride[1] = 0;
    tgi.u4Stride[2] = 0;
    tgi.type = EPortType_Sensor;
    tgi.index = EPortIndex_TG1I;
    tgi.inout  = EPortDirection_In;
    tgi.u4BufSize  = (MUINT32)0;
    vCamIOInPorts.push_back(&tgi);

    vector<PortInfo const*> vCamIOOutPorts;
    PortInfo imgo;
    PortInfo img2o;

    if (1)
    {
        imgo.eImgFmt = tgi.eImgFmt;
        imgo.u4ImgWidth = tgi.u4ImgWidth;
        imgo.u4ImgHeight = tgi.u4ImgHeight;
        // no crop
        imgo.crop.y = 0;
        imgo.crop.h = imgo.u4ImgHeight;
        imgo.type = EPortType_Memory;
        imgo.index = EPortIndex_IMGO;
        imgo.inout  = EPortDirection_Out;
        imgo.u4Stride[0] = u4SensorStride;
        imgo.u4Stride[1] = 0;
        imgo.u4Stride[2] = 0;
        imgo.u4Offset = 0;
        vCamIOOutPorts.push_back(&imgo);
    }
    if (0)
    {
        img2o.eImgFmt = tgi.eImgFmt;
        img2o.u4ImgWidth = tgi.u4ImgWidth;
        img2o.u4ImgHeight = tgi.u4ImgHeight;
        img2o.crop.y = 0;
        img2o.crop.h = img2o.u4ImgHeight;
        img2o.type = EPortType_Memory;
        img2o.index = EPortIndex_IMG2O;
        img2o.inout  = EPortDirection_Out;
        img2o.u4Stride[0] = u4SensorStride;
        img2o.u4Stride[1] = 0;
        img2o.u4Stride[2] = 0;
        vCamIOOutPorts.push_back(&img2o);
        mfgIsYUVPortON = MTRUE;
    }

    ret = mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);
    MY_LOGD("configPipe(ret=%d)", ret);

    // query skip frame to wait for stable
    mu4SkipFrame = 0;

    MY_LOGD("X");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_IT::
skipFrame(MUINT32 const u4SkipCount)
{
    MY_LOGD(" + (u4SkipCount) = (%d)", u4SkipCount);
    MBOOL ret = MTRUE;
    for (MUINT32 i = 0; i < u4SkipCount ; i++)
    {
        //
        QTimeStampBufInfo rQTimeOutBufInfo;
        PortID rPortID(EPortType_Memory,
                       EPortIndex_IMGO,
                       1);
        //
        ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
        if (!ret)
        {
            MY_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMGO) fail ");
            return ret;
        }

        //
        mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);

        if (mfgIsYUVPortON)
        {
            rPortID.index =  EPortIndex_IMG2O;
            ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
            //
            if (!ret)
            {
                MY_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMG2O) fail ");
                return ret;
            }
            //
            mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);
        }
        //ret = mpCamIOPipe->irq( (NSImageio::NSIspio::EPipePass_PASS1_TG1) ,
        //                   NSImageio::NSIspio::EPIPEIRQ_VSYNC
        //                   );
    }

    return ret;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_IT::
start()
{
    MBOOL ret = MTRUE;

    MY_LOGD("E");

    ret = mpCamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER,
                                   (MINT32)EPortIndex_IMGO,
                                    0,
                                    0
                                  );

    if (mfgIsYUVPortON)
    {
       ret = mpCamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER,
                                      (MINT32)EPortIndex_IMG2O,
                                       0,
                                       0
                                      );
    }

    MY_LOGD("mpCamIOPipe->startCQ0");
    ret = mpCamIOPipe->startCQ0();

    // ! let commond queue trigger mode as continuous mode
    #if 0
    ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
    #else
    ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)EPIPE_CQ_NONE,
                                  0,
                                  0
                                   ); //[?]
    #endif

    ret = mpCamIOPipe->start();

    MY_LOGD("mpCamIOPipe->irq(VSYNC)");
    ret = mpCamIOPipe->irq(EPipePass_PASS1_TG1,
                           EPIPEIRQ_VSYNC
                           );

    MY_LOGD("Wait for stable frame:%d", mu4SkipFrame);
    skipFrame(mu4SkipFrame);

    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Ts_IT::
freeRawMem()
{
    if(vRawMem.size() > 0)
    {
        MY_LOGD("deallocMem\n");
        for (int i = 0; i < BUF_NUM; i++)
        {
            char filename[256];
            sprintf(filename, "/data/IT_%dx%d_%02d.raw", u4SensorWidth, u4SensorHeight, i);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(vRawMem.at(i).virtAddr), (u4SensorWidth*u4SensorHeight*10)/8);
            deallocMem(vRawMem.at(i));
        }
    }
}

/*******************************************************************************
*
********************************************************************************/
MVOID*
Ts_IT::
previewProc(MVOID * arg)
{
    MY_LOGD("E");
    int count = (int)((MUINTPTR)arg), i = 0;
    MINT32 ret = 0;

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    do
    {
        MY_LOGD("-----------------------------------");
#if 1
        QTimeStampBufInfo rQTimeOutBufInfo, rQTimeOutBufInfoYuv;
        PortID rPortID(EPortType_Memory, EPortIndex_IMGO, 1);

        //mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);

        MY_LOGD(" mpCamIOPipe->dequeOutBuf(RAW)\n");
        pTestIT->mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
        if (rQTimeOutBufInfo.vBufInfo.size() == 0)
        {
            MY_LOGE("Deque imgo buf no buf");
            goto EXIT_DEQ;
        }

        #if 1
        MY_LOGD("Deque buf count : %d", rQTimeOutBufInfo.vBufInfo.size());
        for (i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++) {
            MY_LOGD("Deque buf #%d : size(%d), VA(x%x), PA(x%x), memID(%d)", i,
                    rQTimeOutBufInfo.vBufInfo.at(i).u4BufSize,
                    rQTimeOutBufInfo.vBufInfo.at(i).u4BufVA,
                    rQTimeOutBufInfo.vBufInfo.at(i).u4BufPA,
                    rQTimeOutBufInfo.vBufInfo.at(i).memID);
        }
        #endif
        if (pTestIT->mfgIsYUVPortON)
        {
            MY_LOGD(" mpCamIOPipe->dequeOutBuf(YUV)\n");
            rPortID.index = EPortIndex_IMG2O;
            pTestIT->mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfoYuv);
            if (rQTimeOutBufInfoYuv.vBufInfo.size() == 0)
            {
                MY_LOGE("Deque imgo buf no buf");
                goto EXIT_DEQ;
            }
        }
#endif

        /*****************************************************
         * Pass 2
         *****************************************************/
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcInPorts;
        vector<NSImageio::NSIspio::PortInfo const*> vPostProcOutPorts;
        NSImageio::NSIspio::PortInfo port_imgi;
        NSImageio::NSIspio::PortInfo port_dispo, port_vido;

        vPostProcInPorts.resize(1);
#if 0 //temp vido
        vPostProcOutPorts.resize(1);
#else
        vPostProcOutPorts.resize(2);
#endif
        //input
        port_imgi.eImgFmt = eImgFmt_BAYER10;
        port_imgi.u4ImgWidth = pTestIT->u4SensorWidth;
        port_imgi.u4ImgHeight = pTestIT->u4SensorHeight;
        port_imgi.u4Stride[0] = pTestIT->u4SensorWidth;
        port_imgi.u4Stride[1] = 0;
        port_imgi.u4Stride[2] = 0;
        port_imgi.u4BufSize = rQTimeOutBufInfo.vBufInfo.at(0).u4BufSize;
        port_imgi.u4BufVA   = rQTimeOutBufInfo.vBufInfo.at(0).u4BufVA;
        port_imgi.u4BufPA   = rQTimeOutBufInfo.vBufInfo.at(0).u4BufPA;
        port_imgi.memID     = rQTimeOutBufInfo.vBufInfo.at(0).memID;
        port_imgi.crop.x = 0;
        port_imgi.crop.y = 0;
        port_imgi.crop.w = pTestIT->u4SensorWidth;
        port_imgi.crop.h = pTestIT->u4SensorHeight;
        port_imgi.u4IsRunSegment = 0;
        port_imgi.type  = NSImageio::NSIspio::EPortType_Memory;
        port_imgi.index = NSImageio::NSIspio::EPortIndex_IMGI;
        port_imgi.inout = NSImageio::NSIspio::EPortDirection_In;
        port_imgi.pipePass = NSImageio::NSIspio::EPipePass_PASS2;
        vPostProcInPorts.at(0) = &port_imgi;
        //output
        port_dispo.eImgFmt = eImgFmt_YUY2;
        port_dispo.u4ImgWidth = DISPO_WIDTH;//pTestIT->u4SensorWidth;
        port_dispo.u4ImgHeight = DISPO_HEIGHT;//pTestIT->u4SensorHeight;
        port_dispo.eImgRot  = NSImageio::NSIspio::eImgRot_0;            //dispo NOT support rotation
        port_dispo.eImgFlip = NSImageio::NSIspio::eImgFlip_OFF;         //dispo NOT support flip
        port_dispo.type     = NSImageio::NSIspio::EPortType_DISP_RDMA;
        port_dispo.index    = NSImageio::NSIspio::EPortIndex_DISPO;
        port_dispo.inout    = NSImageio::NSIspio::EPortDirection_Out;
        port_dispo.u4Stride[0] = DISPO_WIDTH;//pTestIT->u4SensorWidth;
        port_dispo.u4Stride[1] = 0;
        port_dispo.u4Stride[2] = 0;
        port_dispo.u4BufSize = pTestIT->dispoBuf.size;
        port_dispo.u4BufVA  = pTestIT->dispoBuf.virtAddr;
        port_dispo.u4BufPA  = pTestIT->dispoBuf.phyAddr;
        port_dispo.memID    = pTestIT->dispoBuf.memID;
        vPostProcOutPorts.at(0) = &port_dispo;
        //
        port_vido.eImgFmt   = eImgFmt_YUY2;
        port_vido.u4ImgWidth = VIDO_WIDTH;//pTestIT->u4SensorWidth;
        port_vido.u4ImgHeight = VIDO_HEIGHT;//pTestIT->u4SensorHeight;
        port_vido.eImgRot   = NSImageio::NSIspio::eImgRot_0;
        port_vido.eImgFlip  = NSImageio::NSIspio::eImgFlip_OFF;
        port_vido.type      = NSImageio::NSIspio::EPortType_VID_RDMA;
        port_vido.index     = NSImageio::NSIspio::EPortIndex_VIDO;
        port_vido.inout     = NSImageio::NSIspio::EPortDirection_Out;
        port_vido.u4Stride[0] = VIDO_WIDTH;//pTestIT->u4SensorWidth;
        port_vido.u4Stride[1] = 0;
        port_vido.u4Stride[2] = 0;
        port_vido.u4BufSize = pTestIT->vidoBuf.size;
        port_vido.u4BufVA   = pTestIT->vidoBuf.virtAddr;
        port_vido.u4BufPA   = pTestIT->vidoBuf.phyAddr;
        port_vido.memID     = pTestIT->vidoBuf.memID;
#if 1 //temp vido
        vPostProcOutPorts.at(1) = &port_vido;
#endif
        ret = pTestIT->mpPostProcPipe->configPipe(vPostProcInPorts, vPostProcOutPorts);
        if (!ret)
        {
            MY_LOGE("postprocPipe config fail");
            break;
        }
        ///////////////////////////////////////////////////////////
        //enque buffer
        NSImageio::NSIspio::QBufInfo rQBufInfo;

        rPortID.type = EPortType_Memory;
        rPortID.index = EPortIndex_IMGI;
        rPortID.inout = 0;
        for (int i = 0; i < vPostProcInPorts.size(); i++)
        {
            rQBufInfo.vBufInfo.resize(1);
            rQBufInfo.vBufInfo[0].u4BufSize = vPostProcInPorts[i]->u4BufSize; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA = vPostProcInPorts[i]->u4BufVA;
            rQBufInfo.vBufInfo[0].u4BufPA = vPostProcInPorts[i]->u4BufPA;
            rQBufInfo.vBufInfo[0].memID = vPostProcInPorts[i]->memID;
            ret = pTestIT->mpPostProcPipe->enqueInBuf(rPortID, rQBufInfo);
            if (!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                break;
            }
        }

        for (int i=0; i<vPostProcOutPorts.size(); i++)
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
            ret = pTestIT->mpPostProcPipe->enqueOutBuf(rPortID, rQBufInfo);
            if (!ret)
            {
                MY_LOGE("postprocPipe enque in buffer fail");
                break;
            }
        }
        ///////////////////////////////////////////////////////////
        ret = pTestIT->mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
                                      (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger, 0,0);

        //set pass2 IN DMA register before pass2 start
        for(int i=0;i<vPostProcInPorts.size();i++)
        {
            pTestIT->mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcInPorts[i]->index),0,0);
        }
        //set pass2 OUT DMA register before pass2 start
        for(int i=0;i<vPostProcOutPorts.size();i++)
        {
            pTestIT->mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
                                  (MINT32)(vPostProcOutPorts[i]->index),0,0);
        }

        ret = pTestIT->mpPostProcPipe->start();
        if(!ret)
        {
            MY_LOGE("P2 Start Fail!");
            break;
        }

        ///////////////////////////////////////////////////////////
        NSImageio::NSIspio::QTimeStampBufInfo rQTSBufInfo;
        rPortID.type = NSImageio::NSIspio::EPortType_DISP_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_DISPO;
        ret = pTestIT->mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if (!ret)
        {
            MY_LOGE("deque dispo out buffer fail,whileeeeee");
            while(1);
            break;
        }
#if 1//temp vido
        rPortID.type = NSImageio::NSIspio::EPortType_VID_RDMA;
        rPortID.index = NSImageio::NSIspio::EPortIndex_VIDO;
        ret = pTestIT->mpPostProcPipe->dequeOutBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGI("deque vido out buffer fail,whileeeeee");
            while(1);
            break;
        }
#endif
        rPortID.type = NSImageio::NSIspio::EPortType_Memory;
        rPortID.index = NSImageio::NSIspio::EPortIndex_IMGI;
        ret = pTestIT->mpPostProcPipe->dequeInBuf(rPortID,rQTSBufInfo);
        if(!ret)
        {
            MY_LOGE("deque imgi buffer fail,whileeeeee");
            while(1);
            break;
        }
        ret = pTestIT->mpPostProcPipe->stop();

#if 1
        /*****************************************************
         * Pass 1 Enqueue buffer back
         *****************************************************/
        QBufInfo rOutBufInfo;

        for (MUINT32 i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++)
        {
             NSImageio::NSIspio::BufInfo rBuf(rQTimeOutBufInfo.vBufInfo.at(i).u4BufSize,
                     rQTimeOutBufInfo.vBufInfo.at(i).u4BufVA,
                     rQTimeOutBufInfo.vBufInfo.at(i).u4BufPA,
                     rQTimeOutBufInfo.vBufInfo.at(i).memID,
                     rQTimeOutBufInfo.vBufInfo.at(i).bufSecu,
                     rQTimeOutBufInfo.vBufInfo.at(i).bufCohe);
             rOutBufInfo.vBufInfo.push_back(rBuf);
        }

        rPortID.type = EPortType_Memory;
        rPortID.index = EPortIndex_IMGO;
        rPortID.inout = 1;
        pTestIT->mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);

        if (pTestIT->mfgIsYUVPortON)
        {
            QBufInfo rOutBufInfo;
            PortID rPortID(EPortType_Memory, EPortIndex_IMG2O, 1);

            for (MUINT32 i = 0; i < rQTimeOutBufInfoYuv.vBufInfo.size(); i++)
            {
                 NSImageio::NSIspio::BufInfo rBuf(rQTimeOutBufInfoYuv.vBufInfo.at(i).u4BufSize,
                         rQTimeOutBufInfoYuv.vBufInfo.at(i).u4BufVA,
                         rQTimeOutBufInfoYuv.vBufInfo.at(i).u4BufPA,
                         rQTimeOutBufInfoYuv.vBufInfo.at(i).memID,
                         rQTimeOutBufInfoYuv.vBufInfo.at(i).bufSecu,
                         rQTimeOutBufInfoYuv.vBufInfo.at(i).bufCohe);
                 rOutBufInfo.vBufInfo.push_back(rBuf);
            }

            pTestIT->mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);
        }
#else
        if(MFALSE == pTestIT->enqueBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut, 0, 1),pTestIT->rRawBuf))
        {
            MY_LOGD("enqueBuf failed");
            break;
        }
#endif
    } while(--count > 0);

EXIT_DEQ:
    MY_LOGD("X");
    MY_LOGD("-----------------------------------");

    pthread_exit(0);

    return NULL;
}

/*******************************************************************************
*
********************************************************************************/
MVOID
Ts_IT::
TS_Thread_Init(int count)
{
    MY_LOGD("E");

    Mutex::Autolock lock(mLock);

    MY_LOGD("mInitCount(%d)", mInitCount);

    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
    }
    else
    {
        // Init semphore
        ::sem_init(&m_semTestEnDequethread, 0, 0);

        // Create main thread for preview and capture
        pthread_create(&m_TestEnDequethread, NULL, previewProc, (void *)((MUINTPTR)count));
        android_atomic_inc(&mInitCount);
        pthread_join(m_TestEnDequethread, NULL);
    }

    MY_LOGD("X");
}

MBOOL
Ts_IT::
TS_Thread_UnInit()
{
    MY_LOGD("E");

    MBOOL ret = MTRUE;
    Mutex::Autolock lock(mLock);

    MY_LOGD("mInitCount(%d)\n",mInitCount);

    if(mInitCount <= 0)
    {
        // No more users
        MY_LOGD("no more user, X");
        return ret;
    }

    android_atomic_dec(&mInitCount);
    if(mInitCount > 0)
    {
        MY_LOGD("more than one user, X");
        return ret;
    }

    //pthread_cancel(m_TestEnDequethread);

    MY_LOGD("X");
    return ret;
}

/*******************************************************************************
*  Config CamIO Pipe /floria
********************************************************************************/
int
Ts_IT::
startPreview(int count)
{
    MY_LOGD("E");

    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 10) / 8;
    MINT32 ret = 0;

    u4RawBufSize = ((u4RawBufSize + 0xFFF) & ~0xFFF); //align 4k

    /***************************************************
     * Pass 1 : CamIOPipe
     ***************************************************/
    MY_LOGD("ICamIOPipe::createInstance");
    //ICamIOPipe *pCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS,//mapScenarioID(meSWScenarioID, ICamIOPipe::ePipeID)
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS, eScenarioFmt_RAW);
    if (NULL == mpCamIOPipe)
    {
        return 0;
    }
    MY_LOGD("Pipe (Name, ID) = (%s, %d)", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());

    MY_LOGD("mpCamIOPipe->init\n");
    if (!mpCamIOPipe->init())
    {
        MY_LOGD("mpCamIOPipe->init failed");
        return 0;
    }

    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    MY_LOGD("setConfigPortInfo");
    if (setConfigPortInfo() == MFALSE)
    {
        goto TEST_EXIT;
    }

    if(vRawMem.size() > 0)
    {
        freeRawMem();
        TS_Thread_UnInit();
    }

    for (int i = 0; i < BUF_NUM; i++)
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
        vRawMem.push_back(rBuf);
    }

    for (int i = 0; i < BUF_NUM; i++)
    {
#if 1
        QBufInfo rOutBufInfo;
        PortID rPortID(EPortType_Memory, EPortIndex_IMGO, 1);
        NSImageio::NSIspio::BufInfo rBuf(vRawMem.at(i).size,
             vRawMem.at(i).virtAddr,
             vRawMem.at(i).phyAddr,
             vRawMem.at(i).memID,
             vRawMem.at(i).bufSecu,
             vRawMem.at(i).bufCohe);
        rOutBufInfo.vBufInfo.push_back(rBuf);

        mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);
#else
        rRawBuf.vBufInfo.clear();
        NSCamHW::BufInfo rBufInfo(vRawMem.at(i).size, vRawMem.at(i).virtAddr, vRawMem.at(i).phyAddr, vRawMem.at(i).memID);
        rRawBuf.vBufInfo.push_back(rBufInfo);
        MY_LOGD("enqueBuf(%d)", i);
        if (MFALSE == enqueBuf(NSCamPipe::PortID(NSCamPipe::EPortType_MemoryOut, 0, 1), rRawBuf))
        {
            goto TEST_EXIT;
        }
#endif
    }

    /***************************************************
     * Pass 2 : PostProc
     ***************************************************/
    mpPostProcPipe = IPostProcPipe::createInstance(eScenarioID_VSS, eScenarioFmt_RAW);
    if (NULL == mpPostProcPipe)
    {
        MY_LOGE("mpPostProcPipe create instance fail\n");
        goto TEST_EXIT;
    }
    ret = mpPostProcPipe->init();
    if (!mpPostProcPipe->init())
    {
        MY_LOGE("mpPostProcPipe init fail\n");
        goto TEST_EXIT;
    }

    dispoBuf.size = DISPO_WIDTH*DISPO_HEIGHT*2;
    allocMem(dispoBuf);
    vidoBuf.size = VIDO_WIDTH*VIDO_HEIGHT*2;
    allocMem(vidoBuf);

    ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CQ_CHANNEL,
            (MINT32)NSImageio::NSIspio::EPIPE_PASS2_CQ1,0,0);
    ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
            (MINT32)NSImageio::NSIspio::eConfigSettingStage_Init,0,0);
    ret = mpPostProcPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CONFIG_STAGE,
            (MINT32)NSImageio::NSIspio::eConfigSettingStage_UpdateTrigger,0,0);

    start();

    TS_Thread_Init(count);

    if (!mpCamIOPipe->stop())
    {
       MY_LOGE("mpCamIOPipe->stop() fail");
    }

    //MY_LOGD("Preview stop, press to enter to exit");
    //getchar();

TEST_EXIT:

    if (NULL != mpCamIOPipe)
    {
        MY_LOGD("pCamIOPipe->uninit");
        if (MTRUE != mpCamIOPipe->uninit())
        {
            MY_LOGE("pCamIOPipe->uninit fail");
        }
        MY_LOGD("pCamIOPipe->destroyInstance");
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }

    if (NULL != mpPostProcPipe)
    {
        if (MTRUE != mpPostProcPipe->uninit())
        {
            MY_LOGE("mpPostProcPipe uninit fail");
        }
        mpPostProcPipe->destroyInstance();
        mpPostProcPipe = NULL;
    }

    if (dispoBuf.size)
    {
        char filename[256];
        sprintf(filename, "/data/IT_%dx%d_dispo.yuv", DISPO_WIDTH, DISPO_HEIGHT);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(dispoBuf.virtAddr), dispoBuf.size);
        deallocMem(dispoBuf);
    }
    if (vidoBuf.size)
    {
        char filename[256];
        sprintf(filename, "/data/IT_%dx%d_vidio.yuv", VIDO_WIDTH, VIDO_HEIGHT);
        saveBufToFile(filename, reinterpret_cast<MUINT8*>(vidoBuf.virtAddr), vidoBuf.size);
        deallocMem(vidoBuf);
    }

    MY_LOGD("X");

    return 0;
}

} //NSIspio
} //NSImageio


/*******************************************************************************
*  Main Function
********************************************************************************/

using namespace NSImageio;
using namespace NSIspio;
int main_IT_Imageio(int argc, char** argv)
{
    int ret = 0;
    int loopcount = 30;

    if (argc < 3)
    {
        MY_LOGD("Usage: imageiotest 2 <TestCase> <SensorMode> [<FrameNum>]");
        MY_LOGD("   TestCase    : TestPattern(0),Preview(1)");
        MY_LOGD("   SensorMode  : R(5),G(3),B(6),VBAR(13),HBAR(12)");
        MY_LOGD("   FrameNum    : # frames to preview(optional)");
        goto EXIT;
    }
    else
    {
        m_TestCase = atoi(argv[1]);
        pTestIT->mode = 0;//atoi(argv[2]); //sensor mode, 0: preview, 1: capture(full)
    }
    #if FAKE_SENSOR == 1
    pTestIT->mSensorCallerName = argv[2];
    #endif

    if (argc == 4)
    {
        if (atoi(argv[3]) == 0)
        {
            m_CurDev = SENSOR_DEV_MAIN;
        }
        else
        {
            m_CurDev = SENSOR_DEV_SUB;
        }
    }

    MY_LOGD("TestCase   :%d", m_TestCase);
    MY_LOGD("SensorMode :%d", pTestIT->mode);
    MY_LOGD("FrameNum   :%d", loopcount);

    pTestIT->mpIMemDrv = IMemDrv::createInstance();
    if (NULL == pTestIT->mpIMemDrv)
    {
        MY_LOGE("IMemDrv create fail");
        return 0;
    }

    if (!pTestIT->mpIMemDrv->init())
    {
        MY_LOGE("IMemDrv init fail");
        return 0;
    }

    ret = pTestIT->sensorInit();
    if (ret < 0)
    {
        MY_LOGE("Sensor init fail");
        goto EXIT;
    }

    pTestIT->startPreview(loopcount);

    pTestIT->sensorUninit();

    pTestIT->TS_Thread_UnInit();

    pTestIT->freeRawMem();

    MY_LOGD("IMemDrv destroyInstance()");
    pTestIT->mpIMemDrv->destroyInstance();

EXIT:

    return ret;
}

#endif
