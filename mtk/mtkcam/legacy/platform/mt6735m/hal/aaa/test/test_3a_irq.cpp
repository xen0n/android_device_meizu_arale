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

#define LOG_TAG "test3a"
//
#include <stdio.h>
#include <stdlib.h>
//
#include <aaa_log.h>

#include <IEventIrq.h>
#include <mtkcam/hal/IHalSensor.h>
#include <mtkcam/v3/hal/aaa_hal_common.h>
#include <mtkcam/v3/hal/IHal3A.h>
#include "aaa_result.h"

#include <semaphore.h>
#include <utils/threads.h>
#include <mtkcam/metadata/ITemplateRequest.h>
#include <hardware/camera3.h>
#include <system/camera_metadata.h>
#include <mtkcam/v3/hal/mtk_platform_metadata_tag.h>
#include <mtkcam/drv_common/isp_reg.h>

//
//
using namespace NSCam;
using namespace android;
/*******************************************************************************
*   test IEvenIrq
********************************************************************************/
class TestIrq
{
public:
    TestIrq()
    {
        MY_LOG("[%s]", __FUNCTION__);
    }

    ~TestIrq()
    {
        ::pthread_join(m_Thread, NULL);
        ::sem_destroy(&m_SemDone);
        m_prIrq->destroyInstance(LOG_TAG);
        MY_LOG("[%s] -", __FUNCTION__);
    }

    MVOID run()
    {
        MY_LOG("[%s] +", __FUNCTION__);
        m_prIrq = NS3Av3::IEventIrq::createInstance(NS3Av3::IEventIrq::ConfigParam(0, 0, 0), LOG_TAG);
        ::sem_init(&m_SemDone, 0, 0);
        ::pthread_create(&m_Thread, NULL, threadLoop, this);
        ::sem_wait(&m_SemDone);
        MY_LOG("[%s] -", __FUNCTION__);
    }

private:
    static MVOID* threadLoop(MVOID* arg)
    {
        TestIrq* _this = reinterpret_cast<TestIrq*>(arg);
        MINT32 i = 0;
        MY_LOG("[%s] +", __FUNCTION__);
        while (i < 30)
        {
            MY_LOG("[%s] (%d)", __FUNCTION__, i);
            _this->calculate();
            NS3Av3::IEventIrq::Duration rDuration;
            MINT32 i4EventCnt = _this->m_prIrq->wait(rDuration);
            MY_LOG("[%s] (%d, %d, %d)", __FUNCTION__, i4EventCnt, rDuration.i4Duration0, rDuration.i4Duration1);
            _this->apply();
            i ++;
        }
        MY_LOG("[%s] -", __FUNCTION__);
        ::sem_post(&_this->m_SemDone);
        return NULL;
    }

    MINT32 calculate()
    {
        MY_LOG("[%s]+", __FUNCTION__);
        MINT32 i;
        MFLOAT dTest = 3e20;
        for (i = 0; i < 50; i++)
        {
            dTest = dTest / ((MFLOAT)i+1);
            MY_LOG("[%s] %d:dTest(%f)", __FUNCTION__, i, dTest);
        }
        m_dTest = dTest;
        MY_LOG("[%s]- dTest(%f)", __FUNCTION__, dTest);
        return 0;
    }

    MINT32 apply()
    {
        MY_LOG("[%s] m_dTest(%f)", __FUNCTION__, m_dTest);
        m_prIrq->mark();
        return 0;
    }

    MFLOAT              m_dTest;
    NS3Av3::IEventIrq*  m_prIrq;
    pthread_t           m_Thread;
    sem_t               m_SemDone;
};
/*******************************************************************************
*   test IHal3A
********************************************************************************/
class Test3A : public NS3Av3::IHal3ACb
{
public:
    Test3A() : m_pIHal3A(NULL), m_fgPrecap(0)
    {
        MY_LOG("[%s] this(%p)", __FUNCTION__, this);
    }

