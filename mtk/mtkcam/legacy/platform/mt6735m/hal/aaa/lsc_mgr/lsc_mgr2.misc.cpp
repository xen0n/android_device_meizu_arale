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

#define LOG_TAG "lsc_mgr2_misc"
#ifndef ENABLE_MY_LOG
#define ENABLE_MY_LOG           (1)
#define GLOBAL_ENABLE_MY_LOG    (1)
#endif

//#define LSC_DBG

#include "lsc_mgr2.h"

#include <cutils/properties.h>
#include <aaa_log.h>

#include <isp_mgr.h>
//#include <mtkcam/featureio/capturenr.h>

using namespace std;
using namespace NSIspTuning;

namespace NSIspTuningv3 {

#define MAX(a,b)  ((a) < (b) ? (b) : (a))
#define MIN(a,b)  ((a) < (b) ? (a) : (b))
#define ABS(a)    ((a) > 0 ? (a) : -(a))


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

MRESULT
LscMgr2::
CCTOPSetBypass123(MBOOL fgBypass)
{
    LSC_LOG_BEGIN("fgBypass(%d)", fgBypass);

    if (m_fgBypass1to3 != fgBypass)
    {
        m_fgBypass1to3 = fgBypass;
        loadTableFlow(MTRUE);
    }

    LSC_LOG_END("m_fgBypass1to3(%d)", m_fgBypass1to3);

    return 0;
}

MRESULT
LscMgr2::
CCTOPSetSdblkFileCfg(MBOOL fgSave, const char* filename)
{
    m_bDumpSdblk = fgSave;
    m_strSdblkFile = filename;
    return 0;
}

MINT32
LscMgr2::
dumpSdblk(const char* table_name, const ISP_NVRAM_LSC_T& LscConfig, const MUINT32 *ShadingTbl)
{
    string strTblName(table_name);
    string strFilename;
    FILE *fhwtbl,*fsdblk;

    if (ShadingTbl == NULL)
    {
        LSC_ERR("NULL table");
        return -1;
    }

    LSC_LOG_BEGIN("ShadingTbl(%p)", ShadingTbl);

    strFilename = strTblName + ".sdblk";
    fsdblk = fopen(strFilename.c_str(), "w");
    if ( fsdblk == NULL )
    {
        LSC_ERR("Can't open: %s", (const char*) strFilename.c_str());
        return -1;
    }

    strFilename = strTblName + ".hwtbl";
    fhwtbl = fopen(strFilename.c_str(), "w");
    if ( fhwtbl == NULL )
    {
        LSC_ERR("Can't open: %s", (const char*) strFilename.c_str());
        return -1;
    }

    fprintf(fsdblk," %8d  %8d  %8d  %8d  %8d  %8d  %8d  %8d\n",
            LscConfig.ctl1.bits.SDBLK_XOFST,
            LscConfig.ctl1.bits.SDBLK_YOFST,
            LscConfig.ctl2.bits.SDBLK_WIDTH,
            LscConfig.ctl3.bits.SDBLK_HEIGHT,
            LscConfig.ctl2.bits.SDBLK_XNUM,
            LscConfig.ctl3.bits.SDBLK_YNUM,
            LscConfig.lblock.bits.SDBLK_lWIDTH,
            LscConfig.lblock.bits.SDBLK_lHEIGHT);

    MINT32 x_num = LscConfig.ctl2.bits.SDBLK_XNUM + 1;
    MINT32 y_num = LscConfig.ctl3.bits.SDBLK_YNUM + 1;

    MINT32 numCoef = x_num * y_num * 4 * 4;
    MINT32 i, c = 0;

    for (i = numCoef-1; i >= 0; i--)
    {
        MUINT32 coef1, coef2, coef3;
        MUINT32 val = *ShadingTbl++;
        coef3 = (val& 0x3FF00000) >> 20;
        coef2 = (val& 0x000FFC00) >> 10;
        coef1 = val& 0x000003FF;
        fprintf(fsdblk, " %8d %8d %8d", coef1, coef2, coef3);
        fprintf(fhwtbl,"0x%08x, ", val);
        c ++;

        if (c == 4)
        {
            c = 0;
            fprintf(fhwtbl,"\n");
            fprintf(fsdblk,"\n");
        }
    }

    fclose(fhwtbl);
    fclose(fsdblk);

    LSC_LOG_END();

    return 0;
}

inline void setDebugTag(DEBUG_RESERVEA_INFO_T &a_rCamDebugInfo, MINT32 a_i4ID, MINT32 a_i4Value)
{
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldID = CAMTAG(DEBUG_CAM_RESERVE1_MID, a_i4ID, 0);
    a_rCamDebugInfo.Tag[a_i4ID].u4FieldValue = a_i4Value;
}

MRESULT
LscMgr2::
getDebugInfo(DEBUG_RESERVEA_INFO_T &rShadingDbgInfo)
{
    ISP_NVRAM_LSC_T debug;

    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev).get(debug);
    MUINT32 u4Addr = debug.baseaddr.val; //ISP_MGR_LSC_T::getInstance(m_eSensorDev).getAddr();

