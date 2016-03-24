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
#define LOG_TAG "ThreadSensorGainImp"

#include <IThread.h>
#include <IEventIrq.h>

#include <mtkcam/v1/config/PriorityDefs.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <cutils/properties.h>
#include <utils/threads.h>
#include <utils/List.h>
#include <aaa_log.h>
#include <aaa_error_code.h>
#include <isp_tuning.h>
#include <ae_param.h>
#include <aaa_sensor_mgr.h>

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

namespace NS3Av3
{
using namespace android;

class ThreadSensorGainImp : public ThreadSensorGain
{
public:
    virtual MBOOL               destroyInstance();
    virtual MBOOL               postCmd(void* pArg);
    virtual MBOOL               waitFinished();

                                ThreadSensorGainImp(MUINT32 u4SensorDev);
    virtual                     ~ThreadSensorGainImp();
private:
    static MVOID*               onThreadLoop(MVOID*);
    MVOID                       changeThreadSetting();
    MBOOL                       getCmd(ThreadSensorGain::Cmd_T& rCmd);

    MBOOL                       m_fgLogEn;
    MUINT32                     m_u4SensorDev;
    IEventIrq*                  m_pEvent;
    pthread_t                   m_rThread;
    List<ThreadSensorGain::Cmd_T>  m_rCmdQ;
    Mutex                       m_ModuleMtx;
    Condition                   m_rCmdQCond;
    //sem_t                       m_SemFinish;
};

ThreadSensorGain*
ThreadSensorGain::
createInstance(MUINT32 u4SensorDev)
{
    ThreadSensorGainImp* pObj = new ThreadSensorGainImp(u4SensorDev);
    return pObj;
}

MBOOL
ThreadSensorGainImp::
destroyInstance()
{
    delete this;
    return MTRUE;
}

MBOOL
ThreadSensorGainImp::
postCmd(void* pArg)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    ThreadSensorGain::Cmd_T rCmd;
    // data clone
    if (pArg != NULL)
    {
        rCmd = *reinterpret_cast<ThreadSensorGain::Cmd_T*>(pArg);
        rCmd.fgTerminate = 0;
    }

    m_rCmdQ.push_back(rCmd);
    m_rCmdQCond.broadcast();

    MY_LOG_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), magic(%d), exp(%d), gain(%d)",
        __FUNCTION__, m_rCmdQ.size(), rCmd.u4MagicNum, rCmd.u4ExposureTime, rCmd.u4SensorGain);
    return MTRUE;
}

MBOOL
ThreadSensorGainImp::
getCmd(ThreadSensorGain::Cmd_T& rCmd)
{
    Mutex::Autolock autoLock(m_ModuleMtx);

    if (m_rCmdQ.size() == 0)
    {
        m_rCmdQCond.wait(m_ModuleMtx);
    }

    rCmd = *m_rCmdQ.begin();
    MY_LOG_IF(m_fgLogEn, "[%s] m_rCmdQ size(%d), magic(%d), exp(%d), gain(%d)",
        __FUNCTION__, m_rCmdQ.size(), rCmd.u4MagicNum, rCmd.u4ExposureTime, rCmd.u4SensorGain);

    m_rCmdQ.erase(m_rCmdQ.begin());

    if (rCmd.fgTerminate)
    {
        return MFALSE;
    }
    return MTRUE;
}

MBOOL
ThreadSensorGainImp::
waitFinished()
{
    //::sem_wait(&m_SemFinish);
    return MTRUE;
}

ThreadSensorGainImp::
ThreadSensorGainImp(MUINT32 u4SensorDev)
    : m_fgLogEn(0)
    , m_u4SensorDev(u4SensorDev)
{
    MY_LOG("[%s] sensor(%d)", __FUNCTION__, u4SensorDev);
    // init something
    MUINT32 u4LogEn = 0;
    GET_PROP("debug.tsg.log", "0", u4LogEn);
    m_fgLogEn = u4LogEn ? 1 : 0;
    IEventIrq::ConfigParam IrqConfig(u4SensorDev, 0, IEventIrq::E_Event_Vsync_Sensor);
    m_pEvent = IEventIrq::createInstance(IrqConfig, "VS_GAIN");
    //::sem_init(&m_SemFinish, 0, 0);
    // create thread
    ::pthread_create(&m_rThread, NULL, onThreadLoop, this);
}

ThreadSensorGainImp::
~ThreadSensorGainImp()
{
    MY_LOG("[%s +] sensor(%d)", __FUNCTION__, m_u4SensorDev);
    // destroy thread
    postCmd(NULL);
    //::sem_post(&m_SemFinish);
    ::pthread_join(m_rThread, NULL);
    // uninit something
    m_pEvent->destroyInstance("VS_GAIN");
    MY_LOG("[%s -] sensor(%d)", __FUNCTION__, m_u4SensorDev);
}

MVOID
ThreadSensorGainImp::
changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME, "SenGainThread", 0, 0, 0);
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);

    // (2) set policy/priority
#if 1
    int const policy    = SCHED_RR;
    int const priority  = PRIO_RT_3A_THREAD;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //  get
    ::sched_getparam(0, &sched_p);
#else
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_3A;
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is nice value.
    ::sched_setscheduler(0, policy, &sched_p);
    ::setpriority(PRIO_PROCESS, 0, priority);
    //  get
    sched_p.sched_priority = ::getpriority(PRIO_PROCESS, 0);
#endif
    //
    MY_LOG(
        "[%s] sensor(%d), tid(%d), policy:(expect, result)=(%d, %d), priority:(expect, result)=(0x%08x, 0x%08x)", __FUNCTION__
        , m_u4SensorDev, ::gettid()
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}

MVOID*
ThreadSensorGainImp::
onThreadLoop(MVOID* pArg)
{
    ThreadSensorGainImp* _this = reinterpret_cast<ThreadSensorGainImp*>(pArg);
    MUINT32 u4SensorDev = _this->m_u4SensorDev;
    IEventIrq::Duration rDuration;
    ThreadSensorGain::Cmd_T rCmd;

    _this->changeThreadSetting();

    while (_this->getCmd(rCmd))
    {
        // wait next SOF
        _this->m_pEvent->wait(rDuration);
        AAASensorMgr::getInstance().setSensorGain(u4SensorDev, rCmd.u4SensorGain);
        //::sem_post(&_this->m_SemFinish);
    }

    MY_LOG("[%s] Done", __FUNCTION__);
    return NULL;
}

};
