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

#ifndef _AAA_HAL_FLOWCTRL_H_
#define _AAA_HAL_FLOWCTRL_H_

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
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include "aaa_hal_if.h"
#include <ResultBufMgr.h>
#include <IEventIrq.h>
#include <dbg_aaa_param.h>

#include <utils/threads.h>
#include <utils/List.h>

#include <utils/threads.h>
#include <utils/List.h>
//-------------------------------
/*temp mark out*///#include <Local.h>

using namespace android;

#define ISP_P1_UPDATE (1)
#define NOTIFY_3A_DONE (1)

using namespace NSCam;
using namespace android;
using namespace NS3Av3;



namespace NS3Av3
{
//class StateMgr;
//class AAA_Scheduler;
class AaaTimer;
class IEventIrq;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class Hal3AFlowCtrl : public Hal3AIf
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3AFlowCtrl();
    virtual ~Hal3AFlowCtrl();

private: // disable copy constructor and copy assignment operator
    Hal3AFlowCtrl(const Hal3AFlowCtrl&);
    Hal3AFlowCtrl& operator=(const Hal3AFlowCtrl&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3AFlowCtrl* createInstance(MINT32 i4SensorOpenIndex);
    //static Hal3AFlowCtrl* getInstance(MINT32 i4SensorDevId);
    virtual MVOID destroyInstance();
    virtual MBOOL sendCommand(ECmd_T const eCmd, MINTPTR const i4Arg = 0);
    virtual MBOOL sendCommandDerived(ECmd_T const eCmd, MINTPTR const i4Arg = 0);
    virtual inline MBOOL getParams(Param_T &rParam) const
    {
        rParam = m_rParam;
        return MTRUE;
    }
    virtual MBOOL setParams(Param_T const &rNewParam);
    virtual MBOOL autoFocus();
    virtual MBOOL cancelAutoFocus();
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    //virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;
    virtual MBOOL getDebugInfo(android::Vector<MINT32>& keys, android::Vector< android::Vector<MUINT8> >& data) const {return MTRUE;}
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;
    virtual MINT32 attachCb(I3ACallBack* cb);
    virtual MINT32 detachCb(I3ACallBack* cb);
    virtual MBOOL setIspPass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf);
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);
    //virtual MINT32 isNeedFiringFlash(MBOOL bEnCal = MFALSE);
    virtual MVOID setSensorMode(MINT32 i4SensorMode);

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2);

    virtual MVOID setFrameId(MINT32 i4FrmId){m_i4FrmId = i4FrmId;}
    virtual MINT32 getFrameId() const {return m_i4FrmId;}
    virtual MUINT32 getSensorDev() const {return m_i4SensorDev;}
    virtual MINT32 getResult(MUINT32 u4FrmId, Result_T& rResult);
    virtual MINT32 getResultCur(MUINT32 u4FrmId, Result_T& rResult);
    virtual MVOID setFDEnable(MBOOL fgEnable){}
    virtual MBOOL setFDInfo(MVOID* prFaces){return 0;}
    virtual MVOID notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0);
    /**
    * @brief set 3A Preview Mode (refer to E3APreviewMode_T)
    */
    inline virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode)
    {
        m_PvMode = PvMode;
    }

    virtual inline MINT32 getErrorCode() const
    {
        return m_errorCode;
    }

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
    * @brief get 3A Preview Mode (refer to E3APreviewMode_T)
    */
    inline E3APreviewMode_T get3APreviewMode()
    {
        return m_PvMode;
    }