    MBOOL initialize()
    {
        if (onInit())
        {
            m_Request.u4MagicNum = 0;
            m_Request.metadata = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_PREVIEW);
            m_Controls.push_back(m_Request);
            m_Request.u4MagicNum = 1;
            m_Controls.push_back(m_Request);
            m_Request.u4MagicNum = 2;
            m_Controls.push_back(m_Request);

            m_pIHal3A->setSensorMode(NS3Av3::ESensorMode_Preview);
            // set
            m_pIHal3A->set(m_Controls);
            return MTRUE;
        }
        return MFALSE;
    }

    MBOOL destroy()
    {
        return onUninit();
    }

    void run(MINT32 u4Step)
    {
        MINT32 i = 0;

        while (i < u4Step)
        {
            ::usleep(33333);
            ::sem_wait(&m_FakeP1Done);
            deque(i);
            i ++;
        }
        m_pIHal3A->stop();
    }

    virtual void doNotifyCb (
                            MINT32  _msgType,
                            MINTPTR _ext1,
                            MINTPTR _ext2,
                            MINTPTR _ext3
                        )
    {
        MY_LOG("[%s]+", __FUNCTION__);
        if (_msgType == NS3Av3::IHal3ACb::eID_NOTIFY_3APROC_FINISH)
        {
            ::sem_post(&m_FakeP1Done);
            //MY_LOG("[%s]+2", __FUNCTION__);
            MINT32 magic  = reinterpret_cast<MINT32>(_ext1);
            MINT32 status = reinterpret_cast<MINT32>(_ext2);
            // enque
            enque(magic);
            // remove old
            m_Controls.erase(m_Controls.begin());
            // aquire new request
            m_Request.u4MagicNum = magic+1;
            if (m_fgPrecap == 2)
            {
                m_fgPrecap = 0;
                m_Request.metadata = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_STILL_CAPTURE);
            }
            else
            {
                m_Request.metadata = m_pTemplate->getMtkData(CAMERA3_TEMPLATE_PREVIEW);
            }
            if (magic == 15)
            {   // emulate precapture trigger
                NS3Av3::UPDATE_ENTRY_SINGLE(m_Request.metadata, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, (MUINT8)MTK_CONTROL_AE_PRECAPTURE_TRIGGER_START);
            }
            else
            {
                NS3Av3::UPDATE_ENTRY_SINGLE(m_Request.metadata, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, (MUINT8)MTK_CONTROL_AE_PRECAPTURE_TRIGGER_IDLE);
            }
            NS3Av3::UPDATE_ENTRY_SINGLE(m_Request.metadata, MTK_STATISTICS_LENS_SHADING_MAP_MODE, (MUINT8)MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON);
            m_Controls.push_back(m_Request);
            // set
            m_pIHal3A->set(m_Controls);
        }
    }

