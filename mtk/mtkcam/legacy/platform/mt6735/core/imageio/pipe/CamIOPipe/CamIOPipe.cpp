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
#define LOG_TAG "iio/camio"
//
//#define _LOG_TAG_LOCAL_DEFINED_
//#include <my_log.h>
//#undef  _LOG_TAG_LOCAL_DEFINED_
//
#include <mtkcam/Log.h>
//
#include "PipeImp.h"
#include "CamIOPipe.h"
//
#include <cutils/properties.h>  // For property_get().
#include <mtkcam/iopipe/CamIO/IHalCamIO.h> // for p1hwcfg module sel

/*******************************************************************************
*
********************************************************************************/
namespace NSImageio {
namespace NSIspio   {
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
        case EModule_SGG2   :
        case EModule_LCS    :
        case EModule_AAO    :
        case EModule_ESFKO  :
        case EModule_AFO    :
        case EModule_EISO   :
        case EModule_LCSO   :
        case EModule_iHDR   :
             return EPortIndex_TG1I;
             break;
        case EModule_AE_D   :
        case EModule_AWB_D  :
        case EModule_SGG1_D :
        case EModule_AF_D   :
        case EModule_LCS_D  :
        case EModule_AAO_D  :
        case EModule_AFO_D  :
        case EModule_LCSO_D :
        case EModule_iHDR_D :
             if (ePxlMode_Two_Twin == bTwin){
                  return EPortIndex_TG1I;
             }
             else {
                  return EPortIndex_TG2I;
             }
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
        case EModule_SGG1: newId = CAM_ISP_SGG1; break;
        case EModule_FLK: newId = CAM_ISP_FLK; break;
        case EModule_AF: newId = CAM_ISP_AF; break;
        case EModule_SGG2: newId = CAM_ISP_SGG2; break;
        case EModule_LCS: newId = CAM_ISP_LCS; break;
        case EModule_AAO: newId = CAM_DMA_AAO; break;
        case EModule_ESFKO: newId = CAM_DMA_ESFKO; break;
        case EModule_AFO: newId = CAM_DMA_AFO; break;
        case EModule_EISO: newId = CAM_DMA_EISO; break;
        case EModule_LCSO: newId = CAM_DMA_LCSO; break;
        case EModule_iHDR: newId = CAM_CTL_IHDR; break;
        case EModule_AE_D: newId = CAM_ISP_AE_D; break;
        case EModule_AWB_D: newId = CAM_ISP_AWB_D; break;
        case EModule_SGG1_D: newId = CAM_ISP_SGG1_D; break;
        case EModule_AF_D: newId = CAM_ISP_AF_D; break;
        case EModule_LCS_D: newId = CAM_ISP_LCS_D; break;
        case EModule_AAO_D: newId = CAM_DMA_AAO_D; break;
        case EModule_AFO_D: newId = CAM_DMA_AFO_D; break;
        case EModule_LCSO_D: newId = CAM_DMA_LCSO_D; break;
        case EModule_iHDR_D: newId = CAM_CTL_IHDR_D; break;
        case EModule_CAMSV_IMGO: newId = CAM_DMA_IMGO_SV; break;
        default:
            PIPE_ERR("Not support module(%d)",id);
            newId= -1;
            break;
    }
    return newId;
}

HWRWCTRL_P1SEL_MODULE
 p1ConvModuleToSelectID(MUINT32 id) {

   HWRWCTRL_P1SEL_MODULE newId = HWRWCTRL_P1SEL_RSVD;

    switch (id) {
        //raw
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG:    newId = HWRWCTRL_P1SEL_SGG; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN: newId = HWRWCTRL_P1SEL_SGG_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS:    newId = HWRWCTRL_P1SEL_LSC; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN: newId = HWRWCTRL_P1SEL_LSC_EN; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG:    newId = HWRWCTRL_P1SEL_IMG; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE:    newId = HWRWCTRL_P1SEL_UFE; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G:    newId = HWRWCTRL_P1SEL_W2G; break;

        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_D:    newId = HWRWCTRL_P1SEL_SGG_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN_D: newId = HWRWCTRL_P1SEL_SGG_EN_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_D:     newId = HWRWCTRL_P1SEL_LSC_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN_D:  newId = HWRWCTRL_P1SEL_LSC_EN_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG_D:     newId = HWRWCTRL_P1SEL_IMG_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE_D:     newId = HWRWCTRL_P1SEL_UFE_D; break;
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G_D:     newId = HWRWCTRL_P1SEL_W2G_D; break;

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
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G:
            return EPortIndex_TG1I;

        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_SGG_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_LCS_EN_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_IMG_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_UFE_D:
        case NSCam::NSIoPipe::NSCamIOPipe::EPipe_P1Sel_W2G_D:
            if (MTRUE == btwin){
                 return  EPortIndex_TG1I;
            }
            else {
                 return EPortIndex_TG2I;
            }

        default:
            CAM_LOGE("_p1ConvSelectIdToTg::NotSupportModule(%d). Use Enum EPipe_P1SEL",id);
            return  EPortIndex_UNKNOW;
    }
    return  EPortIndex_TG1I;
}



MINT32 getOutPxlByteNFmt( MBOOL bDupicate,MBOOL bCamSV,MUINT32 bRRZO, MUINT32 imgFmt, MINT32* pPixel_byte, MINT32* pFmt, MINT32* pEn_p1)
{
    MUINT32 _pak_en = 0;
    //
    if ( NULL == pPixel_byte ) {
        PIPE_ERR("ERROR:NULL pPixel_byte");
        return -1;
    }
    //
    //if (bRRZO && ( (eImgFmt_FG_BAYER8!=imgFmt)||(eImgFmt_FG_BAYER10!=imgFmt)||(eImgFmt_FG_BAYER12!=imgFmt) ) )
    if (bRRZO && ( (eImgFmt_FG_BAYER8!=imgFmt) && (eImgFmt_FG_BAYER10!=imgFmt) && (eImgFmt_FG_BAYER12!=imgFmt) ) )
    {
        PIPE_ERR("RRZO Not Support fmt(%d,%d)",imgFmt,imgFmt);
    }
    _pak_en = (bDupicate)?( (bCamSV)?(CAMSV_CAMSV2_MODULE_PAK_EN): \
         ((bRRZO)?(CAM_CTL_EN_P1_D_PAKG_D_EN):(CAM_CTL_EN_P1_D_PAK_D_EN)) ) : \
        ( (bCamSV)?(CAMSV_CAMSV_MODULE_PAK_EN): \
        ((bRRZO)?(CAM_CTL_EN_P1_PAKG_EN):(CAM_CTL_EN_P1_PAK_EN))  );
    //
    switch (imgFmt) {
        case eImgFmt_BAYER8:          //= 0x0001,   //Bayer format, 8-bit

        case eImgFmt_FG_BAYER8:
            *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW8):(CAM_FMT_SEL_BAYER8);
            *pEn_p1 |= _pak_en; //CAM_CTL_EN_P1_PAK_EN==CAM_CTL_EN_P1_D_PAK_D_EN
                                                                           //CAM_CTL_EN_P1_PAKG_EN==CAM_CTL_EN_P1_D_PAKG_D_EN
            break;
        case eImgFmt_BAYER10:         //= 0x0002,   //Bayer format, 10-bit
        case eImgFmt_FG_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW10):(CAM_FMT_SEL_BAYER10);
            *pEn_p1 |= _pak_en; //CAM_CTL_EN_P1_PAK_EN==CAM_CTL_EN_P1_D_PAK_D_EN
                                                                           //CAM_CTL_EN_P1_PAKG_EN==CAM_CTL_EN_P1_D_PAKG_D_EN

            break;
        case eImgFmt_BAYER12:         //= 0x0004,   //Bayer format, 12-bit
        case eImgFmt_FG_BAYER12:
            *pPixel_byte = (3 << CAM_ISP_PIXEL_BYTE_FP) >> 1; // 2 pixels-> 3 bytes, 1.5
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW12):(CAM_FMT_SEL_BAYER12);
            *pEn_p1 |= _pak_en; //CAM_CTL_EN_P1_PAK_EN==CAM_CTL_EN_P1_D_PAK_D_EN
                                                                           //CAM_CTL_EN_P1_PAKG_EN==CAM_CTL_EN_P1_D_PAKG_D_EN

            break;
        case eImgFmt_BAYER14:         //= 0x0008,   //Bayer format, 14-bit
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW14):(CAM_FMT_SEL_BAYER14);
            *pEn_p1 |= _pak_en; //CAM_CTL_EN_P1_PAK_EN==CAM_CTL_EN_P1_D_PAK_D_EN
                                                                           //CAM_CTL_EN_P1_PAKG_EN==CAM_CTL_EN_P1_D_PAKG_D_EN
            break;
        case eImgFmt_UFO_BAYER10:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            if(bCamSV){
                PIPE_ERR("camsv support no ufo");
                *pFmt = CAMSV_FMT_SEL_TG_FMT_RAW10;
            }
            else {
                *pFmt = CAM_FMT_SEL_TG_FMT_RAW10;
            }
            *pEn_p1 |= CAM_CTL_EN_P1_UFE_EN;
            //CAM_IMGI_SLOW_DOWN.UFO_IMGO_EN = 1
            break;
        case eImgFmt_YUY2:            //= 0x0100,   //422 format, 1 plane (YUYV)
        case eImgFmt_UYVY:            //= 0x0200,   //422 format, 1 plane (UYVY)
        case eImgFmt_YVYU:            //= 0x080000,   //422 format, 1 plane (YVYU)
        case eImgFmt_VYUY:            //= 0x100000,   //422 format, 1 plane (VYUY)
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_YUV422):(CAM_FMT_SEL_YUV422_1P);
            break;
        case eImgFmt_RGB565:
            *pPixel_byte = 2 << CAM_ISP_PIXEL_BYTE_FP;
            if(bCamSV){
                PIPE_ERR("camsv support no RGB565");
                *pFmt = CAMSV_FMT_SEL_TG_FMT_RAW10;
            }
            else {
                *pFmt = CAM_FMT_SEL_RGB565;
            }
            break;
        case eImgFmt_RGB888:
            *pPixel_byte = 3 << CAM_ISP_PIXEL_BYTE_FP;
            if(bCamSV){
                PIPE_ERR("camsv support no RGB565");
                *pFmt = CAMSV_FMT_SEL_TG_FMT_RAW10;
            }
            else {
                *pFmt = CAM_FMT_SEL_RGB888;
            }
            break;
        case eImgFmt_JPEG:
            if(bCamSV){
                *pPixel_byte = 1 << CAM_ISP_PIXEL_BYTE_FP;
                *pFmt = CAMSV_FMT_SEL_TG_FMT_JPEG;
            }
            else{
                PIPE_ERR("JPG fmt NOT Support in p1\n",imgFmt);
            }
            break;
        case eImgFmt_BLOB:
            *pPixel_byte = (5 << CAM_ISP_PIXEL_BYTE_FP) >> 2; // 4 pixels-> 5 bytes, 1.25
            *pFmt = (bCamSV)?(CAMSV_FMT_SEL_TG_FMT_RAW10):(CAM_FMT_SEL_BAYER10);
            break;
        default:
            PIPE_ERR("eImgFmt:[%d]NOT Support",imgFmt);
            return -1;
    }
    //
    PIPE_INF("getOutPxlByteNFmt - imgFmt(0x%x),*pPixel_byte(%d)",imgFmt,*pPixel_byte);
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
            *scenarioFmt = eScenarioFmt_RGB;
            break;

        case eImgFmt_JPEG:
            *scenarioFmt = eScenarioFmt_JPG;
            break;
        default:
            PIPE_ERR("cvtImgFmt2ScenarioFmt: eImgFmt:[%d]NOT Support",imgFmt);
            *scenarioFmt = eScenarioFmt_RAW;
            return -1;
    }
    return 0;
}




