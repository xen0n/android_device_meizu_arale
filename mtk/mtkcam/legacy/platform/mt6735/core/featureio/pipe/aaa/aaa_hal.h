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
/**
* @file aaa_hal.h
* @brief Declarations of 3A Hal Class (public inherited from Hal3AIf)
*/
#ifndef _AAA_HAL_H_
#define _AAA_HAL_H_

#ifdef MTK_LOG_ENABLE
#undef MTK_LOG_ENABLE
#endif
#define MTK_LOG_ENABLE 1

//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------
#include <mtkcam/drv/isp_drv.h>

#include <mtkcam/featureio/aaa_hal_if.h>
#include <ResultBufMgr/ResultBufMgr.h>

#include <utils/threads.h>
#include <utils/List.h>
//-------------------------------
#include <Local.h>
/*NeedUpdate*///#include <flicker_hal_base.h>
using namespace android;

#define ISP_P1_UPDATE (1)
#define NOTIFY_3A_DONE (1)

/* trace */
#if 1
#ifndef ATRACE_TAG
#define ATRACE_TAG                           ATRACE_TAG_CAMERA
#endif
#include <utils/Trace.h>

#define AAA_TRACE_NAME_LENGTH                 64

#define AAA_TRACE_CALL()                      ATRACE_CALL()
#define AAA_TRACE_NAME(name)                  ATRACE_NAME(name)
#define AAA_TRACE_BEGIN(name)                 ATRACE_BEGIN(name)
#define AAA_TRACE_END()                       ATRACE_END()
#define AAA_TRACE_FMT_BEGIN(fmt, arg...)                  \
do{                                                       \
    if( ATRACE_ENABLED() )                                \
    {                                                     \
        char buf[AAA_TRACE_NAME_LENGTH];                  \
        snprintf(buf, AAA_TRACE_NAME_LENGTH, fmt, ##arg); \
        AAA_TRACE_BEGIN(buf);                             \
    }                                                     \
}while(0)
#define AAA_TRACE_FMT_END()                   AAA_TRACE_END()
#else
#define AAA_TRACE_CALL()
#define AAA_TRACE_NAME(name)
#define AAA_TRACE_BEGIN(name)
#define AAA_TRACE_END()
#define AAA_TRACE_FMT_BEGIN(fmt, arg)
#define AAA_TRACE_FMT_END()
#endif

namespace NS3A
{

class StateMgr;
class AAA_Scheduler;
class AaaTimer;

/*******************************************************************************
*
********************************************************************************/
/**
 * @brief 3A Hal Class, all inherited function interfaces are described in class Hal3AIf, please refer to aaa_hal_if.h
 */
class Hal3A : public Hal3AIf
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3A();
    virtual ~Hal3A();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    static Hal3A* createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);
    static Hal3A* getInstance(MINT32 i4SensorDevId);
    virtual MVOID destroyInstance();
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0);

    virtual inline MINT32 getErrorCode() const
    {
        return m_errorCode;
    }

    virtual inline MBOOL getParams(Param_T &rParam) const
    {
        rParam = m_rParam;
        return MTRUE;
    }

    virtual MBOOL setParams(Param_T const &rNewParam);
    virtual MBOOL getSupportedParams(FeatureParam_T &rFeatureParam);

    virtual inline MBOOL isReadyToCapture() const
    {
        return m_bReadyToCapture;
    }

    virtual MBOOL autoFocus();
    virtual MBOOL cancelAutoFocus();
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;
    virtual MBOOL setDebugInfo(IDbgInfoContainer* pDbgInfoCtnr, MBOOL const fgReadFromHW) const;
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;
    virtual MBOOL setIspProfile(const ParamIspProfile_T& rParamIspProfile, IDbgInfoContainer* pDbgInfoCtnr);
    /**
    * @brief Enable AF thread
    * @param [in] a_bEnable set 1 to enable AF thread
    */
    virtual MRESULT EnableAFThread(MINT32 a_bEnable);
    /**
     * @brief add callbacks for 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 addCallbacks(I3ACallBack* cb);
    /**
     * @brief remove callbacks in 3A HAL, return number of cb in 3A HAL
     */
    virtual MINT32 removeCallbacks(I3ACallBack* cb);
    virtual MINT32 getExposureInfo(ExpSettingParam_T &strHDRInputSetting);
    virtual MINT32 getCaptureParams(CaptureParam_T &a_rCaptureInfo);
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);
    virtual MINT32 getHDRCapInfo(Hal3A_HDROutputParam_T &a_strHDROutputInfo);
    virtual MBOOL setFDInfo(MVOID* a_sFaces);
    virtual MBOOL setOTInfo(MVOID* a_sOT);
    virtual MVOID setFDEnable(MBOOL bEnable);
    virtual MINT32 getRTParams(FrameOutputParam_T &a_strFrameOutputInfo);
    virtual MINT32 isNeedFiringFlash(MBOOL bEnCal);
    virtual MBOOL getASDInfo(ASDInfo_T &a_rASDInfo);
    virtual MINT32 modifyPlineTableLimitation(MBOOL bEnable, MBOOL bEquivalent, MUINT32 u4IncreaseISO_x100, MUINT32 u4IncreaseShutter_x100);
    virtual MBOOL getLCEInfo(LCEInfo_T &a_rLCEInfo);
    virtual MVOID endContinuousShotJobs();
    virtual MVOID enterCaptureProcess();
    virtual MVOID exitCaptureProcess();
    virtual MUINT32 queryFramesPerCycle(MUINT32 fps);
    virtual MINT32 enableAELimiterControl(MBOOL  bIsAELimiter);

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR iArg1, MINTPTR iArg2);

    virtual MVOID setFrameId(MINT32 i4FrmId){m_i4FrmId = i4FrmId;}
    virtual MINT32 getFrameId() const {return m_i4FrmId;}
    virtual MINT32 getResult(MUINT32 i4FrmId, Result_T& rResult) const {return m_rResultBuf.getResult(i4FrmId, rResult);}
    virtual MVOID setSensorMode(MINT32 i4SensorMode);
    virtual MINT32 getSensorType() const
    {
        return m_i4SensorType;
    }
    virtual MINT32 enableFlashQuickCalibration(MINT32 bEn);
    virtual MINT32 getFlashQuickCalibrationResult();
    virtual MVOID setAELock(MBOOL  bIsAELock);
    virtual MVOID setAWBLock(MBOOL  bIsAELock);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
     //
    /**
    * @brief init 3A hal
    * @param [in] i4SensorDev sensor device; please refer to halSensorDev_e in sensor_hal.h
    */
    MRESULT init(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);

    /**
    * @brief uninit 3A hal
    */
    MRESULT uninit();

    /**
    * @brief set error code
    * @param [in] errorCode error code; please refer to aaa_error_code.h
    */
    inline MVOID setErrorCode(MRESULT errorCode)
    {
        m_errorCode = errorCode;
    }
    /**
    * @brief reset ready-to-capture bool flag to be false
    */
    inline MVOID resetReadyToCapture()
    {
        m_bReadyToCapture = MFALSE;
    }
    /**
    * @brief enable ready-to-capture bool flag to be true
    */
    inline MVOID notifyReadyToCapture()
    {
        m_bReadyToCapture = MTRUE;
    }
    /**
    * @brief get current sensor device from 3A Hal instance
    */
    inline MINT32 getSensorDev()
    {
        return m_i4SensorDev;
    }
    /**
    * @brief get current sensor open index from 3A Hal instance
    */
    inline MINT32 getSensorOpenIdx()
    {
        return m_i4SensorOpenIdx;
    }
    /**
    * @brief get TGInfo (already got it in queryTGInfoFromSensorHal())
    */
    inline MUINT getTGInfo()
    {
        return m_TgInfo;
    }
    /**
    * @brief get 3A Capture Mode (already got it in updateCaptureParams(), refer to E3ACaptureMode_T)
    */
    inline MUINT32 get3ACapMode()
    {
        return m_3ACapMode;
    }
    /**
    * @brief set 3A Preview Mode (refer to E3APreviewMode_T)
    */
    inline virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode)
    {
        m_PvMode = PvMode;
    }
    /**
    * @brief get 3A Preview Mode (refer to E3APreviewMode_T)
    */
    inline E3APreviewMode_T get3APreviewMode()
    {
        return m_PvMode;
    }

    /**
     * @brief set AE target mode
     * @param [in] AeTargetMode, refer to eAETARGETMODE in ae_param.h
     */
    virtual MINT32 SetAETargetMode(MUINT32 AeTargetMode);

    /**
    * @brief called (as callback) when flash light turns on/off
    */
    MVOID notifyStrobeMode (MINT32 bEnable);

    MVOID capCheckAndFireFlash_End();

    MVOID postToAESenThread(MBOOL bReadyToUpdate = MFALSE);
    MVOID postToPDThread();

