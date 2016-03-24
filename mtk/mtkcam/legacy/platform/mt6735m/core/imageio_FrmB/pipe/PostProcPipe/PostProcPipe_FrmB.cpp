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
#define LOG_TAG "iio/ppp_FrmB"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PipeImp_FrmB.h"
#include "PostProcPipe_FrmB.h"
//
#include <cutils/properties.h>  // For property_get().
#include <utils/Trace.h> //for systrace
#include <mtkcam/featureio/EIS_Type.h>

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio_FrmB {
namespace NSIspio_FrmB   {
////////////////////////////////////////////////////////////////////////////////

#include "imageio_log.h"                    // Note: DBG_LOG_TAG/LEVEL will be used in header file, so header must be included after definition.

#undef   DBG_LOG_TAG                        // Decide a Log TAG for current file.
#ifndef USING_MTK_LDVT   // Not using LDVT.
#define  DBG_LOG_TAG        ""
#else
#define  DBG_LOG_TAG        LOG_TAG
#endif

//DECLARE_DBG_LOG_VARIABLE(pipe);
EXTERN_DBG_LOG_VARIABLE(pipe);

MINT32 u4PortID[DMA_PORT_TYPE_NUM][DMA_OUT_PORT_NUM] = {/*0*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_WROTO,  (MINT32)-1,  (MINT32)-1,  (MINT32)-1,  (MINT32)-1},
                                                        /*1*/{(MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_WROTO, (MINT32)-1, (MINT32)-1, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*2*/{(MINT32)EPortIndex_IMG2O, (MINT32)-1, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*3*/{(MINT32)EPortIndex_IMGO, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*4*/{(MINT32)-1, (MINT32)-1, (MINT32)-1,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1}
                                                        };

 Scen_Map_CropPathInfo_STRUCT mCropPathInfo[]
    =    {
    //eDrvScenario_CC
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[4][0])},
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[2][0])},
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[1][0])},
    //eDrvScenario_VSS
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[4][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[2][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[1][0])},
    //eDrvScenario_IP
    {eDrvScenario_IP,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[4][0])},
    {eDrvScenario_IP,              (MUINT32)(ECropGroupIndex_1),                &(u4PortID[3][0])},
};



MUINT32 ConvertPortIndexToCapInfoIdx(
        MUINT32 portIDIdx
)
{
    MUINT32 CapInfoIdx;

    switch(portIDIdx){
        case EPortIndex_IMGO:
            CapInfoIdx = 0;
            break;
        case EPortIndex_IMG2O:
            CapInfoIdx = 1;
            break;
        case EPortIndex_WDMAO:
            CapInfoIdx = 3;
            break;
        case EPortIndex_WROTO:
            CapInfoIdx = 4;
            break;
        default:
            PIPE_ERR("[Error]Not support this PortIndex(%d) mapping",portIDIdx);
            CapInfoIdx = 0;
            break;
    };
    return CapInfoIdx;
}


#define SET_TCM_SETTING \
    tcm_en = 0x80000000; /* (reg_4054[31] */    \
    tcm_en |= 0x08000000; /* (reg_4054[27] */    \
    tcm_en |= 0x00000001; /* (reg_4054[0]) */   \
    tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGI_EN)? 0x00000002:0);/*(reg_4054[1])*/     \
    tcm_en |= ((dma_en&CAM_CTL_DMA_EN_LSCI_EN)? 0x00000080:0);/*(reg_4054[7])*/     \
    tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMGO_EN)? 0x00000100:0);/*(reg_4054[8])*/     \
    tcm_en |= ((dma_en&CAM_CTL_DMA_EN_IMG2O_EN)?0x00000200:0);/*(reg_4054[9])*/     \
    tcm_en |= ((enable2&CAM_CTL_EN2_CDRZ_EN)?   0x00010000:0);/*(reg_4054[16])*/    \
    tcm_en |= ((enable1&CAM_CTL_EN1_UNP_EN)?    0x00100000:0);/*(reg_4054[20])*/    \
    tcm_en |= ((enable1&CAM_CTL_EN1_LSC_EN)?    0x00800000:0);/*(reg_4054[23])*/    \
    tcm_en |= ((isCropG2En==MTRUE)?             0x04000000:0);/*(reg_4054[26])*/    \
    tcm_en |= ((enable1&CAM_CTL_EN1_SL2_EN)?    0x40000000:0);/*(reg_4054[30])*/    \









/*******************************************************************************
*
********************************************************************************/
PostProcPipe::
PostProcPipe(
    char const*const szPipeName,
    EPipeID const ePipeID,
    EDrvScenario const eScenarioID,
    EScenarioFmt const eScenarioFmt,
    MINT8 const szUsrName[32]
)
    : PipeImp(szPipeName, ePipeID, eScenarioID, eScenarioFmt),
      m_pIspDrvShell(NULL),
      m_pipePass(EPipePass_PASS2),
      m_pass2_CQ(CAM_ISP_CQ_NONE),
      m_isImgPlaneByImgi(MFALSE)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    std::strcpy((char*)m_szUsrName,(char const*)szUsrName);
    PIPE_INF("usr(%s)",m_szUsrName);
    //memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass1Parameter));
    memset(&this->m_camPass2Param,0x00,sizeof(CamPathPass2Parameter));
    this->m_vBufImgi.resize(1);
    this->m_vBufVipi.resize(1);
    this->m_vBufVip2i.resize(1);
    this->m_vBufDispo.resize(1);
    this->m_vBufVido.resize(1);

    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();

    /* create MdpMgr object */
    if(MFALSE == m_CamPathPass2.ispMdpPipe.createMdpMgr())
    {
        PIPE_ERR("[Error]createMdpMgr fail");
    }

}