protected: //private:
    struct CmdQ_T
    {
        ECmd_T            eCmd;
        ParamIspProfile_T rParamIspProfile;

        CmdQ_T(ECmd_T _eCmd, const ParamIspProfile_T& _rParamIspProfile)
            : eCmd(_eCmd), rParamIspProfile(_rParamIspProfile) {}
    };


    virtual MBOOL validatePass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf);
    virtual MBOOL setSensorAndPass1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MBOOL setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);
    virtual MBOOL isMeetMainFlashOnCondition();
    virtual MBOOL isMeetPreFlashOnCondition();
    virtual MBOOL isStrobeBVTrigger();
    virtual MINT32 getCurrResult(MUINT32 i4FrmId, Result_T& rResult) const {return 0;}
    virtual MVOID updateResult(MUINT32 u4MagicNum);
    virtual MVOID updateImmediateResult(MUINT32 u4MagicNum){}
    virtual MINT32 getCurrentHwId() const{return 0;}

    /**
    * @brief collect all 3A result status, and notify 3A PROC FINISH event
    */
    MVOID on3AProcFinish(MUINT32 u4MagicNum, MUINT32 u4MagicNumCur);
    /**
    * @brief execute 3A commands in current 3A state
    * @param [in] r3ACmd 3A commands; please refer to aaa_hal_if.h
    */
    virtual MBOOL postCommand(ECmd_T const r3ACmd, MINTPTR const i4Arg = 0);
    virtual MBOOL postCommandDerived(ECmd_T const r3ACmd, MINTPTR const i4Arg = 0);
    /**
    * @brief create AE/AWB thread
    */
    virtual MVOID createThread();
    /**
    * @brief destroy AE/AWB thread
    */
    virtual MVOID destroyThread();
    /**
    * @brief change AE/AWB thread setting
    */
    virtual MVOID changeThreadSetting();
    /**
    * @brief AE/AWB thread execution function
    */
    static  MVOID*  onThreadLoop(MVOID*);
    /**
    * @brief add 3A commands in command queue
    * @param [in] r3ACmd 3A commands; please refer to aaa_hal_if.h
    */
    virtual MVOID addCommandQ(ECmd_T const &r3ACmd, MINTPTR i4Arg = 0);
    /**
    * @brief clear all ECmd_Update commands in current command queue
    */
    virtual MVOID clearCommandQ();
    /**
    * @brief get 3A command from the head of 3A command queue
    * @param [in] rCmd 3A commands; please refer to aaa_hal_if.h
    * @param [out] bGetCmd bool to indicate whether command is obtained
    * @param [in] en_timeout bool to indicate whether to enable timeout mechanism or not
    */
    virtual MBOOL getCommand(CmdQ_T &rCmd, MBOOL &bGetCmd, MBOOL en_timeout = MFALSE);

    /*
        * @brief wait for VS irq first, then get 3A command from the head of 3A command queue
        * @param [in] rCmd 3A commands; please refer to aaa_hal_if.h
        */
    //virtual MBOOL waitVSirqThenGetCommand(CmdQ_T &rCmd);

    /**
    * @brief implement sem_wait with timeout mechanism
    * @param [in] sem sem_t
    * @param [in] time_ms relative wait time in mini sec
    * @param [in] info message to print when time out
    */
    virtual MBOOL sem_wait_relativeTime(sem_t *pSem, nsecs_t reltime, const char* info);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected: //private:
    //volatile int  m_Users;
    //mutable Mutex m_Lock;
    //mutable Mutex m_ValidateLock;
    MRESULT       m_errorCode;
    Param_T       m_rParam;
    Result_T      m_rResult;
    ResultBufMgr  m_rResultBuf;
    ResultBufMgr  m_rResultBufCur;
    MBOOL         m_bReadyToCapture;
    MINT32        m_i4SensorDev;
    MINT32        m_i4SensorOpenIdx;
    MUINT         m_TgInfo;
    //MBOOL         m_bDebugEnable;
    MINT32        m_i4FrmId;
    MUINT32       m_u4FrmIdStat;
    MUINT32       m_3ACapMode;
    E3APreviewMode_T   m_PvMode;
    //MBOOL         m_bFaceDetectEnable;

public:
    MBOOL         m_b3APvInitOK;

protected: //private:
    pthread_t       mThread;
    List<CmdQ_T>    mCmdQ;
    Mutex           mModuleMtx;
    Mutex           m3AOperMtx1;
    Mutex           m3AOperMtx2;
    Condition       mCmdQCond;
    sem_t           mSem;
    NS3Av3::AaaTimer* mpVSTimer;
    MBOOL           mbEnable3ASetParams;
    MINT32          mLastFlashOnFrmIdx;

protected:
    //StateMgr*       mpStateMgr;
    IEventIrq*        mpIEventIrq;
    MBOOL           mbMainFlashOnThisFrame;

public:
    I3ACallBack*    mpCbSet;
    //AAA_Scheduler*  mpScheduler;
private:
    MBOOL           mbListenUpdate;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3A framework log/flow control flag
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    MINT32 m_3ALogEnable;
    MINT32 m_3ACtrlEnable;
};

#include <sys/time.h>
//#include <cutils/log.h>
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
        if (mLogEn) MY_LOG("[Timer:%s: %s, SensorDevId: %d] timer start. bClrTime=%d\n", __FUNCTION__, mInfo, mIdx, bClrTime);
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
        if (mLogEn) MY_LOG("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): current=%f,   sum=%f\n", __FUNCTION__, mInfo, mIdx, ts_elapsed_tv_msec, mTime_ms);
        return mTime_ms;
    }

    double printTime()
    {
        if (mLogEn) MY_LOG("[Timer:%s: %s, SensorDevId: %d] =====> time(ms): sum=%f\n", __FUNCTION__, mInfo, mIdx, mTime_ms);
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

#define SEM_TIME_LIMIT_NS       4000000000L

}; // namespace NS3Av3

#endif

