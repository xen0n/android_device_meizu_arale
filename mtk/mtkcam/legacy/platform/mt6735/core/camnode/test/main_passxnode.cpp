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

#define LOG_TAG "CamNodeTest"

#include <stdio.h>
#include <stdlib.h>
//
#include <errno.h>
#include <fcntl.h>

#include <mtkcam/common.h>
#include <mtkcam/camnode/pass1node.h>
#include <mtkcam/camnode/pass2node.h>
#include <mtkcam/camnode/test/testNodes.h>
#include <mtkcam/camnode/AllocBufHandler.h>
//
//
#include <mtkcam/hal/IHalSensor.h>
/*******************************************************************************
*  Main Function
********************************************************************************/
int test_pass1node(int argc, char** argv)
{
    int ret = 0;

    if( argc < 3 )
    {
        printf("Usage: camnodetest 1(pass1 node) <sensorId> <scenario>\n");
        return -1;
    }

#if 1
    printf("NODETEST sensor list get\n");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    printf("NODETEST sensor search\n");
    pHalSensorList->searchSensors();
    printf("NODETEST search sensor end\n");
#endif

    MUINT32 sensorIdx = atoi(argv[1]);
    MUINT32 scenario  = atoi(argv[2]); //prv:0, cap:1

    using namespace NSCamNode;
    ICamGraph* g = ICamGraph::createInstance(sensorIdx, "testpass1");

    AllocBufHandler* pBufhdl = AllocBufHandler::createInstance();
    pBufhdl->init();

    g->setBufferHandler( PASS1_FULLRAW, pBufhdl );
    g->setBufferHandler( PASS1_RESIZEDRAW, pBufhdl );

    Pass1NodeInitCfg p1initcfg(scenario, 3, 10, 0);
    Pass1Node    * pass1 = Pass1Node::createInstance(p1initcfg);
    TestDumpNode * dump  = TestDumpNode::createInstance();

    g->connectData( PASS1_FULLRAW    , PASS1_FULLRAW    , pass1 , dump );
    g->connectData( PASS1_RESIZEDRAW , PASS1_RESIZEDRAW , pass1 , dump );

    printf("NODETEST g init\n");
    g->init();

    printf("NODETEST g start\n");
    g->start();

    printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
    getchar();

    printf("NODETEST g stop\n");
    g->stop();

    printf("NODETEST g uninit\n");
    g->uninit();

    printf("NODETEST g disconnect\n");
    g->disconnect();

    printf("NODETEST node uninit\n");
    pass1->destroyInstance();
    dump->destroyInstance();

    printf("bufhdl uninit\n");
    pBufhdl->uninit();
    printf("NODETEST g destroy\n");
    g->destroyInstance();
    printf("NODETEST bufhdl destroy\n");
    pBufhdl->destroyInstance();
    printf("NODETEST end");

    return ret;
}


int test_pass2node(int argc, char** argv)
{
    int ret = 0;

    if( argc < 2 )
    {
        printf("Usage: camnodetest 2(pass2 node) <dst_fmt>\n");
        printf("dst_fmt: 0 (YUY2), 1(YV12)\n");
        return -1;
    }

    MUINT32 dstfmt = atoi(argv[1]);
    EImageFormat outfmt;
    switch(dstfmt)
    {
        case 0:
            outfmt = eImgFmt_YUY2;
            printf("dst: YUY2\n");
            break;
        case 1:
            outfmt = eImgFmt_YV12;
            printf("dst: YV12\n");
            break;
        default:
            printf("not support dstfmt(0x%x)\n", dstfmt);
            return -1;
            break;
    }

#if 1
    printf("NODETEST sensor list get\n");
    IHalSensorList* const pHalSensorList = IHalSensorList::get();

    printf("NODETEST sensor search\n");
    pHalSensorList->searchSensors();
    printf("NODETEST search sensor end\n");
#endif

    MUINT32 sensorIdx = 0;
    using namespace NSCamNode;
    ICamGraph* g = ICamGraph::createInstance(sensorIdx, "testpass2");

    AllocBufHandler* pBufhdl = AllocBufHandler::createInstance();
    pBufhdl->init();

    //assign bufMgr & buffer handler
    g->setBufferHandler( PASS1_FULLRAW    , pBufhdl );
    g->setBufferHandler( PASS1_RESIZEDRAW , pBufhdl );
    g->setBufferHandler( PASS2_PRV_DST_0  , pBufhdl );

    //alloc a buffer for pass2 out
    AllocInfo allocinfo(320, 320, outfmt,
                        eBUFFER_USAGE_SW_MASK | eBUFFER_USAGE_HW_MASK );
    pBufhdl->requestBuffer( PASS2_PRV_DST_0, allocinfo );

    MUINT32 scenario = 0;
    Pass1NodeInitCfg p1initcfg(scenario, 3, 10, 0);
    Pass1Node    * pass1 = TestPass1Node::createInstance(p1initcfg);
    Pass2Node    * pass2 = Pass2Node::createInstance(PASS2_PREVIEW);
    TestDumpNode * dump  = TestDumpNode::createInstance();

    printf("NODETEST g connect\n");
    g->connectData( PASS1_FULLRAW    , PASS2_PRV_SRC    , pass1 , pass2 );
    g->connectData( PASS2_PRV_DST_0  , PASS2_PRV_DST_0    , pass2 , dump );

    printf("NODETEST g init\n");
    g->init();

    printf("NODETEST g start\n");
    g->start();

    printf("!!!!!!!! press enter to stop... !!!!!!!!\n");
    getchar();

    printf("NODETEST g stop\n");
    g->stop();

    printf("NODETEST g uninit\n");
    g->uninit();

    printf("NODETEST g disconnect\n");
    g->disconnect();

    pass1->destroyInstance();
    pass2->destroyInstance();
    dump->destroyInstance();

    pBufhdl->uninit();

    g->destroyInstance();
    pBufhdl->destroyInstance();
    printf("NODETEST end");

    return ret;
}
