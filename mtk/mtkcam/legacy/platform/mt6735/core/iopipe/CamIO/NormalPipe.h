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

#ifndef _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NORMALPIPE_H_
#define _MTK_PLATFORM_HARDWARE_MTKCAM_IOPIPE_CAMIO_NORMALPIPE_H_

#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe.h>
//#include <mtkcam/drv/sensor_hal.h>
#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_utility.h>
#include <mtkcam/ispio_sw_scenario.h>
#include <mtkcam/hal/IHalSensor.h>
#include "mtkcam/imageio/ispio_stddef.h" //add this for p1 tuning notification

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
//#define ENABLE_FRAME_MGR
#endif
/******************************************************************************
 *
 *
 ******************************************************************************/

#define NPIPE_MAX_SENSOR_CNT        5
#define NPIPE_MAX_ENABLE_SENSOR_CNT 2
typedef struct
{
   MUINT32                    mIdx;
   MUINT32                    mTypeforMw;
   MUINT32                    mDevId;//main/sub/main0/...
   IHalSensor::ConfigParam    mConfig;
   SensorStaticInfo           mSInfo; //static
   SensorDynamicInfo          mDInfo; //Dynamic
   MUINTPTR                   mOccupiedOwner;
}_sensorInfo_t;




class platSensorsInfo{
public :
    platSensorsInfo(void){
        mUserCnt = 0;
        mExistedSCnt = 0;
        mSList = NULL;
        memset(mSenInfo, sizeof(_sensorInfo_t) * NPIPE_MAX_SENSOR_CNT, 0);
    }


   IHalSensorList *mSList;
   MUINT32        mExistedSCnt;
   _sensorInfo_t  mSenInfo[NPIPE_MAX_SENSOR_CNT];
   MUINT32        mUserCnt;
};



class NormalPipe : public IHalCamIO, public INormalPipe
{
public:
     //burstQnum is only for mw member to update,3a member do not need to touch this
     NormalPipe(MUINT32* pSensorIdx, MUINT32 SensorIdxCnt, char const* szCallerName, MINT32 burstQnum);
     NormalPipe(char const* szCallerName); //temp to avoid build error

//protected:
                   ~NormalPipe(){};
public:
    virtual MVOID   destroyInstance(char const* szCallerName);

    virtual MBOOL   start();

    virtual MBOOL   stop();

    virtual MBOOL   init();

    virtual MBOOL   uninit();

    virtual MBOOL   enque(QBufInfo const& rQBuf);

    virtual MBOOL   deque(QBufInfo& rQBuf, MUINT32 u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   Reset();

    //one-time conifg
    virtual MBOOL   configPipe(QInitParam const& vInPorts);

    //run-time config
    virtual MBOOL   configFrame(QFrameParam const& rQParam);

    //replace queue
    virtual MBOOL   replace(BufInfo const& bufOld, BufInfo const& bufNew);

    virtual MBOOL   DummyFrame(QBufInfo const& rQBuf){return MFALSE;};
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);//QQ

public:
    virtual MVOID   attach(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType);
    virtual MBOOL   wait(ISignalConsumer *pConsumer, MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0xFFFFFFFF);
    MBOOL   wait(MINT32 sensorIdx, EPipeSignal eType, uint64_t ns_timeout = 0x300000);

public:
    virtual MUINT32* getIspReg(::ESoftwareScenario scen);

protected:
    MBOOL _setTgPixelAndDualMode(void);
    inline MINT64   getTimeStamp_ns(MUINT32 i4TimeStamp_sec, MUINT32 i4TimeStamp_us) const
    {
        return  i4TimeStamp_sec * 1000000000LL + i4TimeStamp_us * 1000LL;
    }
#if 0
    void Lock(void) ;
    void UnLock(void);
#endif


public:
    class FrameMgr {
    /////
    protected:
#ifdef USE_IMAGEBUF_HEAP
        typedef DefaultKeyedVector<MINTPTR, IImageBuffer* >  MapType;
#else
        typedef DefaultKeyedVector<MINTPTR, BufInfo* >       MapType;
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
        IImageBuffer*           deque(MINTPTR key);
#else
        void                    enque(const BufInfo& pframe);
        BufInfo*                deque(MINTPTR key);
#endif

    };


    MINT32                     mUserCnt;
    MUINT32                    mpSensorIdx[NPIPE_MAX_SENSOR_CNT];   //0/1/2
    MUINT32                    mpEnabledSensorCnt;

protected:
    IHalSensor                 *mpHalSensor;
    IHalSensor::ConfigParam    m_sensorConfigParam;

    MBOOL                         mbTwinEnable;
    static MUINT32                mEnablePath;
    static unsigned long          mTwinEnable[CAM_SV_2+1][2];//record this & twin mode

    NSImageio::NSIspio::ICamIOPipe    *mpCamIOPipe;
    FrameMgr                   *mpFrameMgr;
    const char                 *mpName;

    MSize                      mTgOut_size[5];
    MSize                      mImgoOut_size;
    MSize                      mRrzoOut_size;
    MSize                      mImgodOut_size;
    MSize                      mRrzodOut_size;
    MSize                      mCamSvImgoOut_size;
    MSize                      mCamSv2ImgoOut_size;
    mutable Mutex              mLock;         //mutex
    mutable Mutex              mDeQLock;         //mutex
    mutable Mutex              mEnQLock;         //mutex
    mutable Mutex              mCfgLock;         //mutex
    MBOOL                      mConfigDone;

    MUINT32                    mFrmCntPerMnum; //dbg only
    MUINT32                    mLastMnum; //dbg only
    //
    MINT32                     mBurstQNum;
    MBOOL                      mUpdateBQNumDone;
    MBOOL                      mDumpNPipeFlag;
private:
    MBOOL                      m_bEISChkFlg;
    MBOOL                      mDynSwtRawType;//1:dynamically switch processed/pure raw
    MBOOL                      mPureRaw;//0: processed raw, 1:pure raw
};


};
};
};
#endif