private:
    /**
    * @brief AF thread execution function
    */
    static MVOID* AFThreadFunc(void *arg);

    /**
    * @brief Free Collect command
    */
    MVOID createCollectCmd(MINTPTR i4Arg);

    /**
    * @brief Free Collect command
    */
    MVOID freeCollectCmd();

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile int  m_Users;
    mutable Mutex m_Lock;
    mutable Mutex m_ValidateLock;
    mutable Mutex m_DropLock;
    MRESULT       m_errorCode;
    Param_T       m_rParam;
    Result_T      m_rResult;
    ResultBufMgr  m_rResultBuf;
    MBOOL         m_bReadyToCapture;
    MINT32        m_i4SensorDev;
    MINT32        m_i4SensorOpenIdx;
    MUINT         m_TgInfo;
    MBOOL         m_bDebugEnable;
    /*NeedUpdate*///FlickerHalBase* mpFlickerHal;
    MINT32        m_i4FrmId;
    MUINT32       m_3ACapMode;
    E3APreviewMode_T   m_PvMode;
    MBOOL         m_bFaceDetectEnable;
    MINT32        m_i4StereoWarning;
    MINT32        m_i4SensorType;

public:
    MBOOL         m_b3APvInitOK;
    CollectCmd_T  m_rCollectCmd;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  AE/AWB thread
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    struct CmdQ_T
    {
        ECmd_T            eCmd;
        ParamIspProfile_T rParamIspProfile;

        CmdQ_T(ECmd_T _eCmd, const ParamIspProfile_T& _rParamIspProfile)
            : eCmd(_eCmd), rParamIspProfile(_rParamIspProfile) {}
    };


    MBOOL validate(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);

    /**
    * @brief collect all 3A result status, and notify 3A PROC FINISH event
    */
    MVOID           on3AProcFinish(MINT32 i4MagicNum);
    /**
    * @brief execute 3A commands in current 3A state
    * @param [in] r3ACmd 3A commands; please refer to aaa_hal_if.h
    */
    MBOOL           postCommand(ECmd_T const r3ACmd, MINTPTR const i4Arg = 0);
    /**
    * @brief create AE/AWB thread
    */
    MVOID           createThread();
    /**
    * @brief destroy AE/AWB thread
    */
    MVOID           destroyThread();
    /**
    * @brief create AE thread
    */
    MVOID           createAEThread();
    /**
    * @brief destroy AE thread
    */
    MVOID           destroyAEThread();
    /**
    * @brief change AE/AWB thread setting
    */
    MVOID           changeThreadSetting();
    /**
    * @brief change AE sensor I2C thread setting
    */
    MVOID           changeAESensorThreadSetting();
    /**
    * @brief change AE sensor I2C thread setting
    */
    MVOID           changePDThreadSetting();
    /**
    * @brief AE/AWB thread execution function
    */
    static  MVOID*  onThreadLoop(MVOID*);
    /**
    * @brief AE sensor I2C thread execution function
    */
    static MVOID* AESensorThreadLoop(MVOID*);
    /**
    * @brief PD thread execution function
    */
    static MVOID* PDThreadLoop(MVOID*);

    /**
    * @brief add 3A commands in command queue
    * @param [in] r3ACmd 3A commands; please refer to aaa_hal_if.h
    */
    MVOID           addCommandQ(ECmd_T const &r3ACmd, MINTPTR i4Arg = 0);
    /**
    * @brief clear all ECmd_Update commands in current command queue
    */
    MVOID           clearCommandQ();
    /**
    * @brief get 3A command from the head of 3A command queue
    * @param [in] rCmd 3A commands; please refer to aaa_hal_if.h
    */
    MBOOL           getCommand(CmdQ_T &rCmd, MBOOL &bGetCmd, MBOOL en_timeout = MFALSE);
    /**
    * @brief wait VS irq then get 3A command from the head of 3A command queue
    * @param [in] rCmd 3A commands; please refer to aaa_hal_if.h
    */
    MBOOL           waitVSirqThenGetCommand(CmdQ_T &rCmd);
    /**
    * @brief non-busy wait of Vsync signal
    */
    MVOID           waitVSirq();
    /**
    * @brief flush Vsync signal in case waitVSirq() wait too long
    */
    MVOID           flushVSirq();
    MVOID           flushAFirq();

    /**
    * @brief implement sem_wait with timeout mechanism
    * @param [in] sem sem_t
    * @param [in] time_ms relative wait time in mini sec
    * @param [in] info message to print when time out
    */
    MBOOL           sem_wait_relativeTime(sem_t *pSem, nsecs_t reltime, const char* info, MBOOL isWaitVsync = MFALSE);

    /**
    * @brief query TG information (before waitVSirq), which is used to configured waitVSirq
    */
    MRESULT         queryTGInfoFromSensorHal();

    MRESULT         configUpdate(MUINT32 u4TgInfo, MINT32 i4MagicNum);
    MVOID           setMagicNum2AFMgr(MINT32 magicNum);
    MINT32          getStereo3DWarning();

