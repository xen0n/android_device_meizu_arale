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
#define LOG_TAG "Drv/HWsync.TD"

#include <mtkcam/common.h>
#include <mtkcam/utils/common.h>
//thread
#include <utils/threads.h>
#include <semaphore.h>
#include <mtkcam/v1/config/PriorityDefs.h>
//thread priority
#include <system/thread_defs.h>
#include <utils/ThreadDefs.h>
// For property_get().
#include <cutils/properties.h>
#include "hwsync_drv_imp.h"
#include <sys/prctl.h>
/*************************************************************************************
* Log Utility
*************************************************************************************/
#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#define  DBG_LOG_TAG        LOG_TAG

#include "../../core/drv/inc/drv_log.h"    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.
DECLARE_DBG_LOG_VARIABLE(hwsyncDrvTD);
// Clear previous define, use our own define.
#undef LOG_VRB
#undef LOG_DBG
#undef LOG_INF
#undef LOG_WRN
#undef LOG_ERR
#undef LOG_AST
#define LOG_VRB(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_VERBOSE) { BASE_LOG_VRB(fmt, ##arg); } } while(0)
#define LOG_DBG(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_DEBUG  ) { BASE_LOG_DBG(fmt, ##arg); } } while(0)
#define LOG_INF(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_INFO   ) { BASE_LOG_INF(fmt, ##arg); } } while(0)
#define LOG_WRN(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_WARN   ) { BASE_LOG_WRN(fmt, ##arg); } } while(0)
#define LOG_ERR(fmt, arg...)        do { if (hwsyncDrvTD_DbgLogEnable_ERROR  ) { BASE_LOG_ERR(fmt, ##arg); } } while(0)
#define LOG_AST(cond, fmt, arg...)  do { if (hwsyncDrvTD_DbgLogEnable_ASSERT ) { BASE_LOG_AST(cond, fmt, ##arg); } } while(0)

extern sem_t gsemHWSyncLv;
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::createThread()
{
    pthread_create(&mThread, NULL, onThreadLoop, this);
}

/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::destroyThread()
{
    // post exit means all user call uninit, clear all cmds in cmdlist and add uninit
    clearExecCmds();
    ExecCmd cmd=ExecCmd_LEAVE;
    addExecCmd(cmd);
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::
addExecCmd(
    ExecCmd const &cmd)
{
    LOG_INF("+");
    Mutex::Autolock autoLock(mThreadMtx);
    //[1] add specific command to cmd list
    LOG_DBG("mCmdList, size(%d) + cmd(%d)", mExecCmdList.size(), cmd);
    mExecCmdList.push_back(cmd);
    //[2] broadcast signal to user that a command is enqueued in cmd list
    mExecCmdListCond.broadcast();
}
/******************************************************************************
 *
 ******************************************************************************/
MVOID
HWSyncDrvImp::
clearExecCmds()
{
    Mutex::Autolock autoLock(mThreadMtx);
    //[1] erase all the command in cmd list
    for (list<ExecCmd>::iterator it = mExecCmdList.begin(); it != mExecCmdList.end();)
    {
        it = mExecCmdList.erase(it);
        it++;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
MBOOL
HWSyncDrvImp::
get1stCmd(
    ExecCmd &cmd)
{
    Mutex::Autolock autoLock(mThreadMtx);
    //[1] check if there is command in cmd list
    if (mExecCmdList.size() <= 0)
    {
        LOG_INF("no more cmd, wait here");
        mExecCmdListCond.wait(mThreadMtx);   //wait for someone add enqueue cmd to list
    }
    //[2] get out the first command in correponding cmd list (first in first service)
    if(mExecCmdList.size()>0)
    {
        LOG_INF("get 1st cmd(%d)",*mExecCmdList.begin());
        cmd = *mExecCmdList.begin();
        mExecCmdList.erase(mExecCmdList.begin());
        //[3] condition that thread loop keep doing dequeue or not
        if (cmd == ExecCmd_LEAVE)
        {
            return MFALSE;
        }
        else
        {
            return MTRUE;
        }
    }
    else
    {
        cmd=ExecCmd_UNKNOWN;
        return MTRUE;
    }
}

/******************************************************************************
 *
 ******************************************************************************/
#define PR_SET_NAME 15
MVOID*
HWSyncDrvImp::
onThreadLoop(
    MVOID *arg)
{
    //[1] set thread
    // set thread name
    ::prctl(PR_SET_NAME,"HWSyncDrvThread",0,0,0);
    // set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_PASS1;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    LOG_INF(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
    //  detach thread => cannot be join, it means that thread would release resource after exit
    ::pthread_detach(::pthread_self());

    //[2] do sync if there is enqueue cmd in cmd list
    HWSyncDrvImp *_this = reinterpret_cast<HWSyncDrvImp*>(arg);
    ExecCmd cmd;
    ExecCmd nextCmd=ExecCmd_CHECKDIFF;
    //_this->addExecCmd(nextCmd);   //add the first cmd from changemode function
    while(_this->get1stCmd(cmd))
    {
        LOG_DBG("cmd(%d)",cmd);
        switch(cmd)
        {
            case ExecCmd_CHECKDIFF:
                _this->checkNeedSync();
                break;
            case ExecCmd_DOSYNC:
                _this->doGeneralSync();
                _this->m1stSync=false;
                nextCmd=ExecCmd_CHECKDIFF;
                _this->addExecCmd(nextCmd);
                break;
            case ExecCmd_UNKNOWN:
            default:
                break;
        }
        cmd=ExecCmd_UNKNOWN;//prevent that condition wait in "getfirstCommand" get fake signal
    }
EXIT:
    _this->clearExecCmds();
    _this->revertSensorFps();
    //send leave signal
    ::sem_post(&(_this->mSemHWSyncLv));
    LOG_INF("-");
    return NULL;
}

