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

#define LOG_TAG "testCtrlNode"

#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>
//
#include <camera/MtkCamera.h>
#include <CamUtils.h>
//
#include <mtkcam/v1/camutils/CamFormat.h>
#include <mtkcam/v1/IParamsManager.h>
using namespace android;
using namespace MtkCamUtils;
using namespace NSCam::Utils;
//
#include <mtkcam/featureio/IHal3A.h>
using namespace NS3A;
//
#include <mtkcam/common.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
#include <mtkcam/camnode/AllocBufHandler.h>
using namespace NSCamNode;
//
#include <CamNode/DefaultCtrlNode.h>
#include <CamNode/DefaultBufHandler.h>
//
#include <mtkcam/camnode/test/testNodes.h>
//
#include <mtkcam/hal/IHalSensor.h>
//
using namespace NSCam;
//
#if defined(__func__)
#undef __func__
#endif
#define __func__ __FUNCTION__
//
#define MY_LOGV(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGD(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGI(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGW(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGE(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGA(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
#define MY_LOGF(fmt, arg...)        printf("[%s/%s] " fmt"\n", LOG_TAG, __func__, ##arg)
//
#define USE_DEFAULT_BUF_HANDLER     (1)
#define PASS2_PRV_DST_0_BUF_NUM     (3)
#define PASS2_PRV_DST_0_FORMAT      ("yuv422i-yuyv")
#define PASS2_PRV_DST_0_WIDTH       (400)
#define PASS2_PRV_DST_0_HEIGHT      (300)
//
class TestBuffer : public IImgBuf
{
/******************************************************************************
*   Inheritance from IMemBuf.
*******************************************************************************/
public:
    virtual int64_t             getTimestamp() const                    { return mTimestamp; }
    virtual void                setTimestamp(int64_t const timestamp)   { mTimestamp = timestamp; }
    virtual size_t              getBufSize() const                      { return mBufSize; }
    virtual void*               getVirAddr() const                      { return (void*)mVA; }
    virtual void*               getPhyAddr() const                      { return (void*)mPA; }
    virtual const char*         getBufName() const                      { return mName; }
    virtual int                 getIonFd() const                        { return mMemId; }


/******************************************************************************
*   Inheritance from IImgBuf.
*******************************************************************************/
public:
    virtual String8 const&      getImgFormat()      const               { return mFormat; }
    virtual uint32_t            getImgWidthStride(
                                    uint_t const uPlaneIndex = 0
                                )   const
                                {
                                    return  FmtUtils::queryImgWidthStride(getImgFormat(), getImgWidth(), uPlaneIndex);
                                }

    virtual uint32_t            getImgWidth()       const               { return mWidth;  }
    virtual uint32_t            getImgHeight()      const               { return mHeight; }
    virtual uint32_t            getBitsPerPixel()   const               { return Format::queryImageBitsPerPixel(Utils::Format::queryImageFormat(getImgFormat().string())); }


/******************************************************************************
*   Initialization.
*******************************************************************************/
public:
    TestBuffer(
        uint32_t _w = 0,
        uint32_t _h = 0,
        uint32_t _bufsize = 0,
        uint32_t _va = 0,
        uint32_t _pa = 0,
        uint32_t _memid = 0,
        String8  _format = String8(""),
        const char* _name = "")
        : IImgBuf()
        , mWidth(_w)
        , mHeight(_h)
        , mBufSize(_bufsize)
        , mVA(_va)
        , mPA(_pa)
        , mMemId(_memid)
        , mTimestamp(0)
        , mFormat(_format)
        , mName(_name)
    {
        MY_LOGD("TestBuffer:W(%d),H(%d),BS(%d),VA(0x%08X),PA(0x%08X),MID(%d),F(%s),N(%s)",
                mWidth,
                mHeight,
                mBufSize,
                mVA,
                mPA,
                mMemId,
                mFormat.string(),
                mName);
    }

    virtual                     ~TestBuffer() {};
    bool                        operator!() const   { return mVA != 0; }

/******************************************************************************
*   Initialization.
*******************************************************************************/
private:
    uint32_t        mWidth;
    uint32_t        mHeight;
    uint32_t        mBufSize;
    uint32_t        mVA;
    uint32_t        mPA;
    int32_t         mMemId;
    int64_t         mTimestamp;
    String8         mFormat;
    const char*     mName;
};

