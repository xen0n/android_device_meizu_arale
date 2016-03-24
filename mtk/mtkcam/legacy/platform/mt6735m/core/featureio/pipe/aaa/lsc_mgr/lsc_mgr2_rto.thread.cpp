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
#define LOG_TAG "lsc_mgr2_rto_thread"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "lsc_mgr2.h"

#include <cutils/properties.h>
#include <aaa_log.h>

#include <mtkcam/v1/config/PriorityDefs.h>

#include <mtkcam/hwutils/CameraProfile.h>

#include <shading_tuning_custom.h>

#include <mtkcam/algorithm/libtsf/MTKTsf.h>
#include <mtkcam/algorithm/liblsctrans/ShadingTblTransform.h>

#include <awb_param.h>
#include <awb_tuning_custom.h>
#include <isp_tuning_mgr.h>

namespace NSIspTuning {

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))

#define TSF_SCN_DFT LscMgr2::LSC_SCENARIO_PRV

#define GET_PROP(prop, init, val)\
{\
    char value[PROPERTY_VALUE_MAX] = {'\0'};\
    property_get(prop, value, (init));\
    (val) = atoi(value);\
}

#define LSC_LOG_BEGIN(fmt, arg...)  MY_LOG("[%s +] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG_END(fmt, arg...)    MY_LOG("[%s -] " fmt, __FUNCTION__, ##arg)
#define LSC_LOG(fmt, arg...)        MY_LOG("[%s] " fmt, __FUNCTION__, ##arg)
#define LSC_ERR(fmt, arg...)        MY_ERR("[%s] " fmt, __FUNCTION__, ##arg)

#define TSF_THREAD_BUILD
#define PER_FRAME_RESET 0xffffffff
#define PER_FRAME_INIT  0xfffffffe

#define TSF_LOCK() ::pthread_mutex_lock(&m_Mutex)
#define TSF_UNLOCK() ::pthread_mutex_unlock(&m_Mutex)

