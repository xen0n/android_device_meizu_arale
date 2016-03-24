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
#include "ts_camio.h"

#define BUF_NUM          3


static int m_TestCase = 0;  //0: test pattern, 1: preview
static MUINT32 m_CurDev = SENSOR_DEV_MAIN;

namespace NSImageio {
namespace NSIspio{

Ts_UT TestUT;
Ts_UT *pTestUT = &TestUT;


/******************************************************************************
* Memory
*******************************************************************************/

static void
allocMem(IMEM_BUF_INFO &memBuf)
{
    MY_LOGD("size(%u)", memBuf.size );
    if (pTestUT->mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (pTestUT->mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error");
    }
}

static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (pTestUT->mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
    }

    if (pTestUT->mpIMemDrv->freeVirtBuf(&memBuf)) {
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
Ts_UT::
Ts_UT()
    : mSensorDev(0)
    , mpSensorHal(NULL)
    , mSensorCallerName("TS_UT")
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

Ts_UT::
~Ts_UT()
{

}

/*******************************************************************************
*  Sensor init
********************************************************************************/
int Ts_UT::main_SetSensor_init()
{
#if FAKE_SENSOR == 1
    TS_FakeSensorList *pHalSensorList = 0;
#else
    IHalSensorList *pHalSensorList = 0;
#endif
    MUINT32 sensorNum = 0;
    MINT32 supportedSensorDev = 0;
    MINT32 ret;

    MY_LOGD("E\n");

    mSensorDev = m_CurDev;

    //======================================================
    // search sensor
    //======================================================

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
        MY_LOGD("mpSensorHal is NULL");
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
int Ts_UT::main_SetSensor_uninit()
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
Ts_UT::
setConfigPortInfo()
{
    NSCam::EImageFormat eSensorFmt = NSCam::eImgFmt_BAYER10;
    MUINT32 u4SensorStride = 0;
    MBOOL ret = MTRUE;

    MY_LOGD("E");

    u4SensorStride = ((u4SensorWidth + 7) & ~7); //align8 for RAW10

    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    MY_LOGD("command queue config");
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
        #if 0//dbg only
        imgo.u4ImgHeight = imgo.u4ImgHeight/2;
        imgo.crop.y = 700;
        imgo.crop.h = imgo.u4ImgHeight/2;
        #endif
        imgo.type = EPortType_Memory;
        imgo.index = EPortIndex_IMGO;
        imgo.inout  = EPortDirection_Out;
        imgo.u4Stride[0] = u4SensorStride;
        imgo.u4Stride[1] = 0;
        imgo.u4Stride[2] = 0;
        imgo.u4Offset = 0;
        vCamIOOutPorts.push_back(&imgo);
    }
    #if 0
    {
        img2o.eImgFmt = tgi.eImgFmt;
        img2o.u4ImgWidth = 1600;//tgi.u4ImgWidth;
        img2o.u4ImgHeight = 1200;//tgi.u4ImgHeight;
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
    #endif

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
Ts_UT::
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

        mpCamIOPipe->enqueOutBuf(rPortID, rQTimeOutBufInfo);

        if (mfgIsYUVPortON)
        {
            rPortID.index =  EPortIndex_IMG2O;
            ret = mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);

            if (!ret)
            {
                MY_LOGE("mpCamIOPipe->dequeOutBuf(EPortIndex_IMG2O) fail ");
                return ret;
            }

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
Ts_UT::
start()
{
    MBOOL ret = MTRUE;

    MY_LOGD("E");
    //
    // (1) set buffer to current buffer
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
    //
    MY_LOGD("mpCamIOPipe->start");
    ret = mpCamIOPipe->start();

    // (3) sync vync, 82 has only TG1
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
Ts_UT::
freeRawMem()
{
    if(vRawMem.size() > 0)
    {
        MY_LOGD("[vRawMem] deallocMem\n");
        for (int i = 0; i < BUF_NUM; i++)
        {
            char filename[256];
            sprintf(filename, "/data/raw%dx%d_%02d.raw", u4SensorWidth, u4SensorHeight, i);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(vRawMem.at(i).virtAddr), (u4SensorWidth * u4SensorHeight * 10) / 8);
            deallocMem(vRawMem.at(i));
        }
    }
}
#ifdef _USE_THREAD_QUE_
/*******************************************************************************
*
********************************************************************************/
MVOID*
Ts_UT::
endeque_Thread(MVOID * arg)
{
    MY_LOGD("E");

    int count = (int)((MUINTPTR)arg), i = 0; //enque, deque loop iterations

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    do
    {
        QTimeStampBufInfo rQTimeOutBufInfo, rQTimeOutBufInfoYuv;
        PortID rPortID(EPortType_Memory, EPortIndex_IMGO, 1);

        MY_LOGD("-----------------------------------");
        MY_LOGD("dequeBuf count(%d)", count);

        pTestUT->mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
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
        if (pTestUT->mfgIsYUVPortON)
        {
            MY_LOGD(" mpCamIOPipe->dequeOutBuf(YUV)\n");
            rPortID.index = EPortIndex_IMG2O;
            pTestUT->mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfoYuv);
            if (rQTimeOutBufInfoYuv.vBufInfo.size() == 0)
            {
                MY_LOGE("Deque img2o buf no buf");
                goto EXIT_DEQ;
            }
        }

        MY_LOGD("enqueBuf count(%d)", count);
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
        pTestUT->mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);

        if (pTestUT->mfgIsYUVPortON)
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

            pTestUT->mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo);
        }
    } while(--count > 0);

EXIT_DEQ:
    MY_LOGD("X");
    pthread_exit(0);

    return NULL;
}
/*******************************************************************************
*
********************************************************************************/
MVOID
Ts_UT::
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
        ::sem_init(&m_semTestEnDequethread, 0, 0);

