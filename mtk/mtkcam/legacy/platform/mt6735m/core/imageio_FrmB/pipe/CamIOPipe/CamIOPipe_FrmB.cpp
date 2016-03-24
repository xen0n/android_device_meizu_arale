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
#define LOG_TAG "iio/camio_FrmB"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PipeImp_FrmB.h"
#include "CamIOPipe_FrmB.h"
//
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/iopipe/CamIO/IHalCamIO.h> // for p1hwcfg module sel

/*******************************************************************************
*
********************************************************************************/
//using namespace NSCam;
//using namespace NSIoPipe;
//using namespace NSCamIOPipe;
using namespace NSCam;
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////


/**************************************************************************
 *                      D E F I N E S / M A C R O S                       *
 **************************************************************************/

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif
//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

/*******************************************************************************
* LOCAL PRIVATE FUNCTION
********************************************************************************/

MUINT32 _p1GetModuleTgIdx(MUINT32 id,MBOOL bTwin) {\
    switch (id) {
        case EModule_AE     :
        case EModule_AWB    :
        case EModule_SGG1   :
        case EModule_FLK    :
        case EModule_AF     :
        //case EModule_SGG2   :
        //case EModule_LCS    :
        case EModule_AAO    :
        case EModule_ESFKO  :
        case EModule_AFO    :
        case EModule_EISO   :
        //case EModule_LCSO   :
        //case EModule_iHDR   :
             return EPortIndex_TG1I;
             break;
        default:
            PIPE_ERR("Not support module(%d)",id);
            return EPortIndex_UNKNOW;
            break;
    }
}


MUINT32 p1ConvModuleID(MUINT32 id) {
    MUINT32 newId;
    switch (id) {
        case EModule_AE: newId = CAM_ISP_AE; break;
        case EModule_AWB: newId = CAM_ISP_AWB; break;
        case EModule_SGG1: newId = CAM_ISP_SGG; break;
        case EModule_FLK: newId = CAM_ISP_FLK; break;
        case EModule_AF: newId = CAM_ISP_AF; break;
        //case EModule_SGG2: newId = CAM_ISP_SGG2; break;
        //case EModule_LCS: newId = CAM_ISP_LCS; break;
        case EModule_AAO: newId = CAM_DMA_AAO; break;
        case EModule_ESFKO: newId = CAM_DMA_ESFKO; break;
        case EModule_AFO: newId = CAM_DMA_AFO; break;
        case EModule_EISO: newId = CAM_DMA_EISO; break;
        //case EModule_LCSO: newId = CAM_DMA_LCSO; break;
        //case EModule_iHDR: newId = CAM_CTL_IHDR; break;
        //case EModule_AE_D: newId = CAM_ISP_AE_D; break;
        //case EModule_AWB_D: newId = CAM_ISP_AWB_D; break;
        //case EModule_SGG1_D: newId = CAM_ISP_SGG1_D; break;
        //case EModule_AF_D: newId = CAM_ISP_AF_D; break;
        //case EModule_LCS_D: newId = CAM_ISP_LCS_D; break;
        //case EModule_AAO_D: newId = CAM_DMA_AAO_D; break;
        //case EModule_AFO_D: newId = CAM_DMA_AFO_D; break;
        //case EModule_LCSO_D: newId = CAM_DMA_LCSO_D; break;
        //case EModule_iHDR_D: newId = CAM_CTL_IHDR_D; break;
        default:
            PIPE_ERR("Not support module(%d)",id);
            newId= -1;
            break;
    }
    return newId;
}

NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_MODULE
 p1ConvModuleToSelectID(MUINT32 id) {

   NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_MODULE newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_RSVD;

    switch (id) {
        //raw
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG:         newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_SGG; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN:      newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_SGG_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_HRZ:     newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_SGG_HRZ; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_AA:          newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_AA; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_AA_EN:       newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_AA_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_EIS_DB:      newId = NSCam::NSIoPipe::NSCamIOPipe::HWRWCTRL_P1SEL_EIS_DB; break;
        default:
            PIPE_ERR("p1ConvModuleToSelectID::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
    }
    return newId;
}


MINT32 _p1ConvSelectIdToTg(MUINT32 id,MBOOL btwin) {

    switch (id) {
        //raw
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_HRZ:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_AA:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_AA_EN:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_EIS_DB:
            return EPortIndex_TG1I;
        default:
            PIPE_ERR("_p1ConvSelectIdToTg::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
            return  EPortIndex_UNKNOW;
    }
    return  EPortIndex_TG1I;
}


MINT32 getOutPxlByteNFmt( MBOOL bDupicate,MBOOL bCamSV,MUINT32 bRRZO, MUINT32 imgFmt, MINT32* pPixel_byte, MINT32* pFmt, MINT32* pEn_p1)
{
    //
    if(bCamSV){
        bCamSV = MFALSE;
        PIPE_INF("warning: 82 support no camsv\n");
    }
    if(bRRZO){
        bRRZO = MFALSE;
        PIPE_INF("warning: 82 support no RRZO\n");
    }
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }

    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = CAM_FMT_SEL_BAYER8;
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = CAM_FMT_SEL_BAYER10;
            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = CAM_FMT_SEL_BAYER12;
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = CAM_FMT_SEL_YUV422_1P;
            *pEn_p1 &= (~(CAM_CTL_EN1_PAK_EN|CAM_CTL_EN1_PAK2_EN));
            break;
        case eImgFmt_RGB565:
        case eImgFmt_RGB888:
        case eImgFmt_JPEG:
        case eImgFmt_FG_BAYER8:
        case eImgFmt_FG_BAYER10:
        case eImgFmt_FG_BAYER12:
        case eImgFmt_BAYER14:
        case eImgFmt_UFO_BAYER10:
        default:
            PIPE_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    //
    PIPE_INF("getOutPxlByteNFmt - imgFmt(%d),*pPixel_byte(%d)",imgFmt,*pPixel_byte);
    //PIPE_INF(" ");

    return 0;
}


MINT32 cvtTgFmtToHwVal( MUINT32 imgFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8: return 0;
        case eImgFmt_BAYER10:return 1;
        case eImgFmt_BAYER12:return 2;
        case eImgFmt_BAYER14:return 4;
        case eImgFmt_RGB565: return 5;
        case eImgFmt_RGB888: return 6;
        case eImgFmt_JPEG:   return 7;

        case eImgFmt_YUY2:
        case eImgFmt_UYVY:
        case eImgFmt_YVYU:
        case eImgFmt_VYUY:
             return 3;

        default:
            PIPE_ERR("cvtTgFmtToHwVal: eImgFmt:[%d] NOT Support",imgFmt);
            return -1;
    }
    return 0;
}


MINT32 cvtImgFmt2ScenarioFmt( MUINT32 imgFmt, MINT32* scenarioFmt)
{
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit
        case eImgFmt_FG_BAYER8:
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
        case eImgFmt_FG_BAYER10:
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
        case eImgFmt_FG_BAYER12:
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
        case eImgFmt_UFO_BAYER10:
            *scenarioFmt = eScenarioFmt_RAW;
            break;

        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *scenarioFmt = eScenarioFmt_YUV;
            break;

        case eImgFmt_RGB565:
        case eImgFmt_RGB888:
        case eImgFmt_JPEG:
        default:
            PIPE_ERR("cvtImgFmt2ScenarioFmt: eImgFmt:[%d]NOT Support",imgFmt);
            *scenarioFmt = eScenarioFmt_RAW;
            return -1;
    }
    return 0;
}




MINT32 getTwoPxlInfo(EPxlMode pxlMode,EImageFormat fmt, MINT32* bTwin, MINT32* two_pxl, MINT32* en_p1, MINT32* two_pxl_dmx)
{
    switch (pxlMode) {
        case ePxlMode_One_Single:
            break;
        case ePxlMode_One_Twin:
        case ePxlMode_Two_Single:
        case ePxlMode_Two_Twin:
        default:
            PIPE_ERR("Not Support pxlMode(%d)",pxlMode);
            return -1;
    }
    return 0;
}

/*******************************************************************************
*
********************************************************************************/
CamIOPipe::
CamIOPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EDrvScenario const eScenarioID,
    EScenarioFmt const eScenarioFmt,
    MINT8 const szUsrName[32]
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      m_pIspDrvShell(NULL),
      m_pass1_Raw_CQ(CAM_ISP_CQ_NONE),
      m_pass1TwinMode(0),
      mPath1(0)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    PIPE_INF(":E");
    //
    m_pass1_Raw_CQ  = 0xFFFF;

    std::strcpy((char*)m_szUsrName,(char const*)szUsrName);
    PIPE_INF("usr(%s)",m_szUsrName);
    std::memset(&this->m_camPass1Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgo.resize(1);
    this->m_vBufImg2o.resize(1);
    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();
    //
    m_hrz_update_cnt = 0;

    mMagicNum0 = 0xFFFF; //IMG2O
    mMagicNum1 = 0xFFFE; //3A
    mMagicNum2 = 0xFFFE; //IMGO
    mOpenedPort = 0x0;
    m_bBypassImgo = m_bBypassImg2o = MFALSE;
    PIPE_INF(":X");
}