MVOID
LscMgr2Rto::
updateTsfCfg(UINT32 u4W, UINT32 u4H, const ISP_NVRAM_LSC_T& rLscCfg, ELscScenario_T eLscScn, MBOOL fgForce)
{
#if defined(TSF_BUILD) && !defined(LSC_DBG)
    LSC_LOG_BEGIN();

    TSF_LOCK();

    MBOOL fgBlkChg = 
        (m_rTsfLscParam.x_grid_num != rLscCfg.ctl2.bits.SDBLK_XNUM+2)||
        (m_rTsfLscParam.y_grid_num != rLscCfg.ctl3.bits.SDBLK_YNUM+2);
    MBOOL fgAspectChg = 
        (m_rTsfLscParam.raw_wd*u4H != m_rTsfLscParam.raw_ht*u4W);

    m_rTsfLscParam.raw_wd        = u4W;
    m_rTsfLscParam.raw_ht        = u4H;

    m_rTsfLscParam.x_offset      = rLscCfg.ctl1.bits.SDBLK_XOFST;
    m_rTsfLscParam.y_offset      = rLscCfg.ctl1.bits.SDBLK_YOFST;
    m_rTsfLscParam.block_wd      = rLscCfg.ctl2.bits.SDBLK_WIDTH;
    m_rTsfLscParam.block_ht      = rLscCfg.ctl3.bits.SDBLK_HEIGHT;
    m_rTsfLscParam.x_grid_num    = rLscCfg.ctl2.bits.SDBLK_XNUM+2;
    m_rTsfLscParam.y_grid_num    = rLscCfg.ctl3.bits.SDBLK_YNUM+2;
    m_rTsfLscParam.block_wd_last = rLscCfg.lblock.bits.SDBLK_lWIDTH;
    m_rTsfLscParam.block_ht_last = rLscCfg.lblock.bits.SDBLK_lHEIGHT;

    if (m_pTsfRatio)
    {
        m_pTsfRatio->updateCfg(rLscCfg);
    }

    LSC_LOG("raw_wd(%d), raw_ht(%d), block_wd(%d), block_ht(%d), xgrid(%d), ygrid(%d), wd_last(%d), ht_last(%d)",
        m_rTsfLscParam.raw_wd,
        m_rTsfLscParam.raw_ht,
        m_rTsfLscParam.block_wd,
        m_rTsfLscParam.block_ht,
        m_rTsfLscParam.x_grid_num,
        m_rTsfLscParam.y_grid_num,
        m_rTsfLscParam.block_wd_last,
        m_rTsfLscParam.block_ht_last);

    //
    if (!m_bTSFInit || fgBlkChg || fgAspectChg || fgForce)
    {
        LSC_LOG("Need to reset table, eLscScn(%d), fgBlkChg(%d), fgAspectChg(%d)", eLscScn, fgBlkChg, fgAspectChg);
        tsfResetTbl(eLscScn);
        tsfPostCmd(E_TSF_CMD_INIT);
        tsfPostCmd(E_TSF_CMD_CHG);
        m_bTSFInit = MTRUE;
        m_u4PerFrameStep = PER_FRAME_RESET;
    }
    else
    {
        LSC_LOG("No need to reset table");
        MBOOL fgOnOff = getTsfOnOff();
        if (fgOnOff)
        {
            updateLsc();
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
    }

    MBOOL fgDumpEn = (m_u4LogEn & EN_LSC_LOG_TSF_DUMP) ? MTRUE : MFALSE;
    if (fgDumpEn)
    {
        ISP_NVRAM_LSC_T rLscCfg;
        rLscCfg.ctl1.bits.SDBLK_XOFST       = m_rTsfLscParam.x_offset;
        rLscCfg.ctl1.bits.SDBLK_YOFST       = m_rTsfLscParam.y_offset;
        rLscCfg.ctl2.bits.SDBLK_WIDTH       = m_rTsfLscParam.block_wd;
        rLscCfg.ctl3.bits.SDBLK_HEIGHT      = m_rTsfLscParam.block_ht;
        rLscCfg.ctl2.bits.SDBLK_XNUM        = m_rTsfLscParam.x_grid_num - 2;
        rLscCfg.ctl3.bits.SDBLK_YNUM        = m_rTsfLscParam.y_grid_num - 2;
        rLscCfg.lblock.bits.SDBLK_lWIDTH    = m_rTsfLscParam.block_wd_last;
        rLscCfg.lblock.bits.SDBLK_lHEIGHT   = m_rTsfLscParam.block_ht_last;

        if (dumpSdblk("/sdcard/tsf1/tsfCfgChg", rLscCfg, reinterpret_cast<MUINT32*>(m_pu4ProcTbl)) != 0)
            LSC_ERR("dumpSdblk error");
    }

    TSF_UNLOCK();
    LSC_LOG_END();
#endif
}

MVOID
LscMgr2Rto::
initThreadFunc()
{
    LSC_LOG_BEGIN();

    m_u4PerFrameStep = PER_FRAME_RESET;
    m_prTsf = MTKTsf::createInstance();

    // create tsf instance
    if (!m_prTsf || !m_pTsfRatio)
    {
        LSC_ERR("NULL TSF instance");
        m_eTsfCmd = E_TSF_CMD_IDLE;
        m_fgThreadLoop = 0;
    }
    else
    {
        AWB_STAT_PARAM_T rAwbStatParma;

        switch (m_eSensorDev)
        {
        default:
        case ESensorDev_Main:
            rAwbStatParma = getAWBStatParam<ESensorDev_Main>();
            break;
        case ESensorDev_Sub:
            rAwbStatParma = getAWBStatParam<ESensorDev_Sub>();
            break;
        case ESensorDev_MainSecond:
            rAwbStatParma = getAWBStatParam<ESensorDev_MainSecond>();
            break;
        }
        
        m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_ENV_INFO, 0, &m_rTsfGetEnvInfo);
        LSC_LOG("MTKTSF_FEATURE_GET_ENV_INFO, buffer size(%d)", m_rTsfGetEnvInfo.WorkingBuffSize);

        // allocate working buffer
        MUINT8* gWorkinBuffer = new MUINT8[m_rTsfGetEnvInfo.WorkingBuffSize];
        ::memset(gWorkinBuffer, 0, m_rTsfGetEnvInfo.WorkingBuffSize);

        // allocate AWB info
        m_rTsfRefInfo.u4SizeAwbStat = (rAwbStatParma.i4WindowNumX * rAwbStatParma.i4WindowNumY * 4);
        m_rTsfRefInfo.prAwbStat = new MUINT8[m_rTsfRefInfo.u4SizeAwbStat];
        m_prAwbStat = new MUINT8[m_rTsfRefInfo.u4SizeAwbStat];
        LSC_LOG("AWB stat alloc size(%d), tmp(%p), algo(%p)", m_rTsfRefInfo.u4SizeAwbStat, m_rTsfRefInfo.prAwbStat, m_prAwbStat);

        // allocate table
        m_pu4ProcTbl = new MUINT32[MAX_SHADING_CapTil_SIZE];
        m_pu4ResultTbl = new MUINT32[MAX_SHADING_CapTil_SIZE];
        for (int i = 0; i < 4; i++)
        {
            m_pu4ResultTblQ[i] = new MUINT32[MAX_SHADING_CapTil_SIZE];
        }        
        // ra
        m_pu4RaResultTbl = new MUINT32[MAX_SHADING_CapTil_SIZE];

        // construct data relationship
        m_rTsfEnvInfo.ImgWidth    = rAwbStatParma.i4WindowNumX;
        m_rTsfEnvInfo.ImgHeight   = rAwbStatParma.i4WindowNumY;
        m_rTsfEnvInfo.BayerOrder  = MTK_BAYER_B;
    
        m_rTsfEnvInfo.WorkingBufAddr = (MUINT32*)gWorkinBuffer;
        m_rTsfEnvInfo.pLscConfig = &m_rTsfLscParam;
        m_rTsfEnvInfo.BaseShadingTbl = (MINT32*)m_pu4ProcTbl;
        m_rTsfEnvInfo.ImgAddr = m_prAwbStat;

        tsfResetTbl(TSF_SCN_DFT);

        m_rTsfTbl.pLscConfig = &m_rTsfLscParam;
        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4ProcTbl;

        m_rTsfSetProc.LscRA = 32;
        m_rTsfSetProc.ShadingTbl = (MINT32*)m_pu4ProcTbl;
        m_rTsfResult.ShadingTbl = m_pu4ResultTbl;

        // for exif
        m_pTsfResultInfo = &m_rTsfResult;

        // golden/unit alignment
        if (m_i4OtpState == E_LSC_WITH_MTK_OTP /*m_rTsfEnvInfo.WithOTP*/)
        {
            loadOtpDataForTsf();
        }
        else
        {
            LSC_LOG("No OTP Data");
            m_rTsfEnvInfo.ShadingTbl.Golden.Tbl = NULL;
            m_rTsfEnvInfo.ShadingTbl.Unit.Tbl = NULL;
        }

        m_bTSFInit = MFALSE;
    }
    
    LSC_LOG_END();
}

