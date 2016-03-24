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
#define LOG_TAG "iio/ppp"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include "PipeImp.h"
#include "PostProcPipe.h"
//
#include <cutils/properties.h>  // For property_get().
#include <utils/Trace.h> //for systrace
#include <mtkcam/featureio/eis_type.h>

/*******************************************************************************
*
********************************************************************************/

#define FORCE_EN_DIFFERENR_VIEW_TEST 0

namespace NSImageio {
namespace NSIspio   {
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

MINT32 u4PortID[DMA_PORT_TYPE_NUM][DMA_OUT_PORT_NUM] = {/*0*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_JPEGO, (MINT32)EPortIndex_FEO, (MINT32)EPortIndex_VIPI},
                                                        /*1*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WDMAO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*2*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_WROTO, (MINT32)EPortIndex_VIPI, (MINT32)-1,  (MINT32)-1,             (MINT32)-1},
                                                        /*3*/{(MINT32)EPortIndex_WDMAO, (MINT32)-1, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*4*/{(MINT32)EPortIndex_WROTO, (MINT32)-1, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*5*/{(MINT32)EPortIndex_IMG2O, (MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*6*/{(MINT32)EPortIndex_IMG2O, (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*7*/{(MINT32)EPortIndex_IMG3O, (MINT32)EPortIndex_VIPI,  (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        /*8*/{(MINT32)EPortIndex_FEO,   (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,               (MINT32)-1,             (MINT32)-1},
                                                        };

 Scen_Map_CropPathInfo_STRUCT mCropPathInfo[]
    =    {
    //eDrvScenario_CC
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_CC,              (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_CC_SShot
    {eDrvScenario_CC_SShot,        (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_CC_SShot,        (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    {eDrvScenario_CC_SShot,        (MUINT32)(ECropGroupIndex_3),                   &(u4PortID[4][0])},
    //eDrvScenario_VSS
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[6][0])},
    {eDrvScenario_VSS,             (MUINT32)(ECropGroupIndex_2),                   &(u4PortID[3][0])},
    //eDrvScenario_VSS_MFB_Blending
    {eDrvScenario_VSS_MFB_Blending,(MUINT32)(ECropGroupIndex_1),                   &(u4PortID[4][0])},
    //eDrvScenario_CC_MFB_Blending
    {eDrvScenario_CC_MFB_Blending, (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    //eDrvScenario_CC_MFB_Mixing
    {eDrvScenario_CC_MFB_Mixing,   (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[5][0])},
    {eDrvScenario_CC_MFB_Mixing,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    //eDrvScenario_VSS_MFB_Mixing
    {eDrvScenario_VSS_MFB_Mixing,  (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[4][0])},
    {eDrvScenario_VSS_MFB_Mixing,  (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    //eDrvScenario_CC_vFB_Normal (disable img2o)
    {eDrvScenario_CC_vFB_Normal,   (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {eDrvScenario_CC_vFB_Normal,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {eDrvScenario_CC_vFB_Normal,   (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_CC_vFB_FB
    {eDrvScenario_CC_vFB_FB,       (MUINT32)(ECropGroupIndex_1),                   &(u4PortID[7][0])},
    {eDrvScenario_CC_vFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_2), &(u4PortID[3][0])},
    {eDrvScenario_CC_vFB_FB,       (MUINT32)(ECropGroupIndex_1|ECropGroupIndex_3), &(u4PortID[4][0])},
    //eDrvScenario_IP
    {eDrvScenario_IP,              (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},
    //eDrvScenario_CC_RAW
    {eDrvScenario_CC_RAW,          (MUINT32)(ECropGroupIndex_NONE),                &(u4PortID[7][0])},

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
        case EPortIndex_IMG3O:
        case EPortIndex_IMG3BO:
        case EPortIndex_IMG3CO:
            CapInfoIdx = 2;
            break;
        case EPortIndex_WDMAO:
            CapInfoIdx = 3;
            break;
        case EPortIndex_WROTO:
            CapInfoIdx = 4;
            break;
        case EPortIndex_JPEGO:
            CapInfoIdx = 5;
            break;
        default:
            PIPE_ERR("[Error]Not support this PortIndex(%d) mapping",portIDIdx);
            CapInfoIdx = 0;
            break;
    };
    return CapInfoIdx;

}


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
      m_isImgPlaneByImgi(MFALSE),
      m_Nr3dEn(MFALSE),
      m_Nr3dDmaSel(MFALSE),
      m_CrzEn(MFALSE),
      jpg_WorkingMem_memId(-1),
      jpg_WorkingMem_Size(0),
      pJpgConfigVa(NULL)
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


#if 1
    jpg_WorkingMem_Size=0;
#else//move to config to judge user use jpego or not
    //Jpeg Parameter Initialization (default)
    //jpg_WorkingMem_Size = 320*240*8; //would get the size from sendCommand before init when normal capture pipe
    IMEM_BUF_INFO jpg_buf_info;
    jpg_buf_info.size = jpg_WorkingMem_Size;
    if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&jpg_buf_info) ) {
        PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
        return MFALSE;
    }
    jpg_WorkingMem_memId = jpg_buf_info.memID;

    m_camPass2Param.jpgpara.memID = jpg_buf_info.memID;
    m_camPass2Param.jpgpara.workingMemBuf.base_vAddr = jpg_buf_info.virtAddr;
    m_camPass2Param.jpgpara.workingMemBuf.base_pAddr = jpg_buf_info.phyAddr;
    m_camPass2Param.jpgpara.workingMemBuf.size = jpg_WorkingMem_Size;
    m_camPass2Param.jpgpara.soi_en = MTRUE;
    m_camPass2Param.jpgpara.fQuality = 90;
    pJpgConfigVa = (MUINT32*)jpg_buf_info.virtAddr;
#endif

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
    IMEM_BUF_INFO buf_info;

    //Free JPG Working memory size.
    buf_info.size = jpg_WorkingMem_Size;
    buf_info.memID = jpg_WorkingMem_memId;
    buf_info.virtAddr = (MUINTPTR)pJpgConfigVa;
    if ( m_pIspDrvShell->m_pIMemDrv->freeVirtBuf(&buf_info) ) {
        PIPE_ERR("ERROR:m_pIMemDrv->freeVirtBuf");
        return MFALSE;
    }
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
getCrzPosition(
    EDrvScenario drvScen,
    MBOOL &isSl2cAheadCrz,
    MBOOL &isLceAheadCrz
)
{
    MBOOL ret = MTRUE;

    switch(drvScen){
        case eDrvScenario_CC_CShot:
        case eDrvScenario_CC_MFB_Blending:
        case eDrvScenario_VSS_MFB_Blending:
        case eDrvScenario_IP:
        case eDrvScenario_VSS_MFB_Mixing:
        case eDrvScenario_CC_MFB_Mixing:
        case eDrvScenario_CC:
        case eDrvScenario_VSS:
        case eDrvScenario_CC_SShot:
        case eDrvScenario_CC_RAW:
        case eScenarioID_CONFIG_FMT:
            isSl2cAheadCrz = MTRUE;
            isLceAheadCrz = MTRUE;
            break;
        case eDrvScenario_CC_vFB_Normal:
        case eDrvScenario_CC_vFB_FB:
            isSl2cAheadCrz = MFALSE;
            isLceAheadCrz = MFALSE;
            break;
        default:
            PIPE_ERR("[Error]Not support this drvScen(%d)",drvScen);
            ret = MFALSE;
            break;
    };
    PIPE_DBG("drvScen(%d),isSl2cAheadCrz(%d),isLceAheadCrz(%d)",drvScen,isSl2cAheadCrz,isLceAheadCrz);

    return ret;
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
        case EPortIndex_UFDI:
            dmaChannel = ISP_DMA_UFDI;
            break;
        case EPortIndex_VIPI:
            dmaChannel = ISP_DMA_VIPI;
            break;
        case EPortIndex_VIP2I:
            dmaChannel = ISP_DMA_VIP2I;
            break;
        case EPortIndex_VIP3I:
            dmaChannel = ISP_DMA_VIP3I;
            break;
        case EPortIndex_LCEI:
            dmaChannel = ISP_DMA_LCEI;
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
    //
    if (EPortIndex_IMGI == portID.index) {
        dmaChannel = ISP_DMA_IMGI;
    }
    else if (EPortIndex_VIPI == portID.index) {
        dmaChannel = ISP_DMA_VIPI;
    }
    else if (EPortIndex_VIP2I == portID.index) {
        dmaChannel = ISP_DMA_VIP2I;
    }
    else if (EPortIndex_VIP3I == portID.index) {
        dmaChannel = ISP_DMA_VIP3I;
    }
    else if (EPortIndex_LSCI == portID.index) {
        dmaChannel = ISP_DMA_LSCI;
    }
    else if (EPortIndex_LCEI == portID.index) {
        dmaChannel = ISP_DMA_LCEI;
    }
    else if (EPortIndex_UFDI == portID.index) {
        dmaChannel = ISP_DMA_UFDI;
    }
    //
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
        case EPortIndex_MFBO:
            dmaChannel = ISP_DMA_MFBO;
            break;
        case EPortIndex_IMG2O:
            dmaChannel = ISP_DMA_IMG2O;
            break;
        case EPortIndex_IMG3O:
            dmaChannel = ISP_DMA_IMG3O;
            break;
        case EPortIndex_IMG3BO:
            dmaChannel = ISP_DMA_IMG3BO;
            break;
        case EPortIndex_IMG3CO:
            dmaChannel = ISP_DMA_IMG3CO;
            break;
        case EPortIndex_WDMAO:
            dmaChannel = ISP_DMA_WDMAO;
            break;
        case EPortIndex_WROTO:
            dmaChannel = ISP_DMA_WROTO;
            break;
        case EPortIndex_JPEGO:
            dmaChannel = ISP_DMA_JPEGO;
            break;
        case EPortIndex_IMGO:
            dmaChannel = ISP_DMA_IMGO;
            break;
        case EPortIndex_IMGO_D:
            dmaChannel = ISP_DMA_IMGO_D;
            break;
        case EPortIndex_FEO:
            dmaChannel = ISP_DMA_FEO;
            break;
        case EPortIndex_VENC_STREAMO:
            dmaChannel = ISP_DMA_VENC_STREAMO;
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
    //
    switch(portID.index){
        case EPortIndex_MFBO:
            dmaChannel = ISP_DMA_MFBO;
            break;
        case EPortIndex_IMG2O:
            dmaChannel = ISP_DMA_IMG2O;
            break;
        case EPortIndex_IMG3O:
            dmaChannel = ISP_DMA_IMG3O;
            break;
        case EPortIndex_IMG3BO:
            dmaChannel = ISP_DMA_IMG3BO;
            break;
        case EPortIndex_IMG3CO:
            dmaChannel = ISP_DMA_IMG3CO;
            break;
        case EPortIndex_WDMAO:
            dmaChannel = ISP_DMA_WDMAO;
            break;
        case EPortIndex_WROTO:
            dmaChannel = ISP_DMA_WROTO;
            break;
          case EPortIndex_JPEGO:
            dmaChannel = ISP_DMA_JPEGO;
            break;
        case EPortIndex_IMGO:
            dmaChannel = ISP_DMA_IMGO;
            break;
        case EPortIndex_IMGO_D:
            dmaChannel = ISP_DMA_IMGO_D;
            break;
        case EPortIndex_FEO:
            dmaChannel = ISP_DMA_FEO;
            break;
        case EPortIndex_VENC_STREAMO:
            dmaChannel = ISP_DMA_VENC_STREAMO;
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
    if(portID.index==EPortIndex_JPEGO)
    {
        if((int)rQBufInfo.vBufInfo.size() >0 )
        {
            rQBufInfo.vBufInfo[0].jpg_size=bufInfo.pBufList->back().jpg_size;
            PIPE_DBG("idx(%d),jpg size(0x%x)",idex,rQBufInfo.vBufInfo[0].jpg_size);
        }
        else
        {
            PIPE_ERR("plz allocate buffer to store jpeg real size after deque");
        }
    }
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
     MBOOL isApplyTuning = MFALSE;
    CAM_REG_CTL_SEL_P2 *pP2Sel;
    int idx_imgi = -1;
    int idx_vipi = -1;
    int idx_vip2i = -1;
    int idx_vip3i = -1;
    int idx_img2o = -1;
    int idx_img3o = -1;
    int idx_img3bo = -1;
    int idx_img3co = -1;
    int idx_wroto = -1;
    int idx_wdmao = -1;
    int idx_jpego = -1;
    int idx_lsci = -1;
    int idx_lcei = -1;
    int idx_feo = -1;
    int idx_ufdi=-1;
    int idx_imgo = -1;
    int idx_mfbo=-1;
    int idx_venco = -1;
    //
    MBOOL isSetMdpOut = MFALSE;
    MBOOL isWaitBuf = MTRUE;
    MBOOL isCropG1En = MFALSE;
    int hwScenario = ISP_HW_SCENARIO_CC;
    EDrvScenario drvScenario = eDrvScenario_CC;
    int subMode = ISP_SUB_MODE_MAX;
    MUINT32 p2SubMode = ISP_SUB_MODE_RAW; // for tipie, p2 isp path
    //0x4028
    int cam_out_fmt = CAM_FMT_SEL_BAYER10; //only for ip raw(imgo)
    //0x4030
    int cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
    int img3o_fmt = CAM_IMG3O_FMT_YUV420_2P;
    int vipi_fmt = CAM_VIPI_FMT_YUV420_2P;
    int ufdi_fmt = CAM_UFDI_FMT_UFO_LENGTH;
    int fg_mode = CAM_FG_MODE_DISABLE;
    //
    int mdp_imgxo_p2_en = 0;
    int ctlSelGlobal = 0;
    int pixIdP2 = CAM_PIX_ID_B;
    int dmai_swap = 0;
    int dmai_swap_uv = 0;
    int en_p2 = CAM_CTL_EN_P2_G2G_EN | CAM_CTL_EN_P2_G2C_EN | CAM_CTL_EN_P2_C42_EN;
    int en_p1 = 0;
    int en_p1_dma = 0;
    int en_p2_dma = 0;
    int cq_en_p2 = 0;
    //int int_p2_en = CAM_CTL_INT_P2_STATUS_PASS2_DON_ST|CAM_CTL_INT_P2_STATUS_CQ_DON_ST|CAM_CTL_INT_P2_STATUS_TILE_DON_ST;
    int int_p2_en = 0;
    int tpipe = CAM_MODE_TPIPE;
    int tcm_en = 0x00;              //tcm_en set to tpipe
    MUINT32 tcm_en_collect=0x0; //collect information for tcm_en
    int tdr_en = 0xC0000000;
    int mfb_mode = 0;
    //
    int eis_raw_sel = 0;
    //
    int ccl_sel = 0;
    int ccl_sel_en=0;
    int g2g_sel = 0;
    int g2g_sel_en=0;
    int ctl_sel = 0;
    int ufd_sel = 0;
    int srz1_sel = 1;
    /////////////////////////////////////////////////////////////////////////
    int isShareDmaCtlByTurn =1; /* 1:share DMA(imgci,lsci and lcei) ctl by turning */
    MUINT32 magicNum=0;
    int IMGI_V_FLIP_EN = 0;
    MUINT32 rrzCropX=0, rrzCropY=0;
    MFLOAT rrzRatio = 1.0;
    MBOOL isSl2cAheadCrz=MTRUE;
    MBOOL isLceAheadCrz=MTRUE;
    MUINT32 img3o_uv_h_ratio = 2; // for img3bo & img3co
    MUINT32 img3o_uv_v_ratio = 2; // for img3bo & img3co
    MBOOL   img3o_uv_swap = MFALSE;
    MBOOL   vipi_uv_swap = MFALSE;
    MUINT32 vipi_uv_h_ratio = 2; // for vip2i & vip3i
    MUINT32 vipi_uv_v_ratio = 2; // for vip2i & vip3i
    MINT32 crop1TableIdx = -1;
    MUINT32 i, tableSize, mdpCheck1, mdpCheck2,mdpCheck3;
    //
    m_CrzEn=MFALSE; //initialize
    m_camPass2Param.croppara.crz_enable=MFALSE;
    //

#if 0 // no use CQ
    int pass2_CQ = CAM_ISP_CQ_NONE;//
    int pass2_cq_en = 0;//
#else   //use CQ1
    int pass2_CQ = m_pass2_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
//    int pass2_cq_en = 0;// = (CAM_ISP_CQ0 == pass1_CQ)?CAM_CTL_EN2_CQ0_EN:((CAM_ISP_CQ0B == pass1_CQ)?CAM_CTL_EN2_CQ0B_EN:);//0; //
//    if ( CAM_ISP_CQ_NONE != pass2_CQ) {
//        if ( CAM_ISP_CQ1 == pass2_CQ )  { pass2_cq_en = CAM_CTL_CQ_EN_P2_CQ1_EN;}
//        if ( CAM_ISP_CQ2 == pass2_CQ )  { pass2_cq_en = CAM_CTL_CQ_EN_P2_CQ2_EN;}
//        if ( CAM_ISP_CQ3 == pass2_CQ )  { pass2_cq_en = CAM_CTL_CQ_EN_P2_CQ3_EN;}
//    }
#endif
    //
    PortInfo portInfo_imgi;
    PortInfo portInfo_vipi;
    PortInfo portInfo_vip2i;
    PortInfo portInfo_vip3i;
    PortInfo portInfo_lsci;
    PortInfo portInfo_lcei;
    PortInfo portInfo_ufdi;
    PortInfo portInfo_imgo;
    PortInfo portInfo_img2o;
    PortInfo portInfo_img3o;
    PortInfo portInfo_img3bo;
    PortInfo portInfo_img3co;
    PortInfo portInfo_feo;
    PortInfo portInfo_mfbo;
    PortInfo portInfo_wroto;
    PortInfo portInfo_wdmao;
    PortInfo portInfo_jpego;
    PortInfo portInfo_venco;
    PortCap  portCapInfo;
    MUINT32  CropGroup;
    PortID portID;
    QBufInfo rQBufInfo;
    isp_reg_t *pIspPhyReg=NULL;

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
                            cq_en_p2 = CAM_CTL_CQ_EN_P2_CQ1_EN;
                            //int_p2_en |= CAM_CTL_INT_P2_STATUS_PASS2A_DON_ST;
                            break;
                        case EPipePass_PASS2B:
                            pass2_CQ = CAM_ISP_CQ2;
                            cq_en_p2 = CAM_CTL_CQ_EN_P2_CQ2_EN;
                            //int_p2_en |= CAM_CTL_INT_P2_STATUS_PASS2B_DON_ST;
                            break;
                        case EPipePass_PASS2C:
                            pass2_CQ = CAM_ISP_CQ3;
                            cq_en_p2 = CAM_CTL_CQ_EN_P2_CQ3_EN;
                            //int_p2_en |= CAM_CTL_INT_P2_STATUS_PASS2C_DON_ST;
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
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_IMGI_EN;
                portInfo_imgi =  (PortInfo)*vInPorts[idx_imgi];
                //pixIdP2 = vInPorts[i]->eRawPxlID;
                magicNum = vInPorts[i]->u4MagicNum;

                // input port limitation check for FullG format
                if ((vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER8) || (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER10) ||
                        (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER12) || (vInPorts[i]->eImgFmt == eImgFmt_FG_BAYER14)){
                    //TODO: size must be align 4 for FUllG format
                }

                PIPE_INF("[Imgi][crop_1](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_2](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d)-[crop_3](x,f_x,y,f_y)=(%d,%d,%d,%d),(w,h,tarW,tarH)=(%d,%d,%d,%d),pass2_CQ(%d),dupCqIdx(%d)", \
                        portInfo_imgi.crop1.x,portInfo_imgi.crop1.floatX,portInfo_imgi.crop1.y,portInfo_imgi.crop1.floatY, \
                        portInfo_imgi.crop1.w,portInfo_imgi.crop1.h,portInfo_imgi.resize1.tar_w,portInfo_imgi.resize1.tar_h, \
                        portInfo_imgi.crop2.x,portInfo_imgi.crop2.floatX,portInfo_imgi.crop2.y,portInfo_imgi.crop2.floatY, \
                        portInfo_imgi.crop2.w,portInfo_imgi.crop2.h,portInfo_imgi.resize2.tar_w,portInfo_imgi.resize2.tar_h,
                        portInfo_imgi.crop3.x,portInfo_imgi.crop3.floatX,portInfo_imgi.crop3.y,portInfo_imgi.crop3.floatY, \
                        portInfo_imgi.crop3.w,portInfo_imgi.crop3.h,portInfo_imgi.resize3.tar_w,portInfo_imgi.resize3.tar_h,pass2_CQ,pPipePackageInfo->dupCqIdx);

                break;
            case EPortIndex_VIPI:    //may multi-plane, vipi~vip3i
                idx_vipi = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_VIPI_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_VIPI_TCM_EN;
                portInfo_vipi =  (PortInfo)*vInPorts[idx_vipi];
                if(vInPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                    && vInPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE] && vInPorts[i]->memID[ESTRIDE_2ND_PLANE]) { // check vip2i
                    //
                    if(vInPorts[i]->eImgFmt==eImgFmt_YV16 || vInPorts[i]->eImgFmt==eImgFmt_NV16){
                        vipi_uv_v_ratio = 1;
                    } else if(vInPorts[i]->eImgFmt==eImgFmt_YV12) {
                        vipi_uv_swap = MTRUE;
                    }
                idx_vip2i = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_VIP2I_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_VIP2I_TCM_EN;
                portInfo_vip2i =  (PortInfo)*vInPorts[idx_vip2i];

                    if(vInPorts[i]->u4Stride[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufSize[ESTRIDE_3RD_PLANE] && vInPorts[i]->u4BufVA[ESTRIDE_3RD_PLANE]
                        && vInPorts[i]->u4BufPA[ESTRIDE_3RD_PLANE] && vInPorts[i]->memID[ESTRIDE_3RD_PLANE]) { // check vip3i
                idx_vip3i = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_VIP3I_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_VIP3I_TCM_EN;
                portInfo_vip3i =  (PortInfo)*vInPorts[idx_vip3i];
                    }
                }
                break;
            case EPortIndex_VIP3I: //only vip3i input(weighting table), there are only vipi+vip3i without vip2i for mfb mixing path
                idx_vip3i = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_VIP3I_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_VIP3I_TCM_EN;
                portInfo_vip3i =  (PortInfo)*vInPorts[idx_vip3i];
                break;
            case EPortIndex_LCEI:
                idx_lcei = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_LCEI_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_LCEI_TCM_EN;
                portInfo_lcei =  (PortInfo)*vInPorts[idx_lcei];
                #if 0
                //should disable lce when p2b path
                if(meScenarioID!=eDrvScenario_CC_vFB_FB)
                {
                    PIPE_ERR("[Error] Only vfb-p2b support lcei from imageio");
                }
                #endif
                break;
            case EPortIndex_UFDI:
                idx_ufdi = i;
                en_p2_dma |=  CAM_CTL_EN_P2_DMA_UFDI_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_UFDI_TCM_EN;
                portInfo_ufdi =  (PortInfo)*vInPorts[idx_ufdi];
                if((meScenarioID!=eDrvScenario_CC_vFB_FB)&&(meScenarioID!=eDrvScenario_CC_MFB_Mixing)&&(meScenarioID!=eDrvScenario_VSS_MFB_Mixing))
                {
                en_p2 |= CAM_CTL_EN_P2_UFD_EN;
                ufd_sel = 1;
                }
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
            //Check the crop setting.
            isCropG1En = MTRUE;
            //m_CrzEn = MTRUE;
        }
        switch(vOutPorts[i]->index){
            case EPortIndex_IMG2O:
                idx_img2o = i;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_IMG2O_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_IMG2O_TCM_EN;
                portInfo_img2o =  (PortInfo)*vOutPorts[idx_img2o];
                break;
            case EPortIndex_IMG3O:
                idx_img3o = i;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_IMG3O_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_IMG3O_TCM_EN;
                portInfo_img3o =  (PortInfo)*vOutPorts[idx_img3o];
                if(vOutPorts[i]->u4Stride[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufSize[ESTRIDE_2ND_PLANE] && vOutPorts[i]->u4BufVA[ESTRIDE_2ND_PLANE]
                    && vOutPorts[i]->u4BufPA[ESTRIDE_2ND_PLANE] && vOutPorts[i]->memID[ESTRIDE_2ND_PLANE]) { // check img3bo
                    //
                    if(vOutPorts[i]->eImgFmt==eImgFmt_YV16 || vOutPorts[i]->eImgFmt==eImgFmt_NV16){
                        img3o_uv_v_ratio = 1;
                    } else if(vOutPorts[i]->eImgFmt==eImgFmt_YV12) {
                        img3o_uv_swap = MTRUE;
                    }
                    idx_img3bo = i;
                    en_p2_dma |= CAM_CTL_EN_P2_DMA_IMG3BO_EN;
                    tcm_en_collect |= CAM_CTL_TPIPE_IMG3BO_TCM_EN;
                    portInfo_img3bo =  (PortInfo)*vOutPorts[idx_img3bo];

                    if(vOutPorts[i]->u4Stride[ESTRIDE_3RD_PLANE] && vOutPorts[i]->u4BufSize[ESTRIDE_3RD_PLANE] && vOutPorts[i]->u4BufVA[ESTRIDE_3RD_PLANE]
                        && vOutPorts[i]->u4BufPA[ESTRIDE_3RD_PLANE] && vOutPorts[i]->memID[ESTRIDE_3RD_PLANE]) { // check img3co
                        idx_img3co = i;
                        en_p2_dma |= CAM_CTL_EN_P2_DMA_IMG3CO_EN;
                        tcm_en_collect |= CAM_CTL_TPIPE_IMG3CO_TCM_EN;
                        portInfo_img3co =  (PortInfo)*vOutPorts[idx_img3co];
                    }
                }
                break;
            case EPortIndex_WROTO:
                idx_wroto = i;
                en_p2 |= CAM_CTL_EN_P2_MDPCROP_EN|CAM_CTL_EN_P2_C24B_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_MDPCROP_TCM_EN | CAM_CTL_TPIPE_C24B_TCM_EN;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MDP_WROTO_EN;
                portInfo_wroto =  (PortInfo)*vOutPorts[idx_wroto];
                break;
            case EPortIndex_WDMAO:
                idx_wdmao = i;
                en_p2 |= CAM_CTL_EN_P2_MDPCROP_EN|CAM_CTL_EN_P2_C24B_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_MDPCROP_TCM_EN | CAM_CTL_TPIPE_C24B_TCM_EN;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MDP_WDMAO_EN;
                portInfo_wdmao =  (PortInfo)*vOutPorts[idx_wdmao];
                break;
            case EPortIndex_JPEGO:
                idx_jpego = i;
                en_p2 |= CAM_CTL_EN_P2_MDPCROP_EN|CAM_CTL_EN_P2_C24B_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_MDPCROP_TCM_EN | CAM_CTL_TPIPE_C24B_TCM_EN;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MDP_JPEGO_EN;
                portInfo_jpego =  (PortInfo)*vOutPorts[idx_jpego];
                break;
            case EPortIndex_FEO:
                if(meScenarioID == eDrvScenario_CC_vFB_FB)
                {
                    PIPE_ERR("[Error]do not support FE when P2B path");
                    return MFALSE;
                }
                idx_feo = i;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_FEO_EN;
                //en_p2 |= CAM_CTL_EN_P2_SRZ1_EN | CAM_CTL_EN_P2_FE_EN;
                en_p2 |=CAM_CTL_EN_P2_FE_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_FEO_TCM_EN | CAM_CTL_TPIPE_FE_TCM_EN;
                //criterion about choosing fe_sel?
                portInfo_feo =  (PortInfo)*vOutPorts[idx_feo];
                break;
            case EPortIndex_IMGO: // ip raw scenario
                if((meScenarioID!=eDrvScenario_IP) && (meScenarioID!=eDrvScenario_CC_RAW))
                {
                    PIPE_ERR("[Error]do not support DMA IMGO for this scenario(%d)",meScenarioID);
                    return MFALSE;
                }
                tpipe = CAM_MODE_FRAME;  // only support frame mode for dma imgo in ip scenario
                idx_imgo = i;
                en_p1 = CAM_CTL_EN_P1_PAK_EN;
                en_p1_dma = CAM_CTL_EN_P1_DMA_IMGO_EN;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MDP_IMGXO_IMGO_EN;
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
                    case eImgFmt_BAYER14:
                        cam_out_fmt = CAM_FMT_SEL_BAYER14;
                        break;
                    default:
                        PIPE_ERR("[Error]do not support this fmt(0x%x) for imgo",vOutPorts[i]->eImgFmt);
                        break;
                };
                break;
            case EPortIndex_MFBO: // mfb blending
                idx_mfbo = i;
                portInfo_mfbo =  (PortInfo)*vOutPorts[idx_mfbo];
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MFBO_EN;
                break;
            case EPortIndex_VENC_STREAMO:
                idx_venco = i;
                portInfo_venco =  (PortInfo)*vOutPorts[idx_venco];
                en_p2 |= CAM_CTL_EN_P2_MDPCROP_EN|CAM_CTL_EN_P2_C24B_EN;
                tcm_en_collect |= CAM_CTL_TPIPE_MDPCROP_TCM_EN | CAM_CTL_TPIPE_C24B_TCM_EN;
                en_p2_dma |= CAM_CTL_EN_P2_DMA_MDP_VENC_EN;
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
    /////////////////////////////////////////////////////////////////////////////////////////////
    //calculate crop offset of img3o for tpipe if mdp crop is set
    if(idx_img3o >= 0)
    {
        if((pPipePackageInfo->isV3==true) || (FORCE_EN_DIFFERENR_VIEW_TEST==1))
    {    //support different view angle
        portInfo_img3o.xoffset = portInfo_img3o.xoffset & (~1);    //img3o offset must be even
          portInfo_img3o.yoffset = portInfo_img3o.yoffset & (~1);
          portInfo_img3bo.xoffset = portInfo_img3o.xoffset;
          portInfo_img3bo.yoffset = portInfo_img3o.yoffset;
          portInfo_img3co.xoffset = portInfo_img3o.xoffset;
          portInfo_img3co.yoffset = portInfo_img3o.yoffset;
          PIPE_INF("IMG3O cpoft(%d/%d)",portInfo_img3o.xoffset,portInfo_img3o.yoffset);
    }
    else
    {
          if((idx_wroto + idx_wdmao + idx_jpego + idx_venco) + 4 > 0 )
          {    //would enable mdp output
              portInfo_img3o.xoffset = portInfo_imgi.crop2.x & (~1);    //img3o offset must be even
              portInfo_img3o.yoffset = portInfo_imgi.crop2.y & (~1);
          }
          else if((idx_img2o) + 1 > 0 )
          {    //would enable mdp output
              portInfo_img3o.xoffset = portInfo_imgi.crop1.x & (~1);    //img3o offset must be even
              portInfo_img3o.yoffset = portInfo_imgi.crop1.y & (~1);
          }
          portInfo_img3bo.xoffset = portInfo_img3o.xoffset;
          portInfo_img3bo.yoffset = portInfo_img3o.yoffset;
          portInfo_img3co.xoffset = portInfo_img3o.xoffset;
          portInfo_img3co.yoffset = portInfo_img3o.yoffset;
          PIPE_INF("IMG3O cpoft(%d/%d)",portInfo_img3o.xoffset,portInfo_img3o.yoffset);
      }
  }
    /////////////////////////////////////////////////////////////////////////////////////////////
    //
    for (MUINT32 i = 0 ; i < pPipePackageInfo->vModuleParams.size() ; i++ )
    {
        switch(pPipePackageInfo->vModuleParams[i].eP2module)
    {
            case EP2Module_NR3D:
                        if(reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    en_p2 |= CAM_CTL_EN_P2_NR3D_EN;
                            tcm_en_collect |= CAM_CTL_TPIPE_NR3D_TCM_EN;
                            m_camPass2Param.nr3d_cfg.ctrl_lmtOutCntTh=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->ctrl_lmtOutCntTh;
                    m_camPass2Param.nr3d_cfg.ctrl_onEn=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->ctrl_onEn;
                    m_camPass2Param.nr3d_cfg.onOff_onOfStX=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->onOff_onOfStX;
                    m_camPass2Param.nr3d_cfg.onOff_onOfStY=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->onOff_onOfStY;
                    m_camPass2Param.nr3d_cfg.onSiz_onWd=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->onSiz_onWd;
                    m_camPass2Param.nr3d_cfg.onSiz_onHt=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->onSiz_onHt;
                    m_camPass2Param.nr3d_cfg.gain_weighting=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->gain_weighting;
                    m_camPass2Param.nr3d_cfg.vipi_offst=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->vipi_offst;
                    m_camPass2Param.nr3d_cfg.vipi_readW=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->vipi_readW;
                    m_camPass2Param.nr3d_cfg.vipi_readH=reinterpret_cast<Nr3DCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->vipi_readH;
                    PIPE_DBG("NR3D om_en/lmtoutTh/on_offX/on_offY/on_SizX/on_SizX (0x%x/0x%x/0x%x/0x%x/0x%x/0x%x/0x%x)",m_camPass2Param.nr3d_cfg.ctrl_lmtOutCntTh,\
                        m_camPass2Param.nr3d_cfg.ctrl_onEn,m_camPass2Param.nr3d_cfg.onOff_onOfStX,m_camPass2Param.nr3d_cfg.onOff_onOfStY,\
                        m_camPass2Param.nr3d_cfg.onSiz_onWd,m_camPass2Param.nr3d_cfg.onSiz_onHt,m_camPass2Param.nr3d_cfg.gain_weighting);
                    /////////////////////////////////////////////////////////////////////////////////////////////
                    //NR3D request, need adjust starting address according to x/y offset for tile calculation//
                    if(idx_vipi != -1)
                    {
                        //update vipi offset, image size  and baseAddress here due to feature user could not revise w/h of vipi(frame n+1) in imagebuffer
                        //cuz gpu is reading img3o(frame n) at the same time if eis is on
                        //nr3d_cfg.vipi_offst is in byte, xoffset/yoffset are in pixel for tpipe
                            MINT32 pixelInByte=1;
                            switch(portInfo_vipi.eImgFmt)
                            {
                                case eImgFmt_YUY2:
                                    pixelInByte=2;
                                portInfo_vipi.xoffset = (m_camPass2Param.nr3d_cfg.vipi_offst % portInfo_vipi.u4Stride[0])/pixelInByte;
                        portInfo_vipi.yoffset = m_camPass2Param.nr3d_cfg.vipi_offst / portInfo_vipi.u4Stride[0];
                                    portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vipi.xoffset * pixelInByte);
                                case eImgFmt_YV12:
                                default:
                                    pixelInByte=1;
                                portInfo_vipi.xoffset = (m_camPass2Param.nr3d_cfg.vipi_offst % portInfo_vipi.u4Stride[0])/pixelInByte;
                        portInfo_vipi.yoffset = m_camPass2Param.nr3d_cfg.vipi_offst / portInfo_vipi.u4Stride[0];
                                    portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vipi.xoffset * pixelInByte);
                                    portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vipi.xoffset * pixelInByte);
                                    portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vipi.yoffset * portInfo_vipi.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vipi.xoffset * pixelInByte);

                                    portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip2i.xoffset * pixelInByte);
                                    portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip2i.xoffset * pixelInByte);
                                    portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip2i.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vip2i.yoffset * portInfo_vip2i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip2i.xoffset * pixelInByte);

                                    portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_1ST_PLANE] +
                                        (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_1ST_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                                    portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_2ND_PLANE] +
                                        (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_2ND_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                                    portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] = portInfo_vip3i.u4BufPA[ESTRIDE_3RD_PLANE] +
                                        (portInfo_vip3i.yoffset * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE] + portInfo_vip3i.xoffset * pixelInByte);
                                    break;
                            }
                          PIPE_DBG("VIPI starting PA(0x%x/0x%x/0x%x)/VIP2I(0x%x/0x%x/0x%x)/VIP3I(0x%x/0x%x/0x%x)",\
                              portInfo_vipi.u4BufPA[0],portInfo_vipi.u4BufPA[1],portInfo_vipi.u4BufPA[2],\
                              portInfo_vip2i.u4BufPA[0],portInfo_vip2i.u4BufPA[1],portInfo_vip2i.u4BufPA[2],\
                              portInfo_vip3i.u4BufPA[0],portInfo_vip3i.u4BufPA[1],portInfo_vip3i.u4BufPA[2]);

                          //update real w/h of vipi for tpipe calculation and hw
                          portInfo_vipi.u4ImgWidth = m_camPass2Param.nr3d_cfg.vipi_readW;
                          portInfo_vipi.u4ImgHeight = m_camPass2Param.nr3d_cfg.vipi_readH;
                          portInfo_vip2i.u4ImgWidth = m_camPass2Param.nr3d_cfg.vipi_readW;
                          portInfo_vip2i.u4ImgHeight = m_camPass2Param.nr3d_cfg.vipi_readH;
                          portInfo_vip3i.u4ImgWidth = m_camPass2Param.nr3d_cfg.vipi_readW;
                          portInfo_vip3i.u4ImgHeight = m_camPass2Param.nr3d_cfg.vipi_readH;
                          PIPE_INF("[vipi]ofst(%d),rW/H(%d/%d)",m_camPass2Param.nr3d_cfg.vipi_offst,m_camPass2Param.nr3d_cfg.vipi_readW,m_camPass2Param.nr3d_cfg.vipi_readH);
                      }
                   }
                   else
                   {
                       PIPE_ERR("Null Module Setting");
                   }
                break;
            case EP2Module_SRZ1:
                if(reinterpret_cast<SrzSizeCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct))
                {
                    en_p2|=CAM_CTL_EN_P2_SRZ1_EN;
                    tcm_en_collect |= CAM_CTL_TPIPE_SRZ1_TCM_EN;
                    m_camPass2Param.srz1_cfg.inout_size.in_w=reinterpret_cast<SrzSizeCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->in_w;
                    m_camPass2Param.srz1_cfg.inout_size.in_h=reinterpret_cast<SrzSizeCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->in_h;
                    m_camPass2Param.srz1_cfg.inout_size.out_w=reinterpret_cast<SrzSizeCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->out_w;
                    m_camPass2Param.srz1_cfg.inout_size.out_h=reinterpret_cast<SrzSizeCfg*>(pPipePackageInfo->vModuleParams[i].moduleStruct)->out_h;
                    m_camPass2Param.srz1_cfg.crop.x=0;
                    m_camPass2Param.srz1_cfg.crop.floatX=0;
                    m_camPass2Param.srz1_cfg.crop.y=0;
                    m_camPass2Param.srz1_cfg.crop.floatY=0;
                }
                break;
            default:
                PIPE_ERR("[Error]do not support this vOutPorts(%d),i(%d)",vOutPorts[i]->index,i);
                break;
        };
    }

    if (pPipePackageInfo->isV3==true)
    {
        pixIdP2 = pPipePackageInfo->pixIdP2;
        PIPE_INF("V3 : pixIdP2(%d)",pixIdP2);
    }
    else
    { //V1
    // check rrz info. and pixel id
        if((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))
        {
        pixIdP2=(MINT32)(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->img_pxl_id);
        //
        // check pixel id
        if(pixIdP2!=CAM_PIX_ID_B && pixIdP2!=CAM_PIX_ID_Gb && pixIdP2!=CAM_PIX_ID_Gr && pixIdP2!=CAM_PIX_ID_R){
            PIPE_ERR("[Error]Get incorrect pixelId(%d), set it to 0",pixIdP2);
            pixIdP2 = 0;
        }

        PIPE_DBG("[rrz]dma=%d,pixIdP2(%d)",(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->dma),pixIdP2);
            if((((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->dma)==ISP_DMA_RRZO || (((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->dma)==ISP_DMA_RRZO_D)
            {

            rrzCropX = (MUINT32)(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcX);
            rrzCropY = (MUINT32)(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcY);
            if((((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.dstH)) {
                rrzRatio = (MFLOAT)(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.dstH) / (((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcH);
            } else {
                PIPE_ERR("[Error]wrong dstH(%d),pCookie(0x%x),rrz_info(0x%x)",(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.dstH),(pPipePackageInfo->pPrivateData),&(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info));
            }
            PIPE_DBG("[rrz]pCookie(0x%x),src(%d,%d,%d,%d),dst(%d,%d)",(pPipePackageInfo->pPrivateData), \
                (((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcX),(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcY),(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcW), \
                (((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.srcH),(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.dstW),(((stISP_BUF_INFO*)(pPipePackageInfo->pPrivateData))->rrz_info.dstH));
        }

        }
        else if(portInfo_imgi.eImgFmt==eImgFmt_YVYU || portInfo_imgi.eImgFmt==eImgFmt_UYVY || portInfo_imgi.eImgFmt==eImgFmt_VYUY)
        {
        PIPE_INF("NULL cookie for eImgFmt(0x%08x)",vInPorts[idx_imgi]->eImgFmt);
        }
        else
        {
        PIPE_WRN("[warning]NULL cookie");
        }
    }

    //judge for jpeg working buffer allocation
    if(idx_jpego==-1)
    {}
    else
    {
        if(m_camPass2Param.jpgpara.workingMemBuf.base_vAddr)
        {   //allocated before
        }
        else
        {
            IMEM_BUF_INFO jpg_buf_info;
            jpg_WorkingMem_Size = 2*vOutPorts[idx_jpego]->u4BufSize[0];
            jpg_buf_info.size = jpg_WorkingMem_Size;
            if(jpg_WorkingMem_Size==0)
            {
                PIPE_ERR("ERROR:passin jpeg working buffer size is 0");
                return MFALSE;
            }
            if ( m_pIspDrvShell->m_pIMemDrv->allocVirtBuf(&jpg_buf_info) ) {
                PIPE_ERR("ERROR:m_pIMemDrv->allocVirtBuf");
                return MFALSE;
            }
            jpg_WorkingMem_memId = jpg_buf_info.memID;

            m_camPass2Param.jpgpara.memID = jpg_buf_info.memID;
            m_camPass2Param.jpgpara.workingMemBuf.base_vAddr = jpg_buf_info.virtAddr;
            m_camPass2Param.jpgpara.workingMemBuf.base_pAddr = jpg_buf_info.phyAddr;
            m_camPass2Param.jpgpara.workingMemBuf.size = jpg_WorkingMem_Size;
            m_camPass2Param.jpgpara.soi_en = MTRUE;
            m_camPass2Param.jpgpara.fQuality = 90;
            pJpgConfigVa = (MUINT32*)jpg_buf_info.virtAddr;
        }
        //
        if ((portInfo_jpego.eImgRot == eImgRot_90) || (portInfo_jpego.eImgRot == eImgRot_180))
        {
           if ( -1 != idx_imgi) {
              IMGI_V_FLIP_EN = 1;
              pixIdP2 = ((pixIdP2+2) & 0x03);
           }
           if ( -1 != idx_img2o) {
              portInfo_img2o.eImgRot = eImgRot_180;
              portInfo_img2o.eImgFlip = eImgFlip_ON;
           }
           if ( -1 != idx_img3o) {
              portInfo_img3o.eImgRot = eImgRot_180;
              portInfo_img3o.eImgFlip = eImgFlip_ON;
           }
           if ( -1 != idx_img3bo) {
              portInfo_img3bo.eImgRot = eImgRot_180;
              portInfo_img3bo.eImgFlip = eImgFlip_ON;
           }
           if ( -1 != idx_img3co) {
              portInfo_img3co.eImgRot = eImgRot_180;
              portInfo_img3co.eImgFlip = eImgFlip_ON;
           }
        }
    }


    //judgement about one-plane/multi-plane
    //c02
    MINT32 vipxi_en=(int)((en_p2_dma&0x0000001c)>>2);
    PIPE_DBG("[C02] en_p2_dma(0x%x),vipxi_en(0x%x)\n",en_p2_dma,vipxi_en);
    switch (vipxi_en)
    {   //first-run vipi fmt judgement here for image, second-run judgement later for mfb
        case 1://422, c02_en=0, vipi_fmt=2
            vipi_fmt=CAM_VIPI_FMT_YUV422_1P;
            break;
        case 3://422, c02_en=1, vipi_fmt=1
            en_p2|=CAM_CTL_EN_P2_C02_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_C02_TCM_EN;
            vipi_fmt=CAM_VIPI_FMT_YUV420_2P;
            break;
        case 7://420, crsp_en=1, vipi_fmt=0
            en_p2|=CAM_CTL_EN_P2_C02_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_C02_TCM_EN;
            vipi_fmt=CAM_VIPI_FMT_YUV420_3P;
            break;
        default:    //image only take vipi as input(yuv422), ex: when mfb blending mode, vipi/vip3i is yuv422/weighting table
            vipi_fmt=CAM_VIPI_FMT_YUV422_1P;
            break;
    }
    //crsp
    MINT32 img3x_en=(int)((en_p2_dma&0x00000700)>>8);
    PIPE_DBG("[CRSP] en_p2_dma(0x%x),img3x_en(0x%x)\n",en_p2_dma,img3x_en);
    switch (img3x_en)
    {
        case 1://422, crsp_en=0, img3o_fmt=2
            img3o_fmt=CAM_IMG3O_FMT_YUV422_1P;
            break;
        case 3://422, crsp_en=1, img3o_fmt=1
            en_p2|=CAM_CTL_EN_P2_CRSP_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_CRSP_TCM_EN;
            img3o_fmt=CAM_IMG3O_FMT_YUV420_2P;
            break;
        case 7://420, crsp_en=1, img3o_fmt=0
            en_p2|=CAM_CTL_EN_P2_CRSP_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_CRSP_TCM_EN;
            img3o_fmt=CAM_IMG3O_FMT_YUV420_3P;
            break;
        default:
            break;
    }

    en_p2 |= ((m_Nr3dEn)?(CAM_CTL_EN_P2_NR3D_EN):(0));

    //should be before scenario parsing
    switch( portInfo_imgi.eImgFmt ) {
        case eImgFmt_FG_BAYER8:      //= 0x01000000,   //FG Bayer format, 8-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER8;
            fg_mode = CAM_FG_MODE_ENABLE;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_FG_BAYER10:      //= 0x02000000,   //FG Bayer format, 10-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER10;
            fg_mode = CAM_FG_MODE_ENABLE;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_FG_BAYER12:      //= 0x04000000,   //FG Bayer format, 12-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER12;
            fg_mode = CAM_FG_MODE_ENABLE;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_FG_BAYER14:     //= 0x08000000,   //FG Bayer format, 14-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER14;
            fg_mode = CAM_FG_MODE_ENABLE;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_UFO_BAYER10:
            cam_in_fmt = CAM_FMT_SEL_BAYER10;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_BAYER8:    //= 0x00000001,   //Bayer format, 8-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER8;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_BAYER10:   //= 0x00000002,   //Bayer format, 10-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER10;

            //if(m_Nr3dEn)
            // pixel_byte_vipi = 2<<CAM_ISP_PIXEL_BYTE_FP; ??
            subMode = ISP_SUB_MODE_RAW;
            //
            break;
        case eImgFmt_BAYER12:   //= 0x00000004,   //Bayer format, 12-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER12;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_BAYER14:   //= 0x00000008,   //Bayer format, 14-bit
            cam_in_fmt = CAM_FMT_SEL_BAYER14;
            subMode = ISP_SUB_MODE_RAW;
            break;
        case eImgFmt_NV21:      //= 0x00000100,   //420 format, 2 plane (VU)
        case eImgFmt_NV12:      //= 0x00000040,   //420 format, 2 plane (UV)
            cam_in_fmt = CAM_FMT_SEL_YUV420_2P;
//            en_p2 |= CAM_CTL_EN1_C02_EN;
            //
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIPI_EN;
            }
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_YV12:      //= 0x00000800,   //420 format, 3 plane (YVU)
        case eImgFmt_I420:      //= 0x00000400,   //420 format, 3 plane(YUV)
            cam_in_fmt = CAM_FMT_SEL_YUV420_3P;
//            en_p2 |= CAM_CTL_EN1_C02_EN;
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIP2I_EN;
            }
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_YUY2:      //= 0x00001000,   //422 format, 1 plane (YUYV)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 1;
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_UYVY:      //= 0x00008000,   //422 format, 1 plane (UYVY)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 0;
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_YVYU:      //= 0x00002000,   //422 format, 1 plane (YVYU)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 3;
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_VYUY:      //= 0x00004000,   //422 format, 1 plane (VYUY)
            cam_in_fmt = CAM_FMT_SEL_YUV422_1P;
            dmai_swap = 2;
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_YV16:      //422 format, 3 plane
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIPI_EN;
                idx_vip2i = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIP2I_EN;
            }
            subMode = ISP_SUB_MODE_YUV;
            break;
        case eImgFmt_NV16:      //422 format, 2 plane
            if (m_isImgPlaneByImgi) {
                idx_vipi = idx_imgi;
//                en_p2_dma |=  CAM_CTL_DMA_EN_VIPI_EN;
            }
            subMode = ISP_SUB_MODE_YUV;
            break;

        case eImgFmt_RGB565:    //= 0x00080000,   //RGB 565 (16-bit), 1 plane
            subMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_RGB888:    //= 0x00100000,   //RGB 888 (24-bit), 1 plane
            dmai_swap = 0; //0:RGB,2:BGR
            subMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_ARGB8888:   //= 0x00800000,   //ARGB (32-bit), 1 plane   //ABGR(MSB->LSB)
            dmai_swap = 1; //0:ARGB,1:RGBA,2:ABGR,3:BGRA(LSB->MSB)
            subMode = ISP_SUB_MODE_RGB;
            break;
        case eImgFmt_NV21_BLK:  //= 0x00000200,   //420 format block mode, 2 plane (UV)
        case eImgFmt_NV12_BLK:  //= 0x00000080,   //420 format block mode, 2 plane (VU)
        case eImgFmt_JPEG:      //= 0x00000010,   //JPEG format
        default:
            PIPE_ERR("[Error]NOT Support this format(0x%x)",portInfo_imgi.eImgFmt);
            return MFALSE;
    }

    switch (subMode)
    {  //eRawPxlID
        case ISP_SUB_MODE_RAW:
            subMode = ISP_SUB_MODE_RAW;
            en_p2 |= CAM_CTL_EN_P2_CFA_EN|CAM_CTL_EN_P2_UNP_EN;
            g2g_sel_en=1;
            g2g_sel=1;
            break;
        case ISP_SUB_MODE_YUV:
            //
            en_p2 |= CAM_CTL_EN_P2_C24_EN;
            subMode = ISP_SUB_MODE_YUV;
            g2g_sel_en=1;
            g2g_sel=0;
            break;
        case ISP_SUB_MODE_RGB:
            subMode = ISP_SUB_MODE_RGB;
            ccl_sel_en=1;
            ccl_sel=1;
            break;
        #if 0
        case eScenarioFmt_MFB:
            isShareDmaCtlByTurn = 0;
            subMode = ISP_SUB_MODE_MFB;
            pixel_byte_lsci = 2<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_lcei = 2<<CAM_ISP_PIXEL_BYTE_FP;
            pixel_byte_imgo = 1<<CAM_ISP_PIXEL_BYTE_FP;
            break;
        #endif
        default:
            PIPE_ERR("[Error]NOT Support this subMode(%d)",subMode);
            return MFALSE;
    }

    PIPE_DBG("meScenarioID:[%d],subMode(%d),eImgFmt(%d),dma(0x%x)",meScenarioID,subMode,portInfo_imgi.eImgFmt,en_p2_dma);

    drvScenario = meScenarioID;


    //v3, involve tuning setting
    if(pPipePackageInfo->isV3==true){
            if(pPipePackageInfo->pTuningQue != NULL){
            MUINT32 tuningEn1Tag = 0x00;
            MUINT32 tuningEn2Tag = 0x00;
            MUINT32 tuningDmaTag = 0x00;
            //
            this->m_CamPathPass2.ispTuningCtrl.getEnTuningTag(MTRUE, ISP_DRV_CQ01, (MUINT32)drvScenario, p2SubMode, tuningEn1Tag, tuningEn2Tag, tuningDmaTag);
            //
                isApplyTuning=MTRUE;
            pIspPhyReg = (isp_reg_t*)pPipePackageInfo->pTuningQue;
            en_p1 = (en_p1 & (~tuningEn1Tag)) | (pIspPhyReg->CAM_CTL_EN_P1.Raw & tuningEn1Tag);
            en_p2 = (en_p2 & (~tuningEn2Tag)) | (pIspPhyReg->CAM_CTL_EN_P2.Raw & tuningEn2Tag);
            en_p1_dma = (en_p1_dma & (~tuningDmaTag)) | (pIspPhyReg->CAM_CTL_EN_P1_DMA.Raw & tuningDmaTag);
            //
            if(en_p1_dma & CAM_CTL_EN_P1_DMA_LSCI_EN){
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
                //TODO: add tuning setting here
                //SHOULD PARSE LCE/LCEI DATA IF PASS FROM TUNING
                //tcm_en_collect |= CAM_CTL_TPIPE_LCE_TCM_EN;
                //tcm_en_collect |= CAM_CTL_TPIPE_LCEI_TCM_EN;

            //
            PIPE_INF("[V3]pIspPhyReg(0x%08x)",pIspPhyReg);
            PIPE_INF("[V3]hwScenario(%d),p2SubMode(%d),tuningEn1Tag(0x%x),tuningEn2Tag(0x%x),tuningDmaTag(0x%x)",hwScenario,p2SubMode,tuningEn1Tag,tuningEn2Tag,tuningDmaTag);
            PIPE_INF("[V3_Tuning]en1(0x%08x),en2(0x%08x),dma(0x%08x)",pIspPhyReg->CAM_CTL_EN_P1.Raw,pIspPhyReg->CAM_CTL_EN_P2.Raw,pIspPhyReg->CAM_CTL_EN_P1_DMA.Raw);
            PIPE_INF("[V3]en1(0x%08x),en2(0x%08x),dma(0x%08x)",en_p1,en_p2,en_p1_dma);
        } else {
                isApplyTuning=MFALSE;
                PIPE_ERR("[Error]p2 tuning not be passed via imageio in V3");
            }
            //
        rrzCropX = (MUINT32)pPipePackageInfo->mP1PrivateData.ResizeCropX;
        rrzCropY = (MUINT32)pPipePackageInfo->mP1PrivateData.ResizeCropY;
        if(pPipePackageInfo->mP1PrivateData.ResizeDstH) {
            rrzRatio = (MFLOAT)(pPipePackageInfo->mP1PrivateData.ResizeDstH) / (pPipePackageInfo->mP1PrivateData.ResizeSrcH);
        } else {
            PIPE_ERR("[Error]wrong dstH(%d),hrz_info(0x%x)",pPipePackageInfo->mP1PrivateData.ResizeDstH,rrzRatio);
        }
        PIPE_DBG("[hrz]crop(%d,%d),src(%d,%d),dst(%d,%d)", \
            pPipePackageInfo->mP1PrivateData.ResizeCropX,pPipePackageInfo->mP1PrivateData.ResizeCropY, \
            pPipePackageInfo->mP1PrivateData.ResizeSrcW,pPipePackageInfo->mP1PrivateData.ResizeSrcH, \
            pPipePackageInfo->mP1PrivateData.ResizeDstW,pPipePackageInfo->mP1PrivateData.ResizeDstH);

        }


    //christopher temp add
    ctlSelGlobal=0X00004000;    //ENABLE INT_MRG to get irq


    switch (meScenarioID) {
        case eDrvScenario_CC:       // P2A
            drvScenario = eDrvScenario_CC;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=1;
            - crz_sel=2;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;
            */
            ctl_sel=0x00001640;
            break;
        case eDrvScenario_CC_SShot:       // P2A, img3o is after crz
            drvScenario = eDrvScenario_CC_SShot;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=0;
            - crz_sel=0;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;
            */
            ctl_sel=0x00001040;
            break;
        case eDrvScenario_CC_vFB_Normal: //vFB p2a
            drvScenario = eDrvScenario_CC_vFB_Normal;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=1;
            - mix1_sel=1;
            - nr3d_sel=1;
            - crz_sel=1;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;
            */
            //ctl_sel=0x000015C0;
            ctl_sel=0x000005C0;    //fe_sel is 0 for eis
            break;
        case eDrvScenario_CC_vFB_FB:
            drvScenario = eDrvScenario_CC_vFB_FB;
            ufdi_fmt = CAM_UFDI_FMT_WEIGHTING;
            en_p2|=(CAM_CTL_EN_P2_MIX1_EN|CAM_CTL_EN_P2_SRZ1_EN|CAM_CTL_EN_P2_MIX2_EN|CAM_CTL_EN_P2_SRZ2_EN);
            en_p2|=CAM_CTL_EN_P2_PCA_EN;
            //FIXME, temp enable crz when p2b
            en_p2|=CAM_CTL_EN_P2_CRZ_EN;
            m_CrzEn=MTRUE;

            tcm_en_collect |= CAM_CTL_TPIPE_SRZ1_TCM_EN|CAM_CTL_TPIPE_SRZ2_TCM_EN|CAM_CTL_TPIPE_CRZ_TCM_EN;

            //srz1_in=ufdi,srz1_out=vipi
            m_camPass2Param.srz1_cfg.inout_size.in_w=portInfo_ufdi.u4ImgWidth;
            m_camPass2Param.srz1_cfg.inout_size.in_h=portInfo_ufdi.u4ImgHeight;
            m_camPass2Param.srz1_cfg.inout_size.out_w=portInfo_vipi.u4ImgWidth;
            m_camPass2Param.srz1_cfg.inout_size.out_h=portInfo_vipi.u4ImgHeight;
            m_camPass2Param.srz1_cfg.crop.x=0;
            m_camPass2Param.srz1_cfg.crop.floatX=0;
            m_camPass2Param.srz1_cfg.crop.y=0;
            m_camPass2Param.srz1_cfg.crop.floatY=0;
            //srz2_in=lcei,srz2_out=vipi
            m_camPass2Param.srz2_cfg.inout_size.in_w=portInfo_lcei.u4ImgWidth;
            m_camPass2Param.srz2_cfg.inout_size.in_h=portInfo_lcei.u4ImgHeight;
            m_camPass2Param.srz2_cfg.inout_size.out_w=portInfo_vipi.u4ImgWidth;
            m_camPass2Param.srz2_cfg.inout_size.out_h=portInfo_vipi.u4ImgHeight;
            m_camPass2Param.srz2_cfg.crop.x=0;
            m_camPass2Param.srz2_cfg.crop.floatX=0;
            m_camPass2Param.srz2_cfg.crop.y=0;
            m_camPass2Param.srz2_cfg.crop.floatY=0;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=0;
            - mix1_sel=1;
            - nr3d_sel=1;
            - crz_sel=1;
            - mdp_sel=0;
            - fe_sel=0; (actually we do not support fe in this path)
            - pca_sel=0;
            */
            ctl_sel = 0x00000580;
            srz1_sel = 0;
            break;
        case eDrvScenario_CC_CShot:
            drvScenario = eDrvScenario_CC;
            //
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=1;
            - crz_sel=2;
            - mdp_sel=1; (key)
            - fe_sel=1;
            - pca_sel=0;
            */
            ctl_sel=0x00001d40;
            //
            break;
        case eDrvScenario_CC_MFB_Blending:
        case eDrvScenario_VSS_MFB_Blending:
            if(meScenarioID==eDrvScenario_CC_MFB_Blending)
            {
            drvScenario = eDrvScenario_CC_MFB_Blending;

            }
            else
            {
                drvScenario = eDrvScenario_VSS_MFB_Blending;
                isWaitBuf=MFALSE;
            }
            mfb_mode = 1;
            en_p2 &= 0xfffffddf;//disable g2g/g2c
            en_p2 |= CAM_CTL_EN_P2_MFB_EN;
            vipi_fmt = CAM_VIPI_FMT_MFB_LOWLIGHT_POSTBLENDING;
            m_camPass2Param.mfb_cfg.out_xofst=0;
            if(idx_vip3i != -1)
            {   //other blending sequences
                m_camPass2Param.mfb_cfg.bld_mode=1;
            }
            else
            {   //first blending
                m_camPass2Param.mfb_cfg.bld_mode=0;
            }
            m_camPass2Param.mfb_cfg.bld_ll_db_en=0;
            m_camPass2Param.mfb_cfg.bld_ll_brz_en=1;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=1;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=0;
            - crz_sel=0;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;

            - without mdp output, sel and output port relationship would be as below
                [1] img2o(o)/img3o(x)  -> crz_sel=2, nr3d_sel=1                                                                      ctl_sel=0x00001660;
                [2] img2o(x)/img3o(o)  -> crz_sel=0, nr3d_sel=0 -> mdp only support imgi in/img2o out in isp only mode(20131106)     ctl_sel=0x00001060;
                [3] img2o(o)/img3o(o)  -> crz_sel=0, nr3d_sel=0 -> no need two same output                                           ctl_sel=0x00001060;
            */
            ctl_sel=0x00001060;
            //TODO, temp test
            ctlSelGlobal=0x00004000;
            break;
        case eDrvScenario_CC_MFB_Mixing:
        case eDrvScenario_VSS_MFB_Mixing:
            if(meScenarioID==eDrvScenario_CC_MFB_Mixing)
            {
            drvScenario = eDrvScenario_CC_MFB_Mixing;
            }
            else
            {
                drvScenario = eDrvScenario_VSS_MFB_Mixing;
                isWaitBuf=MFALSE;
            }
            en_p2 &= 0xfffffddf;//disable g2g/g2c
            en_p2 |= CAM_CTL_EN_P2_MFB_EN | CAM_CTL_EN_P2_MIX3_EN;
            tcm_en_collect |= CAM_CTL_TPIPE_MIX3_TCM_EN;
            vipi_fmt = CAM_VIPI_FMT_MFB_LOWLIGHT_POSTBLENDING;
            ufdi_fmt = CAM_UFDI_FMT_WEIGHTING;
            m_camPass2Param.mfb_cfg.out_xofst=0;
            //mixer mode
            m_camPass2Param.mfb_cfg.bld_mode=5;
            m_camPass2Param.mfb_cfg.bld_ll_db_en=0;
            m_camPass2Param.mfb_cfg.bld_ll_brz_en=0;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=1;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=0;
            - crz_sel=0;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;
            */
            ctl_sel=0x00001060;
            break;
        case eDrvScenario_VSS:
            drvScenario = eDrvScenario_VSS;
            isWaitBuf=MFALSE;
            /*
            - sel notes
            - ufd_sel=0;
            - c24_sel=0;
            - srz1_sel=1;
            - mix1_sel=0;
            - nr3d_sel=1;
            - crz_sel=2;
            - mdp_sel=0;
            - fe_sel=1;
            - pca_sel=0;
            */
            ctl_sel=0x00001640;
            break;
        case eDrvScenario_CC_RAW:
            hwScenario = ISP_HW_SCENARIO_CC;
            drvScenario = eDrvScenario_CC_RAW;
            tcm_en = 0xf; //TILE EDGE must be set to 1 in frame mode
            tcm_en_collect = 0xf;
            break;

        case eDrvScenario_IP:
            hwScenario = ISP_HW_SCENARIO_IP;
            drvScenario = eDrvScenario_IP;
            tcm_en = 0xf; //TILE EDGE must be set to 1 in frame mode
            tcm_en_collect = 0xf;
            break;
        default:
            PIPE_ERR("NOT Support scenario(%d)",meScenarioID);
            return MFALSE;
    }

    //Check CRZ is enable or not.
    if (MTRUE == isCropG1En)
    {
        if ( -1 != idx_img2o)
        {
            m_CrzEn = MTRUE;
        }
        else
        {
            if ((ctl_sel & 0x400) == 0x0)
            {
                if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
                {
                    m_CrzEn = MTRUE;
                }
            }
            else
            {
                if ((ctl_sel & 0x80) == 0x80)
                {
                    if (( -1 != idx_img3o) || ( -1 != idx_wroto) || ( -1 != idx_wdmao) || ( -1 != idx_jpego))
                    {
                        m_CrzEn = MTRUE;
                    }
                }

            }
        }
    }
    en_p2 |= ((m_CrzEn)?(CAM_CTL_EN_P2_CRZ_EN):(0));
    tcm_en_collect |= ((m_CrzEn)?(CAM_CTL_TPIPE_CRZ_TCM_EN):(0));
    PIPE_DBG("isCropG1En(%d),m_CrzEn(%d)",isCropG1En,m_CrzEn);

    //check mdp src crop information
    mdpCheck1 = 0;
    mdpCheck2 = 0;
    mdpCheck3= 0;
    crop1TableIdx = -1;
    tableSize = sizeof(mCropPathInfo)/sizeof(Scen_Map_CropPathInfo_STRUCT);
    for(i=0;i<tableSize;i++) {
        if(mCropPathInfo[i].u4DrvScenId==drvScenario && mCropPathInfo[i].u4CropGroup==ECropGroupIndex_1){
            mdpCheck1 = 1;
            crop1TableIdx = i;
        }
        if(mCropPathInfo[i].u4DrvScenId==drvScenario && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_2)){
            mdpCheck2 = 1;
        }
        if(mCropPathInfo[i].u4DrvScenId==drvScenario && mCropPathInfo[i].u4CropGroup==(ECropGroupIndex_1|ECropGroupIndex_3)){
            mdpCheck3 = 1;
        }
    }
    PIPE_INF("drvScenario(%d),mdpCheck1(%d),mdpCheck2(%d),crop1TableIdx(%d)",drvScenario,mdpCheck1,mdpCheck2,crop1TableIdx);

    if(mdpCheck1 && (mdpCheck2 ||mdpCheck3)){
        // support dma out after crz and dma out after crz+prz in that drvScenario

        MBOOL isMdpSrcCropBeSet;
        //MBOOL isMdpSrcFmtBeSet;
        //
        //isMdpSrcFmtBeSet = MFALSE;
        isMdpSrcCropBeSet = MFALSE;
        for(i=0;i<DMA_OUT_PORT_NUM;i++){
            switch(mCropPathInfo[crop1TableIdx].u4PortID[i]){
                case EPortIndex_IMG2O:
                    if(en_p2_dma & CAM_CTL_EN_P2_DMA_IMG2O_EN){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        m_camPass2Param.croppara.mdp_Yin.w = portInfo_img2o.u4ImgWidth;
                        m_camPass2Param.croppara.mdp_Yin.h = portInfo_img2o.u4ImgHeight;
                        m_camPass2Param.croppara.mdp_Yin.stride = portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                        m_camPass2Param.croppara.mdp_src_fmt = portInfo_img2o.eImgFmt;
                        m_camPass2Param.croppara.mdp_src_size = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_1ST_PLANE];
                        m_camPass2Param.croppara.mdp_src_Csize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_2ND_PLANE];
                        m_camPass2Param.croppara.mdp_src_Vsize = portInfo_img2o.u4ImgHeight * portInfo_img2o.u4Stride[ESTRIDE_3RD_PLANE];
                        m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
                        m_camPass2Param.croppara.mdp_UVin.stride = 0; // only one plane
                    }
                    break;
                case EPortIndex_IMG3O:
                    if(en_p2_dma & CAM_CTL_EN_P2_DMA_IMG3O_EN){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        m_camPass2Param.croppara.mdp_Yin.w = portInfo_img3o.u4ImgWidth;
                        m_camPass2Param.croppara.mdp_Yin.h = portInfo_img3o.u4ImgHeight;
                        m_camPass2Param.croppara.mdp_src_fmt = portInfo_img3o.eImgFmt;
                        switch(portInfo_img3o.eImgFmt){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = portInfo_img3o.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_size = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 2;
                                break;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = portInfo_img3o.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_size = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 3;
                                break;
                            default:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = 0;
                                m_camPass2Param.croppara.mdp_src_size = portInfo_img3o.u4ImgHeight * portInfo_img3o.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_img3bo.u4ImgHeight * portInfo_img3bo.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_img3co.u4ImgHeight * portInfo_img3co.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
                                break;
                        }
                    }
                    break;
                case EPortIndex_FEO:
                    if(en_p2_dma & CAM_CTL_EN_P2_DMA_FEO_EN){
                        isMdpSrcCropBeSet = MTRUE;
                        // for mdp crop information
                        m_camPass2Param.croppara.mdp_Yin.w = portInfo_feo.u4ImgWidth;
                        m_camPass2Param.croppara.mdp_Yin.h = portInfo_feo.u4ImgHeight;
                        m_camPass2Param.croppara.mdp_Yin.stride = portInfo_feo.u4Stride[ESTRIDE_1ST_PLANE];
                        m_camPass2Param.croppara.mdp_UVin.stride = 0; // only one plane
                        m_camPass2Param.croppara.mdp_src_fmt = portInfo_imgi.eImgFmt; //kk test
                    }
                    break;
                case EPortIndex_VIPI:
                    if(en_p2_dma & CAM_CTL_EN_P2_DMA_VIPI_EN){
                        isMdpSrcCropBeSet = MTRUE;
                        //isMdpSrcFmtBeSet = MTRUE;
                        // for mdp crop information
                        m_camPass2Param.croppara.mdp_src_fmt = portInfo_vipi.eImgFmt;
                        m_camPass2Param.croppara.mdp_Yin.w = portInfo_vipi.u4ImgWidth;
                        m_camPass2Param.croppara.mdp_Yin.h = portInfo_vipi.u4ImgHeight;
                        switch(portInfo_vipi.eImgFmt){
                            case eImgFmt_NV21:
                            case eImgFmt_NV12:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_size = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 2;
                            case eImgFmt_YV12:
                            case eImgFmt_I420:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = portInfo_vipi.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_size = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 3;
                                break;
                            default:
                                m_camPass2Param.croppara.mdp_Yin.stride = portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_UVin.stride = 0;
                                m_camPass2Param.croppara.mdp_src_size = portInfo_vipi.u4ImgHeight * portInfo_vipi.u4Stride[ESTRIDE_1ST_PLANE];
                                m_camPass2Param.croppara.mdp_src_Csize = portInfo_vip2i.u4ImgHeight * portInfo_vip2i.u4Stride[ESTRIDE_2ND_PLANE];
                                m_camPass2Param.croppara.mdp_src_Vsize = portInfo_vip3i.u4ImgHeight * portInfo_vip3i.u4Stride[ESTRIDE_3RD_PLANE];
                                m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
                                break;
                        }
                    }
                    break;
                default:
                    break;
            }
            if(isMdpSrcCropBeSet == MTRUE)
                break;
        }

        #if 0
        if(isMdpSrcFmtBeSet == MFALSE){
            //set mdp src info. equal to imgi
            m_camPass2Param.croppara.mdp_src_fmt = portInfo_imgi.eImgFmt;
        }
        #endif

        if(isMdpSrcCropBeSet == MFALSE){
            //set mdp src info. equal to imgi
            m_camPass2Param.croppara.mdp_Yin.w = portInfo_imgi.u4ImgWidth;
            m_camPass2Param.croppara.mdp_Yin.h = portInfo_imgi.u4ImgHeight;
            m_camPass2Param.croppara.mdp_Yin.stride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
            m_camPass2Param.croppara.mdp_UVin.stride = 0; // only one plane
            m_camPass2Param.croppara.mdp_src_size = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
            m_camPass2Param.croppara.mdp_src_fmt = portInfo_imgi.eImgFmt;
            m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
        }
    } else {
        //set mdp src info. equal to imgi
        m_camPass2Param.croppara.mdp_src_fmt = portInfo_imgi.eImgFmt;
        m_camPass2Param.croppara.mdp_Yin.w = portInfo_imgi.u4ImgWidth;
        m_camPass2Param.croppara.mdp_Yin.h = portInfo_imgi.u4ImgHeight;
        m_camPass2Param.croppara.mdp_Yin.stride = portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
        m_camPass2Param.croppara.mdp_UVin.stride = 0;
        m_camPass2Param.croppara.mdp_src_size = portInfo_imgi.u4ImgHeight * portInfo_imgi.u4Stride[ESTRIDE_1ST_PLANE];
        m_camPass2Param.croppara.mdp_src_Plane_Num = 1;
    }
    getCrzPosition(drvScenario, isSl2cAheadCrz, isLceAheadCrz);

    //tpipe extension for v3
    if((pPipePackageInfo->isV3==true) || FORCE_EN_DIFFERENR_VIEW_TEST)
    {
        #if 0  //kk test default:1 ==> need to fix for different view angle
        tcm_en_collect|=CAM_CTL_TPIPE_EXTENSION_EN;
        #endif
        tcm_en=tcm_en_collect;
    }
    //

    PIPE_INF("Pass(%d),imgi.fmt(0x%x),meScenarioID(%d),isCropG1En(%d),m_CrzEn(%d),mdpSrcFmt(0x%x),mdp_src(%d,%d,%d,%d),en_p2_dma(0x%08x),crop1TableIdx(%d),mdpCheck1(%d),mdpCheck2(%d),drvScenario(%d),tcm_en(0x%x)",\
    this->m_pipePass,portInfo_imgi.eImgFmt,meScenarioID,isCropG1En,m_CrzEn,m_camPass2Param.croppara.mdp_src_fmt, \
    m_camPass2Param.croppara.mdp_Yin.w,m_camPass2Param.croppara.mdp_Yin.h,m_camPass2Param.croppara.mdp_Yin.stride,m_camPass2Param.croppara.mdp_UVin.stride,\
        en_p2_dma,crop1TableIdx,mdpCheck1,mdpCheck2,drvScenario,tcm_en);
    //
    m_camPass2Param.dupCqIdx = pPipePackageInfo->dupCqIdx;
    m_camPass2Param.burstQueIdx = pPipePackageInfo->burstQIdx;
    m_camPass2Param.magicNum = magicNum;
    m_camPass2Param.rrzRatio = rrzRatio;
    m_camPass2Param.rrzCropX = rrzCropX;
    m_camPass2Param.rrzCropY = rrzCropY;
    m_camPass2Param.isSl2cAheadCrz = isSl2cAheadCrz;
    m_camPass2Param.isLceAheadCrz = isLceAheadCrz;
    m_camPass2Param.isWaitBuf = isWaitBuf;

    /*-----------------------------------------------------------------------------
      m_camPass2Param
      -----------------------------------------------------------------------------*/
    //
    m_camPass2Param.drvScenario = drvScenario;
    m_camPass2Param.isV3 = pPipePackageInfo->isV3;
    m_camPass2Param.pTuningIspReg = pIspPhyReg;
    m_camPass2Param.isApplyTuning = isApplyTuning;
    //top
    //global sel
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw = ctlSelGlobal;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK = 0x00002000;

    //scenario/sub_mode
    m_camPass2Param.isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO = hwScenario;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE = subMode;
    m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P2.Raw =  en_p2;
    m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw = en_p2_dma;
    m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P1.Raw =  en_p1;
    m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw = en_p1_dma;
    //cq_ctl
    m_camPass2Param.isp_top_ctl.CAM_CTL_CQ_EN_P2.Raw = cq_en_p2;
    //ctl_int
    m_camPass2Param.isp_top_ctl.CAM_CTL_INT_P2_EN.Raw = int_p2_en;
    //fmt_sel_p2
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Raw = 0x00; //reset fmt_sel_p2
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.IN_FMT = cam_in_fmt;
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.PIX_ID_P2 = pixIdP2;
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.IMG3O_FMT = img3o_fmt;
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.VIPI_FMT = vipi_fmt;
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.UFDI_FMT = ufdi_fmt;
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Bits.FG_MODE = fg_mode;
    //fmt_sel_p1
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw = 0x00; //reset fmt_sel_p1
    m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.OUT_FMT = cam_out_fmt;

    //sel_p1
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P1.Raw = 0x00; //reset sel_p1


    //ctl_sel
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Raw = ctl_sel;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.CCL_SEL=ccl_sel;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.CCL_SEL_EN=ccl_sel_en;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.G2G_SEL=g2g_sel;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.G2G_SEL_EN=g2g_sel_en;
    m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.UFD_SEL=ufd_sel;
  m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.SRZ1_SEL=srz1_sel;
  m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Bits.MFB_MODE=mfb_mode;
    //

    PIPE_DBG("SelGlo(0x%x),EnP2(0x%x),EnP2Dma(0x%x),EnP1Dma(0x%x),CqEn(0x%x),IntP2En(0x%x),fmtSelP2(0x%x),selP2(0x%x)", \
            m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P2.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P2_DMA.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_CQ_EN.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_INT_P2_EN.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_FMT_SEL_P2.Raw, \
            m_camPass2Param.isp_top_ctl.CAM_CTL_SEL_P2.Raw);


    m_camPass2Param.CQ = pass2_CQ;
    m_camPass2Param.isShareDmaCtlByTurn = isShareDmaCtlByTurn;
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


        if (MTRUE == m_CrzEn)
        {
            m_camPass2Param.croppara.crz_enable = MTRUE;;
            m_camPass2Param.croppara.crz_out.w = portInfo_imgi.resize1.tar_w;
            m_camPass2Param.croppara.crz_out.h = portInfo_imgi.resize1.tar_h;
            m_camPass2Param.croppara.crz_in.w = portInfo_imgi.u4ImgWidth;
            m_camPass2Param.croppara.crz_in.h = portInfo_imgi.u4ImgHeight;
            m_camPass2Param.croppara.crz_crop.x = portInfo_imgi.crop1.x;
            m_camPass2Param.croppara.crz_crop.floatX = portInfo_imgi.crop1.floatX;
            m_camPass2Param.croppara.crz_crop.y = portInfo_imgi.crop1.y;
            m_camPass2Param.croppara.crz_crop.floatY = portInfo_imgi.crop1.floatY;
            m_camPass2Param.croppara.crz_crop.w = portInfo_imgi.crop1.w;
            m_camPass2Param.croppara.crz_crop.h = portInfo_imgi.crop1.h;
        }
        else
        {
            m_camPass2Param.croppara.crz_enable = MFALSE;
            m_camPass2Param.croppara.crz_out.w = 0;
            m_camPass2Param.croppara.crz_out.h = 0;
            m_camPass2Param.croppara.crz_in.w = 0;
            m_camPass2Param.croppara.crz_in.h = 0;
            m_camPass2Param.croppara.crz_crop.x = 0;
            m_camPass2Param.croppara.crz_crop.floatX = 0;
            m_camPass2Param.croppara.crz_crop.y = 0;
            m_camPass2Param.croppara.crz_crop.floatY = 0;
            m_camPass2Param.croppara.crz_crop.w = 0;
            m_camPass2Param.croppara.crz_crop.h = 0;
        }

        //m_camPass2Param.imgi.lIspColorfmt = portInfo_imgi.eImgFmt;
        m_camPass2Param.imgi.v_flip_en = IMGI_V_FLIP_EN;
        //
        //config n-plane in by imgi only
        if (m_isImgPlaneByImgi) {
            portInfo_vipi = portInfo_imgi;
            portInfo_vipi.u4BufPA[0] = 0x01;//for NOT bypass vipi config in pass2
            portInfo_vip2i = portInfo_imgi;
            portInfo_vip2i.u4BufPA[0] = 0x01;//for NOT bypass vip2i config in pass2
            switch( portInfo_imgi.eImgFmt ) {
                case eImgFmt_NV16:      //= 0x8000,   //422 format, 2 plane
                case eImgFmt_NV21:      //= 0x0010,   //420 format, 2 plane (VU)
                case eImgFmt_NV12:      //= 0x0040,   //420 format, 2 plane (UV)
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;
                    break;
                case eImgFmt_YV12:      //= 0x00008,    //420 format, 3 plane (YVU)
                case eImgFmt_I420:      //= 0x20000,   //420 format, 3 plane(YUV)
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;;
                    //Y+U plane size (Y + 1/4Y)
                    m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*5)>>2;

                    //
                    portInfo_vipi.u4ImgWidth    >>= 1;
                    portInfo_vipi.u4ImgHeight   >>= 1;
                    portInfo_vipi.crop1.x        >>= 1;
                    portInfo_vipi.crop1.floatX   >>= 1;
                    portInfo_vipi.crop1.y        >>= 1;
                    portInfo_vipi.crop1.floatY   >>= 1;
                    portInfo_vipi.crop1.w        >>= 1;
                    portInfo_vipi.crop1.h        >>= 1;
                    //
                    portInfo_vip2i.u4ImgWidth    >>= 1;
                    portInfo_vip2i.u4ImgHeight   >>= 1;
                    portInfo_vip2i.crop1.x        >>= 1;
                    portInfo_vip2i.crop1.floatX   >>= 1;
                    portInfo_vip2i.crop1.y        >>= 1;
                    portInfo_vip2i.crop1.floatY   >>= 1;
                    portInfo_vip2i.crop1.w        >>= 1;
                    portInfo_vip2i.crop1.h        >>= 1;

                    break;
                case eImgFmt_YV16:      //= 0x4000,   //422 format, 3 plane
                    //Y plane size
                    m_camPass2Param.imgi.memBuf_c_ofst = m_camPass2Param.imgi.size.w * m_camPass2Param.imgi.size.h;;
                    //Y+U plane size (Y + 1/2Y)
                    m_camPass2Param.imgi.memBuf_v_ofst = (m_camPass2Param.imgi.memBuf_c_ofst*3)>>1;
                    //
                    portInfo_vipi.u4ImgWidth >>= 1;
                    //portInfo_vipi.u4ImgHeight;
                    portInfo_vipi.crop1.x>>= 1;
                    portInfo_vipi.crop1.floatX>>= 1;
                    //portInfo_vipi.crop.y;
                    portInfo_vipi.crop1.w>>= 1;
                    //portInfo_vipi.crop.h;
                    //
                    portInfo_vip2i.u4ImgWidth >>= 1;
                    //portInfo_vip2i.u4ImgHeight;
                    portInfo_vip2i.crop1.x>>= 1;
                    portInfo_vip2i.crop1.floatX>>= 1;
                    //portInfo_vip2i.crop.y;
                    portInfo_vip2i.crop1.w>>= 1;
                    //portInfo_vip2i.crop.h;

                    break;
                case eImgFmt_YUY2:      //= 0x0100,   //422 format, 1 plane (YUYV)
                case eImgFmt_UYVY:      //= 0x0200,   //422 format, 1 plane (UYVY)
                case eImgFmt_RGB565:    //= 0x0400,   //RGB 565 (16-bit), 1 plane
                case eImgFmt_RGB888:    //= 0x0800,   //RGB 888 (24-bit), 1 plane
                case eImgFmt_ARGB8888:   //= 0x1000,   //ARGB (32-bit), 1 plane
                case eImgFmt_BAYER8:    //= 0x0001,   //Bayer format, 8-bit
                case eImgFmt_BAYER10:   //= 0x0002,   //Bayer format, 10-bit
                case eImgFmt_BAYER12:   //= 0x0004,   //Bayer format, 12-bit
                case eImgFmt_NV21_BLK:  //= 0x0020,   //420 format block mode, 2 plane (UV)
                case eImgFmt_NV12_BLK:  //= 0x0080,   //420 format block mode, 2 plane (VU)
                case eImgFmt_JPEG:      //= 0x2000,   //JPEG format
                default:
                    break;
            }
        }
    }
    //
    if (-1 != idx_vipi ) {
        //PIPE_DBG("config vipi m_Nr3dEn(%d)",m_Nr3dEn);
        m_camPass2Param.vipi.ring_en = 0;
        m_camPass2Param.vipi.ring_size= 0;

        this->configDmaPort(&portInfo_vipi
                , m_camPass2Param.vipi
                , (MUINT32)1
                , (MUINT32)1
                , (MUINT32)1
                , ESTRIDE_1ST_PLANE);


        PIPE_DBG("[vipi]size(0x%08x),Va(0x%08x),Pa(0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x)",
                m_camPass2Param.vipi.memBuf.size,m_camPass2Param.vipi.memBuf.base_vAddr,m_camPass2Param.vipi.memBuf.base_pAddr,m_camPass2Param.vipi.memBuf.ofst_addr,m_camPass2Param.vipi.memBuf.alignment,m_camPass2Param.vipi.size.stride);

        if (-1 != idx_vip2i )
        {    //multi-plane input image, vipi/vip2i/vip3i
            this->configDmaPort(&portInfo_vip2i
                    , m_camPass2Param.vip2i
                    , (MUINT32)1
                    , (MUINT32)0
                    , (MUINT32)1
                    , ESTRIDE_2ND_PLANE);
            m_camPass2Param.vip2i.size.w /= vipi_uv_h_ratio;
            m_camPass2Param.vip2i.size.h /= vipi_uv_v_ratio;
            m_camPass2Param.vip2i.size.xsize /= vipi_uv_h_ratio;
            PIPE_DBG("[vip2i]size[%d,%d](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x)",m_camPass2Param.vip2i.size.w,m_camPass2Param.vip2i.size.h,
                    m_camPass2Param.vip2i.memBuf.size,m_camPass2Param.vip2i.memBuf.ofst_addr,m_camPass2Param.vip2i.memBuf.alignment,m_camPass2Param.vip2i.size.stride);

            if (-1 != idx_vip3i )
            {
                this->configDmaPort(&portInfo_vip3i
                        , m_camPass2Param.vip3i
                        , (MUINT32)1
                        , (MUINT32)0
                        , (MUINT32)1
                        , ESTRIDE_3RD_PLANE);
                m_camPass2Param.vip3i.size.w /= vipi_uv_h_ratio;
                m_camPass2Param.vip3i.size.h /= vipi_uv_v_ratio;
                m_camPass2Param.vip3i.size.xsize /= vipi_uv_h_ratio;
                if(vipi_uv_swap) {
                    m_camPass2Param.vip2i.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_3RD_PLANE];
                    m_camPass2Param.vip2i.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_3RD_PLANE];
                    m_camPass2Param.vip3i.memBuf.base_vAddr = portInfo_vipi.u4BufVA[ESTRIDE_2ND_PLANE];
                    m_camPass2Param.vip3i.memBuf.base_pAddr = portInfo_vipi.u4BufPA[ESTRIDE_2ND_PLANE];
                }
                PIPE_DBG("[vip3i]size[%d,%d](0x%08x),Va(0x%08x),Pa(0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x),vipi_uv_swap(%d)",m_camPass2Param.vip3i.size.w,m_camPass2Param.vip3i.size.h,
                        m_camPass2Param.vip3i.memBuf.size,m_camPass2Param.vip3i.memBuf.base_vAddr,m_camPass2Param.vip3i.memBuf.base_pAddr,m_camPass2Param.vip3i.memBuf.ofst_addr,m_camPass2Param.vip3i.memBuf.alignment,m_camPass2Param.vip3i.size.stride,vipi_uv_swap);
            }
        }
        else
        {
            if (-1 != idx_vip3i )
            {
                //mfb mixing path, only vipi+vip3i get weighting table(without vip2i)
                this->configDmaPort(&portInfo_vip3i,m_camPass2Param.vip3i,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
                m_camPass2Param.vip3i.ring_en = 0;
                m_camPass2Param.vip3i.ring_size= 0;
                //
                m_camPass2Param.vip3i.size.stride = portInfo_vip3i.u4Stride[ESTRIDE_1ST_PLANE];
    }
        }
    }

    //for IP_MFB
    #if 0
    pixel_byte_lsci = pixel_byte_imgi;
    //pixel_byte_lcei = pixel_byte_imgi; // no need, lcei would be in via imageio only in vfb p2b
    m_camPass2Param.mfb.bld_mode = 2;
    #endif
    if (-1 != idx_mfbo) {
        //PIPE_DBG("config mfbo");
        this->configDmaPort(&portInfo_mfbo,m_camPass2Param.mfbo,(MUINT32)1,(MUINT32)1,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.mfbo.size.stride = portInfo_mfbo.u4Stride[ESTRIDE_1ST_PLANE];
        //
        m_camPass2Param.mfb_cfg.mfb_out.w=portInfo_mfbo.u4ImgWidth;
        m_camPass2Param.mfb_cfg.mfb_out.h=portInfo_mfbo.u4ImgHeight;
        m_camPass2Param.mfb_cfg.mfb_out.stride=portInfo_mfbo.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_lsci ) {
        //PIPE_DBG("config lsci");
        this->configDmaPort(&portInfo_lsci,m_camPass2Param.lsci,(MUINT32)1,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.lsci.ring_en = 0;
        m_camPass2Param.lsci.ring_size= 0;
        //
        m_camPass2Param.lsci.size.stride = portInfo_lsci.u4Stride[ESTRIDE_1ST_PLANE];
    }
    // vfb p2b, lcei from imageio
    if (-1 != idx_lcei ) {
        //PIPE_DBG("config lcei");
        this->configDmaPort(&portInfo_lcei,m_camPass2Param.lcei,(MUINT32)1,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.lcei.ring_en = 0;
        m_camPass2Param.lcei.ring_size= 0;
        //
        m_camPass2Param.lcei.size.stride = portInfo_lcei.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if (-1 != idx_ufdi ) {
        //PIPE_DBG("config ufdi");
        this->configDmaPort(&portInfo_ufdi,m_camPass2Param.ufdi,(MUINT32)1,(MUINT32)dmai_swap,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass2Param.ufdi.size.stride = portInfo_ufdi.u4Stride[ESTRIDE_1ST_PLANE];
    }
    //
    if ( -1 != idx_wroto) {
        //PIPE_DBG("config wroto");
        // for digital zoom crop
        m_camPass2Param.wroto.withCropM=true;
        m_camPass2Param.wroto.enSrcCrop=true;
        if((pPipePackageInfo->isV3) || FORCE_EN_DIFFERENR_VIEW_TEST)
        {//support different view angle
            m_camPass2Param.wroto.srcCropX = portInfo_imgi.crop3.x;
            m_camPass2Param.wroto.srcCropFloatX = portInfo_imgi.crop3.floatX;
            m_camPass2Param.wroto.srcCropY= portInfo_imgi.crop3.y;
            m_camPass2Param.wroto.srcCropFloatY =  portInfo_imgi.crop3.floatY;
            m_camPass2Param.wroto.srcCropW = portInfo_imgi.crop3.w;
            m_camPass2Param.wroto.srcCropH = portInfo_imgi.crop3.h;
        }
        else
        {
            m_camPass2Param.wroto.srcCropX = portInfo_imgi.crop2.x;
            m_camPass2Param.wroto.srcCropFloatX = portInfo_imgi.crop2.floatX;
            m_camPass2Param.wroto.srcCropY= portInfo_imgi.crop2.y;
            m_camPass2Param.wroto.srcCropFloatY =  portInfo_imgi.crop2.floatY;
            m_camPass2Param.wroto.srcCropW = portInfo_imgi.crop2.w;
            m_camPass2Param.wroto.srcCropH = portInfo_imgi.crop2.h;
        }
        this->configMdpOutPort(vOutPorts[idx_wroto],m_camPass2Param.wroto);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_wdmao) {
        //PIPE_DBG("config wdmao");
        //
        // for digital zoom crop
        m_camPass2Param.wdmao.withCropM=true;
        m_camPass2Param.wdmao.enSrcCrop=true;
        m_camPass2Param.wdmao.srcCropX = portInfo_imgi.crop2.x;
        m_camPass2Param.wdmao.srcCropFloatX = portInfo_imgi.crop2.floatX;
        m_camPass2Param.wdmao.srcCropY= portInfo_imgi.crop2.y;
        m_camPass2Param.wdmao.srcCropFloatY =  portInfo_imgi.crop2.floatY;
        m_camPass2Param.wdmao.srcCropW = portInfo_imgi.crop2.w;
        m_camPass2Param.wdmao.srcCropH = portInfo_imgi.crop2.h;

        this->configMdpOutPort(vOutPorts[idx_wdmao],m_camPass2Param.wdmao);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_jpego) {
        //PIPE_DBG("config jpego");
        //
        this->configMdpOutPort(vOutPorts[idx_jpego],m_camPass2Param.jpego);
        isSetMdpOut = MTRUE;
    }
    if ( -1 != idx_venco) {
        //PIPE_DBG("config venco");
        //
        this->configMdpOutPort(vOutPorts[idx_venco],m_camPass2Param.venco);
        isSetMdpOut = MTRUE;
    }
    //
    if (-1 != idx_imgo ) {  // only support frame mode for dma imgo in ip scenario
        PIPE_DBG("isSetMdpOut(%d),u4BufSize(0x%x)-u4BufVA(0x%x)-u4BufPA(0x%x)",isSetMdpOut, vOutPorts[idx_imgo]->u4BufSize[0],vOutPorts[idx_imgo]->u4BufVA[0],
                    vOutPorts[idx_imgo]->u4BufPA[0]);

        this->configDmaPort(&portInfo_imgo,m_camPass2Param.imgo,(MUINT32)1,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        //
        m_camPass2Param.imgo.size.stride = portInfo_imgo.u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe
        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(vOutPorts[idx_imgo],m_camPass2Param.imgxo);
            mdp_imgxo_p2_en = CAM_CTL_EN_P2_DMA_MDP_IMGXO_IMGO_EN;
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
            mdp_imgxo_p2_en = CAM_CTL_EN_P2_DMA_IMG2O_EN;
            isSetMdpOut = MTRUE;
        }
    }
    //
    if (-1 != idx_img3o ) {
        this->configDmaPort(&portInfo_img3o
                , m_camPass2Param.img3o
                , (MUINT32)1
                , (MUINT32)0
                , (MUINT32)1
                , ESTRIDE_1ST_PLANE);

        if(isSetMdpOut == MFALSE){
            this->configMdpOutPort(vOutPorts[idx_img3o],m_camPass2Param.imgxo);
            mdp_imgxo_p2_en = CAM_CTL_EN_P2_DMA_IMG3O_EN;
            isSetMdpOut = MTRUE;
        }
        PIPE_DBG("[img3o]size(0x%08x),Va(0x%08x),Pa(0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x)",
                m_camPass2Param.img3o.memBuf.size,m_camPass2Param.img3o.memBuf.base_vAddr,m_camPass2Param.img3o.memBuf.base_pAddr,m_camPass2Param.img3o.memBuf.ofst_addr,m_camPass2Param.img3o.memBuf.alignment,m_camPass2Param.img3o.size.stride);

        if (-1 != idx_img3bo )
        {
            this->configDmaPort(&portInfo_img3bo
                    , m_camPass2Param.img3bo
                    , (MUINT32)1
                    , (MUINT32)0
                    , (MUINT32)1
                    , ESTRIDE_2ND_PLANE);
            m_camPass2Param.img3bo.size.w /= img3o_uv_h_ratio;
            m_camPass2Param.img3bo.size.h /= img3o_uv_v_ratio;
            m_camPass2Param.img3bo.size.xsize /= img3o_uv_h_ratio;

            PIPE_DBG("[img3bo]size[%d,%d](0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x)",m_camPass2Param.img3bo.size.w,m_camPass2Param.img3bo.size.h,
                    m_camPass2Param.img3bo.memBuf.size,m_camPass2Param.img3bo.memBuf.ofst_addr,m_camPass2Param.img3bo.memBuf.alignment,m_camPass2Param.img3bo.size.stride);

            if (-1 != idx_img3co )
            {
                this->configDmaPort(&portInfo_img3co
                        , m_camPass2Param.img3co
                        , (MUINT32)1
                        , (MUINT32)0
                        , (MUINT32)1
                        , ESTRIDE_3RD_PLANE);
                m_camPass2Param.img3co.size.w /= img3o_uv_h_ratio;
                m_camPass2Param.img3co.size.h /= img3o_uv_v_ratio;
                m_camPass2Param.img3co.size.xsize /= img3o_uv_h_ratio;
                if(img3o_uv_swap) {
                    m_camPass2Param.img3bo.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_3RD_PLANE];
                    m_camPass2Param.img3bo.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_3RD_PLANE];
                    m_camPass2Param.img3co.memBuf.base_vAddr = vOutPorts[idx_img3o]->u4BufVA[ESTRIDE_2ND_PLANE];
                    m_camPass2Param.img3co.memBuf.base_pAddr = vOutPorts[idx_img3o]->u4BufPA[ESTRIDE_2ND_PLANE];
                }
                PIPE_DBG("[img3co]size[%d,%d](0x%08x),Va(0x%08x),Pa(0x%08x),offset(0x%08x),align(0x%08x),stride(0x%08x),img3o_uv_swap(%d)",m_camPass2Param.img3co.size.w,m_camPass2Param.img3co.size.h,
                        m_camPass2Param.img3co.memBuf.size,m_camPass2Param.img3co.memBuf.base_vAddr,m_camPass2Param.img3co.memBuf.base_pAddr,m_camPass2Param.img3co.memBuf.ofst_addr,m_camPass2Param.img3co.memBuf.alignment,m_camPass2Param.img3co.size.stride,img3o_uv_swap);
            }
        }
    }

#if 0
    if (-1 != idx_feo )
    {
        PIPE_DBG("u4BufSize(0x%x)-u4BufVA(0x%x)-u4BufPA(0x%x)", vOutPorts[idx_feo]->u4BufSize[0],vOutPorts[idx_feo]->u4BufVA[0],
                    vOutPorts[idx_feo]->u4BufPA[0]);
        m_camPass2Param.fe.fem_harris_tpipe_mode = 2;
        //fe mode criterion
        MINT32 p2OUT_w=0;
        MINT32 p2OUT_h=0;
        if (MTRUE == m_CrzEn)
        {
            p2OUT_w=m_camPass2Param.croppara.crz_out.w;
            p2OUT_h=m_camPass2Param.croppara.crz_out.h;
            PIPE_INF("[FESET]p2OUT w/h (%d/%d)",p2OUT_w,p2OUT_h);

            if(p2OUT_w <= D1_WIDTH && p2OUT_h <= D1_HEIGHT)
            {
                //CAM_FE_CTRL = 0x0AE;
                  m_camPass2Param.fe.fem_harris_tpipe_mode = 2;
            }
            else if(p2OUT_w <= EIS_FE_MAX_INPUT_W && p2OUT_h <= EIS_FE_MAX_INPUT_H)
            {
                //CAM_FE_CTRL = 0x0AD;
                m_camPass2Param.fe.fem_harris_tpipe_mode = 1;
            }
            else
            {
                m_camPass2Param.fe.fem_harris_tpipe_mode = 0;
                PIPE_WRN("fem_harris_tpipe_mode should not be 0");
            }
        }
        this->configDmaPort(vOutPorts[idx_feo]
                , m_camPass2Param.feo//@TODO check this
                , (MUINT32)1
                , (MUINT32)0
                , (MUINT32)1
                , ESTRIDE_1ST_PLANE);

        //update feo xsize(unit is byte)
        switch (m_camPass2Param.fe.fem_harris_tpipe_mode)
        {
            case 0:
                    //m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w-1)/32*56;    //56 bytes in one block
                    m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w)/32*56 -1 ;
                    break; // 32x32 block (feo: pixel byte is block bytes)
            case 1:
                    //m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w-1)/16*56;    //56 bytes in one block
                    m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w)/16*56 -1 ;
                    break; // 16x16 block
            default: // 2
                    //m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w-1)/8*56;    //56 bytes in one block
                    m_camPass2Param.feo.size.xsize=(m_camPass2Param.feo.size.w)/8*56 -1 ;
                    break;    // 8x8 block
        }

        m_camPass2Param.feo.size.h = m_camPass2Param.feo.size.h>>(5-m_camPass2Param.fe.fem_harris_tpipe_mode);
        m_camPass2Param.feo.size.stride = vOutPorts[idx_feo]->u4Stride[ESTRIDE_1ST_PLANE];  // for tpipe

        PIPE_DBG("[feo]size(0x%08x),Va(0x%08x),Pa(0x%08x),offset(0x%08x),align(0x%08x)",
                m_camPass2Param.feo.memBuf.size,m_camPass2Param.feo.memBuf.base_vAddr,m_camPass2Param.feo.memBuf.base_pAddr,m_camPass2Param.feo.memBuf.ofst_addr,m_camPass2Param.feo.memBuf.alignment);
    }

    //


    m_camPass2Param.mdp_imgxo_p2_en = mdp_imgxo_p2_en;

#endif

    PIPE_DBG("mdp_imgxo_p2_en(0x%x)",mdp_imgxo_p2_en);

    //
    ret = m_CamPathPass2.config( &m_camPass2Param );
    if( ret != 0 )
    {
        PIPE_ERR("Pass 2 config error!");
        return MFALSE;
    }
    return  MTRUE;
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
        type = ISP_DRV_IRQ_TYPE_INT_P2_ST;
        irq = CAM_CTL_INT_P2_STATUS_PASS2_DON_ST;
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
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)//QQ
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
        case EPIPECmd_SET_NR3D_EN:
            m_Nr3dEn = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_NR3D_DMA_SEL:
            m_Nr3dDmaSel = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_CRZ_EN:
            m_CrzEn = arg1?MTRUE:MFALSE;
            break;
        case EPIPECmd_SET_JPEG_CFG:
            PIPE_DBG("setting jpeg configuration, soi_en:(%d), fQuality:(%d)\n",((JPGCfg*)arg1)->soi_en,((JPGCfg*)arg1)->fQuality);
            m_camPass2Param.jpgpara.soi_en = ((JPGCfg*)arg1)->soi_en;
            m_camPass2Param.jpgpara.fQuality = ((JPGCfg*)arg1)->fQuality;
            break;
        case EPIPECmd_SET_JPEG_WORKBUF_SIZE:
            jpg_WorkingMem_Size = ((MUINT32)arg1);
            break;
        case EPIPECmd_GET_NR3D_GAIN:
            //arg1: [In] software scenario
            //arg2: [In] magic#
            //arg3: [out] gain for nr3d
            *((MINT32*)arg3) = (MINT32) m_CamPathPass2.getNr3dGain((ESoftwareScenario)arg1, (MINT32)arg2);
            break;
        default:
            PIPE_ERR("NOT support command!");
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

/*******************************************************************************
*
********************************************************************************/
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

/*******************************************************************************
*
********************************************************************************/
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

////////////////////////////////////////////////////////////////////////////////
};  //namespace NSIspio
};  //namespace NSImageio

