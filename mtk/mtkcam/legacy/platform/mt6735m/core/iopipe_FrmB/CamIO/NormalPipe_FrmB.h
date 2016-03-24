/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NormalPipe_FRMB_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NormalPipe_FRMB_H_

#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <mtkcam/ispio_sw_scenario.h>
#include "NormalPipe.h"     //include this for using class:normalpipe
#include <deque>
//for thread/semapher
#include <utils/threads.h>
#include <cutils/atomic.h>
#include <semaphore.h>
#include <pthread.h>
#include <mtkcam/v1/config/PriorityDefs.h>

//#include "mtkcam/imageio/p1HwCfg.h"
//#include "mtkcam/drv/isp_drv.h"

//#define IOPIPE_SET_MODUL_REG(handle,RegName,Value) HWRWCTL_SET_MODUL_REG(handle,RegName,Value)
//#define IOPIPE_SET_MODUL_REGS(handle, StartRegName, size, ValueArray) HWRWCTL_SET_MODUL_REGS(handle, StartRegName, size, ValueArray)

//class ICamIOPipe;
/******************************************************************************
 *
 ******************************************************************************/
namespace NSCam {
namespace NSIoPipe {
namespace NSCamIOPipe {

#ifndef USING_MTK_LDVT
#define USE_IMAGEBUF_HEAP
#endif
/******************************************************************************
 *
 *
 ******************************************************************************/
#define MaxPortIdx  2       //so far, only 2 dmao r supported in 1 TG
class NormalPipe_FrmB_Thread;

class NormalPipe_FrmB : public IHalCamIO, public INormalPipe_FrmB
{
friend class NormalPipe_FrmB_Thread;
public:
     //burstQnum is only for mw member to update,3a member do not need to touch this
     NormalPipe_FrmB(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum);

//protected:
                   ~NormalPipe_FrmB(){};
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();

    virtual MBOOL   stop();

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MBOOL   enque(QBufInfo const& rQBuf);

    virtual MBOOL   deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs = 0xFFFFFFFF);

    //one-time conifg
    virtual MBOOL   configPipe(QInitParam const& vInPorts);

    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

    virtual MBOOL   DummyFrame(QBufInfo const& rQBuf);

    virtual MBOOL   configFrame(QFrameParam const& rQParam){return MFALSE;};

    //replace queue
    virtual MBOOL   replace(BufInfo const& bufOld, BufInfo const& bufNew){return MFALSE;};

    typedef enum{
        _DMAO_NONE   = 0x0,
        _IMGO_ENABLE = 0x1,
        _IMG2O_ENABLE= 0x10,
    }eDMAO;
    typedef enum{
        _GetPort_Opened     = 0x1,
        _GetPort_OpendNum   = 0x2,
        _GetPort_Index      = 0x3,
    }ePortInfoCmd;
    MUINT32        GetOpenedPortInfo(NormalPipe_FrmB::ePortInfoCmd cmd,MUINT32 arg1=0);

    MUINT32        GetRingBufferDepth(void);
public:
    virtual MVOID   attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType);
    virtual MBOOL   wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0xFFFFFFFF);
    MBOOL   wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0x300000);

public:
    virtual MUINT32 getIspReg(::ESoftwareScenario scen);

protected:
    MBOOL _setTgPixelAndDualMode(void);
    inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }
    MBOOL    checkEnque(QBufInfo const& rQBuf);
    MBOOL    checkDropEnque(QBufInfo const& rQBuf);
#if 0
    void Lock(void) ;
    void UnLock(void);
#endif


public:
    class FrameMgr {
    /////
    protected:
#ifdef USE_IMAGEBUF_HEAP
        typedef DefaultKeyedVector<int, IImageBuffer* >  MapType;
#else
        typedef DefaultKeyedVector<int, BufInfo* >       MapType;
#endif

        MapType                 mvFrameQ;
        mutable Mutex              mFrameMgrLock;   //for frameMgr enque/deque operation
    /////
    public:
                                FrameMgr()
                                    : mvFrameQ()
                                    {}

        virtual                 ~FrameMgr(){};
        ////
        void                    init();
        void                    uninit();
#ifdef USE_IMAGEBUF_HEAP
        void                    enque(IImageBuffer* pframe);
        IImageBuffer*           deque(int key);
#else
        void                    enque(const BufInfo& pframe);
        BufInfo*                deque(int key);
#endif

    };


    MINT32                     mUserCnt;
    MUINT32                    mpSensorIdx[NPIPE_MAX_SENSOR_CNT];   //0/1/2
    MUINT32                    mpEnabledSensorCnt;