MVOID
LscMgr2Rto::
uninitThreadFunc()
{
    LSC_LOG_BEGIN();

    m_prTsf->TsfExit();
    m_prTsf->destroyInstance(m_prTsf);
    if (m_pu4RaResultTbl)
    {
        delete [] m_pu4RaResultTbl;
        m_pu4RaResultTbl = NULL;
    }
    for (int i = 0; i < 4; i++)
    {
        delete [] m_pu4ResultTblQ[i];
    }
    delete [] m_pu4ProcTbl;
    delete [] m_pu4ResultTbl;
    delete [] m_prAwbStat;
    delete [] m_rTsfRefInfo.prAwbStat;
    delete [] (MUINT8*)m_rTsfEnvInfo.WorkingBufAddr;

    if (m_rTsfEnvInfo.ShadingTbl.Golden.Tbl)
        delete [] m_rTsfEnvInfo.ShadingTbl.Golden.Tbl;

    if (m_rTsfEnvInfo.ShadingTbl.Unit.Tbl)
        delete [] m_rTsfEnvInfo.ShadingTbl.Unit.Tbl;

    LSC_LOG_END();
}

MBOOL
LscMgr2Rto::
tsfResetTbl(ELscScenario_T eLscScn)
{
    // reset proc shading table and result shading table
    MUINT32* pSrc = (MUINT32*)((MUINT8*)(getLut(eLscScn))+m_rTsfCfgTbl.TSF_CFG.tsfCtIdx*getPerLutSize(eLscScn));
    LSC_LOG("eLscScn(%d), m_pu4ProcTbl(%p), m_pu4ResultTbl(%p), src(%p)", eLscScn, m_pu4ProcTbl, m_pu4ResultTbl, pSrc);
    m_pu4TsfRaInputTbl = pSrc;
    ::memcpy(m_pu4ProcTbl, pSrc, getPerLutSize(eLscScn));
    ::memcpy(m_pu4ResultTbl, pSrc, getPerLutSize(eLscScn));
    return MTRUE;
}

MBOOL
LscMgr2Rto::
tsfSetProcInfo(MTK_TSF_SET_PROC_INFO_STRUCT& rProcInfo, MBOOL fgDump, const char* filename)
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_SET_PROC) ? MTRUE : fgDump;
    
    const MINT32* pAwbForceParam = m_rTsfCfgTbl.TSF_CFG.rAWBInput;

    if (m_bTsfForceAwb && pAwbForceParam)
    {
        rProcInfo.ParaL       	= pAwbForceParam[0];
        rProcInfo.ParaC       	= pAwbForceParam[1];
        rProcInfo.FLUO_IDX		= pAwbForceParam[2];
        rProcInfo.DAY_FLUO_IDX	= pAwbForceParam[3];
    }
    else
    {
        rProcInfo.ParaL       	= m_rTsfRefInfo.rAwbInfo.m_i4LV;
        rProcInfo.ParaC       	= m_rTsfRefInfo.rAwbInfo.m_u4CCT;
        rProcInfo.FLUO_IDX		= m_rTsfRefInfo.rAwbInfo.m_FLUO_IDX;
        rProcInfo.DAY_FLUO_IDX	= m_rTsfRefInfo.rAwbInfo.m_DAY_FLUO_IDX;
    }
    rProcInfo.Gain.i4R = m_rTsfRefInfo.rAwbInfo.m_RGAIN;
    rProcInfo.Gain.i4G = m_rTsfRefInfo.rAwbInfo.m_GGAIN;
    rProcInfo.Gain.i4B = m_rTsfRefInfo.rAwbInfo.m_BGAIN;
    rProcInfo.LscRA = 32;

    // AWB stat
    const MUINT8* pSrc = m_rTsfRefInfo.prAwbStat;
    MUINT8* pDst = m_prAwbStat;
    ::memcpy(pDst, pSrc, m_rTsfRefInfo.u4SizeAwbStat);

    if (fgDump)
    {
        FILE* fptr = fopen(filename, "wb");

        if (fptr)
        {
            fwrite(pSrc, m_rTsfRefInfo.u4SizeAwbStat, 1, fptr);
            fwrite(&rProcInfo, sizeof(MTK_TSF_SET_PROC_INFO_STRUCT), 1, fptr);
            fclose(fptr);
        }
    }

    MY_LOG_IF(fgLogEn, "[%s] ForceAwb(%d), L(%d), C(%d), F(%d), DF(%d), R(%d), G(%d), B(%d)\n",
        __FUNCTION__, m_bTsfForceAwb, rProcInfo.ParaL, rProcInfo.ParaC, rProcInfo.FLUO_IDX, rProcInfo.DAY_FLUO_IDX, rProcInfo.Gain.i4R, rProcInfo.Gain.i4G, rProcInfo.Gain.i4B);
    MY_LOG_IF(fgLogEn, "[%s] m_prAwbStat(%p), tmpBuf(%p), size(%d)",
        __FUNCTION__, pDst, pSrc, m_rTsfRefInfo.u4SizeAwbStat);

    return MTRUE;
}