private:
    pthread_t       mThread;
    pthread_t       mAESenThread;
    pthread_t       mPDThread;
    List<CmdQ_T>    mCmdQ;
    Mutex           mModuleMtx;
    Mutex           mAFMtx;
    Mutex           mAESenMtx;
    Mutex           mPDMtx;
    Mutex           mVsyncMtx;
    Condition       mCmdQCond;
    IspDrv*         mpIspDrv;
    sem_t           mSem;
    sem_t           semAFProcEnd;
    sem_t           semAEI2CSen;
    sem_t           semAESen;
    sem_t           semAESenThdEnd;
    sem_t           semPD;
    sem_t           semPDThdEnd;
    sem_t           semVsyncFromAE;
    NS3A::AaaTimer* mpVSTimer;
    MBOOL           mbListenVsync; //true means Update will be sent, wait Vsync first;   false means Update won't be sent, wait commands only
    MBOOL           mbEnable3ASetParams;
    MBOOL           mbInCaptureProcess; // Middleware control this flag. At Cshot, only do 1 time mbInCaptureProcess(TRUE) and mbInCaptureProcess(FALSE)
    MBOOL           mbInCapturing;      // CaptureStart -> CaptureEnd
    mutable MBOOL   mbEnAESenThd;
    MBOOL           mbEnPDThd;
