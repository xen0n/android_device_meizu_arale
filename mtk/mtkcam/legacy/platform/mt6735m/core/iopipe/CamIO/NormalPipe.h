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

//#include <mtkcam/iopipe/CamIO/IHalCamIO.h>
#include <mtkcam/iopipe/CamIO/INormalPipe_wrap.h>

/* sensor
--------------------------------------*/
class SensorHal;

#include <mtkcam/imageio/IPipe.h>
#include <mtkcam/imageio/ICamIOPipe.h>
#include <mtkcam/imageio/ispio_utility.h>

#if 0
#include <mtkcam/ispio_sw_scenario.h>
#endif
//for Wrapper
//#include <mtkcam/common.h>
//#include <mtkcam/common/hw/hwstddef.h>
//
//#include <mtkcam/imageio/ispio_stddef.h>
#include <mtkcam/imageio/ispio_pipe_ports.h>

//move in IHalCamIO.h
//#include <mtkcam/imageio/ispio_pipe_scenario.h>

#include <deque>

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
   MUINT32                    mOccupiedOwner;
}_sensorInfo_t;
/**
*@enum<SensorHalType_e>
*@brief:Sensor output data format type
*  we want use (halSensorType_e)sensorType defined in Sensor_Hal.h
*  but it is ambiguous in Sensor_Hal.h and IHalSensor.h
*  copy to here
*/
typedef enum SensorHalType_s {
    SENSOR_HAL_TYPE_RAW = 0,        /*!<RAW data format type*/
    SENSOR_HAL_TYPE_YUV = 1,         /*!<YUV data format type*/
    SENSOR_HAL_TYPE_YCBCR = 2,        /*!<YCBCR data format type */
    SENSOR_HAL_TYPE_RGB565 = 3,     /*!<RGB565 data format type*/
    SENSOR_HAL_TYPE_RGB888 = 4,        /*!<RGB888 data format type*/
    SENSOR_HAL_TYPE_JPEG = 5,        /*!<JPEG data format type*/
    SENSOR_HAL_TYPE_UNKNOWN = 0xFF,
} SensorHalType_e;


/**
 * @enum ECamIOPipeCmd
 * @brief CamIO pipe command
 */
enum ECamIOPipeCmd {
    ECamIOPipeCmd_QUERY_BAYER_RAW_SRIDE  = 0x1001,         /*!<  for query bayer raw stride.  */
};

/**
 * @enum EPortType
 * @brief Pipe Port Type.
 */
enum EPortType
{
    EPortType_Sensor,                        /*!< Sensor Port Type */
    EPortType_MemoryIn,                   /*!< Memory In Port Type */
    EPortType_MemoryOut,                /*!< Memory Out Port Type */
};

/**
 * @struct PortIDMem
 * @brief Pipe Port ID (Descriptor).
 *
 */
struct PortIDMem
{
public:     //// fields.
    /**
      * @var type
      * EPortType
      */
    EPortType     type    :   8;      //
    /**
      * @var index
      *  port index
      */
    MUINT32       index   :   8;      //
    /**
      * @var inout
      * 0:in/1:out
      */
    MUINT32       inout   :   1;      //
    /**
      * @var reserved
      * reserved for future use.
      */
    MUINT32       reserved:   15;
    //
public:     //// constructors.
    PortIDMem(
        EPortType const _eType  = EPortType_MemoryOut,
        MUINT32 const _index    = 0,
        MUINT32 const _inout    = 0
    )
    {
        type    = _eType;
        index   = _index;
        inout   = _inout;
        reserved= 0;
    }
    //
public:     //// operations.
    MUINT32 operator()() const
    {
        return  *reinterpret_cast<MUINT32 const*>(this);
    }
};


/**
 * @struct PortInfoMem
 * @brief Pipe Port Info.
 *
 */
struct PortInfoMem : public PortIDMem
{
public:     //// constructors.
    PortInfoMem()
        : PortIDMem()
    {
    }
    //
    //
    PortInfoMem(PortIDMem const& _PortID)
        : PortIDMem(_PortID)
    {
    }
    //
};

/**
 * @struct SensorPortInfo
 * @brief Sensor port configuration
 *
 */