MBOOL
LscMgr2Rto::
tsfInit()
{
    MRESULT ret = S_TSF_OK;
    
    LSC_LOG_BEGIN();

    m_prTsf->TsfExit();

    m_rTsfEnvInfo.Para        = m_rTsfCfgTbl.TSF_DATA;
    m_rTsfEnvInfo.pTuningPara = m_rTsfCfgTbl.TSF_PARA;

    m_rTsfEnvInfo.TS_TS       = 1;
    m_rTsfEnvInfo.MA_NUM      = 5;

    m_rTsfEnvInfo.WithOTP = (
        ((m_fg1to3 == E_LSC_123_WITH_MTK_OTP_OK) && 
         (m_i4OtpState==E_LSC_WITH_MTK_OTP)) 
          ? MTRUE : MFALSE);

    LSC_LOG("WithOTP(%d)", m_rTsfEnvInfo.WithOTP);
    
    m_rTsfEnvInfo.EnableORCorrection = (m_rTsfCfgTbl.TSF_PARA[411] == 0) ? 0 : 1;
    LSC_LOG("ORC(%d, %d)", m_rTsfEnvInfo.EnableORCorrection, m_rTsfCfgTbl.TSF_PARA[411]);
    // AWB NVRAM
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4R; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4G; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B   = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rUnitGain.i4B; // 512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4R; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4G; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rCalData.rGoldenGain.i4B; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4R; //809;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4G; //512;
    m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B    = m_prNvram3A->rAWBNVRAM[AWB_NVRAM_IDX_NORMAL].rAlgoCalParam.rPredictorGain.rAWBGain_LSC.i4B; //608;
    LSC_LOG("AwbNvramInfo: UnitGain(%d, %d, %d), GoldenGain(%d, %d, %d), D65Gain(%d, %d, %d)",
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4R  ,
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4G  ,
        m_rTsfEnvInfo.AwbNvramInfo.rUnitGain.i4B  ,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4R,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4G,
        m_rTsfEnvInfo.AwbNvramInfo.rGoldenGain.i4B,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4R   ,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4G   ,
        m_rTsfEnvInfo.AwbNvramInfo.rD65Gain.i4B   );    

    // SensorCrop    
    // 0: full 4:3 FOV
    // 1: full 16:9 FOV (full horizontal FOV, cropped vertical FOV)
    // 2: general cropping case
    m_rTsfEnvInfo.Raw16_9Mode = 2;
    #if 1
    const ShadingTblCropCfg_T& rCropCfg     = m_rSensorCropCfg[m_eSensorMode];
    m_rTsfEnvInfo.SensorCrop.full_width	    = rCropCfg.u4Width;  //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.full_height	= rCropCfg.u4Height; //m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.resize_width	= rCropCfg.u4OutputWidth; //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.resize_height	= rCropCfg.u4OutputHeight;//m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_width	    = rCropCfg.rCrop.u4W; //m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.crop_height	= rCropCfg.rCrop.u4H; //m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs	= rCropCfg.rCrop.u4X; //0;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs	= rCropCfg.rCrop.u4Y; //0;
    #else
    m_rTsfEnvInfo.SensorCrop.full_width	    = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.full_height	= m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.resize_width	= m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.resize_height	= m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_width	    = m_rTsfEnvInfo.pLscConfig->raw_wd;
    m_rTsfEnvInfo.SensorCrop.crop_height	= m_rTsfEnvInfo.pLscConfig->raw_ht;
    m_rTsfEnvInfo.SensorCrop.crop_hor_offs	= 0;
    m_rTsfEnvInfo.SensorCrop.crop_ver_offs	= 0;
    #endif
    LSC_LOG("SensorCrop(%d): Full(%d,%d), Resize(%d,%d), Crop(%d,%d,%d,%d)",
        m_eSensorMode,
        m_rTsfEnvInfo.SensorCrop.full_width	  ,
        m_rTsfEnvInfo.SensorCrop.full_height  ,
        m_rTsfEnvInfo.SensorCrop.resize_width ,
        m_rTsfEnvInfo.SensorCrop.resize_height,
        m_rTsfEnvInfo.SensorCrop.crop_hor_offs,
        m_rTsfEnvInfo.SensorCrop.crop_ver_offs,
        m_rTsfEnvInfo.SensorCrop.crop_width   ,
        m_rTsfEnvInfo.SensorCrop.crop_height  );


    switch( m_rOtp.TableRotation )
    {
        default:
        case 0:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_R0D;
        break;          
        case 1:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_R180D;
        break;    
        case 2:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_MIRROR;
        break;    
        case 3:
            m_rTsfEnvInfo.afn = MTKTSF_AFN_FLIP;
        break;    
    }
    // reset shading table
    //tsfResetTbl(m_eLscScenario /*TSF_SCN_DFT*/);

    MINT32 i4TsfSL2En = 0;
    GET_PROP("debug.lsc_mgr.sl2", "-1", i4TsfSL2En);

    if (i4TsfSL2En == -1)
    {
        // 0:disable, 1:TSF's SL2, 2:NVRAM default
        i4TsfSL2En = isEnableSL2(m_eSensorDev);
        LSC_LOG("TSF set SL2 default mode(%d)", i4TsfSL2En);
    }
    else
    {
        LSC_LOG("TSF set SL2 mode(%d)", i4TsfSL2En);
    }

    m_rTsfEnvInfo.EnableSL2   = i4TsfSL2En;

    LSC_LOG("ImgWidth(%d), ImgHeight(%d), BayerOrder(%d), ImgAddr(%p), ShadingTbl(%p), Raw16_9Mode(%d), EnableSL2(%d), pLscConfig(%p), OTPTableRotation(%d)",
        m_rTsfEnvInfo.ImgWidth, m_rTsfEnvInfo.ImgHeight, m_rTsfEnvInfo.BayerOrder, m_rTsfEnvInfo.ImgAddr,
        m_rTsfEnvInfo.BaseShadingTbl, m_rTsfEnvInfo.Raw16_9Mode, m_rTsfEnvInfo.EnableSL2, m_rTsfEnvInfo.pLscConfig, m_rTsfEnvInfo.afn);

    // init
    ret = m_prTsf->TsfInit(&m_rTsfEnvInfo, NULL);
    // set proc info
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);

    m_prTsf->TsfReset();
    m_bTSF1stGroup = 1;

    LSC_LOG_END();
    
    return MTRUE;
}

