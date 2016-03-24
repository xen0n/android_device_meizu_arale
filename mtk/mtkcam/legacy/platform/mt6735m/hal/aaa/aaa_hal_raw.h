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

#ifndef _AAA_HAL_RAW_H_
#define _AAA_HAL_RAW_H_

//------------Thread-------------
#include <linux/rtpm_prio.h>
#include <pthread.h>
#include <semaphore.h>
//-------------------------------

//#include <mtkcam/drv/isp_drv.h>
#include <aaa_types.h>
#include <aaa_error_code.h>
#include <aaa_log.h>
#include <hal/aaa/aaa_hal_if.h>
#include <hal/aaa/aaa_hal_flowCtrl.h>
#include <hal/aaa/ResultBufMgr/ResultBufMgr.h>
//#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
//#include <mtkcam/iopipe/CamIO/INormalPipe.h>
#include <IEventIrq.h>

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
class StateMgr;
//class AAA_Scheduler;
class AaaTimer;
//class IEventIrq;
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++


class Hal3ARaw : public Hal3AFlowCtrl
{

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
protected:  //    Ctor/Dtor.
    Hal3ARaw();
    virtual ~Hal3ARaw();

private: // disable copy constructor and copy assignment operator
    Hal3ARaw(const Hal3ARaw&);
    Hal3ARaw& operator=(const Hal3ARaw&);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Interfaces.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //
    static Hal3ARaw* createInstance(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);
    static Hal3ARaw* getInstance(MINT32 i4SensorDevId);
    virtual MVOID destroyInstance();
    virtual MBOOL sendCommandDerived(ECmd_T const eCmd, MINTPTR const i4Arg = 0);
    virtual MBOOL setParams(Param_T const &rNewParam);
    virtual MBOOL autoFocus();
    virtual MBOOL cancelAutoFocus();
    virtual MBOOL setZoom(MUINT32 u4ZoomRatio_x100, MUINT32 u4XOffset, MUINT32 u4YOffset, MUINT32 u4Width, MUINT32 u4Height);
    //virtual MBOOL set3AEXIFInfo(IBaseCamExif *pIBaseCamExif) const;
    virtual MBOOL getDebugInfo(android::Vector<MINT32>& keys, android::Vector< android::Vector<MUINT8> >& data) const;
    virtual MINT32 getDelayFrame(EQueryType_T const eQueryType) const;
    virtual MINT32 attachCb(I3ACallBack* cb);
    //virtual MINT32 detachCb(I3ACallBack* cb);
    virtual MBOOL setIspPass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf);
    virtual MINT32 updateCaptureParams(CaptureParam_T &a_rCaptureInfo);
    //virtual MINT32 isNeedFiringFlash(MBOOL bEnCal);
    virtual MVOID setSensorMode(MINT32 i4SensorMode);
    virtual MVOID setFDEnable(MBOOL fgEnable);
    virtual MBOOL setFDInfo(MVOID* prFaces);
    virtual MVOID notifyP1Done(MUINT32 u4MagicNum, MVOID* pvArg = 0);

    virtual MINT32 send3ACtrl(E3ACtrl_T e3ACtrl, MINTPTR i4Arg1, MINTPTR i4Arg2);

    //virtual MVOID setFrameId(MINT32 i4FrmId){m_i4FrmId = i4FrmId;}
    //virtual MINT32 getFrameId() const {return m_i4FrmId;}
    //virtual MINT32 getResult(MUINT32 i4FrmId, Result_T& rResult) const {return m_rResultBuf.getResult(i4FrmId, rResult);}

/*
    inline virtual MVOID set3APreviewMode(E3APreviewMode_T PvMode)
    {
        m_PvMode = PvMode;
    }

    virtual inline MINT32 getErrorCode() const
    {
        return m_errorCode;
    }
*/
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Implementation
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
     //
    MRESULT initRaw(MINT32 i4SensorDevId, MINT32 i4SensorOpenIndex);

    MRESULT uninitRaw();
    MVOID postToAESenThread();