private:
    MBOOL powerOnSensor()
    {
        MY_LOG("[%s] +", __FUNCTION__);
        MBOOL    ret = MFALSE;
        //  (1) Open Sensor
        NSCam::IHalSensorList* pHalSensorList = NSCam::IHalSensorList::get();
        if (!pHalSensorList)
        {
            MY_ERR("pHalSensorList == NULL");
            goto lbExit;
        }
        //
        pHalSensorList->searchSensors();
        m_pHalSensor = pHalSensorList->createSensor(
                                            LOG_TAG,
                                            0);
        if (m_pHalSensor == NULL)
        {
            MY_ERR("m_pHalSensor is NULL");
            goto lbExit;
        }
        //
        m_sensorIdx = 0;
        if( !m_pHalSensor->powerOn(LOG_TAG, 1, &m_sensorIdx) )
        {
            MY_ERR("sensor power on failed: %d", m_sensorIdx);
            goto lbExit;
        }
        //
        ret = MTRUE;
    lbExit:
        MY_LOG("[%s] -", __FUNCTION__);
        return ret;
    }
    MBOOL onInit()
    {
        MY_LOG("[%s]+", __FUNCTION__);
        ::sem_init(&m_FakeP1Done, 0, 0);
        if (!powerOnSensor())
        {
            MY_ERR("Fail to power on sensor");
            return MFALSE;
        }

        m_pIHal3A = NS3Av3::IHal3A::createInstance(NS3Av3::IHal3A::E_Camera_3, 0, LOG_TAG);
        if (!m_pIHal3A)
        {
            MY_ERR("Fail to create IHal3A");
            return MFALSE;
        }
        m_pIHal3A->attachCb(NS3Av3::IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);

        m_pIHal3A->start();

        m_pTemplate = ITemplateRequest::getInstance(0);

        MY_LOG("[%s]-", __FUNCTION__);
        return MTRUE;
    }
    MBOOL onUninit()
    {
        if (m_pHalSensor)
        {
            if( !m_pHalSensor->powerOff(LOG_TAG, 1, &m_sensorIdx) )
            {
                MY_ERR("sensor power off failed: %d", m_sensorIdx);
                return MFALSE;
            }
            m_pHalSensor = NULL;
        }

        if (m_pIHal3A)
        {
            m_pIHal3A->detachCb(NS3Av3::IHal3ACb::eID_NOTIFY_3APROC_FINISH, this);
            m_pIHal3A->destroyInstance(LOG_TAG);
            m_pIHal3A = NULL;
        }

        ::sem_destroy(&m_FakeP1Done);
        return MTRUE;
    }
    MBOOL enque(MINT32 i4Magic)
    {
        MY_LOG("[%s] i4Magic(%d)", __FUNCTION__, i4Magic);
        return MTRUE;
    }
    MBOOL deque(MINT32 i4Magic)
    {
        MY_LOG("[%s] i4Magic(%d)", __FUNCTION__, i4Magic);
        NS3Av3::MetaSet_T result;
        IMetadata result2;
        isp_reg_t regBuf;
        m_pIHal3A->get(i4Magic, result);

        MUINT8 u1AeState, u1AwbState, u1AfState, u1AePrecap;
        NSIspTuning::RAWIspCamInfo_U rCamInfo;
        NS3Av3::QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AE_PRECAPTURE_TRIGGER, u1AePrecap);
        NS3Av3::QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AE_STATE, u1AeState);
        NS3Av3::QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AF_STATE, u1AfState);
        NS3Av3::QUERY_ENTRY_SINGLE(result.appMeta, MTK_CONTROL_AWB_STATE, u1AwbState);
        if (u1AePrecap)
        {
            m_fgPrecap = 1;
        }
        if (m_fgPrecap == 1 && ((u1AeState == MTK_CONTROL_AE_STATE_CONVERGED) || (u1AeState == MTK_CONTROL_AE_STATE_FLASH_REQUIRED)))
        {
            // cap
            m_fgPrecap = 2;
        }

        MY_LOG("[%s] AE(%d), AF(%d), AWB(%d)", __FUNCTION__, u1AeState, u1AfState, u1AwbState);
        MUINT8 u1ShadingMapMode = 0;
        if (NS3Av3::QUERY_ENTRY_SINGLE(result.appMeta, MTK_STATISTICS_LENS_SHADING_MAP_MODE, u1ShadingMapMode))
        {
            if (u1ShadingMapMode == MTK_STATISTICS_LENS_SHADING_MAP_MODE_ON)
            {
                MFLOAT rMap[4] = {0.0f};
                if (NS3Av3::GET_ENTRY_ARRAY(result.appMeta, MTK_STATISTICS_LENS_SHADING_MAP, rMap, 4))
                {
                    MY_LOG("[%s] {%3.3f, %3.3f, %3.3f, %3.3f}", __FUNCTION__, rMap[0], rMap[1], rMap[2], rMap[3]);
                }
                else
                {
                    MY_ERR("Fail to get shading map");
                }
            }
        }
        if (NS3Av3::GET_ENTRY_ARRAY(result.halMeta, MTK_PROCESSOR_CAMINFO, rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U)))
        {
            MY_LOG("rCamInfo.u4Id(%d)", rCamInfo.u4Id);
            MY_LOG("rCamInfo.eIspProfile(%d)", rCamInfo.eIspProfile);
            MY_LOG("rCamInfo.eSensorMode(%d)", rCamInfo.eSensorMode);
            MY_LOG("rCamInfo.eIdx_Scene(%d)", rCamInfo.eIdx_Scene);
            MY_LOG("rCamInfo.u4ISOValue(%d)", rCamInfo.u4ISOValue);
            MY_LOG("rCamInfo.eIdx_ISO(%d)", rCamInfo.eIdx_ISO);
            MY_LOG("rCamInfo.eIdx_PCA_LUT(%d)", rCamInfo.eIdx_PCA_LUT);
            MY_LOG("rCamInfo.eIdx_CCM(%d)", rCamInfo.eIdx_CCM);
            MY_LOG("rCamInfo.eIdx_Shading_CCT(%d)", rCamInfo.eIdx_Shading_CCT);
            MY_LOG("rCamInfo.i4ZoomRatio_x100(%d)", rCamInfo.i4ZoomRatio_x100);
            MY_LOG("rCamInfo.i4LightValue_x10(%d)", rCamInfo.i4LightValue_x10);
        }
        m_pIHal3A->setIsp(MFALSE, result, (void*)&regBuf, &result2);
        return MTRUE;
    }

    MUINT                       m_sensorIdx;
    NS3Av3::IHal3A*             m_pIHal3A;
    NSCam::IHalSensor*          m_pHalSensor;
    NS3Av3::MetaInfo_T          m_Request;
    List<NS3Av3::MetaInfo_T>    m_Controls;
    ITemplateRequest*           m_pTemplate;
    sem_t                       m_FakeP1Done;
    MINT32                      m_fgPrecap;
};