MBOOL
LscMgr2Rto::
tsfCfgChg()
{
    MRESULT ret = S_TSF_OK;
    
    // this is for identical sized tables, but block numbers are different.
    // no cropping support.
    // ex. 10x10 -> 16x16  
    LSC_LOG_BEGIN();
#if 0
    // convert current gain table from nxn to mxm (ex. 10x10 to 16x16)
    // assign base table for tsf
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    // convert transformed table to HW coef table.
    // generate table without breaking temporal smooth
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GEN_CAP_TBL, &m_rTsfTbl, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_GEN_CAP_TBL", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl, MTRUE);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }
#else
    tsfSetTbl(m_pu4TsfRaInputTbl, MTRUE);
#endif

lbExit:    
    LSC_LOG_END();
    
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2Rto::
tsfBatch()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN("step(0x%08x) ratio(%d)", m_u4PerFrameStep, m_i4Rto);

    checkTblEffective();

    // assign base table (ra) for tsf
    if (m_pTsfRatio)
    {
        if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
        {
            ret = E_TSF_ERR;
            goto lbExit;
        }
    }
    m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4RaResultTbl;

    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    tsfSetProcInfo(m_rTsfSetProc, MTRUE, "/sdcard/tsf1/aao_batch.bin");
    if (m_u4PerFrameStep == PER_FRAME_RESET)
    {
        // assign ra table for reversing
        // we should use m_pu4ProcTbl but don't know why m_pu4RaResultTbl is better for preview start
        m_rTsfSetProc.ShadingTbl = (MINT32*) m_pu4RaResultTbl; //m_pu4ProcTbl;     // default NVRAM shading table
    }
    else
    {
        MUINT32 u4Step = m_u4PerFrameStep;
        if (u4Step > 4)
        {
            LSC_ERR("Unexpected step(%d)", u4Step);
            goto lbExit;
        }
        u4Step = (((u4Step + 4) - 3) & 0x3);
        m_rTsfSetProc.ShadingTbl = (MINT32*)m_pu4ResultTblQ[u4Step];
    }

    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl, MTRUE);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }

lbExit:
    m_u4PerFrameStep = PER_FRAME_RESET;
    m_bTSF1stGroup = 0;

    LSC_LOG_END();
    
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2Rto::
tsfBatchCap()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN("step(0x%08x) ratio(%d)", m_u4PerFrameStep, m_i4Rto);

    MBOOL fgDumpEn = (m_u4LogEn & EN_LSC_LOG_TSF_DUMP) ? MTRUE : MFALSE;

    checkTblEffective();

    if (m_u4LogEn & EN_LSC_LOG_TSF_REINIT)
    {
        tsfInit();
    }
    else
    {
        m_prTsf->TsfReset();
    }

    // assign base table (ra) for tsf
    if (m_pTsfRatio)
    {
        // do ratio
        if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
        {
            ret = E_TSF_ERR;
            goto lbExit;
        }
    }    
    m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4RaResultTbl;
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
        m_prTsf->TsfReset();
        goto lbExit;
    }

    tsfSetProcInfo(m_rTsfSetProc, fgDumpEn, "/sdcard/tsf1/tsfAwbStat.bin");
    m_rTsfSetProc.ShadingTbl = (MINT32*) m_pu4ProcTbl; //m_pu4RaResultTbl;//m_pu4TsfRaInputTbl;     // default NVRAM shading table
    if (fgDumpEn)
    {
        ISP_NVRAM_LSC_T rLscCfg;
        rLscCfg.ctl1.bits.SDBLK_XOFST       = m_rTsfLscParam.x_offset;
        rLscCfg.ctl1.bits.SDBLK_YOFST       = m_rTsfLscParam.y_offset;
        rLscCfg.ctl2.bits.SDBLK_WIDTH       = m_rTsfLscParam.block_wd;
        rLscCfg.ctl3.bits.SDBLK_HEIGHT      = m_rTsfLscParam.block_ht;
        rLscCfg.ctl2.bits.SDBLK_XNUM        = m_rTsfLscParam.x_grid_num - 2;
        rLscCfg.ctl3.bits.SDBLK_YNUM        = m_rTsfLscParam.y_grid_num - 2;
        rLscCfg.lblock.bits.SDBLK_lWIDTH    = m_rTsfLscParam.block_wd_last;
        rLscCfg.lblock.bits.SDBLK_lHEIGHT   = m_rTsfLscParam.block_ht_last;

        if (dumpSdblk("/sdcard/tsf1/tsfInput", rLscCfg, reinterpret_cast<MUINT32*>(m_rTsfSetProc.ShadingTbl)) != 0)
            LSC_ERR("dumpSdblk error");
    }

    // disable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)0, 0);
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl, MTRUE);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }

lbExit:    
    // enable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)1, 0);
    m_u4PerFrameStep = PER_FRAME_RESET;

    LSC_LOG_END();
    
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2Rto::
tsfRun()
{
    MRESULT ret = S_TSF_OK;
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MY_LOG_IF(fgLogEn, "[%s +]", __FUNCTION__);

#if 1
    if (!checkTblEffective())
    {
        LSC_LOG("Bypass this run (%d)", m_u4PerFrameStep);
        goto lbExit;
    }
#endif
    if (m_u4PerFrameStep == PER_FRAME_RESET)
    {
        LSC_LOG("wait 1 frame right after batch");
        m_u4PerFrameStep = PER_FRAME_INIT;
        goto lbExit;
    }
    else if (m_u4PerFrameStep == PER_FRAME_INIT)
    {
        m_u4PerFrameStep = 0;
        // start to do 1
        // only set proc info at the 1st frame.
        tsfSetProcInfo(m_rTsfSetProc);
        // set lsc ratio here m_i4RtoApplied
        m_rTsfSetProc.ShadingTbl = (MINT32*)m_pu4ProcTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_PROC_INFO", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = PER_FRAME_INIT;
            goto lbExit;
        }
    }
    else if (m_u4PerFrameStep == 0)
    {
        // update frame 4 (0)
        tsfSetTbl(m_pu4ResultTblQ[m_u4PerFrameStep]);
        tsfSetSL2(m_rTsfResult.SL2Para);
        // backup the current ratio before update
        updateLsc();
        IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        // start to do 1
        // only set proc info at the 1st frame.
        tsfSetProcInfo(m_rTsfSetProc);
        // set lsc ratio here m_i4RtoApplied
        m_rTsfSetProc.ShadingTbl = (MINT32*)m_pu4ResultTblQ[2];
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_PROC_INFO", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = PER_FRAME_INIT;
            goto lbExit;
        }
    }
    else
    {
        // update frame 1, 2, 3
        tsfSetTbl(m_pu4ResultTblQ[m_u4PerFrameStep]);
        updateLsc();
        IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        // start to do 2, 3, 4
    }
    m_u4PerFrameStep ++;
    MY_LOG_IF(fgLogEn, "[%s] step(%d) ra(%d)", __FUNCTION__, m_u4PerFrameStep, m_i4Rto);

    if (m_rTsfGetProc.TsfState == MTKTSF_STATE_OPT_DONE)
    {
        if (m_pTsfRatio)
        {
            // do ratio
            if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4ProcTbl))
            {
                ret = E_TSF_ERR;
                LSC_ERR("Error(0x%08x): Do Ratio Error", ret);
                m_u4PerFrameStep = PER_FRAME_INIT;
                goto lbExit;
            }
        }

        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4ProcTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = PER_FRAME_INIT;
            goto lbExit;
        }
    }
    ret = m_prTsf->TsfMain();
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): TsfMain", ret);
        m_prTsf->TsfReset();
        m_u4PerFrameStep = PER_FRAME_INIT;
        goto lbExit;
    }
    
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &m_rTsfGetProc);
    if (m_rTsfGetProc.TsfState == MTKTSF_STATE_READY)
    {
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, NULL, &m_rTsfResult);
        m_bTSF1stGroup = 0;
        m_u4PerFrameStep = 0;
        ::memcpy(m_pu4ResultTblQ[m_u4PerFrameStep], m_rTsfResult.ShadingTbl, getPerLutSize(TSF_SCN_DFT));
    }
    else
    {
        // frame 1, 2, 3: only main is executed, just need to merge ra table.
        if (m_bTSF1stGroup == 0)
        {
            if (m_pTsfRatio)
            {
                // do ratio
                if (!m_pTsfRatio->genGainTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
                {
                    ret = E_TSF_ERR;
                    LSC_ERR("Error(0x%08x): Do Ratio Error", ret);
                    m_u4PerFrameStep = PER_FRAME_INIT;
                    goto lbExit;
                }
            }
            // do merge
            MTK_TSF_MERGE_TBL_STRUCT rMergeTbl;
            rMergeTbl.InLumaTbl = (MINT32*)m_pu4RaResultTbl; // gain
            rMergeTbl.OutCoefTbl = (MINT32*) m_pu4ResultTblQ[m_u4PerFrameStep]; //m_pu4ProcTbl;
            ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_MERGE_LUMA_TBL, &rMergeTbl, 0);
            //::memcpy(m_pu4ResultTblQ[m_u4PerFrameStep], m_pu4ProcTbl, getPerLutSize(TSF_SCN_DFT));
        }
        else
        {
            ::memcpy(m_pu4ResultTblQ[m_u4PerFrameStep], m_pu4TsfRaInputTbl, getPerLutSize(TSF_SCN_DFT));
        }
    }