CamIOPipe::
~CamIOPipe()
{
    PIPE_INF(":E");
    //
    m_pIspDrvShell->destroyInstance();
    PIPE_INF(":X");
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
init()
{
MBOOL result = MTRUE;

    PIPE_INF("(%s):E",this->m_szUsrName);

    m_CamPathPass1.ispBufCtrl.path = ISP_PASS1;


     //
    if ( m_pIspDrvShell ) {
        m_pIspDrvShell->init((char*)(this->m_szUsrName));
        m_CamPathPass1.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    }
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
uninit()
{
MINT32 ret = 0;
    PIPE_INF("(%s):E",this->m_szUsrName);
#if 0
    m_pIspDrvShell->getPhyIspDrv()->pipeCountDec();
#endif
    //
    m_pIspDrvShell->uninit((char *)this->m_szUsrName);

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
start()
{
int path  = CAM_ISP_PASS1_START;
    PIPE_INF(":E");
    if ( CAM_ISP_CQ_NONE != m_pass1_Raw_CQ ) {
        //because of whole control flow issue, EIS can't pass information at configpipe.
        //And it's no gurrentee that EIS can book notify before rrz/updatep1 , so ,need to involk EIS cfg before
        //pass1 start and flush the setting into phy addr

        if(this->m_pass1_Raw_CQ != CAM_ISP_CQ_NONE){
            if(1 != m_CamPathPass1.setP1Notify()){
            }
            //flush eis setting into phy address
            //enforce cq load into reg before VF_EN if update rrz/tuning is needed
            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
            startCQ0();//for rrz configframe before start
            usleep(2000);//wait for cmdQ flush into hw reg
            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
        }

        this->mDupCmdQMgr.CmdQMgr_start();
        path  = CAM_ISP_PASS1_START;
        m_CamPathPass1.start((void*)&path);
    }

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
startCQ0()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_INF(":E");
    //
    if ( CAM_ISP_CQ0 != m_pass1_Raw_CQ ) {
        PIPE_ERR("startCQ0 Invalid CQ val(%d)",m_pass1_Raw_CQ);
        return MTRUE;
    }

    path  = CAM_ISP_PASS1_CQ0_START;
    m_CamPathPass1.CQ = CAM_ISP_CQ0;
    m_CamPathPass1.start((void*)&path);
    usleep(2000);//wait for cmdQ flush into hw reg
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/

MBOOL
CamIOPipe::
startCQ0B()
{
    int path  = CAM_ISP_PASS1_START;

    PIPE_INF(":E");
    //
    path  = CAM_ISP_PASS1_CQ0B_START;
    m_CamPathPass1.start((void*)&path);
    //
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
stop()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_INF(":E");
    this->mDupCmdQMgr.CmdQMgr_stop();
   if ( CAM_ISP_CQ_NONE != m_pass1_Raw_CQ ) {
       path  = CAM_ISP_PASS1_START;
       m_CamPathPass1.stop((void*)&path);
   }

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo)
{
    MUINT32 dmaChannel = 0;
    PIPE_DBG("tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", gettid(), portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());
    //
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    PIPE_DBG("+ tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MBOOL bImdMode)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;
    stISP_BUF_INFO exbufInfo;
    /*PIPE_DBG("CamIOPipe::enqueOutBuf: E:tid(%d) PortID:(type, index, inout)=(%d, %d, %d)", \
        gettid(), \
        portID.type, portID.index, portID.inout);*/
    //
    if (EPortIndex_IMGO == portID.index)
        dmaChannel = ISP_DMA_IMGO;
    else if (EPortIndex_IMG2O == portID.index)
        dmaChannel = ISP_DMA_IMG2O;
    else {
        PIPE_ERR("unsupported dmao:0x%x\n",portID.index);
        return MFALSE;
    }

    //enque buffer
    bufInfo.memID       = rQBufInfo.vBufInfo[0].memID[0];
    bufInfo.size        = rQBufInfo.vBufInfo[0].u4BufSize[0];
    bufInfo.base_vAddr  = rQBufInfo.vBufInfo[0].u4BufVA[0];
    bufInfo.base_pAddr  = rQBufInfo.vBufInfo[0].u4BufPA[0];
    bufInfo.bufSecu     = rQBufInfo.vBufInfo[0].bufSecu[0];
    bufInfo.bufCohe     = rQBufInfo.vBufInfo[0].bufCohe[0];
    bufInfo.mBufIdx     = rQBufInfo.vBufInfo[0].mBufIdx;//used when replace buffer //Mark by Ryan - No use in mt6582 / enable by Floria
    bufInfo.next = (stISP_BUF_INFO*)NULL;
    //
    PIPE_DBG("PortID=%d, bufInfo,(%d),(0x%08x),(0x%08x),(0x%08x)",
            portID.index, bufInfo.memID,
            bufInfo.size,
            bufInfo.base_vAddr,
            bufInfo.base_pAddr);

    //buffer wanna exchange. to replace original buffer.
    if ( 2 <= rQBufInfo.vBufInfo.size() ) {
        //PIPE_INF("exchange 1st buf. by 2nd buf. and enque it.:size(%d)",rQBufInfo.vBufInfo.size());
        exbufInfo.memID       = rQBufInfo.vBufInfo[1].memID[0];
        exbufInfo.size        = rQBufInfo.vBufInfo[1].u4BufSize[0];
        exbufInfo.base_vAddr  = rQBufInfo.vBufInfo[1].u4BufVA[0];
        exbufInfo.base_pAddr  = rQBufInfo.vBufInfo[1].u4BufPA[0];
        exbufInfo.bufSecu     = rQBufInfo.vBufInfo[1].bufSecu[0];
        exbufInfo.bufCohe     = rQBufInfo.vBufInfo[1].bufCohe[0];
        exbufInfo.mBufIdx     = rQBufInfo.vBufInfo[1].mBufIdx;//used when replace buffer
        exbufInfo.next = (stISP_BUF_INFO*)NULL;
        //set for original buffer info.
        bufInfo.next = (stISP_BUF_INFO*)&exbufInfo;
        //
        PIPE_DBG("exbufInfo,(%d),(0x%08x),(0x%08x),(0x%08x),(%d/%d)",
                exbufInfo.memID,
                exbufInfo.size,
                exbufInfo.base_vAddr,
                exbufInfo.base_pAddr,
                exbufInfo.bufSecu,
                exbufInfo.bufCohe);
    }
    //
    if ( 0 != this->m_CamPathPass1.enqueueBuf( dmaChannel, bufInfo, rQBufInfo.vBufInfo[0].header,bImdMode) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }


    //PIPE_DBG("enqueOutBuf:X");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;

    //PIPE_DBG("E:tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);
    //
    if (EPortIndex_IMGO == portID.index) {
        dmaChannel = ISP_DMA_IMGO;
    }else if (EPortIndex_IMG2O == portID.index) {
        dmaChannel = ISP_DMA_IMG2O;
    } else {
        PIPE_ERR("unsupported dmao:0x%x\n",portID.index);
        return MFALSE;
    }

    //
    bufInfo.pBufList = &bufList;
    if ( 0 != this->m_CamPathPass1.dequeueBuf( dmaChannel,bufInfo) ) {
        PIPE_ERR("ERROR:dma[%d] dequeueBuf", dmaChannel);
        return MFALSE;
    }
    //
    /*PIPE_DBG("dma:[0x%x]/size:[0x%x]/addr:[0x%x] ",dmaChannel,
                                                bufList.size(),
                                                bufList.front().base_vAddr);*/
    //
    rQBufInfo.vBufInfo.resize(bufList.size());

    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID[0]          = bufList.front().memID;
        rQBufInfo.vBufInfo[i].u4BufSize[0]      = bufList.front().size;
        rQBufInfo.vBufInfo[i].u4BufVA[0]        = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufPA[0]        = bufList.front().base_pAddr;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec   = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us    = bufList.front().timeStampUs;
        rQBufInfo.vBufInfo[i].img_w             = bufList.front().img_w;
        rQBufInfo.vBufInfo[i].img_h             = bufList.front().img_h;
        rQBufInfo.vBufInfo[i].img_stride        = bufList.front().img_stride;
        rQBufInfo.vBufInfo[i].img_fmt           = bufList.front().img_fmt;
        rQBufInfo.vBufInfo[i].img_pxl_id        = bufList.front().img_pxl_id;
        rQBufInfo.vBufInfo[i].img_wbn           = bufList.front().img_wbn;
        rQBufInfo.vBufInfo[i].img_ob            = bufList.front().img_ob;
        rQBufInfo.vBufInfo[i].img_lsc           = bufList.front().img_lsc;
        rQBufInfo.vBufInfo[i].img_rpg           = bufList.front().img_rpg;
        rQBufInfo.vBufInfo[i].m_num_0           = bufList.front().m_num_0;
        rQBufInfo.vBufInfo[i].m_num_1           = bufList.front().m_num_0;
        rQBufInfo.vBufInfo[i].frm_cnt           = bufList.front().frm_cnt;
        rQBufInfo.vBufInfo[i].header            = bufList.front().header_info;
        rQBufInfo.vBufInfo[i].headerSize        = bufList.front().header_size;
        if(dmaChannel == ISP_DMA_IMG2O){
            //IMG2O doesn't support input crop -> crop_win = tg size
            rQBufInfo.vBufInfo[i].crop_win.p.x      = 0;
            rQBufInfo.vBufInfo[i].crop_win.p.y      = 0;
            rQBufInfo.vBufInfo[i].crop_win.s.w      = rQBufInfo.vBufInfo[i].img_w;
            rQBufInfo.vBufInfo[i].crop_win.s.h      = rQBufInfo.vBufInfo[i].img_h;
            //
            rQBufInfo.vBufInfo[i].DstSize.w         = bufList.front().hrz_info.dstW;
            rQBufInfo.vBufInfo[i].DstSize.h         = bufList.front().hrz_info.dstH;
        }
        else{
            rQBufInfo.vBufInfo[i].crop_win.p.x      = bufList.front().dma_crop.x;
            rQBufInfo.vBufInfo[i].crop_win.p.y      = bufList.front().dma_crop.y;
            rQBufInfo.vBufInfo[i].crop_win.s.w      = bufList.front().dma_crop.w;
            rQBufInfo.vBufInfo[i].crop_win.s.h      = bufList.front().dma_crop.h;
            rQBufInfo.vBufInfo[i].DstSize.w         = bufList.front().dma_crop.w;
            rQBufInfo.vBufInfo[i].DstSize.h         = bufList.front().dma_crop.h;
        }
        //
        bufList.pop_front();
        //
#if 0
        PIPE_DBG("[MyDeQ]dma(0x%x)/s(%d)/id(0x%x)/bs(0x%x)/va(0x%x)/pa(0x%x)/t(%d.%d)/img(%d,%d,%d,%d,%d,%d,%d,%d,%d)/m(%d,%d)/fc(%d)", \
                dmaChannel, \
                rQBufInfo.vBufInfo.size(), \
                rQBufInfo.vBufInfo[i].memID[0], \
                rQBufInfo.vBufInfo[i].u4BufSize[0], \
                rQBufInfo.vBufInfo[i].u4BufVA[0], \
                rQBufInfo.vBufInfo[i].u4BufPA[0], \
                rQBufInfo.vBufInfo[i].i4TimeStamp_sec, \
                rQBufInfo.vBufInfo[i].i4TimeStamp_us, \
                rQBufInfo.vBufInfo[i].img_w, \
                rQBufInfo.vBufInfo[i].img_h, \
                rQBufInfo.vBufInfo[i].img_stride, \
                rQBufInfo.vBufInfo[i].img_fmt, \
                rQBufInfo.vBufInfo[i].img_pxl_id, \
                rQBufInfo.vBufInfo[i].img_wbn, \
                rQBufInfo.vBufInfo[i].img_ob, \
                rQBufInfo.vBufInfo[i].img_lsc, \
                rQBufInfo.vBufInfo[i].img_rpg, \
                rQBufInfo.vBufInfo[i].m_num_0, \
                rQBufInfo.vBufInfo[i].m_num_1, \
                rQBufInfo.vBufInfo[i].frm_cnt);
#endif
    }

    //
    //PIPE_DBG("X ");
    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
#define BNR_LB  4224
/*******************************************************************************
* general
********************************************************************************/
int path = ISP_PASS_NONE;

int scenario = ISP_SCENARIO_MAX;
int subMode  = ISP_SUB_MODE_MAX;

int cam_out_fmt = CAM_FMT_SEL_YUV422_1P;

int two_pxl = 0;
int two_pxl_dmx = 0;

int pass1_CQ = m_pass1_Raw_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//

int pass1_twin = 0;
int pass1D_twin = 0;


/*******************************************************************************
* tg(raw)
********************************************************************************/
int idx_tgi = -1;

unsigned int tgInFmt = 0;
unsigned int tg2InFmt = 0;

int idx_imgo = -1;
int idx_img2o = -1;
int pix_id_tg = -1;
int pixel_byte_imgo = -1;
int pixel_byte_img2o = -1;

int _enable1 = CAM_CTL_EN1_CAM_EN;;
int _enable2 = 0;
int en_p1_dma = 0;


    int int_p1_en = (ISP_DRV_IRQ_INT_STATUS_TG1_ERR_ST| \
                 ISP_DRV_IRQ_INT_STATUS_DMA_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_CQ_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_AAO_ERR_ST | \
                 ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST | \
                 ISP_DRV_IRQ_INT_STATUS_VS1_ST | \
                 ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST | \
                 ISP_DRV_IRQ_INT_STATUS_AF_DON_ST);

    int ctl_mux_sel = (CAM_CTL_MUX_SEL_BIN_SEL_EN | \
                        CAM_CTL_MUX_SEL_SGG_SEL_EN | \
                        CAM_CTL_MUX_SEL_AA_SEL_EN | \
                        CAM_CTL_MUX_SEL_BIN_SEL_0 | \
                        CAM_CTL_MUX_SEL_SGG_SEL_2 | \
                        CAM_CTL_MUX_SEL_AA_SEL_1 | \
                        CAM_CTL_MUX_SEL_UNP_SEL_EN | \
                        CAM_CTL_MUX_SEL_UNP_SEL_1);

    int ctl_mux_sel2 = (CAM_CTL_MUX_SEL2_BIN_OUT_EN | \
                                CAM_CTL_MUX_SEL2_LSCI_SOF_SEL_EN | \
                                CAM_CTL_MUX_SEL2_LSCI_SOF_SEL_1 | \
                                CAM_CTL_MUX_SEL2_P1_DONE_MUX_EN);

    int ctl_sram_mux_cfg = (CAM_CTL_SRAM_MUX_CFG_SRAM_MUX_MODE_SCENARIO| \
                                CAM_CTL_SRAM_MUX_CFG_SRAM_MUX_TILE_EN | \
                                CAM_CTL_SRAM_MUX_CFG_ESFKO_SOF_SEL | \
                                CAM_CTL_SRAM_MUX_CFG_AAO_SOF_SEL_1 | \
                                CAM_CTL_SRAM_MUX_CFG_ESFKO_SOF_SEL_EN | \
                                CAM_CTL_SRAM_MUX_CFG_AAO_SOF_SEL_EN);

#if 0
int int_p1_en = CAM_CTL_INT_P1_EN_VS1_INT_EN | \
                CAM_CTL_INT_P1_EN_PASS1_DON_EN | \
                CAM_CTL_INT_P1_EN_SOF1_INT_EN | \
               CAM_CTL_INT_P1_EN_CQ0_VS_ERR_EN|\
                CAM_CTL_INT_P1_EN_AF_DON_EN;
#endif

int int_p1_en2 = 0;
int ctl_sel = 0;

PortInfo portInfo_tgi;
PortInfo portInfo_imgo;
PortInfo portInfo_img2o;

MUINT32 tgFps = 30;


PIPE_DBG("+, CQ:%d",m_pass1_Raw_CQ);

    if (0 == vOutPorts.size()) {
        PIPE_ERR("0 == vOutPorts.size()");
        return MFALSE;
    }

    if ( CAM_ISP_CQ_NONE != pass1_CQ) {
        if ( CAM_ISP_CQ0  == pass1_CQ ) { _enable2 |= CAM_CTL_EN2_CQ0_EN;}
        if ( CAM_ISP_CQ0B == pass1_CQ ) { _enable2 |= CAM_CTL_EN2_CQ0B_EN;}
        if ( CAM_ISP_CQ0C == pass1_CQ ) { _enable2 |= CAM_CTL_EN2_CQ0C_EN;}
    }

    //
    PIPE_DBG("P1 in[%d]/out[%d]", vInPorts.size(), vOutPorts.size());
    vector<PortInfo const*>::const_iterator iter;
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {
        if ( 0 == vInPorts[i] ) {PIPE_DBG(" 0 == vInPorts[i]"); continue; }

        PIPE_INF("P1 vInPorts:[%d]:fmt(0x%x),PM(%d),w/h/str(%d/%d/%d,%d,%d),type(%d),idx(%d),io(%d),fps(%d)",
                i, vInPorts[i]->eImgFmt, vInPorts[i]->ePxlMode,
                vInPorts[i]->u4ImgWidth, vInPorts[i]->u4ImgHeight,
                vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                vInPorts[i]->type, vInPorts[i]->index, vInPorts[i]->inout,vInPorts[i]->tgFps);
        //
        switch ( vInPorts[i]->index ) {
            case EPortIndex_TG1I:
                if(vInPorts[i]->u4ImgWidth > BNR_LB){
                    PIPE_ERR("over max supported size\n");
                    return MFALSE;
                }
                this->mPath1 |= path = ISP_PASS1;
                idx_tgi = i;
                pix_id_tg = vInPorts[i]->eRawPxlID;
                portInfo_tgi = (PortInfo)*vInPorts[i];
                _enable1 |= CAM_CTL_EN1_TG1_EN;
                if (0!=getTwoPxlInfo(vInPorts[i]->ePxlMode, vInPorts[i]->eImgFmt, (MINT32*) &pass1_twin, (MINT32*)&two_pxl, (MINT32*)&_enable1, (MINT32*)&two_pxl_dmx )) {
                    PIPE_ERR("ERROR:in param");
                    return MFALSE;
                }
                tgInFmt = cvtTgFmtToHwVal(vInPorts[i]->eImgFmt);


                m_P1HWRWctrl[EPortIndex_TG1I].init((ISP_DRV_CQ_ENUM)pass1_CQ, \
                                     (MUINT32*)this->m_pIspDrvShell);
                tgFps = vInPorts[i]->tgFps;
                break;
            case EPortIndex_TG2I:
            default:
                PIPE_ERR("Not supported TG(0x%x) Index ERR!!!!!!!!!!!!!!!!!!!!!!!!!!",vInPorts[i]->index);
                return MFALSE;
                break;
        }
    }

    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ ) {
        if ( 0 == vOutPorts[i] ) { continue; }
        PIPE_INF("P1 vOutPorts:[%d]:fmt(0x%x),pure(%d),w(%d),h(%d),stirde(%d,%d,%d),type(%d),idx(%d),dir(%d)", \
                 i, vOutPorts[i]->eImgFmt, vOutPorts[i]->u4PureRaw,
                 vOutPorts[i]->u4ImgWidth, vOutPorts[i]->u4ImgHeight,
                 vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                 vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                 vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                 vOutPorts[i]->type, vOutPorts[i]->index, vOutPorts[i]->inout);
        //
        if ( EPortIndex_IMGO == vOutPorts[i]->index ) {
            idx_imgo = i;
            en_p1_dma |= CAM_CTL_DMA_EN_IMGO_EN;
            mOpenedPort |= CAM_CTL_DMA_EN_IMGO_EN;
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
            _enable1 |= CAM_CTL_EN1_PAK_EN;
            getOutPxlByteNFmt(0,0,0, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&cam_out_fmt, (MINT32*)&_enable1 );

            if(1 == vOutPorts[i]->u4PureRaw){
                if(1 == vOutPorts[i]->u4PureRawPak) {
                }else{
                    PIPE_ERR("82 support no unpack dma output\n");
                }
                ctl_mux_sel2 |= CAM_CTL_MUX_SEL2_BIN_OUT_SEL_0;
            }
            else
                ctl_mux_sel2 |= CAM_CTL_MUX_SEL2_BIN_OUT_SEL_2;

            ctl_mux_sel2 |= (CAM_CTL_MUX_SEL2_IMGO_SEL_0 | CAM_CTL_MUX_SEL2_IMGO_SEL_EN);
            ctl_mux_sel2 |= CAM_CTL_MUX_SEL2_P1_DONE_MUX_0;

            ctl_sram_mux_cfg |= CAM_CTL_SRAM_MUX_CFG_IMGO_SOF_SEL;
        }
        if ( EPortIndex_IMG2O == vOutPorts[i]->index ) {
            idx_img2o = i;
            en_p1_dma |= CAM_CTL_DMA_EN_IMG2O_EN;
            mOpenedPort |= CAM_CTL_DMA_EN_IMG2O_EN;
            _enable1 |= (CAM_CTL_EN1_PAK2_EN|CAM_CTL_EN1_HRZ_EN);//HRZ is binded to img2o
            portInfo_img2o =  (PortInfo)*vOutPorts[idx_img2o];
            getOutPxlByteNFmt(0,0,0, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_img2o, (MINT32*)&cam_out_fmt, (MINT32*)&_enable1 );

            if(1 == vOutPorts[i]->u4PureRaw){
                PIPE_ERR("82 img2o support no pureraw dump\n");
                return MFALSE;
            }

            ctl_mux_sel2 |= (CAM_CTL_MUX_SEL2_IMG2O_SEL_0|CAM_CTL_MUX_SEL2_IMG2O_SEL_EN);
            ctl_mux_sel2 |= CAM_CTL_MUX_SEL2_P1_DONE_MUX_1;

            //hrz
            ctl_sram_mux_cfg |= (CAM_CTL_SRAM_MUX_CFG_SGG_SEL_1|CAM_CTL_SRAM_MUX_CFG_IMG2O_SOF_SEL_1);
        }
    }
    //check configuration validity
    //

    //
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {

        MINT32 scenarioFmt = eScenarioFmt_RAW;
        if (0 != cvtImgFmt2ScenarioFmt( (MUINT32)vInPorts[i]->eImgFmt, &scenarioFmt)) {
            PIPE_ERR("Invalid vInPorts[%d]->eImgFmt(%d) ", i, vInPorts[i]->eImgFmt);
            return MFALSE;
        }
        PIPE_INF("i(%d),scenarioFmt(%d),index(%d)",i,scenarioFmt,vInPorts[i]->index);

        switch (scenarioFmt) {
            case eScenarioFmt_RAW:
                subMode = ISP_SUB_MODE_RAW;
                int_p1_en |= ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
                break;
            case eScenarioFmt_YUV:
                subMode = ISP_SUB_MODE_YUV;
                int_p1_en |= ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
                int_p1_en &= (~(CAM_CTL_EN1_PAK_EN|CAM_CTL_EN1_PAK2_EN));
                _enable1 &= (~CAM_CTL_EN1_HRZ_EN);
                //
                ctl_mux_sel = (CAM_CTL_MUX_SEL_SGG_SEL_EN|CAM_CTL_MUX_SEL_SGG_SEL_1);
            break;
            case  eScenarioFmt_JPG:
            case  eScenarioFmt_RGB:
            default:
                PIPE_ERR("NOT Support format(0x%x)\n",scenarioFmt);
            return MFALSE;
        }
    }
    PIPE_INF("subMode(%d)",subMode);


    //
    scenario = meScenarioID;
    switch (meScenarioID) {
        case eDrvScenario_CC:
            scenario = NSImageio::NSIspio::eScenarioID_N3D_IC;
            ////
            break;
        default:
            PIPE_ERR("scenario(0x%x) r not supported\n",meScenarioID);
            return MFALSE;
    }

    //
    //
    /*-----------------------------------------------------------------------------
      m_camPass1Param
     -----------------------------------------------------------------------------*/
    //path
    m_camPass1Param.path = path;
    m_camPass1Param.bypass_ispRawPipe = (ISP_PASS1==path)?0:1;
    //scenario/sub_mode
    m_camPass1Param.sub_mode  = subMode;


    //twin mode, no twin in 82
    m_camPass1Param.isTwinMode = MFALSE;
    m_pass1TwinMode = MFALSE;


    //enable
    m_camPass1Param.en_Top.enable1  = _enable1;
    m_camPass1Param.en_Top.enable2 = _enable2;
    m_camPass1Param.en_Top.dma        = en_p1_dma;
    m_camPass1Param.ctl_int.int_en = int_p1_en;

    //fmt_sel
    m_camPass1Param.fmt_sel.reg_val = 0x00; //reset fmt_sel
    m_camPass1Param.fmt_sel.bit_field.scenario = scenario;
    m_camPass1Param.fmt_sel.bit_field.sub_mode = subMode;
    m_camPass1Param.fmt_sel.bit_field.cam_out_fmt = cam_out_fmt;
    m_camPass1Param.fmt_sel.bit_field.scenario = scenario;
    m_camPass1Param.fmt_sel.bit_field.sub_mode = subMode;

    //ctl_sel
    //WORKAROUND: to fix CQ0B/CQ0C fail issue
    m_camPass1Param.ctl_sel.reg_val = 0;
    m_camPass1Param.ctl_sel.bit_field.tdr_sel = 1;        //tdr doubble buf. preset value
    m_camPass1Param.ctl_sel.bit_field.pass2_db_en = 0;    //
    m_camPass1Param.ctl_sel.bit_field.pass1_db_en = 1;    //
    m_camPass1Param.ctl_sel.bit_field.eis_raw_sel = 1;
    //m_camPass1Param.ctl_sel.bit_field.tg_sel = MFALSE;    //
    //m_camPass1Param.ctl_sel.bit_field.CURZ_BORROW = 1;


    //mux_sel
    m_camPass1Param.ctl_mux_sel.reg_val = ctl_mux_sel;
    //mux_sel2
    m_camPass1Param.ctl_mux_sel2.reg_val = ctl_mux_sel2;
    //
    m_camPass1Param.ctl_sram_mux_cfg.reg_val = ctl_sram_mux_cfg;
    //pix_id
    m_camPass1Param.pix_id = pix_id_tg;
    //
    m_camPass1Param.CQ  = pass1_CQ;

    //cq0c trigger source
    if ( en_p1_dma & CAM_CTL_DMA_EN_IMGO_EN ) {
        m_camPass1Param.cqTrigSrc = CAM_CQ_TRIG_BY_IMGO_DONE;
    }

    if ( en_p1_dma & CAM_CTL_DMA_EN_IMG2O_EN ) {
        m_camPass1Param.cqTrigSrc = CAM_CQ_TRIG_BY_IMG2O_DONE;
    }


    //
    //source -> from TGx
    if ( -1 != idx_tgi ) {
        m_camPass1Param.tg_out_size.w = vInPorts[idx_tgi]->u4ImgWidth;  //not used for Pass1 now
        m_camPass1Param.tg_out_size.h = vInPorts[idx_tgi]->u4ImgHeight; //not used for Pass1 now
    } else {
        PIPE_ERR("-1 == idx_tgi");
        return MFALSE;
    }


    //
    if ( ISP_SUB_MODE_YUV == subMode ) {
        m_camPass1Param.bypass_ispRawPipe = 1;
        m_camPass1Param.bypass_ispRgbPipe = 1;
    }

    //
    m_camPass1Param.bypass_imgo = 1;
    m_camPass1Param.bypass_img2o = 1;
    if (-1 != idx_imgo ) {
        //PIPE_DBG("======= cfg imgo ======= ");
        m_camPass1Param.bypass_imgo = 0;
        if( (portInfo_imgo.crop1.floatX!=0) || (portInfo_imgo.crop1.floatY!=0)){
            PIPE_INF("imgo warning: support no float crop\n");
            portInfo_imgo.crop1.floatX = portInfo_imgo.crop1.floatY = 0;
        }
        if(portInfo_imgo.crop1.w != portInfo_imgo.u4ImgWidth){
            PIPE_ERR("error: imgo: crop_w != dstsize_w(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo.crop1.w,portInfo_imgo.u4ImgWidth);
            return MFALSE;
        }
        if(portInfo_imgo.crop1.h != portInfo_imgo.u4ImgHeight){
            PIPE_ERR("error: imgo: crop_h != dstsize_h(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo.crop1.h,portInfo_imgo.u4ImgHeight);
            return MFALSE;
        }
        // use output dma crop
        this->configDmaPort(&portInfo_imgo,m_camPass1Param.imgo,(MUINT32)pixel_byte_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.imgo.lIspColorfmt = portInfo_imgo.eImgFmt;
        PIPE_INF("imgo_crop [%d, %d, %d, %d]_f(0x%x,0x%x)\n", \
                        m_camPass1Param.imgo.crop.x,m_camPass1Param.imgo.crop.y, \
                        m_camPass1Param.imgo.crop.w,m_camPass1Param.imgo.crop.h, \
                        m_camPass1Param.imgo.crop.floatX,m_camPass1Param.imgo.crop.floatY);

        //for YUV sensor configFrame() setting path
        if ( ISP_SUB_MODE_YUV == this->m_camPass1Param.sub_mode) {
            m_camPass1Param.hrz_in_size         = m_camPass1Param.tg_out_size;
        }

        m_camPass1Param.imgo.tgFps = tgFps;
    }


    if (-1 != idx_img2o ) {
        //PIPE_DBG("======= cfg img2o ======= ");
        m_camPass1Param.bypass_img2o = 0;
        if( (portInfo_img2o.crop1.floatX!=0) || (portInfo_img2o.crop1.floatY!=0)){
            PIPE_INF("img2o warning: support no float crop\n");
            portInfo_img2o.crop1.floatX = portInfo_img2o.crop1.floatY = 0;
        }
        this->configDmaPort(&portInfo_img2o,m_camPass1Param.img2o,(MUINT32)pixel_byte_img2o,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);

        m_camPass1Param.img2o.lIspColorfmt = portInfo_img2o.eImgFmt;
        m_camPass1Param.hrz_in_size         = m_camPass1Param.tg_out_size;

        //img2o support no crop so far, because coordinate must based on TG output.
        //but img2o cropping function is after HRZ . so , support no cropping.
        PIPE_INF("[3ASync]img2o crop=%d,%d",m_camPass1Param.img2o.crop.w, m_camPass1Param.img2o.crop.h );
        m_camPass1Param.img2o.crop.x =0;
        m_camPass1Param.img2o.crop.y =0;

        m_camPass1Param.img2o.tgFps = tgFps;

    }

    //
    if( 0 !=  m_CamPathPass1.config( &m_camPass1Param ) )
    {
        PIPE_ERR("Pass 1 config error!");
        return MFALSE;
    }
    PIPE_DBG("-");

    return  MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    PIPE_DBG("NOT SUPPORT for camio pipe ");
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CamIOPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
int ret = 0;

    PIPE_DBG("+ tid(%d) (u4Param1, u4Param2)=(%d, %d)", gettid(), u4Param1, u4Param2);

    switch ( u4Param1 ) {
/*
        case EPIPECmd_SET_ZOOM_RATIO:
        ret = m_CamPathPass1.setZoom( u4Param2 );
        break;
*/
        default:
            PIPE_ERR("NOT support command!");
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("onSet2Params error!");
        return MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
* Command
********************************************************************************/
MBOOL
CamIOPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
    PIPE_DBG("+ tid(%d) (u4InParam)=(%d)", gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
irq(EPipePass pass,EPipeIRQ irq_int, int userEnum)
{
    return this->irq(pass,irq_int,(int)(NSIspDrv_FrmB::ISP_DRV_IRQ_CLEAR_WAIT),userEnum);
}

MBOOL
CamIOPipe::
irq(EPipePass pass,EPipeIRQ irq_int, int irqclear, int userEnum)
{
    int    ret = 0;
    MINT32 type = 0;
    MUINT32 irq = 0;

    PIPE_DBG("+ tid(%d) (pass,irq_int,clear)=(0x%08x,0x%08x,irqclear)", gettid(), pass, irq_int,irqclear);

    //pass
    if ( EPipePass_PASS1_TG1 != pass && EPipePass_PASS1_TG1_D != pass ) {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
    //irq_int
    switch(pass){
        case EPipePass_PASS1_TG1:    type = ISP_DRV_IRQ_TYPE_INT;
            break;
        default:
            PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
            return MFALSE;
            break;
    }
    if ( EPIPEIRQ_VSYNC == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:    irq = ISP_DRV_IRQ_INT_STATUS_VS1_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else if ( EPIPEIRQ_PATH_DONE == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:    irq = ISP_DRV_IRQ_INT_STATUS_PASS1_TG1_DON_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else if ( EPIPEIRQ_SOF == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:    irq = ISP_DRV_IRQ_INT_STATUS_SOF1_INT_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else {
        PIPE_ERR("IRQ:NOT SUPPORT irq");
        return    MFALSE;
    }
    //
    PIPE_DBG("(type,user,irq,clear)=(0x%08x,%d,0x%08x,%d)", type,userEnum,irq,irqclear);
    //
    ret = m_CamPathPass1.waitIrq(type,irq,irqclear,userEnum);

    if( ret != 0 )
    {
        PIPE_ERR("waitIrq error!");
        return    MFALSE;
    }
    return    MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
registerIrq(Irq_t irq)
{

#if 0
ISP_DRV_WAIT_IRQ_STRUCT drv_irq(irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);

    if ( m_pIspDrvShell ) {
        if (MFALSE == m_pIspDrvShell->getPhyIspDrv()->registerIrq(drv_irq) ) {
            PIPE_ERR("registerIrq(%d,%d,%d,%d,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
            return MFALSE;
        }
    }
#else
int    ret = 0;
    ret = m_CamPathPass1.registerIrq(irq);

    if( ret != 0 )
    {
        PIPE_ERR("registerIrq error!");
        return  MFALSE;
    }
#endif
    return MTRUE;
}
/*******************************************************************************
*
********************************************************************************/
MBOOL
CamIOPipe::
irq(Irq_t irq)
{
#if 0
ISP_DRV_WAIT_IRQ_STRUCT drv_irq(irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
    if ( m_pIspDrvShell ) {
        if (MFALSE == m_pIspDrvShell->getPhyIspDrv()->waitIrq(drv_irq) ) {
            PIPE_ERR("waitIrq(%d,%d,%d,%d,%d,)",irq.Type,irq.Status,irq.Timeout,irq.UserName,irq.irq_TStamp);
            return MFALSE;
        }
    }
#else
int    ret = 0;
    ret = m_CamPathPass1.waitIrq(irq);

    if( ret != 0 )
    {
        PIPE_ERR("waitIrq error!");
        return  MFALSE;
    }
    PIPE_ERR("waitIrq -");

#endif
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MUINT32*
CamIOPipe::getIspReg(MUINT32 path)
{
    MINT32 cq = ISP_DRV_BASIC_CQ_NUM; //tg1 CAM_ISP_CQ_NONE
    //
    if ( m_pIspDrvShell ) {


        if ( EPortIndex_TG1I == path ) {
            cq = ISP_DRV_CQ0;
        }
//        else if ( EPortIndex_TG2I == path ) {
//            cq = ISP_DRV_CQ0_D;
//        }
        else {
            PIPE_ERR("%d",path);
        }

        return this->m_pIspDrvShell->getPhyIspDrv()->getCQInstance((ISP_DRV_CQ_ENUM)cq)->getRegAddr();

    }

    return 0;
}


/*******************************************************************************
*
********************************************************************************/
#define _CmdQSwitchEn(OpenedPort,mag_img20,mag_aa,mag_imgo)({\
    MBOOL _flag = MFALSE;\
    if((OpenedPort & CAM_CTL_DMA_EN_IMG2O_EN) && (OpenedPort & CAM_CTL_DMA_EN_IMGO_EN)){\
        if(this->m_bBypassImgo == MTRUE){\
            if(mag_img20 == mag_aa)\
                _flag = MTRUE;\
        }\
        else if(this->m_bBypassImg2o == MTRUE){\
            if(mag_imgo == mag_aa)\
                _flag = MTRUE;\
        }\
        else {\
            if((mag_img20 == mag_aa) && (mag_img20 == mag_imgo))\
                _flag = MTRUE;\
        }\
    }\
    else if(OpenedPort == CAM_CTL_DMA_EN_IMG2O_EN){\
        if(mag_img20 == mag_aa)\
            _flag = MTRUE;\
    }\
    else if(OpenedPort == CAM_CTL_DMA_EN_IMGO_EN){\
        if(mag_imgo == mag_aa)\
            _flag = MTRUE;\
    }\
    _flag;\
})


MBOOL
CamIOPipe::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MUINT32 tg;
    int    ret = 0;    // 0 for ok , -1 for fail
    IspSize out_size;
    IspDMACfg out_dma;
    MUINT32 dmaChannel = 0;
    IspHrzCfg hrz_cfg;
    IspHrzCfg *pHrz_cfg = 0;
    MUINT32 magicNum = -1;
    IspP1Cfg  p1_cfg;
    MINT32 dupCqIdx = 0;

    DMACfg* pUserImg2oPara = 0;

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    switch ( cmd ) {
        //case EPIPECmd_SET_D_PATH_SCENARIO_TYPE:
        //    meScenarioFmtD = (NSImageio::NSIspio::EScenarioFmt) arg1;//CAM_ISP_CQ0
        //    break;
        case EPIPECmd_SET_EIS_CBFP:
            this->mRunTimeCfgLock.lock();
            m_CamPathPass1.P1Notify_Mapping(cmd,(P1_TUNING_NOTIFY*)arg1);
            this->mRunTimeCfgLock.unlock();
            break;
        case EPIPECmd_SET_CQ_CHANNEL:
            m_pass1_Raw_CQ = arg1;//CAM_ISP_CQ0
            m_CamPathPass1.CQ = m_pass1_Raw_CQ;
            //m_CamPathPass1.flushCqDescriptor((MUINT32) m_pass1_Raw_CQ);
            break;
        case EPIPECmd_SET_CQ_TRIGGER_MODE:
            //TO Physical Reg.
            m_CamPathPass1.setCQTriggerMode(arg1,arg2,arg3);
            m_CQ0TrigMode  = (ISP_DRV_CQ0 == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            m_CQ0BTrigMode = (ISP_DRV_CQ0B == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            break;

        case EPIPECmd_AE_SMOOTH:
        {
            //Arg1: ob_gain
            PIPE_DBG("EPIPECmd_AE_SMOOTH");
            this->m_pIspDrvShell->getPhyIspDrv()->updateCq0bRingBuf((void*)arg1);
            m_CamPathPass1.setCQTriggerMode((MINT32)CAM_ISP_CQ0B,\
                                            (MINT32)EPIPECQ_TRIGGER_SINGLE_EVENT,\
                                            (MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
            startCQ0B();
        }
            break;
        case EPIPECmd_SET_HRZ:
            //Arg1: DMACfg
            //Arg2: EPortIndex_TG1I/EPortIndex_TG2I
            //Arg3: sensorDev
            if (NULL == arg1) {
                PIPE_ERR("[3ASync] SET_IMG2O NULL param(0x%x)!",arg1);
                return MFALSE;
            }
            pUserImg2oPara = (DMACfg*)arg1;

            if (0 == pUserImg2oPara->out.stride) {
                PIPE_ERR("[3ASync] SET_IMG2O Invalid stride(0x%x)!",((DMACfg*)arg1)->out.stride);
                //workaround first
                pUserImg2oPara->out.stride = (((pUserImg2oPara->out.w*3)>>1) * 10)>> 3;
            }

            if (0 ==pUserImg2oPara->out.stride * pUserImg2oPara->crop.w * pUserImg2oPara->crop.h) {
                PIPE_ERR("[3ASync] SET_IMG2O Invalid Para(0x%x/0x%x/0x%x)!",pUserImg2oPara->out.stride, pUserImg2oPara->crop.w, pUserImg2oPara->crop.h);
                return MFALSE;
            }

            if (0 == pUserImg2oPara->out.w * pUserImg2oPara->out.w) {
                PIPE_ERR("[3ASync] SET_IMG2O Invalid OutWind Para(0x%x/0x%x)!", pUserImg2oPara->out.w, pUserImg2oPara->out.h);
                return MFALSE;
            }


            PIPE_DBG("[3ASync]SET_IMG2O path:%d, m_num_0=%x -", arg2, pUserImg2oPara->m_num);

            this->mRunTimeCfgLock.lock();
            if (0 != m_hrz_update_cnt) {
                PIPE_DBG("[3ASync][Warn]SET_IMG2O: DMACfg Enter twice-");
            }

            if ( EPortIndex_TG1I == arg2){
               if (pUserImg2oPara->crop.x > this->m_camPass1Param.hrz_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR: Crop invalid(x(%d)>size_w(%d))",pUserImg2oPara->crop.x, this->m_camPass1Param.hrz_in_size.w);
                    m_hrz_update_cnt = 0;
                    return MFALSE;
                }
                if (pUserImg2oPara->crop.w > this->m_camPass1Param.hrz_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR: Not Support Scale up(in(%d)<roi_w(%d))", pUserImg2oPara->crop.w, this->m_camPass1Param.hrz_in_size.w);
                    m_hrz_update_cnt = 0;
                    return MFALSE;
                }
                pHrz_cfg = &m_hrz_cfg;
                m_hrz_update_cnt++;
            }
            else {
                PIPE_ERR("error: unsupported TG:0x%x\n",arg2);
                return MFALSE;
            }


            pHrz_cfg->img2o_crop.x        = pUserImg2oPara->crop.x;
            pHrz_cfg->img2o_crop.y        = pUserImg2oPara->crop.y;
            pHrz_cfg->img2o_crop.floatX   = pUserImg2oPara->crop.floatX;
            pHrz_cfg->img2o_crop.floatY   = pUserImg2oPara->crop.floatY;
            pHrz_cfg->img2o_crop.w        = pUserImg2oPara->crop.w;
            pHrz_cfg->img2o_crop.h        = pUserImg2oPara->crop.h;

            pHrz_cfg->hrz_out_size.w       = pUserImg2oPara->out.w;
            pHrz_cfg->hrz_out_size.h       = pUserImg2oPara->out.h;
            //pHrz_cfg->rrz_out_size.stride  = pUserImg2oPara->out.stride;
            pHrz_cfg->m_num_0 = pUserImg2oPara->m_num; //MW M number
            //

            if ( EPortIndex_TG1I == arg2){

                pHrz_cfg->hrz_in_size.w       = this->m_camPass1Param.hrz_in_size.w;
                pHrz_cfg->hrz_in_size.h       = this->m_camPass1Param.hrz_in_size.h;
                mMagicNum0 = pHrz_cfg->m_num_0;

                //for YUV sensor, no P1 tuning
                if ( ISP_SUB_MODE_YUV == this->m_camPass1Param.sub_mode) {
                    mMagicNum1 = mMagicNum0;
                    PIPE_DBG("YUV(%d,%d)",mMagicNum0,mMagicNum0);
                }

                if (_CmdQSwitchEn(this->mOpenedPort,mMagicNum0,mMagicNum1,mMagicNum2)) {
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_hrz_cfg.isTwinMode = m_pass1TwinMode;
                    hrz_cfg = m_hrz_cfg;
                    hrz_cfg.pass1_path = ISP_PASS1;
                    m_hrz_update_cnt = 0;

                    //bypass tuningQ for YUV
                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum0, arg3) ){
                            PIPE_ERR("[3ASync]SET_RRZ setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImg2o == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_DMA_EN_IMG2O_EN){
                            if(0 != m_CamPathPass1.setP1HrzCfg(hrz_cfg)){
                                PIPE_ERR("[3ASync]SET_HRZ setP1HrzCfg Fail");
                                ret = -1;
                            }
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_DMA_EN_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]SET_HRZ setP1imgoCfg Fail");
                                ret = -1;
                            }
                        }
                    }

                    if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum0)){
                        ret = -1;
                    }

                    if(1 != m_CamPathPass1.setP1Notify()){
                        ret = -1;
                    }

                    mDupCmdQMgr.CmdQMgr_update();
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                    //startCQ0();
#else
                    if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                        //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                        startCQ0();//for rrz configframe before start
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                    }
#endif
                    if ( 0 != m_CamPathPass1.updateCQI()){
                        if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                            PIPE_ERR("[3ASync]updateCQ Fail");
                           else{
                               PIPE_ERR("[3ASync]CQ ERROR 0x%x\n",m_CamPathPass1.CQ);
                           }
                        ret = -1;
                    }

                } else {
                    PIPE_DBG("[3ASync]SET_HRZ:TG1I Magic0(0x%x)!= Magic1(0x%x)-", mMagicNum0, mMagicNum1);
                }
            }
            else {
                PIPE_ERR("error: unsupport TG:0x%x\n",arg2);
                return -1;
            }
            this->mRunTimeCfgLock.unlock();
            PIPE_DBG("[3ASync]SET_IMG2O [%d], m_num_0=%x -", m_hrz_update_cnt, pHrz_cfg->m_num_0);

            ret = 0;
            break;

        case EPIPECmd_SET_P1_UPDATE:
            //Arg1: TG path number
            //Arg2: magic number
            //Arg3: sensorDev
            //PIPE_DBG("[3ASync]P1_UPDATE+, tg=%d, magicNum=0x%x", arg1, arg2);
            if (((EPortIndex)arg1 != EPortIndex_TG1I)  && ((EPortIndex)arg1 != EPortIndex_TG2I)) {
                PIPE_ERR("[3ASync]NOT support PortIndex(0x%x)!",arg1);
                ret = -1;
                return ret;
            }
            this->mRunTimeCfgLock.lock();
            if ( EPortIndex_TG1I == arg1 ) {
                PIPE_DBG("[3ASync]P1_UPDATE+, TG1I, magicNum=0x%x", arg2);

                //update param here
                mMagicNum1 = arg2;
                if (_CmdQSwitchEn(this->mOpenedPort,mMagicNum0,mMagicNum1,mMagicNum2)){
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_hrz_cfg.isTwinMode = m_pass1TwinMode;
                    hrz_cfg = m_hrz_cfg;
                    hrz_cfg.pass1_path = ISP_PASS1;
                    m_hrz_update_cnt = 0;

                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum1, arg3) ){
                            PIPE_ERR("[3ASync]P1_UPDATE setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImg2o == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_DMA_EN_IMG2O_EN){
                            if(0 != m_CamPathPass1.setP1HrzCfg(hrz_cfg)){
                                PIPE_ERR("[3ASync]P1_UPDATE setP1HrzCfg Fail");
                                ret = -1;
                            }
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_DMA_EN_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]P1_UPDATE setP1imgoCfg Fail");
                                ret = -1;
                            }
                        }
                    }

                    if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum1)){
                        ret = -1;
                    }

                    if(1 != m_CamPathPass1.setP1Notify()){
                        ret = -1;
                    }

                    mDupCmdQMgr.CmdQMgr_update();
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                    //startCQ0();
#else
                    if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                        //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                        startCQ0();//for rrz configframe before start
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                    }
#endif
                    if ( 0 != m_CamPathPass1.updateCQI()){
                        if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                            PIPE_ERR("[3ASync]updateCQ Fail");
                           else{
                               PIPE_ERR("[3ASync]CQ ERROR 0x%x\n",m_CamPathPass1.CQ);
                           }
                        ret = -1;
                    }

                } else {
                    PIPE_DBG("[3ASync]: P1_UPDATE:TG1I Magic0(0x%x)!= Magic1(0x%x)-", mMagicNum0, mMagicNum1);
                }
            }
            else {
                PIPE_ERR("error: unsupport TG:0x%x\n",arg1);
                return -1;
            }
            //
            this->mRunTimeCfgLock.unlock();

            PIPE_DBG("P1_UPDATE -");
            break;
        case EPIPECmd_SET_IMGO:
            {
                DMACfg* _pDma = NULL;
                if (NULL == arg1) {
                    PIPE_ERR("[3ASync] SET_IMGO NULL param(0x%x)!",arg1);
                    return MFALSE;
                }
                _pDma = (DMACfg*)arg1;

                if (0 == _pDma->out.stride) {
                    PIPE_ERR("[3ASync] SET_IMGO Invalid stride(0x%x)!",_pDma->out.stride);
                }

                //if (0 ==_pDma->out.stride * _pDma->crop.w * _pDma->crop.h) {
                if (0 == _pDma->crop.w * _pDma->crop.h) {
                    PIPE_ERR("[3ASync] SET_IMGO Invalid Para(0x%x/0x%x/0x%x)!",_pDma->out.stride, _pDma->crop.w, _pDma->crop.h);
                    return MFALSE;
                }

                if (0 == _pDma->out.w * _pDma->out.h) {
                    PIPE_ERR("[3ASync] SET_IMGO Invalid OutWind Para(0x%x/0x%x)!", _pDma->out.w, _pDma->out.h);
                    return MFALSE;
                }


                PIPE_DBG("[3ASync]SET_IMGO path:%d, m_num_0=%x -", arg2, _pDma->m_num);

                if (_pDma->crop.x > this->m_camPass1Param.tg_out_size.w) {
                    PIPE_ERR("[3ASync]ERROR:TG:0x%x Crop invalid(x(%d)>size_w(%d))",arg2,_pDma->crop.x, this->m_camPass1Param.tg_out_size.w);
                    return MFALSE;
                }
                if (_pDma->crop.w > this->m_camPass1Param.tg_out_size.w) {
                    PIPE_ERR("[3ASync]ERROR:TG:0x%x Not Support Scale up(in(%d)<tg_w(%d))",arg2, _pDma->crop.w, this->m_camPass1Param.tg_out_size.w);
                    return MFALSE;
                }

                //Arg1: TG path number
                //Arg2: magic number
                //Arg3: sensorDev
                //PIPE_DBG("[3ASync]P1_UPDATE+, tg=%d, magicNum=0x%x", arg1, arg2);
                if ((EPortIndex)arg2 != EPortIndex_TG1I) {
                    PIPE_ERR("[3ASync]NOT support TGIdx(0x%x)!",arg2);
                    ret = -1;
                    return ret;
                }
                this->mRunTimeCfgLock.lock();
                memcpy((void*)&this->m_ImgoCfg,(const void*)_pDma,sizeof(DMACfg));
                if ( EPortIndex_TG1I == arg2 ) {
                    PIPE_DBG("[3ASync]SET_IMGO+, TG1I, magicNum=0x%x", this->m_ImgoCfg.m_num);

                    //update param here
                    mMagicNum2 = this->m_ImgoCfg.m_num;
                    if (_CmdQSwitchEn(this->mOpenedPort,mMagicNum0,mMagicNum1,mMagicNum2)){
                        m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                        m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();

                        if(this->mOpenedPort & CAM_CTL_DMA_EN_IMG2O_EN){
                            m_hrz_cfg.isTwinMode = m_pass1TwinMode;
                            hrz_cfg = m_hrz_cfg;
                            hrz_cfg.pass1_path = ISP_PASS1;
                            m_hrz_update_cnt = 0;
                        }

                        if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                            if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum2, arg3) ){
                                PIPE_ERR("[3ASync]SET_IMGO setP1TuneCfg Fail");
                                ret = -1;
                            }
                        }

                        if(this->m_bBypassImg2o == MFALSE){
                            if(this->mOpenedPort & CAM_CTL_DMA_EN_IMG2O_EN){
                                if ( 0 != m_CamPathPass1.setP1HrzCfg(hrz_cfg) ){
                                    PIPE_ERR("[3ASync]SET_IMGO setP1RrzCfg Fail");
                                    ret = -1;
                                }
                            }
                        }

                        if(this->m_bBypassImgo == MFALSE){
                            if(this->mOpenedPort & CAM_CTL_DMA_EN_IMGO_EN){
                                if ( 0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg) ){
                                    PIPE_ERR("[3ASync]SET_IMGO setP1ImgoCfg not ready");
                                    //ret = -1;
                                }
                            }
                        }

                        if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum2)){
                            ret = -1;
                        }

                        if(1 != m_CamPathPass1.setP1Notify()){
                            ret = -1;
                        }
                        mDupCmdQMgr.CmdQMgr_update();
                        m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                        m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                        //startCQ0();
#else
                        if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                            //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                            startCQ0();//for rrz configframe before start
                            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                        }
#endif
                        if ( 0 != m_CamPathPass1.updateCQI()){
                            if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                                PIPE_ERR("[3ASync]updateCQ Fail");
                               else{
                                   PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x5x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                               }
                            ret = -1;
                        }

                    } else {
                        PIPE_DBG("[3ASync]: SET_IMGO:TG1I Magic0(0x%x)!= Magic1(0x%x) != magic2(0x%x)", mMagicNum0, mMagicNum1,mMagicNum2);
                    }
                }
                else {
                    PIPE_ERR("error: unsupport TG:0x%x\n",arg2);
                    return -1;
                }
                //
                this->mRunTimeCfgLock.unlock();

                PIPE_DBG("SET_IMGO -");
                break;
            }
           case EPIPECmd_SET_IMGOBYPASS:
            this->m_bBypassImgo = arg1;
            PIPE_INF("BYPASS imgo when tuning sync (0x%x)\n",this->m_bBypassImgo);
            break;
        case EPIPECmd_SET_IMG2OBYPASS:
            this->m_bBypassImg2o = arg1;
            PIPE_INF("BYPASS img2o when tuning sync (0x%x)\n",this->m_bBypassImg2o);
            break;
        case EPIPECmd_UPDATE_BURST_QUEUE_NUM:
            if(this->m_pIspDrvShell->updateBurstQueueNum(arg1)!= MTRUE)
                ret = -1;
            break;
        case EPIPECmd_GET_MODULE_HANDLE: //handle-based
             //arg1: [In]   enum EModule
             //arg2: [Out] (MUINT32)Module_Handle
             //arg3: user name string
             //ObjAccessHW* HwRWCtrl::getModuleHandle(CAM_MODULE_ENUM module)
             tg = _p1GetModuleTgIdx(arg1,this->m_pass1TwinMode);
             if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))/*||
                ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))*/) {
                PIPE_ERR("GET_MODULE_HANDLE: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                return MFALSE;
             }
             if (EPortIndex_UNKNOW == tg ) {
                 CAM_LOGE("GET_MODULE_HANDLE:User(%s), tg=%d is Invalid",arg3, tg);
                 return -1;
             }

             //CAM_LOGD("GET_MODULE_HANDLE:User(%s)!, tg=%d",arg3, tg);

             *((MINT32*)arg2) = (MINT32) (m_P1HWRWctrl[tg].getModuleHandle((CAM_MODULE_ENUM)p1ConvModuleID(arg1), (char const*)arg3));
             if (NULL == *((MINT32*)arg2)) {
                PIPE_ERR("NOT Rdy || Not-supported module(0x%x ,%d ,%d)!",arg1,p1ConvModuleID(arg1));
                ret = -1;
             }
             break;
        case EPIPECmd_RELEASE_MODULE_HANDLE: //handle-based
            //arg1: [In]   Handle
            //arg2: user name string
             //tg = _p1GetModuleTgIdx(arg1);
            //CAM_LOGD("Handle(0x%x), %s!",arg1, arg2);

             if (!arg1) {
                 PIPE_ERR("RELEASE_MODULE_HANDLE: Invalid Handle(0x%x)!",arg1);
                 ret = -1;
             }else{
                 if((HWRWCTL_HANDLE2RWCTL(arg1))->releaseModuleHandle(arg1, (char const*)arg2)!= MTRUE)
                     ret = -1;
             }

             break;

        case EPIPECmd_SET_MODULE_CFG_DONE:  //handle-based
             //in order to modify fifo
             (HWRWCTL_HANDLE2RWCTL(arg1))->m_isTwinMode = this->m_pass1TwinMode;
             //have this 2 var. is for lower latent crisis of race condition
             this->mRunTimeCfgLock.lock();
             (HWRWCTL_HANDLE2RWCTL(arg1))->m_DupCqIdx = this->mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
             (HWRWCTL_HANDLE2RWCTL(arg1))->m_BurstQNum = this->mDupCmdQMgr.CmdQMgr_GetBurstQ();
             this->mRunTimeCfgLock.unlock();
             //arg1 : handle;
             if((HWRWCTL_HANDLE2RWCTL(arg1))->finishModuleCfg(arg1) != MTRUE)
                 ret = -1;
             break;

        case EPIPECmd_SET_MODULE_EN:
            //arg1 : MODULE-Name;
            //arg2 : MBOOL Enable(TRUE)/Disable(FALSE)
            tg = _p1GetModuleTgIdx(arg1,this->m_pass1TwinMode);
            if (EPortIndex_UNKNOW == tg ) {
                CAM_LOGE("SET_MODULE_EN:User(%s), tg=%d is Invalid",arg3, tg);
                return -1;
            }
            if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))/*||
                ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))*/) {
                PIPE_ERR("SET_MODULE_EN: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                return MFALSE;
            }

            if(m_P1HWRWctrl[tg].setModuleEn((CAM_MODULE_ENUM)p1ConvModuleID(arg1),(MBOOL)arg2)!= MTRUE)
                ret = -1;

            break;

        case EPIPECmd_SET_MODULE_SEL:
            //arg1 : MODULE-Name;
            //arg2 : MBOOL Select(TRUE)/DeSel(FALSE)
            tg = _p1ConvSelectIdToTg(arg1,this->m_pass1TwinMode);
            if (EPortIndex_UNKNOW == tg ) {
                CAM_LOGE("SET_MODULE_SEL:User(%s), tg=%d is Invalid",arg3, tg);
                return -1;
            }
            if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))/*||
                ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))*/) {
                PIPE_ERR("SET_MODULE_SEL: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                return MFALSE;
            }

            if(m_P1HWRWctrl[tg].setModuleSel(p1ConvModuleToSelectID(arg1), (MUINT8)arg2)!= MTRUE)
                ret = -1;
            break;

        case EPIPECmd_SET_MODULE_DBG_DUMP: //dbg only
            //arg1 : handle;
            tg = _p1GetModuleTgIdx(arg1,this->m_pass1TwinMode);

            if(m_P1HWRWctrl[tg].dbgDump((MUINT8)arg1, tg)!= MTRUE)
                ret = -1;

            break;
        case EPIPECmd_GET_CUR_SOF_IDX:
            {
                MUINT32 _flag[6] = {0};
                IspDrv* pDrv = NULL;
                _flag[3] = 1;
                if(this->mPath1 & ISP_PASS1)
                    _flag[0] = 0;
                else
                    PIPE_ERR("current path(0x%x) err",this->mPath1);
                pDrv = this->m_pIspDrvShell->getPhyIspDrv();
                pDrv->dumpDBGLog(_flag,NULL);
                *(MUINT32*)arg1 = _flag[0];
            }
            break;
        case EPIPECmd_GET_CUR_FRM_STATUS:
            {
                MUINT32 _flag[6] = {0};
                IspDrv* pDrv = NULL;
                _flag[5] = 2;
                pDrv = this->m_pIspDrvShell->getPhyIspDrv();
                if(this->mPath1 & ISP_PASS1)
                    _flag[0] = 0;
                else if(this->mPath1 & ISP_PASS1_D)
                    _flag[0] = 1;
                else if(this->mPath1 & ISP_PASS1_CAMSV)
                    _flag[0] = 2;
                else if(this->mPath1 & ISP_PASS1_CAMSV_D)
                    _flag[0] = 3;
                else
                    PIPE_ERR("current path(0x%x) err",this->mPath1);
                pDrv->dumpDBGLog(_flag,NULL);
                *(MUINT32*)arg1 = _flag[0];
            }
            break;
        case EPIPECmd_SET_LCS_CBFP:
        default:
            PIPE_ERR("NOT support command(0x%x)!",cmd);
            ret = -1;
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("sendCommand(0x%x) error!",cmd);
        return MFALSE;
    }
    return  MTRUE;
}


void CamIOPipe::DupCmdQMgr::CmdQMgr_update(void){
    mBurstQ = 0;
#if defined(_PASS1_CQ_CONTINUOUS_MODE_)
    if(m_bStart)
        mDupCmdQIdx = ((mDupCmdQIdx+1) % ISP_DRV_P1_CQ_DUPLICATION_NUM);
    else
        mDupCmdQIdx = mDupCmdQIdx;
#else
    mDupCmdQIdx = 0;
#endif
}

MUINT32 CamIOPipe::DupCmdQMgr::CmdQMgr_GetBurstQ(void){
    return mBurstQ;
}

MUINT32 CamIOPipe::DupCmdQMgr::CmdQMgr_GetDuqQIdx(void){
    return mDupCmdQIdx;
}

void CamIOPipe::DupCmdQMgr::CmdQMgr_start(void){
    m_bStart = MTRUE;
}
void CamIOPipe::DupCmdQMgr::CmdQMgr_stop(void){
    m_bStart = MFALSE;
}

MBOOL CamIOPipe::DupCmdQMgr::CmdQMgr_GetCurStatus(void){
    return m_bStart;
}

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