/*******************************************************************************
*
********************************************************************************/
static void usage()
{
    MY_LOG("Usage: test_3a <0: normal preview, 1: pip preview>\n");
}

static void TestCamInfo()
{
    NSIspTuning::RAWIspCamInfo_U rCamInfo;
    rCamInfo.eIspProfile = NSIspTuning::EIspProfile_NormalPreview;
    rCamInfo.eSensorMode = NSIspTuning::ESensorMode_Capture;
    rCamInfo.eIdx_Scene = MTK_CONTROL_SCENE_MODE_DISABLED;
    rCamInfo.u4ISOValue = 200;
    rCamInfo.eIdx_ISO = NSIspTuning::eIDX_ISO_200;
    rCamInfo.eIdx_PCA_LUT = NSIspTuning::eIDX_PCA_MIDDLE;
    rCamInfo.eIdx_CCM = NSIspTuning::eIDX_CCM_A;
    rCamInfo.eIdx_Shading_CCT = NSIspTuning::eIDX_Shading_CCT_CWF;
    rCamInfo.i4ZoomRatio_x100 = 300;
    rCamInfo.i4LightValue_x10 = 1000;

    IMetadata m;
    NS3Av3::UPDATE_ENTRY_ARRAY(m, MTK_PROCESSOR_CAMINFO, rCamInfo.data, sizeof(NSIspTuning::RAWIspCamInfo_U));

    NSIspTuning::RAWIspCamInfo_U rCamInfo2;
    if (NS3Av3::GET_ENTRY_ARRAY(m, MTK_PROCESSOR_CAMINFO, rCamInfo2.data, sizeof(NSIspTuning::RAWIspCamInfo_U)))
    {
        MY_LOG("TestCamInfo");
        MY_LOG("%d, %d", rCamInfo.eIspProfile, rCamInfo2.eIspProfile);
        MY_LOG("%d, %d", rCamInfo.eSensorMode, rCamInfo2.eSensorMode);
        MY_LOG("%d, %d", rCamInfo.eIdx_Scene, rCamInfo2.eIdx_Scene);
        MY_LOG("%d, %d", rCamInfo.u4ISOValue, rCamInfo2.u4ISOValue);
        MY_LOG("%d, %d", rCamInfo.eIdx_ISO, rCamInfo2.eIdx_ISO);
        MY_LOG("%d, %d", rCamInfo.eIdx_PCA_LUT, rCamInfo2.eIdx_PCA_LUT);
        MY_LOG("%d, %d", rCamInfo.eIdx_CCM, rCamInfo2.eIdx_CCM);
        MY_LOG("%d, %d", rCamInfo.eIdx_Shading_CCT, rCamInfo2.eIdx_Shading_CCT);
        MY_LOG("%d, %d", rCamInfo.i4ZoomRatio_x100, rCamInfo2.i4ZoomRatio_x100);
        MY_LOG("%d, %d", rCamInfo.i4LightValue_x10, rCamInfo2.i4LightValue_x10);
    }
}

/*******************************************************************************
*  Main Function
********************************************************************************/
int main(int argc, char** argv)
{
    int ret = 0;

    if ( argc < 2 )
    {
        usage();
        return -1;
    }
    //
    MY_LOG("camtest start");
    #if 0

    TestIrq testIrq;
    testIrq.run();

    #else
    TestCamInfo();

    Test3A test3a;
    if (test3a.initialize())
    {
        test3a.run(30);
    }
    test3a.destroy();
    #endif
    MY_LOG("camtest end");
    //
    return ret;
}

