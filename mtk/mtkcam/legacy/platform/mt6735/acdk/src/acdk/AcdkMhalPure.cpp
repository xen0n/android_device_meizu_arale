/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
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

//! \file  AcdkMhalPure.cpp

#define LOG_TAG "AcdkMhalPure"

#include <stdio.h>
#include <stdlib.h>
#include <linux/rtpm_prio.h>
#include <utils/threads.h>
#include <sys/prctl.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <vector>
#include <sys/resource.h>
#include <cutils/properties.h>

#include "mtkcam/acdk/AcdkTypes.h"
#include "AcdkLog.h"
#include "AcdkErrCode.h"
#include "mtkcam/acdk/AcdkCommon.h"
#include "AcdkCallback.h"
#include "AcdkSurfaceView.h"
#include "AcdkMhalBase.h"

using namespace NSAcdkMhal;

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/IPostProcPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>

using namespace NSImageio;
using namespace NSIspio;

//#include "mtkcam/hal/sensor_hal.h"\
#include "hal/IHalSensor.h"
#include "isp_function.h"
#include <mtkcam/drv/isp_drv.h>
#include <mtkcam/v1/config/PriorityDefs.h>

#include "AcdkMhalPure.h"

#define MEDIA_PATH "/sdcard"
#define ACDK_MT6582_MDP_WO_IRQ 1 //SL test ACDK_6582_MDP without IRQ don
//#ifndef USING_MTK_LDVT   // Not using LDVT.
//#define ACDK_CAMERA_3A
#define ALL_SENSOR_USE_TG1
//AcdkMhalPure#endif
/*******************************************************************************
* Global variable
*******************************************************************************/
static sem_t g_SemMainHigh, g_SemMainHighBack, g_SemMainHighEnd;
static pthread_t g_threadMainHigh;
static AcdkMhalPure *g_pAcdkMHalPureObj = NULL;
static acdkObserver g_acdkMhalObserver;
static stISP_BUF_INFO g_CookieVa;
static MUINT32 is_yuv_sensor = 0;

static vector<PortInfo const*> g_vCamIOInPorts(1);
static vector<PortInfo const*> g_vCamIOIOutPorts(1);
//static vector<PortInfo const*> g_vCdpInPorts(3);
//static vector<PortInfo const*> g_vCdpOutPorts(3);
static vector<PortInfo const*> g_vPostProcInPorts(3);
static vector<PortInfo const*> g_vPostProcOutPorts(3);

static MINT32 g_acdkMhalPureDebug = 0;

/*******************************************************************************
* AcdkMhalPure
* brif : Constructor
*******************************************************************************/
AcdkMhalPure::AcdkMhalPure()
{
    mAcdkMhalState = ACDK_MHAL_NONE;

    m_pICamIOPipe = NULL;
    m_pPostProcPipe = NULL;
#ifdef ACDK_CAMERA_3A
    m_p3AHal = NULL;   //cotta--temp for 82
#endif
    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

    mReadyForCap = MFALSE;

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    g_pAcdkMHalPureObj = this;
    #ifdef ACDK_CAMERA_3A
    sensorScenario = 0;
    mCurMagicNum = 0;
    #endif
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalSetState(acdkMhalState_e newState)
{
    Mutex::Autolock lock(mLock);

    ACDK_LOGD("Now(0x%04x), Next(0x%04x)", mAcdkMhalState, newState);

    if(newState == ACDK_MHAL_ERROR)
    {
        goto ACDK_MHAL_SET_STATE_EXIT;
    }

    switch(mAcdkMhalState)
    {
    case ACDK_MHAL_NONE:
        switch(newState)
        {
        case ACDK_MHAL_INIT:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_NONE");
            ACDK_LOGE("State error ACDK_MHAL_NONE");
            break;
        }
        break;
    case ACDK_MHAL_INIT:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_INIT");
            ACDK_LOGE("State error ACDK_MHAL_INIT");
            break;
        }
        break;
    case ACDK_MHAL_IDLE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_CAPTURE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_IDLE");
            ACDK_LOGE("State error ACDK_MHAL_IDLE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_PREVIEW:
        case ACDK_MHAL_PRE_CAPTURE:
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PREVIEW");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW");
            break;
        }
        break;
    case ACDK_MHAL_PRE_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_PREVIEW_STOP:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_PRE_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_PRE_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_PREVIEW_STOP:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_PREVIEW_STOP");
            ACDK_LOGE("State error ACDK_MHAL_PREVIEW_STOP");
            break;
        }
        break;
    case ACDK_MHAL_CAPTURE:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
            break;
        default:
            //ACDK_ASSERT(0, "State error MHAL_CAM_CAPTURE");
            ACDK_LOGE("State error ACDK_MHAL_CAPTURE");
            break;
        }
        break;
    case ACDK_MHAL_ERROR:
        switch(newState)
        {
        case ACDK_MHAL_IDLE:
        case ACDK_MHAL_UNINIT:
            break;
        default:
            //ACDK_ASSERT(0, "State error ACDK_MHAL_ERROR");
            ACDK_LOGE("State error ACDK_MHAL_ERROR");
            break;
        }
        break;
    default:
        //ACDK_ASSERT(0, "Unknown state");
        ACDK_LOGE("Unknown state");
        break;
    }

ACDK_MHAL_SET_STATE_EXIT:

    mAcdkMhalState = newState;

    ACDK_LOGD("X, state(0x%04x)", mAcdkMhalState);
}

/*******************************************************************************
* acdkMhalGetState
* brif : get state of acdk mhal
*******************************************************************************/
acdkMhalState_e AcdkMhalPure::acdkMhalGetState()
{
    Mutex::Autolock _l(mLock);
    return mAcdkMhalState;
}

/*******************************************************************************
* acdkMhalReadyForCap
* brif : get status of mReadyForCap falg
*******************************************************************************/
MBOOL AcdkMhalPure::acdkMhalReadyForCap()
{
    return mReadyForCap;
}

/*******************************************************************************
* acdkMhalProcLoop
* brif : preview and capture thread executing function
*******************************************************************************/
static MVOID *acdkMhalProcLoop(MVOID *arg)
{
    ::prctl(PR_SET_NAME,"acdkMhalProcLoop",0,0,0);

    MINT32 const policy = SCHED_OTHER;
    MINT32 const priority = NICE_CAMERA_PASS1;
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    sched_p.sched_priority = priority;
    ::sched_setscheduler(0, policy, &sched_p);

    ::sched_getparam(0, &sched_p);
    ACDK_LOGD("policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d),tid=%d", policy,
                                                                                             ::sched_getscheduler(0),
                                                                                             priority,
                                                                                             sched_p.sched_priority,
                                                                                             ::gettid());

     //  detach thread => cannot be join
    ::pthread_detach(::pthread_self());

    acdkMhalState_e eState;

    eState = g_pAcdkMHalPureObj->acdkMhalGetState();
    while(eState != ACDK_MHAL_UNINIT)
    {
        ACDK_LOGD("Wait semMainHigh");
        ::sem_wait(&g_SemMainHigh); // wait here until someone use sem_post() to wake this semaphore up
        ACDK_LOGD("Got semMainHigh");

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();

        switch(eState)
        {
        case ACDK_MHAL_PREVIEW:
            g_pAcdkMHalPureObj->acdkMhalPreviewProc();
            ::sem_post(&g_SemMainHighBack);
            break;
        case ACDK_MHAL_CAPTURE:
            g_pAcdkMHalPureObj->acdkMhalCaptureProc();
            break;
        case ACDK_MHAL_UNINIT:
            break;
        default:
            ACDK_LOGD("T.B.D");
            break;
        }

        eState = g_pAcdkMHalPureObj->acdkMhalGetState();
    }


    ::sem_post(&g_SemMainHighEnd);
    ACDK_LOGD("-");

    return NULL;
}

/*******************************************************************************
* destroyInstanc
* brif : destroy AcdkMhalPure object
*******************************************************************************/
void AcdkMhalPure::destroyInstance()
{
    g_pAcdkMHalPureObj = NULL;
    delete this;
}