class QueueMgr {
    /////
    protected:
        std::deque<BufInfo>    m_v_pQueue;
        mutable Mutex           mQueueLock;   //for frameMgr enque/deque operation
    /////
    public:
                                QueueMgr()
                                    : m_v_pQueue()
                                    {}

        virtual                 ~QueueMgr(){};
        ////
        void                    init();
        void                    uninit();
        //FIFO cmd
    typedef enum{
        eCmd_getsize = 0,
        eCmd_push = 1,
        eCmd_pop = 2,
        eCmd_front = 3,
        eCmd_end = 4,
        eCmd_at = 5,
        eCmd_pop_back = 6
    }QueueMgr_cmd;
        MBOOL                   sendCmd(QueueMgr_cmd cmd,MUINTPTR arg1,MUINTPTR arg2);
    };


protected:
    IHalSensor                 *mpHalSensor;

    MBOOL                         mbTwoPixelEnable;
    static MUINT32                mEnablePath;
    static unsigned long                mTwinEnable[CAM_SV_2+1][2];//record this & twin mode

    NSImageio_FrmB::NSIspio_FrmB::ICamIOPipe    *mpCamIOPipe;
    FrameMgr                   *mpFrameMgr;
    const char                 *mpName;
    MSize                      mTgOut_size[3];//user marco _TGMapping for array indexing
    MSize                      mImgoOut_size;
    MSize                      mImg2oOut_size;
    mutable Mutex              mLock;           //mutex for container
    mutable Mutex              mDeQLock;         //mutex
    mutable Mutex              mEnQLock;         //protect enque() invoked by multi-thread
    mutable Mutex              mCfgLock;         //mutex
    MBOOL                      mConfigDone;

    //
    MINT32                     mBurstQNum;
    MBOOL                      mUpdateBQNumDone;
private:
    MBOOL                      m_bStarted;
    MUINT32                    mRingBufDepth;   //total enabled dmao ring-buf-depth sum.

    //array indexing r mapping by macro:_PortMap(...)
    QueueMgr*                   mpEnqueRequest[MaxPortIdx];
    QueueMgr*                   mpDummyRequest[MaxPortIdx];
    QueueMgr*                   mpEnqueReserved[MaxPortIdx];
    //container of dequed buffer, in order to chk replace or not
    QueueMgr*                   mpDeQueue[MaxPortIdx];
    MUINT32                     mOpenedPort;
    MUINT32                     PortIndex[MaxPortIdx];  //record index in portmap.h
    MUINT32                     mPrvEnqSOFIdx;
    MBOOL                       m_b1stEnqLoopDone;
public:
    mutable Mutex               mEnqContainerLock;  //protect sw enque container which is accessed by drv_thread & other user threads
    NormalPipe_FrmB_Thread      *mpFrmB_Thread;
    fp_DropCB                   m_DropCB;
    void*                       m_returnCookie; //acompany with m_DropCB;
    MBOOL                       m_PerFrameCtl;//1:supports per frame control
};


class NormalPipe_FrmB_Thread
{
public:
    /**
        @param obj:obj address
    */
    NormalPipe_FrmB_Thread(NormalPipe_FrmB* obj);
    ~NormalPipe_FrmB_Thread(){};
    static NormalPipe_FrmB_Thread* CreateInstance(NormalPipe_FrmB* obj);
    MBOOL                   DestroyInstance(void);
    MBOOL                   init(void);
    MBOOL                   uninit(void);
    static MVOID*           IspEnqueThread(void* arg);
    MBOOL                   Start(void);
    MBOOL                   PreStop(void);
    MBOOL                   Stop(void);
    MVOID                   EnqueRequest(MUINT32 sof_idx);
    MVOID                   ImmediateRequest(MBOOL bByPassHwEnq);
private:
    //QBufInfo*               ReplaceChk(QBufInfo* prQBuf);
    MBOOL                   Enque(BufInfo* pBuf);
    MBOOL                   DmaCfg(BufInfo* pParam);
    MBOOL                   TuningUpdate(MUINT32 magNum);
    MVOID                   FrameToEnque(MUINT32 _OpenedPort);
    MVOID                   DummyFrameToEnque(MUINT32 _OpenedPort);

    sem_t                   m_semEnqueThread;
    pthread_t               m_EnqueThread;
    mutable Mutex           mLock;//lock m_bStart
    MBOOL                   m_bStart;
    volatile MUINT32        mCnt;
    NormalPipe_FrmB*        m_pNormalpipe_FrmB;
    MUINT32                 m_EnqueCnt;
    std::vector<MUINT32>    m_DropEnQ;
};

};
};
};
#endif