MINT32 getTwoPxlInfo(EPxlMode pxlMode,EImageFormat fmt, MINT32* bTwin, MINT32* two_pxl, MINT32* en_p1, MINT32* two_pxl_dmx)
{
    PIPE_INF("PxlMode=0x%x, fmt=0x%x\n", pxlMode, fmt);
    switch (pxlMode) {
        case ePxlMode_One_Single:
            break;
        case ePxlMode_One_Twin:
            *bTwin = 1;
            *en_p1 |= (CAM_CTL_EN_P1_DMX_EN|CAM_CTL_EN_P1_RMX_EN|CAM_CTL_EN_P1_BMX_EN);
            break;
        case ePxlMode_Two_Single:
            if (eImgFmt_YUY2==fmt || eImgFmt_YVYU==fmt || eImgFmt_VYUY==fmt || eImgFmt_UYVY==fmt) {
               *en_p1 |= CAM_CTL_EN_P1_WBN_EN; //CAM_CTL_EN_P1_D_WBN_D_EN == CAM_CTL_EN_P1_WBN_EN
               PIPE_INF("YUV sensor, Two_Single\n");
            }
            else
                PIPE_INF("RAW sensor, Two_Single\n");

            *bTwin = 0;
            *two_pxl = 1;
            *two_pxl_dmx = 1;
            break;
        case ePxlMode_Two_Twin:
            if (eImgFmt_YUY2==fmt || eImgFmt_YVYU==fmt || eImgFmt_VYUY==fmt || eImgFmt_UYVY==fmt) {
                PIPE_ERR("ERROR:Not support YUV+2-pixel");
                return -1;
            }
            *bTwin = 1;
            *two_pxl = 1;
            *en_p1 |= (CAM_CTL_EN_P1_DMX_EN|CAM_CTL_EN_P1_RMX_EN|CAM_CTL_EN_P1_BMX_EN);
            break;
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
      m_pass1_RawD_CQ(CAM_ISP_CQ_NONE),
      m_pass1TwinMode(0),
      mPath1(0)
{
    //
    DBG_LOG_CONFIG(imageio, pipe);
    //
    PIPE_INF(":E");
    //
    m_pass1_Raw_CQ  = 0xFFFF;
    m_pass1_RawD_CQ = 0xFFFF;

    std::strcpy((char*)m_szUsrName,(char const*)szUsrName);
    PIPE_INF("usr(%s)",m_szUsrName);
    std::memset(&this->m_camPass1Param,0x00,sizeof(CamPathPass1Parameter));
    this->m_vBufImgo.resize(1);
    this->m_vBufImg2o.resize(1);
    /*** create isp driver ***/
    m_pIspDrvShell = IspDrvShell::createInstance();
    //
    m_rrz_update_cnt = 0;
    m_rrzd_update_cnt = 0;

    mMagicNum0 = 0xFFFF; //RRZO
    mMagicNum1 = 0xFFFE; //3A
    mMagicNum2 = 0xFFFE; //IMGO
    mMagicNum0_D = 0xFFFF; //RRZO
    mMagicNum1_D = 0xFFFE; //3A
    mMagicNum2_D = 0xFFFE; //IMGO
    mOpenedPort = 0x0;
    m_bBypassImgo = MFALSE;
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
#if 0
        m_pIspDrvShell->getPhyIspDrv()->pipeCountInc();
        //clear TOP enable register at every time pass1 init.
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_EN_P1, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_EN_P1_DMA, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_EN_P1_D, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_EN_P1_DMA_D, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_FMT_SEL_P1, 0,ISP_DRV_USER_ISPF);
        ISP_WRITE_REG(m_pIspDrvShell->getPhyIspDrv(),CAM_CTL_FMT_SEL_P1_D, 0,ISP_DRV_USER_ISPF);
        //
#endif
    }
#if 0
    ////////////////////////////////////////////////////////////////////////////////
    ///--adding clear buf information at the beginnning to avoid some problems
    ISP_BUFFER_CTRL_STRUCT buf_ctrl;
    MUINT32 dummy;
    int rt_dma;
    //imgo
    rt_dma=_imgo_;
    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
    buf_ctrl.data_ptr = (MUINT32)&dummy;
    if ( MTRUE != m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
    {
       PIPE_ERR("ERROR:rtBufCtrl_clear imgo fail");
       return -1;
    }
    //img2o
    rt_dma=_rrzo_;
    buf_ctrl.ctrl = ISP_RT_BUF_CTRL_CLEAR;
    buf_ctrl.buf_id = (_isp_dma_enum_)rt_dma;
    buf_ctrl.data_ptr = (MUINT32)&dummy;
    if ( MTRUE != m_pIspDrvShell->m_pPhyIspDrv_bak->rtBufCtrl((void*)&buf_ctrl) )
    {
       PIPE_ERR("ERROR:rtBufCtrl_clear img2o fail");
       return -1;
    }
    ///////////////////////////////////////////////////////////////////////////////////
#endif

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
    m_pIspDrvShell->uninit((char*)this->m_szUsrName);

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
    if (( CAM_ISP_CQ_NONE != m_pass1_Raw_CQ ) || ( CAM_ISP_CQ_NONE != m_pass1_RawD_CQ )) {
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
        else if(this->m_pass1_RawD_CQ != CAM_ISP_CQ_NONE){
            if(1 != m_CamPathPass1.setP1Notify()){
            }
            //flush eis setting into phy address
            //enforce cq load into reg before VF_EN if update rrz/tuning is needed
            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
            startCQ0_D();//for rrz configframe before start
            usleep(2000);//wait for cmdQ flush into hw reg
            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
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
    if (( CAM_ISP_CQ0 != m_pass1_Raw_CQ ) && ( CAM_ISP_CQ0 != m_pass1_RawD_CQ )) {
        PIPE_ERR("startCQ0 Invalid CQ val(%d,%d)",m_pass1_Raw_CQ,m_pass1_RawD_CQ);
        return MTRUE;
    }

    path  = CAM_ISP_PASS1_CQ0_START;
    m_CamPathPass1.CQ = CAM_ISP_CQ0;
    m_CamPathPass1.start((void*)&path);

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
startCQ0_D()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_INF(":E");
    //
    if (( CAM_ISP_CQ0_D != m_pass1_Raw_CQ ) && ( CAM_ISP_CQ0_D != m_pass1_RawD_CQ )) {
        PIPE_ERR("startCQ0_D Invalid CQ val");
        return MTRUE;
    }

    path  = CAM_ISP_PASS1_CQ0_D_START;
    m_CamPathPass1.CQ_D = CAM_ISP_CQ0_D;
    m_CamPathPass1.start((void*)&path);

    return  MTRUE;
}

/*******************************************************************************
*
********************************************************************************/

MBOOL
CamIOPipe::
startCQ0B_D()
{
int path  = CAM_ISP_PASS1_START;

    PIPE_INF(":E");
    //
    path  = CAM_ISP_PASS1_CQ0B_D_START;
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
   if (( CAM_ISP_CQ_NONE != m_pass1_Raw_CQ ) || ( CAM_ISP_CQ_NONE != m_pass1_RawD_CQ )) {
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
    if (EPortIndex_IMGO == portID.index) {dmaChannel = ISP_DMA_IMGO; }
    else if (EPortIndex_UFEO == portID.index) { dmaChannel = ISP_DMA_UFEO; }
    else if (EPortIndex_RRZO == portID.index) { dmaChannel = ISP_DMA_RRZO; }
    else if (EPortIndex_IMGO_D == portID.index) { dmaChannel = ISP_DMA_IMGO_D; }
    else if (EPortIndex_RRZO_D == portID.index) { dmaChannel = ISP_DMA_RRZO_D; }
    else if (EPortIndex_CAMSV_IMGO == portID.index) { dmaChannel = ISP_DMA_CAMSV_IMGO; }
    else if (EPortIndex_CAMSV2_IMGO == portID.index) { dmaChannel = ISP_DMA_CAMSV2_IMGO; }
    //enque buffer
    bufInfo.memID       = rQBufInfo.vBufInfo[0].memID[0];
    bufInfo.size        = rQBufInfo.vBufInfo[0].u4BufSize[0];
    bufInfo.base_vAddr  = rQBufInfo.vBufInfo[0].u4BufVA[0];
    bufInfo.base_pAddr  = rQBufInfo.vBufInfo[0].u4BufPA[0];
    bufInfo.bufSecu     = rQBufInfo.vBufInfo[0].bufSecu[0];
    bufInfo.bufCohe     = rQBufInfo.vBufInfo[0].bufCohe[0];
    bufInfo.mBufIdx     = rQBufInfo.vBufInfo[0].mBufIdx;//used when replace buffer
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
    if (EPortIndex_IMGO == portID.index) {dmaChannel = ISP_DMA_IMGO; }
    else if (EPortIndex_UFEO == portID.index) { dmaChannel = ISP_DMA_UFEO; }
    else if (EPortIndex_RRZO == portID.index) { dmaChannel = ISP_DMA_RRZO; }
    else if (EPortIndex_IMGO_D == portID.index) { dmaChannel = ISP_DMA_IMGO_D; }
    else if (EPortIndex_RRZO_D == portID.index) { dmaChannel = ISP_DMA_RRZO_D; }
    else if (EPortIndex_CAMSV_IMGO == portID.index) { dmaChannel = ISP_DMA_CAMSV_IMGO; }
    else if (EPortIndex_CAMSV2_IMGO == portID.index) { dmaChannel = ISP_DMA_CAMSV2_IMGO; }
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
        rQBufInfo.vBufInfo[i].m_num_1           = bufList.front().m_num_1;
        rQBufInfo.vBufInfo[i].frm_cnt           = bufList.front().frm_cnt;
        rQBufInfo.vBufInfo[i].raw_type          = bufList.front().raw_type;
        rQBufInfo.vBufInfo[i].header            = bufList.front().header_info;
        rQBufInfo.vBufInfo[i].headerSize        = bufList.front().header_size;
        if((dmaChannel == ISP_DMA_RRZO) || (dmaChannel == ISP_DMA_RRZO_D)){
            rQBufInfo.vBufInfo[i].crop_win.p.x      = bufList.front().rrz_info.srcX;
            rQBufInfo.vBufInfo[i].crop_win.p.y      = bufList.front().rrz_info.srcY;
            rQBufInfo.vBufInfo[i].crop_win.s.w      = bufList.front().rrz_info.srcW;
            rQBufInfo.vBufInfo[i].crop_win.s.h      = bufList.front().rrz_info.srcH;
            rQBufInfo.vBufInfo[i].DstSize.w         = bufList.front().rrz_info.dstW;
            rQBufInfo.vBufInfo[i].DstSize.h         = bufList.front().rrz_info.dstH;
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

        {
          stISP_BUF_INFO *ptr = (stISP_BUF_INFO*)(rQBufInfo.vBufInfo[i].header);
          PIPE_DBG("rrzInfo[%d,%d,%d,%d,%d,%d]", \
                    ptr->rrz_info.srcX,ptr->rrz_info.srcY,ptr->rrz_info.srcW,\
                    ptr->rrz_info.srcH,ptr->rrz_info.dstW,ptr->rrz_info.dstH);
        }
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
/*******************************************************************************
* general
********************************************************************************/
    int path = ISP_PASS_NONE;
    int pathD = ISP_PASS_NONE;

int scenario = ISP_SCENARIO_MAX;
int subMode  = ISP_SUB_MODE_MAX;
    int subModeD = ISP_SUB_MODE_MAX;

int continuous = 0;
int D_continuous = 0;

int cam_out_fmt = CAM_FMT_SEL_YUV422_1P;
int pakG_fmt = CAM_FMT_SEL_TG_FMT_RAW10;

int camD_out_fmt = CAM_FMT_SEL_YUV422_1P;
int D_pakG_fmt = CAM_FMT_SEL_TG_FMT_RAW10;

int two_pxl = 0;
int two_pxl_dmx = 0;

int pass1_CQ = m_pass1_Raw_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//
    int pass1_CQ_D = m_pass1_RawD_CQ;//CAM_ISP_CQ0; //CAM_ISP_CQ_NONE;//

int pass1_cq_en = 0;
int pass1_twin = 0;
int pass1D_twin = 0;


/*******************************************************************************
* tg(raw)
********************************************************************************/
int idx_tgi = -1;
int idx_tg2i = -1;
unsigned int tgInFmt = 0;
unsigned int tg2InFmt = 0;

int idx_imgo = -1;
int idx_rrzo = -1;
int pix_id_tg = -1;
int pixel_byte_imgo = -1;
int pixel_byte_rrzo = -1;

int en_p1 = 0;
int en_p1_dma = 0;
int int_p1_en = CAM_CTL_INT_P1_EN_VS1_INT_EN | \
                CAM_CTL_INT_P1_EN_PASS1_DON_EN | \
                CAM_CTL_INT_P1_EN_SOF1_INT_EN | \
               CAM_CTL_INT_P1_EN_CQ0_VS_ERR_EN|\
                CAM_CTL_INT_P1_EN_AF_DON_EN;


int int_p1_en2 = 0;
int ctl_sel = 0;

PortInfo portInfo_tgi;
PortInfo portInfo_imgo;
PortInfo portInfo_rrzo;

/*******************************************************************************
* tg_d(raw_d)
********************************************************************************/
//int idx_tgi_d = -1;
int idx_imgo_d = -1;
int idx_rrzo_d = -1;
int pix_id_tg_d = -1;
int pixel_byte_imgo_d = 1;
int pixel_byte_rrzo_d = -1;

int en_p1_d = 0;
int en_p1_dma_d = 0;
int int_p1_en_d=CAM_CTL_INT_P1_EN_D_VS1_INT_EN | \
                CAM_CTL_INT_P1_EN_D_PASS1_DON_EN | \
                CAM_CTL_INT_P1_EN_D_SOF1_INT_EN| \
                CAM_CTL_INT_P1_EN_D_AF_DON_EN ;
int int_p1_en2_d = 0;
int ctl_sel_d = 0;



PortInfo portInfo_tgi_d;
PortInfo portInfo_imgo_d;
PortInfo portInfo_rrzo_d;
/*******************************************************************************
* camsv(none)
********************************************************************************/
int int_camsv_en = CAMSV_CAMSV_INT_EN_VS1_INT_EN | \
                   CAMSV_CAMSV_INT_EN_PASS1_DON_INT_EN | \
                   CAMSV_CAMSV_INT_EN_TG_SOF_INT_EN;
int idx_camsv_tgi = -1;
int idx_camsv_imgo = -1;
int en_camsv = CAMSV_CAMSV_MODULE_DB_EN;
int pixel_byte_camsv_imgo = -1;
int camsv_fmt = -1;
int camsv_imgofmt = -1;
int camsv_imgo_bus = 3;
MBOOL bCamsv_2pix = MFALSE;
MBOOL camsv_paksel = 1;
//int camsv_tg_fmt = -1;
//int camsv_tg_sw = -1;
PortInfo portInfo_camsv_tgi;
PortInfo portInfo_camsv_imgo;
/*******************************************************************************
* camsv_d(none)
********************************************************************************/
int int_camsv2_en = CAMSV_CAMSV2_INT_EN_VS1_INT_EN | \
                    CAMSV_CAMSV2_INT_EN_PASS1_DON_INT_EN | \
                    CAMSV_CAMSV2_INT_EN_TG_SOF_INT_EN;
int idx_camsv2_tgi = -1;
int idx_camsv2_imgo = -1;
int en_camsv2 = CAMSV_CAMSV2_MODULE_DB_EN;
int pixel_byte_camsv2_imgo = -1;
int camsv2_fmt = -1;
int camsv2_imgofmt = -1;
int camsv2_imgo_bus = 3;
MUINT32 tgFps = 30;
MBOOL bCamsv2_2pix = MFALSE;
MBOOL camsv2_paksel = 1;
//int camsv2_tg_fmt = -1;
//int camsv2_tg_sw = -1;

PortInfo portInfo_camsv2_tgi;
PortInfo portInfo_camsv2_imgo;
PIPE_DBG("+, CQ:%d/%d",m_pass1_Raw_CQ, m_pass1_RawD_CQ);

    if (0 == vOutPorts.size()) {
        PIPE_ERR("0 == vOutPorts.size()");
        return MFALSE;
    }

    //Raw
    if ( CAM_ISP_CQ_NONE != pass1_CQ) {
        if ( CAM_ISP_CQ0  == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0_EN;}
        if ( CAM_ISP_CQ0B == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0B_EN;}
        if ( CAM_ISP_CQ0C == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0C_EN;}
        if ( CAM_ISP_CQ0_D  == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0_D_EN;}
        if ( CAM_ISP_CQ0B_D == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0B_D_EN;}
        if ( CAM_ISP_CQ0C_D == pass1_CQ ) { pass1_cq_en = CAM_CTL_CQ_EN_CQ0C_D_EN;}
    }

    //RawD
    if ( CAM_ISP_CQ_NONE != pass1_CQ_D) {
        if ( CAM_ISP_CQ0  == pass1_CQ_D ) { pass1_cq_en   |= CAM_CTL_CQ_EN_CQ0_EN;}
        if ( CAM_ISP_CQ0B == pass1_CQ_D ) { pass1_cq_en   |= CAM_CTL_CQ_EN_CQ0B_EN;}
        if ( CAM_ISP_CQ0C == pass1_CQ_D ) { pass1_cq_en   |= CAM_CTL_CQ_EN_CQ0C_EN;}
        if ( CAM_ISP_CQ0_D  == pass1_CQ_D ) { pass1_cq_en |= CAM_CTL_CQ_EN_CQ0_D_EN;}
        if ( CAM_ISP_CQ0B_D == pass1_CQ_D ) { pass1_cq_en |= CAM_CTL_CQ_EN_CQ0B_D_EN;}
        if ( CAM_ISP_CQ0C_D == pass1_CQ_D ) { pass1_cq_en |= CAM_CTL_CQ_EN_CQ0C_D_EN;}
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
                mPath1 |= path = ISP_PASS1;
                idx_tgi = i;
                pix_id_tg = vInPorts[i]->eRawPxlID;
                portInfo_tgi = (PortInfo)*vInPorts[i];
                en_p1 |= CAM_CTL_EN_P1_TG1_EN;
                if (0!=getTwoPxlInfo(vInPorts[i]->ePxlMode, vInPorts[i]->eImgFmt, (MINT32*) &pass1_twin, (MINT32*)&two_pxl, (MINT32*)&en_p1, (MINT32*)&two_pxl_dmx )) {
                    PIPE_ERR("ERROR:in param");
                    return MFALSE;
                }
                tgInFmt = cvtTgFmtToHwVal(vInPorts[i]->eImgFmt);
                m_P1HWRWctrl[EPortIndex_TG1I].init((ISP_DRV_CQ_ENUM)pass1_CQ, \
                                     (MUINT32*)this->m_pIspDrvShell);
                tgFps = vInPorts[i]->tgFps;
                //PIPE_DBG("EPortIndex_TG1I-");
                break;
            case EPortIndex_TG2I:
                path = ISP_PASS_NONE;
                mPath1 |= pathD = ISP_PASS1_D;
                idx_tg2i = i;
                pix_id_tg_d = vInPorts[i]->eRawPxlID;
                portInfo_tgi_d = (PortInfo)*vInPorts[i];
                en_p1_d |= CAM_CTL_EN_P1_D_TG1_D_EN;
                if (0!=getTwoPxlInfo(vInPorts[i]->ePxlMode, vInPorts[i]->eImgFmt, (MINT32*) &pass1D_twin, (MINT32*)&two_pxl, (MINT32*)&en_p1_d, (MINT32*)&two_pxl_dmx )) {
                    PIPE_ERR("ERROR:in param");
                    return MFALSE;
                }
                tg2InFmt = cvtTgFmtToHwVal(vInPorts[i]->eImgFmt);
                if ( pass1D_twin ) { PIPE_ERR("TG2 Not Support Twin"); }
                //CAM_LOGD("m_P1HWRWctrl[2].init ");
                m_P1HWRWctrl[EPortIndex_TG2I].init((ISP_DRV_CQ_ENUM)pass1_CQ_D, \
                                 (MUINT32*)this->m_pIspDrvShell);
                //PIPE_DBG("EPortIndex_TG2I-");
                break;
            case EPortIndex_CAMSV_TG1I:
                path = ISP_PASS1_CAMSV;
                idx_camsv_tgi = i;
                portInfo_camsv_tgi = (PortInfo)*vInPorts[i];
                en_camsv |= CAMSV_CAMSV_MODULE_TG_EN;
                switch(vInPorts[i]->ePxlMode) {
                    case ePxlMode_One_Single:   bCamsv_2pix = MFALSE;
                         break;
                    case ePxlMode_Two_Single:   bCamsv_2pix = MTRUE;
                         break;
                    default:
                         PIPE_ERR("camsv Not Support Twin");
                         break;
                }
                break;
            case EPortIndex_CAMSV_TG2I:
                pathD = ISP_PASS1_CAMSV_D;
                idx_camsv2_tgi = i;
                portInfo_camsv2_tgi = (PortInfo)*vInPorts[i];
                en_camsv2 |= CAMSV_CAMSV2_MODULE_TG_EN;
                switch(vInPorts[i]->ePxlMode) {
                      case ePxlMode_One_Single: bCamsv2_2pix = MFALSE;
                           break;
                      case ePxlMode_Two_Single: bCamsv2_2pix = MTRUE;
                           break;
                      default:
                           PIPE_ERR("camsv2 Not Support Twin");
                           break;
                }
                break;
            default:
                PIPE_ERR("Not supported TG Index ERR!!!!!!!!!!!!!!!!!!!!!!!!!!");
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
            en_p1_dma |= CAM_CTL_EN_P1_DMA_IMGO_EN;
            mOpenedPort |= CAM_CTL_EN_P1_DMA_IMGO_EN;
            portInfo_imgo =  (PortInfo)*vOutPorts[idx_imgo];
            //PIPE_DBG("EPortIndex_IMGO: idx_imgo=%d, en_p1_dma=%x, en_p1=%x, portInfo_imgo=%x, eImgFmt=%d", idx_imgo, en_p1_dma, en_p1, portInfo_imgo, (MUINT32)vOutPorts[i]->eImgFmt);
            getOutPxlByteNFmt(0,0,0, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo, (MINT32*)&cam_out_fmt, (MINT32*)&en_p1 );
            if(1 == vOutPorts[i]->u4PureRaw){
                if(1 == vOutPorts[i]->u4PureRawPak) {
                    ctl_sel |= (CAM_CTL_SEL_P1_IMG_SEL);
                }else{
                    ctl_sel |= CAM_CTL_SEL_P1_UFE_SEL2;
                    if( (1 << CAM_ISP_PIXEL_BYTE_FP) !=  pixel_byte_imgo) {
                        pixel_byte_imgo= (2 << CAM_ISP_PIXEL_BYTE_FP);
                    }
                }
            }
        }
        else if ( EPortIndex_RRZO == vOutPorts[i]->index ) {
            idx_rrzo = i;
            en_p1_dma |= CAM_CTL_EN_P1_DMA_RRZO_EN;
            en_p1     |= CAM_CTL_EN_P1_RRZ_EN;
            mOpenedPort |= CAM_CTL_EN_P1_DMA_RRZO_EN;
            portInfo_rrzo =  (PortInfo)*vOutPorts[idx_rrzo];
            getOutPxlByteNFmt(0,0,1, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo, (MINT32*)&pakG_fmt, (MINT32*)&en_p1 );
            //PIPE_DBG("EPortIndex_RRZO: idx_rrzo=%d, en_p1_dma=%x, en_p1=%x, portInfo_rrzo=%x, eImgFmt=%d", idx_rrzo, en_p1_dma, en_p1, portInfo_rrzo, (MUINT32)vOutPorts[i]->eImgFmt);
            //if ( (0 == pass1_twin) && (CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL < vOutPorts[i]->u4ImgWidth) ) {
            //    PIPE_ERR("rrzo out width exceed (%d > %d)",vOutPorts[i]->u4ImgWidth,CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL);
            //    PIPE_ERR("ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!ERR!!");
            //    return MFALSE;
            //}
        }
        else if ( EPortIndex_IMGO_D == vOutPorts[i]->index ) {
            idx_imgo_d = i;
            en_p1_dma_d |= CAM_CTL_EN_P1_DMA_D_IMGO_D_EN;
            mOpenedPort |= CAM_CTL_EN_P1_DMA_IMGO_EN;//this para. can use imgo to stand for imgo_d, in order to simply marco:_CmdQSwitchEn
            portInfo_imgo_d =  (PortInfo)*vOutPorts[idx_imgo_d];

            if ( eImgFmt_UFO_BAYER10 == vOutPorts[i]->eImgFmt ) {
                PIPE_ERR("NOT Support UFO format for imgo_d");
                return MFALSE;
            }
            getOutPxlByteNFmt(1,0,0, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_imgo_d, (MINT32*)&camD_out_fmt, (MINT32*)&en_p1_d );
            if(1 == vOutPorts[i]->u4PureRaw){
                if(1 == vOutPorts[i]->u4PureRawPak) {
                    ctl_sel_d |= (CAM_CTL_SEL_P1_D_IMG_SEL_D);
                }else{
                    ctl_sel_d |= CAM_CTL_SEL_P1_D_UFE_SEL2_D;
                    if( (1 << CAM_ISP_PIXEL_BYTE_FP) !=  pixel_byte_imgo_d) {
                        pixel_byte_imgo_d= (2 << CAM_ISP_PIXEL_BYTE_FP);
                    }
                }
            }
        }
        else if ( EPortIndex_RRZO_D == vOutPorts[i]->index ) {
            idx_rrzo_d = i;
            en_p1_dma_d |= CAM_CTL_EN_P1_DMA_D_RRZO_D_EN;
            en_p1_d     |= CAM_CTL_EN_P1_D_RRZ_D_EN;
            mOpenedPort |= CAM_CTL_EN_P1_DMA_RRZO_EN;//this para. can use rrzo to stand for rrzo_d,in order to simply marco:_CmdQSwitchEn
            portInfo_rrzo_d =  (PortInfo)*vOutPorts[idx_rrzo_d];
            getOutPxlByteNFmt(1,0,1, (MUINT32)vOutPorts[i]->eImgFmt, (MINT32*)&pixel_byte_rrzo_d, (MINT32*)&D_pakG_fmt, (MINT32*)&en_p1_d );
            //if ( (0 == pass1_twin) && (CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL < vOutPorts[i]->u4ImgWidth) ) {
            //    PIPE_ERR("rrzo_d out width exceed (%d > %d)",vOutPorts[i]->u4ImgWidth,CAM_RRZ_MAX_LINE_BUFFER_IN_PIXEL);
            //    return MFALSE;
            //}
        }
        else if (EPortIndex_CAMSV_IMGO == vOutPorts[i]->index) {
            en_camsv |= CAMSV_CAMSV_MODULE_TG_EN;
            en_camsv |= CAMSV_CAMSV_MODULE_PAK_EN;
            en_camsv |= CAMSV_CAMSV_MODULE_DB_EN;
            bCamsv_2pix = MFALSE;

            idx_camsv_imgo = i;
            en_camsv |= CAMSV_CAMSV_MODULE_IMGO_EN;
            portInfo_camsv_imgo = (PortInfo)*vOutPorts[idx_camsv_imgo];

            getOutPxlByteNFmt(0,1,0, (MUINT32)vOutPorts[i]->eImgFmt, \
                (MINT32*)&pixel_byte_camsv_imgo, (MINT32*)&camsv_fmt, (MINT32*)&en_camsv );
            if(1 == vOutPorts[i]->u4PureRaw){
                if(0 == vOutPorts[i]->u4PureRawPak) {
                    en_camsv |= CAMSV_CAMSV_MODULE_PAK_SEL;
                    pixel_byte_camsv_imgo= (2 << CAM_ISP_PIXEL_BYTE_FP);
                }
            }
        }
        else if (EPortIndex_CAMSV2_IMGO == vOutPorts[i]->index) {
            idx_camsv2_imgo = i;
            en_camsv2 |= CAMSV_CAMSV2_MODULE_IMGO_EN;
            portInfo_camsv2_imgo = (PortInfo)*vOutPorts[idx_camsv2_imgo];
            getOutPxlByteNFmt(0,1,0, (MUINT32)vOutPorts[i]->eImgFmt, \
                (MINT32*)&pixel_byte_camsv2_imgo, (MINT32*)&camsv2_fmt, (MINT32*)&en_camsv2 );
            if(1 == vOutPorts[i]->u4PureRaw){
                if(0 == vOutPorts[i]->u4PureRawPak) {
                    en_camsv2 |= CAMSV_CAMSV2_MODULE_PAK_SEL;
                    pixel_byte_camsv2_imgo= (2 << CAM_ISP_PIXEL_BYTE_FP);
                }
            }
        }

    }
    //check configuration validity
    //

    //
    for (MUINT32 i = 0 ; i < vInPorts.size() ; i++ ) {

         MINT32 scenarioFmt = eScenarioFmt_RAW;
         if (0 != cvtImgFmt2ScenarioFmt( (MUINT32)vInPorts[i]->eImgFmt, &scenarioFmt)) {
             PIPE_ERR("Invalid vInPorts[%d]->eImgFmt(%d) ", i, vInPorts[i]->eImgFmt);
         }
         PIPE_INF("i(%d),scenarioFmt(%d),index(%d)",i,scenarioFmt,vInPorts[i]->index);

         switch (scenarioFmt) /*(meScenarioFmt)*/ {
             case eScenarioFmt_RAW:
                 if (EPortIndex_TG1I == vInPorts[i]->index) {
                     subMode = ISP_SUB_MODE_RAW;
                 } else if (EPortIndex_TG2I == vInPorts[i]->index)  {
                     subMode = ISP_SUB_MODE_RAW; // p2 driver need to get sub mode from p1 driver
                     subModeD = ISP_SUB_MODE_RAW;
                 }
                 camsv_imgofmt = camsv2_imgofmt = CAMSV_FMT_SEL_IMGO_OTHER;
                 camsv_imgo_bus = (MTRUE == bCamsv_2pix)? (3):(1);
                 camsv2_imgo_bus = (MTRUE == bCamsv2_2pix)? (3):(1);

#if 0//isp can IMGO only
                 //rrz must be enabled if ues raw(rrz_d must be enabled if use raw_d)
                 if(en_p1 & CAM_CTL_EN_P1_TG1_EN) {
                     if( (en_p1 & CAM_CTL_EN_P1_RRZ_EN) == 0) {
                         PIPE_ERR("rrz(rrzo) must be enabled when use RAW\n");
                         return MFALSE;
                     }
                 }
                 if(en_p1_d & CAM_CTL_EN_P1_D_TG1_D_EN) {
                     if( (en_p1_d & CAM_CTL_EN_P1_D_RRZ_D_EN) == 0) {
                         PIPE_ERR("rrz_d(rrzo_d) must be enabled when use RAW_D\n");
                         return MFALSE;
                     }
                 }
#endif
                 break;
             case eScenarioFmt_YUV:

                 if (EPortIndex_TG1I == vInPorts[i]->index) {
                     subMode = ISP_SUB_MODE_YUV;
                 }else if (EPortIndex_TG2I == vInPorts[i]->index)  {
                     subMode = ISP_SUB_MODE_YUV; // p2 driver need to get sub mode from p1 driver
                     subModeD = ISP_SUB_MODE_YUV;
                 }

                 if ( (ISP_PASS1 == path) || (ISP_PASS1 == pathD) ) {
                     ctl_sel |= CAM_CTL_SEL_P1_UFE_SEL2;
                 }
                 else if ( (ISP_PASS1_D == path) || (ISP_PASS1_D == pathD) ) {
                     ctl_sel_d |= CAM_CTL_SEL_P1_D_UFE_SEL2_D;
                 }
                 camsv_imgofmt = camsv2_imgofmt = CAMSV_FMT_SEL_IMGO_YUV;

                 if(en_camsv & CAMSV_CAMSV_MODULE_TG_EN) {
                      if(MTRUE == bCamsv_2pix) {
                        PIPE_INF("sv1 yuv fmt support only 1pix mode\n");
                        bCamsv_2pix = MFALSE;
                      }
                      en_camsv |= CAMSV_CAMSV_MODULE_PAK_SEL;
                 }
                 if(en_camsv2 & CAMSV_CAMSV2_MODULE_TG_EN){
                      if(MTRUE == bCamsv2_2pix) {
                        PIPE_INF("sv2 yuv fmt support only 1pix mode\n");
                        bCamsv2_2pix = MFALSE;
                      }
                      en_camsv2 |= CAMSV_CAMSV2_MODULE_PAK_SEL;
                 }
                 camsv_imgo_bus = 0;
                 camsv2_imgo_bus = 0;
                 break;
             case  eScenarioFmt_JPG:
                 if (EPortIndex_TG1I == vInPorts[i]->index) {
                     subMode = ISP_SUB_MODE_JPG;
                 }else if (EPortIndex_TG2I == vInPorts[i]->index)  {
                     subMode = ISP_SUB_MODE_JPG; // p2 driver need to get sub mode from p1 driver
                     subModeD = ISP_SUB_MODE_JPG;
                 }
                 camsv_imgofmt = camsv2_imgofmt = CAMSV_FMT_SEL_IMGO_JPG;
                 if(en_camsv & CAMSV_CAMSV_MODULE_TG_EN) {
                      if(MFALSE == bCamsv_2pix) {
                        PIPE_INF("sv1 jpg fmt must be in 2pix mode\n");
                        bCamsv_2pix = MTRUE;
                      }
                      en_camsv |= CAMSV_CAMSV_MODULE_PAK_SEL;
                 }
                 if(en_camsv2 & CAMSV_CAMSV2_MODULE_TG_EN){
                      if(MFALSE == bCamsv2_2pix) {
                        PIPE_INF("sv2 jpg fmt must be in 2pix mode\n");
                        bCamsv2_2pix = MTRUE;
                      }
                      en_camsv2 |= CAMSV_CAMSV2_MODULE_PAK_SEL;
                 }
                 camsv_imgo_bus = 1;
                 camsv2_imgo_bus = 1;
                 break;
             case  eScenarioFmt_RGB:
                 if (EPortIndex_TG1I == vInPorts[i]->index) {
                     subMode = ISP_SUB_MODE_RGB;
                 }else if (EPortIndex_TG2I == vInPorts[i]->index)  {
                     subMode = ISP_SUB_MODE_RGB; // p2 driver need to get sub mode from p1 driver
                     subModeD = ISP_SUB_MODE_RGB;
                 }

             default:
               camsv_imgofmt = camsv2_imgofmt = CAMSV_FMT_SEL_IMGO_OTHER;
                 PIPE_ERR("NOT Support submode");
                 return MFALSE;
         }
    }
    PIPE_INF("subMode(%d),subModeD(%d)",subMode, subModeD);


    //
    scenario = meScenarioID;
    switch (meScenarioID) {
        case eDrvScenario_CC:       //video snap shot
            //
            //a special scenario
            scenario = eDrvScenario_CC;
            continuous = 1;
            //
            break;
        default:
            PIPE_ERR("NOT Support scenario for PASS1");
            return MFALSE;
    }

    //
    //
    /*-----------------------------------------------------------------------------
      m_camPass1Param
      -----------------------------------------------------------------------------*/
    //path
    m_camPass1Param.path = path;
    m_camPass1Param.pathD = pathD;
    m_camPass1Param.bypass_ispRawPipe = ( ISP_PASS1==path || ISP_PASS1_D==path ) || ( ISP_PASS1==pathD || ISP_PASS1_D==pathD )?0:1;
    //scenario/sub_mode
    m_camPass1Param.sub_mode  = subMode;
    m_camPass1Param.sub_mode_D = subModeD;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SCENARIO.Bits.SCENARIO = scenario;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SCENARIO.Bits.SUB_MODE = subMode;
    //twin mode
    m_camPass1Param.isTwinMode = pass1_twin; //raw-d doesn't support TwinMode
    m_pass1TwinMode = pass1_twin;
    //single/continuous mode
    m_camPass1Param.b_continuous[0] = continuous; //
    m_camPass1Param.b_continuous[1] = continuous; // TODO:

    //global: always set PASS1_DB_EN and  PASS1_D_DB_EN of CAM_CTL_SEL_GLOBAL_MASK
    // in order to easy config in TOP_CTRL->_config()
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL_MASK = 0x001FCFEB;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Bits.DUAL_MODE = (pass1_twin?1:0);
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Bits.PASS1_DB_EN = 0;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Bits.PASS1_D_DB_EN = 0;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_GLOBAL.Bits.INT_MRG = 1;

    //enable table
    if(pass1_twin){
        en_p1 |= CAM_CTL_EN_P1_HBIN_EN;
    }
    m_camPass1Param.isp_top_ctl.CAM_CTL_EN_P1.Raw       = en_p1;
    m_camPass1Param.isp_top_ctl.CAM_CTL_EN_P1_DMA.Raw   = en_p1_dma;
    m_camPass1Param.isp_top_ctl.CAM_CTL_INT_P1_EN.Raw   = int_p1_en;
    m_camPass1Param.isp_top_ctl.CAM_CTL_INT_P1_EN2.Raw  = int_p1_en2;


    m_camPass1Param.isp_top_ctl.CAM_CTL_EN_P1_D.Raw         = (en_p1_d | \
            (pass1_twin?((CAM_CTL_EN_P1_RRZ_EN&en_p1)?(CAM_CTL_EN_P1_D_RRZ_D_EN):(0)):(0)));
    m_camPass1Param.isp_top_ctl.CAM_CTL_EN_P1_DMA_D.Raw  = en_p1_dma_d;
    m_camPass1Param.isp_top_ctl.CAM_CTL_INT_P1_EN_D.Raw  = int_p1_en_d;
    m_camPass1Param.isp_top_ctl.CAM_CTL_INT_P1_EN2_D.Raw = int_p1_en2_d;


    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_MODULE_EN.Raw = en_camsv;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_INT_EN.Raw    = int_camsv_en;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_MODULE_EN.Raw= en_camsv2;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_INT_EN.Raw   = int_camsv2_en;
    //
    m_camPass1Param.isp_top_ctl.CAM_CTL_CQ_EN.Raw = pass1_cq_en;
    //
    m_camPass1Param.CQ  = pass1_CQ;
    m_camPass1Param.CQ_D = pass1_CQ_D;
    //cq0c trigger source
    if ( en_p1_dma & CAM_CTL_EN_P1_DMA_IMGO_EN ) {
        m_camPass1Param.cqTrigSrc = CAM_CQ_TRIG_BY_IMGO_DONE;
    }
    else if ( en_p1_dma & CAM_CTL_EN_P1_DMA_RRZO_EN ) {
        m_camPass1Param.cqTrigSrc = CAM_CQ_TRIG_BY_RRZO_DONE;
    }
    else {
        m_camPass1Param.cqTrigSrc = CAM_CQ_TRIG_BY_NONE;
    }
    //cq0c_d trigger source
    if ( en_p1_dma_d & CAM_CTL_EN_P1_DMA_IMGO_EN ) {
        m_camPass1Param.cqTrigSrc_D = CAM_CQ_TRIG_BY_IMGO_DONE;
    }
    else if ( en_p1_dma_d & CAM_CTL_EN_P1_DMA_RRZO_EN ) {
        m_camPass1Param.cqTrigSrc_D = CAM_CQ_TRIG_BY_RRZO_DONE;
    }
    else {
        m_camPass1Param.cqTrigSrc_D = CAM_CQ_TRIG_BY_NONE;
    }
    PIPE_DBG("cqTrigSrc(%d),cqTrigSrc_D(%d)",m_camPass1Param.cqTrigSrc,m_camPass1Param.cqTrigSrc_D);

    //fmt_sel
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_MASK = 0x0780F033;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw  = 0x00; //reset fmt_sel
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.TG1_FMT  = tgInFmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.OUT_FMT  = cam_out_fmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.PAKG_FMT = pakG_fmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.PIX_ID   = pix_id_tg;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.TWO_PIX  = two_pxl;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.TWO_PIX_DMX = two_pxl_dmx;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.TWO_PIX_BMX = (pass1_twin&two_pxl)?1:0;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Bits.TWO_PIX_RMX = (pass1_twin&two_pxl)?1:0;;

    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D_MASK = 0x0780F033;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Raw  = 0x00;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.TG1_FMT_D  = tg2InFmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.OUT_FMT_D  = camD_out_fmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.PAKG_FMT_D = D_pakG_fmt;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.PIX_ID_D   = pass1_twin?pix_id_tg:pix_id_tg_d;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.TWO_PIX_D  = two_pxl;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.TWO_PIX_DMX_D = two_pxl_dmx;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.TWO_PIX_BMX_D = (pass1_twin&two_pxl)?1:0;;
    m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1_D.Bits.TWO_PIX_RMX_D = (pass1_twin&two_pxl)?1:0;;
    //CAMSV fmt_sel , mark,these will be set at senif_drv
    //PIPE_ERR("camsv/camsv2 format/pak is no rdy\n");
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_FMT_SEL_MASK = 0x0f030000;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Raw  = 0;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Bits.IMGO_FORMAT = camsv_imgofmt;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Bits.TG1_FMT     = camsv_fmt;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_FMT_SEL.Bits.IMGO_BUS_SIZE = camsv_imgo_bus;

    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_FMT_SEL_MASK = 0x0f030000;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Raw  = 0;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Bits.IMGO_FORMAT = camsv2_imgofmt;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Bits.TG1_FMT     = camsv2_fmt;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_FMT_SEL.Bits.IMGO_BUS_SIZE = camsv2_imgo_bus;

    //camsv pak
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Raw = 0;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Bits.PAK_DBL_MODE = bCamsv_2pix;
    switch(camsv_fmt) {
        case CAMSV_FMT_SEL_TG_FMT_RAW8:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW8;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW10:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW10;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW12:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW12;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW14:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW14;
            break;
    }
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Raw = 0;
    m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Bits.PAK_DBL_MODE = bCamsv2_2pix;
    switch(camsv2_fmt) {
        case CAMSV_FMT_SEL_TG_FMT_RAW8:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW8;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW10:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW10;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW12:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW12;
            break;
        case CAMSV_FMT_SEL_TG_FMT_RAW14:
            m_camPass1Param.isp_top_ctl.CAMSV_CAMSV2_PAK.Bits.PAK_MODE = CAMSV_PAK_MODE_FMT_RAW14;
            break;
    }
    PIPE_DBG(" fmt_SEL_P1=0x%x", m_camPass1Param.isp_top_ctl.CAM_CTL_FMT_SEL_P1.Raw);
    //LSC
    m_camPass1Param.twin_lsc_crop_offx = 0; //disable LSC 4:3 map to 16:9
    m_camPass1Param.twin_lsc_crop_offy = 0;

    //
    //int DB_en = 1;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_P1.Raw   = ctl_sel;
    m_camPass1Param.isp_top_ctl.CAM_CTL_SEL_P1_D.Raw = ctl_sel_d;
    //
    //source -> from TGx
    if ( -1 != idx_tgi ) {
        m_camPass1Param.tg_out_size.w = vInPorts[idx_tgi]->u4ImgWidth;  //not used for Pass1 now
        m_camPass1Param.tg_out_size.h = vInPorts[idx_tgi]->u4ImgHeight; //not used for Pass1 now
        PIPE_DBG("TGIn: %d,%d", vInPorts[idx_tgi]->u4ImgWidth, vInPorts[idx_tgi]->u4ImgHeight);
    } else {
        PIPE_DBG("-1 == idx_tgi");
    }

    if ( -1 != idx_tg2i) {
        m_camPass1Param.tg_out_size.w = vInPorts[idx_tg2i]->u4ImgWidth;  //not used for Pass1 now
        m_camPass1Param.tg_out_size.h = vInPorts[idx_tg2i]->u4ImgHeight; //not used for Pass1 now
        PIPE_DBG("D TGIn: %d,%d", vInPorts[idx_tg2i]->u4ImgWidth, vInPorts[idx_tg2i]->u4ImgHeight);
    } else {
        PIPE_DBG("D  -1 == idx_tg2i");
    }

    //
    if ( ISP_SUB_MODE_YUV == subMode ) {
        m_camPass1Param.bypass_ispRawPipe = 1;
        m_camPass1Param.bypass_ispRgbPipe = 1;
        //m_camPass1Param.bypass_ispYuvPipe = 1;
    }
    m_camPass1Param.bypass_imgo = 1;
    m_camPass1Param.bypass_rrzo = 1;
    m_camPass1Param.bypass_imgo_d = 1;
    m_camPass1Param.bypass_rrzo_d = 1;
    m_camPass1Param.bypass_camsv_imgo = 1;
    m_camPass1Param.bypass_camsv2_imgo = 1;



    if (-1 != idx_imgo ) { // use output dma crop

        //PIPE_DBG("======= cfg imgo ======= ");
        m_camPass1Param.bypass_imgo = 0;
        if( (portInfo_imgo.crop1.floatX!=0) || (portInfo_imgo.crop1.floatY!=0)){
            PIPE_INF("imgo warning: support no float crop\n");
            portInfo_imgo.crop1.floatX = portInfo_imgo.crop1.floatY = 0;
        }
        if(portInfo_imgo.crop1.w != portInfo_imgo.u4ImgWidth){
            PIPE_ERR("error: imgo: crop_w != dstsize_w(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo.crop1.w,portInfo_imgo.u4ImgWidth);
            //portInfo_imgo.u4ImgWidth = portInfo_imgo.crop1.w;
            return MFALSE;
        }
        if(portInfo_imgo.crop1.h != portInfo_imgo.u4ImgHeight){
            PIPE_ERR("error: imgo: crop_h != dstsize_h(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo.crop1.h,portInfo_imgo.u4ImgHeight);
            //portInfo_imgo.u4ImgHeight = portInfo_imgo.crop1.h;
            return MFALSE;
        }
        // use output dma crop
        this->configDmaPort(&portInfo_imgo,m_camPass1Param.imgo,(MUINT32)pixel_byte_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.imgo.lIspColorfmt = portInfo_imgo.eImgFmt;
//        m_camPass1Param.imgo.format = portInfo_imgo.eImgFmt;
        if ( en_p1 & CAM_CTL_EN_P1_WBN_EN ) {
            PIPE_INF("WBN ON");
            if ( m_camPass1Param.tg_out_size.w < portInfo_imgo.u4ImgWidth ) {
                PIPE_ERR("ERROR:IMGO out bigger than in");
            }
        }

        //for Raw sensor, IMGO only, configFrame() setting path
        m_camPass1Param.rrz_in_size         = m_camPass1Param.tg_out_size;

        //for YUV sensor configFrame() setting path
        /*if ( ISP_SUB_MODE_YUV == this->m_camPass1Param.sub_mode) {
            m_camPass1Param.rrz_in_size         = m_camPass1Param.tg_out_size;
        }*/

        m_camPass1Param.imgo.tgFps = tgFps;
    }


    if (-1 != idx_rrzo ) { // use output dma crop

        //PIPE_DBG("======= cfg rrzo ======= ");
        m_camPass1Param.bypass_rrzo = 0;
        if( (portInfo_rrzo.crop1.floatX!=0) || (portInfo_rrzo.crop1.floatY!=0)){
            PIPE_INF("rrzo warning: support no float crop\n");
            portInfo_rrzo.crop1.floatX = portInfo_rrzo.crop1.floatY = 0;
        }
        if(portInfo_rrzo.crop1.w < portInfo_rrzo.u4ImgWidth){
            PIPE_ERR("error: rrzo: crop_w < dstsize_w(0x%x,0x%x), resizer support only scale down\n",portInfo_rrzo.crop1.w,portInfo_rrzo.u4ImgWidth);
            //portInfo_rrzo.u4ImgWidth = portInfo_rrzo.crop1.w;
            return MFALSE;
        }
        if(portInfo_rrzo.crop1.h < portInfo_rrzo.u4ImgHeight){
            PIPE_ERR("error: rrzo: crop_h < dstsize_h(0x%x,0x%x), resizer support only scale down\n",portInfo_rrzo.crop1.h,portInfo_rrzo.u4ImgHeight);
            //portInfo_rrzo.u4ImgHeight = portInfo_rrzo.crop1.h;
            return MFALSE;
        }

         // use output dma crop
        this->configDmaPort(&portInfo_rrzo,m_camPass1Param.rrzo,(MUINT32)pixel_byte_rrzo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.rrzo.lIspColorfmt = portInfo_rrzo.eImgFmt;
        //m_camPass1Param.rrzo.format = portInfo_rrzo.eImgFmt;

        m_camPass1Param.rrz_in_size         = m_camPass1Param.tg_out_size;
        m_camPass1Param.rrz_in_roi.x        = m_camPass1Param.rrzo.crop.x;
        m_camPass1Param.rrz_in_roi.y        = m_camPass1Param.rrzo.crop.y;
        m_camPass1Param.rrz_in_roi.floatX   = m_camPass1Param.rrzo.crop.floatX;
        m_camPass1Param.rrz_in_roi.floatY   = m_camPass1Param.rrzo.crop.floatY;
        m_camPass1Param.rrz_in_roi.w        = m_camPass1Param.rrzo.crop.w;
        m_camPass1Param.rrz_in_roi.h        = m_camPass1Param.rrzo.crop.h;
        //We use RRZ to crop, thus after passing info to RRZ, reset the RRZOCrop to full img of rrz out
        PIPE_INF("[3ASync]rrzo crop=%d,%d",m_camPass1Param.rrzo.crop.w, m_camPass1Param.rrzo.crop.h );
        m_camPass1Param.rrzo.crop.x =0;
        m_camPass1Param.rrzo.crop.y =0;
        m_camPass1Param.rrzo.crop.w = portInfo_rrzo.u4ImgWidth;
        m_camPass1Param.rrzo.crop.h = portInfo_rrzo.u4ImgHeight;

        if ( en_p1 & CAM_CTL_EN_P1_WBN_EN ) {
            PIPE_INF("WBN ON");
            m_camPass1Param.rrz_in_size.w >>= 1;
            m_camPass1Param.rrz_in_roi.x        = m_camPass1Param.rrzo.crop.x;
            m_camPass1Param.rrz_in_roi.floatX   = m_camPass1Param.rrzo.crop.floatX;
            m_camPass1Param.rrz_in_roi.w        = m_camPass1Param.rrzo.crop.w;
        }

        m_camPass1Param.rrzo.tgFps = tgFps;
    }


    if (-1 != idx_imgo_d ) {

        //PIPE_DBG("======= cfg imgo_d ======= ");
        m_camPass1Param.bypass_imgo_d = 0;
        if( (portInfo_imgo_d.crop1.floatX!=0) || (portInfo_imgo_d.crop1.floatY!=0)){
            PIPE_INF("imgo_d warning: support no float crop\n");
            portInfo_imgo_d.crop1.floatX = portInfo_imgo_d.crop1.floatY = 0;
        }
        if(portInfo_imgo_d.crop1.w != portInfo_imgo_d.u4ImgWidth){
            PIPE_ERR("error: imgo_d: crop_w != dstsize_w(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo_d.crop1.w,portInfo_imgo_d.u4ImgWidth);
            //portInfo_imgo_d.u4ImgWidth = portInfo_imgo_d.crop1.w;
            return MFALSE;
        }
        if(portInfo_imgo_d.crop1.h != portInfo_imgo_d.u4ImgHeight){
            PIPE_ERR("error: imgo_d: crop_h != dstsize_h(0x%x,0x%x), this port have no build-in resizer\n",portInfo_imgo_d.crop1.h,portInfo_imgo_d.u4ImgHeight);
            //portInfo_imgo_d.u4ImgHeight = portInfo_imgo_d.crop1.h;
            return MFALSE;
        }

        this->configDmaPort(&portInfo_imgo_d,m_camPass1Param.imgo_d,(MUINT32)pixel_byte_imgo_d,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.imgo_d.lIspColorfmt = portInfo_imgo_d.eImgFmt;
//        m_camPass1Param.imgo_d.format = portInfo_imgo_d.eImgFmt;
        if ( en_p1_d & CAM_CTL_EN_P1_D_WBN_D_EN ) {
            PIPE_INF("WBN-D ON");
            if ( m_camPass1Param.tg_out_size.w < portInfo_imgo_d.u4ImgWidth ) {
                PIPE_ERR("ERROR:IMGOD out bigger than in");
            }
        }

        //for Raw sensor, IMGO only, configFrame() setting path
        m_camPass1Param.rrz_d_in_size         = m_camPass1Param.tg_out_size;

        //for YUV sensor configFrame() setting path
        /*if ( ISP_SUB_MODE_YUV == this->m_camPass1Param.sub_mode_D) {
            m_camPass1Param.rrz_d_in_size         = m_camPass1Param.tg_out_size;
        }*/

    }


    if (-1 != idx_rrzo_d ) { // use output dma crop

        //PIPE_DBG("======= cfg rrzo_d ======= ");
        m_camPass1Param.bypass_rrzo_d = 0;
        if( (portInfo_rrzo_d.crop1.floatX!=0) || (portInfo_rrzo_d.crop1.floatY!=0)){
            PIPE_INF("rrzo_d warning: support no float crop\n");
            portInfo_rrzo_d.crop1.floatX = portInfo_rrzo_d.crop1.floatY = 0;
        }
        if(portInfo_rrzo_d.crop1.w < portInfo_rrzo_d.u4ImgWidth){
            PIPE_ERR("error: rrzo_d: crop_w < dstsize_w(0x%x,0x%x), resizer support only scale down\n",portInfo_rrzo_d.crop1.w,portInfo_rrzo_d.u4ImgWidth);
            //portInfo_rrzo_d.u4ImgWidth = portInfo_rrzo_d.crop1.w;
            return MFALSE;
        }
        if(portInfo_rrzo_d.crop1.h < portInfo_rrzo_d.u4ImgHeight){
            PIPE_ERR("error: rrzo_d: crop_h < dstsize_h(0x%x,0x%x), resizer support only scale down\n",portInfo_rrzo_d.crop1.h,portInfo_rrzo_d.u4ImgHeight);
            //portInfo_rrzo_d.u4ImgHeight = portInfo_rrzo_d.crop1.h;
            return MFALSE;
        }

         // use output dma crop
        this->configDmaPort(&portInfo_rrzo_d,m_camPass1Param.rrzo_d,(MUINT32)pixel_byte_rrzo_d,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.rrzo_d.lIspColorfmt = portInfo_rrzo_d.eImgFmt;
//        m_camPass1Param.rrzo_d.format = portInfo_rrzo_d.eImgFmt;
        m_camPass1Param.rrz_d_in_size         = m_camPass1Param.tg_out_size;
        m_camPass1Param.rrz_d_in_roi.x        = m_camPass1Param.rrzo_d.crop.x;
        m_camPass1Param.rrz_d_in_roi.y        = m_camPass1Param.rrzo_d.crop.y;
        m_camPass1Param.rrz_d_in_roi.floatX   = m_camPass1Param.rrzo_d.crop.floatX;
        m_camPass1Param.rrz_d_in_roi.floatY   = m_camPass1Param.rrzo_d.crop.floatY;
        m_camPass1Param.rrz_d_in_roi.w        = m_camPass1Param.rrzo_d.crop.w;
        m_camPass1Param.rrz_d_in_roi.h        = m_camPass1Param.rrzo_d.crop.h;
        //We use RRZ to crop, thus after passing info to RRZ, reset the RRZOCrop to full image of rrz out
        m_camPass1Param.rrzo_d.crop.x =0;
        m_camPass1Param.rrzo_d.crop.y =0;
        m_camPass1Param.rrzo_d.crop.w = portInfo_rrzo_d.u4ImgWidth;
        m_camPass1Param.rrzo_d.crop.h = portInfo_rrzo_d.u4ImgHeight;

        if ( en_p1_d & CAM_CTL_EN_P1_D_WBN_D_EN ) {
             m_camPass1Param.rrz_d_in_size.w >>= 1;
             m_camPass1Param.rrz_d_in_roi.x        = m_camPass1Param.rrzo_d.crop.x;
             m_camPass1Param.rrz_d_in_roi.floatX   = m_camPass1Param.rrzo_d.crop.floatX;
             m_camPass1Param.rrz_d_in_roi.w        = m_camPass1Param.rrzo_d.crop.w;
             PIPE_INF("WBN_D ON");
        }

    }


    if(-1 != idx_camsv_imgo){

        PIPE_INF("cfg camsv_imgo");
        m_camPass1Param.bypass_camsv_imgo = 0;
        if((portInfo_camsv_imgo.crop1.y!=0) || (portInfo_camsv_imgo.crop1.h == portInfo_camsv_imgo.u4ImgHeight))
            PIPE_DBG("[warning]camsv crop_y=%d crop_floatY(%d) crop_h=%d ", \
                portInfo_camsv_imgo.crop1.y,portInfo_camsv_imgo.crop1.floatY,portInfo_camsv_imgo.crop1.h);

        this->configDmaPort(&portInfo_camsv_imgo,m_camPass1Param.camsv_imgo,(MUINT32)pixel_byte_camsv_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.camsv_imgo.lIspColorfmt = portInfo_camsv_imgo.eImgFmt;
        //m_camPass1Param.camsv_imgo.format = portInfo_camsv_imgo.eImgFmt;
    }

    if(-1 != idx_camsv2_imgo){

        PIPE_INF("cfg camsv2_imgo");
        m_camPass1Param.bypass_camsv2_imgo = 0;
        if((portInfo_camsv2_imgo.crop1.y!=0) || (portInfo_camsv2_imgo.crop1.h == portInfo_camsv2_imgo.u4ImgHeight))
            PIPE_DBG("[warning]camsv2 crop_y=%d crop_floatY(%d) crop_h=%d ", \
                portInfo_camsv2_imgo.crop1.y,portInfo_camsv2_imgo.crop1.floatY,portInfo_camsv2_imgo.crop1.h);

        this->configDmaPort(&portInfo_camsv2_imgo,m_camPass1Param.camsv2_imgo,(MUINT32)pixel_byte_camsv2_imgo,(MUINT32)0,(MUINT32)1,ESTRIDE_1ST_PLANE);
        m_camPass1Param.camsv2_imgo.lIspColorfmt = portInfo_camsv2_imgo.eImgFmt;
        //m_camPass1Param.camsv2_imgo.format = portInfo_camsv2_imgo.eImgFmt;
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
    int    ret = 0;
    MINT32 type = 0;
    MUINT32 irq = 0;

    PIPE_DBG("+ tid(%d) (pass,irq_int)=(0x%08x,0x%08x)", gettid(), pass, irq_int);

    //pass
    if ( EPipePass_PASS1_TG1 != pass && EPipePass_PASS1_TG1_D != pass ) {
        PIPE_ERR("IRQ:NOT SUPPORT pass path");
        return MFALSE;
    }
    //irq_int
    switch(pass){
        case EPipePass_PASS1_TG1:   type = ISP_DRV_IRQ_TYPE_INT_P1_ST;
            break;
        case EPipePass_PASS1_TG1_D: type = ISP_DRV_IRQ_TYPE_INT_P1_ST_D;
            break;
        case EPipePass_CAMSV1_TG:   type = ISP_DRV_IRQ_TYPE_INT_CAMSV;
            break;
        case EPipePass_CAMSV2_TG:   type = ISP_DRV_IRQ_TYPE_INT_CAMSV2;
            break;
        default:
            PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
            return MFALSE;
            break;
    }
    if ( EPIPEIRQ_VSYNC == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:   irq = CAM_CTL_INT_P1_STATUS_VS1_INT_ST;
                break;
            case EPipePass_PASS1_TG1_D: irq  = CAM_CTL_INT_P1_STATUS_D_VS1_INT_ST;
                break;
            case EPipePass_CAMSV1_TG:   irq  = CAMSV_INT_STATUS_VS1_ST;
                break;
            case EPipePass_CAMSV2_TG:   irq  = CAMSV2_INT_STATUS_VS1_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else if ( EPIPEIRQ_PATH_DONE == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:   irq = CAM_CTL_INT_P1_STATUS_PASS1_DON_ST;
                break;
            case EPipePass_PASS1_TG1_D: irq  = CAM_CTL_INT_P1_STATUS_D_PASS1_DON_ST;
                break;
            case EPipePass_CAMSV1_TG:   irq  = CAMSV_INT_STATUS_PASS1_DON_ST;
                break;
            case EPipePass_CAMSV2_TG:   irq  = CAMSV2_INT_STATUS_PASS1_DON_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else if ( EPIPEIRQ_SOF == irq_int ) {
        switch(pass){
            case EPipePass_PASS1_TG1:   irq = CAM_CTL_INT_P1_STATUS_SOF1_INT_ST;
                break;
            case EPipePass_PASS1_TG1_D: irq  = CAM_CTL_INT_P1_STATUS_D_SOF1_INT_ST;
                break;
            case EPipePass_CAMSV1_TG:   irq  = CAMSV_INT_STATUS_TG_SOF1_ST;
                break;
            case EPipePass_CAMSV2_TG:   irq  = CAMSV2_INT_STATUS_TG_SOF1_ST;
                break;
            default:
                PIPE_ERR("errpr:tg(0x%x) r not supported\n",pass);
                return MFALSE;
                break;
        }
    }
    else {
        PIPE_ERR("IRQ:NOT SUPPORT irq");
        return  MFALSE;
    }
    //
    PIPE_DBG("(type,user,irq)=(0x%08x,%d,0x%08x)", type,userEnum, irq);
    //
    ret = m_CamPathPass1.waitIrq(type,irq,userEnum);

    if( ret != 0 )
    {
        PIPE_ERR("waitIrq error!");
        return  MFALSE;
    }
    return  MTRUE;
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
        else if ( EPortIndex_TG2I == path ) {
            cq = ISP_DRV_CQ0_D;
        }
        else {
            PIPE_ERR("%d",path);
        }

        return this->m_pIspDrvShell->getPhyIspDrv()->getCQInstance((ISP_DRV_CQ_ENUM)cq)->getRegAddr();

    }

    return 0;
}


/*******************************************************************************
* Note: in cam1 (m_bBypassImgo=MTRUE)
* if always pure raw & two_single mode & IMGO only
* => mag_rrz0(configFrame) == mag_aa(SET_P1_UPDATE)
********************************************************************************/
#define _CmdQSwitchEn(OpenedPort,Sub_mode,mag_rrz0,mag_aa,mag_imgo)({\
    MBOOL _flag = MFALSE;\
    if((OpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN) && (OpenedPort & CAM_CTL_EN_P1_DMA_IMGO_EN)){\
        if(this->m_bBypassImgo == MTRUE){\
            if(mag_rrz0 == mag_aa)\
                _flag = MTRUE;\
        }\
        else {\
            if((mag_rrz0 == mag_aa) && (mag_rrz0 == mag_imgo))\
                _flag = MTRUE;\
        }\
    }\
    else if((OpenedPort == CAM_CTL_EN_P1_DMA_RRZO_EN) || (Sub_mode == ISP_SUB_MODE_YUV)){\
        if(mag_rrz0 == mag_aa)\
            _flag = MTRUE;\
    }\
    else if(OpenedPort == CAM_CTL_EN_P1_DMA_IMGO_EN){\
        if(this->m_bBypassImgo == MTRUE){\
            if(mag_rrz0 == mag_aa)\
                _flag = MTRUE;\
        }\
        else {\
            if(mag_imgo == mag_aa)\
                _flag = MTRUE;\
        }\
    }\
    _flag;\
})

MBOOL
CamIOPipe::
sendCommand(MINT32 cmd, MINTPTR arg1, MINTPTR arg2, MINTPTR arg3)
{
    MUINT32 tg;
    int    ret = 0; // 0 for ok , -1 for fail
    IspSize out_size;
    IspDMACfg out_dma;
    MUINT32 dmaChannel = 0;
    IspRrzCfg rrz_cfg;
    IspRrzCfg *pRrz_cfg = 0;
    MUINT32 magicNum = -1;
    IspP1Cfg  p1_cfg;
    MINT32 dupCqIdx = 0;

    RRZCfg* pUserRrzPara = 0;

    PIPE_DBG("+ tid(%d) (cmd,arg1,arg2,arg3)=(0x%08x,0x%08x,0x%08x,0x%08x)", gettid(), cmd, arg1, arg2, arg3);

    switch ( cmd ) {
        //case EPIPECmd_SET_D_PATH_SCENARIO_TYPE:
        //    meScenarioFmtD = (NSImageio::NSIspio::EScenarioFmt) arg1;//CAM_ISP_CQ0
        //    break;
        case EPIPECmd_SET_EIS_CBFP:
        case EPIPECmd_SET_LCS_CBFP:
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

        case EPIPECmd_SET_D_CQ_CHANNEL:
            m_pass1_RawD_CQ = arg1;//CAM_ISP_CQ0
            m_CamPathPass1.CQ_D = m_pass1_RawD_CQ;
            //m_CamPathPass1.flushCqDescriptor((MUINT32) m_pass1_RawD_CQ);
            break;
        case EPIPECmd_SET_D_CQ_TRIGGER_MODE:
            //TO Physical Reg.
            m_CamPathPass1.setCQTriggerMode(arg1,arg2,arg3);
            m_CQ0DTrigMode  = (ISP_DRV_CQ0_D  == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            m_CQ0BDTrigMode = (ISP_DRV_CQ0B_D == (ISP_DRV_CQ_ENUM)arg1)?arg2:0;
            break;

        case EPIPECmd_SET_RRZ:
            //Arg1: RRZCfg
            //Arg2: EPortIndex_TG1I/EPortIndex_TG2I
            //Arg3: sensorDev
            if (NULL == arg1) {
                PIPE_ERR("[3ASync] SET_RRZ NULL param(0x%x)!",arg1);
                return MFALSE;
            }
            pUserRrzPara = (RRZCfg*)arg1;

            if (0 == pUserRrzPara->out.stride) {
                PIPE_ERR("[3ASync] SET_RRZ Invalid stride(0x%x)!",((RRZCfg*)arg1)->out.stride);
            }

            //if (0 ==pUserRrzPara->out.stride * pUserRrzPara->crop.w * pUserRrzPara->crop.h) {
            if (0 == pUserRrzPara->crop.w * pUserRrzPara->crop.h) {
                PIPE_ERR("[3ASync] SET_RRZ Invalid Para(0x%x/0x%x/0x%x)!",pUserRrzPara->out.stride, pUserRrzPara->crop.w, pUserRrzPara->crop.h);
                return MFALSE;
            }

            if (0 == pUserRrzPara->out.w * pUserRrzPara->out.h) {
                PIPE_ERR("[3ASync] SET_RRZ Invalid OutWind Para(0x%x/0x%x)!", pUserRrzPara->out.w, pUserRrzPara->out.h);
                return MFALSE;
            }


            PIPE_DBG("[3ASync]SET_RRZ path:%d, m_num_0=%x -", arg2, pUserRrzPara->m_num);

            this->mRunTimeCfgLock.lock();
            if (0 != m_rrz_update_cnt) {
                PIPE_DBG("[3ASync][Warn]SET_RRZ: RRZCfg Enter twice-");
            }

            if ( EPortIndex_TG1I == arg2){
                if (pUserRrzPara->crop.x > this->m_camPass1Param.rrz_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR: Crop invalid(x(%d)>size_w(%d))",pUserRrzPara->crop.x, this->m_camPass1Param.rrz_in_size.w);
                    m_rrz_update_cnt = 0;
                    return MFALSE;
                }
                if (pUserRrzPara->crop.w > this->m_camPass1Param.rrz_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR: Not Support Scale up(in(%d)<roi_w(%d))", pUserRrzPara->crop.w, this->m_camPass1Param.rrz_in_size.w);
                    m_rrz_update_cnt = 0;
                    return MFALSE;
                }
                    pRrz_cfg = &m_rrz_cfg;
                    m_rrz_update_cnt++;
            }
            else if ( EPortIndex_TG2I == arg2){
                if (pUserRrzPara->crop.x > this->m_camPass1Param.rrz_d_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR:D: CropInvalid(x(%d)>size_w(%d))",pUserRrzPara->crop.x, this->m_camPass1Param.rrz_d_in_size.w);
                    m_rrzd_update_cnt = 0;
                    return MFALSE;
                }
                if (pUserRrzPara->crop.w > this->m_camPass1Param.rrz_d_in_size.w) {
                    PIPE_ERR("[3ASync]ERROR:D: NotSupportScale up roi_w(%d)>(in(%d))", pUserRrzPara->crop.w, this->m_camPass1Param.rrz_d_in_size.w);
                    m_rrzd_update_cnt = 0;
                    return MFALSE;
                }
                pRrz_cfg = &m_rrz_d_cfg;
                m_rrzd_update_cnt++;
            }


            pRrz_cfg->rrz_in_roi.x        = pUserRrzPara->crop.x;
            pRrz_cfg->rrz_in_roi.y        = pUserRrzPara->crop.y;
            pRrz_cfg->rrz_in_roi.floatX   = pUserRrzPara->crop.floatX;
            pRrz_cfg->rrz_in_roi.floatY   = pUserRrzPara->crop.floatY;
            pRrz_cfg->rrz_in_roi.w        = pUserRrzPara->crop.w;
            pRrz_cfg->rrz_in_roi.h        = pUserRrzPara->crop.h;
            pRrz_cfg->rrz_out_size.w       = pUserRrzPara->out.w;
            pRrz_cfg->rrz_out_size.h       = pUserRrzPara->out.h;
            pRrz_cfg->rrz_out_size.stride  = pUserRrzPara->out.stride;
            pRrz_cfg->lsc_en = 0;
            pRrz_cfg->sd_lwidth = 0;
            pRrz_cfg->sd_xnum = 0;
            pRrz_cfg->m_num_0 = pUserRrzPara->m_num; //MW M number
            pRrz_cfg->mImgSel = pUserRrzPara->mImgSel;
            //

            if ( EPortIndex_TG1I == arg2){

                pRrz_cfg->rrz_in_size.w       = this->m_camPass1Param.rrz_in_size.w;
                pRrz_cfg->rrz_in_size.h       = this->m_camPass1Param.rrz_in_size.h;
                mMagicNum0 = pRrz_cfg->m_num_0;

                //for YUV sensor, no P1 tuning /mark this because 3A will sendCommand(P1_Update)
                /*if ( ISP_SUB_MODE_YUV == this->m_camPass1Param.sub_mode) {
                    mMagicNum1 = mMagicNum0;
                    PIPE_DBG("YUV(%d,%d)",mMagicNum0,mMagicNum1);
                }//*/

                if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0,mMagicNum1,mMagicNum2)) {
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_CamPathPass1.pass1_path = ISP_PASS1;
                    m_rrz_cfg.isTwinMode = m_pass1TwinMode;
                    rrz_cfg = m_rrz_cfg;
                    rrz_cfg.twin_lsc_crop_offx = 0;
                    rrz_cfg.twin_lsc_crop_offy = 0;
                    rrz_cfg.pass1_path = ISP_PASS1;
                    m_rrz_update_cnt = 0;

                    //bypass tuningQ for YUV
                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum0, arg3) ){
                            PIPE_ERR("[3ASync]SET_RRZ setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                        if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                            PIPE_ERR("[3ASync]SET_RRZ setP1RrzCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]SET_RRZ setP1imgoCfg Fail");
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
                        else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                            PIPE_ERR("[3ASync]updateCQ_D Fail");
                        else{
                            PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x%x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                        }
                        ret = -1;
                    }

                } else {
                    PIPE_DBG("[3ASync]SET_RRZ:TG1I Magic0(0x%x)!= Magic1(0x%x) != Magic2(0x%x)-", mMagicNum0, mMagicNum1,mMagicNum2);
                }
            }
            else if ( EPortIndex_TG2I == arg2){

                pRrz_cfg->rrz_in_size.w       = this->m_camPass1Param.rrz_d_in_size.w;
                pRrz_cfg->rrz_in_size.h       = this->m_camPass1Param.rrz_d_in_size.h;
                mMagicNum0_D = pRrz_cfg->m_num_0;
                if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0_D,mMagicNum1_D,mMagicNum2_D)) {
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_CamPathPass1.pass1_path = ISP_PASS1_D;
                    m_rrz_d_cfg.isTwinMode = 0;//m_pass1TwinMode;
                    rrz_cfg = m_rrz_d_cfg;
                    rrz_cfg.twin_lsc_crop_offx = 0;
                    rrz_cfg.twin_lsc_crop_offy = 0;
                    rrz_cfg.pass1_path = ISP_PASS1_D;
                    m_rrzd_update_cnt = 0;

                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum0_D, arg3) ){
                            PIPE_ERR("[3ASync]SET_RRZ_D setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                        if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                            PIPE_ERR("[3ASync]SET_RRZ_D setP1RrzCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]SET_RRZ_D setP1imgoCfg not ready");
                                //ret = -1;
                            }
                        }
                    }

                    if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum0_D)){
                        ret = -1;
                    }

                    if(1 != m_CamPathPass1.setP1Notify()){
                        ret = -1;
                    }
                    mDupCmdQMgr.CmdQMgr_update();
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();

#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                    //startCQ0_D();
#else
                    if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                        //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                        startCQ0_D();//for rrz configframe before start
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                    }
#endif
                    if ( 0 != m_CamPathPass1.updateCQI()){
                        if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                            PIPE_ERR("[3ASync]updateCQ Fail");
                        else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                            PIPE_ERR("[3ASync]updateCQ_D Fail");
                        else{
                            PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x%x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                        }
                        ret = -1;
                    }
                } else {
                    PIPE_DBG("[3ASync]SET_RRZ:TG2I Magic0(0x%x)!= Magic1(0x%x)!=magic2(0x%x)-", mMagicNum0_D, mMagicNum1_D,mMagicNum2_D);
                }

            }
            this->mRunTimeCfgLock.unlock();
            PIPE_DBG("[3ASync]SET_RRZ [%d/%d], m_num_0=%x -", m_rrz_update_cnt, m_rrzd_update_cnt, pRrz_cfg->m_num_0);

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
                if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0,mMagicNum1,mMagicNum2)){
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_CamPathPass1.pass1_path = ISP_PASS1;
                    m_rrz_cfg.isTwinMode = m_pass1TwinMode;
                    rrz_cfg = m_rrz_cfg;
                    rrz_cfg.twin_lsc_crop_offx = 0;
                    rrz_cfg.twin_lsc_crop_offy = 0;
                    rrz_cfg.pass1_path = ISP_PASS1;
                    m_rrz_update_cnt = 0;

                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum1, arg3) ){
                            PIPE_ERR("[3ASync]P1_UPDATE setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                        if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                            PIPE_ERR("[3ASync]P1_UPDATE setP1RrzCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]P1_UPDATE setP1imgoCfg not ready");
                                //ret = -1;
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
                        else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                            PIPE_ERR("[3ASync]updateCQ_D Fail");
                        else{
                            PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x5x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                        }
                        ret = -1;
                    }

                } else {
                    PIPE_DBG("[3ASync]: P1_UPDATE:TG1I Magic0(0x%x)!= Magic1(0x%x) != magic2-", mMagicNum0, mMagicNum1,mMagicNum2);
                }
            }
            else if ( EPortIndex_TG2I == arg1 ) {
                //update param here
                PIPE_DBG("[3ASync]P1_UPDATE+, TG2I, magicNum=0x%x", arg2);

                mMagicNum1_D = arg2;
                if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0_D,mMagicNum1_D,mMagicNum2_D)){
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                    m_CamPathPass1.pass1_path = ISP_PASS1_D;
                    m_rrz_d_cfg.isTwinMode = 0;//m_pass1TwinMode;
                    rrz_cfg = m_rrz_d_cfg;
                    rrz_cfg.twin_lsc_crop_offx = 0;
                    rrz_cfg.twin_lsc_crop_offy = 0;
                    rrz_cfg.pass1_path = ISP_PASS1_D;
                    m_rrzd_update_cnt = 0;

                    if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                        if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum1_D, arg3) ){
                            PIPE_ERR("[3ASync]P1_UPDATE D setP1TuneCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                        if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                             PIPE_ERR("[3ASync]P1_UPDATE D setP1RrzCfg Fail");
                            ret = -1;
                        }
                    }

                    if(this->m_bBypassImgo == MFALSE){
                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_IMGO_EN){
                            if(0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg)){
                                PIPE_ERR("[3ASync]P1_UPDATE D setP1imgoCfg not ready");
                                //ret = -1;
                            }
                        }
                    }

                    if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum1_D)){
                        ret = -1;
                    }

                    if(1 != m_CamPathPass1.setP1Notify()){
                        ret = -1;
                    }
                    mDupCmdQMgr.CmdQMgr_update();
                    m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                    m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                    //startCQ0_D();
