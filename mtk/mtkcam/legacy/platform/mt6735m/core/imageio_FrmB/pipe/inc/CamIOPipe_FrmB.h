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
#ifndef _ISPIO_CAMIO_PIPE_H_
#define _ISPIO_CAMIO_PIPE_H_
//
#include <vector>
//
using namespace std;
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>
#include <mtkcam/hal/IHalSensor.h>
//
#include "cam_path_FrmB.h"

//#include <mtkcam/drv/sensor_hal.h>
//TODO:remove later
#include "isp_function_FrmB.h"
#include "mtkcam/imageio/p1HwCfg.h"
//#include "kd_imgsensor_define.h"
//
/*******************************************************************************
*
********************************************************************************/
using namespace NSIspDrv_FrmB;
using namespace NSDrvCam_FrmB;
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////


/*******************************************************************************
*
********************************************************************************/
class PipeImp;


/*******************************************************************************
*
********************************************************************************/
class CamIOPipe : public PipeImp
{
private:
    class DupCmdQMgr{
        public:
            DupCmdQMgr():
                mBurstQ(0),
                mDupCmdQIdx(0),
                m_bStart(0)
                {};
            void        CmdQMgr_start(void);
            void        CmdQMgr_stop(void);
            void        CmdQMgr_update(void);
            MUINT32     CmdQMgr_GetBurstQ(void);
            MUINT32     CmdQMgr_GetDuqQIdx(void);
            MBOOL       CmdQMgr_GetCurStatus(void);
        private:
            MUINT32 mBurstQ;
            MUINT32 mDupCmdQIdx;
            MUINT32 m_bStart;
    };

//
public:     ////    Constructor/Destructor.
                    CamIOPipe(
                        char const*const szPipeName,
                        EPipeID const ePipeID,
                        EDrvScenario const eScenarioID,
                        EScenarioFmt const eScenarioFmt,
                        MINT8 const szUsrName[32]);

                    virtual ~CamIOPipe();

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start();
    virtual MBOOL   startCQ0();
    virtual MBOOL   startCQ0B();
    //virtual MBOOL   startCQ0_D(); //Mark by Ryan - No use in mt6582
    //virtual MBOOL   startCQ0B_D(); //Mark by Ryan - No use in mt6582
    virtual MBOOL   stop();

public:     ////    Buffer Quening.
    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo);
    virtual MBOOL   dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);
    //
    virtual MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo, MBOOL bImdMode = 0);
    virtual MBOOL   dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
    virtual MBOOL   configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);
public:     ////    Commands.
    virtual MBOOL   onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2);
    virtual MBOOL   onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam);

//extend
public:     ////    Interrupt handling
    virtual MBOOL   irq(EPipePass pass, EPipeIRQ irq_int, int userEnum);
    virtual MBOOL   irq(EPipePass pass, EPipeIRQ irq_int, int irqclear, int userEnum);
    virtual MBOOL   registerIrq(Irq_t irq);
    virtual MBOOL   irq(Irq_t irq);
    virtual MUINT32* getIspReg(MUINT32 path);
public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);
//
private:
    mutable Mutex           mLock;  // for multi-thread
    //
    IspDrvShell*            m_pIspDrvShell;
    CamPathPass1            m_CamPathPass1;
    CamPathPass1Parameter   m_camPass1Param;
    vector<BufInfo>         m_vBufImgo;
    vector<BufInfo>         m_vBufImg2o;
    MINT32                  m_pass1_Raw_CQ;  //init val = CAM_ISP_CQ_NONE

    MINT32                  m_CQ0TrigMode;

    MINT32                  m_CQ0BTrigMode;

    MINT32                  m_CQ0CTrigMode;

    MUINT32                 m_pass1TwinMode;
    MINT8                   m_szUsrName[32];
    mutable Mutex           mRunTimeCfgLock;  // for multi-thread

    IspHrzCfg               m_hrz_cfg;
    DMACfg                  m_ImgoCfg;
    MINT32                  m_hrz_update_cnt;
    MUINT32                 mMagicNum0; //RRZO
    MUINT32                 mMagicNum1; //3A
    MUINT32                 mMagicNum2; //IMGO
    NSCam::NSIoPipe::NSCamIOPipe::HwRWCtrl                m_P1HWRWctrl[4];
    DupCmdQMgr              mDupCmdQMgr;
    MUINT32                 mPath1;
    MUINT32                 mOpenedPort;    //mapping to marco:_CmdQSwitchEn(...)
    MBOOL                   m_bBypassImgo;  //for v1 to bypass imgo at sendcommand:set_rrz,p1_update
    MBOOL                   m_bBypassImg2o;
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_CAMIO_PIPE_H_