    MINT32* pTsfExif = NULL;
    if (m_pTsfResultInfo)
    {
        pTsfExif = reinterpret_cast<MINT32*>(m_pTsfResultInfo->ExifData);
        LSC_LOG("pTsfExif(%p), ExifData(%p)", pTsfExif, &m_pTsfResultInfo->ExifData[0]);
    }

    ::memset(&rShadingDbgInfo, 0, sizeof(rShadingDbgInfo));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_VERSION, (MUINT32)SHAD_DEBUG_TAG_VERSION);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)m_fg1to3);
    // setDebugTag(rShadingDbgInfo, SHAD_TAG_1TO3_EN, (MUINT32)(m_rIspNvram.ISPComm.CommReg[EEPROM_INFO_IN_COMM_LOAD] == CAL_DATA_LOAD));
    setDebugTag(rShadingDbgInfo, SHAD_TAG_SCENE_IDX, (MUINT32)m_eLscScenario);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CT_IDX, (MUINT32)m_u4CTIdx);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_DMA_EN, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_BASE_ADDR, (MUINT32)u4Addr);
    //setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSCI_XSIZE, (MUINT32)debug.xsize.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_CTL_EN1, (MUINT32)m_fgOnOff);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL1, (MUINT32)debug.ctl1.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL2, (MUINT32)debug.ctl2.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_CTL3, (MUINT32)debug.ctl3.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_LBLOCK, (MUINT32)debug.lblock.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_RATIO, (MUINT32)debug.ratio.val);
    setDebugTag(rShadingDbgInfo, SHAD_TAG_CAM_LSC_GAIN_TH, (MUINT32)0/*debug.gain_th.val*/);

    // TSF related
    setDebugTag(rShadingDbgInfo, SHAD_TAG_TSF_EN, m_bTSF);
    if (pTsfExif && m_bTSF)
    {
        MINT32 i;
        #ifdef TSF_BUILD
        ::pthread_mutex_lock(&m_Mutex);
        #endif
        for (i = SHAD_TAG_CNT1; i < SHAD_TAG_END; i++)
        {
            setDebugTag(rShadingDbgInfo, i, *pTsfExif++);
        }
        #ifdef TSF_BUILD
        ::pthread_mutex_unlock(&m_Mutex);
        #endif
    }

    // SDBLK dump
    if (m_bDumpSdblk)
    {
        if (dumpSdblk(m_strSdblkFile.c_str(), debug, m_pBaseAddr) != 0)
            LSC_ERR("dumpSdblk error\n");
    }

    LSC_LOG_END();

    return 0;
}
#if 0
MRESULT
LscMgr2::
getDebugTbl(DEBUG_SHAD_ARRAY_INFO_T &rShadingDbgTbl, DEBUG_SHAD_ARRAY_2_T& rShadRestTbl)
{
    ISP_NVRAM_LSC_T debug;

    LSC_LOG_BEGIN();

    ISP_MGR_LSC_T::getInstance(m_eSensorDev, m_eSensorTG).get(debug);

    ::memset(&rShadingDbgTbl, 0, sizeof(DEBUG_SHAD_ARRAY_INFO_T));
    ::memset(&rShadRestTbl, 0, sizeof(DEBUG_SHAD_ARRAY_2_T));

    rShadingDbgTbl.hdr.u4KeyID = DEBUG_SHAD_TABLE_KEYID;
    rShadingDbgTbl.hdr.u4ModuleCount = ModuleNum<1, 0>::val;
    rShadingDbgTbl.hdr.u4DbgSHADArrayOffset = sizeof(DEBUG_SHAD_ARRAY_INFO_S::Header);

    rShadingDbgTbl.rDbgSHADArray.u4BlockNumX = debug.ctl2.bits.LSC_SDBLK_XNUM + 1;
    rShadingDbgTbl.rDbgSHADArray.u4BlockNumY = debug.ctl3.bits.LSC_SDBLK_YNUM + 1;

    MUINT32 u4Blocks =
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumX *
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumY;

    MUINT32 u4RestBlocks = 0;

    if (u4Blocks >= APPN_SHAD_BLOCK_NUM_MAX)
    {
        u4RestBlocks = u4Blocks - APPN_SHAD_BLOCK_NUM_MAX;
        u4Blocks = APPN_SHAD_BLOCK_NUM_MAX;
    }

    rShadingDbgTbl.rDbgSHADArray.u4CountU32 = u4Blocks*4*4;
    MUINT32* pu4Addr = m_pBaseAddr;
    ::memcpy(rShadingDbgTbl.rDbgSHADArray.u4Array, pu4Addr, u4Blocks*4*4*sizeof(MUINT32));

    rShadRestTbl.u4CountU32 = u4RestBlocks*4*4;
    if (u4RestBlocks)
    {
        ::memcpy(rShadRestTbl.u4Array, (MUINT32*) pu4Addr+SHAD_ARRAY_VALUE_SIZE, u4RestBlocks*4*4*sizeof(MUINT32));
    }

    LSC_LOG_END("X(%d),Y(%d),Cnt(%d),Cnt2(%d)",
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumX,
        rShadingDbgTbl.rDbgSHADArray.u4BlockNumY,
        rShadingDbgTbl.rDbgSHADArray.u4CountU32,
        rShadRestTbl.u4CountU32);

    return 0;
}

MBOOL
LscMgr2::
setSwNr()
{
    ISP_NVRAM_SL2_T rSl2Cfg;
    ISP_MGR_SL2_T::getInstance(m_eSensorDev).get(rSl2Cfg);

    LSC_LOG("SL2 cen(0x%08x), rr0(0x%08x), rr1(0x%08x), rr2(0x%08x)",
        rSl2Cfg.cen, rSl2Cfg.max0_rr, rSl2Cfg.max1_rr, rSl2Cfg.max2_rr);

    SwNRParam::getInstance(m_i4SensorIdx)->setSL2B(
        rSl2Cfg.cen.bits.SL2_CENTR_X,
        rSl2Cfg.cen.bits.SL2_CENTR_Y,
        rSl2Cfg.max0_rr.val,
        rSl2Cfg.max1_rr.val,
        rSl2Cfg.max2_rr.val);

    return MTRUE;
}
#endif

MINT32
LscMgr2::
getGainTable(MUINT32 u4Bayer, MUINT32 u4GridNumX, MUINT32 u4GridNumY, MFLOAT* pGainTbl)
{
    #warning "hardcode table"
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    *pGainTbl++ = 1.0f;
    return 0;
}

}