PostProcPipe::
~PostProcPipe()
{
    /*** release isp driver ***/
    m_pIspDrvShell->destroyInstance();

    /* destroy MdpMgr object */
    if(MFALSE == m_CamPathPass2.ispMdpPipe.destroyMdpMgr())
    {
        PIPE_ERR("[Error]destroyMdpMgr fail");
    }

}
/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
init()
{
    PIPE_INF("(%s):E",this->m_szUsrName);
    //
    if ( m_pIspDrvShell ) {
        //
        m_pIspDrvShell->init("PostProcPipe");
        //
        // record the p2 pipe user count and reset isp p2 if this is the last user.
        m_pIspDrvShell->getPhyIspDrv()->pipeCountInc(eIspDrvPipePath_P2);
        //
        m_CamPathPass2.ispTopCtrl.setIspDrvShell((IspDrvShell*)m_pIspDrvShell);
    }
    //
    PIPE_INF("X");

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
uninit()
{
    PIPE_INF("(%s):E",this->m_szUsrName);
    //
    // record the p2 pipe user count and reset isp p2 if this is the last user.
    m_pIspDrvShell->getPhyIspDrv()->pipeCountDec(eIspDrvPipePath_P2);
    //
    m_pIspDrvShell->uninit("PostProcPipe");
    //
    PIPE_INF(":X");
    //
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
start()
{
int path  = CAM_ISP_PASS2_START;


    PIPE_DBG(":E:pass[%d] +",this->m_pipePass);

    if ( EPipePass_PASS2 == this->m_pipePass ) {
        path  = CAM_ISP_PASS2_START;
    }
    else if ( EPipePass_PASS2B == this->m_pipePass ) {
        path  = CAM_ISP_PASS2B_START;
    }
    else if ( EPipePass_PASS2C == this->m_pipePass ) {
        path  = CAM_ISP_PASS2C_START;
    }
    //
    int ret=0;
    ret=m_CamPathPass2.start((void*)&path);
    if(ret<0)
    {
        PIPE_ERR("start ERR");
        return MFALSE;
    }
    else
    {
        return MTRUE;
    }
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
stop()
{
    PIPE_DBG(":E");

    m_CamPathPass2.stop(NULL);

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
getCropFunctionEnable(EDrvScenario drvScen, MINT32 portID, MUINT32& CropGroup)
{
    MUINT32 idx,j;
//    MUINT32 numPortId;
//    ESoftwareScenario scenTmp = swScen;
    if (eScenarioID_CONFIG_FMT <= drvScen)
    {
        PIPE_ERR("getCropFunctionEnable drv scen error:(%d)",drvScen);
        return MTRUE;
    }

    PIPE_DBG("cropPath Array Size = (%d), Scen_Map_CropPathInfo_STRUCT Size =(%d), scenTmp:(%d)", sizeof(mCropPathInfo), sizeof(Scen_Map_CropPathInfo_STRUCT), drvScen);
    for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
    {
        if (drvScen == mCropPathInfo[idx].u4DrvScenId)
        {
//            numPortId = mCropPathInfo[idx].u4NumPortId;
            CropGroup = mCropPathInfo[idx].u4CropGroup;

            for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
            {
                if (portID == mCropPathInfo[idx].u4PortID[j])
                {
                    return MTRUE;
                }
            }
        }
    }
    return MFALSE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryCropPathNum(EDrvScenario drvScen, MUINT32& pathNum)
{
    MUINT32 size = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    MUINT32 i, num;

    num = 0;
    for(i=0;i<size;i++){
        if(mCropPathInfo[i].u4DrvScenId == drvScen)
            num++;
    }
    pathNum = num;

    PIPE_DBG("drvScen(%d),pathNum(%d)", drvScen, pathNum);
    return MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
queryScenarioInfo(EDrvScenario drvScen, vector<CropPathInfo>& vCropPaths)
{
    //temp
    //ESoftwareScenario swScen=eSoftwareScenario_Main_Normal_Stream;

    MUINT32 idx,j,i;
    MUINT32 portidx = 0;
    MUINT32 u4NumGroup;
    //ESoftwareScenario scenTmp = swScen;
    if (eScenarioID_CONFIG_FMT <= drvScen)
    {
        PIPE_ERR("query scenario info drv scen error:(%d)",drvScen);
        return MTRUE;
    }

    PIPE_DBG("cropPath Array Size = (%d), Scen_Map_CropPathInfo_STRUCT Size =(%d), drvScen:(%d)", sizeof(mCropPathInfo), sizeof(Scen_Map_CropPathInfo_STRUCT), drvScen);
    for (idx = 0; idx<((sizeof(mCropPathInfo))/(sizeof(Scen_Map_CropPathInfo_STRUCT))); idx++)
    {
        if (drvScen == mCropPathInfo[idx].u4DrvScenId)
        {
            if (portidx >= vCropPaths.size())
            {
                PIPE_ERR("query scenario info portidx error:(%d)",portidx);
                return MFALSE;
            }
            if(vCropPaths[portidx].PortIdxVec.size()>0)
            {
                vCropPaths[portidx].PortIdxVec.clear();
            }
            u4NumGroup = 0;
            for(i=ECropGroupShiftIdx_1;i<ECropGroupShiftIdx_Num;i++){
               if((1<<i)&mCropPathInfo[idx].u4CropGroup)
                   u4NumGroup++;
            }
            vCropPaths[portidx].u4NumGroup = u4NumGroup;
            vCropPaths[portidx].u4CropGroup = mCropPathInfo[idx].u4CropGroup;
            //pCropPathInfo->PortIdxVec.resize(numPortId);
            for (j = 0 ; j < DMA_OUT_PORT_NUM; j++)
            {
                if(mCropPathInfo[idx].u4PortID[j])
                {
                    vCropPaths[portidx].PortIdxVec.push_back(mCropPathInfo[idx].u4PortID[j]);
                }
                else
                {
                    break;
                }
            }
            //vCropPaths.push_back(pathInfo);
            //vCropPaths.at(portidx) = pCropPathInfo;
            portidx++;
        }
    }
    return MTRUE;
}



/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
enqueInBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 const enqueCq,MINT32 const dupCqIdx)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;

    PIPE_DBG("tid(%d),enqueCq(%d),PortID:(type, index, inout)=(%d, %d, %d)", gettid(),enqueCq, portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    switch(portID.index){
        case EPortIndex_IMGI:
            dmaChannel = ISP_DMA_IMGI;
            break;
        default:
            PIPE_ERR("[Error]Not support this port index(%d)",portID.index);
            break;
    };


    //
    bufInfo.base_pAddr = rQBufInfo.vBufInfo[0].u4BufPA[0];
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA[0];
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID[0];
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize[0];
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu[0];
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe[0];
    if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel , bufInfo,enqueCq,dupCqIdx) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }
    //

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
dequeMdpFrameEnd( MINT32 const eDrvSce )
{
    MBOOL ret = MTRUE;
    //
    if ( 0 != this->m_CamPathPass2.dequeueMdpFrameEnd(eDrvSce) ) {
        PIPE_ERR("ERROR:dequeMdpFrameEnd");
        ret = MFALSE;
    }

    //
    return  ret;
}



/*******************************************************************************
*
********************************************************************************/
EIspRetStatus
PostProcPipe::
dequeInBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, EDrvScenario eDrvSce, MINT32 const dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;
    EIspRetStatus ret;

    PIPE_INF("dequeInBuf:tid(%d),dequeCq(%d),PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(),dequeCq, portID.type, portID.index, portID.inout, u4TimeoutMs);

    if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_LSCI == portID.index) {
        dmaChannel = ISP_DMA_LSCI;
    }
    bufInfo.pBufList = &bufList;
    ret = this->m_CamPathPass2.dequeueBuf( dmaChannel,bufInfo,eDrvSce,dequeCq,dequeDupCqIdx,dequeBurstQIdx);
    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }

    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID[0] = bufList.front().memID;
        rQBufInfo.vBufInfo.at(i).u4BufVA[0] = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo[i].u4BufSize[0] = bufList.front().size;
        bufList.pop_front();
    }
EXIT:
    //
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
enqueOutBuf(PortID const portID, QBufInfo const& rQBufInfo,MINT32 const enqueCq,MINT32 const dupCqIdx)
{
    MUINT32 dmaChannel = 0;
    stISP_BUF_INFO bufInfo;

    PIPE_DBG("+,tid(%d),enqueCq(%d),PortID:(type, index, inout)=(%d, %d, %d)", gettid(),enqueCq,portID.type, portID.index, portID.inout);
    PIPE_DBG("QBufInfo:(user, reserved, num)=(%x, %d, %d)", rQBufInfo.u4User, rQBufInfo.u4Reserved, rQBufInfo.vBufInfo.size());

    switch(portID.index){
        case EPortIndex_IMG2O:
            dmaChannel = ISP_DMA_IMG2O;
            break;
            case EPortIndex_WDMAO:
                dmaChannel = ISP_DMA_WDMAO;
                break;
            case EPortIndex_WROTO:
                dmaChannel = ISP_DMA_WROTO;
                break;
            case EPortIndex_IMGO:
                    dmaChannel = ISP_DMA_IMGO;
                    break;
        default:
            PIPE_ERR("[Error]Not support this enqueOutBuf index(%d)",portID.index);
            break;
    }
    //
    //bufInfo.type = (ISP_BUF_TYPE)rQBufInfo.vBufInfo[0].eBufType;
    bufInfo.base_pAddr = rQBufInfo.vBufInfo[0].u4BufPA[0];
    bufInfo.base_vAddr = rQBufInfo.vBufInfo[0].u4BufVA[0];
    bufInfo.memID = rQBufInfo.vBufInfo[0].memID[0];
    bufInfo.size = rQBufInfo.vBufInfo[0].u4BufSize[0];
    bufInfo.bufSecu = rQBufInfo.vBufInfo[0].bufSecu[0];
    bufInfo.bufCohe = rQBufInfo.vBufInfo[0].bufCohe[0];

    if ( 0 != this->m_CamPathPass2.enqueueBuf( dmaChannel, bufInfo,enqueCq,dupCqIdx) ) {
        PIPE_ERR("ERROR:enqueueBuf");
        return MFALSE;
    }

    PIPE_DBG("[%d]: (0x%08x,0x%08x,0x%08x)/(0x%08x,0x%08x,0x%08x)/(0x%08x,0x%08x,0x%08x) ",portID.index,
                                        rQBufInfo.vBufInfo[0].u4BufSize[0],
                                        rQBufInfo.vBufInfo[0].u4BufVA[0],
                                        rQBufInfo.vBufInfo[0].u4BufPA[0],
                                        rQBufInfo.vBufInfo[0].u4BufSize[1],
                                        rQBufInfo.vBufInfo[0].u4BufVA[1],
                                        rQBufInfo.vBufInfo[0].u4BufPA[1],
                                        rQBufInfo.vBufInfo[0].u4BufSize[2],
                                        rQBufInfo.vBufInfo[0].u4BufVA[2],
                                        rQBufInfo.vBufInfo[0].u4BufPA[2]);

    PIPE_DBG("-,");
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
EIspRetStatus
PostProcPipe::
dequeOutBuf(PortID const portID, QTimeStampBufInfo& rQBufInfo, EDrvScenario eDrvSce, MINT32 const dequeCq,MINT32 const dequeBurstQIdx, MINT32 const dequeDupCqIdx, MUINT32 const u4TimeoutMs /*= 0xFFFFFFFF*/)
{
    MUINT32 dmaChannel = 0;
    stISP_FILLED_BUF_LIST bufInfo;
    ISP_BUF_INFO_L  bufList;
    int idex=0;
    EIspRetStatus ret;
    //
    ISP_TRACE_CALL();

    PIPE_DBG("tid(%d) PortID:(type, index, inout, timeout)=(%d, %d, %d, %d)", gettid(), portID.type, portID.index, portID.inout, u4TimeoutMs);

    //Mark by Ryan - Some Ports are no use in mt6582
    switch(portID.index){
        case EPortIndex_IMG2O:
            dmaChannel = ISP_DMA_IMG2O;
            break;
            case EPortIndex_WDMAO:
                dmaChannel = ISP_DMA_WDMAO;
                break;
            case EPortIndex_WROTO:
                dmaChannel = ISP_DMA_WROTO;
                break;
            case EPortIndex_IMGO:
                    dmaChannel = ISP_DMA_IMGO;
                    break;
        default:
            PIPE_ERR("[Error]Not support this dequeOutBuf index(%d)",portID.index);
            break;
    }
    //
    bufInfo.pBufList = &bufList;
    ret = this->m_CamPathPass2.dequeueBuf( dmaChannel,bufInfo,eDrvSce,dequeCq,dequeDupCqIdx,dequeBurstQIdx);
    if ( ret == eIspRetStatus_Failed ) {
        PIPE_ERR("ERROR:dequeueBuf");
        goto EXIT;
    }
    //
    idex=bufInfo.pBufList->size()-1;
    //
    rQBufInfo.vBufInfo.resize(bufList.size());
    for ( MINT32 i = 0; i < (MINT32)rQBufInfo.vBufInfo.size() ; i++) {
        rQBufInfo.vBufInfo[i].memID[0] = bufList.front().memID;
        rQBufInfo.vBufInfo.at(i).u4BufVA[0] = bufList.front().base_vAddr;
        rQBufInfo.vBufInfo.at(i).u4BufPA[0] = bufList.front().base_pAddr;
        rQBufInfo.vBufInfo[i].u4BufSize[0] = bufList.front().size;
        rQBufInfo.vBufInfo[i].i4TimeStamp_sec = bufList.front().timeStampS;
        rQBufInfo.vBufInfo[i].i4TimeStamp_us = bufList.front().timeStampUs;
        bufList.pop_front();

        PIPE_DBG("i(%d),va(0x%08x/0x%08x/0x%08x),pa(0x%08x/0x%08x/0x%08x),size(0x%08x/0x%08x/0x%08x)",
            i,rQBufInfo.vBufInfo.at(i).u4BufVA[0],rQBufInfo.vBufInfo.at(i).u4BufVA[1],rQBufInfo.vBufInfo.at(i).u4BufVA[2],\
            rQBufInfo.vBufInfo.at(i).u4BufPA[0],rQBufInfo.vBufInfo.at(i).u4BufPA[1],rQBufInfo.vBufInfo.at(i).u4BufPA[2],\
            rQBufInfo.vBufInfo[i].u4BufSize[0],rQBufInfo.vBufInfo[i].u4BufSize[1],rQBufInfo.vBufInfo[i].u4BufSize[2]);
    }
    //
EXIT:
    return  ret;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipe(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts, PipePackageInfo *pPipePackageInfo)
{

    ISP_TRACE_CALL();

    //int dupCqIdx=0;  //which p2 cq group set
    if(pPipePackageInfo->dupCqIdx>=ISP_DRV_P2_CQ_DUPLICATION_NUM || pPipePackageInfo->dupCqIdx<0)
    {
        PIPE_ERR("exception cqGroupSetIdx(%d)",pPipePackageInfo->dupCqIdx);
        return MFALSE;
    }
    int ret = 0;
    int idx_imgi = -1;
    int idx_img2o = -1;
    int idx_wroto = -1;
    int idx_wdmao = -1;
    int idx_lsci = -1;
    int idx_imgo = -1;
    //
    MBOOL isSetMdpOut = MFALSE;
    MBOOL isWaitBuf = MTRUE;
    MBOOL isCropG1En = MFALSE;
    MBOOL isCropG2En = MFALSE;  // mdp_crop
    int hwScenario = ISP_HW_SCENARIO_CC;
    MBOOL isApplyTuning = MFALSE;
    EDrvScenario drvScenario = eDrvScenario_CC;
    int commonSubMode = ISP_SUB_MODE_MAX; // for hw register
    MUINT32 p2SubMode = ISP_SUB_MODE_RAW; // for tipie, p2 isp path
    //0x4028
    int cam_out_fmt = CAM_FMT_SEL_BAYER10; //only for ip raw(imgo)
    //0x4030
    int cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
    //
    int mdp_imgxo_p2_en = 0;
    int ctlSelGlobal = 0;
    int pixIdP2 = CAM_PIX_ID_B;
    int dmai_swap = 0;
    int dmai_swap_uv = 0;
    int enable1 = CAM_CTL_EN1_CAM_EN | CAM_CTL_EN1_G2G_EN;
        int enable2 = CAM_CTL_EN2_G2C_EN | CAM_CTL_EN2_C42_EN;
    int dma_en = 0;
    int cq_en_p2 = 0;
    int int_en= ISP_DRV_IRQ_INT_STATUS_PASS2_DON_ST|ISP_DRV_IRQ_INT_STATUS_CQ_DON_ST|ISP_DRV_IRQ_INT_STATUS_TPIPE_DON_ST|ISP_DRV_IRQ_INT_STATUS_CQ_ERR_ST|ISP_DRV_IRQ_INT_STATUS_DMA_ERR_ST;
    int int_dma_en=0;
    int intb_en=ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;
    int intc_en=ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST;
    int tpipe = CAM_MODE_TPIPE;
    int tcm_en = 0x00;
    int tdr_en = 0xC0000000;
    int ccl_sel = 0;
    int ccl_sel_en=0;
    int c02_sel = 0;
    int c02_sel_en=0;
    int g2g_sel = 0;
    int g2g_sel_en=0;
    int ctl_mux_sel = 0;
    int ctl_mux_sel2 = 0;
    int ctl_sram_mux_cfg = 0;
    //
    int eis_raw_sel = 0;
    //
    unsigned int imgo_mux_en=0;
    unsigned int imgo_mux=0;
    unsigned int img2o_mux_en=0;
    unsigned int img2o_mux=0;
    /////////////////////////////////////////////////////////////////////////
    bool cdrz_en=false;
    int isShareDmaCtlByTurn =1; /* 1:share DMA(imgci,lsci and lcei) ctl by turning */
    MUINT32 magicNum=0;
    int IMGI_V_FLIP_EN = 0;
    MBOOL isSl2cAheadCrz=MTRUE;
    MBOOL isLceAheadCrz=MTRUE;
    MINT32 crop1TableIdx = -1;
    MUINT32 i, tableSize, mdpCheck1, mdpCheck2;
    //
    m_camPass2Param.croppara.cdrz_enable=MFALSE;
    //
    int pass2_CQ = m_pass2_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
    //
    PortInfo portInfo_imgi;
    PortInfo portInfo_lsci;
    PortInfo portInfo_imgo;
    PortInfo portInfo_img2o;
    PortInfo portInfo_mfbo;
    PortInfo portInfo_wroto;
    PortInfo portInfo_wdmao;
    PortCap  portCapInfo;
    MUINT32  CropGroup;
    PortID portID;
    QBufInfo rQBufInfo;
    isp_reg_t *pIspPhyReg=NULL;
    //
    MUINT32 hrzCropX=0, hrzCropY=0;
    MFLOAT hrzRatio = 1.0;
    //

    ///////////////////////////////////////////////////////////////////////////////////////
    //in/out dma check
    PIPE_DBG("in[%d]/out[%d]", vInPorts.size(), vOutPorts.size());
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ )
    {
        if ( 0 == vInPorts[i] ) { continue; }
        //
        PIPE_INF("vInPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%08x,0x%08x,0x%08x),PA(0x%08x,0x%08x,0x%08x),memID(0x%x,0x%x,0x%x),magicNum(0x%x)",\
                                                        i,
                                                        vInPorts[i]->eImgFmt,
                                                        vInPorts[i]->u4ImgWidth,
                                                        vInPorts[i]->u4ImgHeight,
                                                        vInPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vInPorts[i]->type,
                                                        vInPorts[i]->index,
                                                        vInPorts[i]->inout,
                                                        vInPorts[i]->u4BufSize[0],
                                                        vInPorts[i]->u4BufSize[1],
                                                        vInPorts[i]->u4BufSize[2],
                                                        vInPorts[i]->u4BufVA[0],
                                                        vInPorts[i]->u4BufVA[1],
                                                        vInPorts[i]->u4BufVA[2],
                                                        vInPorts[i]->u4BufPA[0],
                                                        vInPorts[i]->u4BufPA[1],
                                                        vInPorts[i]->u4BufPA[2],
                                                        vInPorts[i]->memID[0],
                                                        vInPorts[i]->memID[1],
                                                        vInPorts[i]->memID[2],
                                                        vInPorts[i]->u4MagicNum);
        //
        if(i==0)
        {
            this->m_pipePass = (EPipePass)(vInPorts[i]->pipePass);
            PIPE_DBG("m_pipePass:[%d]",this->m_pipePass);
            switch (this->m_pipePass) {
                case EPipePass_PASS2:
                    pass2_CQ = CAM_ISP_CQ1;
                    cq_en_p2 = CAM_CTL_EN2_CQ1_EN;
                    break;
                case EPipePass_PASS2B:
                    pass2_CQ = CAM_ISP_CQ2;
                    cq_en_p2 = CAM_CTL_EN2_CQ2_EN;
                    break;
                case EPipePass_PASS2C:
                    pass2_CQ = CAM_ISP_CQ3;
                    cq_en_p2 = CAM_CTL_EN2_CQ3_EN;
                    break;
                default:
                    PIPE_ERR("[Error]NOT Support concurrency m_pipePass(%d)",this->m_pipePass);
                    return MFALSE;
            }
            PIPE_DBG("pass2_CQ:[%d]",pass2_CQ);
        }
        //
        switch(vInPorts[i]->index)
        {
            case EPortIndex_IMGI:
                        idx_imgi = i;
                dma_en |=  CAM_CTL_DMA_EN_IMGI_EN;
                portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
                //pixIdP2 = vInPorts[i]->eRawPxlID;
                magicNum = vInPorts[i]->u4MagicNum;
                PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)--[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),pass2_CQ(%d),dupCqIdx(%d)", \
                        portInfo_imgi.crop1.x,portInfo_imgi.crop1.floatX,portInfo_imgi.crop1.y,portInfo_imgi.crop1.floatY, \
                        portInfo_imgi.crop1.w,portInfo_imgi.crop1.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h, \
                        portInfo_imgi.crop2.x,portInfo_imgi.crop2.floatX,portInfo_imgi.crop2.y,portInfo_imgi.crop2.floatY, \
                        portInfo_imgi.crop2.w,portInfo_imgi.crop2.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h,pass2_CQ,pPipePackageInfo->dupCqIdx);
                break;
            default:
                PIPE_ERR("[Error]Not support this input port(%d),i(%d)",vInPorts[i]->index,i);
                break;
        }
        portID.index = vInPorts[i]->index;
        rQBufInfo.vBufInfo.resize(1);
        for(int k=0;k<3;k++)
        {
            rQBufInfo.vBufInfo[0].u4BufSize[k] = vInPorts[i]->u4BufSize[k]; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA[k] = vInPorts[i]->u4BufVA[k];
            rQBufInfo.vBufInfo[0].u4BufPA[k] = vInPorts[i]->u4BufPA[k];
            rQBufInfo.vBufInfo[0].memID[k] = vInPorts[i]->memID[k];
        }
        enqueInBuf(portID, rQBufInfo,pass2_CQ,pPipePackageInfo->dupCqIdx);
    }
    //
    for (MUINT32 i = 0 ; i < vOutPorts.size() ; i++ )
    {
        if ( 0 == vOutPorts[i] ) { continue; }
        //
        PIPE_INF("vOutPorts:[%d]:(0x%x),w(%d),h(%d),stride(%d,%d,%d),type(%d),idx(%d),dir(%d),Size(0x%08x,0x%08x,0x%08x),VA(0x%08x,0x%08x,0x%08x),PA(0x%08x,0x%08x,0x%08x),memID(0x%x,0x%x,0x%x)",i,\
                                                        vOutPorts[i]->eImgFmt,
                                                        vOutPorts[i]->u4ImgWidth,
                                                        vOutPorts[i]->u4ImgHeight,
                                                        vOutPorts[i]->u4Stride[ESTRIDE_1ST_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE],
                                                        vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE],
                                                        vOutPorts[i]->type,
                                                        vOutPorts[i]->index,
                                                        vOutPorts[i]->inout,
                                                        vOutPorts[i]->u4BufSize[0],
                                                        vOutPorts[i]->u4BufSize[1],
                                                        vOutPorts[i]->u4BufSize[2],
                                                        vOutPorts[i]->u4BufVA[0],
                                                        vOutPorts[i]->u4BufVA[1],
                                                        vOutPorts[i]->u4BufVA[2],
                                                        vOutPorts[i]->u4BufPA[0],
                                                        vOutPorts[i]->u4BufPA[1],
                                                        vOutPorts[i]->u4BufPA[2],
                                                        vOutPorts[i]->memID[0],
                                                        vOutPorts[i]->memID[1],
                                                        vOutPorts[i]->memID[2]);


        if ((MTRUE == getCropFunctionEnable(meScenarioID, (MINT32)vOutPorts[i]->index, CropGroup))
            && (CropGroup & ECropGroupIndex_1))
        {
            //Check the crop1 is enabled by dma.
            isCropG1En = MTRUE;
        }
        switch(vOutPorts[i]->index){
        case EPortIndex_IMG2O:
            idx_img2o = i;
            dma_en |= CAM_CTL_DMA_EN_IMG2O_EN;
            portInfo_img2o =  (PortInfo)*vOutPorts[idx_img2o];
            break;
        case EPortIndex_WROTO:
            isCropG2En = MTRUE;
            idx_wroto = i;
            enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
            dma_en |= CAM_CTL_DMA_EN_WROTO_EN;
            portInfo_wroto =  (PortInfo)*vOutPorts[idx_wroto];
            break;
        case EPortIndex_WDMAO:
            isCropG2En = MTRUE;
            idx_wdmao = i;
            enable2 |= CAM_CTL_EN2_UV_CRSA_EN;
            dma_en |= CAM_CTL_DMA_EN_WDMAO_EN;
            portInfo_wdmao =  (PortInfo)*vOutPorts[idx_wdmao];
                break;
                case EPortIndex_IMGO: // ip raw scenario
            if(meScenarioID!=eDrvScenario_IP)
            {
                PIPE_ERR("[Error]do not support DMA IMGO for this scenario(%d)",meScenarioID);
                return MFALSE;
            }
                       tpipe = CAM_MODE_FRAME;  // only support frame mode for dma imgo in ip scenario
            idx_imgo = i;
            //////////////////////////////
            //default ip-raw in 82 is raw in/yuv out (imgi in/imgo out)
            //if we want to use the raw in/raw out, should set the following setting
            // pak_en=1, imgo_mux_en=1, imgo_mux_sel=0, imgo_dma flexible setting (format_en=1, format=0)
            //enable1 |= CAM_CTL_EN1_PAK_EN;
            //////////////////////////////
            dma_en |= CAM_CTL_DMA_EN_IMGO_EN;
            dma_en |= CAM_CTL_DMA_EN_MDP_IMGXO_IMGO_EN;
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
            switch(vOutPorts[i]->eImgFmt){
                case eImgFmt_BAYER8:
                    cam_out_fmt = CAM_FMT_SEL_BAYER8;
                    break;
                case eImgFmt_BAYER10:
                    cam_out_fmt = CAM_FMT_SEL_BAYER10;
                    break;
                case eImgFmt_BAYER12:
                    cam_out_fmt = CAM_FMT_SEL_BAYER12;
                    break;
                case eImgFmt_YUY2:
                case eImgFmt_UYVY:
                case eImgFmt_YVYU:
                case eImgFmt_VYUY:
                    cam_out_fmt = CAM_FMT_SEL_YUV422_1P;
                    break;
                default:
                    PIPE_ERR("[Error]do not support this fmt(0x%x) for imgo",vOutPorts[i]->eImgFmt);
                    break;
            };
                    break;
            default:
                    PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",vOutPorts[i]->index,i);
                    break;
        };
        portID.index = vOutPorts[i]->index;
        rQBufInfo.vBufInfo.resize(1);
        for(int k=0;k<3;k++)
        {
            rQBufInfo.vBufInfo[0].u4BufSize[k] = vOutPorts[i]->u4BufSize[k]; //bytes
            rQBufInfo.vBufInfo[0].u4BufVA[k] = vOutPorts[i]->u4BufVA[k];
            rQBufInfo.vBufInfo[0].memID[k] = vOutPorts[i]->memID[k];
        }
        enqueOutBuf(portID, rQBufInfo,pass2_CQ,pPipePackageInfo->dupCqIdx);
    }

    //
    for (MUINT32 i = 0 ; i < pPipePackageInfo->vModuleParams.size() ; i++ )
    {
        switch(pPipePackageInfo->vModuleParams[i].eP2module){
            default:
                PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",vOutPorts[i]->index,i);
                break;
        };
    }


    /////////////////////////////////////////////////////////////////////////////
    //private data, may changed usage
    // check rrz info. and pixel id
    #if 0
    if((stISP_BUF_INFO*)pCookie) {
        pixIdP2=(MINT32)(((stISP_BUF_INFO*)pCookie)->img_pxl_id);
    }
    //
    // check pixel id
    if(pixIdP2!=CAM_PIX_ID_B && pixIdP2!=CAM_PIX_ID_Gb && pixIdP2!=CAM_PIX_ID_Gr && pixIdP2!=CAM_PIX_ID_R){
        PIPE_ERR("[Error]Get incorrect pixelId(%d), set it to 0",pixIdP2);
        pixIdP2 = 0;
    }

    PIPE_DBG("[rrz]dma=%d,pixIdP2(%d)",(((stISP_BUF_INFO*)pCookie)->dma),pixIdP2);
    #endif
    /////////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////////////////
    //scenario format check
    //should be before scenario parsing
    switch( portInfo_imgi.eImgFmt ) {
        case eImgFmt_BAYER8:    //= 0x00000001,   //Bayer format, 8-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER8;
            commonSubMode = ISP_SUB_MODE_RAW;
                    p2SubMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_BAYER10:    //= 0x00000002,   //Bayer format, 10-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER10;
            commonSubMode = ISP_SUB_MODE_RAW;
            p2SubMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_BAYER12:    //= 0x00000004,   //Bayer format, 12-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER12;
            commonSubMode = ISP_SUB_MODE_RAW;
            p2SubMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_YUY2:        //= 0x00001000,   //422 format, 1 plane (YUYV)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 1;
            commonSubMode = ISP_SUB_MODE_YUV;
            p2SubMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_UYVY:        //= 0x00008000,   //422 format, 1 plane (UYVY)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 0;
            commonSubMode = ISP_SUB_MODE_YUV;
            p2SubMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_YVYU:        //= 0x00002000,   //422 format, 1 plane (YVYU)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 3;
            commonSubMode = ISP_SUB_MODE_YUV;
            p2SubMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_VYUY:        //= 0x00004000,   //422 format, 1 plane (VYUY)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 2;
            commonSubMode = ISP_SUB_MODE_YUV;
            p2SubMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_RGB565:    //= 0x00080000,   //RGB 565 (16-bit), 1 plane
            commonSubMode = ISP_SUB_MODE_RGB;
            p2SubMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_RGB888:    //= 0x00100000,   //RGB 888 (24-bit), 1 plane
            dmai_swap = 0; //0:RGB,2:BGR
            commonSubMode = ISP_SUB_MODE_RGB;
            p2SubMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_ARGB8888:     //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
            dmai_swap = 1; //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
            commonSubMode = ISP_SUB_MODE_RGB;
            p2SubMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_FG_BAYER8:         //full-G
        case eImgFmt_FG_BAYER10:    //full-G
        case eImgFmt_FG_BAYER12:        //full-G
        case eImgFmt_FG_BAYER14:        //full-G
        case eImgFmt_BAYER14:        //bayer 14
        case eImgFmt_NV21:        //= 0x00000100,   //420 format, 2 plane (VU)
            case eImgFmt_NV12:        //= 0x00000040,   //420 format, 2 plane (UV)
            case eImgFmt_YV12:        //= 0x00000800,   //420 format, 3 plane (YVU)
            case eImgFmt_I420:        //= 0x00000400,   //420 format, 3 plane(YUV)
        case eImgFmt_NV21_BLK:    //= 0x00000200,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:    //= 0x00000080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_YV16:        //422 format, 3 plane
        case eImgFmt_NV16:        //422 format, 2 plane
        case eImgFmt_JPEG:        //= 0x00000010,   //JPEG format
        default:
            PIPE_ERR("[Error]NOT Support this format(0x%x)",portInfo_imgi.eImgFmt);
            return MFALSE;
    }

    switch (commonSubMode)
    {
        case ISP_SUB_MODE_RAW:
            commonSubMode = ISP_SUB_MODE_RAW;
            p2SubMode = ISP_SUB_MODE_RAW;
            enable1 |= CAM_CTL_EN1_CFA_EN|CAM_CTL_EN1_UNP_EN;
            g2g_sel_en=1;
            g2g_sel=1;
            break;
        case ISP_SUB_MODE_YUV:
            enable1 |= CAM_CTL_EN1_C24_EN;
            commonSubMode = ISP_SUB_MODE_YUV;
            p2SubMode = ISP_SUB_MODE_YUV;
            g2g_sel_en=1;
            g2g_sel=0;
            c02_sel_en=1;
            c02_sel=0;
            break;
        case ISP_SUB_MODE_RGB:
            commonSubMode = ISP_SUB_MODE_RGB;
            p2SubMode = ISP_SUB_MODE_RGB;
            ccl_sel_en=1;
            ccl_sel=3;
            break;
        default:
            PIPE_ERR("[Error]NOT Support this commonSubMode(%d)",commonSubMode);
            return MFALSE;
    }
    PIPE_DBG("meScenarioID:[%d],commonSubMode(%d),p2SubMode(%d),eImgFmt(%d),dma(0x%x)",meScenarioID,commonSubMode,p2SubMode,portInfo_imgi.eImgFmt,dma_en);


    //Check CDRZ is enable or not.
    if (MTRUE == isCropG1En)
    {
        if((idx_wdmao+idx_wroto)>-2)
        {
            PIPE_ERR("do not support img2o and mdp output simultaneously, cropG1en(%d), idx_img2o/wdma/wroto(%d/%d/%d)",isCropG1En,idx_img2o,idx_wdmao,idx_wroto);
            return MFALSE;
        }
        else
        {
            if( ( -1 != idx_img2o) &&  (ctl_mux_sel2 & 0x00200040))
            {    //img2o output after cdrz (cc and vss)
                cdrz_en=true;
            }
            else if( ( -1 != idx_imgo) &&  (ctl_mux_sel2 & 0x00100010))
            {    //imgo output after cdrz (ip-raw)
                cdrz_en=true;
            }
        }
    }
    enable2 |= ((cdrz_en)?(CAM_CTL_EN2_CDRZ_EN):(0));
    PIPE_DBG("isCropG1En(%d),cdrz_en(%d)",isCropG1En,cdrz_en);
    //mdp crop source always enqal to imgi if mdp output is enabled
    if((idx_wdmao+idx_wroto+idx_img2o+idx_imgo)>-4)
    {
        m_camPass2Param.croppara.mdp_src_fmt = portInfo_imgi.eImgFmt;
        m_camPass2Param.croppara.mdp_Yin.w = portInfo_imgi.u4ImgWidth;
        m_camPass2Param.croppara.mdp_Yin.h = portInfo_imgi.u4ImgHeight;
        m_camPass2Param.croppara.mdp_Yin.stride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
        m_camPass2Param.croppara.mdp_UVin.stride = 0;
        m_camPass2Param.croppara.mdp_src_size = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
        m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
    }


    // set en1, en2 and dma referred to tuning buffer
    if(pPipePackageInfo->isV3==true){
        // set pixid got from sensor
        pixIdP2 = pPipePackageInfo->pixIdP2;

        // set tuning data
        if(pPipePackageInfo->pTuningQue != NULL){
            MUINT32 tuningEn1Tag = 0x00;
            MUINT32 tuningEn2Tag = 0x00;
            MUINT32 tuningDmaTag = 0x00;
            //
            this->m_CamPathPass2.ispTuningCtrl.getEnTuningTag(MTRUE, ISP_DRV_CQ01, (MUINT32)drvScenario, p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
            //
            isApplyTuning = MTRUE;
            pIspPhyReg = (isp_reg_t*)pPipePackageInfo->pTuningQue;
            enable1 = (enable1 & (~tuningEn1Tag)) | (pIspPhyReg->CAM_CTL_EN1.Raw & tuningEn1Tag);
            enable2 = (enable2 & (~tuningEn2Tag)) | (pIspPhyReg->CAM_CTL_EN2.Raw & tuningEn2Tag);
            dma_en = (dma_en & (~tuningDmaTag)) | (pIspPhyReg->CAM_CTL_DMA_EN.Raw & tuningDmaTag);
            //
            if(dma_en & CAM_CTL_DMA_EN_LSCI_EN){
                int lsci_swap = 0;

                PIPE_DBG("config lsci");
                //
                lsci_swap = pIspPhyReg->CAM_LSCI_STRIDE.Bits.SWAP;
                portInfo_lsci.u4BufSize[0] = pIspPhyReg->CAM_LSCI_STRIDE.Bits.STRIDE * pIspPhyReg->CAM_LSCI_YSIZE.Bits.YSIZE;
                portInfo_lsci.u4BufPA[0] = pIspPhyReg->CAM_LSCI_BASE_ADDR.Bits.BASE_ADDR;
                portInfo_lsci.u4ImgWidth = pIspPhyReg->CAM_LSCI_XSIZE.Bits.XSIZE;
                portInfo_lsci.u4ImgHeight = pIspPhyReg->CAM_LSCI_YSIZE.Bits.YSIZE;
                portInfo_lsci.u4Stride[0] = pIspPhyReg->CAM_LSCI_STRIDE.Bits.STRIDE;
                portInfo_lsci.xoffset = 0;
                portInfo_lsci.yoffset = 0;
                //
                this->configDmaPort(&portInfo_lsci,m_camPass2Param.lsci,(MUINT32)1,(MUINT32)lsci_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
                //
                m_camPass2Param.lsci.ring_en = 0;
                m_camPass2Param.lsci.ring_size= 0;
                //
                m_camPass2Param.lsci.size.stride = pIspPhyReg->CAM_LSCI_STRIDE.Bits.STRIDE;
                PIPE_DBG("[LSCI](x,y)=(%d,%d),s(%d),pa(0x%x)",\
                    m_camPass2Param.lsci.size.w,m_camPass2Param.lsci.size.h,m_camPass2Param.lsci.size.stride,\
                    m_camPass2Param.lsci.memBuf.base_pAddr);
            }
            //
            PIPE_DBG("[V3]pIspPhyReg(0x%08x)",pIspPhyReg);
            PIPE_INF("[V3]hwScenario(%d),p2SubMode(%d),tuningEn1Tag(0x%x),tuningEn2Tag(0x%x),tuningDmaTag(0x%x)",hwScenario,p2SubMode,tuningEn1Tag,tuningEn2Tag,tuningDmaTag);
            PIPE_INF("[V3_Tuning]en1(0x%08x),en2(0x%08x),dma(0x%08x)",pIspPhyReg->CAM_CTL_EN1.Raw,pIspPhyReg->CAM_CTL_EN2.Raw,pIspPhyReg->CAM_CTL_DMA_EN.Raw);
            PIPE_DBG("[V3]en1(0x%08x),en2(0x%08x),dma(0x%08x)",enable1,enable2,dma_en);
        } else {
            isApplyTuning = MFALSE;
            PIPE_ERR("[Error]p2 tuning not be passed via imageio in V3");
        }
        //
        hrzCropX = (MUINT32)pPipePackageInfo->mP1PrivateData.ResizeCropX;
        hrzCropY = (MUINT32)pPipePackageInfo->mP1PrivateData.ResizeCropY;
        if(pPipePackageInfo->mP1PrivateData.ResizeDstH) {
            hrzRatio = (MFLOAT)(pPipePackageInfo->mP1PrivateData.ResizeDstH) / (pPipePackageInfo->mP1PrivateData.ResizeSrcH);
        } else {
            PIPE_ERR("[Error]wrong dstH(%d),hrz_info(0x%x)",pPipePackageInfo->mP1PrivateData.ResizeDstH,hrzRatio);
        }
        PIPE_INF("[hrz]crop(%d,%d),src(%d,%d),dst(%d,%d), pixIdP2(%d)", \
            pPipePackageInfo->mP1PrivateData.ResizeCropX,pPipePackageInfo->mP1PrivateData.ResizeCropY, \
            pPipePackageInfo->mP1PrivateData.ResizeSrcW,pPipePackageInfo->mP1PrivateData.ResizeSrcH, \
            pPipePackageInfo->mP1PrivateData.ResizeDstW,pPipePackageInfo->mP1PrivateData.ResizeDstH, pixIdP2);

    }


    ///////////////////////////////////////////////////////////////////////////////////////
    //set drv scenario
    drvScenario = meScenarioID;

    switch (meScenarioID) {
        case eDrvScenario_CC:        //normal path (N3D_IC)
            hwScenario = ISP_HW_SCENARIO_CC;
            drvScenario = eDrvScenario_CC;
             /*
            -VSS ISP flow is as follows, pass1 is dump before or after HRZ. PASS2 is from after HRZ.
            -Please set the following flexible setting
                -BIN_OUT_SEL_EN =1(reg_4078,[19], set reg_40c8, clr reg_40cc)
                -BIN_OUT_SEL = 2 or 3(reg_4078,[3:2], set reg_40c8, clr reg_40cc)
                    -2:before HRZ
                    -3:after  HRZ
                -PREGAIN_SEL=1 (reg_407c,[24], set reg_40d0,, clr reg_40d4)
                -SGG_HRZ_SEL=1 (reg_407c,[28], set reg_40d0,, clr reg_40d4)
                -EIS_RAW_SEL=1 (reg_4018,[16], set reg_40a0,, clr reg_40a4)
                -PASS1_DONE_MUX_EN = 1 (reg_4078,[30], set reg_40c8, clr reg_40cc)
                -PASS1_DONE_MUX = 1 (reg_4078,[12:8], set reg_40c8, clr reg_40cc)
                -BIN_EN = 0 (reg_4004,[2], set reg_4080, clr reg_4084)
            */
            eis_raw_sel = 1;        //CHRISTOPHER: for what??
            ctl_mux_sel = 0x00100008;     // 4074. AAO_SEL  //from old 82: 0x00100008
            //pass2 mux en should be high, and uv_crsa_en should be high if direct link to mdp
            ctl_mux_sel2 = 0x80280308;      // 4078. add set PASS1_DONE_MUX to 3, BIN_OUT_SEL to 8 //from old 82: 0xC0280308
            ctl_sram_mux_cfg=0x510054F7;

            //note, if pass2 output img2o, bit14 in 4078 should be high
            //should enable selection when isp only mode
            if((idx_img2o> -1 ) && ((idx_wroto+idx_wroto) == -2))
            {
                //pass2 done mux=img2o
                //img2o mux = 1 (after cdrz)
                ctl_mux_sel2 |=0x80004040;
                img2o_mux_en=1;
                img2o_mux=1;
                //disble bin_out_sel_enable
                ctl_mux_sel2 &= 0xfff7fff3;
                //
                ctl_sram_mux_cfg &= 0x1fffffff;

                PIPE_INF("TEST in isp only mode");
            }
            SET_TCM_SETTING;
            break;
        case eDrvScenario_VSS:
            hwScenario = ISP_HW_SCENARIO_CC;
            drvScenario = eDrvScenario_VSS;
            isWaitBuf=MFALSE;
            eis_raw_sel = 1;        //CHRISTOPHER: for what??
            ctl_mux_sel = 0x00000000;     // 4074. AAO_SEL  //from old 82: 0x00100008
            //pass2 mux en should be high, and uv_crsa_en should be high if direct link to mdp
            ctl_mux_sel2 = 0x80280308;      // 4078. add set PASS1_DONE_MUX to 3, BIN_OUT_SEL to 8 //from old 82: 0xC0280308
            ctl_sram_mux_cfg=0x510054F7;

            //note, if pass2 output img2o, bit14 in 4078 should be high
            //should enable selection when isp only mode
            if((idx_img2o>-1) && ((idx_wroto+idx_wroto) == -2))
            {
                //pass2 done mux=img2o
                //img2o mux = 1 (after cdrz)
                ctl_mux_sel2 |=0x00004040;
                img2o_mux_en=1;
                img2o_mux=1;
                //disble bin_out_sel_enable
                ctl_mux_sel2 &= 0xfff7fff3;
                //
                ctl_sram_mux_cfg &= 0x1fffffff;

                PIPE_INF("TEST in isp only mode");
            }
            SET_TCM_SETTING;
            break;
        case eDrvScenario_IP:
            hwScenario = ISP_HW_SCENARIO_IP;
            drvScenario = eDrvScenario_IP;
            tdr_en = 0xf; //TILE EDGE must be set to 1 in frame mode
            break;
        default:
            PIPE_ERR("NOT Support scenario(%d)",meScenarioID);
            return MFALSE;
    }




    PIPE_INF("Pass(%d),imgi.fmt(0x%x),meScenarioID(%d),isCropG1En(%d),cdrz_en(%d),mdpSrcFmt(0x%x),mdp_src(%d,%d,%d,%d),en_p2_dma(0x%08x),crop1TableIdx(%d),mdpCheck1(%d),mdpCheck2(%d),drvScenario(%d)",\
    this->m_pipePass,portInfo_imgi.eImgFmt,meScenarioID,isCropG1En,cdrz_en,m_camPass2Param.croppara.mdp_src_fmt, \
    m_camPass2Param.croppara.mdp_Yin.w,m_camPass2Param.croppara.mdp_Yin.h,m_camPass2Param.croppara.mdp_Yin.stride,m_camPass2Param.croppara.mdp_UVin.stride,\
        dma_en,crop1TableIdx,mdpCheck1,mdpCheck2,drvScenario);
    //
    m_camPass2Param.dupCqIdx = pPipePackageInfo->dupCqIdx;
    m_camPass2Param.burstQueIdx = pPipePackageInfo->burstQIdx;
    m_camPass2Param.magicNum = magicNum;
    m_camPass2Param.isWaitBuf = isWaitBuf;

    /*-----------------------------------------------------------------------------
      m_camPass2Param
      -----------------------------------------------------------------------------*/
    //
    m_camPass2Param.drvScenario = drvScenario;
    m_camPass2Param.isV3 = pPipePackageInfo->isV3;
    m_camPass2Param.pTuningIspReg = pIspPhyReg;
    m_camPass2Param.isApplyTuning = isApplyTuning;
    m_camPass2Param.p2SubMode = p2SubMode;
    m_camPass2Param.isMdpCropEn = ((isCropG2En==MTRUE)? MTRUE : MFALSE);
    //from p1 hrz info
    m_camPass2Param.hrzRatio = hrzRatio;
    m_camPass2Param.hrzCropX = hrzCropX;
    m_camPass2Param.hrzCropY = hrzCropY;
    //scenario/sub_mode
    m_camPass2Param.en_Top.enable1 = enable1;
    m_camPass2Param.en_Top.enable2 = enable2;
    m_camPass2Param.en_Top.dma = dma_en;
    //cq_enable
    m_camPass2Param.en_Top.enable2 |= cq_en_p2;
    //fmt
    m_camPass2Param.fmt_sel.bit_field.scenario=hwScenario;
    m_camPass2Param.fmt_sel.bit_field.sub_mode=commonSubMode;
    m_camPass2Param.fmt_sel.bit_field.cam_in_fmt=cam_in_fmt;
    m_camPass2Param.fmt_sel.bit_field.cam_out_fmt=cam_out_fmt;
    m_camPass2Param.pix_id = pixIdP2;
    //cam_ctl_pix_id
    m_camPass2Param.ctl_pix_id.reg_val = 0x00;
    m_camPass2Param.ctl_pix_id.bit_field.TG_PIX_ID_EN = 0;
    m_camPass2Param.ctl_pix_id.bit_field.BPC_TILE_EDGE_SEL_EN = 0;
    m_camPass2Param.ctl_pix_id.bit_field.MDP_SEL = 1;
    m_camPass2Param.ctl_pix_id.bit_field.CTL_EXTENSION_EN = 0;
    //sel
    m_camPass2Param.ctl_sel.bit_field.eis_raw_sel=eis_raw_sel;
    m_camPass2Param.ctl_sel.bit_field.pass2_db_en=0;
    m_camPass2Param.ctl_sel.bit_field.tdr_sel=1;
    m_camPass2Param.ctl_mux_sel.reg_val = ctl_mux_sel;
    m_camPass2Param.ctl_mux_sel2.reg_val = ctl_mux_sel2;
    m_camPass2Param.ctl_mux_sel.bit_field.G2G_SEL_EN = g2g_sel_en;
    m_camPass2Param.ctl_mux_sel.bit_field.G2G_SEL = g2g_sel;
    m_camPass2Param.ctl_mux_sel.bit_field.C02_SEL_EN = c02_sel_en;
    m_camPass2Param.ctl_mux_sel.bit_field.C02_SEL = c02_sel;
    m_camPass2Param.ctl_mux_sel2.bit_field.CCL_SEL_EN = ccl_sel_en;
    m_camPass2Param.ctl_mux_sel2.bit_field.CCL_SEL = ccl_sel;
    m_camPass2Param.ctl_mux_sel2.bit_field.IMGI_MUX_EN=1;        //enable dma flexible setting,choose imgi input
    m_camPass2Param.ctl_sram_mux_cfg.reg_val = ctl_sram_mux_cfg;
    //interrupt
    m_camPass2Param.ctl_int.int_en = int_en;
    m_camPass2Param.ctl_int.intb_en = intb_en;
    m_camPass2Param.ctl_int.intc_en = intc_en;
    //for tpipe
    m_camPass2Param.imgo_mux_en=imgo_mux_en;
    m_camPass2Param.imgo_mux=imgo_mux;
    m_camPass2Param.img2o_mux_en=img2o_mux_en;
    m_camPass2Param.img2o_mux=img2o_mux;

    PIPE_INF("drvScenario(%d),EN1(0x%x),EN2(0x%x),DMAEN(0x%x),FMTSEL(0x%x),CTLSEL(0x%x),MUXSEL(0x%x),MUXSEL2(0x%x),SRAM_MUX(0x%x),p2SubMode(%d)", \
            drvScenario, \
            m_camPass2Param.en_Top.enable1, \
            m_camPass2Param.en_Top.enable2, \
            m_camPass2Param.en_Top.dma, \
            m_camPass2Param.fmt_sel.reg_val, \
            m_camPass2Param.ctl_sel.reg_val, \
            m_camPass2Param.ctl_mux_sel.reg_val, \
            m_camPass2Param.ctl_mux_sel2.reg_val,\
            m_camPass2Param.ctl_sram_mux_cfg.reg_val,\
            m_camPass2Param.p2SubMode);


    m_camPass2Param.CQ = pass2_CQ;
    //m_camPass2Param.isShareDmaCtlByTurn = isShareDmaCtlByTurn;
    m_camPass2Param.tpipe = tpipe;
    m_camPass2Param.tcm_en = tcm_en; // 0x407c
    m_camPass2Param.tdr_en = tdr_en; // 0x4084


    //
    if ( -1 != idx_imgi ) {
        //PIPE_DBG("config imgi");
        this->configDmaPort(&portInfo_imgi,m_camPass2Param.imgi,(MUINT32)1,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.imgi.ring_en = 0;
        m_camPass2Param.imgi.ring_size= 0;
        //
        m_camPass2Param.imgi.memBuf_c_ofst = 0;
        m_camPass2Param.imgi.memBuf_v_ofst = 0;
        //
        m_camPass2Param.src_img_size.h = vInPorts[idx_imgi]->u4ImgHeight;
        //
        // for digital zoom crop
        m_camPass2Param.croppara.mdp_crop.x = portInfo_imgi.crop2.x;
        m_camPass2Param.croppara.mdp_crop.floatX = portInfo_imgi.crop2.floatX;
        m_camPass2Param.croppara.mdp_crop.y = portInfo_imgi.crop2.y;
        m_camPass2Param.croppara.mdp_crop.floatY = portInfo_imgi.crop2.floatY;
        m_camPass2Param.croppara.mdp_crop.w = portInfo_imgi.crop2.w;
        m_camPass2Param.croppara.mdp_crop.h = portInfo_imgi.crop2.h;


        if (MTRUE == cdrz_en)
        {
            m_camPass2Param.croppara.cdrz_enable = MTRUE;
            m_camPass2Param.croppara.cdrz_out.w = portInfo_imgi.resize1.tar_w;
            m_camPass2Param.croppara.cdrz_out.h = portInfo_imgi.resize1.tar_h;
            m_camPass2Param.croppara.cdrz_in.w = portInfo_imgi.u4ImgWidth;
            m_camPass2Param.croppara.cdrz_in.h = portInfo_imgi.u4ImgHeight;
            m_camPass2Param.croppara.cdrz_crop.x = portInfo_imgi.crop1.x;
            m_camPass2Param.croppara.cdrz_crop.floatX = portInfo_imgi.crop1.floatX;
            m_camPass2Param.croppara.cdrz_crop.y = portInfo_imgi.crop1.y;
            m_camPass2Param.croppara.cdrz_crop.floatY = portInfo_imgi.crop1.floatY;
            m_camPass2Param.croppara.cdrz_crop.w = portInfo_imgi.crop1.w;
            m_camPass2Param.croppara.cdrz_crop.h = portInfo_imgi.crop1.h;
        }
        else
        {
            m_camPass2Param.croppara.cdrz_enable = MFALSE;
            m_camPass2Param.croppara.cdrz_out.w = 0;
            m_camPass2Param.croppara.cdrz_out.h = 0;
            m_camPass2Param.croppara.cdrz_in.w = 0;
            m_camPass2Param.croppara.cdrz_in.h = 0;
            m_camPass2Param.croppara.cdrz_crop.x = 0;
            m_camPass2Param.croppara.cdrz_crop.floatX = 0;
            m_camPass2Param.croppara.cdrz_crop.y = 0;
            m_camPass2Param.croppara.cdrz_crop.floatY = 0;
            m_camPass2Param.croppara.cdrz_crop.w = 0;
            m_camPass2Param.croppara.cdrz_crop.h = 0;
        }

        //m_camPass2Param.imgi.lIspColorfmt = portInfo_imgi.eImgFmt;
        m_camPass2Param.imgi.v_flip_en = IMGI_V_FLIP_EN;
        //
    }
    //
    #if 0 // lsci set via tuning buffer queue
    if (-1 != idx_lsci ) {
        //PIPE_DBG("config lsci");
        this->configDmaPort(&portInfo_lsci,m_camPass2Param.lsci,(MUINT32)1,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.lsci.ring_en = 0;
        m_camPass2Param.lsci.ring_size= 0;
        //
        m_camPass2Param.lsci.size.stride = portInfo_lsci.u4Stride[ESTRIDE_1ST_PLANE];
    }
    #endif

    if ( -1 != idx_wroto) {
        //PIPE_DBG("config wroto");
        //
        this->configMdpOutPort(vOutPorts[idx_wroto],m_camPass2Param.wroto);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_wdmao) {
        //PIPE_DBG("config wdmao");
        //
        this->configMdpOutPort(vOutPorts[idx_wdmao],m_camPass2Param.wdmao);
        isSetMdpOut = MTRUE;
    }
    if (-1 != idx_imgo ) {    // only support frame mode for dma imgo in ip scenario
        PIPE_DBG("isSetMdpOut(%d),u4BufSize(0x%x)-u4BufVA(0x%x)-u4BufPA(0x%x)",isSetMdpOut, vOutPorts[idx_imgo]->u4BufSize[0],vOutPorts[idx_imgo]->u4BufVA[0],
                    vOutPorts[idx_imgo]->u4BufPA[0]);

        this->configDmaPort(&portInfo_imgo,m_camPass2Param.imgo,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.imgo.size.stride = portInfo_imgo.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(vOutPorts[idx_imgo],m_camPass2Param.imgxo);
            mdp_imgxo_p2_en = CAM_CTL_DMA_EN_MDP_IMGXO_IMGO_EN;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_img2o ) {
        //PIPE_DBG("config img2o isSetMdpOut(%d)",isSetMdpOut);
        PIPE_DBG("u4BufSize(0x%x)-u4BufVA(0x%x)-u4BufPA(0x%x)", vOutPorts[idx_img2o]->u4BufSize[0],vOutPorts[idx_img2o]->u4BufVA[0],
                    vOutPorts[idx_img2o]->u4BufPA[0]);

        this->configDmaPort(&portInfo_img2o,m_camPass2Param.img2o,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.img2o.size.stride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(vOutPorts[idx_img2o],m_camPass2Param.imgxo);
            mdp_imgxo_p2_en = CAM_CTL_DMA_EN_IMG2O_EN;
            isSetMdpOut = MTRUE;
        }
    }
    //
    m_camPass2Param.mdp_imgxo_p2_en = mdp_imgxo_p2_en;
    PIPE_DBG("mdp_imgxo_p2_en(0x%x)",mdp_imgxo_p2_en);
    //
    ret = m_CamPathPass2.config( &m_camPass2Param );
    if( ret != 0 )
    {
        PIPE_ERR("Pass 2 config error!");
        return MFALSE;
    }

    return    MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
configPipeUpdate(vector<PortInfo const*>const& vInPorts, vector<PortInfo const*>const& vOutPorts)
{
    PIPE_DBG("NOT SUPPORT for postproc pipe ");
    return  MTRUE;
}
/*******************************************************************************
* Command
********************************************************************************/
MBOOL
PostProcPipe::
onSet2Params(MUINT32 const u4Param1, MUINT32 const u4Param2)
{
    PIPE_DBG("tid(%d) (u4Param1, u4Param2)=(%d, %d)", gettid(), u4Param1, u4Param2);
    return  MTRUE;
}


/*******************************************************************************
* Command
********************************************************************************/
MBOOL
PostProcPipe::
onGet1ParamBasedOn1Input(MUINT32 const u4InParam, MUINT32*const pu4OutParam)
{
    PIPE_DBG("tid(%d) (u4InParam)=(%d)",gettid(), u4InParam);
    *pu4OutParam = 0x12345678;
    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
irq(EPipePass pass,EPipeIRQ irq_int, int userEnum)
{
int    ret = 0;
MINT32 type = 0;
MUINT32 irq = 0;

    PIPE_DBG("tid(%d) (type,irq)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //irq_int
    if ( EPIPEIRQ_PATH_DONE != irq_int ) {
        PIPE_ERR("IRQ:NOT SUPPORT irq for PASS2");
        return MFALSE;
    }
    //pass
    if ( EPipePass_PASS2 == pass ) {
        type = ISP_DRV_IRQ_TYPE_INT;
        irq = ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;
    }
#if 0 //_mt6593fpga_dvt_use_
    else if ( EPipePass_PASS2B == pass ) {
        type = ISP_DRV_IRQ_TYPE_INTB;
        irq = ISP_DRV_IRQ_INTB_STATUS_PASS2_DON_ST;

    }
    else if ( EPipePass_PASS2C == pass ) {
        type = ISP_DRV_IRQ_TYPE_INTC;
        //irq = ISP_DRV_IRQ_INTC_STATUS_PASS2_DON_ST;
    }
#endif
    else {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
    //
    PIPE_DBG("(type,user,irq)=(0x%08x%d,0x%08x)", type,userEnum, irq);
    //
    ret = m_CamPathPass2.waitIrq(type,userEnum,irq);
    if( ret != 0 ) {
        PIPE_ERR("waitIrq error!");
//        m_CamPathPass2.tdriPipe.runTpipeDbgLog();
        return  MFALSE;
    }

    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL
PostProcPipe::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    int    ret = 0;
    MUINT32 dmaChannel = 0;

    PIPE_DBG("tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    switch ( cmd ) {
/*
        case EPIPECmd_SET_ZOOM_RATIO:
        ret = m_CamPathPass2.setZoom( arg1 );
        break;
*/
#if 0
        case EPIPECmd_SET_CURRENT_BUFFER:
            //
            if ( EPortIndex_IMGI == arg1 ) { dmaChannel = ISP_DMA_IMGI; }
            if ( EPortIndex_VIPI == arg1 ) { dmaChannel = ISP_DMA_VIPI; }
            if ( EPortIndex_VIP2I == arg1 ) { dmaChannel = ISP_DMA_VIP2I; }
            if ( EPortIndex_VIP3I == arg1 ) { dmaChannel = ISP_DMA_VIP3I; }
            if ( EPortIndex_IMG2O == arg1 ) { dmaChannel = ISP_DMA_IMG2O; }
            if ( EPortIndex_IMG3O == arg1 ) { dmaChannel = ISP_DMA_IMG3O; }
            if ( EPortIndex_IMG3BO == arg1 ) { dmaChannel = ISP_DMA_IMG3BO; }
            if ( EPortIndex_IMG3CO == arg1 ) { dmaChannel = ISP_DMA_IMG3CO; }
            //
            m_CamPathPass2.setDMACurrBuf((MUINT32) dmaChannel);
            break;
#endif
#if 0
        case EPIPECmd_SET_NEXT_BUFFER:
            if ( EPortIndex_IMGI == arg1 ) {
                dmaChannel = ISP_DMA_IMGI;
            }
            if ( EPortIndex_VIPI == arg1 ) {
                dmaChannel = ISP_DMA_VIPI;
            }
            if ( EPortIndex_VIP2I == arg1 ) {
                dmaChannel = ISP_DMA_VIP2I;
            }
            if ( EPortIndex_DISPO == arg1 ) {
                dmaChannel = ISP_DMA_IMG3O;
            }
            if ( EPortIndex_VIDO == arg1 ) {
                dmaChannel = ISP_DMA_IMG3BO;
            }
            //
            m_CamPathPass2.setDMANextBuf((MUINT32) dmaChannel);
            break;
#endif
        case EPIPECmd_SET_CQ_CHANNEL:
            //m_pass2_CQ = arg1;//CAM_ISP_CQ0
            //m_CamPathPass2.CQ = m_pass2_CQ;
            m_CamPathPass2.flushCqDescriptor((MUINT32) arg1,(MUINT32) arg2,(MUINT32) arg3);
            break;
#if 0
        case EPIPECmd_FREE_MAPPED_BUFFER:
            {
                stISP_BUF_INFO buf_info = (stISP_BUF_INFO)(*(stISP_BUF_INFO*)arg2);
                m_CamPathPass2.freePhyBuf(arg1,buf_info);
            }
            break;
#endif
        case EPIPECmd_SET_IMG_PLANE_BY_IMGI:
            m_isImgPlaneByImgi = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_JPEG_CFG:
        case EPIPECmd_SET_NR3D_EN:
         case EPIPECmd_SET_CRZ_EN:
         case EPIPECmd_SET_NR3D_DMA_SEL:
        default:
            PIPE_ERR("NOT support command (0x%x)!",cmd);
            return MFALSE;
    }

    if( ret != 0 )
    {
        PIPE_ERR("sendCommand error!");
        return MFALSE;
    }
    return  MTRUE;
}


/*******************************************************************************
*
********************************************************************************/
MBOOL PostProcPipe::endequeFrameCtrl(EPIPE_BUFQUECmd cmd,MUINT32 callerID,MINT32 p2burstQIdx,MINT32 p2dupCQIdx,MINT32 timeout)
{
    MBOOL ret=MFALSE;
    if(m_pIspDrvShell->getPhyIspDrv() == NULL)
    {
        PIPE_ERR("NULL pointer");
        return ret;
    }
    switch(cmd)
    {
        case EPIPE_BUFQUECmd_ENQUE_FRAME:
            ret=m_pIspDrvShell->getPhyIspDrv()->enqueP2Frame(callerID,p2burstQIdx,p2dupCQIdx);
            break;
        case EPIPE_BUFQUECmd_WAIT_DEQUE:
            ret=m_pIspDrvShell->getPhyIspDrv()->waitP2Deque();
            break;
        case EPIPE_BUFQUECmd_DEQUE_SUCCESS:
            ret=m_pIspDrvShell->getPhyIspDrv()->dequeP2FrameSuccess(callerID,p2dupCQIdx);
            break;
        case EPIPE_BUFQUECmd_DEQUE_FAIL:
            ret=m_pIspDrvShell->getPhyIspDrv()->dequeP2FrameFail(callerID,p2dupCQIdx);
            break;
        case EPIPE_BUFQUECmd_WAIT_FRAME:
            ret=m_pIspDrvShell->getPhyIspDrv()->waitP2Frame(callerID,p2dupCQIdx,timeout);
            break;
        case EPIPE_BUFQUECmd_WAKE_WAITFRAME:
            ret=m_pIspDrvShell->getPhyIspDrv()->wakeP2WaitedFrames();
            break;
        case EPIPE_BUFQUECmd_CLAER_ALL:
            ret=m_pIspDrvShell->getPhyIspDrv()->freeAllP2Frames();
            break;
        default:
            break;
    }
    PIPE_DBG("cmd(%d),ret(%d)",cmd,ret);
    return ret;
}

/*Mark by Ryan - No use in mt6582
MBOOL PostProcPipe::startVideoRecord(MINT32 wd,MINT32 ht, MINT32 fps,EDrvScenario drvScen,MUINT32 cqIdx)
{
    bool ret=false;
    ret=m_CamPathPass2.ispMdpPipe.startVideoRecord(wd,ht,fps,drvScen,cqIdx);
    if(ret==MFALSE)
    {
        PIPE_ERR("[Error]startVideoRecord fail");
    }
    return ret;
}

MBOOL PostProcPipe::
stopVideoRecord(EDrvScenario drvScen,MUINT32 cqIdx)
{
    bool ret=false;
    ret=m_CamPathPass2.ispMdpPipe.stopVideoRecord(drvScen,cqIdx);
    if(ret==MFALSE)
    {
        PIPE_ERR("[Error]stopVideoRecord fail");
    }
    return ret;
}
*/
////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

