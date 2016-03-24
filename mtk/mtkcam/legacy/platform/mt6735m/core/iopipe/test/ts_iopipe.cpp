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
#include "ts_iopipe.h"

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

    printf("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    printf("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT, S_IRWXU);
    if (fd < 0) {
        printf("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    printf("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            printf("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    printf("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

/*******************************************************************************
*
********************************************************************************/
Ts_UT::
Ts_UT()
    : mpSensorHal(NULL)
//    , mu4DeviceID(0)
    , mfgIsYUVPortON(MFALSE)
    , continuous(1)
    , is_yuv_sensor(0)
    , u4Scenario(ACDK_SCENARIO_ID_CAMERA_PREVIEW)
    , u4Bitdepth(10)
    , u4SensorFullWidth(0)
    , u4SensorFullHeight(0)
    , u4SensorHalfWidth(0)
    , u4SensorHalfHeight(0)
    , u4SensorWidth(0)
    , u4SensorHeight(0)
{
    sensorDevId = (halSensorDev_e)(is_yuv_sensor ? SENSOR_DEV_SUB : SENSOR_DEV_MAIN);
}

Ts_UT::
~Ts_UT()
{

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
Ts_UT::
skipFrame(MUINT32 const u4SkipCount)
{
    MBOOL ret = MTRUE;

    return ret;
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
        printf("[freeRawMem] vRawMem\n");
        int vSize = vRawMem.size();
        for (int i = 0; i < vSize; i++)
        {
            char filename[256];
            sprintf(filename, "/data/iopipe/raw%dx%d_%02d.raw", u4SensorWidth, u4SensorHeight, i);
            saveBufToFile(filename, reinterpret_cast<MUINT8*>(vRawMem.at(i).virtAddr), u4SensorWidth * u4SensorHeight * 2);
            deallocMem(vRawMem.at(i));
        }

        //deque mem
        printf("[freeRawMem] vDequeMem\n");
        vSize = vDequeMem.size();
        for (int i = 0; i < vSize; i++)
        {
            deallocMem(vDequeMem.at(i));
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
    int count = (int)arg; //enque, deque loop iterations
    MSize mDstSize;
    mDstSize.w = pTestUT->u4SensorWidth;
    mDstSize.h = pTestUT->u4SensorHeight;

    //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());
    do
    {
        printf("-----------------------------------\n");
        printf("[endeque_Thread] dequeBuf count(%d)\n", count);

        QBufInfo halCamIOOutQBuf;
        QBufInfo rEnBuf;
        /*
         * Deque
         */
        halCamIOOutQBuf.mvOut.clear();
#ifdef USE_IMAGEBUF_HEAP
        BufInfo rDeBufInfo(PORT_IMGO, pTestUT->rDequeBuf.mvOut.at(0).mBuffer, pTestUT->rDequeBuf.mvOut.at(0).mSize, pTestUT->rDequeBuf.mvOut.at(0).mVa, pTestUT->rDequeBuf.mvOut.at(0).mPa);
#else
        BufInfo rDeBufInfo(PORT_IMGO, NULL, pTestUT->vDequeMem.at(0).size, pTestUT->vDequeMem.at(0).virtAddr, pTestUT->vDequeMem.at(0).phyAddr);
#endif
        halCamIOOutQBuf.mvOut.push_back(rDeBufInfo);
        //
        if(!pTestUT->mpCamIO->deque(halCamIOOutQBuf))
        {
            printf("[endeque_Thread] deque failed!!\n");
            return NULL;
        }
        /*
         * Enque
         */
        rEnBuf.mvOut.clear();
        BufInfo rBufInfo(halCamIOOutQBuf.mvOut.at(0).mPortID,
#ifdef USE_IMAGEBUF_HEAP
                         halCamIOOutQBuf.mvOut.at(0).mBuffer,
#else
                         NULL,
#endif
                         halCamIOOutQBuf.mvOut.at(0).mSize,
                         halCamIOOutQBuf.mvOut.at(0).mVa,
                         halCamIOOutQBuf.mvOut.at(0).mPa);

        rEnBuf.mvOut.push_back(rBufInfo);

        /* * * * * * * * * * * *
         * setting before enque
         * * * * * * * * * * * */
        rEnBuf.mvOut.at(0).FrameBased.mMagicNum_tuning = 0;
        rEnBuf.mvOut.at(0).FrameBased.mDstSize = mDstSize;
        rEnBuf.mvOut.at(0).FrameBased.mCropRect = MRect(MPoint(0, 0), mDstSize);//no crop
        //
        printf("[endeque_Thread] enqueBuf count(%d)\n", count);
        if(MFALSE == pTestUT->mpCamIO->enque(rEnBuf))
        {
            printf("[endeque_Thread] enqueBuf failed\n");
            return NULL;
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

void prepareSensor()
{
    MUINT const gSensorId = 1;
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    pHalSensorList->searchSensors();
    IHalSensor* pSensorHalObj = pHalSensorList->createSensor("tester", gSensorId);
    MUINT32 sensorArray[1] = {gSensorId};
}

/*******************************************************************************
*  Config CamIO Pipe /floria
********************************************************************************/
int
Ts_UT::
main_ts_IOPipe_ZSD(int count)
{
//    int count = 1000; //enque, deque loop iterations

    printf("[iopipetest] E\n");

    //for Enqueue, raw buf
    MUINT32 u4RawBufSize = (u4SensorWidth * u4SensorHeight * 2    + L1_CACHE_BYTES-1) & ~(L1_CACHE_BYTES-1);
    //for buffer per frame
    int BufIdx = BUF_NUM, nCt = BUF_NUM;
#ifdef USE_IMAGEBUF_HEAP
    MUINT32 bufStridesInBytes[3] = {1600, 0, 0};
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
#endif


#if 0
    ringbuffer* mpRingImgo = new ringbuffer(
                                            2,//PASS1_FULLRAW
                                            PORT_IMGO,
                                            0//fakeResized ? PASS1_RESIZEDRAW : 0
                                            );
#endif
    MY_LOGD("+");
    /*------------------------------
    *    (1) Create Instance
    *-----------------------------*/
    /* eScenarioID_VSS, eScenarioFmt_RAW */
    MUINT mSensorIdx = 0;//0:MAIN
    const char Name = '1';
    MUINT mIspEnquePeriod = 1;

    prepareSensor();//test IHalSensor

    printf("[iopipetest] INormalPipe::createInstance\n");
    mpCamIO = (INormalPipe*)INormalPipe::createInstance(mSensorIdx, &Name, mIspEnquePeriod);

#if 0//camera 3.0, should create INormalPipe_FrmB class instance
    mpCamIO = (IHalCamIO*)INormalPipe_FrmB::createInstance(mSensorIdx, &Name, mIspEnquePeriod);
#endif

    /*------------------------------
        test 3A build pass
    ------------------------------*/
    #if 0
    MINT32 handle;
    MBOOL fgRet = mpCamIO->sendCommand(NSImageio_FrmB::NSIspio_FrmB::EPIPECmd_GET_MODULE_HANDLE,
                                       NSImageio_FrmB::NSIspio_FrmB::EModule_AF,
                                       (MINT32)&handle,
                                       (MINT32)(&("AFMgr::setFlkWinConfig()")));
    MINT32 wintmp;
    IOPIPE_SET_MODUL_REG(handle, CAM_AF_WINX01, wintmp);
    #endif

    /*------------------------------
    *    (2) init
    *-----------------------------*/
    printf("[iopipetest] mpCamIO->init\n");
    if(!mpCamIO->init())
    {
        printf("[iopipetest] mpCamIO->init failed!!\n");
    }

    /*------------------------------
    *    (3). Config pipe + RAW10
    *-----------------------------*/
    MSize mDstSize;
    mDstSize.w = u4SensorWidth;
    mDstSize.h = u4SensorHeight;

    //prepare sensor config
    vector<IHalSensor::ConfigParam> vSensorCfg;
    IHalSensor::ConfigParam sensorCfg =
    {
        mSensorIdx,                         /* index            */
        mDstSize,                           /* crop             no reference in NormalPipe */
        u4Scenario,                         /* scenarioId       */
        0,                                  /* isBypassScenario */
        1,                                  /* isContinuous     */
        0,                                  /* iHDROn           */
        0,                                  /* framerate        */
        0,                                  /* two pixel on     */
        0,                                  /* debugmode        */
    };
    vSensorCfg.push_back(sensorCfg);
    printf("[iopipetest] sensor %dx%d, sce %d, bypass %d, con %d, hdr %d, fps %d, twopxl %d\n",
                sensorCfg.crop.w,
                sensorCfg.crop.h,
                sensorCfg.scenarioId,
                sensorCfg.isBypassScenario,
                sensorCfg.isContinuous,
                sensorCfg.HDRMode,
                sensorCfg.framerate,
                sensorCfg.twopixelOn);
    //

    vector<portInfo> vPortInfo;
    //
    portInfo OutPort(
            PORT_IMGO,
            eImgFmt_BAYER10,
            mDstSize, //dst size
            0, //pPortCfg->mCrop, //crop
            u4SensorWidth, //pPortCfg->mStrideInByte[0],
            0, //pPortCfg->mStrideInByte[1],
            0, //pPortCfg->mStrideInByte[2],
            0, // pureraw
            MTRUE               //packed
            );
    vPortInfo.push_back(OutPort);
    printf("[iopipetest] config portID(0x%x), fmt(%u), size(%dx%d), crop(%u,%u,%u,%u)\n",
            OutPort.mPortID, OutPort.mFmt, OutPort.mDstSize.w, OutPort.mDstSize.h,
            OutPort.mCropRect.p.x, OutPort.mCropRect.p.y,
            OutPort.mCropRect.s.w, OutPort.mCropRect.s.h);
    printf("[iopipetest] stride(%u,%u,%u), pureRaw(%u), pack(%d)\n",
            OutPort.mStride[0], OutPort.mStride[1], OutPort.mStride[2],
            OutPort.mPureRaw, OutPort.mPureRawPak);
    //
    QInitParam halCamIOinitParam(
                0,
                u4Bitdepth,
                vSensorCfg,
                vPortInfo);

    printf("[iopipetest] mpCamIO->configPipe\n");
    if(!mpCamIO->configPipe(halCamIOinitParam, eScenarioID_VSS))
    {
        printf("[iopipetest] mpCamIO->configPipe failed!!\n");
        goto TEST_EXIT;
    }

    /*------------------------------
    *    (4). Enqueue
    *     4.1, raw buf
    *-----------------------------*/
#ifdef _USE_THREAD_QUE_
    if(vRawMem.size() > 0)
    {
        freeRawMem();
        TS_Thread_UnInit();
    }
    nCt--;
#else
    //
    int nReplace;
    printf("*****************************************\n");
    printf("Buffer per frame(1:y; else:n)\n");
    scanf("%d", &nReplace);
    printf("*****************************************\n");

    if(nReplace != 1)
    {
        nCt--;
    }
#endif

    /* * * * * * * * * * * *
     * buffer per frame
     * nReplace = 1
     * size = BUF_NUM + 1
     *(replace buffer)

     * * * * * * * * * * * *

     * sequential buffer
     * nReplace != 1
     * size = BUF_NUM
     *(no replace buffer)
     * * * * * * * * * * * */
    printf("[iopipetest] allocMem: RawBuff\n");
    for (int i = 0; i <= nCt; i++) //BUF_NUM=3
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
#ifdef USE_IMAGEBUF_HEAP
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( rBuf.memID,rBuf.virtAddr,0,rBuf.bufSecu, rBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                        MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
        IImageBuffer* tempBuffer = pHeap->createImageBuffer();
        tempBuffer->incStrong(tempBuffer);
        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        //
        BufInfo rBufInfo;
        rBufInfo.mPortID = PORT_IMGO;
        rBufInfo.mBuffer = tempBuffer;
        rBufInfo.FrameBased.mMagicNum_tuning = 0;
        rBufInfo.FrameBased.mDstSize = mDstSize;
        rBufInfo.FrameBased.mCropRect = MRect(MPoint(0, 0), mDstSize);//no crop

        rRawBuf.mvOut.push_back(rBufInfo);
#endif
        vRawMem.push_back(rBuf);
        printf("[iopipetest] vRawMem(%d)PA(0x%x)VA(0x%x)\n", i, vRawMem.at(i).phyAddr,vRawMem.at(i).virtAddr);
    }

    //for deque
    for (int i = 0; i <= 0; i++)
    {
        IMEM_BUF_INFO rBuf;
        rBuf.size = u4RawBufSize;
        allocMem(rBuf);
#ifdef USE_IMAGEBUF_HEAP
        PortBufInfo_v1 portBufInfo = PortBufInfo_v1( rBuf.memID,rBuf.virtAddr,0,rBuf.bufSecu, rBuf.bufCohe);
        IImageBufferAllocator::ImgParam imgParam = IImageBufferAllocator::ImgParam((eImgFmt_BAYER10),
                                                        MSize(1280, 720), bufStridesInBytes, bufBoundaryInBytes, 1);
        sp<ImageBufferHeap> pHeap = ImageBufferHeap::create( LOG_TAG, imgParam,portBufInfo,MTRUE);
        IImageBuffer* tempBuffer = pHeap->createImageBuffer();
        tempBuffer->incStrong(tempBuffer);
        tempBuffer->lockBuf(LOG_TAG,eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN);
        //
        BufInfo rBufInfo;
        rBufInfo.mPortID = PORT_IMGO;
        rBufInfo.mBuffer = tempBuffer;
        rDequeBuf.mvOut.push_back(rBufInfo);
#endif
        vDequeMem.push_back(rBuf);
        printf("[iopipetest] vDequeMem(%d)PA(0x%x)VA(0x%x)\n", i, vDequeMem.at(i).phyAddr, vDequeMem.at(i).virtAddr);
    }

    /* enque 3 buffers */
    for (int i = 0; i < BUF_NUM; i++)
    {
#if 1
        QBufInfo rInBuf;
        rInBuf.mvOut.clear();
    #ifdef USE_IMAGEBUF_HEAP
        BufInfo rBufInfo(PORT_IMGO, rRawBuf.mvOut.at(i).mBuffer, rRawBuf.mvOut.at(i).mSize, rRawBuf.mvOut.at(i).mVa, rRawBuf.mvOut.at(i).mPa);
    #else
        BufInfo rBufInfo(PORT_IMGO, NULL, vRawMem.at(i).size, vRawMem.at(i).virtAddr, vRawMem.at(i).phyAddr);
    #endif
        rInBuf.mvOut.push_back(rBufInfo);
        rInBuf.mvOut.at(0).FrameBased.mMagicNum_tuning = 0;
        rInBuf.mvOut.at(0).FrameBased.mDstSize = mDstSize;
        rInBuf.mvOut.at(0).FrameBased.mCropRect = MRect(MPoint(0, 0), mDstSize);//no crop

#ifdef USE_IMAGEBUF_HEAP
    printf("[iopipetest] enque.PA(0x%x)VA(0x%x)buf(0x%x)\n", rInBuf.mvOut.at(0).mBuffer->getBufPA(0), rInBuf.mvOut.at(0).mBuffer->getBufVA(0), rInBuf.mvOut.at(0).mBuffer);
#else
    printf("[iopipetest] enque.PA(0x%x)VA(0x%x)\n", rInBuf.mvOut.at(0).mPa, rInBuf.mvOut.at(0).mVa);
#endif

        printf("[iopipetest] enque(%d)\n", i);
        if(!mpCamIO->enque(rInBuf))
        {
            printf("[iopipetest] enque failed!!\n");
            goto TEST_EXIT;
        }
#else
        /* * * * * * * * * * * * * * * * * * * * * * * * * *
         * test:
         * if enque a buffer,and it's port is not IMGO,
         * mpCamIO->enque(rRawBuf)will return MFALSE
         * * * * * * * * * * * * * * * * * * * * * * * * * */
        QBufInfo rInBuf;
        rInBuf.mvOut.clear();
    #ifdef USE_IMAGEBUF_HEAP
        BufInfo rBufInfo2(PORT_IMGO, rRawBuf.mvOut.at(i).mBuffer, rRawBuf.mvOut.at(i).mSize, rRawBuf.mvOut.at(i).mVa, rRawBuf.mvOut.at(i).mPa);
    #else
        BufInfo rBufInfo2(PORT_IMG2O, NULL, vRawMem.at(i).size, vRawMem.at(i).virtAddr, vRawMem.at(i).phyAddr);
    #endif
        rInBuf.mvOut.push_back(rBufInfo2);

        printf("[iopipetest] enque(%d)\n", i);

        if(!mpCamIO->enque(rInBuf)){//try to enque IMGO port
            printf("[iopipetest] enque(%d) again\n", i);
            rInBuf.mvOut.clear();
            BufInfo rBufInfo(PORT_IMGO, NULL, vRawMem.at(i).size, vRawMem.at(i).virtAddr, vRawMem.at(i).phyAddr);
            rInBuf.mvOut.push_back(rBufInfo);
            if(!mpCamIO->enque(rInBuf))
            {
                printf("[iopipetest] enque failed!!\n");
                goto TEST_EXIT;
            }
        }
#endif
    }
    /*------------------------------
    *    (5). start
    *-----------------------------*/
    printf("[iopipetest] mpCamIO->start\n");
    if(!mpCamIO->start())
    {
        printf("[iopipetest] mpCamIO->start failed!!\n");
        goto TEST_EXIT;
    }
#if 0//for debug
    // test: wait VSYNC
    printf("[iopipetest] mpCamIO->irq(VSYNC)\n");
    if(!mpCamIO->wait(mSensorIdx, EPipeSignal_SOF))
    {
        printf("[iopipetest] wait VSYNC failed!!\n");
        goto TEST_EXIT;
    }
    // test: wait pass1 done
    printf("[iopipetest] mpCamIO->irq(p1done)\n");
    if(!mpCamIO->wait(mSensorIdx, EPipeSignal_EOF))
    {
        printf("[iopipetest] wait p1done failed!!\n");
        goto TEST_EXIT;
    }
#endif
    /*------------------------------
    *    (6). deque/enque loop
    *    --> dequeue
    *    --> enqueue
    *-----------------------------*/
#ifdef _USE_THREAD_QUE_
    TS_Thread_Init(count);
#else
    do
    {
        printf("-------------------------------------------\n");
        QBufInfo halCamIOOutQBuf;
        QBufInfo rEnBuf;
        //
        halCamIOOutQBuf.mvOut.clear();
    #ifdef USE_IMAGEBUF_HEAP
        BufInfo rDeBufInfo(PORT_IMGO, rDequeBuf.mvOut.at(0).mBuffer, rDequeBuf.mvOut.at(0).mSize, rDequeBuf.mvOut.at(0).mVa, rDequeBuf.mvOut.at(0).mPa);
    #else
        BufInfo rDeBufInfo(PORT_IMGO, NULL, vDequeMem.at(0).size, vDequeMem.at(0).virtAddr, vDequeMem.at(0).phyAddr);
    #endif
        halCamIOOutQBuf.mvOut.push_back(rDeBufInfo);

    #ifdef USE_IMAGEBUF_HEAP
        //do nothing
    #else
        printf("[iopipetest] dequeBuf.PA(0x%x)VA(0x%x)size(%d)\n", halCamIOOutQBuf.mvOut.at(0).mPa, halCamIOOutQBuf.mvOut.at(0).mVa, halCamIOOutQBuf.mvOut.size());
    #endif
        //
        printf("[iopipetest] dequeBuf count(%d)\n", count);
        if(!mpCamIO->deque(halCamIOOutQBuf))
        {
            printf("[iopipetest] deque failed!!\n");
            goto TEST_EXIT;
        }
    #ifdef USE_IMAGEBUF_HEAP
        printf("[iopipetest] dequeBuf.PA(0x%x)VA(0x%x)buf(0x%x)\n", halCamIOOutQBuf.mvOut.at(0).mBuffer->getBufPA(0), halCamIOOutQBuf.mvOut.at(0).mBuffer->getBufVA(0), halCamIOOutQBuf.mvOut.at(0).mBuffer);
    #else
        printf("[iopipetest] dequeBuf.PA(0x%x)VA(0x%x)\n", halCamIOOutQBuf.mvOut.at(0).mPa, halCamIOOutQBuf.mvOut.at(0).mVa);
    #endif
        /* * * * * * * * * * * *
         * get ResultMetadata
         * * * * * * * * * * * */
        ResultMetadata result = halCamIOOutQBuf.mvOut.at(0).mMetaData;
        /* * * * * * * * * * * *
         * check dummy frame
         * * * * * * * * * * * */
        if(halCamIOOutQBuf.mvOut.at(0).mMetaData.m_bDummyFrame == MTRUE)
        {
            printf("[iopipetest] this is a dummy frame\n");
        }
        /* * * * * * * * * * * *
         * sequential buffer
         *(no replace buffer)
         * * * * * * * * * * * */
        if(nReplace != 1)
        {
            rEnBuf.mvOut.clear();
            BufInfo rBufInfo(PORT_IMGO,
#ifdef USE_IMAGEBUF_HEAP
                             halCamIOOutQBuf.mvOut.at(0).mBuffer,
#else
                             NULL,
#endif
                             halCamIOOutQBuf.mvOut.at(0).mSize,
                             halCamIOOutQBuf.mvOut.at(0).mVa,
                             halCamIOOutQBuf.mvOut.at(0).mPa);

            rEnBuf.mvOut.push_back(rBufInfo);
        }
        /* * * * * * * * * * * *
         * buffer per frame
         *(replace buffer)
         * * * * * * * * * * * */
        else
        {
            rEnBuf.mvOut.clear();
        #ifdef USE_IMAGEBUF_HEAP
            BufInfo rBufInfo(PORT_IMGO, rRawBuf.mvOut.at(BufIdx).mBuffer, rRawBuf.mvOut.at(BufIdx).mSize, rRawBuf.mvOut.at(BufIdx).mVa, rRawBuf.mvOut.at(BufIdx).mPa);
        #else
            BufInfo rBufInfo(PORT_IMGO, NULL, vRawMem.at(BufIdx).size, vRawMem.at(BufIdx).virtAddr, vRawMem.at(BufIdx).phyAddr);
        #endif
            rEnBuf.mvOut.push_back(rBufInfo);
        }

    #ifdef USE_IMAGEBUF_HEAP
        printf("[iopipetest] enqueBuf.PA(0x%x)VA(0x%x),BufId(%d)\n", rEnBuf.mvOut.at(0).mBuffer->getBufPA(0), rEnBuf.mvOut.at(0).mBuffer->getBufVA(0), BufIdx);
    #else
        printf("[iopipetest] enqueBuf.PA(0x%x)VA(0x%x),BufId(%d)\n", rEnBuf.mvOut.at(0).mPa, rEnBuf.mvOut.at(0).mVa, BufIdx);
    #endif
        /* * * * * * * * * * * *
         * setting before enque
         * * * * * * * * * * * */
        rEnBuf.mvOut.at(0).FrameBased.mMagicNum_tuning = 0;
        rEnBuf.mvOut.at(0).FrameBased.mDstSize = mDstSize;
        rEnBuf.mvOut.at(0).FrameBased.mCropRect = MRect(MPoint(0, 0), mDstSize);//no crop
        //
        printf("[iopipetest] enque count(%d)\n", count);
        if(!mpCamIO->enque(rEnBuf))
        {
            printf("[iopipetest] enque failed!!\n");
            goto TEST_EXIT;
        }
        //
        if(++BufIdx > BUF_NUM) BufIdx = 0;
        //
    } while(--count > 0);
#endif

    /*------------------------------
    *    (7). Stop
    *-----------------------------*/
    printf("[iopipetest] mpCamIO->stop\n");
    mpCamIO->stop();

TEST_EXIT:
    /*------------------------------
    *    (8). uninit
    *-----------------------------*/
    printf("[iopipetest] mpCamIO->uninit\n");
    if( !mpCamIO->uninit() )
    {
        MY_LOGE("uninit failed");
    }

    /*------------------------------
    *    (9). destory instance
    *-----------------------------*/
    printf("[iopipetest] mpCamIO->destroyInstance\n");
    mpCamIO->destroyInstance(&Name);
    mpCamIO = NULL;



    printf("[iopipetest] X\n");
    return 0;
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main_iopipe(int testcaseType,int testcaseNum)
{
    int ret = 0;
    int loopcount = 0;
    pTestUT->mode = testcaseNum;

    /*----------------------------
            IMEM
    ----------------------------*/
    printf("[Main] IMemDrv::createInstance\n");
    pTestUT->mpIMemDrv =  IMemDrv::createInstance();
    if (NULL == pTestUT->mpIMemDrv)
    {
        printf("[Main] g_pIMemDrv is NULL\n");
        MY_LOGE("g_pIMemDrv is NULL");
        return 0;
    }
    printf("[Main] g_pIMemDrv->init\n");
    if ( !pTestUT->mpIMemDrv->init() )
    {
        printf("[Main] g_pIMemDrv->init fail\n");
        MY_LOGE("g_pIMemDrv init fail");
    }
    /*----------------------------
            Test Items
    ----------------------------*/
    while(testcaseType != 1)
    {
        switch(testcaseType)
        {
            case 0://pass1
                printf("[Main] testcaseType(%d)\n", testcaseType);
                printf("[Main] call main_SetSensor_init\n");
                /* get u4SensorWidth, u4SensorHeight*/
                ret = pTestUT->main_SetSensor_init();
                if(ret < 0)
                    break;
                printf("[Main] main_SetSensor_uninit\n");
                pTestUT->main_SetSensor_uninit();

                /* enter loop count*/
                while(loopcount<=0){
                    printf("*****************************************\n");
                    printf("please deque/enque loop times(must >0)\n");
                    scanf("%d", &loopcount);
                    printf("*****************************************\n");
                }

                printf("[Main] main_CamIO_ZSD\n");
                pTestUT->main_ts_IOPipe_ZSD(loopcount);
                break;
            default:
                break;
        }
        /* Get next testitem */
        printf("*****************************************\n");
        printf("please input the test item,\n0: camio_zsd;\nelse:");
#ifdef _USE_THREAD_QUE_
            printf("exit enque+deque thread\n");
#else
            printf("exit\n");
#endif
            printf("*****************************************\n");
            scanf("%d", &testcaseType);
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

