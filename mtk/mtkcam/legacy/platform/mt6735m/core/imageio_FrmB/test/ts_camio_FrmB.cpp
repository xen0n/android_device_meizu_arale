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
#include "ts_camio_FrmB.h"

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
        MY_LOGE("[camioFt] g_pIMemDrv->allocVirtBuf() error");
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (pTestUT->mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("[camioFt] mpIMemDrv->mapPhyAddr() error");
    }
}

static void deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (pTestUT->mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("[camioFt] m_pIMemDrv->unmapPhyAddr() error");
    }

    if (pTestUT->mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("[camioFt] m_pIMemDrv->freeVirtBuf() error");
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

    MY_LOGD("[camioFt] (name, buf, size) = (%s, %x, %d)", fname, buf, size);
    MY_LOGD("[camioFt] opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        MY_LOGE("[camioFt] failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    MY_LOGD("[camioFt] writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            MY_LOGE("[camioFt] failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    MY_LOGD("[camioFt] done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}


namespace NSImageio_FrmB {
namespace NSIspio_FrmB{

/*******************************************************************************
*
********************************************************************************/
Ts_UT::
Ts_UT()
    : mpSensorHal(NULL)
    , mu4DeviceID(0)
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
    //SENSOR_DEV_SUB : ENSOR_DEV_MAIN
    sensorDevId = is_yuv_sensor ? static_cast<halSensorDev_e>(2) : static_cast<halSensorDev_e>(1);
}

Ts_UT::
~Ts_UT()
{

}


/*******************************************************************************
*
********************************************************************************/
ERawPxlID
Ts_UT::
mapRawPixelID(MUINT32 const u4PixelID)
{
    switch(u4PixelID)
    {
        case 0:
            return ERawPxlID_B;
        break;
        case 1:
            return ERawPxlID_Gb;
        break;
        case 2:
            return ERawPxlID_Gr;
        break;
        case 3:
            return ERawPxlID_R;
        break;
        default:
            return ERawPxlID_Gb;
        break;
    }
}

/******************************************************************************
*
*******************************************************************************/
void
Ts_UT::mapBufInfo(NSCamHW::BufInfo &rCamPipeBufInfo, BufInfo const &rBufInfo)
{
    rCamPipeBufInfo.u4BufSize = rBufInfo.u4BufSize[0];
    rCamPipeBufInfo.u4BufVA = rBufInfo.u4BufVA[0];
    rCamPipeBufInfo.u4BufPA = rBufInfo.u4BufPA[0];
    rCamPipeBufInfo.i4MemID = rBufInfo.memID[0];
}

/******************************************************************************
* ConfigPipe & ConfigSensor
*******************************************************************************/
MBOOL
Ts_UT::
setConfigPortInfo(MINT32 nOutPort)
{
    MY_LOGD("+");
    printf("[setConfigPortInfo] E\n");
//    NSCamPipe::SensorPortInfo rSensorPort(SENSOR_DEV_MAIN, ACDK_SCENARIO_ID_CAMERA_PREVIEW, 10, MTRUE, MFALSE, 0);
    NSCamPipe::SensorPortInfo rSensorPort(static_cast<halSensorDev_e>(1), ACDK_SCENARIO_ID_CAMERA_PREVIEW, 10, MTRUE, MFALSE, 0);

    MUINT32 u4RawStride[3] = {u4SensorWidth*10/8, 0, 0};
    MUINT32 u4Raw2Stride[3] = {u4SensorWidth*10/8/2, 0, 0};

    NSCamPipe::MemoryOutPortInfo rRawPort(NSCamHW::ImgInfo(NSCam::eImgFmt_BAYER10, u4SensorWidth, u4SensorHeight),
                               u4RawStride, 0, 0);
    //
    vector<NSCamPipe::PortID const*> vInPorts;
    vector<NSCamPipe::PortID const*> vOutPorts;

    NSCamPipe::PortID const* InSenPort= reinterpret_cast<NSCamPipe::PortID const*> (&rSensorPort);
    NSCamPipe::PortID const* OutRawPort= reinterpret_cast<NSCamPipe::PortID const*> (&rRawPort);
    //
    printf("[setConfigPortInfo] push_backports\n");
    vInPorts.push_back(InSenPort);
    vOutPorts.push_back(OutRawPort);

    if(nOutPort > 1)
    {
        printf("nOutPort>1\n");
        NSCamPipe::MemoryOutPortInfo rRaw2Port(NSCamHW::ImgInfo(NSCam::eImgFmt_BAYER10, u4SensorWidth, u4SensorHeight),
                               u4Raw2Stride, 0, 0);
        NSCamPipe::PortID const* OutRaw2Port= reinterpret_cast<NSCamPipe::PortID const*> (&rRaw2Port);
        vOutPorts.push_back(OutRaw2Port);
    }

    //-----------------------------------------------------------

    MBOOL ret = MTRUE;

    if (0 == vInPorts.size()
    || 0 == vOutPorts.size()
    || vOutPorts.size() > 2)
    {
        printf("[setConfigPortInfo] Port config error\n");
        MY_LOGE("[camioFt] Port config error");
        return MFALSE;
    }
    //
    if (NSCamPipe::EPortType_Sensor != vInPorts.at(0)->type)
    {
        printf("[setConfigPortInfo] The IN port type should be sensor type\n");
        MY_LOGE("[camioFt] The IN port type should be sensor type");
        return MFALSE;
    }
    //
    for (MUINT32 i = 0; i < vOutPorts.size(); i++)
    {
        if (NSCamPipe::EPortType_MemoryOut != vOutPorts.at(i)->type)
        {
            printf("[setConfigPortInfo] The OUT port type should be EPortType_MemoryOut\n");
            MY_LOGE("[camioFt] The OUT port type should be EPortType_MemoryOut");
            return MFALSE;
        }
    }

    // (1). callbacks
    printf("[setConfigPortInfo] callback\n");
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    // (2). command queue config
    printf("[setConfigPortInfo] command queue config\n");
    MY_LOGD("[camioFt] SET_CQ_CHANNEL");
    printf("[setConfigPortInfo] \tSET_CQ_CHANNEL\n");
    ret = ret && mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,
                                  (MINT32)EPIPE_PASS1_CQ0,
                                   0,
                                   0
                                   );

    MY_LOGD("[camioFt]SET_CQ_TRIGGER_MODE");///////////cause error
    printf("[setConfigPortInfo] \tSET_CQ_TRIGGER_MODE\n");
    ret = ret && mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)EPIPE_PASS1_CQ0,
                                  (MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,
                                  (MINT32)EPIPECQ_TRIG_BY_START
                                  );
    if (!ret)
    {
        printf("[setConfigPortInfo] Cammand queue config fail:%d\n", mpCamIOPipe->getLastErrorCode());
        MY_LOGE("[camioFt] Cammand queue config fail:%d", mpCamIOPipe->getLastErrorCode());
        return ret;
    }
    //
    // (3). In sensor port
    MY_LOGD("[camioFt] In sensor port");
    printf("[setConfigPortInfo] In sensor port\n");
    vector<PortInfo const*> vCamIOInPorts;
    NSCamPipe::SensorPortInfo const* const pSensorPort = reinterpret_cast<NSCamPipe::SensorPortInfo const*> (vInPorts.at(0));
    ::memcpy(&mrSensorPortInfo, const_cast<NSCamPipe::SensorPortInfo*>(pSensorPort),sizeof(NSCamPipe::SensorPortInfo));
    MUINT32 u4RawPixelID = 0;
    NSCam::EImageFormat eSensorFmt = eImgFmt_BAYER10;

    // (3.1) Sensor instance
    if (NULL == mpSensorHal)
    {
        mpSensorHal = SensorHal::createInstance();
        if (NULL == mpSensorHal)
        {
            printf("[setConfigPortInfo] Null sensorHal object\n");
            MY_LOGE("[camioFt] Null sensorHal object");
            return MFALSE;
        }
    }
    //
    mpSensorHal->sendCommand(static_cast<halSensorDev_e>(mrSensorPortInfo.u4DeviceID),
                                 SENSOR_CMD_SET_SENSOR_DEV,
                                 0,
                                 0,
                                 0
                                 );
    //
    //mpSensorHal->init();

    printf("[setConfigPortInfo] call querySensorInfo()\n");
    ret = querySensorInfo( mrSensorPortInfo.u4DeviceID,
                           mrSensorPortInfo.u4Scenario,
                           mrSensorPortInfo.u4Bitdepth,
                           eSensorFmt,
                           u4SensorWidth,
                           u4SensorHeight,
                           u4RawPixelID);
    if(ret == MFALSE)
    {
        printf("[setConfigPortInfo] querySensorInfo fail\n");
        return ret;
    }

    printf("[setConfigPortInfo] Sensor: (devID,scen)(%d,%d) (w,h,fmt,bits,pixID)(%d,%d,0x%x,%d,%d) (bpDelay,bpScen,rawType)(%d,%d,%d)\n",
                       mrSensorPortInfo.u4DeviceID,
                       mrSensorPortInfo.u4Scenario,
                       u4SensorWidth,
                       u4SensorHeight,
                       eSensorFmt,
                       mrSensorPortInfo.u4Bitdepth, u4RawPixelID,
                       mrSensorPortInfo.fgBypassDelay,
                       mrSensorPortInfo.fgBypassScenaio,
                       mrSensorPortInfo.u4RawType);

    MY_LOGD("[camioFt] Sensor: (devID,scen)(%d,%d) (w,h,fmt,bits,pixID)(%d,%d,0x%x,%d,%d) (bpDelay,bpScen,rawType)(%d,%d,%d)",
                       mrSensorPortInfo.u4DeviceID,
                       mrSensorPortInfo.u4Scenario,
                       u4SensorWidth,
                       u4SensorHeight,
                       eSensorFmt,
                       mrSensorPortInfo.u4Bitdepth, u4RawPixelID,
                       mrSensorPortInfo.fgBypassDelay,
                       mrSensorPortInfo.fgBypassScenaio,
                       mrSensorPortInfo.u4RawType);
    //
    MUINT32 u4SensorStride = u4SensorWidth;
    if (eImgFmt_BAYER8 == eSensorFmt || eImgFmt_BAYER10 == eSensorFmt || eImgFmt_BAYER12 == eSensorFmt)
    {
        printf("[setConfigPortInfo] call queryRawStride()\n");
        u4SensorStride = queryRawStride(eSensorFmt, u4SensorWidth);
    }

    //MY_LOGD("SensorPortInfo: (width, height, format, stride) = (%d, %d, 0x%x, %d, %d, %d)",
    //                 u4SensorWidth, u4SensorHeight, eSensorFmt, u4SensorStride);

    printf("[setConfigPortInfo] config tgi\n");
    PortInfo tgi;
    tgi.eImgFmt = eSensorFmt;
    tgi.eRawPxlID = mapRawPixelID(u4RawPixelID);
    tgi.u4ImgWidth = u4SensorWidth;
    tgi.u4ImgHeight = u4SensorHeight;
    tgi.u4Stride[0] = u4SensorStride;
    tgi.u4Stride[1] = 0;
    tgi.u4Stride[2] = 0;
    tgi.type = EPortType_Sensor;
    mu4DeviceID = pSensorPort->u4DeviceID;
    tgi.index = EPortIndex_TG1I;
    tgi.inout  = EPortDirection_In;
    tgi.u4BufSize[0]  = (MUINT32)0;
    vCamIOInPorts.push_back(&tgi);

    // The raw type, 0: pure raw, 1: pre-process raw
//    if(pSensorPort->u4RawType == 1)
//    {
//        ret = mpCamIOPipe->sendCommand(NSImageio::NSIspio::EPIPECmd_SET_IMGO_RAW_TYPE,
//                                      (MINT32)NSImageio::NSIspio::eRawImageType_PreProc,
//                                       0,
//                                       0
//                                      );
//    }
    // (4). Out Port
    printf("[setConfigPortInfo] config imgo\n");
    vector<PortInfo const*> vCamIOOutPorts;
    PortInfo imgo;
    PortInfo img2o;
    for (MUINT32 i = 0; i < vOutPorts.size(); i++)
    {
        NSCamPipe::MemoryOutPortInfo const* const memOutPort= reinterpret_cast<NSCamPipe::MemoryOutPortInfo const*> (vOutPorts.at(i));
        //
        //if (0 == memOutPort->index)
        if (0 == i)
        {
            imgo.eImgFmt = tgi.eImgFmt;
            imgo.u4ImgWidth = tgi.u4ImgWidth;
            imgo.u4ImgHeight = tgi.u4ImgHeight;
            // no crop
            imgo.crop1.x = 0;
            imgo.crop1.y = 0;
            imgo.crop1.w = imgo.u4ImgWidth;
            imgo.crop1.h = imgo.u4ImgHeight;

            imgo.type = EPortType_Memory;
            imgo.index = EPortIndex_IMGO;
            imgo.inout  = EPortDirection_Out;
            imgo.u4Stride[0] = memOutPort->u4Stride[0];
            imgo.u4Stride[1] = memOutPort->u4Stride[1];
            imgo.u4Stride[2] = memOutPort->u4Stride[2];
            imgo.u4Offset = 0;
            vCamIOOutPorts.push_back(&imgo);
            MY_LOGD("[camioFt] Out 0: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d)",
                imgo.eImgFmt, imgo.u4ImgWidth, imgo.u4ImgHeight,
                imgo.u4Stride[0],  imgo.u4Stride[1],  imgo.u4Stride[2]);
        }
        //else if (1 == memOutPort->index)
        else if (1 == i)
        {
            img2o.eImgFmt = tgi.eImgFmt;
            img2o.u4ImgWidth = tgi.u4ImgWidth/2;//0.5
            img2o.u4ImgHeight = tgi.u4ImgHeight;

            img2o.crop1.x = 0;
            img2o.crop1.y = 0;
            img2o.crop1.w = img2o.u4ImgWidth;
            img2o.crop1.h = img2o.u4ImgHeight;

            img2o.type = EPortType_Memory;
            img2o.index = EPortIndex_IMG2O;
            img2o.inout  = EPortDirection_Out;
            img2o.u4Stride[0] = u4Raw2Stride[0];
            img2o.u4Stride[1] = u4Raw2Stride[1];
            img2o.u4Stride[2] = u4Raw2Stride[2];
            vCamIOOutPorts.push_back(&img2o);
            mfgIsYUVPortON = MFALSE;
            MY_LOGD("[camioFt] Out 1: (fmt,w,h)(0x%x,%d,%d) stride(%d,%d,%d)",
                    img2o.eImgFmt,  img2o.u4ImgWidth, img2o.u4ImgHeight,
                    img2o.u4Stride[0],  img2o.u4Stride[1],  img2o.u4Stride[2]);
        }
    }

    printf("[setConfigPortInfo] call mpCamIOPipe->configPipe\n");
    ret = mpCamIOPipe->configPipe(vCamIOInPorts, vCamIOOutPorts);

    if(ret == MFALSE)
    {
        printf("[setConfigPortInfo] configPipe fail\n");
        return ret;
    }
    printf("[setConfigPortInfo] configPipe(ret=%d)\n", ret);
    //
    ret = configSensor(pSensorPort->u4DeviceID,
                       pSensorPort->u4Scenario,
                       u4SensorWidth, u4SensorHeight,
                       pSensorPort->fgBypassDelay,
                       pSensorPort->fgBypassScenaio, MTRUE);
    if(ret == MFALSE)
    {
        printf("[setConfigPortInfo] configSensor fail\n");
        return ret;
    }
    printf("[setConfigPortInfo] configSensor(ret=%d)\n", ret);

    // The raw type, 0: pure raw, 1: pre-process raw  2: sensor test pattern
//    if(pSensorPort->u4RawType == 2)
    {
        MINT32 u32Enable = 1;
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(pSensorPort->u4DeviceID),
                                           static_cast<int>(NSCam::SENSOR_CMD_SET_TEST_PATTERN_OUTPUT),//0x1010)
                                           (MINT32)&u32Enable,
                                           0,
                                           0);
        MY_LOGD("[camioFt] Sensor Test Pattern");
    }

    // query skip frame to wait for stable
    mu4SkipFrame = 0;
    if (mrSensorPortInfo.fgBypassDelay == MFALSE)
    {
        MUINT32 u4Mode = 2;//SENSOR_CAPTURE_DELAY;
        //
        mpSensorHal->sendCommand(static_cast<halSensorDev_e>(mrSensorPortInfo.u4DeviceID),
                                              static_cast<int>(0x2013), //SENSOR_CMD_GET_UNSTABLE_DELAY_FRAME_CNT
                                              reinterpret_cast<int>(&mu4SkipFrame),
                                              reinterpret_cast<int>(&u4Mode));
    }

    printf("[setConfigPortInfo] X\n");
    MY_LOGD("-");
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_UT::
skipFrame(MUINT32 const u4SkipCount)
{
    MY_LOGD("[camioFt] + (u4SkipCount) = (%d)", u4SkipCount);
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
            MY_LOGE("[camioFt] mpCamIOPipe->dequeOutBuf(EPortIndex_IMGO) fail ");
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
                MY_LOGE("[camioFt] mpCamIOPipe->dequeOutBuf(EPortIndex_IMG2O) fail ");
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
Ts_UT::
start()
{
    MBOOL ret = MTRUE;

    printf("[start] E\n");
    //
    // (1) set buffer to current buffer
//    ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
//                                   (MINT32)EPortIndex_IMGO,
//                                    0,
//                                    0
//                                  );
//
//    if (mfgIsYUVPortON)
//    {
//       ret = mpCamIOPipe->sendCommand((MINT32)NSImageio::NSIspio::EPIPECmd_SET_CURRENT_BUFFER,
//                                      (MINT32)EPortIndex_IMG2O,
//                                       0,
//                                       0
//                                      );
//    }

#if 0//FLORIA disable, already in mpCamIOPipe->start
    //
    // (2) start CQ
    printf("[start] mpCamIOPipe->startCQ0\n");
    ret = mpCamIOPipe->startCQ0();
    //
    // ! let commond queue trigger mode as continuous mode
    printf("[start] SET_CQ_TRIGGER_MODE\n");
    ret = mpCamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                                  (MINT32)EPIPE_PASS1_CQ0,
                                  (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                                  (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
#endif
    // (3) sync vync, 82 has only TG1
    printf("[start] mpCamIOPipe->irq(VSYNC)\n");
    ret = mpCamIOPipe->irq(EPipePass_PASS1_TG1,
                           EPIPEIRQ_VSYNC,
                           (int)ISP_DRV_IRQ_USER_ISPDRV);
    //
    printf("[start] mpCamIOPipe->start\n");
    ret = mpCamIOPipe->start();

    // (3) sync vync
//    printf("[start] mpCamIOPipe->irq(VSYNC)\n");
//    ret = mpCamIOPipe->irq(EPipePass_PASS1_TG1,
//                           EPIPEIRQ_VSYNC,
//                           (int)ISP_DRV_IRQ_USER_ISPDRV);
//    printf("[start] Wait for stable frame:%d\n", mu4SkipFrame);
//    MY_LOGD(" Wait for stable frame:%d", mu4SkipFrame);
//    skipFrame(mu4SkipFrame);

    return  ret;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_UT::
enqueBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QBufInfo const& rQBufInfo)
{
    //FUNCTION_LOG_START;
    //MY_LOGD("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++)
    {
        MY_LOGD("+ tid(%d) type(%d,%d,%d), (VA,PA,Size,ID)(%x, %x, %d, %d)",
            gettid(),
            ePortID.type, ePortID.index, ePortID.inout,
            rQBufInfo.vBufInfo.at(i).u4BufVA, rQBufInfo.vBufInfo.at(i).u4BufPA,
            rQBufInfo.vBufInfo.at(i).u4BufSize, rQBufInfo.vBufInfo.at(i).i4MemID);
    }

    //
    if (NSCamPipe::EPortType_MemoryOut != ePortID.type)
    {
        MY_LOGE("[camioFt] enqueBuf only support memory out port type");
        return MFALSE;
    }

    // Note:: can't update config, but address
    //
    QBufInfo rOutBufInfo;
    PortID rPortID(EPortType_Memory,
                   EPortIndex_IMGO,
                   1);

    //
    if (1 == ePortID.index)      //yuv out buf
    {
        rPortID.index = EPortIndex_IMG2O;
    }

    //
    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++)
    {
/*           BufInfo rBuf;
          rBuf.u4BufSize[0]    = rQBufInfo.vBufInfo.at(i).u4BufSize;
        rBuf.u4BufVA[0]        = rQBufInfo.vBufInfo.at(i).u4BufVA;
        rBuf.u4BufPA[0]        = rQBufInfo.vBufInfo.at(i).u4BufPA;
        rBuf.memID[0]        = rQBufInfo.vBufInfo.at(i).i4MemID;
        rBuf.bufSecu[0]        = rQBufInfo.vBufInfo.at(i).i4BufSecu;
        rBuf.bufCohe[0]        = rQBufInfo.vBufInfo.at(i).i4BufCohe;
        rOutBufInfo.vBufInfo.push_back(rBuf);
*/
        rOutBufInfo.vBufInfo.resize(1);
        rOutBufInfo.vBufInfo[0].u4BufSize[0]= (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufSize;
        rOutBufInfo.vBufInfo[0].u4BufVA[0]     = (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufVA;
        rOutBufInfo.vBufInfo[0].u4BufPA[0]    = (MUINT32)rQBufInfo.vBufInfo.at(i).u4BufPA;
        rOutBufInfo.vBufInfo[0].memID[0]    = rQBufInfo.vBufInfo.at(i).i4MemID;
        rOutBufInfo.vBufInfo[0].bufSecu[0]    = rQBufInfo.vBufInfo.at(i).i4BufSecu;
        rOutBufInfo.vBufInfo[0].bufCohe[0]    = rQBufInfo.vBufInfo.at(i).i4BufCohe;
        rOutBufInfo.vBufInfo[0].mBufIdx     = (MINT32)0xFFFF;//no replace
    }
    //
    if(!mpCamIOPipe->enqueOutBuf(rPortID, rOutBufInfo))
    {
        return  MFALSE;
    }
    //FUNCTION_LOG_END;
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_UT::
dequeHWBuf(MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    //FUNCTION_LOG_START;
    printf("[dequeHWBuf] E\n");
    MY_LOGD("+ tid(%d) tout)=(%d)", gettid(), u4TimeoutMs);
    //
    QTimeStampBufInfo rQTimeOutBufInfo;
    PortID rPortID(EPortType_Memory,
                   EPortIndex_IMGO,
                   1);

    //Jackie, wait deque directly
    // (1). wait interrupt done
    //mpCamIOPipe->irq(NSImageio::NSIspio::EPipePass_PASS1_TG1,NSImageio::NSIspio::EPIPEIRQ_PATH_DONE);

    // (2.1). dequeue buffer , Raw
    printf("[dequeHWBuf] mpCamIOPipe->dequeOutBuf(RAW)\n");
    if(!mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo))
    {
        printf("[dequeHWBuf] mpCamIOPipe->dequeOutBuf(RAW) failed\n");
        return MFALSE;
    }

    // (2.2). put buffer Raw in queue
    printf("[dequeHWBuf] put buffer in queue(RAW)\n");
    if(rQTimeOutBufInfo.vBufInfo.size()> 0)
    {
        mrRawQTBufInfo.u4User            = rQTimeOutBufInfo.u4User;
        mrRawQTBufInfo.u4Reserved        = rQTimeOutBufInfo.u4Reserved;
        mrRawQTBufInfo.i4TimeStamp_sec    = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_sec;
        mrRawQTBufInfo.i4TimeStamp_us    = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_us;
    }

    for (MUINT32 i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++)
    {
        NSCamHW::BufInfo rBufInfo;
        mapBufInfo(rBufInfo, rQTimeOutBufInfo.vBufInfo.at(i));

        mrRawQTBufInfo.vBufInfo.push_back(rBufInfo);
    }

    if (mfgIsYUVPortON)
    {
        // (3.1). dequeue buffer , Raw
        printf("[dequeHWBuf] mpCamIOPipe->dequeOutBuf(YUV)\n");
        rPortID.index =  EPortIndex_IMG2O;
        mpCamIOPipe->dequeOutBuf(rPortID, rQTimeOutBufInfo);
        // (3.2). put buffer yuv in queue
        printf("[dequeHWBuf] put buffer in queue(YUV)\n");
        if(rQTimeOutBufInfo.vBufInfo.size()>0)
        {
            mrYuvQTBufInfo.u4User = rQTimeOutBufInfo.u4User;
            mrYuvQTBufInfo.u4Reserved = rQTimeOutBufInfo.u4Reserved;
            mrYuvQTBufInfo.i4TimeStamp_sec = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_sec;
            mrYuvQTBufInfo.i4TimeStamp_us = rQTimeOutBufInfo.vBufInfo.at(0).i4TimeStamp_us;
        }
        for (MUINT32 i = 0; i < rQTimeOutBufInfo.vBufInfo.size(); i++)
        {
            NSCamHW::BufInfo rBufInfo;
            mapBufInfo(rBufInfo, rQTimeOutBufInfo.vBufInfo.at(i));
            mrYuvQTBufInfo.vBufInfo.push_back(rBufInfo);
        }
    }
    printf("[dequeHWBuf] X\n");
    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_UT::
dequeBuf(NSCamPipe::PortID const ePortID, NSCamPipe::QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    printf("[dequeBuf] E, tid(%d) type(%d, %d, %d, %d)\n", gettid(), ePortID.type, ePortID.index, ePortID.inout, u4TimeoutMs);
    MY_LOGD("[camioFt] + tid(%d) type(%d, %d, %d, %d)", gettid(), ePortID.type, ePortID.index, ePortID.inout, u4TimeoutMs);
    //
    // (1) check if the buffer already dequeue
    // (2) if the buffer is not dequeue, dequeue from HW.
    NSCamPipe::QTimeStampBufInfo *pQTBufInfo = NULL;

    if (0 == ePortID.index)
    {
        pQTBufInfo = &mrRawQTBufInfo;
    }
    else
    {
        printf("[dequeBuf] The YUV por is not on\n");
        if (!mfgIsYUVPortON)
        {
            MY_LOGE("[camioFt]The YUV por is not on \n");
            return MFALSE;
        }
        pQTBufInfo = &mrYuvQTBufInfo;
    }

    if (pQTBufInfo->vBufInfo.size() == 0)
    {
        printf("[dequeBuf] call dequeHWBuf\n");
        if(!dequeHWBuf(u4TimeoutMs))
            return  MFALSE;
    }
    //
    rQBufInfo.u4User = pQTBufInfo->u4User;
    rQBufInfo.u4Reserved = pQTBufInfo->u4Reserved;
    rQBufInfo.i4TimeStamp_sec = pQTBufInfo->i4TimeStamp_sec;
    rQBufInfo.i4TimeStamp_us = pQTBufInfo->i4TimeStamp_us;

    for (MUINT32 i = 0; i < pQTBufInfo->vBufInfo.size(); i++)
    {
        NSCamHW::BufInfo rBufInfo(pQTBufInfo->vBufInfo.at(i).u4BufSize,
                         pQTBufInfo->vBufInfo.at(i).u4BufVA,
                         pQTBufInfo->vBufInfo.at(i).u4BufPA,
                         pQTBufInfo->vBufInfo.at(i).i4MemID);
        rQBufInfo.vBufInfo.push_back(rBufInfo);
    }

    pQTBufInfo->vBufInfo.clear();

    for (MUINT32 i = 0; i < rQBufInfo.vBufInfo.size(); i++)
    {
        MY_LOGD("[camioFt] (VA,PA,Size,ID)(0x%x,0x%x,%d,%d), ts(%d.%06d)",
            rQBufInfo.vBufInfo.at(i).u4BufVA,
            rQBufInfo.vBufInfo.at(i).u4BufPA, rQBufInfo.vBufInfo.at(i).u4BufSize, rQBufInfo.vBufInfo.at(i).i4MemID,
            rQBufInfo.i4TimeStamp_sec, rQBufInfo.i4TimeStamp_us);
    }
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MVOID
Ts_UT::
freeRawMem()
{
    for (int j = 0; j < 2; j++)
    {
        if(vRawMem[j].size() > 0)
        {
            printf("[vRawMem] deallocMem[%d]\n", j);
            int nImgW = (j==0)? u4SensorWidth:(u4SensorWidth >>1 );
            //   u4SensorWidth * u4SensorHeight * 2
            //=> u4SensorWidth * 1.25 * u4SensorHeight
            int nImgSize = (nImgW+(nImgW>>2))*u4SensorHeight ;

            for (int i = 0; i < BUF_NUM; i++)
            {
                char filename[256];
                sprintf(filename, "/data/imageio/raw%dx%d_%02d.raw", nImgW, u4SensorHeight, i);
                saveBufToFile(filename, reinterpret_cast<MUINT8*>(vRawMem[j].at(i).virtAddr), nImgSize);
                //imago_w * 1.25 * imgo_h
                deallocMem(vRawMem[j].at(i));
            }
        }
        if(vDequeMem[j].size() > 0)
        {
            printf("[vDequeMem] deallocMem\n");
            for (int i = 0; i <= 0; i++)
            {
                deallocMem(vDequeMem[j].at(i));
            }
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
    printf("[endeque_Thread] E\n");
//    MUINT32 count = 1000; //enque, deque loop iterations
    int count = (int)arg; //enque, deque loop iterations
      PortID rPortID;
    rPortID.index = PORT_IMGO.index;

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    do
    {
        printf("-----------------------------------\n");
        printf("[endeque_Thread] dequeBuf count(%d)\n", count);

        QTimeStampBufInfo rQTSBufInfo;
        if(!pTestUT->mpCamIOPipe->dequeOutBuf(rPortID, rQTSBufInfo))
        {
            printf("[endeque_Thread] dequeBuf=MFALSE\n");
            break;
        }

        pTestUT->rRawBuf.vBufInfo.clear();
        BufInfo rBufInfo;
        rBufInfo.u4BufSize[0] = rQTSBufInfo.vBufInfo[0].u4BufSize[0];
        rBufInfo.u4BufVA[0] = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        rBufInfo.u4BufPA[0] = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        pTestUT->rRawBuf.vBufInfo.push_back(rBufInfo);

        printf("[iopipetest] enque count(%d)\n", count);
        if(!pTestUT->mpCamIOPipe->enqueOutBuf(rPortID, pTestUT->rRawBuf))
        {
            printf("[endeque_Thread] enqueBuf failed\n");
            break;
        }
    } while(--count > 0);

    printf("[endeque_Thread] X\n");
    printf("-----------------------------------\n");
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
    printf("[TS_Thread_Init] E\n");
    Mutex::Autolock lock(mLock);
    printf("[TS_Thread_Init] mInitCount(%d) \n", mInitCount);
    //
    if(mInitCount > 0)
    {
        android_atomic_inc(&mInitCount);
    }
    else
    {
        // Init semphore
        ::sem_init(&m_semTestEnDequethread, 0, 0);

        // Create main thread for preview and capture
        pthread_create(&m_TestEnDequethread, NULL, endeque_Thread, (void *)count);
        android_atomic_inc(&mInitCount);
        pthread_join(m_TestEnDequethread, NULL);
    }
    printf("[TS_Thread_Init] X\n");
}

MBOOL
Ts_UT::
TS_Thread_UnInit()
{
    MBOOL ret = MTRUE;
    printf("[TS_Thread_UnInit] E\n");

    Mutex::Autolock lock(mLock);
    printf("[Thread_UnInit] mInitCount(%d)\n",mInitCount);
    //
    if(mInitCount <= 0)
    {
        // No more users
        printf("[TS_Thread_UnInit] no more user, X\n");
        return ret;
    }
    // More than one user
    android_atomic_dec(&mInitCount);
    //
    if(mInitCount > 0)
    {
        printf("[TS_Thread_UnInit] more than one user, X\n");
        return ret;
    }

//    pthread_cancel(m_TestEnDequethread);

    printf("[TS_Thread_UnInit] X\n");
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
//    int count = 1000; //enque, deque loop iterations

    printf("[imageiotest] E\n");
    PortID rPortID;
    rPortID.index = PORT_IMGO.index;

    //for (6). Enqueue, raw buf
    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 2    + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);

    MY_LOGD("+");
    /*------------------------------
    *    (1) Create Instance
    *-----------------------------*/
    NSCamPipe::ESWScenarioID const meSWScenarioID = NSCamPipe::eSWScenarioID_MTK_PREVIEW;
    NSCamPipe::EScenarioFmt const meScenarioFmt = NSCamPipe::eScenarioFmt_RAW;

    /*------------------------------
    *    (2) Query port property
    *-----------------------------*/
    printf("[imageiotest] queryPipeProperty\n");
    vector<NSCamPipe::PortProperty> rInPortProperty;
    vector<NSCamPipe::PortProperty> rOutPortProperty;
    if(queryPipeProperty(meSWScenarioID, rInPortProperty,rOutPortProperty))
    {
        MY_LOGD("[camioFt] Port Property (IN, OUT): (%d, %d)", rInPortProperty.size(), rOutPortProperty.size());
        for (MUINT32 i = 0; i < rInPortProperty.size(); i++)
        {
            MY_LOGD("[camioFt] IN: (type,index,inout) = (%d, %d, %d)", rInPortProperty.at(i).type, rInPortProperty.at(i).index, rInPortProperty.at(i).inout);
            MY_LOGD("[camioFt] IN: (fmt, rot, flip) = (%d, %d, %d)", rInPortProperty.at(i).u4SupportFmt,
                                         rInPortProperty.at(i).fgIsSupportRotate, rInPortProperty.at(i).fgIsSupportFlip);
        }
        for (MUINT32 i = 0; i < rOutPortProperty.size(); i++)
        {
            MY_LOGD("[camioFt] OUT: (type,index,inout) = (%d, %d, %d)", rOutPortProperty.at(i).type, rOutPortProperty.at(i).index, rOutPortProperty.at(i).inout);
            MY_LOGD("[camioFt] OUT: (fmt, rot, flip) = (%d, %d, %d)", rOutPortProperty.at(i).u4SupportFmt,
                                         rOutPortProperty.at(i).fgIsSupportRotate, rOutPortProperty.at(i).fgIsSupportFlip);
        }
    }
    /*------------------------------
    *    (3) init
    *-----------------------------*/
    printf("[imageiotest] ICamIOPipe::createInstance\n");
    char const Name[32] = {'1'};
    mpCamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, eScenarioFmt_RAW, (MINT8 const*)Name);
    if (mpCamIOPipe != NULL)
    {
        printf("[imageiotest] Pipe (Name, ID) = (%s, %d)\n", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
        MY_LOGD("[camioFt] Pipe (Name, ID) = (%s, %d)", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
    }
    else
    {
        printf("[imageiotest] createInstance failed\n");
        goto TEST_EXIT;
    }

    printf("[imageiotest] mpCamIOPipe->init\n");
    if (!mpCamIOPipe->init())
    {
        printf("[imageiotest] mpCamIOPipe->init failed\n");
        MY_LOGD("[camioFt] mpCamIOPipe->init failed");
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (4). setCallback
    *-----------------------------*/
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    /*------------------------------
    *    (5). Config pipe + RAW10
    *-----------------------------*/
    printf("[imageiotest] setConfigPortInfo\n");
    if(setConfigPortInfo(1) == MFALSE)
    {
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (6). Enqueue, raw buf
    *-----------------------------*/
    if(vRawMem[0].size() > 0)
    {
        freeRawMem();
#ifdef _USE_THREAD_QUE_
        TS_Thread_UnInit();
#endif
    }

    for (int i = 0; i < BUF_NUM; i++) //BUF_NUM=3
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
        vRawMem[0].push_back(rBuf);
    }

    //for deque
    for (int i = 0; i <= 0; i++)
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
        vDequeMem[0].push_back(rBuf);
        printf("[imageiotest] vDequeMem(%d)PA(0x%x)VA(0x%x)\n", i, vDequeMem[0].at(i).phyAddr, vDequeMem[0].at(i).virtAddr);
    }

    for (int i = 0; i < BUF_NUM; i++)
    {
        rRawBuf.vBufInfo.clear();
        BufInfo rBufInfo;
        rBufInfo.u4BufSize[0]   = vRawMem[0].at(i).size;
        rBufInfo.u4BufVA[0]     = vRawMem[0].at(i).virtAddr;
        rBufInfo.u4BufPA[0]     = vRawMem[0].at(i).phyAddr;
        rRawBuf.vBufInfo.push_back(rBufInfo);
        printf("[imageiotest] enqueBuf(%d)\n", i);
        if(!mpCamIOPipe->enqueOutBuf(rPortID, rRawBuf))
        {
            goto TEST_EXIT;
        }
    }

    /*------------------------------
    *    (7). start
    *-----------------------------*/
    printf("[imageiotest] start\n");
    start();

    /*------------------------------
    *    (8). enqueue
    *    --> dequeue
    *    --> enqueue
    *-----------------------------*/
    // Create thread for enque & deque
#ifdef _USE_THREAD_QUE_
    TS_Thread_Init(count);
#else
/*----------------------------------------------------*/
    do
    {
        printf("[imageiotest] dequeBuf count(%d)\n", count);

        QTimeStampBufInfo rQTSBufInfo;
        if(!mpCamIOPipe->dequeOutBuf(rPortID, rQTSBufInfo))
        {
            printf("[imageiotest] dequeBuf=MFALSE\n");
            goto TEST_EXIT;
        }

        rRawBuf.vBufInfo.clear();
        BufInfo rBufInfo;
        rBufInfo.u4BufSize[0]   = rQTSBufInfo.vBufInfo[0].u4BufSize[0];
        rBufInfo.u4BufVA[0]     = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        rBufInfo.u4BufPA[0]     = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        rBufInfo.img_w          = rQTSBufInfo.vBufInfo[0].img_w;
        rBufInfo.img_h          = rQTSBufInfo.vBufInfo[0].img_h;
        rBufInfo.img_stride     = rQTSBufInfo.vBufInfo[0].img_stride;
        rBufInfo.img_fmt        = rQTSBufInfo.vBufInfo[0].img_fmt;
        rBufInfo.img_pxl_id     = rQTSBufInfo.vBufInfo[0].img_pxl_id;
        rBufInfo.img_fmt        = rQTSBufInfo.vBufInfo[0].img_fmt;

        rRawBuf.vBufInfo.push_back(rBufInfo);

        printf("[iopipetest] enque count(%d)\n", count);
        if(!mpCamIOPipe->enqueOutBuf(rPortID, rRawBuf))
        {
            printf("[imageiotest] enqueBuf=MFALSE\n");
            goto TEST_EXIT;
        }

    } while(--count > 0);
#endif//_USE_THREAD_QUE_

TEST_EXIT:
    //
    if (NULL != mpCamIOPipe)
    {
        /*------------------------------
        *     (9). Stop
        *-----------------------------*/
        printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
        getchar();
        getchar();
        printf("[imageiotest] pCamIOPipe->stop\n");
        if (!mpCamIOPipe->stop())
        {
           MY_LOGE("[camioFt] mpCamIOPipe->stop() fail");
        }

        /*------------------------------
        *    (10). uninit
        *-----------------------------*/
        printf("[imageiotest] pCamIOPipe->uninit\n");
        if (MTRUE != mpCamIOPipe->uninit())
        {
            printf("[imageiotest] pCamIOPipe->uninit fail\n");
            MY_LOGE("[camioFt] pCamIOPipe->uninit fail");
        }
        printf("[imageiotest] pCamIOPipe->destroyInstance\n");
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }

    printf("[imageiotest] X\n");
    MY_LOGD("-");
    return 0;
}

/*******************************************************************************
*   2 port/floria
********************************************************************************/
int
Ts_UT::
main_ts_CamIO_2port(int count)
{
//    int count = 1000; //enque, deque loop iterations

    printf("[imageiotest] E\n");
    DMACfg pUserImg2oPara, pUserImgoPara;
    PortID rPortID[2];
    rPortID[0].index = PORT_IMGO.index;
      rPortID[1].index = PORT_IMG2O.index;

    //for (6). Enqueue, raw buf
    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 2    + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    MINT32 nMagic = 0;//

    MY_LOGD("+");
    /*------------------------------
    *    (1) Create Instance
    *-----------------------------*/
    NSCamPipe::ESWScenarioID const meSWScenarioID = NSCamPipe::eSWScenarioID_MTK_PREVIEW;
    NSCamPipe::EScenarioFmt const meScenarioFmt = NSCamPipe::eScenarioFmt_RAW;

    /*------------------------------
    *    (2) Query port property
    *-----------------------------*/
    printf("[imageiotest] queryPipeProperty\n");
    vector<NSCamPipe::PortProperty> rInPortProperty;
    vector<NSCamPipe::PortProperty> rOutPortProperty;
    if(queryPipeProperty(meSWScenarioID, rInPortProperty,rOutPortProperty))
    {
        MY_LOGD("[imageiotest]  Port Property (IN, OUT): (%d, %d)", rInPortProperty.size(), rOutPortProperty.size());
        for (MUINT32 i = 0; i < rInPortProperty.size(); i++)
        {
            MY_LOGD("[imageiotest] IN: (type,index,inout) = (%d, %d, %d)", rInPortProperty.at(i).type, rInPortProperty.at(i).index, rInPortProperty.at(i).inout);
            MY_LOGD("[imageiotest] IN: (fmt, rot, flip) = (%d, %d, %d)", rInPortProperty.at(i).u4SupportFmt,
                                         rInPortProperty.at(i).fgIsSupportRotate, rInPortProperty.at(i).fgIsSupportFlip);
        }
        for (MUINT32 i = 0; i < rOutPortProperty.size(); i++)
        {
            MY_LOGD("[imageiotest] OUT: (type,index,inout) = (%d, %d, %d)", rOutPortProperty.at(i).type, rOutPortProperty.at(i).index, rOutPortProperty.at(i).inout);
            MY_LOGD("[imageiotest] OUT: (fmt, rot, flip) = (%d, %d, %d)", rOutPortProperty.at(i).u4SupportFmt,
                                         rOutPortProperty.at(i).fgIsSupportRotate, rOutPortProperty.at(i).fgIsSupportFlip);
        }
    }
    /*------------------------------
    *    (3) init
    *-----------------------------*/
    printf("[imageiotest] ICamIOPipe::createInstance\n");
    char const Name[32] = {'1'};
    mpCamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, eScenarioFmt_RAW, (MINT8 const*)Name);
    if (mpCamIOPipe != NULL)
    {
        printf("[imageiotest] Pipe (Name, ID) = (%s, %d)\n", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
        MY_LOGD("[camioFt] Pipe (Name, ID) = (%s, %d)", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
    }
    else
    {
        printf("[imageiotest] createInstance failed\n");
        goto TEST_EXIT;
    }

    printf("[imageiotest] mpCamIOPipe->init\n");
    if (!mpCamIOPipe->init())
    {
        printf("[imageiotest] mpCamIOPipe->init failed\n");
        MY_LOGD("[camioFt] mpCamIOPipe->init failed");
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (4). setCallback
    *-----------------------------*/
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    /*------------------------------
    *    (5). Config pipe + RAW10
    *-----------------------------*/
    printf("[imageiotest] setConfigPortInfo\n");
    MY_LOGD("[camioFt] setConfigPortInfo");
    if(setConfigPortInfo(2) == MFALSE)
    {
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (6). Enqueue, raw buf
    *-----------------------------*/
    if(vRawMem[0].size() > 0 || vRawMem[1].size() > 0)
    {
        freeRawMem();
    }

    for(int j =0; j < 2; j++)
    {
        IMEM_BUF_INFO rBuf;
        for (int i = 0; i < BUF_NUM; i++) //BUF_NUM=3
        {
            rBuf.size = u4RawBufSize;
            allocMem(rBuf);
            vRawMem[j].push_back(rBuf);
        }
    }

    //for deque
    for(int j =0; j < 2; j++)
    {
        IMEM_BUF_INFO rBuf;
        for (int i = 0; i <= 0; i++)
        {
            rBuf.size = u4RawBufSize;
            allocMem(rBuf);
            vDequeMem[j].push_back(rBuf);
            printf("[iopipetest] vDequeMem(%d)PA(0x%x)VA(0x%x)\n", i, vDequeMem[j].at(i).phyAddr, vDequeMem[j].at(i).virtAddr);
        }
    }

    for(int j =0; j < 2; j++)
    {
        BufInfo rBufInfo;
        for (int i = 0; i < BUF_NUM; i++)
        {
            rRawBuf.vBufInfo.clear();
            rBufInfo.u4BufSize[0]   = vRawMem[j].at(i).size;
            rBufInfo.u4BufVA[0]     = vRawMem[j].at(i).virtAddr;
            rBufInfo.u4BufPA[0]     = vRawMem[j].at(i).phyAddr;
            rRawBuf.vBufInfo.push_back(rBufInfo);
            printf("[imageiotest] enqueBuf(%d)\n", i);
            if(!mpCamIOPipe->enqueOutBuf(rPortID[j], rRawBuf))
            {
                goto TEST_EXIT;
            }
        }
    }

    /*------------------------------
    *    (7). start
    *-----------------------------*/
    //IMGO
    pUserImgoPara.crop.x      = 0;
    pUserImgoPara.crop.y      = 0;
    pUserImgoPara.crop.floatX = 0;
    pUserImgoPara.crop.floatY = 0;
    pUserImgoPara.crop.w      = u4SensorWidth;
    pUserImgoPara.crop.h      = u4SensorHeight;
    pUserImgoPara.out.w = u4SensorWidth;
    pUserImgoPara.out.h = u4SensorHeight;
    pUserImgoPara.out.stride = 0;

    //IMG2O
    pUserImg2oPara.crop.x      = 0;
    pUserImg2oPara.crop.y      = 0;
    pUserImg2oPara.crop.floatX = 0;
    pUserImg2oPara.crop.floatY = 0;
    pUserImg2oPara.crop.w      = u4SensorWidth/2;
    pUserImg2oPara.crop.h      = u4SensorHeight;
    pUserImg2oPara.out.w = u4SensorWidth/2;//HRZ
    pUserImg2oPara.out.h = u4SensorHeight;
    pUserImg2oPara.out.stride = 0;
    /*----------------------------------------------------*/
    //IMGO
    pUserImgoPara.m_num = nMagic;
    printf("[imageiotest] SET_IMGO(0x%x)\n", EPIPECmd_SET_IMGO);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGO, (MINT32)(&pUserImgoPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);
    //IMG2O
    pUserImg2oPara.m_num = nMagic;
    printf("[imageiotest] SET_HRZ(0x%x)\n", EPIPECmd_SET_HRZ);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_HRZ, (MINT32)(&pUserImg2oPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);
    //P1 update
    printf("[imageiotest] P1_UPDATE(0x%x)\n", EPIPECmd_SET_P1_UPDATE);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, EPortIndex_TG1I, nMagic, eIspUpdateTgtDev_Main);
    //-------------------------------------
    printf("[imageiotest] start\n");
    start();

    /*------------------------------
    *    (8). enqueue
    *    --> dequeue
    *    --> enqueue
    *-----------------------------*/
/*----------------------------------------------------*/
    do
    {
        printf("[imageiotest] dequeBuf count(%d)\n", count);

        QTimeStampBufInfo rQTSBufInfo;
        for(int j = 0; j < 2; j++)
        {
            if(!mpCamIOPipe->dequeOutBuf(rPortID[j], rQTSBufInfo))
            {
                printf("[imageiotest] dequeBuf[%d]=MFALSE\n", j);
                goto TEST_EXIT;
            }

            rRawBuf.vBufInfo.clear();
            BufInfo rBufInfo;
            rBufInfo.u4BufSize[0]   = rQTSBufInfo.vBufInfo[0].u4BufSize[0];
            rBufInfo.u4BufVA[0]     = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
            rBufInfo.u4BufPA[0]     = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
            rBufInfo.img_w          = rQTSBufInfo.vBufInfo[0].img_w;
            rBufInfo.img_h          = rQTSBufInfo.vBufInfo[0].img_h;
            rBufInfo.img_stride     = rQTSBufInfo.vBufInfo[0].img_stride;
            rBufInfo.img_fmt        = rQTSBufInfo.vBufInfo[0].img_fmt;
            rBufInfo.img_pxl_id     = rQTSBufInfo.vBufInfo[0].img_pxl_id;
            rBufInfo.img_fmt        = rQTSBufInfo.vBufInfo[0].img_fmt;
            rBufInfo.m_num_0        = rQTSBufInfo.vBufInfo[0].m_num_0;

            rRawBuf.vBufInfo.push_back(rBufInfo);

            printf("[iopipetest] enque count(%d)\n", count);
            if(!mpCamIOPipe->enqueOutBuf(rPortID[j], rRawBuf))
            {
                printf("[imageiotest] enqueBuf[%d]=MFALSE\n", j);
                goto TEST_EXIT;
            }
        }
    } while(--count > 0);

TEST_EXIT:
    //
    if (NULL != mpCamIOPipe)
    {
        /*------------------------------
        *     (9). Stop
        *-----------------------------*/
        printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
        getchar();
        getchar();
        printf("[imageiotest] pCamIOPipe->stop\n");
        if (!mpCamIOPipe->stop())
        {
           MY_LOGE("[camioFt] mpCamIOPipe->stop() fail");
        }

        /*------------------------------
        *    (10). uninit
        *-----------------------------*/
        printf("[imageiotest] pCamIOPipe->uninit\n");
        if (MTRUE != mpCamIOPipe->uninit())
        {
            printf("[imageiotest] pCamIOPipe->uninit fail\n");
            MY_LOGE("[camioFt] pCamIOPipe->uninit fail");
        }
        printf("[imageiotest] pCamIOPipe->destroyInstance\n");
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }

    printf("[imageiotest] X\n");
    MY_LOGD("-");
    return 0;
}
/*******************************************************************************
*  Test P1Update,Set_HRZ /floria
********************************************************************************/
int
Ts_UT::
main_ts_CamIO_Update(int count)
{
//    int count = 1000; //enque, deque loop iterations
    printf("[CamIO_Update] E\n");
    DMACfg pUserImg2oPara, pUserImgoPara;
    PortID rPortID[2];
    /*-----------------------------*/
    int nPort = 2;
    /*------------------------------*/

    rPortID[0].index = PORT_IMGO.index;
      rPortID[1].index = PORT_IMG2O.index;

    //for (6). Enqueue, raw buf
    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 2    + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    MINT32 nMagic = 0;//

    MY_LOGD("+");
    /*------------------------------
    *    (1) Create Instance
    *-----------------------------*/
    NSCamPipe::ESWScenarioID const meSWScenarioID = NSCamPipe::eSWScenarioID_MTK_PREVIEW;
    NSCamPipe::EScenarioFmt const meScenarioFmt = NSCamPipe::eScenarioFmt_RAW;

    /*------------------------------
    *    (2) Query port property
    *-----------------------------*/
    printf("[imageiotest] queryPipeProperty\n");
    vector<NSCamPipe::PortProperty> rInPortProperty;
    vector<NSCamPipe::PortProperty> rOutPortProperty;
    if(queryPipeProperty(meSWScenarioID, rInPortProperty,rOutPortProperty))
    {
        MY_LOGD("[camioFt] Port Property (IN, OUT): (%d, %d)", rInPortProperty.size(), rOutPortProperty.size());
        for (MUINT32 i = 0; i < rInPortProperty.size(); i++)
        {
            MY_LOGD("[camioFt] IN: (type,index,inout) = (%d, %d, %d)", rInPortProperty.at(i).type, rInPortProperty.at(i).index, rInPortProperty.at(i).inout);
            MY_LOGD("[camioFt] IN: (fmt, rot, flip) = (%d, %d, %d)", rInPortProperty.at(i).u4SupportFmt,
                                         rInPortProperty.at(i).fgIsSupportRotate, rInPortProperty.at(i).fgIsSupportFlip);
        }
        for (MUINT32 i = 0; i < rOutPortProperty.size(); i++)
        {
            MY_LOGD("[camioFt] OUT: (type,index,inout) = (%d, %d, %d)", rOutPortProperty.at(i).type, rOutPortProperty.at(i).index, rOutPortProperty.at(i).inout);
            MY_LOGD("[camioFt] OUT: (fmt, rot, flip) = (%d, %d, %d)", rOutPortProperty.at(i).u4SupportFmt,
                                         rOutPortProperty.at(i).fgIsSupportRotate, rOutPortProperty.at(i).fgIsSupportFlip);
        }
    }
    /*------------------------------
    *    (3) init
    *-----------------------------*/
    printf("[imageiotest] ICamIOPipe::createInstance\n");
    char const Name[32] = {'1'};
    mpCamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, eScenarioFmt_RAW, (MINT8 const*)Name);
    if (mpCamIOPipe != NULL)
    {
        printf("[imageiotest] Pipe (Name, ID) = (%s, %d)\n", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
        MY_LOGD("[camioFt] Pipe (Name, ID) = (%s, %d)", mpCamIOPipe->getPipeName(), mpCamIOPipe->getPipeId());
    }
    else
    {
        printf("[imageiotest] createInstance failed\n");
        goto TEST_EXIT;
    }

    printf("[imageiotest] mpCamIOPipe->init\n");
    if (!mpCamIOPipe->init())
    {
        printf("[imageiotest] mpCamIOPipe->init failed\n");
        MY_LOGD("[camioFt] mpCamIOPipe->init failed");
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (4). setCallback
    *-----------------------------*/
    mpCamIOPipe->setCallbacks(NULL, NULL, NULL);

    /*------------------------------
    *    (5). Config pipe + RAW10
    *-----------------------------*/
    printf("[imageiotest] setConfigPortInfo\n");
    MY_LOGD("[camioFt] setConfigPortInfo");
    if(setConfigPortInfo(nPort) == MFALSE)
    {
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (6). Enqueue, raw buf
    *-----------------------------*/
    if(vRawMem[0].size() > 0 || vRawMem[1].size() > 0)
    {
        freeRawMem();
    }

    for(int j =0; j < nPort; j++)
    {
        IMEM_BUF_INFO rBuf;
        for (int i = 0; i < BUF_NUM; i++) //BUF_NUM=3
        {
            rBuf.size = u4RawBufSize;
            allocMem(rBuf);
            vRawMem[j].push_back(rBuf);
        }
    }

    //for deque
    for(int j =0; j < nPort; j++)
    {
        IMEM_BUF_INFO rBuf;
        for (int i = 0; i <= 0; i++)
        {
            rBuf.size = u4RawBufSize;
            allocMem(rBuf);
            vDequeMem[j].push_back(rBuf);
            printf("[iopipetest] vDequeMem(%d)PA(0x%x)VA(0x%x)\n", i, vDequeMem[j].at(i).phyAddr, vDequeMem[j].at(i).virtAddr);
        }
    }

    for(int j =0; j < nPort; j++)
    {
        BufInfo rBufInfo;
        for (int i = 0; i < BUF_NUM; i++)
        {
            rRawBuf.vBufInfo.clear();
            rBufInfo.u4BufSize[0]   = vRawMem[j].at(i).size;
            rBufInfo.u4BufVA[0]     = vRawMem[j].at(i).virtAddr;
            rBufInfo.u4BufPA[0]     = vRawMem[j].at(i).phyAddr;
            rRawBuf.vBufInfo.push_back(rBufInfo);
            printf("[imageiotest] enqueBuf(%d)\n", i);
            if(!mpCamIOPipe->enqueOutBuf(rPortID[j], rRawBuf))
            {
                goto TEST_EXIT;
            }
        }
    }

    /*------------------------------
    *    settings before start
    *-----------------------------*/
    //IMGO
    pUserImgoPara.crop.x      = 0;
    pUserImgoPara.crop.y      = 0;
    pUserImgoPara.crop.floatX = 0;
    pUserImgoPara.crop.floatY = 0;
    pUserImgoPara.crop.w      = u4SensorWidth;
    pUserImgoPara.crop.h      = u4SensorHeight;
    pUserImgoPara.out.w = u4SensorWidth;
    pUserImgoPara.out.h = u4SensorHeight;
    pUserImgoPara.out.stride = 0;
    //IMG2O
    pUserImg2oPara.crop.x      = 0;
    pUserImg2oPara.crop.y      = 0;
    pUserImg2oPara.crop.floatX = 0;
    pUserImg2oPara.crop.floatY = 0;
    pUserImg2oPara.crop.w      = u4SensorWidth/2;
    pUserImg2oPara.crop.h      = u4SensorHeight;
    pUserImg2oPara.out.w = u4SensorWidth/2;//HRZ
    pUserImg2oPara.out.h = u4SensorHeight;
    pUserImg2oPara.out.stride = 0;
    /*----------------------------------------------------*/
    //IMGO
    pUserImgoPara.m_num = nMagic;
    printf("[imageiotest] SET_IMGO(0x%x)\n", EPIPECmd_SET_IMGO);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGO, (MINT32)(&pUserImgoPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);
    //IMG2O
    pUserImg2oPara.m_num = nMagic;
    printf("[imageiotest] SET_HRZ(0x%x)\n", EPIPECmd_SET_HRZ);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_HRZ, (MINT32)(&pUserImg2oPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);
    //P1 update
    printf("[imageiotest] P1_UPDATE(0x%x)\n", EPIPECmd_SET_P1_UPDATE);
    mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, EPortIndex_TG1I, nMagic, eIspUpdateTgtDev_Main);

    /* only for UT,
       need to enable
       1. p1ConvModuleID, _p1GetModuleTgIdx in CamIOPipe_FrmB.cpp,
       2. setModuleEn in p1Hwcfg_FrmB.cpp
       CAM_ISP_OBC actually implements in isp_function_FrmB.cpp!!!
    */
    #if 0
    printf("[imageiotest] OB_MODULE: enable\n");
    if (MFALSE==mpCamIOPipe->sendCommand(EPIPECmd_SET_MODULE_EN,EModule_OB, MTRUE, NULL))
    {
        printf("EPIPECmd_SET_MODULE_En fail: EModule_OB");
        goto TEST_EXIT;
    }
    #endif

    /*------------------------------
    *    (7). start
    *-----------------------------*/
    printf("[imageiotest] start\n");
    start();

    /*------------------------------
    *    (8). enqueue
    *    --> dequeue
    *    --> enqueue
    *-----------------------------*/
    do
    {
        printf("--------------------------------------\n", nMagic);

        //deque
        printf("[imageiotest] dequeBuf count(%d)\n", count);

        QTimeStampBufInfo rQTSBufInfo[2];
        for(int j = 0; j < nPort; j++)
        {
            if(!mpCamIOPipe->dequeOutBuf(rPortID[j], rQTSBufInfo[j]))
            {
                printf("[imageiotest] dequeBuf[%d]=MFALSE\n", j);
                goto TEST_EXIT;
            }

            printf("w/h/stride(%d/%d/%d)\n",
                                rQTSBufInfo[j].vBufInfo[0].img_w, \
                                rQTSBufInfo[j].vBufInfo[0].img_h, \
                                rQTSBufInfo[j].vBufInfo[0].img_stride);
        }

        //Magic Num
        printf("[imageiotest] nMagic(%d)\n", nMagic);

        printf("[imageiotest] SET_IMGO(0x%x)\n", EPIPECmd_SET_IMGO);
        pUserImgoPara.m_num = nMagic;
        mpCamIOPipe->sendCommand(EPIPECmd_SET_IMGO, (MINT32)(&pUserImgoPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);

        printf("[imageiotest] SET_HRZ(0x%x)\n", EPIPECmd_SET_HRZ);
        pUserImg2oPara.m_num = nMagic;
        mpCamIOPipe->sendCommand(EPIPECmd_SET_HRZ, (MINT32)(&pUserImg2oPara), EPortIndex_TG1I, eIspUpdateTgtDev_Main);

        printf("[imageiotest] P1_UPDATE(0x%x)\n", EPIPECmd_SET_P1_UPDATE);
        mpCamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE, EPortIndex_TG1I, nMagic, eIspUpdateTgtDev_Main);
        nMagic++;


        //enque
        for(int j = 0; j < nPort; j++)
        {
            rRawBuf.vBufInfo.clear();
            BufInfo rBufInfo;
            rBufInfo.u4BufSize[0]   = rQTSBufInfo[j].vBufInfo[0].u4BufSize[0];
            rBufInfo.u4BufVA[0]     = rQTSBufInfo[j].vBufInfo[0].u4BufVA[0];
            rBufInfo.u4BufPA[0]     = rQTSBufInfo[j].vBufInfo[0].u4BufPA[0];
            rBufInfo.img_w          = rQTSBufInfo[j].vBufInfo[0].img_w;
            rBufInfo.img_h          = rQTSBufInfo[j].vBufInfo[0].img_h;
            rBufInfo.img_stride     = rQTSBufInfo[j].vBufInfo[0].img_stride;
            rBufInfo.img_fmt        = rQTSBufInfo[j].vBufInfo[0].img_fmt;
            rBufInfo.img_pxl_id     = rQTSBufInfo[j].vBufInfo[0].img_pxl_id;
            rBufInfo.img_fmt        = rQTSBufInfo[j].vBufInfo[0].img_fmt;

            rRawBuf.vBufInfo.push_back(rBufInfo);

            printf("[iopipetest] enque count(%d)\n", count);
            if(!mpCamIOPipe->enqueOutBuf(rPortID[j], rRawBuf))
            {
                printf("[imageiotest] enqueBuf[%d]=MFALSE\n", j);
                goto TEST_EXIT;
            }
        }
        usleep(10000);//sleep 10ms
    } while(--count > 0);

TEST_EXIT:
    //
    if (NULL != mpCamIOPipe)
    {
        /*------------------------------
        *     (9). Stop
        *-----------------------------*/
        printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
        getchar();
        getchar();
        /* only for UT */
        #if 0
        printf("[imageiotest] OB_MODULE: disable\n");
        if (MFALSE==mpCamIOPipe->sendCommand(EPIPECmd_SET_MODULE_EN,EModule_OB, MFALSE, NULL))
        {
            printf("EPIPECmd_SET_MODULE_En fail: EModule_OB");
        }
        #endif

        printf("[imageiotest] pCamIOPipe->stop\n");
        if (!mpCamIOPipe->stop())
        {
           MY_LOGE("[camioFt] mpCamIOPipe->stop() fail");
        }

        /*------------------------------
        *    (10). uninit
        *-----------------------------*/
        printf("[imageiotest] pCamIOPipe->uninit\n");
        if (MTRUE != mpCamIOPipe->uninit())
        {
            printf("[imageiotest] pCamIOPipe->uninit fail\n");
            MY_LOGE("[camioFt] pCamIOPipe->uninit fail");
        }
        printf("[imageiotest] pCamIOPipe->destroyInstance\n");
        mpCamIOPipe->destroyInstance();
        mpCamIOPipe = NULL;
    }

    printf("[imageiotest] X\n");
    MY_LOGD("-");
    return 0;

}

}
}
/*******************************************************************************
*  Main Function
********************************************************************************/
int main_Camio(int argc, char** argv)
{
    int ret = 0;
    int testItem = 0;
    int loopcount = 0;

    if (argc != 3)
    {
        printf("Usage: ImageioTest 0 <test_item> <mode>\n");
        printf("<test item> 0:enque/deque, 1:open 2 port, 2: updateP1, else:exit\n");
        printf("<mode> 0: prev\n");
        goto EXIT;
    }
    else
    {
      testItem = atoi(argv[1]);
      pTestUT->mode = 0;;//atoi(argv[2]);
    }

    /*----------------------------
            IMEM
    ----------------------------*/
    printf("[Main] IMemDrv::createInstance\n");
    pTestUT->mpIMemDrv =  IMemDrv::createInstance();
    if (NULL == pTestUT->mpIMemDrv)
    {
        printf("[Main] g_pIMemDrv is NULL\n");
        MY_LOGE("[camioFt] g_pIMemDrv is NULL");
        return 0;
    }
    printf("[Main] g_pIMemDrv->init\n");
    if ( !pTestUT->mpIMemDrv->init() )
    {
        printf("[Main] g_pIMemDrv->init fail\n");
        MY_LOGE("[camioFt] g_pIMemDrv init fail");
    }

    /*----------------------------
            Test Items
    ----------------------------*/
    while(loopcount<=0){
        printf("*****************************************\n");
        printf("please loop times(must >0)\n");
        scanf("%d", &loopcount);
        printf("*****************************************\n");
    }
    while(testItem >= 0 && testItem <= 2)
    {
        switch(testItem)
        {
            case 0://pass1
                printf("[Main] testItem(%d)\n", testItem);
                printf("[Main] call main_SetSensor_init\n");
                ret = pTestUT->main_SetSensor_init();
                if(ret < 0)
                    break;

                printf("[Main] main_CamIO_ZSD\n");
                pTestUT->main_ts_CamIO_ZSD(loopcount);
                printf("[Main] main_SetSensor_uninit\n");
                pTestUT->main_SetSensor_uninit();
                break;
            case 1://pass1: imgo+img2o
                printf("[Main] testItem(%d)\n", testItem);
                printf("[Main] call main_SetSensor_init\n");
                ret = pTestUT->main_SetSensor_init();
                if(ret < 0)
                    break;

                printf("[Main] main_CamIO_2port\n");
                pTestUT->main_ts_CamIO_2port(loopcount);
                printf("[Main] main_SetSensor_uninit\n");
                pTestUT->main_SetSensor_uninit();
                break;
            case 2://
                printf("[Main] testItem(%d)\n", testItem);
                printf("[Main] call main_SetSensor_init\n");
                ret = pTestUT->main_SetSensor_init();
                if(ret < 0)
                    break;

                printf("[Main] main_CamIO_Update\n");
                pTestUT->main_ts_CamIO_Update(loopcount);
                printf("[Main] main_SetSensor_uninit\n");
                pTestUT->main_SetSensor_uninit();
                break;
            default:
                break;
        }
        /* Get next testitem */
        printf("*****************************************\n");
        printf("please input the test item,\n0: camio_zsd,\n1: camio 2 port\n2: updateP1\nelse:");
#ifdef _USE_THREAD_QUE_
            printf("exit enque+deque thread\n");
#else
            printf("exit\n");
#endif
            printf("*****************************************\n");
            scanf("%d", &testItem);
    }

 #ifdef _USE_THREAD_QUE_
    pTestUT->TS_Thread_UnInit();
 #endif

    printf("[Main] freeRawMem\n");
    pTestUT->freeRawMem();

    printf("[Main] g_pIMemDrv->destroyInstance()\n");
    pTestUT->mpIMemDrv->destroyInstance();


EXIT:
    printf("press any key to exit \n");
    getchar();

    return ret;
}