/*******************************************************************************
* acdkMhalInit
* brif : initialize camera
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalInit()
{
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32  err = ACDK_RETURN_NO_ERROR;

    //====== Acdk Mhal State Setting ======

    mAcdkMhalState = ACDK_MHAL_INIT;    //set state to Init state

    //====== Get Debug Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};
    property_get("camera.acdk.debug", value, "0");
    g_acdkMhalPureDebug = atoi(value);

    ACDK_LOGD("g_acdkMhalPureDebug(%d)",g_acdkMhalPureDebug);

    //====== Init Thread for Preview and Capture ======

    // Init semphore
    ::sem_init(&g_SemMainHigh, 0, 0);
    ::sem_init(&g_SemMainHighBack, 0, 0);
    ::sem_init(&g_SemMainHighEnd, 0, 0);

    // Create main thread for preview and capture
    pthread_attr_t const attr = {0, NULL, 1024 * 1024, 4096, SCHED_OTHER, NICE_CAMERA_PASS1};
    pthread_create(&g_threadMainHigh, &attr, acdkMhalProcLoop, NULL);

    //=== Set State to Idle State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalUninit
* brif : Uninitialize camera
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalUninit()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e eState;

    //====== Uninitialization ======

    // Check it is in the idle mode
    // If it is not, has to wait until idle
    eState = acdkMhalGetState();

    ACDK_LOGD("eState(0x%x)",eState);

    if(eState != ACDK_MHAL_NONE)
    {
        if((eState != ACDK_MHAL_IDLE) && (eState != ACDK_MHAL_ERROR))
        {
            ACDK_LOGD("Camera is not in the idle state");
            if(eState & ACDK_MHAL_PREVIEW_MASK)
            {
                err = acdkMhalPreviewStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalPreviewStop fail(0x%x)",err);
                }
            }
            else if(eState & ACDK_MHAL_CAPTURE_MASK)
            {
                err = acdkMhalCaptureStop();

                if(err != ACDK_RETURN_NO_ERROR)
                {
                    ACDK_LOGE("acdkMhalCaptureStop fail(0x%x)",err);
                }
            }

            // Polling until idle
            while(eState != ACDK_MHAL_IDLE)
            {
                // Wait 10 ms per time
                usleep(10000);
                eState = acdkMhalGetState();
            }
            ACDK_LOGD("Now camera is in the idle state");
        }

        //====== Set State to Uninit State ======

        acdkMhalSetState(ACDK_MHAL_UNINIT);

        //====== 3A  Object Uninit ======

#ifdef ACDK_CAMERA_3A
        if(m_p3AHal != NULL)
        {
            m_p3AHal->destroyInstance("ACDKMHALPURE");
        }
#endif
        //====== Semephore Process ======

        //post sem
        ACDK_LOGD("post g_SemMainHigh");
        ::sem_post(&g_SemMainHigh);

        //wait sem
        ACDK_LOGD("wait for g_SemMainHighEnd");
        ::sem_wait(&g_SemMainHighEnd);
        ACDK_LOGD("got g_SemMainHighEnd");
    }
    else
    {
        acdkMhalSetState(ACDK_MHAL_UNINIT);
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalCBHandle
* brif : callback handler
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalCBHandle(MUINT32 a_type, MUINT8* a_addr1, MUINT8* a_addr2, MUINT32 const a_dataSize)
{
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"+");

    if(!g_acdkMhalObserver)
    {
        ACDK_LOGE("callback is NULL");
    }

    // Callback to upper layer
    g_acdkMhalObserver.notify(a_type, a_addr1, a_addr2, a_dataSize);
}

/*******************************************************************************
* acdkMhal3ASetParm
* brif : set 3A parameter
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhal3ASetParam(MINT32 devID, MUINT8 IsFactory)
{
    ACDK_LOGD("devID(%d)",devID);

#ifdef ACDK_CAMERA_3A
    Param_T cam3aParam;

    if(m_p3AHal == NULL)
    {
        m_p3AHal = IHal3A::createInstance(IHal3A::E_Camera_1,devID,"ACDKMHALPURE");
    }

    //====== 3AHal Set Parameter ======

    if(m_p3AHal == NULL)
    {
        ACDK_LOGE("m_p3AHal is NULL");
        return ACDK_RETURN_NULL_OBJ;
    }

    //get 3A parameter (default value)
    if(!m_p3AHal->getParams(cam3aParam))
    {
        ACDK_LOGE("m_p3AHal->getParams() fail");
        return ACDK_RETURN_API_FAIL;
    }

    //3A parameter setting
    cam3aParam.u4CamMode = eAppMode_PhotoMode;
    //AF mode sync ( 1.factory set to continue mode  2.Meda mode set to single)
    if(IsFactory)
        cam3aParam.u4AfMode = 1;
    else
        cam3aParam.u4AfMode = 0;
    //Tuning parameter setting
    cam3aParam.u4BrightnessMode = 1;
    cam3aParam.u4HueMode        = 1;
    cam3aParam.u4SaturationMode = 1;
    cam3aParam.u4EdgeMode       = 1;
    cam3aParam.u4ContrastMode   = 1;

    //set zoom info
    m_p3AHal->setZoom(100, 0, 0, mAcdkMhalPrvParam.sensorWidth, mAcdkMhalPrvParam.sensorHeight);

    //set 3A parameter
    if(!m_p3AHal->setParams(cam3aParam))
    {
        ACDK_LOGE("m_p3AHal->setParams() fail");
        //return ACDK_RETURN_API_FAIL;
    }

    // set 3A callback
    if(!m_p3AHal->addCallbacks(g_pAcdkMHalPureObj))
    {
        ACDK_LOGE("m_p3AHal->setCallbacks() fail");
        return ACDK_RETURN_API_FAIL;
    }

    ACDK_LOGD("-");
#endif
    return ACDK_RETURN_NO_ERROR;
}
#define PASS2_FULLG

/*******************************************************************************
* acdkMhalGetPrvList
* brif : Get preview size list
*******************************************************************************/

String8 AcdkMhalPure::acdkMhalGetPrvList(MUINT32 nSIndex)
{
    String8 sPrvSize = String8("");
    return sPrvSize;
}

/*******************************************************************************
* acdkMhalPreviewStart
* brif : Start preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");
    char szUser[32] = "acdk_preview";
    EScenarioFmt eScenarioFmt = eScenarioFmt_RAW;
    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalPreviewStart] Camera State is not IDLE");

    //====== Local Variable Declaration ======
    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalPrvParam, a_pBuffIn, sizeof(acdkMhalPrvParam_t));
    QBufInfo rQBufInfo;

    //====== Set 3A Parameter ======

#ifdef ACDK_CAMERA_3A
    acdkMhal3ASetParam((MINT32)mAcdkMhalPrvParam.sensorIndex,(MUINT8)mAcdkMhalPrvParam.IsFactoryMode);
#endif
    //====== Member Variable ======

    for(int i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
        ACDK_LOGD("imgImemBuf  : virAddr[%d]=0x%x",i,mAcdkMhalPrvParam.imgImemBuf[i].virtAddr);
        ACDK_LOGD("dispImemBuf : virAddr[%d]=0x%x",i,mAcdkMhalPrvParam.dispImemBuf[i].virtAddr);
    }

    //====== Init CameraIO Pipe ======
    if(mAcdkMhalPrvParam.sensorType == 2) // 1-RAW,2-YUV
    {
        eScenarioFmt = eScenarioFmt_YUV;
        is_yuv_sensor = 1;
    }
    else
    {
        eScenarioFmt = eScenarioFmt_RAW;
        is_yuv_sensor = 0;
    }
    //m_pICamIOPipe = ICamIOPipe::createInstance(mAcdkMhalPrvParam.scenarioHW, mAcdkMhalPrvParam.sensorType?eScenarioFmt_YUV:eScenarioFmt_RAW);

    m_pICamIOPipe = ICamIOPipe::createInstance(eDrvScenario_CC, \
                                               eScenarioFmt,\
                                               (MINT8 const*)szUser);

    ACDK_LOGD("eScenarioFmt=0x%x, is_yuv_sensor=0x%x",eScenarioFmt,is_yuv_sensor);
    if(MTRUE != m_pICamIOPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->init() fail");
        goto acdkMhalPreviewStart_Exit;
    }
    //====== Config CamIO Pipe ======
 /*   if (is_yuv_sensor)
    m_tgi.ePxlMode    = (mAcdkMhalPrvParam.mSensorPixelMode == ONE_PIXEL_MODE) ? ePxlMode_One_Single : ePxlMode_Two_Single;
  else
  m_tgi.ePxlMode    = (mAcdkMhalPrvParam.mSensorPixelMode == ONE_PIXEL_MODE) ? ePxlMode_One_Single : ePxlMode_Two_Twin;*/
  switch (mAcdkMhalPrvParam.mSensorPixelMode) {
          case TWO_PIXEL_MODE:
             if (2 == mAcdkMhalPrvParam.sensorType) {
                 m_tgi.ePxlMode = ePxlMode_Two_Single;
             }
             else {
                 m_tgi.ePxlMode = ePxlMode_Two_Twin;
             }
             break;
         case ONE_PIXEL_MODE:
         default:
               m_tgi.ePxlMode = ePxlMode_One_Single;
            break;
     }

#ifdef ALL_SENSOR_USE_TG1
    m_tgi.index       = (mAcdkMhalPrvParam.sensorID == SENSOR_DEV_MAIN) ? EPortIndex_TG1I : EPortIndex_TG1I;//  port index
#else
    m_tgi.index       = (mAcdkMhalPrvParam.sensorID == SENSOR_DEV_MAIN) ? EPortIndex_TG1I : EPortIndex_TG2I;//  port index
#endif
    m_tgi.eImgFmt     = (EImageFormat)mAcdkMhalPrvParam.sensorFormat; //  Image Pixel Format
    m_tgi.eRawPxlID   = (ERawPxlID)mAcdkMhalPrvParam.sensorColorOrder;
    m_tgi.u4ImgWidth  = mAcdkMhalPrvParam.sensorWidth;  // Image Width
    m_tgi.u4ImgHeight = mAcdkMhalPrvParam.sensorHeight; // Image Height
    m_tgi.u4PureRaw   = 1;
    m_tgi.u4PureRawPak = 1;
    m_tgi.type        = EPortType_Sensor;               // EPortType
    m_tgi.inout       = EPortDirection_In;              // 0:in/1:out
    ACDK_LOGD("m_tgi.ePxlMode=0x%x",m_tgi.ePxlMode);
    ACDK_LOGD("m_tgi.index=0x%x",m_tgi.index);
    ACDK_LOGD("m_tgi.eImgFmt=0x%x",m_tgi.eImgFmt);
    ACDK_LOGD("m_tgi.eRawPxlID=0x%x",m_tgi.eRawPxlID);
    ACDK_LOGD("m_tgi.u4ImgWidth=0x%x",m_tgi.u4ImgWidth);
    ACDK_LOGD("m_tgi.u4ImgHeight=0x%x",m_tgi.u4ImgHeight);
    ACDK_LOGD("m_tgi.type=0x%x",m_tgi.type);
    ACDK_LOGD("m_tgi.inout=0x%x",m_tgi.inout);

    m_imgo.eImgFmt      = m_tgi.eImgFmt;     // Image Pixel Format
    m_imgo.u4PureRaw    = 0;
    m_imgo.u4ImgWidth   = m_tgi.u4ImgWidth;  // Image Width
    m_imgo.u4ImgHeight  = m_tgi.u4ImgHeight; // Image Height
    m_imgo.crop1.y      = 0;
    m_imgo.crop1.floatY = 0;
    m_imgo.crop1.h = m_imgo.u4ImgHeight;
    m_imgo.crop1.w = m_imgo.u4ImgWidth;
    if(is_yuv_sensor)
        m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.sensorStride[0]*2;
    else
        m_imgo.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.sensorStride[0]*10/8;    //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalPrvParam.sensorStride[1]; //  unit:PIXEL
    m_imgo.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalPrvParam.sensorStride[2]; //  unit:PIXEL
    m_imgo.type  = EPortType_Memory;        // EPortType
