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
#ifndef _ISPIO_POSTPROC_PIPE_H_
#define _ISPIO_POSTPROC_PIPE_H_
//
#include <vector>
#include <map>
#include <list>
//
using namespace std;
//
//
#include <mtkcam/imageio/ispio_pipe_ports.h>
#include <mtkcam/imageio/ispio_pipe_buffer.h>
//
#include "cam_path_FrmB.h"

/*******************************************************************************
*
********************************************************************************/
using namespace NSIspDrv_FrmB;
using namespace NSDrvCam_FrmB;
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////

#define DMA_PORT_TYPE_NUM   5
#define DMA_OUT_PORT_NUM    7


/*******************************************************************************
*
********************************************************************************/
class PipeImp;
struct Scen_Map_CropPathInfo_STRUCT{
    EDrvScenario u4DrvScenId;
    MUINT32 u4CropGroup;
    MINT32  u4PortID[DMA_PORT_TYPE_NUM];
    Scen_Map_CropPathInfo_STRUCT()
    {
       u4DrvScenId = eDrvScenario_CC;
       u4CropGroup = 0;
       u4PortID[0] = EPortIndex_IMG2O;
       u4PortID[1] = EPortIndex_IMG2O;
       u4PortID[2] = EPortIndex_IMG2O;
       u4PortID[3] = EPortIndex_IMG2O;
       u4PortID[4] = EPortIndex_IMG2O;
       u4PortID[5] = EPortIndex_IMG2O;
       u4PortID[6] = EPortIndex_IMG2O;
       u4PortID[7] = EPortIndex_IMG2O;
    }

    Scen_Map_CropPathInfo_STRUCT(EDrvScenario drvScenId,
        MUINT32 cropGroup, MINT32 portid[])
    {
       u4DrvScenId = drvScenId;
       u4CropGroup = cropGroup;
       u4PortID[0] = portid[0];
       u4PortID[1] = portid[1];
       u4PortID[2] = portid[2];
       u4PortID[3] = portid[3];
       u4PortID[4] = portid[4];
       u4PortID[5] = portid[5];
       u4PortID[6] = portid[6];
       u4PortID[7] = portid[7];
    }
};



/*******************************************************************************
*
********************************************************************************/
class PostProcPipe : public PipeImp
{
public:     ////    Constructor/Destructor.
                    PostProcPipe(
                        char const*const szPipeName,
                        EPipeID const ePipeID,
                        EDrvScenario const eScenarioID,
                        EScenarioFmt const eScenarioFmt,
                        MINT8 const szUsrName[32]);

                    virtual ~PostProcPipe();

public:     ////    Instantiation.
    virtual MBOOL   init();
    virtual MBOOL   uninit();

public:     ////    Operations.
    virtual MBOOL   start();
    virtual MBOOL   stop();

public:     ////    Buffer Quening.
    virtual MBOOL   dequeMdpFrameEnd(MINT32 const eDrvSce);
    virtual MBOOL   enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 enqueCq,MINT32 dupCqIdx);
    virtual EIspRetStatus  dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,EDrvScenario eDrvSce=eDrvScenario_CC, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);
    //
    virtual MBOOL   enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 enqueCq,MINT32 dupCqIdx);
    virtual EIspRetStatus   dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo,EDrvScenario eDrvSce=eDrvScenario_CC, MINT32 dequeCq=0,MINT32 const dequeBurstQIdx=0, MINT32 const dequeDupCqIdx=0, MUINT32 const u4TimeoutMs = 0xFFFFFFFF);

    virtual MBOOL   queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum);

    virtual MBOOL   queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths);

public:     ////    Settings.
    virtual MBOOL   configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, PipePackageInfo *pPipePackageInfo);


    virtual MBOOL   configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts);

public:     ////    Commands.
    virtual MBOOL   onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2);
    virtual MBOOL   onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam);

public:     ////    Interrupt handling
    virtual MBOOL   irq(EPipePass pass,EPipeIRQ irq_int, int userEnum);

public:     ////    original style sendCommand method
    virtual MBOOL   sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3);

public:     ////    enqueue/dequeue control in ihalpipewarpper
    virtual MBOOL   endequeFrameCtrl(EPIPE_BUFQUECmd cmd,MUINT32 callerID,MINT32 p2burstQIdx,MINT32 p2dupCQIdx,MINT32 timeout=0);
public:
    /*Mark by Ryan - No use in mt6582
    virtual MBOOL   startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps,EDrvScenario drvScen,MUINT32 cqIdx);//slow motion
    virtual MBOOL   stopVideoRecord(EDrvScenario drvScen,MUINT32 cqIdx);//slow motion
    */
public:     ////

private:
    MBOOL getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup);
    IspDrvShell*             m_pIspDrvShell;
    CamPathPass2             m_CamPathPass2;
    CamPathPass2Parameter    m_camPass2Param;
    vector<BufInfo>          m_vBufImgi;
    vector<BufInfo>          m_vBufVipi;
    vector<BufInfo>          m_vBufVip2i;
    vector<BufInfo>          m_vBufDispo;
    vector<BufInfo>          m_vBufVido;
    MINT8                    m_szUsrName[32];
    //
    MUINT32 cq1_size,cq1_phy;
    MUINT8 *cq1_vir;
    MINT32 cq1_memId;
    MUINT32 cq2_size,cq2_phy;
    MUINT8 *cq2_vir;
    MINT32 cq2_memId;
    MUINT32 cq3_size,cq3_phy;
    MUINT8 *cq3_vir;
    MINT32 cq3_memId;
    //
    EPipePass   m_pipePass;
    //
    MINT32  m_pass2_CQ;
    //
    MBOOL   m_isImgPlaneByImgi;
    //
    MBOOL   m_SeeeEn;
    //
    MUINT32 pixIdP2; //pixel id got from sensor
    //
};


////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio
#endif  //  _ISPIO_POSTPROC_PIPE_H_