/*
    inline MINT32 getSensorDev()
    {
        return m_i4SensorDev;
    }
    inline MINT32 getSensorOpenIdx()
    {
        return m_i4SensorOpenIdx;
    }
    inline MUINT getTGInfo()
    {
        return m_TgInfo;
    }
    inline MUINT32 get3ACapMode()
    {
        return m_3ACapMode;
    }
    inline E3APreviewMode_T get3APreviewMode()
    {
        return m_PvMode;
    }
*/


protected: //private:

    virtual MBOOL           validatePass2(MINT32 flowType, const NSIspTuning::RAWIspCamInfo& rCamInfo, void* pRegBuf);
    virtual MBOOL           setSensorAndPass1(const ParamIspProfile_T& rParamIspProfile, MBOOL fgPerframe);
    virtual MINT32          getCurrResult(MUINT32 i4FrmId, Result_T& rResult) const;
    virtual MVOID           updateResult(MUINT32 u4MagicNum);
    virtual MBOOL           isStrobeBVTrigger();
    virtual MBOOL           setFlashLightOnOff(MBOOL bOnOff/*1=on; 0=off*/, MBOOL bMainPre/*1=main; 0=pre*/);

    virtual MBOOL           postCommandDerived(ECmd_T const r3ACmd, MINTPTR const i4Arg = 0);
    virtual MVOID           updateImmediateResult(MUINT32 u4MagicNum);

    virtual MBOOL           get3AEXIFInfo(EXIF_3A_INFO_T& rExifInfo) const;
    virtual MBOOL           getASDInfo(ASDInfo_T& rASDInfo) const;
    virtual MINT32          getCurrentHwId() const;

    MVOID           createThreadRaw();
    MVOID           destroyThreadRaw();
    //MVOID           changeThreadSetting();
    //static  MVOID*  onThreadLoop(MVOID*);
    //MVOID           addCommandQ(ECmd_T const &r3ACmd, MINT32 i4Arg = 0);
    //MVOID           clearCommandQ();
    //MBOOL           getCommand(CmdQ_T &rCmd, MBOOL &bGetCmd, MBOOL en_timeout = MFALSE);
    //MBOOL           waitVSirqThenGetCommand(CmdQ_T &rCmd);
    //MBOOL           sem_wait_relativeTime(sem_t *pSem, nsecs_t reltime, const char* info);

    /**
    * @brief change AE sensor I2C thread setting
    */
    MVOID changeAESensorThreadSetting();

    /**
    * @brief AE sensor I2C thread execution function
    */
    static MVOID* AESensorThreadLoop(MVOID*);

    /**
    * @brief Enable AF thread
    * @param [in] a_bEnable set 1 to enable AF thread
    */
    virtual MRESULT EnableAFThread(MINT32 a_bEnable);

private:
    /**
    * @brief AF thread execution function
    */
    static MVOID* AFThreadFunc(void *arg);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Data member
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
private:
    volatile int  m_Users;
    mutable Mutex m_Lock;
    mutable Mutex m_ValidateLock;

private:
    pthread_t       mAESenThread;
    Mutex           mAFMtx;
    Mutex           mAESenMtx;
    Mutex           mP2Mtx;
    sem_t           semAFProcEnd;
    sem_t           semAESen;
    sem_t           semAESenThdEnd;
    MBOOL           mbEnAESenThd;
    MUINT32         m_u4FlashSeq;

protected:
    sem_t           mSemAFThreadstart;
    MINT32          mbAFThreadLoop;
    MINT32          mbSemAFIRQWait;
    pthread_t       mAFThread;
    StateMgr*       mpStateMgr;
    MBOOL           m_bFaceDetectEnable;
    IEventIrq*      mpAFEventIrq;
    MINT32          mi4InCaptureProcess;

public:
    MBOOL           mbPostAESenThd;
    //I3ACallBack*    mpCbSet;
    //AAA_Scheduler*  mpScheduler;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  3A framework log/flow control flag
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:
    //MINT32 m_3ALogEnable;
    //MINT32 m_3ACtrlEnable;
};

template <MINT32 sensorDevId>
class Hal3ARawDev : public Hal3ARaw
{
public:
    static Hal3ARawDev* getInstance()
    {
        static Hal3ARawDev<sensorDevId> singleton;
        return &singleton;
    }
    Hal3ARawDev()
        : Hal3ARaw()
    {
    }

private:

};


}; // namespace NS3Av3

#endif