#ifdef ALL_SENSOR_USE_TG1
    m_imgo.index  = EPortIndex_IMGO;     //  port index
#else
    if(m_tgi.index == EPortIndex_TG1I)
    {
        m_imgo.index  = EPortIndex_IMGO;     //  port index
    }
    else
    {
        m_imgo.index  = EPortIndex_IMGO_D;     //  port index
    }
#endif
    m_imgo.inout = EPortDirection_Out;      // 0:in/1:out
    mu4SensorDelay = mAcdkMhalPrvParam.mu4SensorDelay; //Sensor Delay Frame
    ACDK_LOGD("m_imgo.eImgFmt=0x%x",m_imgo.eImgFmt);
    ACDK_LOGD("m_imgo.u4ImgWidth=0x%x",m_imgo.u4ImgWidth);
    ACDK_LOGD("m_imgo.u4ImgHeight=0x%x",m_imgo.u4ImgHeight);
    ACDK_LOGD("m_imgo.crop1.w=0x%x",m_imgo.crop1.w);
    ACDK_LOGD("m_imgo.crop1.h=0x%x",m_imgo.crop1.h);
    ACDK_LOGD("m_imgo.u4Stride[ESTRIDE_1ST_PLANE]=0x%x",m_imgo.u4Stride[ESTRIDE_1ST_PLANE]);
    // single mode line buffer 3072 , twin mode can accept line buffer 2304 x2
    // sensor mode two pixel , isp would use twin mode ( over 13MP / 30fps)s
    m_rrzo.eImgFmt        = eImgFmt_FG_BAYER10;  //  Image Pixel Format
    m_rrzo.u4ImgWidth     = mAcdkMhalPrvParam.rrzoWidth; //2304;        //  Image Width
    m_rrzo.u4ImgHeight    = m_tgi.u4ImgHeight; //1920;        //  Image Height
    m_rrzo.crop1.x         = 0;
    m_rrzo.crop1.y         = 0;
    m_rrzo.crop1.floatX    = 0;
    m_rrzo.crop1.floatY    = 0;

    if(m_rrzo.u4ImgWidth > 2304)
    {
        MINT32 rrzowidth,rrzoheight;
        rrzowidth = 2048;
        rrzoheight = (((m_rrzo.u4ImgHeight*2048)/m_rrzo.u4ImgWidth)>>2)<<2;//rrzoheight* ((MFLOAT)m_imgo.u4ImgWidth/2304);
        m_rrzo.u4ImgWidth = rrzowidth;
        m_rrzo.u4ImgHeight = rrzoheight;

    }
    m_rrzo.crop1.w         = m_rrzo.u4ImgWidth;
    m_rrzo.crop1.h         = m_rrzo.u4ImgHeight;
    m_rrzo.u4Stride[ESTRIDE_1ST_PLANE] = m_rrzo.crop1.w*10/8*1.5;//  unit:PIXEL
    m_rrzo.type   = EPortType_Memory;     //  EPortType
    if(m_tgi.index == EPortIndex_TG1I)
    {
        m_rrzo.index  = EPortIndex_RRZO;     //  port index
    }
    else
    {
        m_rrzo.index  = EPortIndex_RRZO_D;     //  port index
    }
    m_rrzo.inout  = EPortDirection_Out; //  0:in/1:out


    ACDK_LOGD("m_rrzo.eImgFmt=0x%x",m_rrzo.eImgFmt);
    ACDK_LOGD("m_rrzo.u4ImgWidth=0x%x",m_rrzo.u4ImgWidth);
    ACDK_LOGD("m_rrzo.u4ImgHeight=0x%x",m_rrzo.u4ImgHeight);
    ACDK_LOGD("m_rrzo.u4Stride[ESTRIDE_1ST_PLANE]=0x%x",m_rrzo.u4Stride[ESTRIDE_1ST_PLANE]);
    //set CQ first before pipe config
#ifdef ALL_SENSOR_USE_TG1
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0,0,0);
    //single trigger
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
#else
    if(m_tgi.index == EPortIndex_TG1I)
    {
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0,0,0);
        //single trigger
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
    }
    else
    {
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_D_CQ_CHANNEL,(MINT32)CAM_ISP_CQ0_D,0,0);
        //single trigger
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_D_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
    }
#endif

    // config IO port
    g_vCamIOInPorts.at(0)   = &m_tgi;
    //g_vCamIOIOutPorts.at(0) = &m_imgo;
    if(is_yuv_sensor)
    {
        g_vCamIOIOutPorts.resize(1);
        g_vCamIOIOutPorts.at(0) = &m_imgo;
    }
    else
    {
        g_vCamIOIOutPorts.resize(2);
        g_vCamIOIOutPorts.at(0) = &m_imgo;
        g_vCamIOIOutPorts.at(1) = &m_rrzo;
    }


    if(MTRUE != m_pICamIOPipe->configPipe(g_vCamIOInPorts, g_vCamIOIOutPorts))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->configPipe() fail");
        goto acdkMhalPreviewStart_Exit;
    }
    //====== RRZO Resize for hw limitation
    if(0)//m_imgo.u4ImgWidth > 2304)
    {
        RRZCfg rrz;
        MINT32 rrzowidth,rrzoheight;

        rrzowidth = 2304;
        rrzoheight = (((m_imgo.u4ImgHeight*2304)/m_imgo.u4ImgWidth)>>2)<<2;//rrzoheight* ((MFLOAT)m_imgo.u4ImgWidth/2304);

        ACDK_LOGD("RRZO Resize width %d, height %d",rrzowidth,rrzoheight);
        //
        rrz.crop.x      = 0;
        rrz.crop.y      = 0;
        rrz.crop.floatX = 0;
        rrz.crop.floatY = 0;
        rrz.crop.w      = m_rrzo.u4ImgWidth;
        rrz.crop.h      = m_rrzo.u4ImgHeight;
        rrz.out.w       = rrzowidth;
        rrz.out.h       = rrzoheight;
        rrz.out.stride  = rrzowidth;
        rrz.m_num = 0;
        if (!m_pICamIOPipe->sendCommand(EPIPECmd_SET_RRZ,(MINTPTR)&rrz,0,0) ){
            return -1;
        }

        //if (!m_pICamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE,EPortIndex_TG1I,0,0) ) {
        if (!m_pICamIOPipe->sendCommand(EPIPECmd_SET_P1_UPDATE,m_tgi.index,0,0) ) {
            return -1;
        }

        m_rrzo.crop1.w         = rrz.out.w;
        m_rrzo.crop1.h         = rrz.out.h;
        m_rrzo.u4Stride[ESTRIDE_1ST_PLANE] = rrz.out.w;
        ACDK_LOGD("rmag(%d),imag(%d)",rrz.m_num,0);
    }
    //====== Enqueue Buffer ======
    //IMGO
    rQBufInfo.vBufInfo.resize(1);
    rQBufInfo.vBufInfo[0].header   = NULL;
    for(MINT32 i = 0; i < OVERLAY_BUFFER_CNT; ++i)
    {
#ifdef ALL_SENSOR_USE_TG1
        mPortID.index = EPortIndex_IMGO;
#else
        if(m_tgi.index == EPortIndex_TG1I)
        {
            mPortID.index = EPortIndex_IMGO;
        }
        else
        {
            mPortID.index = EPortIndex_IMGO_D;
        }
#endif
        rQBufInfo.vBufInfo[0].memID[0]   = mAcdkMhalPrvParam.imgImemBuf[i].memID;
        rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalPrvParam.imgImemBuf[i].size;  //bytes
        rQBufInfo.vBufInfo[0].u4BufVA[0] = mAcdkMhalPrvParam.imgImemBuf[i].virtAddr;
        rQBufInfo.vBufInfo[0].u4BufPA[0] = mAcdkMhalPrvParam.imgImemBuf[i].phyAddr;
        m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
        ACDK_LOGD("IMGO m_pICamIOPipe->enqueOutBuf[%d].u4BufPA(%x)",i,rQBufInfo.vBufInfo[0].u4BufVA[0]);
        if(is_yuv_sensor == 0) // raw sensor use rrzo
        {
#ifdef ALL_SENSOR_USE_TG1
            mPortID.index = EPortIndex_RRZO;
#else
            if(m_tgi.index == EPortIndex_TG1I)
            {
                mPortID.index = EPortIndex_RRZO;
            }
            else
            {
                mPortID.index = EPortIndex_RRZO_D;
            }
#endif
            rQBufInfo.vBufInfo[0].memID[0]   = mAcdkMhalPrvParam.rrzoImemBuf[i].memID;
            rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalPrvParam.rrzoImemBuf[i].size;  //bytes
            rQBufInfo.vBufInfo[0].u4BufVA[0] = mAcdkMhalPrvParam.rrzoImemBuf[i].virtAddr;
            rQBufInfo.vBufInfo[0].u4BufPA[0] = mAcdkMhalPrvParam.rrzoImemBuf[i].phyAddr;
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
            ACDK_LOGD("RRZO m_pICamIOPipe->enqueOutBuf[%d].u4BufPA(%x)",i,rQBufInfo.vBufInfo[0].u4BufVA[0]);
        }
    }

    //====== Init Postproc Pipe ======
    m_pPostProcPipe = IPostProcPipe::createInstance(eDrvScenario_CC, eScenarioFmt_RAW);

    if(MTRUE != m_pPostProcPipe->init())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->init() fail");
        goto acdkMhalPreviewStart_Exit;
    }