//
/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;

    //if( argc < 2 )
    //{
    //    MY_LOGD("Usage: camnodetest 1(pass1 node) <sensorId> <scenario>");
    //    return -1;
    //}
    MUINT32 sensorId = atoi(argv[1]);
    MUINT32 scenario = atoi(argv[2]); //prv:0, cap:1

    MY_LOGD("Sensor:list get");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();
    MY_LOGD("Sensor:search");
    pHalSensorList->searchSensors();
    MY_LOGD("ICamGraph:createInstance");
    ICamGraph* g = ICamGraph::createInstance(sensorId, "testCtrlNode");
    MY_LOGD("AllocBufHandler:createInstance");
    AllocBufHandler* pBufhdl = AllocBufHandler::createInstance();
    MY_LOGD("AllocBufHandler:init");
    pBufhdl->init();

    #if USE_DEFAULT_BUF_HANDLER
    //
    MUINT32 i, plane, imgFmt;
    IImageBufferAllocator* allocator = IImageBufferAllocator::getInstance();
    IImageBuffer* pIIMageBUffer[PASS2_PRV_DST_0_BUF_NUM];
    sp<TestBuffer> spTestBuffer[PASS2_PRV_DST_0_BUF_NUM];
    //allocate buffer
    MUINT32 bufStridesInBytes[3] = { 0, 0, 0};
    imgFmt = Utils::Format::queryImageFormat(PASS2_PRV_DST_0_FORMAT);
    plane = Format::queryPlaneCount(imgFmt);
    //
    for(i=0; i<plane; i++)
    {
        bufStridesInBytes[i] = Format::queryPlaneBitsPerPixel(imgFmt, i)*FmtUtils::queryImgWidthStride(PASS2_PRV_DST_0_FORMAT, PASS2_PRV_DST_0_WIDTH, i);
    }
    //
    MINT32 bufBoundaryInBytes[3] = {0, 0, 0};
    //
    IImageBufferAllocator::ImgParam imgParam(
                                        imgFmt,
                                        MSize(PASS2_PRV_DST_0_WIDTH, PASS2_PRV_DST_0_HEIGHT),
                                        bufStridesInBytes,
                                        bufBoundaryInBytes,
                                        plane);
    //
    for(i=0; i<PASS2_PRV_DST_0_BUF_NUM; i++)
    {
        MY_LOGD("IImageBufferAllocator:alloc_ion:i(%d)",i);
        pIIMageBUffer[i] = allocator->alloc_ion(LOG_TAG, imgParam);
        if(pIIMageBUffer[i] == 0)
        {
            MY_LOGE("NULL Buffer");
            return  MFALSE;
        }
        //
        MY_LOGD("IImageBufferAllocator:lockBuf:i(%d)",i);
        if(!pIIMageBUffer[i]->lockBuf(LOG_TAG, eBUFFER_USAGE_HW_CAMERA_READWRITE | eBUFFER_USAGE_SW_READ_OFTEN))
        {
            MY_LOGE("IImageBufferAllocator:lockBuf i(%d) fail",i);
            return  MFALSE;
        }
        //
        spTestBuffer[i] = new TestBuffer(
                                PASS2_PRV_DST_0_WIDTH,
                                PASS2_PRV_DST_0_HEIGHT,
                                pIIMageBUffer[i]->getBufSizeInBytes(0) + pIIMageBUffer[i]->getBufSizeInBytes(1) + pIIMageBUffer[i]->getBufSizeInBytes(2),
                                pIIMageBUffer[i]->getBufVA(0),
                                pIIMageBUffer[i]->getBufPA(0),
                                pIIMageBUffer[i]->getFD(0),
                                String8(PASS2_PRV_DST_0_FORMAT),
                                "TestBuffer");
    }
    //
    sp<IImgBufQueue>            spImgBufQueue        = new ImgBufQueue(IImgBufProvider::eID_DISPLAY, "CameraTest@ImgBufQue");
    sp<ImgBufProvidersManager>  spImgBufProvidersMgr = new ImgBufProvidersManager;
    //
    if(spImgBufQueue == 0)
    {
        MY_LOGD("IImgBufQueue:Fail to new");
        return ret;
    }
    if(spImgBufProvidersMgr == 0)
    {
        MY_LOGD("ImgBufProvidersManager:Fail to new");
        return ret;
    }
    //
    MY_LOGD("IImgBufQueue:stopProcessor");
    spImgBufQueue->stopProcessor();
    //
    for(i=0; i<PASS2_PRV_DST_0_BUF_NUM; i++)
    {
        MY_LOGD("IImgBufQueue:enqueProcessor i(%d)",i);
        spImgBufQueue->enqueProcessor(ImgBufQueNode(spTestBuffer[i],ImgBufQueNode::eSTATUS_TODO));
    }
    //
    MY_LOGD("IImgBufQueue:startProcessor");
    if(!spImgBufQueue->startProcessor())
    {
        MY_LOGE("IImgBufQueue:startProcessor fail");
        return MFALSE;
    }
    //
    MY_LOGD("ImgBufProvidersManager:setProvider");
    spImgBufProvidersMgr->setProvider(
                            spImgBufQueue->getProviderId(),
                            spImgBufQueue);
    //
    DefaultBufHandler* pDefBufHdl = DefaultBufHandler::createInstance("TEST", spImgBufProvidersMgr);
    pDefBufHdl->init();
    #endif

    //assign bufMgr & buffer handler
    g->setBufferHandler(PASS1_FULLRAW,      pBufhdl);
    g->setBufferHandler(PASS1_RESIZEDRAW,   pBufhdl);

    #if USE_DEFAULT_BUF_HANDLER
    g->setBufferHandler(PASS2_PRV_DST_0,    pDefBufHdl);
    g->setBufferHandler(PASS2_PRV_DST_1,    pDefBufHdl);
    #else
    g->setBufferHandler(PASS2_PRV_DST_0,    pBufhdl);
    //alloc a buffer for pass2 out
    AllocInfo allocinfo(
                400,
                300,
                eImgFmt_YUY2,
                eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
    pBufhdl->requestBuffer(PASS2_PRV_DST_0, allocinfo);
    #endif


    Pass1NodeInitCfg p1initcfg(scenario, 3, 10);
    Pass1Node* pass1                    = Pass1Node::createInstance(p1initcfg);
    DefaultCtrlNode* defaultCtrlNode    = DefaultCtrlNode::createInstance("TEST",DefaultCtrlNode::CTRL_NODE_DEFAULT);
    Pass2Node* pass2                    = Pass2Node::createInstance(PASS2_PREVIEW);
    TestDumpNode* dump                  = TestDumpNode::createInstance();

    MY_LOGD("ICamGraph:connect");
    g->connectData(PASS1_FULLRAW,       CONTROL_FULLRAW,    pass1,              defaultCtrlNode );
    g->connectData(PASS1_RESIZEDRAW,    CONTROL_RESIZEDRAW, pass1,              defaultCtrlNode );
    g->connectData(CONTROL_PRV_SRC,     PASS2_PRV_SRC,      defaultCtrlNode,    pass2 );
    g->connectData(PASS2_PRV_DST_0,     PASS2_PRV_DST_0,    pass2,              dump );
    MY_LOGD("CamGraph:connectNotify");
    g->connectNotify(PASS1_EOF, pass1,  defaultCtrlNode);
    //
    MY_LOGD("ICamGraph:init");
    g->init();
    MY_LOGD("ICamGraph:start");
    g->start();

    #if USE_DEFAULT_BUF_HANDLER
    MUINT32 bufCnt = 0;
    while(bufCnt < PASS2_PRV_DST_0_BUF_NUM)
    {
        MY_LOGD("IImgBufQueue:dequeProcessor bufCnt(%d)",bufCnt);
        Vector<ImgBufQueNode> vQueNode;
        spImgBufQueue->dequeProcessor(vQueNode);
        MY_LOGD("IImgBufQueue:size(%d)",vQueNode.size());
        //
        for(i=0; i<vQueNode.size(); i++)
        {
            MY_LOGD("IImgBufQueue:i(%d),VA(0x%08X)",
                    i,
                    (MUINT32)(vQueNode[i].getImgBuf()->getVirAddr()));
        }
        //
        bufCnt += vQueNode.size();
    }
    //
    MY_LOGD("IImgBufQueue:stopProcessor");
    spImgBufQueue->stopProcessor();
    MY_LOGD("ImgBufProvidersManager:setProvider to NULL");
    spImgBufProvidersMgr->setProvider(spImgBufQueue->getProviderId(), NULL);
    spImgBufProvidersMgr = NULL;
    spImgBufQueue = NULL;
    //
    for(i=0; i<PASS2_PRV_DST_0_BUF_NUM; i++)
    {
        MY_LOGD("Clear TestBuffer,IImageBuffer,IImageBufferAllocator i(%d)",i);
        spTestBuffer[i] = NULL;
        pIIMageBUffer[i]->unlockBuf(LOG_TAG);
        allocator->free(pIIMageBUffer[i]);
    }
    #else
    MY_LOGD("STOP here!!!!!!!!!!!!!!");
    getchar();
    #endif

    MY_LOGD("ICamGraph:stop");
    g->stop();
    MY_LOGD("ICamGraph:uninit");
    g->uninit();
    MY_LOGD("ICamGraph:disconnect");
    g->disconnect();

    MY_LOGD("Pass1Node:destroyInstance");
    pass1->destroyInstance();

    MY_LOGD("DefaultCtrlNode:destroyInstance");
    defaultCtrlNode->destroyInstance();

    MY_LOGD("Pass2Node:destroyInstance");
    pass2->destroyInstance();

    MY_LOGD("TestDumpNode:destroyInstance");
    dump->destroyInstance();

    MY_LOGD("ICamGraph:destroyInstance");
    g->destroyInstance();

    MY_LOGD("AllocBufHandler:uninit");
    pBufhdl->uninit();
    MY_LOGD("AllocBufHandler:destroyInstance");
    pBufhdl->destroyInstance();

    MY_LOGD("DefaultBufHandler:uninit");
    pDefBufHdl->uninit();
    MY_LOGD("DefaultBufHandler:destroyInstance");
    pDefBufHdl->destroyInstance();

    MY_LOGD("NODETEST end");
    //
    return ret;
}
