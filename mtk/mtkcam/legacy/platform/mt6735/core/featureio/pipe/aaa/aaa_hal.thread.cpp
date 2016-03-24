/******************************************************************************
*
*******************************************************************************/
#define LOG_TAG "aaa_hal/3Athread"
//
#include <utils/threads.h>
#include <utils/List.h>
//using namespace android;
//
/*NeedUpdate*///#include <common/CamTypes.h>
#include <mtkcam/v1/config/PriorityDefs.h>
/*NeedUpdate*///#include <common/CamDefs.h>
#include <mtkcam/utils/common.h>
using namespace android;
using namespace NSCam::Utils;

//
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal.h"
#include <state_mgr/aaa_state.h>
#include <state_mgr/aaa_state_mgr.h>
#include "aaa_scheduler.h"
#include <ae_mgr_if.h>


using namespace NS3A;
//
//
#include <pthread.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/resource.h>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/mt_sched.h>
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
                  DB_OPT_FTRACE, \
                  String); \
          } while(0)
#else
#define AEE_ASSERT_3A_HAL(String)
#endif



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::createThread()
{
    sem_init(&mSem, 0, 0);
    sem_init(&semAFProcEnd, 0, 0);
    sem_init(&semPD, 0, 0);
    sem_init(&semPDThdEnd, 0, 1);
    pthread_create(&mThread, NULL, onThreadLoop, this);
    pthread_create(&mPDThread, NULL, PDThreadLoop, this);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::createAEThread()
{
    sem_init(&semAEI2CSen, 0, 0);
    sem_init(&semAESen, 0, 0);
    sem_init(&semAESenThdEnd, 0, 1);
    sem_init(&semVsyncFromAE, 0, 0);
    // modify timerslack.
    prctl(PR_SET_TIMERSLACK, 5000, 0, 0, 0);
    pthread_create(&mAESenThread, NULL, AESensorThreadLoop, this);
}



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::destroyThread()
{
    MY_LOG("[%s]+", __FUNCTION__);

    // post exit
    addCommandQ(ECmd_Uninit);

    //
    pthread_join(mThread, NULL);

    //
    MY_LOG("[%s]-", __FUNCTION__);
}

/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::destroyAEThread()
{
    MY_LOG("[%s]+", __FUNCTION__);

    //
    pthread_join(mAESenThread, NULL);

    //
    MY_LOG("[%s]-", __FUNCTION__);
}



/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::changeThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"3ATHREAD", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = get3AThreadNiceValue(m_i4SensorDev);//NICE_CAMERA_3A_MAIN;
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
MVOID
Hal3A::changeAESensorThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"AESenThd", 0, 0, 0);

    // (2) set policy/priority
    {
        int const expect_policy     = SCHED_OTHER;
        int const expect_priority   = NICE_CAMERA_AE;
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
MVOID
Hal3A::changePDThreadSetting()
{
    // (1) set name
    ::prctl(PR_SET_NAME,"PDThd", 0, 0, 0);

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

}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3A::sendCommand(ECmd_T const r3ACmd, MINTPTR const i4Arg)
{
    AAA_TRACE_FMT_BEGIN("3AsendCmd %d", r3ACmd);
    MBOOL bRet = MTRUE;
    if (mpStateMgr->getStateStatus().eCurrState == eState_AF)
        if ((r3ACmd == ECmd_RecordingStart) || (r3ACmd == ECmd_RecordingEnd) || (r3ACmd == ECmd_CameraPreviewEnd))
        {
            MY_LOG_IF(1, "[Hal3A::sendCommand] eState_AF: get r3ACmd(%d)", r3ACmd);
            cancelAutoFocus();
        }
#if 1 //move back from setIspProfile(), due to command order change
    if ((r3ACmd == ECmd_CameraPreviewStart) || (r3ACmd == ECmd_CaptureStart))
        queryTGInfoFromSensorHal(); //update TG info for VS irq configuration. This should be done after MW config sensor AND before ECmd_Update is called
#endif

    if (r3ACmd == ECmd_CameraPreviewStart)
    {
        //set 3A scheduler parameters
        //sync with MW:
        //if MinFps == MaxFps, MW query M with this fps; ==> here we use this fps to set M
        //otherwise, just normal preview case, MW won't query M; ==> here we use 30 fps to set M =1
        MUINT32 fps = (m_rParam.i4MinFps == m_rParam.i4MaxFps) ? m_rParam.i4MaxFps : 30000;
        mpScheduler->setSchdParams(m_i4SensorDev, fps, ((m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));

        //set semAESenThdEnd to be 1, to avoid the deadlock caused by no AE post from PreviewStart to PreviewEnd
        int Val;
        ::sem_getvalue(&semAESenThdEnd, &Val);
        if (Val == 0) ::sem_post(&semAESenThdEnd); //to be 1
        MY_LOG_IF(1, "[Hal3A::sendCommand] PreviewStart semAESenThdEnd(%d) before post", Val);
        //sem_init(&semAESenThdEnd, 0, 1);

        ::sem_getvalue(&semPDThdEnd, &Val);
        if (Val == 0) ::sem_post(&semPDThdEnd); //to be 1
        MY_LOG_IF(1, "[Hal3A::sendCommand] PreviewStart semPDThdEnd(%d) before post", Val);
        //sem_init(&semPDThdEnd, 0, 1);

    }
    if (r3ACmd == ECmd_CameraPreviewEnd)
    {
        //set default scheduling
        mpScheduler->setSchdParams(m_i4SensorDev, 30000, ((m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));
    }
    if(r3ACmd == ECmd_CaptureStart)
    {
        Mutex::Autolock lock(mVsyncMtx);
        int Val;
        ::sem_getvalue(&semVsyncFromAE, &Val);
        if (Val == 1) ::sem_wait(&semVsyncFromAE); //to be 0
    }
    if (r3ACmd == ECmd_CaptureEnd)
    {
        capCheckAndFireFlash_End();
    }

    if (r3ACmd == ECmd_Update)
    {
        if(!mbInCapturing)
        {
            // reset semVsyncFromAE to be 0
            Mutex::Autolock lock(mVsyncMtx);
            int Val;
            ::sem_getvalue(&semVsyncFromAE, &Val);
            if (Val == 1) ::sem_wait(&semVsyncFromAE); //to be 0
        }
    }

    switch (r3ACmd){
        //type 1: run command by 3A thread
        case ECmd_CameraPreviewStart:
        case ECmd_CaptureStart:
        case ECmd_CamcorderPreviewStart:
        case ECmd_PrecaptureEnd:
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
            //bRet = sem_wait_relativeTime(&mSem, SEM_TIME_LIMIT_NS, "sendCommand::mSem(1)");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(mSem(1)).");
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
        case ECmd_CaptureEnd:
        case ECmd_CamcorderPreviewEnd:
        case ECmd_Uninit:
            clearCommandQ();
            addCommandQ(r3ACmd);
            ::sem_wait(&mSem);
            //bRet = sem_wait_relativeTime(&mSem, SEM_TIME_LIMIT_NS, "sendCommand::mSem(2)");
            if (!bRet) AEE_ASSERT_3A_HAL("sem_wait_relativeTime(mSem(2)).");
        break;

        //type 4: run current command directly by caller thread
        default:
            bRet = postCommand(r3ACmd, i4Arg);
            if (!bRet) AEE_ASSERT_3A_HAL("type4 postCommand fail.");
        break;
    }

    AAA_TRACE_FMT_END();
    return true;
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::clearCommandQ()
{
    MY_LOG("[%s]+", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);

    int Qsize = mCmdQ.size();

    for (List<CmdQ_T>::iterator it = mCmdQ.begin(); it != mCmdQ.end();)
    {
        ECmd_T eCmd = it->eCmd;
        if ((eCmd == ECmd_Update) && (mCmdQ.size() >= 2)) //add size limitation to avoid mCmdQ deadlock
        {
            MY_LOG("[%s] cmd size(%d), clear(%d)", __FUNCTION__, mCmdQ.size(), eCmd);
            it = mCmdQ.erase(it);
        }
        else //include 2 cases; 1). keep all cmds except for ECmd_Update, 2). keep at least 1 ECmd_Update in mCmdQ
        {
            MY_LOG("[%s] Command not killed: cmd size(%d), beginning cmd(%d)", __FUNCTION__, mCmdQ.size(), eCmd);
            it++;
        }
    }
    MY_LOG("[%s]- Qsize(%d)", __FUNCTION__, Qsize);
}


/******************************************************************************
*
*******************************************************************************/
MVOID
Hal3A::addCommandQ(ECmd_T const &r3ACmd, MINTPTR i4Arg)
{
    Mutex::Autolock autoLock(mModuleMtx);

    ParamIspProfile_T rParam;
    // data clone
    if (i4Arg != 0)
    {
        rParam = *reinterpret_cast<ParamIspProfile_T*>(i4Arg);
    }

    mCmdQ.push_back(CmdQ_T(r3ACmd, rParam));
    mCmdQCond.broadcast();

    MY_LOG_IF(LOG_PERFRAME_ENABLE, "[%s] mCmdQ size(%d) + cmd(%d) magic(%d)", __FUNCTION__, mCmdQ.size(), r3ACmd, rParam.i4MagicNum);
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
Hal3A::getCommand(CmdQ_T &rCmd, MBOOL &bGetCmd, MBOOL en_timeout)
{
    //returning MFALSE means break onThreadLoop
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] +", __FUNCTION__);
    Mutex::Autolock autoLock(mModuleMtx);
    MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQ.size()=%d, en_timeout(%d)", __FUNCTION__, mCmdQ.size(), en_timeout);

    if (mCmdQ.size() == 0)
    {
        if (en_timeout)
        {
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.waitRelative +", __FUNCTION__);
            mCmdQCond.waitRelative(mModuleMtx, (long long int)(mpScheduler->getVsTimeOutLimit_ns()));
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.waitRelative -", __FUNCTION__);
        }
        else
        {
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.wait +", __FUNCTION__);
            mCmdQCond.wait(mModuleMtx);
            MY_LOG_IF(m_3ALogEnable & EN_3A_FLOW_LOG, "[%s] mCmdQCond.wait -", __FUNCTION__);
        }
    }
    if (mCmdQ.size() == 0) //this is only for en_timeout == 1 & timeout case
    {
        MY_LOG("[%s] mCmdQ.size() = %d after mCmdQCond.wait/waitRelative\n", __FUNCTION__, mCmdQ.size());
        bGetCmd = MFALSE; // timeout, no command received
        return MTRUE;
    }
    bGetCmd = MTRUE;
    // two cases: 1. en_timeout == 1, but get command in time
    //                 2. en_timeout == 0, finally get command

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
MBOOL
Hal3A::waitVSirqThenGetCommand(CmdQ_T &rCmd)
{
    //returning MFALSE means break onThreadLoop
    MBOOL bRet;
    MBOOL bGetCmd;
    MINT32 time_ms;

    if (!mbListenVsync) return getCommand(rCmd, bGetCmd, MFALSE);

    while(1)
    {
        AAA_TRACE_BEGIN("waitVsync(A)");
        waitVSirq();
        AAA_TRACE_END();
        mpVSTimer->start("VS Timer", m_i4SensorDev, 1);
        bRet = getCommand(rCmd, bGetCmd, MTRUE);
        time_ms = mpVSTimer->End();

        if (bGetCmd) return bRet; //if receiving command, return
    }
    //impossible to get here, just to avoid compilation warning
    return MTRUE;

}

/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3A::onThreadLoop(MVOID *arg)
{
    MY_LOG("[%s]+", __FUNCTION__);

    CmdQ_T rCmd(ECmd_Update, ParamIspProfile_T());
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);
    MBOOL bIsFirstOfCmdSet;
    MBOOL bToExeUpdateAfterPrecap;
    MBOOL bGetCmd;
    AaaTimer VsyncTimer;
    AaaTimer VSCallBackTimer;
    MBOOL fgLogEn = (_this->m_3ALogEnable & EN_3A_FLOW_LOG) ? MTRUE : MFALSE;

    // (1) change thread setting
    _this->changeThreadSetting();

    // (2) thread-in-loop
    #if 1
    while (_this->getCommand(rCmd, bGetCmd, MFALSE))
    {
        //MY_LOG_IF(1, "rCmd(%d)+", rCmd);
        //
        switch (rCmd.eCmd)
        {
            case ECmd_PrecaptureStart:
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "rCmd(%d)+", rCmd.eCmd);
                //
                if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                    AEE_ASSERT_3A_HAL("onThreadLoop ECmd_PrecaptureStart fail.");
                }
                //
                ::sem_post(&_this->mSem);
                MY_LOG_IF(1, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
                bToExeUpdateAfterPrecap = MFALSE;
                {
                    Mutex::Autolock autoLock(_this->mModuleMtx); //protect memory access
                    if (_this->mCmdQ.size() && (_this->mCmdQ.begin()->eCmd == ECmd_Update)) bToExeUpdateAfterPrecap = MTRUE;
                }
            if (bToExeUpdateAfterPrecap)
            {
                MY_LOG("Update follows PrecaptureStart");
                MBOOL bGetCmd;
                _this->getCommand(rCmd, bGetCmd); //continue with case ECmd_Update:
            }
            else break; //break switch (rCmd.eCmd)

            case ECmd_Update:
                //
                MY_LOG_IF(fgLogEn, "rCmd(%d)+", rCmd.eCmd);
                AAA_TRACE_FMT_BEGIN("update %d", rCmd.rParamIspProfile.i4MagicNum);
                VsyncTimer.start("VsyncUpdate", _this->m_i4SensorDev, ((_this->m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));
                //
                //::sem_post(&_this->mSem);
                //
                bIsFirstOfCmdSet = MTRUE; //the first command of init drop frame or slow motion.
                _this->mpScheduler->importWork();
                _this->setMagicNum2AFMgr(rCmd.rParamIspProfile.i4MagicNum);

                {
                    Mutex::Autolock autoLock(_this->mAESenMtx);
                int Val;
                    ::sem_getvalue(&_this->semAEI2CSen, &Val);
                    if (Val == 1){
                        ::sem_wait(&_this->semAEI2CSen);
                        MY_LOG_IF(1, "[%s][reset] semAEI2CSen = %d", __FUNCTION__, Val);
                    }

                    ::sem_getvalue(&_this->semAESen, &Val);
                    if (Val == 1){
                        ::sem_wait(&_this->semAESen);
                        MY_LOG_IF(1, "[%s][reset] semAESen = %d", __FUNCTION__, Val);
                    }
                }

                for (int frameIdx=1;
                     (frameIdx <= (int)(_this->mpScheduler->getValidFrameIdx())) || (_this->mpStateMgr->getFrameCount() <= (-1));
                     // for init drop frames, frameCnt <= -1, frameCnt == -1 means this ECmd_Update will execute 3A, not bypass
                     //                            once FrameCnt = -1, frameIdx == 1, this is the first exe Vsycn, ok!!
                     //                            from above, first Validate will be executed at 2+(ValidateFrameIdx at first cycle)th Vsync
                     // for normal case, frameCnt >= 0, do 3A for frameIdx 1~mValidFrameIdx
                     frameIdx++)
                {
                    if (_this->mpStateMgr->getFrameCount() < (-1)) frameIdx = 0; // frameIdx == 1 when  getFrameCount = -1

                    if ( 1/*! _this->isReadyToCapture()*/)
                    {
                        MY_LOG_IF(fgLogEn, "[Hal3A::onThreadLoop] start waitVSirq.");
                        AAA_TRACE_BEGIN("sem_wait(semVsyncFromAE)");
                        if ((_this->mpStateMgr->getFrameCount() <= (-1)) || _this->mbInCapturing)
                        {
                            ::sem_wait(&_this->semVsyncFromAE);
                            MY_LOG_IF(1, "[Hal3A::onThreadLoop] sem_wait(semVsyncFromAE) done.");
                        }
                        bIsFirstOfCmdSet = MFALSE; //reset flag
                        AAA_TRACE_END();

                        if (_this->mpStateMgr->getFrameCount() > (-1)) _this->mpScheduler->setFrameIdx(frameIdx);
                        else _this->mpScheduler->setFrameIdx(1);
                        VSCallBackTimer.start("VSCallBack",_this->m_i4SensorDev,((_this->m_3ALogEnable & EN_3A_SCHEDULE_LOG) ? MTRUE : MFALSE));
                        _this->mpCbSet->doNotifyCb(I3ACallBack::eID_NOTIFY_VSYNC_DONE, rCmd.rParamIspProfile.i4MagicNum, 0, 0);
                        VSCallBackTimer.End();
                        if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                        {
                            MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                            AEE_ASSERT_3A_HAL("onThreadLoop ECmd_Update fail.");
                        }
                    }
                    else
                    {
                        MY_LOG("already ready to capture. save one VS");
                    }
                }
                //
                VsyncTimer.End();
                AAA_TRACE_FMT_END();
                MY_LOG_IF(fgLogEn, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);  

                // after VsyncUpdate, start to process commands of other types in CmdQ
                while (1)
                {
                    MBOOL bExeOptCmd = MFALSE;
                    {
                        Mutex::Autolock autoLock(_this->mModuleMtx); //protect memory access
                        if (_this->mCmdQ.size()
                            && (_this->mCmdQ.begin()->eCmd != ECmd_Update))
                        {
                            bExeOptCmd = MTRUE;
                        }
                    }
                    if (bExeOptCmd)
                    {
                        MBOOL bGetCmd;
                        if(!_this->getCommand(rCmd, bGetCmd))
                        {
                            //break onThreadLoop
                            MY_LOG("-");
                            return NULL;
                        }
                        else
                        {
                            MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "rCmd(%d)+", rCmd.eCmd);
                            //just like what we do in the following "default"
                            //
                            if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                            {
                                MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                                AEE_ASSERT_3A_HAL("onThreadLoop postCommand fail(1).");
                            }
                            //
                            ::sem_post(&_this->mSem);
                            MY_LOG_IF(1, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
                        }
                    }
                    else break;
                }
            break;

            default:
                MY_LOG_IF(_this->m_3ALogEnable & EN_3A_FLOW_LOG, "rCmd(%d)+", rCmd.eCmd);
                //
                if ( ! _this->postCommand(rCmd.eCmd, reinterpret_cast<MINTPTR>(&rCmd.rParamIspProfile)))
                {
                    MY_ERR("Cmd(%d) failed(0x%x)", rCmd.eCmd, _this->getErrorCode());
                    AEE_ASSERT_3A_HAL("onThreadLoop postCommand fail(2).");
                }
                //
                ::sem_post(&_this->mSem);
                MY_LOG_IF(1, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
            break;
        }
        //
        //
        //MY_LOG_IF(1, "rCmd(%d), magic#(%d)-", rCmd.eCmd, rCmd.rParamIspProfile.i4MagicNum);
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
/*
    int status = mt_sched_exitaffinity(0);
    if(!status){
        MY_LOG("exit affinity to CPU successfully, status=%d\n", status);
    }else{
        MY_LOG("exit affinity to CPU FAIL, status=%d\n", status);
    }
*/

    MY_LOG("[%s]-", __FUNCTION__);

    return NULL;
}

/******************************************************************************
*
*******************************************************************************/
MVOID*
Hal3A::AESensorThreadLoop(MVOID *arg)
{
    Hal3A *_this = reinterpret_cast<Hal3A*>(arg);
    // (1) change thread setting
    _this->changeAESensorThreadSetting();

    MBOOL fgLogEn = (_this->m_3ALogEnable & EN_3A_FLOW_LOG) ? MTRUE : MFALSE;
    // (2) thread-in-loop
    while(1)
    {
        AAA_TRACE_BEGIN("waitVsync(A)");
        MY_LOG_IF(fgLogEn, "waitVsync start.");
        _this->waitVSirq();
        if (_this->mEnFlushVSIrq)
        {
            _this->mFlushVSIrqDone = 1;
            MY_LOG_IF(1, "[AESensorThreadLoop] mFlushVSIrqDone = 1\n");
            int Val;
            ::sem_getvalue(&_this->semVsyncFromAE, &Val);
            if (Val == 0)
            {
                MY_LOG("post semVsyncFromAE");
                ::sem_post(&_this->semVsyncFromAE); //to be 1
            }
            break;
        }
        MY_LOG_IF(fgLogEn, "waitVsync done.");
        AAA_TRACE_END();
        MY_LOG_IF(fgLogEn, "mbInCapturing = %d", _this->mbInCapturing);
        if((_this->mpStateMgr->getFrameCount() <= (-1)) || _this->mbInCapturing)
        {
            Mutex::Autolock lock(_this->mVsyncMtx);
            // post semVsyncFromAE for init drop frame and slow motion
            int Val;
            ::sem_getvalue(&_this->semVsyncFromAE, &Val);
            if (Val == 0)
            {
                MY_LOG("post semVsyncFromAE");
                ::sem_post(&_this->semVsyncFromAE); //to be 1
            }
        }

        if ( ! _this->mbEnAESenThd)
        {
            _this->mFlushVSIrqDone = 1; // no need to flush
            MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] mFlushVSIrqDone (1)");
            break;
        }

        MBOOL bRet = _this->sem_wait_relativeTime(&_this->semAEI2CSen,
            (long long int)(_this->mpScheduler->getAEThreadVsTimeOutLimit_ns()),
            "AESensorThreadLoop::semAEI2CSen",
            MTRUE);

        if (!bRet)
        {
            {
                Mutex::Autolock autoLock(_this->mAESenMtx);
                int Val;
                ::sem_getvalue(&_this->semAEI2CSen, &Val);
                if (Val == 1) ::sem_wait(&_this->semAEI2CSen);
                MY_LOG_IF(fgLogEn, "[%s][reset] semAEI2CSen = %d", __FUNCTION__, Val);

                ::sem_getvalue(&_this->semAESen, &Val);
                if (Val == 1) ::sem_wait(&_this->semAESen);
                MY_LOG_IF(fgLogEn, "[%s][reset] semAESen = %d", __FUNCTION__, Val);
            }
            MY_LOG_IF(fgLogEn, "Timeout : sem_wait_relativeTime(semAEI2CSen).");
            continue;
        }

        if ( ! _this->mbEnAESenThd)
        {
            _this->mFlushVSIrqDone = 1; // no need to flush
            MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] mFlushVSIrqDone (2)");
            break;
        }

        bRet = _this->sem_wait_relativeTime(&_this->semAESen,
            (long long int)(_this->mpScheduler->getAEThreadVsTimeOutLimit_ns() * 5),
            "AESensorThreadLoop::semAESen",
            MTRUE);

        if (!bRet)
        {
            MY_ERR("Timeout : sem_wait_relativeTime(semAESen).");
            continue;
        }

        if ( ! _this->mbEnAESenThd)
        {
            _this->mFlushVSIrqDone = 1; // no need to flush
            MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] mFlushVSIrqDone (3)");
            break;
        }
        if(_this->mbInCapturing)
        {
            MY_LOG_IF(1, "[AESensorThreadLoop] updateCaptureShutterValue start\n");
            IAeMgr::getInstance().updateCaptureShutterValue(_this->m_i4SensorDev);
            MY_LOG_IF(1, "[AESensorThreadLoop] updateCaptureShutterValue end\n");
        } else
        {
            MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] updateSensorbyI2C start\n");
            IAeMgr::getInstance().updateSensorbyI2C(_this->m_i4SensorDev);
            MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] updateSensorbyI2C end\n");
        }
    }

    //sync with 3A thread when PreviewEnd
    {
        int Val;
        MY_LOG_IF(fgLogEn, "[Hal3A::AESensorThreadLoop] start waiting mAESenMtx lock (3)\n");
        Mutex::Autolock autoLock(_this->mAESenMtx);
        MY_LOG_IF(fgLogEn, "[Hal3A::AESensorThreadLoop] get mAESenMtx lock (3)\n");

        ::sem_getvalue(&_this->semAESenThdEnd, &Val);
	    MY_LOG_IF(fgLogEn, "[AESensorThreadLoop] semAESenThdEnd before post = %d\n", Val);
        if (Val == 0) ::sem_post(&_this->semAESenThdEnd); //to be 1, 1 means AE set Sensor done, this can tolerate I2C delay too long
    }
    return NULL;
}

MVOID
Hal3A::postToAESenThread(MBOOL bReadyToUpdate)
{
    Mutex::Autolock autoLock(mAESenMtx);
    int Val;
    ::sem_getvalue(&semAESenThdEnd, &Val);
    if (Val == 1) ::sem_wait(&semAESenThdEnd); //to be 0, it won't block, 0 means AE set Sensor not ready yet, this can tolerate I2C delay too long
    if(!bReadyToUpdate)
    {
        ::sem_getvalue(&semAEI2CSen, &Val);
        if(Val == 0)
            ::sem_post(&semAEI2CSen);
    }
    else
    {
        ::sem_getvalue(&semAESen, &Val);
        if(Val == 0)
            ::sem_post(&semAESen);
    }
    MY_LOG_IF((m_3ALogEnable & EN_3A_FLOW_LOG), "[postToAESenThread] sem_post semAESen, semAESenThdEnd before wait = %d\n", Val);
}



MVOID
Hal3A::postToPDThread()
{
    Mutex::Autolock autoLock(mPDMtx);
    int Val;
    ::sem_getvalue(&semPDThdEnd, &Val);
    if (Val == 1) ::sem_wait(&semPDThdEnd); //to be 0, it won't block, 0 means PD task not ready yet
    ::sem_post(&semPD);
    MY_LOG_IF(1, "[postToPDThread] sem_post semPD, semPDThdEnd before wait = %d\n", Val);
}