//#define PASS2_FULLG
    //====== Config PostProcPipe ======
#ifdef PASS2_FULLG
    if(is_yuv_sensor == 0)
    {
        m_imgi.eImgFmt     = m_rrzo.eImgFmt;    //  Image Pixel Format
        m_imgi.u4ImgWidth  = m_rrzo.crop1.w;    //  Image Width
        m_imgi.u4ImgHeight = m_rrzo.crop1.h;    //  Image Height
        //m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_rrzo.u4Stride[ESTRIDE_1ST_PLANE];    //  pixel
        m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgi.u4ImgWidth*10/8*1.5;    //  byte
    }
    else
    {
        m_imgi.eImgFmt     = eImgFmt_YUY2;//m_imgo.eImgFmt;    //  Image Pixel Format
        m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;    //  Image Width
        m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
        m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    //  pixel
    }
    m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = 0;
    m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = 0;
#else
    m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
    m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
    m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
    m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE]*10/8;    //  pixel
    m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];
    m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];
#endif
    m_imgi.crop1.x = 0;
    m_imgi.crop1.floatX = 0;
    m_imgi.crop1.y = 0;
    m_imgi.crop1.floatY = 0;
    m_imgi.crop1.w = m_imgi.u4ImgWidth;
    m_imgi.crop1.h = m_imgi.u4ImgHeight;
    m_imgi.resize1.tar_w = m_imgi.u4ImgWidth;
    m_imgi.resize1.tar_h = m_imgi.u4ImgHeight;
    m_imgi.crop2.x = 0;
    m_imgi.crop2.floatX = 0;
    m_imgi.crop2.y = 0;
    m_imgi.crop2.floatY = 0;
    m_imgi.crop2.w = m_imgi.u4ImgWidth;
    m_imgi.crop2.h = m_imgi.u4ImgHeight;
    m_imgi.resize2.tar_w = m_imgi.u4ImgWidth;
    m_imgi.resize2.tar_h = m_imgi.u4ImgHeight;
    m_imgi.type        = EPortType_Memory;      //  EPortType
    m_imgi.index       = EPortIndex_IMGI;       //  port index
    m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
    m_imgi.pipePass    = EPipePass_PASS2;       //  select pass
#ifdef PASS2_FULLG
    if(is_yuv_sensor == 0)
    {
        m_imgi.u4BufVA[0]     = mAcdkMhalPrvParam.rrzoImemBuf[0].virtAddr; //  Vir Address of pool
        m_imgi.u4BufPA[0]     = mAcdkMhalPrvParam.rrzoImemBuf[0].phyAddr; //  Phy Address of pool
        m_imgi.memID[0]       = (MINT32)mAcdkMhalPrvParam.rrzoImemBuf[0].memID;
        m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.rrzoImemBuf[0].size;
    }
    else
    {
        m_imgi.u4BufVA[0]     = mAcdkMhalPrvParam.imgImemBuf[0].virtAddr; //  Vir Address of pool
        m_imgi.u4BufPA[0]     = mAcdkMhalPrvParam.imgImemBuf[0].phyAddr; //  Phy Address of pool
        m_imgi.memID[0]       = (MINT32)mAcdkMhalPrvParam.imgImemBuf[0].memID;
        m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.imgImemBuf[0].size;
    }
#else
    m_imgi.u4BufVA[0]     =  mAcdkMhalPrvParam.imgImemBuf[0].virtAddr; //  Vir Address of pool
    m_imgi.u4BufPA[0]     = mAcdkMhalPrvParam.imgImemBuf[0].phyAddr; //  Phy Address of pool
    m_imgi.memID[0]       = (MINT32)mAcdkMhalPrvParam.imgImemBuf[0].memID;
    m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.imgImemBuf[0].size;
#endif
    //m_imgi.eRawPxlID   = (ERawPxlID)mAcdkMhalPrvParam.sensorColorOrder;//ERawPxlID_R;
    m_imgi.u4MagicNum = 0;//magicNum

    m_wroto.eImgFmt     = (EImageFormat)mAcdkMhalPrvParam.frmParam.frmFormat;  // Image Pixel Format
    m_wroto.eImgRot     = (EImageRotation)mAcdkMhalPrvParam.frmParam.orientation;    // Rotation
    m_wroto.eImgFlip    = mAcdkMhalPrvParam.frmParam.flip ? eImgFlip_ON : eImgFlip_OFF;
    m_wroto.u4ImgWidth  = mAcdkMhalPrvParam.frmParam.w;  // Dispaly Width. concept is assign memory footprint width and height
    m_wroto.u4ImgHeight = mAcdkMhalPrvParam.frmParam.h;  // Display Height. concept is assign memory footprint width and height
    // m_wroto.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalPrvParam.frmParam.stride[0]; //  Unit : Pixel
    m_wroto.u4Stride[ESTRIDE_1ST_PLANE] = m_wroto.u4ImgWidth*2;//  Unit : Byte
    m_wroto.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalPrvParam.frmParam.stride[1];
    m_wroto.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalPrvParam.frmParam.stride[2];
    m_wroto.type        = EPortType_Memory;     //  EPortType
    m_wroto.index       = EPortIndex_WROTO;      // temp use for debug EPortIndex_IMG2O/EPortIndex_WROTO;     //  port index
    m_wroto.inout       = EPortDirection_Out;   //  0:in/1:out
    m_wroto.pipePass    = EPipePass_PASS2;      //  select pass
    m_wroto.u4BufVA[0]     = mAcdkMhalPrvParam.dispImemBuf[0].virtAddr;
    m_wroto.u4BufPA[0]     = mAcdkMhalPrvParam.dispImemBuf[0].phyAddr;
    m_wroto.memID[0]       = (MINT32)mAcdkMhalPrvParam.dispImemBuf[0].memID;
    m_wroto.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.dispImemBuf[0].size;  //bytes

    g_vPostProcInPorts.resize(1);
    g_vPostProcInPorts.at(0) = &m_imgi;

    g_vPostProcOutPorts.resize(1);
    g_vPostProcOutPorts.at(0) = &m_wroto;

    //
    memset((MVOID *)&g_CookieVa,0,sizeof(stISP_BUF_INFO));
    g_CookieVa.img_pxl_id = (ERawPxlID)mAcdkMhalPrvParam.sensorColorOrder;//ERawPxlID_R;
#if 0
    // Configue Pipe & Enque buffer
    if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts,0))
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
        goto acdkMhalPreviewStart_Exit;
    }
#endif
    //====== Setting Callback ======
    g_acdkMhalObserver = mAcdkMhalPrvParam.acdkMainObserver;

    //====== Set State to Preview State ======

    acdkMhalSetState(ACDK_MHAL_PREVIEW);

    //====== Post Sem ======

    ::sem_post(&g_SemMainHigh);

acdkMhalPreviewStart_Exit:

    if(err != ACDK_RETURN_NO_ERROR)
    {
        if(m_pICamIOPipe != NULL)
        {
            ACDK_LOGE("release m_pICamIOPipe");
            m_pICamIOPipe->uninit();
            m_pICamIOPipe->destroyInstance();
            m_pICamIOPipe = NULL;
        }

        if(m_pPostProcPipe != NULL)
        {
            ACDK_LOGE("release m_pPostProcPipe");
            m_pPostProcPipe->uninit();
            m_pPostProcPipe->destroyInstance();
            m_pPostProcPipe = NULL;
        }
    }

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreviewStop
* brif : stop preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewStop()
{
    ACDK_LOGD("+");

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    ACDK_LOGD("state(%d)", state);
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("is in IDLE state");
        return err;
    }
    else if(state != ACDK_MHAL_PREVIEW_STOP)
    {
        if(state & ACDK_MHAL_PREVIEW_MASK)
        {
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_PRE_CAPTURE)
        {
            // In preCapture, has to wait 3A ready flag
            ACDK_LOGD("it is ACDK_MHAL_PRE_CAPTURE state");
            acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);
        }
        else if(state == ACDK_MHAL_CAPTURE)
        {
            // It is in capture flow now, preview has been already stopped
            ACDK_LOGD("it is ACDK_MHAL_CAPTURE state");
            state = acdkMhalGetState();
            while(state == ACDK_MHAL_CAPTURE)
            {
                usleep(20);
                state = acdkMhalGetState();
            }

            acdkMhalSetState(ACDK_MHAL_IDLE);
        }
        else
        {
            // Unknown
            ACDK_LOGE("un know state(%d)", state);
        }
    }

    //====== Wait Semaphore ======

    ACDK_LOGD("Wait g_SemMainHighBack");
    ::sem_wait(&g_SemMainHighBack);
    ACDK_LOGD("Got g_SemMainHighBack");

    //====== 3A Scenario Control ======

#ifdef ACDK_CAMERA_3A
    m_p3AHal->sendCommand(ECmd_CameraPreviewEnd);
#endif
    //====== Stop Preview ======

    if(MTRUE != m_pICamIOPipe->stop())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->stop() fail");
    }

    //====== Uninit CamIO & PostProcPipe ======

    if(MTRUE != m_pICamIOPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pICamIOPipe->uninit() fail");
    }

    m_pICamIOPipe->destroyInstance();
    m_pICamIOPipe = NULL;
    ACDK_LOGD("m_pICamIOPipe->destroyInstance()");

    if(MTRUE != m_pPostProcPipe->uninit())
    {
        err = ACDK_RETURN_API_FAIL;
        ACDK_LOGE("m_pPostProcPipe->uninit() fail");
    }

    m_pPostProcPipe->destroyInstance();
    m_pPostProcPipe = NULL;
    ACDK_LOGD("m_pPostProcPipe->destroyInstance()");


    //====== Initialize Member Variable =====

    memset(&mAcdkMhalPrvParam,0,sizeof(acdkMhalPrvParam_t));

    //====== Set Acdk Mhal State ======

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("-");
    return err;
}