protected:
    IspDrv*         mpIspDrv_forAF;
    sem_t           mSemAFThreadstart;
    MINT32          mbAFThreadLoop;
    MINT32          mbSemAFIRQWait;
    pthread_t       mAFThread;
    StateMgr*       mpStateMgr;
    mutable MINT32  mEnFlushVSIrq;
    mutable MINT32  mFlushVSIrqDone;
    mutable MINT32  mEnFlushAFIrq;
    mutable MINT32  mFlushAFIrqDone;


public:
    CallBackSet*    mpCbSet;
    AAA_Scheduler*  mpScheduler;
    NS3A::AaaTimer* mpStateLockTimer;
    MINT32 m_3AMagicNumber;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3A framework log/flow control
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MINT32 m_3ALogEnable;
    MINT32 m_3ACtrlEnable;
    MINT32 m_3ATimeOutEnable;
};


template <MINT32 sensorDevId>
class Hal3ADev : public Hal3A
{
public:
    static Hal3ADev* getInstance()
    {
        static Hal3ADev<sensorDevId> singleton;
        return &singleton;
    }
    Hal3ADev()
        : Hal3A()
    {
    }

private:

};


#include <sys/time.h>
#include <cutils/log.h>
class AaaTimer {
public:

    inline MINT32 getUsTime() //it's not accurate to find the elapsed time of an algorithm/process
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);

        return tv.tv_sec * 1000000 + tv.tv_usec;
    }

    AaaTimer()
        : mInfo(MNULL)
        , mIdx(0)
        , mLogEn(MFALSE)
        , mTime_ms(0.0)
    {}

    AaaTimer(const char* info, MINT32 sensorDevId, MBOOL logEn)
        : mInfo(info), mIdx(sensorDevId), mLogEn(logEn), mTime_ms(0)
    {
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
    }

    MVOID start(const char* info, MINT32 sensorDevId, MBOOL logEn, MBOOL bClrTime = 1) //used by global/static variables
    {
        mInfo = info;
        mIdx = sensorDevId;
        mLogEn = logEn;
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] timer start. bClrTime=%d\n", __FUNCTION__, mInfo, mIdx, bClrTime);
        if (bClrTime) mTime_ms = 0.0;
        clock_gettime(CLOCK_MONOTONIC, &ts_start);
    }
    double End()
    {
        struct timespec ts_end;
        struct timespec ts_elapsed;
        clock_gettime(CLOCK_MONOTONIC, &ts_end);
        ts_elapsed.tv_sec = ts_end.tv_sec - ts_start.tv_sec;
        ts_elapsed.tv_nsec = ts_end.tv_nsec - ts_start.tv_nsec;

        if (ts_elapsed.tv_nsec < 0)
        {
            ts_elapsed.tv_nsec += 1000000000;
            ts_elapsed.tv_sec -= 1;
        }

        double ts_elapsed_tv_msec = ((double)ts_elapsed.tv_nsec)/1000000;
        ts_elapsed_tv_msec += ts_elapsed.tv_sec*1000;
        mTime_ms += ts_elapsed_tv_msec;
        /*
        MINT32 endTime = getUsTime();
        double t = ((double)(endTime - mStartTime)) / 1000;
        mTime_ms += t;
        */
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): current=%f,   sum=%f\n", __FUNCTION__, mInfo, mIdx, ts_elapsed_tv_msec, mTime_ms);
        return mTime_ms;
    }

    double printTime()
    {
        if (mLogEn) ALOGD("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): sum=%f\n", __FUNCTION__, mInfo, mIdx, mTime_ms);
        return mTime_ms;
    }

    ~AaaTimer()
    {
    }