lbExit:

    MY_LOG_IF(fgLogEn, "[%s -](0x%08x)", __FUNCTION__, ret);
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2Rto::
tsfPostCmd(E_TSF_CMD_T eCmd)
{
    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_THREAD) ? MTRUE : MFALSE;
    MBOOL fgOnOff = getTsfOnOff();
    
    switch (eCmd)
    {
    case E_TSF_CMD_INIT:
        LSC_LOG("TSF(%s), eCmd(E_TSF_CMD_INIT)", (fgOnOff?"On":"Off"));
        tsfInit();
        if (fgOnOff)
        {
            //updateLsc();
            //IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
        break;
    case E_TSF_CMD_CHG:
        LSC_LOG("TSF(%s), eCmd(E_TSF_CMD_CHG)", (fgOnOff?"On":"Off"));
        tsfCfgChg();
        if (fgOnOff)
        {
            updateLsc();
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
        break;
    case E_TSF_CMD_BATCH:
        MY_LOG_IF(fgLogEn, "[%s] TSF(%s), eCmd(E_TSF_CMD_BATCH)", __FUNCTION__, (fgOnOff?"On":"Off"));
        if (fgOnOff)
        {        
            tsfBatch();
            updateLsc();
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
        break;
    case E_TSF_CMD_BATCH_CAP:
        MY_LOG_IF(fgLogEn, "[%s] TSF(%s), eCmd(E_TSF_CMD_BATCH_CAP)", __FUNCTION__, (fgOnOff?"On":"Off"));
        if (fgOnOff)
        {        
            tsfBatchCap();
            updateLsc();
            IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
        }
        break;
    case E_TSF_CMD_RUN:
        if (fgOnOff)
        {
            MY_LOG_IF(fgLogEn, "[%s] SensorDev(%d) TSF ON, eCmd(E_TSF_CMD_RUN)", __FUNCTION__, m_eSensorDev);
            tsfRun();
        }
        break;
    default:
        LSC_LOG("TSF ON, eCmd(E_TSF_CMD_IDLE)");
        break;
    }

    return MTRUE;
}


//=============================================================================================
//
//=============================================================================================
MBOOL
LscMgr2RtoCycle::
tsfBatch()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN();

    if (m_pTsfRatio)
    {
        // do ratio
        if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
        {
            ret = E_TSF_ERR;
            goto lbExit;
        }

        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4RaResultTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
            m_prTsf->TsfReset();
            goto lbExit;
        }
    }

    tsfSetProcInfo(m_rTsfSetProc);
    m_rTsfSetProc.ShadingTbl = (MINT32*) m_pu4RaResultTbl;
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }
    