/*******************************************************************************
* acdkMhalPreCapture
* brif : change ACDK mhal state to preCapture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreCapture()
{
    ACDK_LOGD("+");

    MINT32 err = ACDK_RETURN_NO_ERROR;

    //====== Change State ======

    acdkMhalSetState(ACDK_MHAL_PRE_CAPTURE);

    ACDK_LOGD("-");

    return err;
}


/*******************************************************************************
* acdkMhalCaptureStart
* brif : init capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureStart(MVOID *a_pBuffIn)
{
    ACDK_LOGD("+");
#if 0

    ACDK_ASSERT(acdkMhalGetState() == ACDK_MHAL_IDLE, "[acdkMhalCaptureStart] Camera State is not IDLE");

    //====== Variable Setting ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    memcpy(&mAcdkMhalCapParam, a_pBuffIn, sizeof(acdkMhalCapParam_t));

    //======  Capture Object ======

    ISingleShot *singleShot = ISingleShot::createInstance(eShotMode_NormalShot, "ACDK_NormalShot");
    if(singleShot == NULL)
    {
        ACDK_LOGE("singleShot create fail");
        return ACDK_RETURN_NULL_OBJ;
    }

    singleShot->init();

    //====== Setting Callback ======

    g_acdkMhalObserver = mAcdkMhalCapParam.acdkMainObserver;

    ACDK_LOGD("mAcdkMhalCapParam.capType = 0x%x", mAcdkMhalCapParam.capType);
    if(mAcdkMhalCapParam.capType & 0x3C)
    {
        singleShot->enableDataMsg(ECamShot_DATA_MSG_BAYER | ECamShot_DATA_MSG_POSTVIEW);
    }
    else if(mAcdkMhalCapParam.capType == JPEG_TYPE)
    {
        singleShot->enableDataMsg(ECamShot_DATA_MSG_JPEG | ECamShot_DATA_MSG_POSTVIEW);
    }

    //====== Set State to Capture State ======

    acdkMhalSetState(ACDK_MHAL_CAPTURE);

    //====== Capture Paramter Setting ======

    // shot param
    ShotParam rShotParam((EImageFormat)mAcdkMhalCapParam.capFormat, // yuv format
                          mAcdkMhalCapParam.capWidth,               // picutre width
                          mAcdkMhalCapParam.CapHeight,              // picture height
                          mAcdkMhalCapParam.capOrientation,         // picture rotation
                          mAcdkMhalCapParam.capFlip,                // picture flip => single shot not support
                          mAcdkMhalCapParam.qvFormat,               // postview format
                          mAcdkMhalCapParam.qvWidth,                // postview width
                          mAcdkMhalCapParam.qvHeight,               // postview height
                          mAcdkMhalCapParam.qvOrientation,          // postview rotation => no use. acdkMain should handle rotation by itself
                          mAcdkMhalCapParam.qvFlip,                 // postview flip => single shot not support
                          mAcdkMhalCapParam.zoom);

    ACDK_LOGD("rShotParam.ePictureFmt        = 0x%x", rShotParam.ePictureFmt);
    ACDK_LOGD("rShotParam.u4PictureWidth     = %u", rShotParam.u4PictureWidth);
    ACDK_LOGD("rShotParam.u4PictureHeight    = %u", rShotParam.u4PictureHeight);
    ACDK_LOGD("rShotParam.u4PictureRotation  = %u", rShotParam.u4PictureRotation);
    ACDK_LOGD("rShotParam.u4PictureFlip      = %u", rShotParam.u4PictureFlip);
    ACDK_LOGD("rShotParam.ePostViewFmt       = 0x%x", rShotParam.ePostViewFmt);
    ACDK_LOGD("rShotParam.u4PostViewWidth    = %u", rShotParam.u4PostViewWidth);
    ACDK_LOGD("rShotParam.u4PostViewHeight   = %u", rShotParam.u4PostViewHeight);
    ACDK_LOGD("rShotParam.u4PostViewRotation = %u", rShotParam.u4PostViewRotation);
    ACDK_LOGD("rShotParam.u4PostViewFlip     = %u", rShotParam.u4PostViewFlip);
    ACDK_LOGD("rShotParam.u4ZoomRatio        = %u", rShotParam.u4ZoomRatio);

    // jpeg param
    JpegParam rJpegParam(mAcdkMhalCapParam.jpegQuality, //Quality : 0 ~ 100
                         mAcdkMhalCapParam.jpegIsSOI);  //isSOI

    //ACDK_LOGD("rJpegParam.u4ThumbWidth   = %u", rJpegParam.u4ThumbWidth);
    //ACDK_LOGD("rJpegParam.u4ThumbHeight  = %u", rJpegParam.u4ThumbHeight);
    //ACDK_LOGD("rJpegParam.u4ThumbQuality = %u", rJpegParam.u4ThumbQuality);
    //ACDK_LOGD("rJpegParam.fgThumbIsSOI   = %u", rJpegParam.fgThumbIsSOI);
    ACDK_LOGD("rJpegParam.u4Quality = %u", rJpegParam.u4Quality);
    ACDK_LOGD("rJpegParam.fgIsSOI   = %u", rJpegParam.fgIsSOI);

    // sensor param
    SensorParam rSensorParam(mAcdkMhalCapParam.sensorDev,       // device ID
                             mAcdkMhalCapParam.sensorScenario,  // scenaio
                             mAcdkMhalCapParam.bitDepth,        // bit depth
                             mAcdkMhalCapParam.byPassDelay,     // bypass delay
                             mAcdkMhalCapParam.byPassScenario,  // bypass scenario
                             mAcdkMhalCapParam.rawTypr);        // RAW type : 0-pure raw, 1-processed raw

    ACDK_LOGD("rSensorParam.u4DeviceID      = %u", rSensorParam.u4DeviceID);
    ACDK_LOGD("rSensorParam.u4Scenario      = %u", rSensorParam.u4Scenario);
    ACDK_LOGD("rSensorParam.u4Bitdepth      = %u", rSensorParam.u4Bitdepth);
    ACDK_LOGD("rSensorParam.fgBypassDelay   = %u", rSensorParam.fgBypassDelay);
    ACDK_LOGD("rSensorParam.fgBypassScenaio = %u", rSensorParam.fgBypassScenaio);
    ACDK_LOGD("rSensorParam.u4RawType       = %u", rSensorParam.u4RawType);

    singleShot->setCallbacks(NULL, camShotDataCB, this);

    singleShot->setShotParam(rShotParam);

    singleShot->setJpegParam(rJpegParam);

    ACDK_LOGD("mAcdkMhalCapParam.capCnt(%u)",mAcdkMhalCapParam.capCnt);

    for(MUINT32 i = 0; i < mAcdkMhalCapParam.capCnt; ++i)
    {
        singleShot->startOne(rSensorParam);
    }

    err = singleShot->uninit();
    singleShot->destroyInstance();

    ACDK_LOGD("X");
    return err;
#else

    mFocusDone = MFALSE;
    mFocusSucceed = MFALSE;

#ifdef ACDK_CAMERA_3A
    m_p3AHal->autoFocus();
#else
    mFocusDone = MTRUE;
    mFocusSucceed = MTRUE;
#endif
    ACDK_LOGD("wait focus done");
    while(mFocusDone == MFALSE)
    {
        usleep(200);
    }

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif
}


/*******************************************************************************
* acdkMhalCaptureStop
* brif : stop capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureStop()
{
    ACDK_LOGD("+");
#if 0

    //====== Local Variable Declaration ======

    MINT32 err = ACDK_RETURN_NO_ERROR;
    acdkMhalState_e state = acdkMhalGetState();

    //====== Check State ======

    //check AcdkMhal state
    if(state == ACDK_MHAL_IDLE)
    {
        ACDK_LOGD("in IDLE state");
        return err;
    }
    else if(state & ACDK_MHAL_PREVIEW_MASK)
    {
        ACDK_LOGD("in preview group state");
        return err;
    }
    else if(state & ACDK_MHAL_CAPTURE_MASK)
    {
        ACDK_LOGD("in capture group state");
        acdkMhalSetState(ACDK_MHAL_IDLE);
    }
    else
    {
        ACDK_LOGE("state:%d", state);
        ACDK_ASSERT(0, "[acdkMhalCaptureStop] state is unknow");
    }

    //====== Initialize Member Variable ======

    memset(&mAcdkMhalCapParam,0,sizeof(acdkMhalCapParam_t));

    //====== Set AcdkMhal State ======

    mReadyForCap = MFALSE;

    acdkMhalSetState(ACDK_MHAL_IDLE);

    ACDK_LOGD("X");
    return err;
#else
    mReadyForCap = MFALSE;
    mFocusDone = MFALSE;

    ACDK_LOGD("-");
    return ACDK_RETURN_NO_ERROR;
#endif
}

/*******************************************************************************
* acdkMhalPreviewProc
* brif : handle flow control of preview
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalPreviewProc()
{
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"+");

    //====== Loca Variable Declaration ======

    MINT32  imgo_memID[OVERLAY_BUFFER_CNT] = {0};
    MINT32  rrzo_memID[OVERLAY_BUFFER_CNT] = {0};
    MUINTPTR imgo_virAddr[OVERLAY_BUFFER_CNT] = {0}, imgo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINTPTR rrzo_virAddr[OVERLAY_BUFFER_CNT] = {0}, rrzo_phyAddr[OVERLAY_BUFFER_CNT] = {0};
    MUINT32 imgi_memID, imgi_virAddr, imgi_phyAddr, dispo_memID;
    MUINTPTR dispo_virAddr, dispo_phyAddr;
    MUINTPTR imgo_virA, imgo_phyA;
    MUINT32 imgo_num = 0,rrzo_num = 0;
    MBOOL isPreCap = MFALSE;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;
    int delayframecnt = 0;
    MUINT32 m_i4FrameCount = 0;
    #ifdef ACDK_CAMERA_3A
    ParamIspProfile_T _3A_profile(EIspProfile_Preview, mCurMagicNum, MTRUE, MTRUE);
    #endif
    //====== Get Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //====== Variable Setting ======
    g_vPostProcInPorts.resize(1);
    g_vPostProcOutPorts.resize(1);

    //====== Preview Process ======
#ifdef ACDK_CAMERA_3A
    ACDK_LOGD("sensorScenario 0x%x, magic %d", sensorScenario, mCurMagicNum);
    //3AHal control
    //3A scenario control
    m_p3AHal->setSensorMode(sensorScenario);
    ACDK_LOGD("3A preview start");
    m_p3AHal->sendCommand(ECmd_CameraPreviewStart, mCurMagicNum);
    m_p3AHal->setIspProfile(_3A_profile);

#endif

    //sync VD
#ifdef ALL_SENSOR_USE_TG1
    // start CQ
    m_pICamIOPipe->startCQ0();

    // because 3A need to dynamic update, so need to use continuous mode
    m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                             (MINT32)EPIPE_PASS1_CQ0,
                             (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                             (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);

    if(m_tgi.index == EPortIndex_TG1I)
    {
        ACDK_LOGD("irq(EPipePass_PASS1_TG1)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_VSYNC,ISP_DRV_IRQ_USER_ACDK);
    }
    else if(m_tgi.index == EPortIndex_TG2I)
    {
        //ACDK_LOGD("irq(EPipePass_PASS1_TG2)(EPIPEIRQ_VSYNC)");
        //m_pICamIOPipe->irq(EPipePass_PASS1_TG1_D,EPIPEIRQ_VSYNC,ISP_DRV_IRQ_USER_ACDK);
    }


#else
    if(m_tgi.index == EPortIndex_TG1I)
    {
        // start CQ
        m_pICamIOPipe->startCQ0();

        // because 3A need to dynamic update, so need to use continuous mode
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,
                                 (MINT32)EPIPE_PASS1_CQ0,
                                 (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                                 (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);

        ACDK_LOGD("irq(EPipePass_PASS1_TG1)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_VSYNC,ISP_DRV_IRQ_USER_ACDK);
    }
    else if(m_tgi.index == EPortIndex_TG2I)
    {
        // start CQ
        m_pICamIOPipe->startCQ0_D();

        // because 3A need to dynamic update, so need to use continuous mode
        m_pICamIOPipe->sendCommand(EPIPECmd_SET_D_CQ_TRIGGER_MODE,
                                 (MINT32)EPIPE_PASS1_CQ0_D,
                                 (MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,
                                 (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);


        ACDK_LOGD("irq(EPipePass_PASS1_TG2)(EPIPEIRQ_VSYNC)");
        m_pICamIOPipe->irq(EPipePass_PASS1_TG1_D,EPIPEIRQ_VSYNC,ISP_DRV_IRQ_USER_ACDK);
    }
#endif

    ACDK_LOGD("Preview pass1 start");
    m_pICamIOPipe->start();

    //pass1 continuous
    while((acdkMhalGetState() & ACDK_MHAL_PREVIEW_MASK) || (ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState()))
    {
        //====== In PreCapture State Or Not ======
        // 3A need at least 3 frame to change  preview mode to capture mdoe
        if(ACDK_MHAL_PRE_CAPTURE == acdkMhalGetState() && isPreCap == MFALSE && m_i4FrameCount > 2)
        {
            ACDK_LOGD("in precapture state");

            //====== 3A Scenario Control ======

#ifdef ACDK_CAMERA_3A
            m_p3AHal->sendCommand(ECmd_PrecaptureStart);
#endif
            isPreCap = MTRUE;
        }

        //====== Pass1 ======
        ACDK_LOGD("waiting pass1 done");

        //dequeue pass1 OUT buffer
#ifdef ALL_SENSOR_USE_TG1
        mPortID.index = EPortIndex_IMGO;
#else
        if(m_tgi.index == EPortIndex_TG1I)
        {
            mPortID.index = EPortIndex_IMGO;
        }
        else
        {
            mPortID.index = EPortIndex_IMGO_D;
        }
#endif
        m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo,eDrvScenario_CC,6,0);
        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"[ROUND][dequeOutBuf][imgo]mID(0x%x),size(0x%x),addr(0x%x/0x%x),T(%d.%d),img(%d/%d/%d/%d),m_fc(0x%x/0x%x/%d)", \
            rQTSBufInfo.vBufInfo[0].memID[0], \
            rQTSBufInfo.vBufInfo[0].u4BufSize[0], \
            rQTSBufInfo.vBufInfo[0].u4BufVA[0], \
            rQTSBufInfo.vBufInfo[0].u4BufPA[0], \
            rQTSBufInfo.vBufInfo[0].i4TimeStamp_sec, \
            rQTSBufInfo.vBufInfo[0].i4TimeStamp_us, \
            rQTSBufInfo.vBufInfo[0].img_w, \
            rQTSBufInfo.vBufInfo[0].img_h, \
            rQTSBufInfo.vBufInfo[0].img_stride, \
            rQTSBufInfo.vBufInfo[0].img_fmt, \
            rQTSBufInfo.vBufInfo[0].m_num_0, \
            rQTSBufInfo.vBufInfo[0].m_num_1, \
            rQTSBufInfo.vBufInfo[0].frm_cnt);

        //check dequeue buffer number
        if(rQTSBufInfo.vBufInfo.size() == 0)
        {
            ACDK_LOGD("pass1 dequeue no buffer");
            continue;
        }
        else
        {
            imgo_num = (MUINT32)rQTSBufInfo.vBufInfo.size();

            for(MUINT32 i = 0; i < imgo_num; ++i)
            {
                imgo_virAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufVA[0];
                imgo_phyAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufPA[0];
                imgo_memID[i]   = rQTSBufInfo.vBufInfo[i].memID[0];
                ACDK_LOGD_DYN(g_acdkMhalPureDebug,"imgo_virAddr[%d](0x%x),imgo_phyAddr[%d](0x%x)",i,imgo_virAddr[i],i,imgo_phyAddr[i]);
            }
            imgo_virA = imgo_virAddr[0];
            imgo_phyA = imgo_phyAddr[0];
            ACDK_LOGD("imgo_virAddr(%p),imgo_phyAddr(%p) size%d",imgo_virA,imgo_phyA,mAcdkMhalPrvParam.imgImemBuf[0].size);
        }
#ifdef ALL_SENSOR_USE_TG1
        mPortID.index = EPortIndex_RRZO;
#else
        if(m_tgi.index == EPortIndex_TG1I)
        {
            mPortID.index = EPortIndex_RRZO;
        }
        else
        {
            mPortID.index = EPortIndex_RRZO_D;
        }
#endif
        m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo,eDrvScenario_CC,6,0);
        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"[ROUND][dequeOutBuf][rrzo]mID(0x%x),size(0x%x),addr(0x%x/0x%x),T(%d.%d),img(%d/%d/%d/%d),m_fc(0x%x/0x%x/%d)", \
           rQTSBufInfo.vBufInfo[0].memID[0], \
           rQTSBufInfo.vBufInfo[0].u4BufSize[0], \
           rQTSBufInfo.vBufInfo[0].u4BufVA[0], \
           rQTSBufInfo.vBufInfo[0].u4BufPA[0], \
           rQTSBufInfo.vBufInfo[0].i4TimeStamp_sec, \
           rQTSBufInfo.vBufInfo[0].i4TimeStamp_us, \
           rQTSBufInfo.vBufInfo[0].img_w, \
           rQTSBufInfo.vBufInfo[0].img_h, \
           rQTSBufInfo.vBufInfo[0].img_stride, \
           rQTSBufInfo.vBufInfo[0].img_fmt, \
           rQTSBufInfo.vBufInfo[0].m_num_0, \
           rQTSBufInfo.vBufInfo[0].m_num_1, \
           rQTSBufInfo.vBufInfo[0].frm_cnt);

       //check dequeue buffer number
       if(rQTSBufInfo.vBufInfo.size() == 0)
       {
           ACDK_LOGD("pass1 dequeue no buffer");
           continue;
       }
       else
       {
           rrzo_num = (MUINT32)rQTSBufInfo.vBufInfo.size();

           for(MUINT32 i = 0; i < rrzo_num; ++i)
           {
               rrzo_virAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufVA[0];
               rrzo_phyAddr[i] = rQTSBufInfo.vBufInfo[i].u4BufPA[0];
               rrzo_memID[i]   = rQTSBufInfo.vBufInfo[i].memID[0];
               ACDK_LOGD_DYN(g_acdkMhalPureDebug,"rrzo_virAddr[%d](%p),rrzo_phyAddr[%d](%p)",i,imgo_virAddr[i],i,imgo_phyAddr[i]);
           }

       }

        if(dumpEnable == 1 || dumpEnable == 3)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            ACDK_LOGD("prv VA(%p)",imgo_virAddr[0]);

            sprintf(szFileName, "%s/acdkPrv1.bin" , MEDIA_PATH);

            //====== Write RAW Data ======

            FILE *pFp = fopen(szFileName, "wb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
            }
            else
            {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr[0];
                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalPrvParam.imgImemBuf[0].size, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("Save image file name:%s, w(%u), h(%u)", szFileName, m_tgi.u4ImgWidth, m_tgi.u4ImgHeight);

                fclose(pFp);
            }
        }

        //====== 3A Scenario Conotrol ======

#ifdef ACDK_CAMERA_3A
        //m_p3AHal->sendCommand(ECmd_Update);
        ACDK_LOGD("3A update: magic %d", mCurMagicNum);
        m_p3AHal->sendCommand(ECmd_Update, reinterpret_cast<MINTPTR>(&_3A_profile));
#endif
        m_i4FrameCount++;
        ACDK_LOGD("m_i4FrameCount = %d",m_i4FrameCount);

        //====== Pass2 ======
        //====== Config pass2 every time  & enque buffer at the same time ======
#ifdef PASS2_FULLG
        if(is_yuv_sensor == 0)
        {
            m_imgi.u4BufVA[0]     = rrzo_virAddr[0]; //  Vir Address of pool
            m_imgi.u4BufPA[0]     = rrzo_phyAddr[0]; //  Phy Address of pool
            m_imgi.memID[0]       = (MINT32)rrzo_memID[0];
            m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.rrzoImemBuf[0].size;
        }
        else
        {
            m_imgi.u4BufVA[0]     = imgo_virAddr[0]; //  Vir Address of pool
            m_imgi.u4BufPA[0]     = imgo_phyAddr[0]; //  Phy Address of pool
            m_imgi.memID[0]       = (MINT32)imgo_memID[0];
            m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.imgImemBuf[0].size;
        }
#else
        m_imgi.u4BufVA[0]     = imgo_virAddr[0]; //  Vir Address of pool
        m_imgi.u4BufPA[0]     = imgo_phyAddr[0]; //  Phy Address of pool
        m_imgi.memID[0]       = (MINT32)imgo_memID[0];
        m_imgi.u4BufSize[0]   = (MUINT32)mAcdkMhalPrvParam.imgImemBuf[0].size;
#endif
        g_vPostProcInPorts.at(0) = &m_imgi;
        g_vPostProcOutPorts.at(0) = &m_wroto;

        NSImageio::NSIspio::PipePackageInfo pipePackageInfo;
    pipePackageInfo.burstQIdx=0;
    pipePackageInfo.dupCqIdx=0;
    pipePackageInfo.pPrivateData=&g_CookieVa;
    pipePackageInfo.pTuningQue=NULL;
    pipePackageInfo.isV3=0;
        if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts,&pipePackageInfo))
        {
            ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
        }

        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass2 start");
        m_pPostProcPipe->start();

        ACDK_LOGD("waiting pass2 done");
        //dequeue pass2 out buffer
        mPortID.index = EPortIndex_WROTO;
        m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo,eDrvScenario_CC,CAM_ISP_CQ1,0);
        dispo_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        dispo_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        dispo_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

        //dequeue pass2 in buffer
        mPortID.index = EPortIndex_IMGI;
        m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo,eDrvScenario_CC,CAM_ISP_CQ1,0);
        imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        imgi_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

        m_pPostProcPipe->dequeMdpFrameEnd(eDrvScenario_CC);

        ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass2 stop");
        if(rQTSBufInfo.vBufInfo.size() == 0)
        {
            ACDK_LOGD("pass2 deque no buffer");
        }
        else
        {
            //====== Preview CallBack ======
            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"dispo_virAddr(0x%p),dispo_phyAddr(0x%p)", dispo_virAddr,dispo_phyAddr);
            if (delayframecnt>=mu4SensorDelay)
            {
               acdkMhalCBHandle(ACDK_CB_PREVIEW, (MUINT8 *)dispo_virAddr, (MUINT8 *)imgo_virA);
            }
            else
            {
               delayframecnt++;
            }

         }

        //====== Enqueue Pass1 Buffer ======
        rQBufInfo.vBufInfo.resize(1);
        for(MUINT32 i = 0; i < imgo_num; ++i)
        {
            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass1 enque imgo_virAddr[%d](%p)",i,imgo_virAddr[i]);
            if(m_tgi.index == EPortIndex_TG1I)
            {
                mPortID.index = EPortIndex_IMGO;
            }
            else
            {
                mPortID.index = EPortIndex_IMGO_D;
            }
            rQBufInfo.vBufInfo[0].u4BufVA[0]  = imgo_virAddr[i];
            rQBufInfo.vBufInfo[0].u4BufPA[0]  = imgo_phyAddr[i];
            rQBufInfo.vBufInfo[0].memID[0]    = imgo_memID[i];
            rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalPrvParam.imgImemBuf[0].size;
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
            ACDK_LOGD_DYN(g_acdkMhalPureDebug,"pass1 enque imgo_virAddr[%d](%p)",i,imgo_virAddr[i]);
            if(m_tgi.index == EPortIndex_TG1I)
            {
                mPortID.index = EPortIndex_RRZO;
            }
            else
            {
                mPortID.index = EPortIndex_RRZO_D;
            }
            rQBufInfo.vBufInfo[0].u4BufVA[0]  = rrzo_virAddr[i];
            rQBufInfo.vBufInfo[0].u4BufPA[0]  = rrzo_phyAddr[i];
            rQBufInfo.vBufInfo[0].memID[0]    = rrzo_memID[i];
            rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalPrvParam.rrzoImemBuf[0].size;
            m_pICamIOPipe->enqueOutBuf(mPortID, rQBufInfo);
        }

        ACDK_LOGD("pass1 enque done");
        imgo_num = 0;

        //====== Ready for Capture Or Not ======

        if(isPreCap == MTRUE)
        {
#ifdef ACDK_CAMERA_3A
            if(MTRUE == m_p3AHal->isReadyToCapture())
            {
                ACDK_LOGD("ready for capture");

                acdkMhalSetState(ACDK_MHAL_PREVIEW_STOP);

                m_p3AHal->sendCommand(ECmd_PrecaptureEnd);

                mReadyForCap = MTRUE;
                isPreCap = MFALSE;
            }
#else
            mReadyForCap = MTRUE;
#endif
        }
    }
    ACDK_LOGD_DYN(g_acdkMhalPureDebug,"-");

    ACDK_LOGD("xxxxxxxxxxxxxxxx");
    return ACDK_RETURN_NO_ERROR;
}


/*******************************************************************************
* acdkMhalCaptureProc
* brif : handle flow control of capture
*******************************************************************************/
MINT32 AcdkMhalPure::acdkMhalCaptureProc()
{
#if 0
    ACDK_LOGD("+");

    //====== Loca Variable Declaration ======

    MINT32 imgo_memID,imgi_memID,vido_memID;
    MUINT32 imgi_virAddr,imgi_phyAddr,imgo_virAddr,imgo_phyAddr,vido_virAddr,vido_phyAddr;
    QBufInfo rQBufInfo;
    QTimeStampBufInfo rQTSBufInfo;

    //====== Variable Setting ======

    g_vPostProcInPorts.resize(1);
    g_vPostProcOutPorts.resize(1);

    //====== Get Property ======

    char value[PROPERTY_VALUE_MAX] = {'\0'};

    property_get("camera.acdkdump.enable", value, "0");
    MINT32 dumpEnable = atoi(value);

    //====== Pass1 ======

    //set pass1 out DMA register before CQ0 start
    //use CQ0 to update DMA register
    ACDK_LOGD("EPIPECmd_SET_CURRENT_BUFFER");
    m_pICamIOPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGO,0,0);

    ACDK_LOGD("CQ0 start");

    m_pICamIOPipe->startCQ0();

    ACDK_LOGD("pass1 start");

    //3AHal control
    m_p3AHal->setIspProfile(EIspProfile_NormalCapture);

    m_pICamIOPipe->start(); //one time capture

    //3A scenario control
    m_p3AScenario->sendCommand(ECmd_CaptureStart);

    ACDK_LOGD("waiting pass1 done");