protected:
    const char* mInfo;
    struct timespec ts_start;
    MINT32 mIdx;
    MBOOL mLogEn;
    double mTime_ms;
};

//define log control
#define EN_3A_FLOW_LOG        1
#define EN_3A_SCHEDULE_LOG    2
#define EN_3A_STEREO_LOG      4

/*NeedUpdate*/
#define ENABLE_3A_CODE_BODY     0 // to disable unverified parts

// 3A module switch
#define ENABLE_3A_GENERAL       1
#define ENABLE_AAOBUF           2
#define ENABLE_AFOBUF           4
#define ENABLE_ISPTUNE          8
#define ENABLE_AWB              16
#define ENABLE_AE               32
#define ENABLE_AF               64
#define ENABLE_FLASH            128
#define ENABLE_LSC              256
#define ENABLE_FLICKER          512
#define ENABLE_FORCEVALIDATE    1024

#define SEM_TIME_LIMIT_NS       8000000000L

}; // namespace NS3A
typedef struct
{
    MINT64 iYvalue;
    MINT32 i4ISO;
    MINT32 i4IsAEStable;
    MINT32 i4SceneLV;
    MINT32 ishutterValue;
    MUINT8 aeBlockV[25];
    MINT32 i4IsFlashFrm;
    MINT32 i4AEBlockAreaYCnt;
    MUINT8 *pAEBlockAreaYvalue;
}AE2AFInfo_T;

typedef struct
{
    MINT32 i4Type;
    char* path;
}CollectCmd_T;

#endif
