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
#define LOG_TAG "ThreadSensorI2CImp"

#include <IThread.h>
#include <pthread.h>
#include <semaphore.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <isp_tuning.h>
#include <ae_param.h>
#include <aaa_sensor_mgr.h>

namespace NS3Av3
{
using namespace android;

class ThreadSensorI2CImp : public ThreadSensorI2C
{
public:
    //static ThreadSensorI2CImp*  createInstance(MUINT32 u4SensorDev);
    virtual MBOOL               destroyInstance();
    virtual MBOOL               postCmd(void* pArg);
    virtual MBOOL               waitFinished();

                                ThreadSensorI2CImp(MUINT32 u4SensorDev);
    virtual                     ~ThreadSensorI2CImp();
private:
    static MVOID*               onThreadLoop(MVOID*);
    MBOOL                       getCmd(ThreadSensorI2C::Cmd_T& rCmd);

    MBOOL                       m_fgLogEn;
    IThread*                    m_pThreadGain;
    MUINT32                     m_u4SensorDev;
    pthread_t                   m_rThread;
    List<ThreadSensorI2C::Cmd_T>  m_rCmdQ;
    Mutex                       m_ModuleMtx;
    Condition                   m_rCmdQCond;
    sem_t                       m_SemFinish;
};

ThreadSensorI2C*
ThreadSensorI2C::
createInstance(MUINT32 u4SensorDev)
{
    ThreadSensorI2CImp* pObj = new ThreadSensorI2CImp(u4SensorDev);
    return pObj;
}

MBOOL
ThreadSensorI2CImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

MBOOL
ThreadSensorI2CImp::
postCmd(void* pArg)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    ThreadSensorI2C::Cmd_T rCmd;
    // data clone
    if (pArg != NULL)
    {
        rCmd = *reinterpret_cast<ThreadSensorI2C::Cmd_T*>(pArg);
        rCmd.fgTerminate = 0;
    }

    m_rCmdQ.push_back(rCmd);
    m_rCmdQCond.broadcast();

    MY_LOG_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), magic(%d), exp(%d), gain(%d), TG(%d)",
        __FUNCTION__, m_rCmdQ.size(), rCmd.u4MagicNum, rCmd.u4ExposureTime, rCmd.u4SensorGain, rCmd.i4TgInfo);
    return MTRUE;
}

MBOOL
ThreadSensorI2CImp::
getCmd(ThreadSensorI2C::Cmd_T& rCmd)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    m_rCmdQCond.wait(m_ModuleMtx);

    rCmd = *m_rCmdQ.begin();
    MY_LOG_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), magic(%d), exp(%d), gain(%d), TG(%d)",
        __FUNCTION__, m_rCmdQ.size(), rCmd.u4MagicNum, rCmd.u4ExposureTime, rCmd.u4SensorGain, rCmd.i4TgInfo);

    m_rCmdQ.erase(m_rCmdQ.begin());

    if (rCmd.fgTerminate)
    {
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
ThreadSensorI2CImp::
waitFinished()
{
    ::sem_wait(&m_SemFinish);
    return MTRUE;
}

ThreadSensorI2CImp::
ThreadSensorI2CImp(MUINT32 u4SensorDev)
    : m_fgLogEn(0)
    , m_pThreadGain(NULL)
    , m_u4SensorDev(u4SensorDev)
{
    // init something
    ::sem_init(&m_SemFinish, 0, 0);
    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);
}

ThreadSensorI2CImp::
~ThreadSensorI2CImp()
{
    // destroy thread
    postCmd(NULL);
    ::sem_post(&m_SemFinish);
    ::pthread_join(m_rThread, NULL);
    // uninit something
}

MVOID*
ThreadSensorI2CImp::
onThreadLoop(MVOID* pArg)
{
    ThreadSensorI2CImp* _this = reinterpret_cast<ThreadSensorI2CImp*>(pArg);
    ThreadSensorI2C::Cmd_T rCmd;
    MUINT32 u4SensorDev = _this->m_u4SensorDev;

    while (_this->getCmd(rCmd))
    {
        // apply sensor setting
        AAASensorMgr::getInstance().setSensorExpTime(u4SensorDev, rCmd.u4ExposureTime);
        // post to gain thread if needed.
        IThread* pGainThread = _this->m_pThreadGain;
        if (pGainThread)
        {
            ThreadSensorGain::Cmd_T rCmdGain(rCmd.u4MagicNum, rCmd.u4ExposureTime, rCmd.u4SensorGain, rCmd.i4TgInfo);
            pGainThread->postCmd(&rCmdGain);
        }
        else
        {
            AAASensorMgr::getInstance().setSensorGain(u4SensorDev, rCmd.u4SensorGain);
        }
        ::sem_post(&_this->m_SemFinish);
    }
    return NULL;
}
};