lbExit:
    m_u4PerFrameStep = 0;

    MY_LOG("[%s -](0x%08x)", __FUNCTION__, ret);
   
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2RtoCycle::
tsfBatchCap()
{
    MRESULT ret = S_TSF_OK;

    LSC_LOG_BEGIN();

    MBOOL fgDumpEn = (m_u4LogEn & EN_LSC_LOG_TSF_DUMP) ? MTRUE : MFALSE;

    if (fgDumpEn)
    {
        ISP_NVRAM_LSC_T rLscCfg;
        rLscCfg.ctl1.bits.SDBLK_XOFST       = m_rTsfLscParam.x_offset;
        rLscCfg.ctl1.bits.SDBLK_YOFST       = m_rTsfLscParam.y_offset;
        rLscCfg.ctl2.bits.SDBLK_WIDTH       = m_rTsfLscParam.block_wd;
        rLscCfg.ctl3.bits.SDBLK_HEIGHT      = m_rTsfLscParam.block_ht;
        rLscCfg.ctl2.bits.SDBLK_XNUM        = m_rTsfLscParam.x_grid_num - 2;
        rLscCfg.ctl3.bits.SDBLK_YNUM        = m_rTsfLscParam.y_grid_num - 2;
        rLscCfg.lblock.bits.SDBLK_lWIDTH    = m_rTsfLscParam.block_wd_last;
        rLscCfg.lblock.bits.SDBLK_lHEIGHT   = m_rTsfLscParam.block_ht_last;

        if (dumpSdblk("/sdcard/tsf2/tsfInput", rLscCfg, reinterpret_cast<MUINT32*>(m_rTsfSetProc.ShadingTbl)) != 0)
            LSC_ERR("dumpSdblk error");
    }

    if (m_u4LogEn & EN_LSC_LOG_TSF_REINIT)
    {
        tsfInit();
    }
    else
    {
        m_prTsf->TsfReset();
    }

    if (m_pTsfRatio)
    {
        // do ratio
        if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
        {
            ret = E_TSF_ERR;
            goto lbExit;
        }

        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4RaResultTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
            m_prTsf->TsfReset();
            goto lbExit;
        }
    }
    
    tsfSetProcInfo(m_rTsfSetProc, fgDumpEn, "/sdcard/tsf2/tsfAwbStat.bin");
    m_rTsfSetProc.ShadingTbl = (MINT32*) m_pu4ProcTbl;
    // disable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)0, 0);
    ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_BATCH, &m_rTsfSetProc, &m_rTsfResult);
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_BATCH", ret);
        m_prTsf->TsfReset();
    }
    else
    {
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
    }
    
    // enable temporal smooth
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_CONFIG_SMOOTH, (void*)1, 0);
    
lbExit:
    m_u4PerFrameStep = 0;

    MY_LOG("[%s -](0x%08x)", __FUNCTION__, ret);
    
    return ret == S_TSF_OK;
}

MBOOL
LscMgr2RtoCycle::
tsfRun()
{
    MRESULT ret = S_TSF_OK;

    MBOOL fgLogEn = (m_u4LogEn & EN_LSC_LOG_TSF_RUN) ? MTRUE : MFALSE;

    MY_LOG_IF(fgLogEn, "[%s +] Rto(%d) step(%d)", __FUNCTION__, m_i4Rto, m_u4PerFrameStep);

    if (m_u4PerFrameStep == 0)
    {
        // only set proc info at the 1st frame.
        tsfSetProcInfo(m_rTsfSetProc);
        m_rTsfSetProc.ShadingTbl = (MINT32*) m_pu4ProcTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_PROC_INFO, &m_rTsfSetProc, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_PROC_INFO", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = 0;
            goto lbExit;
        }
    }

    if (m_pTsfRatio && m_rTsfGetProc.TsfState == MTKTSF_STATE_OPT_DONE)
    {
        // do ratio
        if (!m_pTsfRatio->genHwTbl(m_i4Rto, m_pu4TsfRaInputTbl, m_pu4RaResultTbl))
        {
            ret = E_TSF_ERR;
            LSC_ERR("Error(0x%08x): Do Ratio Error", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = 0;
            goto lbExit;
        }

        m_rTsfTbl.ShadingTbl = (MINT32*)m_pu4RaResultTbl;
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_SET_TBL_CHANGE, &m_rTsfTbl, NULL);
        if (ret != S_TSF_OK)
        {
            LSC_ERR("Error(0x%08x): MTKTSF_FEATURE_SET_TBL_CHANGE", ret);
            m_prTsf->TsfReset();
            m_u4PerFrameStep = 0;
            goto lbExit;
        }
    }

    m_u4PerFrameStep ++;
    ret = m_prTsf->TsfMain();
    if (ret != S_TSF_OK)
    {
        LSC_ERR("Error(0x%08x): TsfMain", ret);
        m_prTsf->TsfReset();
        m_u4PerFrameStep = 0;
        goto lbExit;
    }
    
    m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_PROC_INFO, 0, &m_rTsfGetProc);
    if (m_rTsfGetProc.TsfState == MTKTSF_STATE_READY)
    {
        ret = m_prTsf->TsfFeatureCtrl(MTKTSF_FEATURE_GET_RESULT, NULL, &m_rTsfResult);
        tsfSetTbl(m_rTsfResult.ShadingTbl);
        tsfSetSL2(m_rTsfResult.SL2Para);
        m_u4PerFrameStep = 0;
        // batch the current ratio before update
        m_i4RtoFor1stFrm = m_i4RtoApplied;
        updateLsc();
        IspTuningMgr::getInstance().forceValidate(m_eSensorDev);
    }

lbExit:
    MY_LOG_IF(fgLogEn, "[%s -](0x%08x)", __FUNCTION__, ret);

    return ret == S_TSF_OK;
}

};