        // Create main thread for preview and capture
        pthread_create(&m_TestEnDequethread, NULL, endeque_Thread, (void *)((MUINTPTR)count));
        android_atomic_inc(&mInitCount);
        pthread_join(m_TestEnDequethread, NULL);
    }
    MY_LOGD("X");
}

MBOOL
Ts_UT::
TS_Thread_UnInit()
{
    MBOOL ret = MTRUE;
    MY_LOGD("E");

    Mutex::Autolock lock(mLock);
    MY_LOGD("mInitCount(%d)\n",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        MY_LOGD("no more user, X");
        return ret;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        MY_LOGD("more than one user, X");
        return ret;
    }

//    pthread_cancel(m_TestEnDequethread);

    MY_LOGD("X");
    return ret;
}
#endif // _USE_THREAD_QUE_
/*******************************************************************************
*  Config CamIO Pipe /floria
********************************************************************************/
int
Ts_UT::
main_ts_CamIO_ZSD(int count)
{
    MY_LOGD("E");

    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 10) / 8;

    u4RawBufSize = ((u4RawBufSize + 0xFFF) & ~0xFFF); //align 4k

    MY_LOGD("ICamIOPipe::createInstance");
//    ICamIOPipe *pCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS,//mapScenarioID(meSWScenarioID, ICamIOPipe::ePipeID)
    mpCamIOPipe = ICamIOPipe::createInstance(eScenarioID_VSS,//mapScenarioID(meSWScenarioID, ICamIOPipe::ePipeID)
                                             eScenarioFmt_RAW);//mapScenarioFmt(meScenarioFmt)
    if (mpCamIOPipe != NULL)
    {
        MY_LOGD("Pipe (Name, ID) = (%s, %d)", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
    }

    MY_LOGD("mpCamIOPipe->init\n");
    if (!mpCamIOPipe->init())
    {
        MY_LOGD("mpCamIOPipe->init failed");
    }

    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    MY_LOGD("setConfigPortInfo");
    if(setConfigPortInfo() == MFALSE)
    {
        goto TEST_EXIT;
    }

    #ifdef _USE_THREAD_QUE_
    if(vRawMem.size() > 0)
    {
        freeRawMem();
        TS_Thread_UnInit();
    }
    #endif

    for (int i = 0; i < BUF_NUM; i++)
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
        vRawMem.push_back(rBuf);
    }

    for (int i = 0; i < BUF_NUM; i++)
    {
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
    }

    MY_LOGD("start");
    start();

    #ifdef _USE_THREAD_QUE_
    TS_Thread_Init(count);
    #endif//_USE_THREAD_QUE_

    MY_LOGD("pCamIOPipe->stop");
    if (!mpCamIOPipe->stop())
    {
       MY_LOGE("mpCamIOPipe->stop() fail");
    }

TEST_EXIT:

    /*------------------------------
    *    (10). uninit
    *-----------------------------*/
    //
    if (NULL != mpCamIOPipe)
    {
        MY_LOGD("pCamIOPipe->uninit");
        if (MTRUE != mpCamIOPipe->uninit())
        {
            MY_LOGD("pCamIOPipe->uninit fail");
            MY_LOGE("pCamIOPipe->uninit() fail");
        }
        MY_LOGD("pCamIOPipe->destroyInstance");
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
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
int main_Camio(int argc, char** argv)
{
    int ret = 0;
    int loopcount = 30;

    if (argc < 3)
    {
        MY_LOGD("Usage: imageiotest 0 <TestCase> <SensorMode> [<FrameNum>]");
        MY_LOGD("   TestCase    : TestPattern(0),Preview(1)");
        MY_LOGD("   SensorMode  : R(5),G(3),B(6),VBAR(13),HBAR(12)");
        MY_LOGD("   FrameNum    : # frames to preview(optional)");
        goto EXIT;
    }
    else
    {
        m_TestCase = atoi(argv[1]);
        pTestUT->mode = 0;//atoi(argv[2]); //sensor mode, 0: preview, 1: capture(full)
    }
    #if FAKE_SENSOR == 1
    pTestUT->mSensorCallerName = argv[2];
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
    MY_LOGD("SensorMode :%d", pTestUT->mode);
    MY_LOGD("FrameNum   :%d", loopcount);

    MY_LOGD("IMemDrv createInstance");
    pTestUT->mpIMemDrv = IMemDrv::createInstance();
    if (NULL == pTestUT->mpIMemDrv)
    {
        MY_LOGE("IMemDrv is NULL");
        return 0;
    }
    MY_LOGD("IMemDrv init");
    if ( !pTestUT->mpIMemDrv->init() )
    {
        MY_LOGE("IMemDrv init fail");
    }

    ret = pTestUT->main_SetSensor_init();
    if(ret < 0)
        goto EXIT;

    pTestUT->main_ts_CamIO_ZSD(loopcount);

    pTestUT->main_SetSensor_uninit();

    //MY_LOGD("Preview stop, press to enter to exit");
    //getchar();

    #ifdef _USE_THREAD_QUE_
    pTestUT->TS_Thread_UnInit();
    #endif

    MY_LOGD("freeRawMem");
    pTestUT->freeRawMem();

    MY_LOGD("pIMemDrv destroyInstance()");
    pTestUT->mpIMemDrv->destroyInstance();

EXIT:

    return ret;
}




