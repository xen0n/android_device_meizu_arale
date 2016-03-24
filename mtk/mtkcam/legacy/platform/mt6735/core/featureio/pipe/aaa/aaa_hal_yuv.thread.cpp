/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "aaa_hal_yuv/3Athread"
//
#include <utils/threads.h>
#include <utils/List.h>
using namespace android;
//
#include <mtkcam/v1/config/PriorityDefs.h>
//
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal_yuv.h"
using namespace NS3A;
//
//
#ifdef USE_3A_THREAD

#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

/******************************************************************************
*
*******************************************************************************/
#ifndef ENABLE_MY_LOG
    #define ENABLE_MY_LOG   (1)
#endif
#define LOG_PERFRAME_ENABLE (1)
/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AYuv::createThread()
{
    sem_init(&mSem, 0, 0);
    pthread_create(&mThread, NULL, onThreadLoop, this);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AYuv::destroyThread()
{
    MY_LOG("+");

    // post exit
    addCommandQ(ECmd_Uninit);

    //
    pthread_join(mThread, NULL);

    //
    MY_LOG("-");
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AYuv::changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"3ATHREAD_YUV", 0, 0, 0);

    // (2) set policy/priority
    int const policy    = SCHED_OTHER;
    int const priority  = NICE_CAMERA_3A_MAIN;
    //
    struct sched_param sched_p;
    ::sched_getparam(0, &sched_p);
    //
    //  set
    sched_p.sched_priority = priority;  //  Note: "priority" is real-time priority.
    ::sched_setscheduler(0, policy, &sched_p);
    //
    //  get
    ::sched_getparam(0, &sched_p);
    //
    MY_LOG(
        "policy:(expect, result)=(%d, %d), priority:(expect, result)=(%d, %d)"
        , policy, ::sched_getscheduler(0)
        , priority, sched_p.sched_priority
    );
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3AYuv::sendCommand(ECmd_T const r3ACmd, MINTPTR const i4Arg)
{
#if 1 //move back from setIspProfile(), due to command order change
    if ((r3ACmd == ECmd_CameraPreviewStart) || (r3ACmd == ECmd_CaptureStart))
        queryTGInfoFromSensorHal(); //update TG info for VS irq configuration. This should be done after MW config sensor AND before ECmd_Update is called
#endif
    switch (r3ACmd){
        //type 1: run command by 3A thread
        case ECmd_CameraPreviewStart:
        case ECmd_CamcorderPreviewStart:
        case ECmd_PrecaptureEnd:
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
        break;

        //type 2: without wait
        case ECmd_Update:
            addCommandQ(r3ACmd, i4Arg);
        break;

        //type 3: clear previous commands and run current command by 3A thread
        case ECmd_PrecaptureStart: //from type1 to type3, avoid capture delay
        case ECmd_RecordingStart: //from type1 to type3, avoid recording delay
        case ECmd_RecordingEnd: //from type1 to type3, avoid recording stop delay
        case ECmd_CameraPreviewEnd:
        case ECmd_CamcorderPreviewEnd:
        case ECmd_Uninit:
            clearCommandQ();
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
        break;

        //type 4: run current command directly by caller thread
        default:
            postCommand(r3ACmd, i4Arg);
        break;
    }

    return true;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AYuv::clearCommandQ()
{
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mCmdQ.size();

    for (List<CmdQ_T>::iterator it = mCmdQ.begin(); it != mCmdQ.end();)
    {
        ECmd_T eCmd = it->eCmd;
        if ((eCmd == ECmd_Update) && (mCmdQ.size() >= 2)) //add size limitation to avoid mCmdQ deadlock
        {
            MY_LOG("cmd size(%d), clear(%d), ", mCmdQ.size(), eCmd);
            it = mCmdQ.erase(it);
        }
        else //include 2 cases; 1). keep all cmds except for ECmd_Update, 2). keep at least 1 ECmd_Update in mCmdQ
        {
            MY_LOG("Command not killed: cmd size(%d), beginning cmd(%d), ", mCmdQ.size(), eCmd);
            it++;
        }
    }
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3AYuv::addCommandQ(ECmd_T const &r3ACmd, MINTPTR i4Arg)
{
    Mutex::Autolock autoLock(mModuleMtx);

    ParamIspProfile_T rParam;
    // data clone
    if (i4Arg != 0)
    {
        rParam = *reinterpret_cast<ParamIspProfile_T*>(i4Arg);
    }

    MY_LOG_IF(LOG_PERFRAME_ENABLE, "mCmdQ size(%d) + cmd(%d) magic(%d)", mCmdQ.size(), r3ACmd, rParam.i4MagicNum);

    mCmdQ.push_back(CmdQ_T(r3ACmd, rParam));
    mCmdQCond.broadcast();
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3AYuv::getCommand(CmdQ_T &rCmd)
{
    Mutex::Autolock autoLock(mModuleMtx);

    if (mCmdQ.size() == 0)
    {
        mCmdQCond.wait(mModuleMtx);
    }
    if (mCmdQ.size() == 0)
        MY_ERR("mCmdQ.size() = %d after mCmdQCond.wait(), failed(0x%x)", mCmdQ.size(), E_3A_ERR);

    rCmd = *mCmdQ.begin();
    MY_LOG("CmdQ-size(%d), todo(%d, %d)", mCmdQ.size(), rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);

    mCmdQ.erase(mCmdQ.begin());

    if (rCmd.eCmd == ECmd_Uninit){
        return MFALSE;
    }
    else {
        return MTRUE;
    }
}


/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3AYuv::onThreadLoop(MVOID *arg)
{
    MY_LOG("[%s] +", __FUNCTION__);

    CmdQ_T rCmd(ECmd_Update, ParamIspProfile_T());
    Hal3AYuv *_this = reinterpret_cast<Hal3AYuv*>(arg);

    // (1) change thread setting
    _this->changeThreadSetting();

    // (2) thread-in-loop
#if 1
    while (_this->getCommand(rCmd))
    {
        MBOOL fgLogEn = (_this->m_u4LogEn & HAL3AYUV_LOG_THREAD) ? MTRUE : MFALSE;

        MY_LOG_IF(fgLogEn, "rCmd(%d)+", rCmd);
        //
        switch (rCmd.eCmd)
        {
            case ECmd_Update:
                //
                //::sem_post(&_this->mSem);
                //
                if ( ! _this->isReadyToCapture())
                {
                    MY_LOG_IF(fgLogEn, "[%s] start waitVSirq.", __FUNCTION__);
                    _this->waitVSirq();
                    MY_LOG_IF(1, "[%s] waitVSirq done.", __FUNCTION__);

                    _this->m_cbSet.doNotifyCb(I3ACallBack::eID_NOTIFY_VSYNC_DONE, rCmd.rParamIspProfile.i4MagicNum, 0, 0);
                    if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                    {
                        MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                    }
                }
                else
                {
                    MY_LOG("already ready to capture. save one VS");
                }
            break;

            default:
                //
                if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                }
                //
                ::sem_post(&_this->mSem);
            break;
        }
        //
        //
        MY_LOG_IF(fgLogEn, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
    }
#else
    while (1)
    {
        switch (_this->mpStateMgr->getCurrState())
        {
        case eState_CameraPreview:
        case eState_CamcorderPreview:
        case eState_Precapture:
        case eState_Recording:
            MY_LOG("waitVSirq (+)");
            _this->waitVSirq();
            MY_LOG("waitVSirq (-)");
            break;
        default:
            MY_LOG("NO waitVSirq");
            break;
        }

        if (_this->getCommand(rCmd))
        {
            if (rCmd.eCmd == ECmd_Update)
            {
                if ( ! _this->isReadyToCapture())
                {
                    if ( ! _this->postCommand(rCmd.eCmd, rCmd.iFrmId))
                    {
                        MY_ERR("Cmd(%d) failed(0x%x)", rCmd, _this->getErrorCode());
                    }
                }
                else
                {
                    MY_LOG("already ready to capture. save one VS");
                }
            }
            else
            {
                if ( ! _this->postCommand(rCmd.eCmd, rCmd.iFrmId))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd, _this->getErrorCode());
                }
                //
                ::sem_post(&_this->mSem);
            }
        }
        else
        {
            ::sem_post(&_this->mSem);
            break;
        }
    }
#endif

    MY_LOG("[%s] -", __FUNCTION__);

    return NULL;
}

#endif