#if 0 // all these 3 steps are not required because FBC is implemented in CamIOPipe

    m_pICamIOPipe->irq(EPipePass_PASS1_TG1,EPIPEIRQ_PATH_DONE,ISP_DRV_IRQ_USER_ACDK);

    ACDK_LOGD("zsd pass1 done");
#endif
    //dequeue pass1 OUT buffer
    mPortID.index = EPortIndex_IMGO;
    m_pICamIOPipe->dequeOutBuf(mPortID, rQTSBufInfo);

    //MUST do
    ACDK_LOGD("clear VFDATA_EN bit");
    m_pICamIOPipe->stop();

    if(rQTSBufInfo.vBufInfo.size() == 0)
    {
        ACDK_LOGD("pass1 done deque no buffer");
    }
    else
    {
        imgo_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
        imgo_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
        imgo_memID = rQTSBufInfo.vBufInfo[0].memID[0];
        ACDK_LOGD("pass1 done,imgo_virAddr(0x%x)",imgo_virAddr);

        if(dumpEnable == 4)
        {
            char szFileName[256];
            MINT32 i4WriteCnt = 0;

            ACDK_LOGD("prv VA(0x%x)",imgo_virAddr);

            sprintf(szFileName, "%s/acdkCap1.bin" , MEDIA_PATH);

            //====== Write RAW Data ======

            FILE *pFp = fopen(szFileName, "wb");

            if(NULL == pFp )
            {
                ACDK_LOGE("Can't open file to save image");
                fclose(pFp);
            }
            else
            {
                MUINT8 *pVirBufin = (MUINT8 *)imgo_virAddr;
                i4WriteCnt = fwrite(pVirBufin, 1, mAcdkMhalCapParam.capPass1ImemBuf.size, pFp);

                fflush(pFp);

                if(0 != fsync(fileno(pFp)))
                {
                    ACDK_LOGE("[.capPass1ImemBuf.size] fync fail");
                    fclose(pFp);
                    return ACDK_RETURN_NULL_OBJ;
                }

                ACDK_LOGD("[.capPass1ImemBuf.size] Save image file name:%s, w(%u), h(%u)", szFileName, m_tgi.u4ImgWidth, m_tgi.u4ImgHeight);

                fclose(pFp);
            }
        }

        if(mAcdkMhalCapParam.captureType == RAW_TYPE)
        {
            //====== Raw Capture Callback ======

            acdkMhalCBHandle(ACDK_CB_RAW, imgo_virAddr);

            ACDK_LOGD("ACDK_CB_RAW back");
        }

        //====== Pass2 ======

        for(MUINT32 i = 0; i < 2; ++i)
        {
            //enqueue pass2 IN buffer
            rQBufInfo.vBufInfo.resize(1);

            mPortID.index = EPortIndex_IMGI;
            rQBufInfo.vBufInfo[0].u4BufVA[0] = imgo_virAddr;
            rQBufInfo.vBufInfo[0].u4BufPA[0] = imgo_phyAddr;
            rQBufInfo.vBufInfo[0].memID[0]   = imgo_memID;
            rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalCapParam.capPass1ImemBuf.size;
            m_pPostProcPipe->enqueInBuf(mPortID, rQBufInfo);

            ACDK_LOGD("Update pass2 tuning cmdQ");

            //config pass2 every time

            g_vPostProcInPorts.at(0)  = &m_imgi;
            g_vPostProcOutPorts.at(0) = &m_vido;

            if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
            {
                ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
            }

            //set pass2 IN DMA register before pass2 start
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_IMGI,0,0);

            //set pass2 OUT DMA register before pass2 start
            //m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_DISPO,0,0);
            m_pPostProcPipe->sendCommand((MINT32)EPIPECmd_SET_CURRENT_BUFFER, (MINT32)EPortIndex_VIDO,0,0);

            ACDK_LOGD("pass2 start");

            m_pPostProcPipe->start();

            ACDK_LOGD("waiting pass2 done");

            m_pPostProcPipe->irq(EPipePass_PASS2,EPIPEIRQ_PATH_DONE,ISP_DRV_IRQ_USER_ACDK);

            ACDK_LOGD("pass2 done");

            //====== Dequeue ======

            //dequeue pass2 out buffer
            mPortID.index = EPortIndex_VIDO;
            m_pPostProcPipe->dequeOutBuf(mPortID, rQTSBufInfo);
            vido_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
            vido_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
            vido_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

            //dequeue pass2 in buffer
            mPortID.index = EPortIndex_IMGI;
            m_pPostProcPipe->dequeInBuf(mPortID, rQTSBufInfo);
            imgi_virAddr = rQTSBufInfo.vBufInfo[0].u4BufVA[0];
            imgi_phyAddr = rQTSBufInfo.vBufInfo[0].u4BufPA[0];
            imgi_memID   = rQTSBufInfo.vBufInfo[0].memID[0];

            m_pPostProcPipe->stop();
            ACDK_LOGD("pass2 stop");

            //====== Capture Callback ======

            ACDK_LOGD("vido_virAddr(0x%x)", vido_virAddr);

            if(mAcdkMhalCapParam.captureType == RAW_TYPE || i == 1)
            {
                acdkMhalCBHandle(ACDK_CB_QV, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_QV back");

                break;
            }
            else
            {
                acdkMhalCBHandle(ACDK_CB_JPEG, vido_virAddr);

                ACDK_LOGD("pass2 ACDK_CB_JPEG back");

                //====== Config PostProcPipe 2nd Run for QV after JPEG Capture ======

                m_imgi.eImgFmt     = m_imgo.eImgFmt;        //  Image Pixel Format
                m_imgi.u4ImgWidth  = m_imgo.u4ImgWidth;     //  Image Width
                m_imgi.u4ImgHeight = m_imgo.u4ImgHeight;    //  Image Height
                m_imgi.u4Stride[ESTRIDE_1ST_PLANE] = m_imgo.u4Stride[ESTRIDE_1ST_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_2ND_PLANE] = m_imgo.u4Stride[ESTRIDE_2ND_PLANE];    // unit:PIXEL
                m_imgi.u4Stride[ESTRIDE_3RD_PLANE] = m_imgo.u4Stride[ESTRIDE_3RD_PLANE];    // unit:PIXEL
                m_imgi.crop.x = 0;                          // unit:PIXEL
                m_imgi.crop.y = 0;                          // unit:PIXEL
                m_imgi.crop.w = m_imgi.u4ImgWidth;          // unit:PIXEL
                m_imgi.crop.h = m_imgi.u4ImgHeight;         // unit:PIXEL
                m_imgi.type        = EPortType_Memory;      //  EPortType
                m_imgi.index       = EPortIndex_IMGI;       //  port index
                m_imgi.inout       = EPortDirection_In;     //  0:in/1:out
                m_imgi.pipePass    = EPipePass_PASS2;       //  select pass

                // config qv
                m_vido.eImgFmt     = (EImageFormat)mAcdkMhalCapParam.qvParam.frmFormat;   // Image Pixel Format
                m_vido.eImgRot     = (EImageRotation)mAcdkMhalCapParam.qvParam.orientation;    //dispo NOT support rotation
                m_vido.eImgFlip    = mAcdkMhalCapParam.qvParam.flip ? eImgFlip_ON : eImgFlip_OFF;  //dispo NOT support flip
                m_vido.u4ImgWidth  = mAcdkMhalCapParam.qvParam.w;           // Image Width
                m_vido.u4ImgHeight = mAcdkMhalCapParam.qvParam.h;           // Image Height
                m_vido.u4Stride[ESTRIDE_1ST_PLANE] = mAcdkMhalCapParam.qvParam.stride[0];
                m_vido.u4Stride[ESTRIDE_2ND_PLANE] = mAcdkMhalCapParam.qvParam.stride[1];
                m_vido.u4Stride[ESTRIDE_3RD_PLANE] = mAcdkMhalCapParam.qvParam.stride[2];
                m_vido.type        = EPortType_DISP_RDMA;   // EPortType
                m_vido.index       = EPortIndex_VIDO;       // port index
                m_vido.inout       = EPortDirection_Out;    // 0:in/1:out

                //set CQ first before pipe config
                m_pPostProcPipe->sendCommand(EPIPECmd_SET_CQ_CHANNEL,(MINT32)EPIPE_PASS2_CQ1,0,0);

                // config IO port
                g_vPostProcInPorts.at(0) = &m_imgi;
                g_vPostProcOutPorts.at(0) = &m_vido;

                if(MTRUE != m_pPostProcPipe->configPipe(g_vPostProcInPorts, g_vPostProcOutPorts))
                {
                    ACDK_LOGE("m_pPostProcPipe->configPipe() fail");
                    break;
                }

                // enque buffer
                ACDK_LOGD("capQVImemBuf : virAddr=0x%x",mAcdkMhalCapParam.capQVImemBuf.virtAddr);

                mPortID.index = EPortIndex_VIDO;    //VIDO
                rQBufInfo.vBufInfo.resize(1);
                rQBufInfo.vBufInfo[0].u4BufSize[0] = mAcdkMhalCapParam.capQVImemBuf.size;    //bytes
                rQBufInfo.vBufInfo[0].u4BufVA[0]   = mAcdkMhalCapParam.capQVImemBuf.virtAddr;
                rQBufInfo.vBufInfo[0].u4BufPA[0]   = mAcdkMhalCapParam.capQVImemBuf.phyAddr;
                rQBufInfo.vBufInfo[0].memID[0]     = mAcdkMhalCapParam.capQVImemBuf.memID;

                m_pPostProcPipe->enqueOutBuf(mPortID, rQBufInfo);
            }
        }
    }

    ACDK_LOGD("-");