struct SensorPortInfo : public PortInfoMem
{
public:
    /**
      * @var u4DeviceID
      * Sensor device id
      */
    MUINT32     u4DeviceID;
    /**
      * @var u4Scenario
      * Sensor scenario, preview/capture/video ...etc
      */
    MUINT32     u4Scenario;
    /**
      * @var u4Bitdepth
      * The sensor raw bitdepth, the value is 8, 10
      */
    MUINT32     u4Bitdepth;
    /**
      * @var fgBypassDelay
      * Bypass sensor delay if ture, useful when sensor chnage mode
      * such as from preview mode to capture mode
      */
    MBOOL       fgBypassDelay;    //
    /**
      * @var fgBypassScenaio
      * Bypass to set the scenario, if the user don't want to set
      * sensor setting again can set it to true.
      */
    MBOOL       fgBypassScenaio;
    /**
      * @var u4RawType
      * The raw type, 0: pure raw, 1: pre-process raw
      */
    MUINT32    u4RawType;


public:     //// constructors.
    SensorPortInfo()
        : PortInfoMem(PortIDMem(EPortType_Sensor, 0, 0))
        , u4DeviceID(0)
        , u4Scenario(0)
        , u4Bitdepth(8)
        , fgBypassDelay(MFALSE)
        , fgBypassScenaio(MFALSE)
        , u4RawType(0)
    {
    }

    //
    SensorPortInfo(
        MUINT32 const _u4DeviceID,
        MUINT32 const _u4Scenario,
        MUINT32 const _u4Bitdepth,
        MBOOL const _fgBypassDelay,
        MBOOL const _fgBypassScenaio,
        MUINT32 const _u4RawType = 0
        )
        : PortInfoMem(PortIDMem(EPortType_Sensor, 0, 0))
        , u4DeviceID(_u4DeviceID)
        , u4Scenario(_u4Scenario)
        , u4Bitdepth(_u4Bitdepth)
        , fgBypassDelay(_fgBypassDelay)
        , fgBypassScenaio(_fgBypassScenaio)
        , u4RawType(_u4RawType)
    {
    }
};

#define NPIPE_MAX_SENSOR_CNT        5
#define NPIPE_MAX_ENABLE_SENSOR_CNT 2

/******************************************************************************
 *        class NormalPipe
 *
 ******************************************************************************/

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

    //one-time conifg
    virtual MBOOL   configPipe(QInitParam const& vInPorts, NSImageio::NSIspio::EScenarioID const eSWScenarioID);
    virtual MBOOL   configPipe(QInitParam const& vInPorts);

    virtual MBOOL DummyFrame(QBufInfo const& rQBuf);
    //run-time config
    virtual MBOOL   configFrame(QFrameParam const& rQParam);

    //replace queue
    virtual MBOOL   replace(BufInfo const& bufOld, BufInfo const& bufNew);
    virtual MBOOL   sendCommand(MINT32 cmd, MINT32 arg1, MINT32 arg2, MINT32 arg3);

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
#if 0
    void Lock(void) ;
    void UnLock(void);
#endif

private: //flor wrapper
    MBOOL querySensorInfo(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4BitDepth, EImageFormat &eFmt,  MUINT32 &u4Width, MUINT32 &u4Height, MUINT32 & u4RawPixelID);
    MBOOL configSensor(MUINT32 const u4DeviceID, MUINT32 const u4Scenario, MUINT32 const u4Width, MUINT32 const u4Height, MUINT32 const byPassDelay, MUINT32 const u4ByPassScenario,  MBOOL const fgIsContinuous);
    MBOOL skipFrame(MUINT32 const u4SkipCount);

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
    }QueueMgr_cmd;
        MBOOL                   sendCmd(QueueMgr_cmd cmd,MUINT32 arg1,MUINT32 arg2);
    };



protected:
//    IHalSensor                 *mpHalSensor;
    MBOOL                         mbTwoPixelEnable;
    static MUINT32                mEnablePath;
//    static MUINT32                mTwinEnable[CAM_SV_2+1][2];//record this & twin mode

    NSImageio::NSIspio::ICamIOPipe    *mpCamIOPipe;
    FrameMgr                   *mpFrameMgr;
    const char                 *mpName;
    MSize                      mTgOut_size[5];//user marco _TGMapping for array indexing
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
    MBOOL                      mDumpNPipeFlag;
//    MBOOL                      mUpdateBQNumDone;
    /* for Wrapper */
    NSImageio::NSIspio::EScenarioID mScenarioID;
    NSImageio::NSIspio::EScenarioFmt mScenarioFmt;
private:
    MBOOL            m_bEISChkFlg;
    MBOOL            mfgIsYUVPortON;
    /* sensor */
    SensorHal*        mpSensorHal;
    SensorPortInfo    mrSensorPortInfo;
    MUINT32 mu4DeviceID;
    MUINT32 mu4SkipFrame;

private:
    MBOOL                       m_bStarted;
    MUINT32                    mRingBufDepth;    //total enabled dmao ring-buf-depth sum.

    #define                     MaxPortIdx  2       //so far, only 2 dmao r supported in 1 TG
    //container of dequed buffer, in order to chk replace or not
    QueueMgr*                    mpDeQueue[MaxPortIdx];
    MUINT32                     PortIndex[MaxPortIdx];    //record index in portmap.h

    sem_t               mSemDeQue;//lock deque
    sem_t               mSemEnQue;//lock enque
};

/******************************************************************************
 *
 ******************************************************************************/
};
};
};
#endif