#else
                    if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                        //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                        startCQ0_D();//for rrz configframe before start
                        sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                    }
#endif
                    if ( 0 != m_CamPathPass1.updateCQI()){
                        if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                            PIPE_ERR("[3ASync]updateCQ Fail");
                        else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                            PIPE_ERR("[3ASync]updateCQ_D Fail");
                        else{
                            PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x%x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                        }
                        ret = -1;
                    }
                } else {
                    PIPE_DBG("[3ASync]:TG2I Magic0(0x%x)!= Magic1(0x%x) != Magic2(0x%x)-", mMagicNum0_D, mMagicNum1_D,mMagicNum2_D);
                }
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
                if (((EPortIndex)arg2 != EPortIndex_TG1I)  && ((EPortIndex)arg2 != EPortIndex_TG2I)) {
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
                    if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0,mMagicNum1,mMagicNum2)){
                        m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                        m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                        m_CamPathPass1.pass1_path = ISP_PASS1;
                        m_rrz_cfg.isTwinMode = m_pass1TwinMode;
                        rrz_cfg = m_rrz_cfg;
                        rrz_cfg.twin_lsc_crop_offx = 0;
                        rrz_cfg.twin_lsc_crop_offy = 0;
                        rrz_cfg.pass1_path = ISP_PASS1;
                        m_rrz_update_cnt = 0;

                        if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                            if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum2, arg3) ){
                                PIPE_ERR("[3ASync]SET_IMGO setP1TuneCfg Fail");
                                ret = -1;
                            }
                        }

                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                            if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                                PIPE_ERR("[3ASync]SET_IMGO setP1RrzCfg Fail");
                                ret = -1;
                            }
                        }

                        if ( 0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg) ){
                            PIPE_ERR("[3ASync]SET_IMGO setP1ImgoCfg not ready");
                            //ret = -1;
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
                            else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                                PIPE_ERR("[3ASync]updateCQ_D Fail");
                            else{
                                PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x5x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                            }
                            ret = -1;
                        }

                    } else {
                        PIPE_DBG("[3ASync]: SET_IMGO:TG1I Magic0(0x%x)!= Magic1(0x%x) != magic2(0x%x)", mMagicNum0, mMagicNum1,mMagicNum2);
                    }
                }
                else if ( EPortIndex_TG2I == arg2 ) {
                    //update param here
                    PIPE_DBG("[3ASync]SET_IMGO+, TG2I, magicNum=0x%x", this->m_ImgoCfg.m_num);

                    mMagicNum2_D = this->m_ImgoCfg.m_num;
                    if (_CmdQSwitchEn(this->mOpenedPort,this->m_camPass1Param.sub_mode,mMagicNum0_D,mMagicNum1_D,mMagicNum2_D)){
                        m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                        m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
                        m_CamPathPass1.pass1_path = ISP_PASS1_D;
                        m_rrz_d_cfg.isTwinMode = 0;//m_pass1TwinMode;
                        rrz_cfg = m_rrz_d_cfg;
                        rrz_cfg.twin_lsc_crop_offx = 0;
                        rrz_cfg.twin_lsc_crop_offy = 0;
                        rrz_cfg.pass1_path = ISP_PASS1_D;
                        m_rrzd_update_cnt = 0;

                        if ( ISP_SUB_MODE_YUV != this->m_camPass1Param.sub_mode) {
                            if ( 0 != m_CamPathPass1.setP1TuneCfg(mMagicNum2_D, arg3) ){
                                PIPE_ERR("[3ASync]SET_IMGO D setP1TuneCfg Fail");
                                ret = -1;
                            }
                        }

                        if(this->mOpenedPort & CAM_CTL_EN_P1_DMA_RRZO_EN){
                            if ( 0 != m_CamPathPass1.setP1RrzCfg(rrz_cfg) ){
                                 PIPE_ERR("[3ASync]SET_IMGO D setP1RrzCfg Fail");
                                ret = -1;
                            }
                        }

                        if ( 0 != m_CamPathPass1.setP1ImgoCfg(this->m_ImgoCfg) ){
                            PIPE_ERR("[3ASync]SET_IMGO D setP1ImgoCfg not ready");
                            //ret = -1;
                        }

                        if(0!= m_CamPathPass1.setP1MagicNum(mMagicNum2_D)){
                            ret = -1;
                        }

                        if(1 != m_CamPathPass1.setP1Notify()){
                            ret = -1;
                        }
                        mDupCmdQMgr.CmdQMgr_update();
                        m_CamPathPass1.BurstQNum = mDupCmdQMgr.CmdQMgr_GetBurstQ();
                        m_CamPathPass1.DupQIdx = mDupCmdQMgr.CmdQMgr_GetDuqQIdx();
#ifndef _PASS1_CQ_CONTINUOUS_MODE_
                        //startCQ0_D();
#else
                        if(this->mDupCmdQMgr.CmdQMgr_GetCurStatus() == MFALSE){
                            //enforce cq load into reg before VF_EN if update rrz/tuning is needed
                            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_SINGLE_IMMEDIATE,(MINT32)EPIPECQ_TRIG_BY_START);
                            startCQ0_D();//for rrz configframe before start
                            sendCommand(EPIPECmd_SET_CQ_TRIGGER_MODE,(MINT32)EPIPE_PASS1_CQ0_D,(MINT32)EPIPECQ_TRIGGER_CONTINUOUS_EVENT,(MINT32)EPIPECQ_TRIG_BY_PASS1_DONE);
                        }
#endif
                        if ( 0 != m_CamPathPass1.updateCQI()){
                            if(m_CamPathPass1.CQ == CAM_ISP_CQ0)
                                PIPE_ERR("[3ASync]updateCQ Fail");
                            else if(m_CamPathPass1.CQ_D == CAM_ISP_CQ0_D)
                                PIPE_ERR("[3ASync]updateCQ_D Fail");
                            else{
                                PIPE_ERR("[3ASync]CQ ERROR 0x%x_0x%x\n",m_CamPathPass1.CQ,m_CamPathPass1.CQ_D);
                            }
                            ret = -1;
                        }
                    } else {
                        PIPE_DBG("[3ASync]:SET_IMGO D TG2I Magic0(0x%x)!= Magic1(0x%x) != magic2(0x%x)-", mMagicNum0_D, mMagicNum1_D,mMagicNum2_D);
                    }
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
             if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))||
                ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))) {
                PIPE_ERR("GET_MODULE_HANDLE: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                return MFALSE;
             }
             if (EPortIndex_UNKNOW == tg ) {
                 CAM_LOGE("GET_MODULE_HANDLE:User(%s), tg=%d is Invalid",arg3, tg);
                 return -1;
             }
             //CAM_LOGD("GET_MODULE_HANDLE:User(%s)!, tg=%d",arg3, tg);
             *((MINTPTR*)arg2) = (MINTPTR) (m_P1HWRWctrl[tg].getModuleHandle((CAM_MODULE_ENUM)p1ConvModuleID(arg1), (char const*)arg3));
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
            //TODO: refine later after setting path confirmed
            if ( EModule_CAMSV_IMGO == arg1)  {
                IspDrv* pPhyIspDrv = this->m_pIspDrvShell->getPhyIspDrv();
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_CAMSV_CLK_EN,0x00008005,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_CAMSV_INT_EN ,0x481,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_TG_VF_CON,0x00001001,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_CAMSV_MODULE_EN,0x40000015,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_CAMSV_PAK,0x03,ISP_DRV_USER_ISPF);
                ISP_WRITE_REG(pPhyIspDrv,CAMSV_CAMSV_FMT_SEL,0x01000000, ISP_DRV_USER_ISPF);
                ret = 0;
            }
            else {
                tg = _p1GetModuleTgIdx(arg1,this->m_pass1TwinMode);
                if (EPortIndex_UNKNOW == tg ) {
                    CAM_LOGE("SET_MODULE_EN:User(%s), tg=%d is Invalid",arg3, tg);
                    return -1;
                }
                if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))||
                            ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))) {
                            PIPE_ERR("SET_MODULE_EN: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                            return MFALSE;
                        }
                if(m_P1HWRWctrl[tg].setModuleEn((CAM_MODULE_ENUM)p1ConvModuleID(arg1),(MBOOL)arg2)!= MTRUE)
                    ret = -1;
            }
            break;

        case EPIPECmd_SET_MODULE_SEL:
            //arg1 : MODULE-Name;
            //arg2 : MBOOL Select(TRUE)/DeSel(FALSE)
            tg = _p1ConvSelectIdToTg(arg1,this->m_pass1TwinMode);
            if (EPortIndex_UNKNOW == tg ) {
                CAM_LOGE("SET_MODULE_SEL:User(%s), tg=%d is Invalid",arg3, tg);
                return -1;
            }
            if (((EPortIndex_TG1I == tg) && (!(this->mPath1 & ISP_PASS1)))||
                ((EPortIndex_TG2I == tg) && (!(this->mPath1 & ISP_PASS1_D)))) {
                PIPE_ERR("SET_MODULE_SEL: User(%s), tg=%d NotMatch Path=0x%x",arg3, tg, mPath1);
                return MFALSE;
            }
            if(m_P1HWRWctrl[tg].setModuleSel(p1ConvModuleToSelectID(arg1), (MUINT8)arg2)!= MTRUE)
                ret = -1;
            break;

        case EPIPECmd_SET_MODULE_DBG_DUMP: //dbg only
            //arg1 : handle;
            tg = _p1GetModuleTgIdx(arg1,this->m_pass1TwinMode);
            if(m_P1HWRWctrl[tg].dbgDump(arg1, tg)!= MTRUE)
                ret = -1;
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
        case EPIPECmd_GET_CUR_SOF_IDX:
            {
                MUINT32 _flag[6] = {0};
                IspDrv* pDrv = NULL;
                _flag[3] = 1;
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
                pDrv = this->m_pIspDrvShell->getPhyIspDrv();
                pDrv->dumpDBGLog(_flag,NULL);
                *(MUINT32*)arg1 = _flag[0];
            }
            break;
        case EPIPECmd_GET_CUR_FRM_RAWFMT:
            {
                MUINT32 cam_crl_sel;
                IspDrv* pDrv = NULL;
                pDrv = this->m_pIspDrvShell->getPhyIspDrv();
                if (arg2 == EPortIndex_TG1I)
                {
                    cam_crl_sel = pDrv->readReg(0x00004034);
                }
                else if (arg2 == EPortIndex_TG2I)
                {
                    cam_crl_sel = pDrv->readReg(0x00004038);
                }
                if (cam_crl_sel & 0x40)
                {
                    //ISP_PURE_RAW
                    *(MUINT32*)arg1 = 0x01;
                }
                else
                {
                    //ISP_RROCESSED_RAW
                    *(MUINT32*)arg1 = 0x00;
                }
            }
            break;
        default:
            PIPE_ERR("NOT support command!");
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