#endif
    return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetShutTime()
{
#ifdef ACDK_CAMERA_3A
    m_p3AHal->getCaptureParams(mCap3AParam);

    ACDK_LOGD("mCap3AParam.u4Eposuretime(%u)",mCap3AParam.u4Eposuretime);

    return mCap3AParam.u4Eposuretime;
#endif
return ACDK_RETURN_NO_ERROR;
}

/*******************************************************************************
*
*******************************************************************************/
MVOID AcdkMhalPure::acdkMhalSetShutTime(MUINT32 a_time)
{
#ifdef ACDK_CAMERA_3A
    // get and then set
    m_p3AHal->getCaptureParams(mCap3AParam);

    ACDK_LOGD("ori(%u),new(%u)",mCap3AParam.u4Eposuretime,a_time);

    mCap3AParam.u4Eposuretime = a_time;
    m_p3AHal->updateCaptureParams(mCap3AParam);
#endif
}

/*******************************************************************************
*
*******************************************************************************/
MUINT32 AcdkMhalPure::acdkMhalGetAFInfo()
{
    MUINT32 u32AFInfo;

    ACDK_LOGD("acdkMhalGetAFInfo");
    u32AFInfo = g_pAcdkMHalPureObj->mFocusSucceed;

    return u32AFInfo;

}
/*******************************************************************************
* doNotifyCb
*******************************************************************************/
void AcdkMhalPure::doNotifyCb(int32_t _msgType,
                                 int32_t _ext1,
                                 int32_t _ext2,
                                 int32_t _ext3)
{
    ACDK_LOGD("_msgType(%d),_ext1(%d)",_msgType,_ext1);

#ifdef ACDK_CAMERA_3A
    if(_msgType == eID_NOTIFY_AF_FOCUSED)
    {
        if(_ext1 == 1)
        {
            g_pAcdkMHalPureObj->mFocusSucceed = MTRUE;
            ACDK_LOGD("Focus Succeed");
        }
        else if(_ext1 == 0)
        {
            g_pAcdkMHalPureObj->mFocusSucceed = MFALSE;
            ACDK_LOGD("Focus Fail");
        }

        g_pAcdkMHalPureObj->mFocusDone = MTRUE;
    }
#endif
    ACDK_LOGD("-");
}

/*******************************************************************************
* doDataCb
*******************************************************************************/
void AcdkMhalPure::doDataCb(int32_t _msgType,
                               void *_data,
                               uint32_t _size)
{

}



