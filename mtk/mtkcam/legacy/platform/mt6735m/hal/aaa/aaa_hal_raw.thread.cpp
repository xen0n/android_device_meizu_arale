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

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
#define LOG_TAG "Hal3ARaw/thread"

#include <utils/threads.h>
#include <utils/List.h>
#include <mtkcam/v1/config/PriorityDefs.h>
#include <mtkcam/utils/common.h>
using namespace android;
using namespace NSCam::Utils;

//
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <hal/aaa/aaa_hal_raw.h>
#include <hal/aaa/IEventIrq.h>
#include <hal/aaa/ae_mgr/ae_mgr_if.h>

//#include <mtkcam/core/featureio/pipe/aaa/state_mgr/aaa_state.h>
#include <hal/aaa/state_mgr/aaa_state_mgr.h>
//#include <mtkcam/core/featureio/pipe/aaa/aaa_scheduler.h>
#include <mtkcam/Trace.h>


using namespace NS3Av3;
//
//
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>
/*
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/mt_sched.h>
*/
/******************************************************************************
*
*******************************************************************************/
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)

#if defined(HAVE_AEE_FEATURE)
#include <aee.h>
#define AEE_ASSERT_3A_HAL(String) \
          do { \
              aee_system_exception( \
                  "Hal3A", \
                  NULL, \
                  DB_OPT_DEFAULT, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3ARaw::createThreadRaw()
{
    //sem_init(&mSem, 0, 0);
    sem_init(&semAFProcEnd, 0, 1);
    sem_init(&semAESen, 0, 0);
    sem_init(&semAESenThdEnd, 0, 1);
    //pthread_create(&mThread, NULL, onThreadLoop, this);
    pthread_create(&mAESenThread, NULL, AESensorThreadLoop, this);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3ARaw::destroyThreadRaw()
{
    MY_LOG("[%s] +", __FUNCTION__);

    mbEnAESenThd = MFALSE;
    ::sem_post(&semAESen);
    pthread_join(mAESenThread, NULL);

    //
    MY_LOG("[%s] -", __FUNCTION__);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3ARaw::changeAESensorThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"AESenThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_3A_MAIN;
        int policy = 0, priority = 0;
        setThreadPriority(expect_policy, expect_priority);
        getThreadPriority(policy, priority);
        //
        MY_LOG(
            "[Hal3A::onThreadLoop] policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
            , expect_policy, policy, expect_priority, priority
        );
    }
/*
    cpu_set_t cmask, mt_get_cmask, mt_set_cmask;
    unsigned long len = sizeof(cmask);
    int cpu, status;
    CPU_ZERO(&mt_set_cmask);
    CPU_ZERO(&mt_get_cmask);
    CPU_ZERO(&cmask);

    cpu = 1;
    CPU_SET(cpu, &mt_set_cmask);
    status = mt_sched_setaffinity(0, len, &mt_set_cmask);
    if(!status){
        MY_LOG("set affinity to CPU(%d) successfully, status=%d\n", cpu, status);
    }else{
        MY_LOG("set affinity to CPU(%d) FAIL, status=%d\n", cpu, status);
    }

    status = mt_sched_getaffinity(0, len, &cmask, &mt_get_cmask);
    MY_LOG("get affinity: status=%d, CPU_ISSET(cpu, &cmask)=%d, CPU_EQUAL(&mt_get_cmask, &mt_set_cmask)=%d\n",
        status, CPU_ISSET(cpu, &cmask), CPU_EQUAL(&mt_get_cmask, &mt_set_cmask));
*/
}
/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3ARaw::sendCommandDerived(ECmd_T const r3ACmd, MINTPTR const i4Arg)
{
    MY_LOG_IF(m_3ALogEnable, "[%s] r3ACmd(%d)\n", __FUNCTION__, r3ACmd);
    if (r3ACmd == ECmd_CameraPreviewStart)
    {
        //set semAESenThdEnd to be 1, to avoid the deadlock caused by no AE post from PreviewStart to PreviewEnd
        int Val;
        ::sem_getvalue(&semAESenThdEnd, &Val);
        if (Val == 0) ::sem_post(&semAESenThdEnd); //to be 1
        MY_LOG_IF(1, "[Hal3A::sendCommandDerived] PreviewStart semAESenThdEnd(%d) before post", Val);
        //sem_init(&semAESenThdEnd, 0, 1);

        mbPostAESenThd = MFALSE;

        //set semAFProcEnd to be 1, to avoid the deadlock caused by no AF post from PreviewStart to PreviewEnd
        ::sem_getvalue(&semAFProcEnd, &Val);
        if (Val == 0) ::sem_post(&semAFProcEnd); //to be 1
        MY_LOG_IF(1, "[Hal3A::sendCommandDerived] PreviewStart semAFProcEnd(%d) before post", Val);

        mi4InCaptureProcess = 0;
    }

    if (r3ACmd == ECmd_CameraPreviewEnd)
    {
        EState_T eCurState = mpStateMgr->getStateStatus().eCurrState;
        MY_LOG_IF(1, "[%s] ECmd_CameraPreviewEnd: eCurState(%d)", __FUNCTION__, eCurState);
        switch (eCurState)
        {
        case eState_AF:
            cancelAutoFocus();
            break;
        case eState_Precapture:
            mpStateMgr->transitState(eState_Precapture, eState_CameraPreview);
            break;
        default:
            break;
        }
    }

    return MTRUE;
}
/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3ARaw::AESensorThreadLoop(MVOID *arg)
{
    Hal3ARaw *_this = reinterpret_cast<Hal3ARaw*>(arg);
    // (1) change thread setting
    _this->changeAESensorThreadSetting();
    MBOOL bRet;

    // (2) thread-in-loop
    while(1)
    {
        ::sem_wait(&_this->semAESen);
        if ( ! _this->mbEnAESenThd) break;

        MY_LOG_IF(_this->m_3ALogEnable, "[AESensorThreadLoop] updateSensorbyI2C\n");
        CAM_TRACE_BEGIN("AE Sensor I2C");
        IAeMgr::getInstance().updateSensorbyI2C(_this->m_i4SensorDev);
        CAM_TRACE_END();

        //sync with 3A thread when PreviewEnd
        {
            int Val;
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AESensorThreadLoop] start waiting mAESenMtx lock (3)\n");
            Mutex::Autolock autoLock(_this->mAESenMtx);
            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "[Hal3A::AESensorThreadLoop] get mAESenMtx lock (3)\n");

            ::sem_getvalue(&_this->semAESenThdEnd, &Val);
            MY_LOG_IF(_this->m_3ALogEnable, "[AESensorThreadLoop] semAESenThdEnd before post = %d\n", Val);
            if (Val == 0) ::sem_post(&_this->semAESenThdEnd); //to be 1, 1 means AE set Sensor done, this can tolerate I2C delay too long
        }
    }
    return NULL;
}

MVOID
Hal3ARaw::postToAESenThread()
{
    Mutex::Autolock autoLock(mAESenMtx);
    int Val;
    ::sem_getvalue(&semAESenThdEnd, &Val);
    if (Val == 1) ::sem_wait(&semAESenThdEnd); //to be 0, it won't block, 0 means AE set Sensor not ready yet, this can tolerate I2C delay too long
    ::sem_post(&semAESen);
    MY_LOG_IF(m_3ALogEnable, "[postToAESenThread] sem_post semAESen, semAESenThdEnd before wait = %d\n", Val);
}




